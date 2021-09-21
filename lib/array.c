/*------------------------------------------------------------*
 | array.c                                                    |
 | copyright 1999,  Andrew Sumner (andrewsumner@yahoo.com)    |
 |                                                            |
 | This is a source file for the awka package, a translator   |
 | of the AWK programming language to ANSI C.                 |
 |                                                            |
 | This library is free software; you can redistribute it     |
 | and/or modify it under the terms of the GNU General        |
 | Public License (GPL).                                      |
 |                                                            |
 | This library is distributed in the hope that it will be    |
 | useful, but WITHOUT ANY WARRANTY; without even the implied |
 | warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR    |
 | PURPOSE.                                                   |
 *------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define _ARRAY_C

#define SLOT_GROWTH  20
#define STR_GROWTH   20

char _awka_setdol0_len = 0;
char _awka_setdoln = 0;
char _dol0_only = 0;
char _rebuild0 = 0, _rebuildn = 0, _rebuild0_now = 0;
char fs_or_fw = 0;  /* fs=0, fw=1, fpat=2 */

#define _IN_LIBRARY
#include "libawka.h"
#include "array_priv.h"
#include "number.h"
#include "garbage.h"
#include "builtin_priv.h"

char _a_space[256], *nullstr = "";
int _awka_dol0_len = 0;
int _split_req;
int _split_max;

int *fw_loc, fw_allc = 0, fw_used = 0;
int *sw_loc, sw_allc = 0, sw_used = 0;

static a_VAR *emptyvar = NULL;

static a_HSHNode * _awka_hshfindint( _a_HSHarray *, unsigned int, char, char );

static INLINE int
_awka_isanint( char *s )
{
  register char *p = s;

  if (!*p || (*p == '0' && *(p+1) != '\0'))
    return FALSE;

  while (*p)
    if (!isdigit(*p++))
      return FALSE;

  return TRUE;
}


#define mix(a, b, c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

/*
 * _awka_hashstr
 * calculates a reasonably unique int value from a string.
 * This is derived from public-domain code by Bob Jenkins.
 */
unsigned int
_awka_hashstr( char *str, register int len )
{
  typedef unsigned long int ub4;
  typedef unsigned char ub1;
  register char *p = str;
  register ub4 a,b,c,length=len;
  static ub4 last_hash = 10949823;

  if (len == 1)
    return *str;

  if (len < 8)
  {
    c = *p;

    for (a=1; a<len && *p; a++)
      c += (c << 4) + *p++;

    return (unsigned int) c;
  }

  a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
  c = (ub4) 9325281762;         /* the previous hash value */

  /*---------------------------------------- handle most of the key */
  while (len >= 12)
  {
    a += (str[0] +((ub4)str[1]<<8) +((ub4)str[2]<<16) +((ub4)str[3]<<24));
    b += (str[4] +((ub4)str[5]<<8) +((ub4)str[6]<<16) +((ub4)str[7]<<24));
    c += (str[8] +((ub4)str[9]<<8) +((ub4)str[10]<<16)+((ub4)str[11]<<24));
    mix(a,b,c);
    str += 12; len -= 12;
  }

  /*------------------------------------- handle the last 11 bytes */
  c += length;
  switch (len)              /* all the case statements fall through */
  {
    case 11: c+=((ub4)str[10]<<24);
    case 10: c+=((ub4)str[9]<<16);
    case 9 : c+=((ub4)str[8]<<8);
       /* the first byte of c is reserved for the length */
    case 8 : b+=((ub4)str[7]<<24);
    case 7 : b+=((ub4)str[6]<<16);
    case 6 : b+=((ub4)str[5]<<8);
    case 5 : b+=str[4];
    case 4 : a+=((ub4)str[3]<<24);
    case 3 : a+=((ub4)str[2]<<16);
    case 2 : a+=((ub4)str[1]<<8);
    case 1 : a+=str[0];
    /* case 0: nothing left to add */
  }

  mix(a,b,c);

  /*-------------------------------------------- report the result */
  return (unsigned int) c;
}

#define check_emptyvar() { \
  if (!emptyvar) { \
	  malloc( &emptyvar, sizeof(a_VAR) ); \
	  emptyvar->ptr = (char *) NULL; \
  	  emptyvar->allc = emptyvar->slen = emptyvar->temp = emptyvar->type2 = 0; \
	  emptyvar->dval = 0.0; \
	  emptyvar->type = a_VARNUL; \
  } \
}

/*
 * _awka_hshinitnode
 * prepares a new a_HSHNode for life in the outside world
 */
#define _awka_hshinitnode( node ) \
{ \
  (node)->next = NULL; \
  (node)->key = NULL; \
  (node)->hval = 0; \
  check_emptyvar(); \
  memcpy( (node)->var, emptyvar, sizeof(a_VAR) ); \
}

/*
 * _awka_splitinitnode
 * prepares a new a_HSHNode for life in the outside world
 */
#define _awka_splitinitnode( node ) \
{ \
  (node)->next = NULL; \
  (node)->hval = 0; \
  check_emptyvar(); \
  memcpy( (node)->var, emptyvar, sizeof(a_VAR) ); \
}

void
_awka_hshdouble( _a_HSHarray *array )
{
  a_HSHNode *node, *prevnode;
  unsigned int old_hashmask = array->hashmask, highbit, i, newi;

  array->hashmask = (array->hashmask * 2) + 1;
  realloc( &array->slot, (array->hashmask+1) * sizeof(a_HSHNode *) );

  memset(array->slot + old_hashmask + 1, 0, (old_hashmask + 1) * sizeof(a_HSHNode *));

  /* find highest bit in new hashmask */
  highbit = i = array->hashmask;
  newi = 0;

  while (i) {
    highbit = i;
    newi++;
    i = i >> 1;
  }

  newi--;
  highbit = highbit << newi;

  /* loop through nodes, relocating those with new addresses */
  for (i=0; i<=old_hashmask; i++)
  {
    if (!array->slot[i])
      continue;

    prevnode = NULL;
    node = array->slot[i];
    do
    {
      if (node->hval & highbit)
      {
        /* this wants to move */
        if (!prevnode)
          array->slot[i] = node->next;
        else
          prevnode->next = node->next;

        newi = node->hval & array->hashmask;

        if (newi <= old_hashmask)
          awka_error("array: internal corruption detected.\n");

        node->next = array->slot[newi];
        array->slot[newi] = node;

        if (!prevnode)
          node = array->slot[i];
        else
          node = prevnode->next;
      }
      else
      {
        /* this wants to stay */
        prevnode = node;
        node = node->next;
      }

    } while (node);
  }
}

static a_HSHNode *
_awka_hshfindstr(
  _a_HSHarray *array,
  char *key,
  int len,
  unsigned int hval,
  char create,
  char shadow
)
{
  register unsigned int idx;
  a_HSHNode *node, *node2 = NULL, *prevnode = NULL;

  idx = hval & array->hashmask;
  node = array->slot[idx];

  while (node)
  {
    if (node->type == _a_ARR_STR &&
        hval == node->hval &&
        *(node->key) == *key)
    {
      if (*key == '\0' || !strcmp(node->key, key))
      {
        /* got a match */
        if (create != a_ARR_DELETE)
        {
          if (prevnode)
          {
            prevnode->next = node->next;
            node->next = array->slot[idx];
          }

          array->last = array->slot[idx] = node;

          return node;
        }

        if (array->flag & _a_ARR_INT && shadow == FALSE)
        {
          if (!isalpha(key[0]) && _awka_isanint(key))
            _awka_hshfindint(array, atoi(key), a_ARR_DELETE, TRUE);
        }

        if (shadow == FALSE)
        {
          awka_killvar(node->var);

          if (node->var)
            free(node->var);
        }
        if (prevnode == NULL)
          array->slot[idx] = node->next;
        else
          prevnode->next = node->next;

        array->nodeno--;

        if (node->key)
          free(node->key);

        free(node);

        array->last = NULL;

        return node; /* this return value won't be used - don't worry */
      }
    }

    prevnode = node;
    node = node->next;
  }

  /* node not at home */
  if (create != a_ARR_CREATE)
    return NULL;

  malloc(&node, sizeof(a_HSHNode));
  malloc(&node->key, len+1);
  memcpy(node->key, key, len+1);
  node->shadow = shadow;
  node->hval = hval;
  node->type = _a_ARR_STR;
  array->last = node->next = array->slot[idx];
  array->slot[idx] = node;

  if (shadow == TRUE)
    return node;

  array->nodeno++;

  malloc( &node->var, sizeof(a_VAR));
  node->var->ptr = NULL;
  node->var->dval = 0.0;
  node->var->type = a_VARNUL;
  node->var->type2 = node->var->temp = node->var->allc = node->var->slen = 0;

  if (array->flag & _a_ARR_INT)
  {
    if (!isalpha(key[0]) && _awka_isanint(key))
    {
      node2 = _awka_hshfindint(array, atoi(key), create, TRUE);
      node2->var = node->var;
    }
  }

  return node;
}

static a_HSHNode *
_awka_hshfindint(
  _a_HSHarray *array,
  unsigned int hval,
  char create,
  char shadow
)
{
  register unsigned int idx, i;
  a_HSHNode *node, *node2 = NULL, *prevnode = NULL;
  static char buf[24];

  idx = hval & array->hashmask;
  node = array->slot[idx];

  while (node)
  {
    if (node->type == _a_ARR_INT && hval == node->hval)
    {
      /* got a match */
      if (create != a_ARR_DELETE)
      {
        if (prevnode && node == array->last)
        {
          prevnode->next = node->next;
          node->next = array->slot[idx];
          array->slot[idx] = node;
        }

        array->last = node;

        return node;
      }

      if (array->flag & _a_ARR_STR && shadow == FALSE)
      {
        sprintf(buf, "%d", hval);
        i = strlen(buf);
        _awka_hshfindstr(array, buf, strlen(buf), _awka_hashstr(buf, i), a_ARR_DELETE, TRUE);
      }

      if (shadow == FALSE)
      {
        awka_killvar(node->var);

        if (node->var)
          free(node->var);
      }

      if (prevnode == NULL)
        array->slot[idx] = node->next;
      else
        prevnode->next = node->next;

      array->nodeno--;

      free(node);

      array->last = NULL;

      return node;
    }

    prevnode = node;
    node = node->next;
  }

  /* node not at home */
  if (create != a_ARR_CREATE)
    return NULL;

  malloc( &node, sizeof(a_HSHNode));
  node->key = NULL;
  node->shadow = shadow;
  node->hval = hval;
  node->type = _a_ARR_INT;
  node->next = array->slot[idx];
  array->last = array->slot[idx] = node;

  if (shadow == TRUE)
    return node;

  array->nodeno++;

  malloc( &node->var, sizeof(a_VAR));
  node->var->ptr = NULL;
  node->var->dval = 0.0;
  node->var->type = a_VARNUL;
  node->var->type2 = node->var->temp = node->var->allc = node->var->slen = 0;

  if (array->flag & _a_ARR_STR)
  {
    sprintf(buf, "%d", hval);
    i = strlen(buf);
    node2 = _awka_hshfindstr(array, buf, strlen(buf), _awka_hashstr(buf, i), create, TRUE);
    node2->var = node->var;
  }

  return node;
}

void
_awka_hashtoint( _a_HSHarray *array )
{
  a_HSHNode *node, *node2;
  register int i;

  if (array->nodeno > _a_HSH_MAXDEPTH)
    _awka_hshdouble(array);

  for (i=0; i<=array->hashmask; i++)
  {
    node = array->slot[i];

    while (node)
    {
      if (node->shadow == TRUE || node->type != _a_ARR_STR)
      {
        node = node->next;
        continue;
      }

      if (node->key[0] && !isalpha(node->key[0]) && _awka_isanint(node->key))
      {
        node2 = _awka_hshfindint(array, atoi(node->key), a_ARR_CREATE, TRUE);
        node2->var = node->var;
      }

      node = node->next;
    }
  }

  array->flag |= _a_ARR_INT;
}

void
_awka_hashtostr( _a_HSHarray *array )
{
  a_HSHNode *node, *node2;
  register int i, j;
  static char buf[24];

  if (array->nodeno > _a_HSH_MAXDEPTH)
    _awka_hshdouble(array);

  for (i=0; i<=array->hashmask; i++)
  {
    node = array->slot[i];

    while (node)
    {
      if (node->shadow == TRUE || node->type != _a_ARR_INT)
      {
        node = node->next;
        continue;
      }

      sprintf(buf, "%d", node->hval);
      j = strlen(buf);
      node2 = _awka_hshfindstr(array, buf, j, _awka_hashstr(buf, j), a_ARR_CREATE, TRUE);
      node2->var = node->var;
      node = node->next;
    }
  }

  array->flag |= _a_ARR_STR;
}

/*
 * _awka_split2hsh
 * Changes an array created by 'split' to a
 * general-purpose HSH array.
 */
_a_HSHarray *
_awka_split2hsh( _a_HSHarray *array )
{
  a_HSHNode *node;
  _a_HSHarray *newarray;
  int i;

  if (array->type != a_ARR_TYPE_SPLIT)
    return NULL;

  malloc( &newarray, sizeof(_a_HSHarray));
  newarray->hashmask = _a_HASHMASK;
  malloc( &newarray->slot, (_a_HASHMASK+1) * sizeof(a_HSHNode *) );
  newarray->type = a_ARR_TYPE_HSH;
  newarray->splitstr = NULL;
  newarray->splitallc = 0;
  newarray->nodeno = 0;
  newarray->flag = _a_ARR_INT;
  newarray->subscript = array->subscript;

  for (i=0; i<=_a_HASHMASK; i++)
    newarray->slot[i] = NULL;

  for (i=0; i<array->nodeno; i++)
  {
    node = _awka_hshfindint( newarray, i+array->base, a_ARR_CREATE, FALSE );
    awka_varcpy(node->var, array->slot[i]->var);

    if (array->slot[i]->key != _a_SPLT_BASESTR)
      awka_killvar(array->slot[i]->var);

    free( array->slot[i] );
  }

  if (array->slot)
    free(array->slot);

  if (array->splitstr)
    free(array->splitstr);

  free(array);

  return newarray;
}

/*
 * _a_HshDestroyTreeNode
 * passed the address of a node, this will recursively
 * free its children then itself.  Calling this with the
 * slot node of a array will free the entire array.
 */
void
_awka_hshdestroyarray( _a_HSHarray *array )
{
  a_HSHNode *node, *nextnode;
  int i;

  for (i=0; i<=array->hashmask; i++)
  {
    node = array->slot[i];

    while (node)
    {
      if (!node->shadow)
      {
        awka_killvar(node->var);

        if (node->var)
          free(node->var);
      }
      nextnode = node->next;

      if (node->key)
        free(node->key);

      free(node);
      node = nextnode;
    }
  }

  if (array->slot)
    free(array->slot);

  if (array->subscript)
    free(array->subscript);

  array->hashmask = array->nodeno = array->nodeallc = 0;
  array->slot = NULL;
  array->type = a_ARR_TYPE_NULL;
}


#define MERGE_STRING \
      p = awka_gets(var); \
      thislen = var->slen; \
      len += thislen + slen; \
      if (len >= s->alloc) \
      { \
        s->alloc += len + ((used-i-1) * STR_GROWTH) + 1; \
        s->alloc = realloc( &s->str, s->alloc); \
        op = (s->str + (oldlen > 0 ? oldlen : 1)) - 1; \
      } \
      if (i) { \
        if (slen == 1) \
          *op++ = *subsep; \
        else  \
        { \
          memcpy(op, subsep, slen); \
          op += slen; \
        } \
      } \
      if (thislen == 1) \
        *op++ = *p; \
      else \
      { \
        memcpy(op, p, thislen); \
        op += thislen; \
      } \
      oldlen = len
/*
 * _awka_arraymergesubscripts
 * Given multiple array subscripts, this merges them into
 * a single char * inserting SUBSEP between each.
 */
char *
_awka_arraymergesubscripts( _a_Subscript *s, a_VARARG *va, int *thelen )
{
  register int i = 0, len = 0, oldlen = 0, slen = 0, thislen, used = va->used;
  register char *p, *op = s->str, *subsep;
  register char is_dbl = FALSE;
  a_VAR *var = va->var[0];

  if (used > s->dalloc)
  {
    if (!s->dalloc)
    {
      s->dalloc = va->used + 3;
      malloc( &s->delem, s->dalloc * sizeof(double));
      malloc( &s->pelem, s->dalloc * sizeof(char *));
      malloc( &s->lelem, s->dalloc * sizeof(int));
      malloc( &s->dset, s->dalloc);
      s->elem = 0;
    }
    else
    {
      s->dalloc = va->used + 3;
      realloc( &s->delem, s->dalloc * sizeof(double));
      realloc( &s->pelem, s->dalloc * sizeof(char *));
      realloc( &s->lelem, s->dalloc * sizeof(int));
      realloc( &s->dset, s->dalloc);
    }
  }

  if (!s->str)
  {
    s->alloc = malloc( &s->str, STR_GROWTH);
    op = s->str;
  }

  subsep = awka_gets1(a_bivar[a_SUBSEP]);
  slen = a_bivar[a_SUBSEP]->slen;

  is_dbl = (var->type == a_VARDBL ||
           (var->type == a_VARUNK && var->type2 == a_DBLSET));

  if (s->str && is_dbl == TRUE)
  {
    for (; i<used; var = va->var[++i])
    {
      if (s->dset[i] == FALSE || i >= s->elem)
        break;

      is_dbl = (var->type == a_VARDBL ||
               (var->type == a_VARUNK && var->type2 == a_DBLSET));

      if (!is_dbl || s->delem[i] != var->dval)
        break;

      op = s->pelem[i];
      oldlen = len = s->lelem[i];
    }

    if (i == used)
    {
      s->elem = i;
      *thelen = len - 1;

      return s->str;
    }
  }

  if (is_dbl == TRUE)
  {
    for (; i<used; var = va->var[++i])
    {
      if (!(var->type == a_VARDBL ||
           (var->type == a_VARUNK && var->type2 == a_DBLSET)))
        break;

      MERGE_STRING;
      s->pelem[i] = op;
      s->lelem[i] = len;
      s->delem[i] = var->dval;
      s->dset[i] = TRUE;
    }

    if (i == used)
    {
      s->elem = i;
      *op = '\0';
      *thelen = len - 1;

      return s->str;
    }
  }

  s->elem = i;

  for (; i<used; var = va->var[++i])
  {
    MERGE_STRING;
  }

  if (op)
    *op = '\0';

  *thelen = len - 1;

  return s->str;
}


/*
 * _awka_arrayinitargv
 * This initialises the ARGV builtin-var with contents
 * of *argv[].  Called by a_Init().
 */
void
_awka_arrayinitargv( char **ptr, int argc, char *argv[] )
{
  int len, i;
  _a_HSHarray *array;
  register a_HSHNode *a_s = NULL;

  malloc( &a_bivar[a_ARGV]->ptr, sizeof(_a_HSHarray) );
  array = (_a_HSHarray *) a_bivar[a_ARGV]->ptr;
  array->type = a_ARR_TYPE_SPLIT;
  array->nodeno = array->nodeallc = argc;
  malloc( &array->slot, argc * sizeof(a_HSHNode *));
  array->splitstr = NULL;
  array->splitallc = 0;

  for (i=0; i<argc; i++)
  {
    malloc( &array->slot[i], sizeof(a_HSHNode));
    a_s = array->slot[i];
    a_s->next = NULL;
    a_s->hval = 0;

    malloc( &a_s->var, sizeof(a_VAR));
    a_s->var->slen = a_s->var->allc = len = strlen(argv[i]);
    malloc( &a_s->var->ptr, len+1);
    memcpy(a_s->var->ptr, argv[i], len+1);
    a_s->var->type = a_VARUNK;

    if (_awka_isnumber(argv[i]) == TRUE)
    {
      a_s->var->type2 = a_DBLSET;
      a_s->var->dval = strtod(argv[i], NULL);
    }
    else
      a_s->var->type2 = -1;

    a_s->key = _a_SPLT_LOCALSTR;
    a_s->type = _a_ARR_INT;
  }
}

/*
 * _awka_arrayinitenviron
 * This initialises the ENVIRON builtin-var.
 * Called by a_Init().
 */
void
_awka_arrayinitenviron( char **ptr, int env_used )
{
  extern char **environ;
  register char *s, **p = environ, *q;
  char *tmpstr;
  int alloc;
  a_VAR *tmp = NULL, *ret;

  if (!env_used)
    return;

  awka_varinit(tmp);
  alloc = malloc( &tmpstr, 30 );
  awka_arraycreate( a_bivar[a_ENVIRON], a_ARR_TYPE_HSH );

  while ((q = *p))
  {
    if ((s = strchr(q, '=')))
    {
      if (s - q >= alloc)
        alloc = realloc( &tmpstr, (s-q)+1 );

      memcpy(tmpstr, q, (s - q));
      tmpstr[s-q] = '\0';
      awka_strcpy(tmp, tmpstr);
      ret = awka_arraysearch1( a_bivar[a_ENVIRON], tmp, a_ARR_CREATE, 0);
      awka_strcpy(ret, s+1);
      ret->type = a_VARUNK;

      if (_awka_isnumber(ret->ptr) == TRUE)
      {
        ret->type2 = a_DBLSET;
        ret->dval = strtod(ret->ptr, NULL);
      }
      else
        ret->type2 = -1;
    }

    p++;
  }

  free(tmpstr);
  awka_killvar(tmp);

  if (tmp)
    free(tmp);
}

_a_Subscript *
_awka_createsubscript()
{
  _a_Subscript *subscript;

  malloc( &subscript, sizeof(_a_Subscript) );
  subscript->str = NULL;
  subscript->delem = NULL;
  subscript->pelem = NULL;
  subscript->lelem = NULL;
  subscript->dset = NULL;
  subscript->alloc = subscript->dalloc = subscript->elem = 0;

  return subscript;
}

/* PUBLIC INTERFACE FUNCTIONS BELOW */

/*
 * awka_arraycreate
 * creates a new, null array and makes var point to it.
 */
void
awka_arraycreate( a_VAR *var, char type )
{
  register _a_HSHarray *array;

  if (var->ptr)
    free(var->ptr);

  var->type = a_VARARR;
  malloc( &var->ptr, sizeof(_a_HSHarray) );
  array = (_a_HSHarray *) var->ptr;

  array->subscript = _awka_createsubscript();
  array->last = NULL;

  if (type == a_ARR_TYPE_HSH)
  {
    array->hashmask = _a_HASHMASK;
    malloc( &array->slot, (_a_HASHMASK+1) * sizeof(a_HSHNode *) );
    array->type = a_ARR_TYPE_HSH;
    array->splitstr = NULL;
    array->splitallc = 0;
    memset(array->slot, 0, (_a_HASHMASK+1) * sizeof(a_HSHNode *) );
    array->nodeno = array->base = array->nodeallc = 0;
  }
  else
  {
    array->hashmask = 0;
    /* array->slot = NULL; */
    array->type = a_ARR_TYPE_SPLIT;
    array->splitstr = NULL;
    array->splitallc = 0;
    array->nodeno = array->base = 0;
    array->nodeallc = 0;
    malloc( &array->slot, GROWSZ * sizeof(a_HSHNode *) );
  }

  array->flag = 0;
}

/*
 * awka_arrayclear
 * deletes memory held by a array and sets it to empty status.
 * awk - 'delete(MyArr)'
 */
void
awka_arrayclear( a_VAR *var )
{
  _a_HSHarray *array;
  register int i;
  a_HSHNode *node, *nextnode;

  if (var->type == a_VARNUL || !var->ptr || (var->type == a_VARSTR && var->ptr[0] == '\0'))
    awka_arraycreate( var, a_ARR_TYPE_HSH );

  if (var->type != a_VARARR)
    awka_error("runtime error: Scalar used as array in call to ArrayClear\n");

  array = (_a_HSHarray *) var->ptr;

  if (array->type == a_ARR_TYPE_NULL)
    return;

  if (array->type == a_ARR_TYPE_SPLIT)
  {
    for (i=0; i<array->nodeallc; i++)
    {
      if (array->slot[i])
      {
        if (array->slot[i]->key == _a_SPLT_LOCALSTR)
          awka_killvar(array->slot[i]->var);

        free(array->slot[i]->var);
        free(array->slot[i]);
      }
    }

    if (array->splitstr)
      free(array->splitstr);

    if (array->slot)
      free(array->slot);
  }
  else
  {
    for (i=0; i<=array->hashmask; i++)
    {
      node = array->slot[i];

      while (node)
      {
        if (node->shadow == FALSE)
        {
          awka_killvar(node->var);
          free(node->var);
        }

        if (node->key)
          free(node->key);

        nextnode = node->next;
        free(node);
        node = nextnode;
      }
    }

    if (array->slot)
      free(array->slot);
  }

  if (array->subscript)
  {
    if (array->subscript->str)   free(array->subscript->str);
    if (array->subscript->delem) free(array->subscript->delem);
    if (array->subscript->pelem) free(array->subscript->pelem);
    if (array->subscript->lelem) free(array->subscript->lelem);
    if (array->subscript->dset)  free(array->subscript->dset);
    free(array->subscript);
  }

  array->hashmask = array->nodeno = array->nodeallc = array->splitallc = 0;
  array->slot = NULL;
  array->subscript = NULL;
  array->type = a_ARR_TYPE_NULL;
  array->splitstr = NULL;
}

static INLINE a_VAR *
_awka_arraynullvar()
{
  a_VAR *pv;

  _awka_tmpvar(pv);

  if (pv->ptr)
    awka_killvar(pv);

  pv->slen = (unsigned) -1;
  pv->allc = 0;
  pv->type = a_VARDBL;
  pv->dval = 0.0;
  pv->temp = pv->type2 = 0;

  return pv;
}

static INLINE a_VAR *
_awka_arrayfoundvar()
{
  a_VAR *pv;

  _awka_tmpvar(pv);

  if (pv->ptr)
    awka_killvar(pv);

  pv->slen = (unsigned) 0;
  pv->allc = 0;
  pv->type = a_VARDBL;
  pv->dval = 1.0;
  pv->temp = pv->type2 = 0;

  return pv;
}

void
_awka_growarray(_a_HSHarray *array, int i)
{
  int j;
  a_HSHNode *node;

  realloc( &array->slot, (i+GROWSZ) * sizeof(a_HSHNode *));

  for (j=i+1; j<(i+GROWSZ); j++)
    array->slot[j] = NULL;

  for (j=array->nodeallc; j<=i; j++)
  {
    malloc( &node, sizeof(a_HSHNode) );
    array->slot[j] = node;
    malloc( &node->var, sizeof(a_VAR));
    _awka_splitinitnode(node);
    node->key = _a_SPLT_LOCALSTR;
  }

  array->nodeallc = i + GROWSZ;
}

/***************** array searching *************************/

/*
 * _awka_arraysearchsplit
 * performs a search for a key on a 'split' array.
 */
a_VAR *
_awka_arraysearchsplit(_a_HSHarray *array, int i, char create, int set)
{
  char *x;
  register int j;
  register a_HSHNode *a_s = NULL;

  /* split array - is v->type a double? */
  if (i >= array->nodeno)
  {
    if (create == a_ARR_QUERY)
      return _awka_arraynullvar();
    if (i >= array->nodeallc)
      _awka_growarray(array, i);
    else
    {
      for (j=array->nodeno; j<=i; j++)
      {
        a_s = array->slot[j];

        if (!a_s)
        {
          malloc( &array->slot[j], sizeof(a_HSHNode));
          a_s = array->slot[j];
          malloc( &a_s->var, sizeof(a_VAR));
        }
        else
        {
          if (a_s->key == _a_SPLT_LOCALSTR &&
              a_s->var->ptr)
            awka_killvar(a_s->var);
        }

        _awka_splitinitnode(a_s);
        a_s->key = _a_SPLT_LOCALSTR;
      }
    }
    array->nodeno = i+1;
  }

  a_s = array->slot[i];

  if (set == 0 ||
     (a_s->key == _a_SPLT_LOCALSTR &&
      a_s->var->type2 == a_DBLSET))
    return a_s->var;

  if (a_s->key == _a_SPLT_BASESTR)
  {
    if (a_s->var->type == a_VARUNK ||
        a_s->var->type == a_VARSTR)
    {
      a_s->var->allc = malloc( &x, a_s->var->slen + 1);
      strcpy(x, a_s->var->ptr);
      a_s->var->ptr = x;
    }
    else
      a_s->var->ptr = NULL;

    a_s->key = _a_SPLT_LOCALSTR;
  }

  return a_s->var;
}

/*
 * For '1-based' split arrays, this is called if
 * the array is searched using element zero.  It
 * adjusts the true base of the array to zero by
 * shifting all elements up by one.
 */
void
_awka_lowerbase( _a_HSHarray *array )
{
  register int i;

  if (array->nodeallc == array->nodeno)
  {
    array->nodeallc += 10;
    realloc(&array->slot, array->nodeallc * sizeof(a_HSHNode *));
    for (i=array->nodeno; i<array->nodeallc; i++)
      array->slot[i] = NULL;
  }

  for (i=array->nodeno; i>0; i--)
    array->slot[i] = array->slot[i-1];

  malloc( &array->slot[0], sizeof(a_HSHNode));
  malloc( &array->slot[0]->var, sizeof(a_VAR));
  _awka_splitinitnode(array->slot[0]);
  array->slot[0]->key = _a_SPLT_LOCALSTR;

  array->nodeno++;
  array->base = 0;
}

/*
 * awka_arraysearch1
 * Interface for searching, inserting and deleting
 * elements in an array variable.
 */
a_VAR *
awka_arraysearch1( a_VAR *v, a_VAR *element, char create, int set )
{
  int i = -1, j;
  register char *ptr = NULL, is_an_int = FALSE;
  unsigned int hval;
  a_HSHNode *node = NULL;
  _a_HSHarray *array;
  a_VAR *pv;

  /* check our arguments */
  if (v->type != a_VARARR && v->type != a_VARNUL)
    awka_error("runtime error: Scalar used as array in call to ArraySearch\n");

  if (!v->ptr)
    awka_arraycreate( v, a_ARR_TYPE_HSH );

  array = (_a_HSHarray *) v->ptr;
  if (array->slot == NULL)
  {
    if (create != a_ARR_CREATE) return _awka_arraynullvar();
    awka_arraycreate( v, a_ARR_TYPE_HSH );
    array = (_a_HSHarray *) v->ptr;
  }

  switch (array->type)
  {
    case a_ARR_TYPE_SPLIT:
      if (element->type == a_VARDBL ||
         (element->type == a_VARUNK && element->type2 == a_DBLSET))
      {
        i = (int) element->dval;
        is_an_int = ((double) i == element->dval);
      }
      else
      {
        ptr = awka_gets1(element);
        if ((is_an_int = _awka_isanint(ptr)) == TRUE)
          i = atoi(ptr);
      }

      if (create != a_ARR_DELETE && is_an_int == TRUE && i >= 0)
      {
        if (i == 0 && array->base)
          _awka_lowerbase(array);
        j = i - array->base;  /* this makes all arrays zero based */
        if (j >= 0 && j < array->nodeno + 132)
        {
          pv = _awka_arraysearchsplit(array, j, create, set);
          if (pv)
          {
            if (pv->slen != -1 && create == a_ARR_QUERY)
              return _awka_arrayfoundvar();
            return pv;
          }
          if (create == a_ARR_QUERY) break;
        }
      }
      array = _awka_split2hsh(array);
      v->ptr = (char *) array;

    case a_ARR_TYPE_HSH:
      if (element->type == a_VARDBL ||
         (element->type == a_VARUNK && element->type2 == a_DBLSET))
      {
        if (is_an_int == FALSE)
        {
          i = (int) element->dval;
          is_an_int = ((double) i == element->dval);
        }

        if (is_an_int == TRUE)
        {
          if (!(array->flag & _a_ARR_INT))
          {
            if (array->flag & _a_ARR_STR)
              _awka_hashtoint(array);
            array->flag |= _a_ARR_INT;
          }
          node = _awka_hshfindint(array, i, create, FALSE);
          break;
        }
      }

      if (!ptr)
        ptr = awka_gets(element);
      if (!(array->flag & _a_ARR_STR))
      {
        if (array->flag & _a_ARR_INT)
          _awka_hashtostr(array);
        array->flag |= _a_ARR_STR;
      }
      hval = _awka_hashstr(ptr, element->slen);
      node = _awka_hshfindstr(array, ptr, element->slen, hval, create, FALSE );
  } /* switch */

  if (node)
  {
    switch (create)
    {
      case a_ARR_CREATE:
        if (array->type == a_ARR_TYPE_HSH)
        {
          if (array->flag & _a_ARR_INT && array->flag & _a_ARR_STR)
            i = (array->nodeno / 2) / array->hashmask;
          else
            i = array->nodeno / array->hashmask;
          if (i > _a_HSH_MAXDEPTH)
            _awka_hshdouble(array);
        }

	/* set unknown for typeof() */
	if (node->var->type == a_VARNUL)
          node->var->type = a_VARUNK;

        return node->var;

      case a_ARR_QUERY:
        return _awka_arrayfoundvar();

      case a_ARR_DELETE:
        /* if array is now empty destroy it
        if (array->nodeno == 0)
          _awka_hshdestroyarray( array ); */
        return NULL;
    }
  }

  return _awka_arraynullvar();
}

/*
 * awka_arraysearch
 * Interface for searching, inserting and deleting
 * multiple-dimension elements in an array variable.
 */
a_VAR *
awka_arraysearch( a_VAR *v, a_VARARG *va, char create )
{
  int i, j;
  unsigned int hval;
  a_HSHNode *node = NULL;
  _a_HSHarray *array;
  char *s;

  /* check our arguments */
  if (v->type != a_VARARR && v->type != a_VARNUL)
    awka_error("runtime error: Scalar used as array in call to ArraySearch\n");

  if (!v->ptr)
    awka_arraycreate( v, a_ARR_TYPE_HSH );

  array = (_a_HSHarray *) v->ptr;
  if (array->slot == NULL)
  {
    if (create != a_ARR_CREATE) return _awka_arraynullvar();
    awka_arraycreate( v, a_ARR_TYPE_HSH );
    array = (_a_HSHarray *) v->ptr;
  }

  if (array->type == a_ARR_TYPE_SPLIT)
  {
    /* convert array to hash so we can do a hash search */
    array = _awka_split2hsh(array);
    v->ptr = (char *) array;
  }

  if (!(array->flag & _a_ARR_STR))
  {
    if (array->flag & _a_ARR_INT)
      _awka_hashtostr(array);
    array->flag |= _a_ARR_STR;
  }
  if (!array->subscript)
    array->subscript = _awka_createsubscript();
  s = _awka_arraymergesubscripts( array->subscript, va, &j );
  hval = _awka_hashstr(s, j);
  node = _awka_hshfindstr(array, s, j, hval, create, FALSE );

  if (node)
  {
    switch (create)
    {
      case a_ARR_CREATE:
        if (array->type == a_ARR_TYPE_HSH)
        {
          if (array->flag & _a_ARR_INT && array->flag & _a_ARR_STR)
            i = (array->nodeno / 2) / array->hashmask;
          else
            i = array->nodeno / array->hashmask;
          if (i > _a_HSH_MAXDEPTH)
            _awka_hshdouble(array);
        }
        return node->var;

      case a_ARR_QUERY:
        return _awka_arrayfoundvar();

      case a_ARR_DELETE:
        /* if array is now empty destroy it
        if (array->nodeno == 0)
          _awka_hshdestroyarray( array ); */
        return NULL;
    }
  }

  return _awka_arraynullvar();
}

#define ADD_SPLIT_NODE(i) { \
    if (!array->slot[(i)]) \
    {\
      malloc( &node, sizeof(a_HSHNode)); \
      array->slot[(i)] = node; \
      malloc( &node->var, sizeof(a_VAR)); \
      _awka_splitinitnode(node); \
      node->key = _a_SPLT_BASESTR; \
      array->slot[(i)] = node; \
    } \
    else \
      node = array->slot[(i)]; \
}

/*********** FS splitting utility functions ******************/
/*
 * Split by identifying delimeters that match an regexp
 */
int
_awka_splitre( _a_HSHarray *array, a_VAR *fs, int max, int oldnodeno )
{
  awka_regexp *r = (awka_regexp *) fs->ptr;
  a_HSHNode *node;
  register int i = 0, alloc = SLOT_GROWTH, len, j;
  char *start, *end, *s, *earlystart, termchar;
  static regmatch_t pmatch;

  s = earlystart = array->splitstr;
  if (!array->nodeallc || !array->slot)
  {
    malloc( &array->slot, alloc * sizeof(a_HSHNode *) );
    memset( array->slot, 0, alloc * sizeof(a_HSHNode *) );
    //for (i=0; i<alloc; i++) 
    //  array->slot[i] = NULL;
    i = 0;
  }
  else
    alloc = array->nodeallc;

  if (array->splitstr[0] == '\0')
  {
    array->nodeno = 1;
    if (!array->slot[0])
    {
      malloc( &node, sizeof(a_HSHNode));
      array->slot[0] = node;
      malloc( &node->var, sizeof(a_VAR));
      node->var->type = a_VARNUL;
      /* initialise below */
    }
    else
    {
      node = array->slot[0];

      if (node->var->type != a_VARUNK && node->var->type != a_VARNUL)
        awka_killvar(node->var);
    }

    _awka_splitinitnode(node);
    node->key = _a_SPLT_BASESTR;
    node->var->type = a_VARUNK;
    node->var->ptr = nullstr;
    node->type = _a_ARR_INT;
    if (array->nodeallc == 0) array->nodeallc = alloc;

    return 1;
  }

  len = strlen(array->splitstr);

  while (i < max && !awka_regexec(r, s, 1, &pmatch, REG_NEEDSTART))
  {
    start = s + pmatch.rm_so;
    end   = s + pmatch.rm_eo;

    ADD_SPLIT_NODE(i);
    i++;

    node->var->type = a_VARUNK;
    node->type = _a_ARR_INT;
    node->var->type2 = 0;

    if (node->key == _a_SPLT_BASESTR)
      node->var->ptr = s;
    else
    {
      if (!node->var->ptr)
        node->var->allc = malloc( &node->var->ptr, (start - s) + 1 );
      else if (node->var->allc < (start - s) + 1)
        node->var->allc = realloc( &node->var->ptr, (start - s) + 1 );

      memcpy(node->var->ptr, s, start - s);
    }

    node->var->slen = (start - s);
    termchar = *(node->var->ptr + (start - s));
    *(node->var->ptr + (start - s)) = '\0';

    if (i >= alloc)
    {
      j = alloc;
      alloc += SLOT_GROWTH;
      realloc( &array->slot, alloc * sizeof(a_HSHNode *));
      for (; j<alloc; j++) array->slot[j] = NULL;
    }

    s = earlystart = end;
    if (!*s)
      break;

  } /* while match */

  if ((i < max && *earlystart) || termchar != '\0')
  {
    /* got a trailing field */
    s = array->splitstr + len;

    ADD_SPLIT_NODE(i);
    i++;

    node->var->type = a_VARUNK;
    node->type = _a_ARR_INT;
    node->var->type2 = 0;

    if (node->key == _a_SPLT_BASESTR)
      node->var->ptr = earlystart;
    else
    {
      if (!node->var->ptr)
        node->var->allc = malloc( &node->var->ptr, (s - earlystart) + 1 );
      else if (node->var->allc <= s - earlystart)
        node->var->allc = realloc( &node->var->ptr, (s - earlystart) + 1 );
      memcpy(node->var->ptr, earlystart, s - earlystart);
    }

    node->var->slen = (s - earlystart);
    *(node->var->ptr + (s - earlystart)) = '\0';

    if (i >= alloc)
    {
      j = alloc;
      alloc = i + 1;
      realloc( &array->slot, alloc * sizeof(a_HSHNode *));
      for (; j<alloc; j++) array->slot[j] = NULL;
    }
  }

  for (j=i; j<oldnodeno; j++)
  {
    if (array->slot[j]->key == _a_SPLT_BASESTR)
    {
      check_emptyvar();
      memcpy( array->slot[j]->var, emptyvar, sizeof(a_VAR) );
      array->slot[j]->var->ptr = nullstr;
    }
    else
    {
      awka_killvar(array->slot[j]->var);
      array->slot[j]->key = _a_SPLT_BASESTR;
    }
  }

  array->nodeno = i;
  array->nodeallc = (array->nodeallc < array->nodeno ? array->nodeno : array->nodeallc);

  return i;
}

#define FPATSZ  256
/*
 * Split by identifying content that matches an regexp
 * fpat is an _RE_MATCH compiled a_VARREG
 */
int
_awka_splitcontentre( _a_HSHarray *array, a_VAR *fpat, int max, int oldnodeno, char **delims )
{
  awka_regexp *r;
  a_HSHNode *node;
  register int i = 0, j;
  int alloc = SLOT_GROWTH, len;
  char *start, *end, *s, *earlystart, mlen;
  static regmatch_t pmatch;

  memset(&pmatch, 0, sizeof(pmatch));

  s = earlystart = end = array->splitstr;

  if (!array->nodeallc || !array->slot)
  {
    malloc( &array->slot, alloc * sizeof(a_HSHNode *) );
    memset(array->slot, 0, alloc * sizeof(a_HSHNode *) );
    //for (i=0; i<alloc; i++) 
    //  array->slot[i] = NULL;
    i = 0;
  }
  else
    alloc = array->nodeallc;

  if (array->splitstr[0] == '\0')
  {
    array->nodeno = 1;
    if (!array->slot[0])
    {
      malloc( &node, sizeof(a_HSHNode));
      array->slot[0] = node;
      malloc( &node->var, sizeof(a_VAR));
      node->var->type = a_VARNUL;
      /* initialised below */
    }
    else
    {
      node = array->slot[0];

      if (node->var->type != a_VARUNK && node->var->type != a_VARNUL)
        awka_killvar(node->var);
    }

    _awka_splitinitnode(node);
    node->key = _a_SPLT_BASESTR;
    node->var->type = a_VARUNK;
    node->var->ptr = nullstr;
    node->type = _a_ARR_INT;

    if (array->nodeallc == 0)
      array->nodeallc = alloc;

    return 1;
  }

  /* setup the regexp */
  if (fpat->type != a_VARREG)
    _awka_getreval(fpat, __FILE__, __LINE__, _RE_MATCH);

  r = (awka_regexp *) fpat->ptr;

  len = strlen(array->splitstr);

  while (i < max && awka_regexec(r, s, 1, &pmatch, 0) == 0)
  {
    if (!*end)
      break;

    if (pmatch.rm_so == pmatch.rm_eo)
    { /* skip a delimeter that is not a recognized field contents pattern */
      s = earlystart = ++end;
      continue;
    }

    start = s + pmatch.rm_so;
    end   = s + pmatch.rm_eo;
    mlen = end - start;

    ADD_SPLIT_NODE(i);
    i++;

    node->var->type = a_VARUNK;
    node->type = _a_ARR_INT;
    node->var->type2 = 0;
    if (!node->var->ptr)
      node->var->allc = malloc( &node->var->ptr, mlen + 1 );
    else if (node->var->allc < mlen + 1)
      node->var->allc = realloc( &node->var->ptr, mlen + 1 );
    memcpy(node->var->ptr, start, mlen);
    node->var->slen = mlen;
    *(node->var->ptr + mlen) = '\0';

    if (i >= alloc)
    {
      /* extend slot if have just filled the last slot */
      j = alloc;
      alloc += SLOT_GROWTH;
      realloc( &array->slot, alloc * sizeof(a_HSHNode *));
      for (; j<alloc; j++) 
        array->slot[j] = NULL;
    }

    s = earlystart = end;
    if (!*end) 
      break;
  } /* while match */

  if (i < max && *earlystart)
  {
    /* got a trailing field */
    s = array->splitstr + len;

    ADD_SPLIT_NODE(i);
    i++;

    node->var->type = a_VARUNK;
    node->type = _a_ARR_INT;
    node->var->type2 = 0;

    if (node->key == _a_SPLT_BASESTR)
      node->var->ptr = earlystart;
    else
    {
      if (!node->var->ptr)
        node->var->allc = malloc( &node->var->ptr, (s - earlystart) + 1 );
      else if (node->var->allc <= s - earlystart)
        node->var->allc = realloc( &node->var->ptr, (s - earlystart) + 1 );
      memcpy(node->var->ptr, earlystart, s - earlystart);
    }

    node->var->slen = (s - earlystart);
    *(node->var->ptr + (s - earlystart)) = '\0';

    if (i >= alloc)
    {
      j = alloc;
      alloc = i + 1;
      realloc( &array->slot, alloc * sizeof(a_HSHNode *));
      for (; j<alloc; j++)
        array->slot[j] = NULL;
    }
  }

  for (j=i; j<oldnodeno; j++)
  {
    if (array->slot[j]->key == _a_SPLT_BASESTR)
    {
      check_emptyvar();
      memcpy( array->slot[j]->var, emptyvar, sizeof(a_VAR) );
      array->slot[j]->var->ptr = nullstr;
    }
    else
    {
      awka_killvar(array->slot[j]->var);
      array->slot[j]->key = _a_SPLT_BASESTR;
    }
  }

  array->nodeno = i;
  array->nodeallc = (array->nodeallc < array->nodeno ? array->nodeno : array->nodeallc);

  return i;
}

/*
 * Split using a null FS (split input into each individual char)
 */
int
_awka_split_null( _a_HSHarray *array, int max, int oldnodeno )
{
  register int i = 0, alloc = 10, j;
  a_HSHNode *node;

  /* NULL FS - split by character */
  alloc = strlen(array->splitstr);
  alloc = (alloc < max ? alloc : max);
  for (j=alloc; j<oldnodeno; j++)
  {
    if (array->slot[j]->key == _a_SPLT_BASESTR)
    {
      check_emptyvar();
      memcpy( array->slot[j]->var, emptyvar, sizeof(a_VAR) );
    }
    else
    {
      awka_killvar(array->slot[j]->var);
      array->slot[j]->key = _a_SPLT_BASESTR;
    }
  }

  array->nodeno = alloc;
  if (array->slot)
  {
    if (array->nodeallc < alloc)
    {
      realloc( &array->slot, alloc * sizeof(a_HSHNode *) );
      for (i=array->nodeallc; i<alloc; i++)
        array->slot[i] = NULL;
      array->nodeallc = alloc;
    }
  }
  else
  {
    malloc( &array->slot, array->nodeno * sizeof(a_HSHNode *) );
    memset(array->slot, 0, array->nodeno * sizeof(a_HSHNode *) );
    array->nodeallc = alloc;
  }

  for (i=0; i<array->nodeno; i++)
  {
    if (!array->slot[i])
    {
      malloc( &node, sizeof(a_HSHNode) );
      malloc( &node->var, sizeof(a_VAR));
      _awka_splitinitnode(node);
      node->key = _a_SPLT_LOCALSTR;
      array->slot[i] = node;
    }
    else
    {
      node = array->slot[i];

      if (node->var->type != a_VARUNK && node->var->type != a_VARNUL)
        awka_killvar(node->var);
    }

    node->var->type = a_VARUNK;
    node->type = _a_ARR_INT;
    node->var->type2 = 0;

    if (node->var->ptr == NULL)
      node->var->allc = malloc(&node->var->ptr, 2);

    node->var->ptr[0] = array->splitstr[i];
    node->var->ptr[1] = '\0';
    node->var->type2 = 0;

    if (isdigit(array->splitstr[i]))
    {
      node->var->type2 = a_DBLSET;
      node->var->dval = node->var->ptr[0] - '0';
    }

    node->var->slen = 1;
  }

  array->nodeallc = (array->nodeallc < array->nodeno ? array->nodeno : array->nodeallc);

  return array->nodeno;
}

#define SWALLOW_SPACE \
  while (_a_space[(unsigned char) *p]) p++

#define SWALLOW_WORD \
  while (*p && !_a_space[(unsigned char) *p]) p++

/*
 * Fields are separated by runs of whitespace. Leading and trailing whitespace are ignored.
 * This is the default FS setting
 */
int
_awka_split_space( _a_HSHarray *array, int max, int oldnodeno )
{
  register int i = 0, alloc = SLOT_GROWTH, j;
  register char *p, *q;
  a_HSHNode *node;

  /* whitespace separator */
  p = q = array->splitstr;
  if (!array->nodeallc)
  {
    malloc( &array->slot, alloc * sizeof(a_HSHNode *) );
    memset(array->slot, 0, alloc * sizeof(a_HSHNode *) );
    //for (i=0; i<alloc; i++)
    //  array->slot[i] = NULL;
    i = 0;
  }
  else
    alloc = array->nodeallc;

  do {
    /* walk up to next word */
    SWALLOW_SPACE;
    if (!*p) /* hit end of str */
      break;

    if (i >= alloc)
    {
      j = alloc;
      alloc += SLOT_GROWTH;
      realloc( &array->slot, alloc * sizeof(a_HSHNode *));
      for (; j<alloc; j++)
        array->slot[j] = NULL;
    }

    q = p;
    /* walk to space after end of word */
    SWALLOW_WORD;

    /* copy this to a node */
    if (!array->slot[i])
    {
      malloc( &node, sizeof(a_HSHNode) );
      malloc( &node->var, sizeof(a_VAR));
      _awka_splitinitnode(node);
      //node->var->ptr = NULL;
      node->key = _a_SPLT_BASESTR;
      array->slot[i++] = node;
    }
    else
    {
      node = array->slot[i++];

      if (node->var->type != a_VARUNK && node->var->type != a_VARNUL)
        awka_killvar(node->var);
    }

    node->var->type = a_VARUNK;
    node->type = _a_ARR_INT;
    node->var->type2 = 0;
    if (node->key == _a_SPLT_BASESTR)
    {
      node->var->ptr = q;
      node->var->slen = (p-q);
      if (!*p)
      {
        *(node->var->ptr + (p-q)) = '\0';
        break;
      }
      *(node->var->ptr + (p-q)) = '\0';
      q = ++p;
      continue;
    }

    if (node->var->ptr == NULL)
      node->var->allc = malloc( &node->var->ptr, (p - q) + 1 );
    else if (node->var->allc <= (p-q))
      node->var->allc = realloc( &node->var->ptr, (p - q) + 1 );
    memcpy(node->var->ptr, q, p - q);

    node->var->slen = (p - q);
    if (!*p)
    {
      *(node->var->ptr + (p-q)) = '\0';
      break;
    }
    *(node->var->ptr + (p-q)) = '\0';
    q = ++p;

  } while (i < max && *p);

  for (j=i; j<oldnodeno; j++)
  {
    array->slot[j]->var->slen = 0;
    if (array->slot[j]->key == _a_SPLT_BASESTR)
      array->slot[j]->var->ptr = nullstr;
    else
    {
      awka_killvar(array->slot[j]->var);
      array->slot[j]->key = _a_SPLT_BASESTR;
    }
    array->slot[j]->var->dval = 0;
    array->slot[j]->var->type2 = 0;
    array->slot[j]->var->type = a_VARNUL;
  }
  array->nodeno = i;
  array->nodeallc = (array->nodeallc < array->nodeno ? array->nodeno : array->nodeallc);
  return array->nodeno;
}

/*
 * splitting by a single char (that isn't a spce - which is handled uniquely)
 */
int
_awka_split_single_char( _a_HSHarray *array, char fs, int max, int oldnodeno )
{
  register int i = 0, alloc = 40, j;
  register char *p, *q;
  a_HSHNode *node;

  /* single character separator */
  p = q = array->splitstr;
  if (!array->nodeallc)
  {
    malloc( &array->slot, alloc * sizeof(a_HSHNode *) );
    for (j=0; j<alloc; j++)
      array->slot[j] = NULL;
  }
  else
    alloc = array->nodeallc;

  while (i < max && *p)
  {
    if (i == alloc)
    {
      j = alloc;
      alloc += 40;
      realloc( &array->slot, alloc * sizeof(a_HSHNode *));
      for (; j<alloc; j++) array->slot[j] = NULL;
    }

    /* walk to character after end of word */
    while (*p != fs && *p) p++;

    /* copy this to a node */
    ADD_SPLIT_NODE(i);
    i++;

    node->var->type = a_VARUNK;
    node->type = _a_ARR_INT;
    node->var->type2 = 0;
    if (node->key == _a_SPLT_BASESTR)
    {
      node->var->ptr = q;
      node->var->slen = (p - q);
      if (!*p)
      {
        *(node->var->ptr + (p-q)) = '\0';
        break;
      }
      *(node->var->ptr + (p-q)) = '\0';
      if (*(p+1) != '\0')
      {
        q = ++p;
        continue;
      }
    }
    else
    {
      if (node->var->ptr == NULL)
        node->var->allc = malloc( &node->var->ptr, (p - q) + 1 );
      else if (node->var->allc <= (p - q))
        node->var->allc = malloc( &node->var->ptr, (p - q) + 1 );
      memcpy(node->var->ptr, q, p - q);
      node->var->slen = (p - q);
      if (!*p)
      {
        *(node->var->ptr + (p-q)) = '\0';
        break;
      }
      *(node->var->ptr + (p-q)) = '\0';

      if (*(p+1) != '\0')
      {
        q = ++p;
        continue;
      }
    }

    /* got a trailing null field */
    ADD_SPLIT_NODE(i);
    i++;

    node->var->type = a_VARUNK;
    node->type = _a_ARR_INT;
    node->var->type2 = 0;
    if (node->key == _a_SPLT_BASESTR)
      node->var->ptr = p+1;
    else
    {
      if (node->var->ptr == NULL)
        node->var->allc = malloc( &node->var->ptr, 1);
      node->var->type2 = 0;
      node->var->ptr[0] = '\0';
      node->var->dval = 0;
    }
    node->var->slen = 0;
    if (i == alloc)
    {
      j = alloc;
      alloc += 40;
      realloc( &array->slot, alloc * sizeof(a_HSHNode *));
      for (; j<alloc; j++) array->slot[j] = NULL;
    }
    break;
  }

  for (j=i; j<oldnodeno; j++)
  {
    if (array->slot[j]->key == _a_SPLT_BASESTR)
    {
      check_emptyvar();
      memcpy( array->slot[j]->var, emptyvar, sizeof(a_VAR) );
      array->slot[j]->var->ptr = nullstr;
    }
    else
    {
      awka_killvar(array->slot[j]->var);
      array->slot[j]->key = _a_SPLT_BASESTR;
    }
  }

  array->nodeno = i;
  array->nodeallc = (array->nodeallc < array->nodeno ? array->nodeno : array->nodeallc);

  return array->nodeno;
}

/*********** FIELDWIDTHS splitting ************************/
/*
 * _awka_parse_width_format
 * splits a FIELDWIDTHS format string into an array of ints
 */
void
_awka_parse_width_format( char *format, int fw )
{
  register char *p1 = format, *p2;
  int i = 0, end = FALSE;
  int *loc, used, allc;

  if (fw)
  {
    loc = fw_loc;
    used = fw_used = 0;
    allc = fw_allc;
  }
  else
  {
    loc = sw_loc;
    used = sw_used = 0;
    allc = sw_allc;
  }

  if (!allc)
  {
    allc = STR_GROWTH;
    malloc( &loc, allc * sizeof(int) );
  }

  while (*p1)
  {
    /* walk up to first word */
    while (isspace(*p1)) p1++;
    if (!*p1) break;
    if (!isdigit(*p1)) break;
    p2 = p1;

    /* walk to end of word */
    while (isdigit(*p2)) p2++;
    if (*p2 && !isspace(*p2))
    {
      used = -1;
      break;
    }

    if (!*p2)
    {
      i = atoi(p1);
      end = TRUE;
    }
    else
    {
      *p2 = '\0';
      i = atoi(p1);
      *p2 = ' ';
    }

    if (i <= 0)
    {
      used = -1;
      break;
    }
    if (used+1 >= allc)
    {
      allc *= 2;
      realloc(&loc, allc * sizeof(int));
    }
    loc[used++] = i;

    if (end) break;
    p1 = p2+1;
  }

  if (fw)
  {
    fw_used = used;
    fw_loc = loc;
    fw_allc = allc;
  }
  else
  {
    sw_used = used;
    sw_loc = loc;
    sw_allc = allc;
  }
}

/*
 * _awka_arraysplitwidth
 * splits an array using the format in FIELDWIDTHS
 */
double
_awka_arraysplitwidth( char *str, a_VAR *v, int max )
{
  static char *format = NULL;
  static int format_allc = 0;
  int i, j, len = strlen(str), flen, curlen = 0, count = 0;
  _a_HSHarray *array;
  a_HSHNode *node;
  a_VAR *tmpv;

  awka_gets(a_bivar[a_FIELDWIDTHS]);
  if (!format)
  {
    format_allc = malloc(&format, a_bivar[a_FIELDWIDTHS]->slen+1);
    fw_allc = STR_GROWTH;
    malloc(&fw_loc, STR_GROWTH * sizeof(int));
    strcpy(format, a_bivar[a_FIELDWIDTHS]->ptr);
    _awka_parse_width_format(format, TRUE);

    if (!fw_used)
      return -1;
  }
  else if (strcmp(format, a_bivar[a_FIELDWIDTHS]->ptr))
  {
    if (a_bivar[a_FIELDWIDTHS]->slen >= format_allc)
      format_allc = realloc( &format, a_bivar[a_FIELDWIDTHS]->slen+1 );
    strcpy(format, a_bivar[a_FIELDWIDTHS]->ptr);
    _awka_parse_width_format(format, TRUE);

    if (!fw_used)
      return -1;
  }

  if (len && str[len-1] == '\n')
    str[--len] = '\0';

  array = (_a_HSHarray *) v->ptr;
  if (!array->nodeallc)
  {
    malloc( &array->slot, fw_used * sizeof(a_HSHNode *) );
    memset(array->slot, 0, fw_used * sizeof(a_HSHNode *) );
    //for (j=0; j<fw_used; j++)
    //  array->slot[j] = NULL;
    array->nodeallc = fw_used;
  }
  else if (array->nodeallc < fw_used)
  {
    realloc( &array->slot, fw_used * sizeof(a_HSHNode *) );
    for (j=array->nodeallc; j<fw_used; j++)
      array->slot[j] = NULL;
    array->nodeallc = fw_used;
  }

  for (i=0; i<fw_used; i++)
  {
    count++;
    if (i < fw_used)
    {
      flen = fw_loc[i];
      if (fw_loc[i] + curlen > len)
        flen = len - curlen;
    }
    else
      flen = len - curlen;

    if (!array->slot[i])
    {
      malloc( &node, sizeof(a_HSHNode) );
      malloc( &node->var, sizeof(a_VAR));
      _awka_splitinitnode(node);
      node->key = _a_SPLT_LOCALSTR;
      array->slot[i] = node;
    }
    else
      node = array->slot[i];

    if (node->var->type != a_VARUNK && node->var->type != a_VARNUL)
      awka_gets(node->var);

    node->var->type = a_VARUNK;
    node->type = _a_ARR_INT;
    node->var->type2 = 0;

    if (node->key == _a_SPLT_BASESTR || !node->var->ptr)
      node->var->allc = malloc( &node->var->ptr, flen+1 );
    else if (node->var->allc <= flen)
      node->var->allc = realloc( &node->var->ptr, flen+1 );

    memcpy(node->var->ptr, str + curlen, flen);
    *(node->var->ptr + flen) = '\0';
    node->var->slen = flen;
    curlen += flen;
  }

  for (j=i; j<array->nodeno; j++)
  {
    if (array->slot[j]->key == _a_SPLT_BASESTR)
    {
      check_emptyvar();
      memcpy( array->slot[j]->var, emptyvar, sizeof(a_VAR) );
      array->slot[j]->var->ptr = nullstr;
    }
    else
    {
      awka_gets(array->slot[j]->var);
      array->slot[j]->var->ptr[0] = '\0';
      array->slot[j]->var->slen = array->slot[j]->var->type2 = 0;
      array->slot[j]->var->dval = 0.0;
      array->slot[j]->var->type = a_VARNUL;
    }
  }

  array->nodeno = count;

  tmpv = awka_arraysearch1( a_bivar[a_PROCINFO], awka_tmp_str2var("FS"), a_ARR_CREATE, 0 );
  strcpy(tmpv->ptr, "FIELDWIDTHS");

  return array->nodeno;
}

/*
 * _awka_arraysplitpat
 * awk builtin 'split' function, here because it is essentially
 * an array function.
 * Given a string str, splits it into smaller strings
 * using a_bivar[a_FPAT] which is a pattern to determine the contents of each field, 
 * then builds v into a 'split' array to hold the results.
 */
double
awka_arraysplitpat( char *str, a_VAR *v, a_VAR *fpat, int max )
{
  register char *ptr;
  _a_HSHarray *array;
  int i, oldnodeno;
  double ret;
  a_VAR *tmpv;

  if (!fpat)
    awka_error("runtime error: No pattern passed to ArraySplitPat\n");

  if (fpat->type == a_VARARR)
    awka_error("runtime error: Array used as scalar in call to ArraySplitPat\n");

  array = (_a_HSHarray *) v->ptr;
  oldnodeno = array->nodeno;

  if (array->nodeno)
  {
    if (array->type == a_ARR_TYPE_HSH)
    {
      awka_arrayclear(v);
      oldnodeno = 0;
    }
    else
      array->nodeno = 0;
  }
  array->type = a_ARR_TYPE_SPLIT;

  i = strlen(str);

  if (!array->splitstr)
    array->splitallc = malloc( &array->splitstr, i+1);
  else if (array->splitallc < i+1)
    array->splitallc = realloc( &array->splitstr, i+1);

  memcpy(array->splitstr, str, i+1);

  array->type = a_ARR_TYPE_SPLIT;
  array->base = 1;

  tmpv = awka_getarrayval( a_bivar[a_PROCINFO], awka_tmp_str2var("FS"));
  strcpy(tmpv->ptr, "FPAT");

  /* split the string */
  return i ? _awka_splitcontentre( array, fpat, max, oldnodeno, NULL ) : 0.0;
}

/*
 * awka_arraysplitstr
 * awk builtin 'split' function, here because it is essentially
 * an array function.
 *
 * Given a string str, splits it into smaller strings
 * using fs, 
 * then builds a 'split' array v to hold the splits.
 */
double
awka_arraysplitstr( char *str, a_VAR *v, a_VAR *fs, int max, char main_split )
{
  register char *ptr;
  _a_HSHarray *array;
  int i, oldnodeno;
  double ret = -1;

  /* check arguments */
  if (v->type != a_VARARR && v->type != a_VARNUL && !(v->type == a_VARSTR && v->ptr[0] == '\0'))
    awka_error("runtime error: Scalar used as array in call to ArraySplitStr\n");

  /* Get array ready to populate */
  if (v->type == a_VARNUL || v->type == a_VARSTR ||
     (v->type == a_VARARR && v->ptr == NULL))
    awka_arraycreate( v, a_ARR_TYPE_SPLIT );

  if (!fs)
  {
    if (main_split) 
    {
      if (fs_or_fw == 1)
        ret = _awka_arraysplitwidth( str, v, max );
      else if (fs_or_fw == 2)
        ret = awka_arraysplitpat( str, v, awka_vardup(a_bivar[a_FPAT]), max );

      if (ret > -1)
        return ret;

      fs_or_fw = 0;
    }

    fs = a_bivar[a_FS];
  }

  if (fs->type == a_VARARR)
    awka_error("runtime error: Array used as scalar in call to ArraySplitStr\n");

  if (fs->type == a_VARNUL)
  {
    fs->allc = malloc( &fs->ptr, 1 );
    fs->ptr[0] = '\0';
    fs->slen = fs->type2 = 0;
    fs->type = a_VARSTR;
  }

  array = (_a_HSHarray *) v->ptr;
  oldnodeno = array->nodeno;

  if (array->nodeno)
  {
    if (array->type == a_ARR_TYPE_HSH)
    {
      awka_arrayclear(v);
      oldnodeno = 0;
    }
    else
      array->nodeno = 0;
  }
  array->type = a_ARR_TYPE_SPLIT;

  i = strlen(str);

  if (!array->splitstr)
    array->splitallc = malloc( &array->splitstr, i+1);
  else if (array->splitallc < i+1)
    array->splitallc = realloc( &array->splitstr, i+1);

  memcpy(array->splitstr, str, i+1);

  array->type = a_ARR_TYPE_SPLIT;
  array->base = 1;

  /* split the string */
  if (i && fs->type != a_VARREG)
  {
    ptr = awka_gets1(fs);
    if (fs->slen > 1)
      _awka_getreval(fs, __FILE__, __LINE__, _RE_SPLIT);
  }

  if (i)
  { 
    if (fs->type == a_VARREG)
      return _awka_splitre( array, fs, max, oldnodeno );

    switch (ptr[0])
    {
      case '\0':
        return _awka_split_null(array, max, oldnodeno);
      case ' ':
        return _awka_split_space(array, max, oldnodeno);
      default:
        return _awka_split_single_char( array, ptr[0], max, oldnodeno );
    }
  }

  return 0.0;
}

/********************** $0 splitting/assigning *******************/

a_VAR *
_awka_NF()
{
  if (_rebuildn == TRUE)
  {
    awka_setd(a_bivar[a_NF]) = awka_arraysplitstr(awka_gets1(a_bivar[a_DOL0]), a_bivar[a_DOLN], a_bivar[a_FS], _split_max, TRUE);
    _rebuildn = FALSE;
  }
  return a_bivar[a_NF];
}

/*
 * awka_dol0
 * this takes care of accessing & modifying awk $0 variable.
 * its here because awka_doln is here
 */
a_VAR *
_awka_dol0(int set)
{
  register int i, j, k, oldlen;
  register _a_HSHarray *array;
  register char *ptr, *op;

  a_HSHNode *node;
  register a_VAR *var, *ofs, *dol0;
  a_VAR *tmpv = NULL;
  static char *sformat = NULL;
  static int s_allc = 0;

  /* $0 accessed */
  dol0 = a_bivar[a_DOL0]; ofs = a_bivar[a_OFS];

  switch (set)
  {
    case a_DOL_SET:  /* 1 */
      if (_rebuild0)
        _rebuild0_now = TRUE;
      else
        _rebuildn = TRUE;
      _rebuild0 = FALSE;
      _awka_setdol0_len = _awka_setdoln = FALSE;
      dol0->type2 = 0;
      return dol0;

    case a_DOL_GET:  /* 0 */
    case -1:
      if (_rebuildn == TRUE && _split_req==1)
      {
	 if (!fs_or_fw)
	 {
	   /* use fieldwidths = savewidths to process savewidths */
	   char *tfw = awka_gets(a_bivar[a_FIELDWIDTHS]);
	   char *tsavew = awka_gets(a_bivar[a_SAVEWIDTHS]);

	   strcpy(a_bivar[a_FIELDWIDTHS]->ptr, tsavew);
	   strcpy(a_bivar[a_SAVEWIDTHS]->ptr, "");

           awka_setd(a_bivar[a_NF]) = awka_arraysplitstr(awka_gets1(dol0), a_bivar[a_DOLN], NULL, _split_max, TRUE);

	   strcpy(a_bivar[a_FIELDWIDTHS]->ptr, tfw);
	   strcpy(a_bivar[a_SAVEWIDTHS]->ptr, tsavew);
	 }
	 else
	 {
           awka_setd(a_bivar[a_NF]) = awka_arraysplitstr(awka_gets1(dol0), a_bivar[a_DOLN], NULL, _split_max, TRUE);
	 }
         _rebuildn = FALSE;
      }
      /* fall thru */
    default:
      array = (_a_HSHarray *) a_bivar[a_DOLN]->ptr;
      /* if ((_awka_setdol0_len == TRUE || _rebuild0 || _rebuild0_now) && set != -1) // last seen in 0.5.8 */
      if ((_rebuild0 || _rebuild0_now) && set != -1)
      {
        if (awka_getd1(a_bivar[a_NF]))  /* have to rebuild $0 from $n */
        {
          if (a_bivar[a_NF]->dval > array->nodeno)
          {
            /* Have to extend $n array to match NF */
            j = (int) a_bivar[a_NF]->dval;
            if (j > array->nodeallc)
            {
              if (!array->slot)
                malloc( &array->slot, j * sizeof(a_HSHNode *) );
              else
                realloc( &array->slot, j * sizeof(a_HSHNode *) );
            }

            for (i=array->nodeno; i<j; i++)
            {
              if (i >= array->nodeallc)
              {
                malloc( &node, sizeof(a_HSHNode));
                malloc( &node->var, sizeof(a_VAR));
                array->slot[i] = node;
                _awka_hshinitnode(node);
                node->var->type = a_VARUNK;
                node->key = _a_SPLT_BASESTR;
                node->var->ptr = nullstr;
              }
              else if (array->slot[i]->key == _a_SPLT_LOCALSTR)
              {
                if (!array->slot[i]->var->ptr)
                {
                  array->slot[i]->var->allc = malloc( &array->slot[i]->var->ptr, 1 );
                  array->slot[i]->var->ptr[0] = '\0';
                }
              }
              else
                array->slot[i]->var->ptr = nullstr;

              array->slot[i]->var->slen = array->slot[i]->var->type2 = 0;
              array->slot[i]->var->dval = 0.0;
            }

            array->nodeallc = (j > array->nodeallc ? j : array->nodeallc);
            array->nodeno = j;
          }

          /* rebuild $0 */
          awka_forcestr(dol0);
          awka_forcestr(ofs);
          var = array->slot[0]->var;
          ptr = awka_gets1(var);
          k = (int) A_MIN(array->nodeno, a_bivar[a_NF]->dval);
          j = var->slen + (k * 50) + 1;
          if (j >= dol0->allc)
          {
            if (dol0->ptr)
              dol0->allc = realloc( &dol0->ptr, j+1 );
            else
              dol0->allc = malloc( &dol0->ptr, j+1 );
          }
          op = dol0->ptr;

          /* no SAVEWIDTHS - rebuild according to OFS */
          rebuild_normal:
          memcpy(dol0->ptr, ptr, var->slen+1);
          op += var->slen;
          oldlen = var->slen;
          for (i=1; i<k; i++)
          {
            var = array->slot[i]->var;
            if (array->slot[i]->key == _a_SPLT_BASESTR &&
                var->type != a_VARUNK && var->type != a_VARNUL)
            {
              array->slot[i]->key = _a_SPLT_LOCALSTR;
            }

            ptr = awka_gets1(var);
            j += var->slen + ofs->slen;

            if (j >= dol0->allc)
            {
              dol0->allc = realloc( &dol0->ptr, j+1 );
              op = dol0->ptr + oldlen;
            }

            memcpy(op, ofs->ptr, ofs->slen);
            op += ofs->slen;
            memcpy(op, ptr, var->slen+1);
            op += var->slen;
            oldlen += var->slen + ofs->slen;
          }

          dol0->slen = _awka_dol0_len = oldlen;
          dol0->type = a_VARUNK;

          if (k <= 1 && j == 0)
          {
            dol0->ptr[0] = '\0';
            _awka_dol0_len = dol0->slen = 0;
          }

          tmpv = awka_getarrayval( a_bivar[a_PROCINFO], awka_tmp_str2var("FS") );
	  strcpy(tmpv->ptr, "FS");
        }
        else
        {
          awka_forcestr(dol0);
          dol0->ptr[0] = '\0';
          dol0->type = a_VARUNK;
          _awka_dol0_len = dol0->slen = 0;
        }

        _awka_setdol0_len = _awka_setdoln = _rebuild0 = _rebuild0_now = FALSE;
      }

      if (set == 2)
      {
        _rebuild0 = _rebuild0_now = FALSE;
        _awka_setdol0_len = _awka_setdoln = FALSE;
        _rebuildn = TRUE;
      }

      return dol0;
  }
}

/********************** $n field retrieval ********************/
/*
 * awka_doln
 * this takes care of accessing & modifying awk $n variables.
 * its here because the field array is an array.
 *
 * set is a_DOL_GET for readonly, a_DOL_SET to set the field value
 */
a_VAR *
awka_doln(int idx, int set)
{
  static char *x;
  register int i, j, nf = (int) awka_NFget()->dval;
  _a_HSHarray *array = (_a_HSHarray *) a_bivar[a_DOLN]->ptr;
  a_VAR *a_s_v = NULL;
  static a_VAR *nullvar = NULL;

  a_HSHNode *node;

  if (idx == 0)
  {
    return awka_dol0(set);
  }

  if (_rebuildn == TRUE)
  {
    awka_setd(a_bivar[a_NF]) = awka_arraysplitstr(awka_gets1(a_bivar[a_DOL0]), a_bivar[a_DOLN], a_bivar[a_FS], _split_max, TRUE);
    nf = (int) a_bivar[a_NF]->dval;
    array = (_a_HSHarray *) a_bivar[a_DOLN]->ptr;
    _rebuildn = FALSE;
  }

  idx--;
  if (idx < 0)
    awka_error("awka_doln: field variable referenced with negative index.\n");

  switch (set)
  {
    case a_DOL_GET:  /* 0 */
      /* if (idx >= array->nodeno) */
      if (idx >= nf || idx >= array->nodeno)
      {
        if (!nullvar)
  	{
	  malloc( &nullvar, sizeof(a_VAR) );
	  nullvar->allc = malloc( &nullvar->ptr, 1 );
	  nullvar->ptr[0] = '\0';
  	  nullvar->slen = nullvar->temp = nullvar->type2 = 0;
	  nullvar->type = a_VARUNK;
	  nullvar->dval = 0.0;
	}
        return nullvar;
      }
      break;

    default:
      _rebuild0 = TRUE;
      _awka_setdoln = TRUE;

      /* if (idx >= array->nodeno)  */
      if (idx >= nf || idx >= array->nodeno)
      {
        /* extend field array */
        array->nodeno = (array->nodeno > nf ? nf : array->nodeno);
        j = idx+1;
        if (j > array->nodeallc)
        {
          realloc( &array->slot, j * sizeof(a_HSHNode *) );
          array->nodeallc = j;
        }

        for (i=array->nodeno; i<j; i++)
        {
          malloc( &node, sizeof(a_HSHNode));
          malloc( &node->var, sizeof(a_VAR));
          _awka_hshinitnode(node);
          node->key = _a_SPLT_LOCALSTR;
          node->var->type = a_VARUNK;
          //node->var->type2 = 0;
          //node->var->dval = 0.0;
          node->var->allc = malloc( &node->var->ptr, 1 );
          node->var->ptr[0] = '\0';
          node->var->slen = 0;
          array->slot[i] = node;
        }
        array->nodeno = j;
        awka_setd(a_bivar[a_NF]) = j;
      }
      break;
  } /* switch */

  if (set == a_DOL_GET || array->slot[idx]->key == _a_SPLT_LOCALSTR)
    return array->slot[idx]->var;

  a_s_v = array->slot[idx]->var;
  if (a_s_v->type == a_VARUNK || a_s_v->type == a_VARSTR)
  {
    a_s_v->allc = malloc( &x, a_s_v->slen + 1);
    strcpy(x, a_s_v->ptr);
    a_s_v->ptr = x;
  }
  else
    a_s_v->ptr = NULL;

  array->slot[idx]->key = _a_SPLT_LOCALSTR;
  return a_s_v;
}

/*************************** array SORT functions ******************/
/*
 * awka_alistcmp
 * compares two nodes to see which is higher in sort order
 *
 * sorttype:
 *  2 -> numeric
 *  4 -> inverse (descending)  so 6 is inverse numeric
 *  8 -> values not index      so 10 is numeric values, 12 is inverse str values
 */
static INLINE int
_awka_alistcmp(a_HSHNode *node1, a_HSHNode *node2, int sorttype)
{
  char tmp[26], tmp2[26];

  if (sorttype & 8)
  { /* sort values - auto handles Numeric vs Alpha */
    if (sorttype & 4)
      return awka_varcmp(node2->var, node1->var);
    return awka_varcmp(node1->var, node2->var);
  } /* 8 */
  else
  if (sorttype & 2)
  {
    /* Numeric Sort */
    if (node1->type == _a_ARR_INT && node2->type == _a_ARR_STR)
    {
      if (node1->hval == atoi(node2->key)) return 0;
      if (sorttype & 4)
        return (node1->hval > atoi(node2->key) ? -1 : 1);
      return (node1->hval < atoi(node2->key) ? -1 : 1);
    }

    if (node1->type == _a_ARR_STR && node2->type == _a_ARR_INT)
    {
      if (atoi(node1->key) == node2->hval) return 0;
      if (sorttype & 4)
        return (atoi(node1->key) > node2->hval ? -1 : 1);
      return (atoi(node1->key) < node2->hval ? -1 : 1);
    }

    if (node1->type == _a_ARR_INT && node2->type == _a_ARR_INT)
    {
      if (node1->hval == node2->hval) return 0;
      if (sorttype & 4)
        return (node1->hval > node2->hval ? -1 : 1);
      return (node1->hval < node2->hval ? -1 : 1);
    }

    if (sorttype & 4)
      return (atoi(node1->key) > atoi(node2->key) ? -1 : 1);
    return (atoi(node1->key) < atoi(node2->key) ? -1 : 1);
  }
  else
  {
    /* Alphabetical Sort */
    if (node1->type == _a_ARR_INT && node2->type == _a_ARR_STR)
    {
      sprintf(tmp, "%d", node1->hval);
      if (sorttype & 4)
        return strcmp(node2->key, tmp);
      return strcmp(tmp, node2->key);
    }

    if (node1->type == _a_ARR_STR && node2->type == _a_ARR_INT)
    {
      sprintf(tmp, "%d", node2->hval);
      if (sorttype & 4)
        return strcmp(tmp, node1->key);
      return strcmp(node1->key, tmp);
    }

    if (node1->type == _a_ARR_INT && node2->type == _a_ARR_INT)
    {
      if (node1->hval == node2->hval) return 0;
      sprintf(tmp, "%d", node1->hval);
      sprintf(tmp2, "%d", node2->hval);
      if (sorttype & 4)
        return strcmp(tmp2, tmp);
      return strcmp(tmp, tmp2);
    }

    if (sorttype & 4)
      return strcmp(node2->key, node1->key);
    return strcmp(node1->key, node2->key);
  }
}

#define QSWAP(a, b) \
  node = a; \
  a = b; \
  b = node

/*
 * awka_qsort
 * provides a sorted 'for (people in trains)' list
 */
void
_awka_qsort(a_HSHNode **nlist, int hi, int sorttype)
{
  unsigned i, j, ln, rn;
  a_HSHNode *node;

  while (hi > 1)
  {
    QSWAP(nlist[0], nlist[hi/2]);
    for (i = 0, j = hi; ; )
    {
      do
        if (--j > hi) { j++; break; }
      while (_awka_alistcmp(nlist[j], nlist[0], sorttype) > 0);

      do
        if (++i >= hi) { i--; break; }
      while (i < j && _awka_alistcmp(nlist[i], nlist[0], sorttype) < 0);

      if (i >= j)
        break;
      QSWAP(nlist[i], nlist[j]);
    }
    QSWAP(nlist[j], nlist[0]);
    ln = j;
    rn = hi - ++j;
    if (ln < rn)
    {
      _awka_qsort(nlist, ln, sorttype);
      nlist += j;
      hi = rn;
    }
    else
    {
      _awka_qsort(nlist + j, rn, sorttype);
      hi = ln;
    }
  }
}

/*
 * awka_arrayloop
 * for implementation of awk 'for (people in trains)' loops.
 * Calling function responsible for freeing returned array.
 */
int
awka_arrayloop( a_ListHdr *ah, a_VAR *v, char asort )
{
  a_List *alist;
  int i=0, j, sorttype = 0;
  _a_HSHarray *array;
  a_HSHNode *node;

  if (v->type != a_VARARR)
    awka_error("runtime error: Scalar used as array in call to ArrayLoop\n");
  array = (_a_HSHarray *) v->ptr;

  sorttype = awka_getd(a_bivar[a_SORTTYPE]);

  if (ah->used == ah->allc)
  {
    if (ah->allc == 0)
      malloc( &ah->list, 5 * sizeof(a_List));
    else
      realloc( &ah->list, (ah->allc + 5) * sizeof(a_List));
    ah->allc += 5;
  }
  alist = &(ah->list[ah->used++]);

  if (!array)
  {
    malloc( &alist->node, sizeof(a_HSHNode *));
    alist->node[0] = 0;
    alist->type = a_ARR_TYPE_HSH;
    return 0;
  }

  malloc( &alist->node, (array->nodeno+1) * sizeof(a_HSHNode *) );
  alist->type = array->type;
  alist->base = array->base;
  alist->nodeno = array->nodeno;

  if (array->type == a_ARR_TYPE_HSH)
  {
    for (j=0, i=0; j<=array->hashmask; j++)
    {
      node = array->slot[j];
      while (node)
      {
        if (node->shadow == FALSE)
          alist->node[i++] = node;
        node = node->next;
      }
    }

    if (sorttype && i > 1)
      _awka_qsort(alist->node, i, sorttype);
    else if (asort && i > 1)
      _awka_qsort(alist->node, i, 1);
  }
  else
  {
    /* split array */
    for (i=0; i<array->nodeno; i++)
      alist->node[i] = array->slot[i];
  }

  alist->node[i] = NULL;
  return 0;
}

/*
 * awka_arraynext
 * gets next node in a 'for (people in trains)' loops.
 */
int
awka_arraynext( a_VAR *v, a_ListHdr *ah, int pos )
{
  a_List *alist = &(ah->list[ah->used-1]);

  switch (alist->type)
  {
    case a_ARR_TYPE_HSH:
      if (pos >= alist->nodeno || alist->node[pos] == NULL)
        return 0;

      if (v->type == a_VARARR)
        awka_error("runtime error: Array used as scalar in call to ArrayNext.\n");

      if (alist->node[pos]->type == _a_ARR_INT)
        awka_setd(v) = (double) ((int) alist->node[pos]->hval);
      else
        awka_strcpy(v, alist->node[pos]->key);
      break;

    case a_ARR_TYPE_SPLIT:
      if (pos >= alist->nodeno || alist->node[pos] == NULL)
        return 0;

      if (v->type != a_VARDBL)
        _awka_setdval(v, __FILE__, __LINE__);

      v->dval = pos + alist->base;
      break;

    default:
      return 0;
  }

  return pos+1;
}

/*
 * Get the var from the listHdr based on arraynext "next" position
 * which is the int returned by awka_arraynext()
 */
a_VAR *
awka_arraynextget(a_ListHdr *ah, int pos )
{
  a_List *alist = &(ah->list[ah->used-1]);

  return alist->node[pos-1]->var;
}

void
awka_alistfree( a_ListHdr *ah )
{
  a_List *alist;
  alist = &(ah->list[--ah->used]);

  if (alist->node)
    free(alist->node);
}

void
awka_alistfreeall( a_ListHdr *ah )
{
  int i;

  if (ah->list)
  {
    for (i=0; i<ah->used; i++)
      if (ah->list[i].node)
        free(ah->list[i].node);

    free(ah->list);
  }
  ah->used = ah->allc = 0;
  ah->list = NULL;
}

/*
 * asort()  - gawk 3.1.0 array sorting function
 * sorts src.  If dest==NULL, overwrites src's indexes
 * with numbers indicating position in sort order starting
 * from 1.  Otherwise copies src into dest with new index.
 * returns number of elements in src.
 */
double
awka_asort( a_VAR *src, a_VAR *dest )
{
  a_ListHdr listhdr;
  double ret = (double) ((_a_HSHarray *) src->ptr)->nodeno;
  _a_HSHarray *array;
  a_VAR *ivar = NULL, *newnode, *idxvar;
  int idx, nodest = 0;

  listhdr.allc = listhdr.used = 0;
  listhdr.list = NULL;
  awka_varinit(ivar);
  awka_varinit(idxvar);

  /* make a sorted list of pointers to array elements */
  awka_arrayloop( &listhdr, src, 1 );

  /* make sure dest is ready to receive the new array */
  if (dest)
    awka_arrayclear( dest );
  else
  {
    awka_varinit( dest );
    dest->type = a_VARARR;
    nodest = 1;
  }
  array = (_a_HSHarray *) dest->ptr;
  if (array)
    array->type = a_ARR_TYPE_SPLIT;
  else
    awka_arraycreate( dest, a_ARR_TYPE_SPLIT );

  /* kludgy hack.  but easily coded and understood  ;-)
   * this iterates through src and inserts each element
   * into dest, using the index in the sorted loop as
   * the array index * /
  for ( idx=0; (idx = awka_arraynext(ivar, &listhdr, idx)) > 0; )
  {
    awka_vardblset( idxvar, (double) idx );
    newnode = awka_arraysearch1( dest, idxvar, a_ARR_CREATE, 0 );
    awka_varcpy( newnode, awka_arraysearch1( src, ivar, a_ARR_CREATE, 0 ) );
  }
  */
  for ( idx=0; (idx = awka_arraynext(ivar, &listhdr, idx)) > 0; )
  {
    awka_vardblset( idxvar, (double) idx );
    newnode = awka_arraysearch1( dest, idxvar, a_ARR_CREATE, 0 );
    awka_varcpy( newnode, awka_arraynextget(&listhdr, idx ) );
  }
  awka_alistfree( &listhdr );

  if (nodest)
  {
    /* oops, dest not provided.  We kill src and make it dest. */
    awka_killvar( src );
    memcpy(src, dest, sizeof(a_VAR));
  }

  return ret;
}
