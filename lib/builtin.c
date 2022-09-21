/*------------------------------------------------------------*
 | builtin.c                                                  |
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
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

#ifndef NO_TIME_H
#  include <time.h>
#else
#  include <sys/types.h>
#endif

#define BUILTIN_HOME
#define _IN_LIBRARY

#include "libawka.h"
#include "varg.h"
#include "builtin_priv.h"
#include "number.h"
#include "nstring.h"
#include "garbage.h"

#include <math.h>

extern int _awka_fileoffset;
int _awka_curfile = -1, _awka_file_read = TRUE;
int _awka_file_stream = -1;  // current file stream
int _dol0_used = 0;

int awka_fclose( int i );

#define _awka_getstringvar \
  if (keep == a_TEMP) \
  { \
    _awka_tmpvar(outvar); \
    awka_forcestr(outvar); \
  } \
  else \
  { \
    malloc( &outvar, sizeof(a_VAR)); \
    memset( outvar, 0, sizeof(a_VAR)); \
  } \
  outvar->type2 = 0; \
  outvar->type = a_VARSTR

#define _awka_getdoublevar \
  if (keep == a_TEMP) \
  { \
    _awka_tmpvar(outvar); \
    if (outvar->type == a_VARREG) \
      _awka_re2null(outvar); \
  } \
  else \
  { \
    awka_varinit(outvar); \
  } \
  outvar->type = a_VARDBL; \
  outvar->type2 = 0; \
  outvar->dval = 0

a_VAR *
awka_getstringvar(char keep)
{
  a_VAR *outvar;

  _awka_getstringvar;
  return outvar;
}

a_VAR *
awka_getdoublevar(char keep)
{
  a_VAR *outvar;

  _awka_getdoublevar;
  return outvar;
}

#define _awka_checkbiargs( va, name, func ) \
{ \
  if (va->used < _a_bi_vararg[func].min_args) \
    awka_error("internal runtime error: only %d args passed to %s - needed %d.\n", \
                           va->used, name, _a_bi_vararg[func].min_args); \
 \
  if (va->used > _a_bi_vararg[func].max_args) \
    awka_error("internal runtime error: %d args passed to %s - max allowed is %d.\n", \
                           va->used, name, _a_bi_vararg[func].max_args); \
}

/*
 * awka_VarArg
 * given a variable list of VARs as input,
 * creates an _a_VarArg structure.  Usually used to pass to
 * builtin functions.
 */
a_VARARG *
awka_vararg(char keep, a_VAR *var, ...)
{
  va_list ap;
  a_VARARG *va;

  if (keep == a_TEMP)
  {
    _awka_tmpvar_a(va);
  }
  else
    malloc( &va, sizeof(a_VARARG));

  va->used = 0;
  va->var[0] = var;

  if (var != NULL)
  {
    va_start(ap, var);
    while (va->used < VARARGSZ-1 && (va->var[++va->used] = va_arg(ap, a_VAR *)) != NULL)
      ;
    va_end(ap);
  }

  return va;
}

a_VARARG *
awka_arg0(char keep)
{
  a_VARARG *va;

  if (keep == a_TEMP)
    { _awka_tmpvar_a(va); }
  else
    malloc( &va, sizeof(a_VARARG));

  va->used = 0;

  return va;
}

a_VARARG *
awka_arg1(char keep, register a_VAR *var)
{
  a_VARARG *va;

  if (keep == a_TEMP)
  {
    _awka_tmpvar_a(va);
  }
  else
    malloc( &va, sizeof(a_VARARG));

  va->used = 1;
  va->var[0] = var;

  return va;
}

a_VARARG *
awka_arg2(char keep, a_VAR *v1, a_VAR *v2)
{
  a_VARARG *va;

  if (keep == a_TEMP)
  {
    _awka_tmpvar_a(va);
  }
  else
    malloc( &va, sizeof(a_VARARG));

  va->used = 2;
  va->var[0] = v1;
  va->var[1] = v2;

  return va;
}

a_VARARG *
awka_arg3(char keep, a_VAR *v1, a_VAR *v2, a_VAR *v3)
{
  a_VARARG *va;

  if (keep == a_TEMP)
  {
    _awka_tmpvar_a(va);
  }
  else
    malloc( &va, sizeof(a_VARARG));

  va->used = 3;
  va->var[0] = v1;
  va->var[1] = v2;
  va->var[2] = v3;

  return va;
}

a_VAR *
awka_strconcat( char keep, a_VARARG *va )
{
  register int len, oldlen, i=1, alloc;
  a_VAR *outvar;
  register char *ptr, *op;

  /* check argument list */
  _awka_checkbiargs( va, "awka_strconcat", _BI_STRCONCAT );
  _awka_getstringvar;

  ptr = awka_gets1(va->var[0]);
  /*alloc = va->var[0]->slen + ((va->used-1) * 50) + 1; */
  alloc = va->var[0]->slen * (va->used) + 1;

  if (!outvar->ptr)
    alloc = malloc( &outvar->ptr, alloc );
  else if (outvar->allc < alloc)
    alloc = realloc( &outvar->ptr, alloc );
  else
    alloc = outvar->allc;

  oldlen = len = va->var[0]->slen;
  memcpy(outvar->ptr, ptr, len+1);
  op = outvar->ptr + va->var[0]->slen;

  for (i=1; i<va->used; i++)
  {
    oldlen = len;
    ptr = awka_gets1(va->var[i]);
    len += va->var[i]->slen;

    if (len >= alloc)
    {
      alloc = realloc( &outvar->ptr, alloc + len + ((va->used - i - 1) * 20) );
      op = outvar->ptr + oldlen;
    }

    memcpy(op, ptr, va->var[i]->slen+1);
    op += va->var[i]->slen;
  }

  outvar->slen = len;
  outvar->allc = alloc;

  return (outvar);
}

static inline void
_awka_strconcat5(a_VAR *res,
		 char *s1, int s1len, char *s2, int s2len,
		 char *s3, int s3len, char *s4, int s4len,
		 char *s5, int s5len)
{
  register char *op;

  /* concat the 5 strings into  res */
  awka_setstrlen(res, s1len + s2len + s3len + s4len + s5len);

  op = res->ptr;
  if (s1len) {
    memcpy(op, s1, s1len);
    op += s1len;
  }

  if (s2len) {
    memcpy(op, s2, s2len);
    op += s2len;
  }

  if (s3len) {
    memcpy(op, s3, s3len);
    op += s3len;
  }

  if (s4len) {
    memcpy(op, s4, s4len);
    op += s4len;
  }

  if (s5len) {
    memcpy(op, s5, s5len);
    op += s5len;
  }

  *op = '\0';
}
/*
 * awka_strconcat2
 * Concatenates 2 strings as per awk 'x = "abc" z;'
 *
 * concatenating an array (not array element)
 * gets the length of the array as a number converted
 * to a string
 */
a_VAR *
awka_strconcat2( char keep, a_VAR *v1, a_VAR *v2)
{
  int p1len, p2len;
  char *p1, *p2;
  a_VAR *outvar;

  /* create a variable & put the strings together */
  _awka_getstringvar;

  /* how long are the combined strings? */
  if (v1->type == a_VARARR) {
    p1 = awka_gets1(awka_tmp_dbl2var(awka_alength(v1)));
    p1len = strlen(p1);
  } else {
    p1 = awka_gets1(v1);
    p1len = v1->slen;
  }

  if (v2->type == a_VARARR) {
    p2 = awka_gets1(awka_tmp_dbl2var(awka_alength(v2)));
    p2len = strlen(p2);
  } else {
    p2 = awka_gets1(v2);
    p2len = v2->slen;
  }

  _awka_strconcat5(outvar,
		  p1, p1len, p2, p2len,
		  NULL, 0, NULL, 0, NULL, 0);

  return (outvar);
}

a_VAR *
awka_strconcat3( char keep, a_VAR *v1, a_VAR *v2, a_VAR *v3)
{
  char *p1, *p2, *p3;
  int p1len, p2len, p3len;
  a_VAR *outvar;

  /* create a variable & put the strings together */
  _awka_getstringvar;

  /* how long are the combined strings? */
  if (v1->type == a_VARARR) {
    p1 = awka_gets1(awka_tmp_dbl2var(awka_alength(v1)));
    p1len = strlen(p1);
  } else {
    p1 = awka_gets1(v1);
    p1len = v1->slen;
  }

  if (v2->type == a_VARARR) {
    p2 = awka_gets1(awka_tmp_dbl2var(awka_alength(v2)));
    p2len = strlen(p2);
  } else {
    p2 = awka_gets1(v2);
    p2len = v2->slen;
  }

  if (v3->type == a_VARARR) {
    p3 = awka_gets1(awka_tmp_dbl2var(awka_alength(v3)));
    p3len = strlen(p3);
  } else {
    p3 = awka_gets1(v3);
    p3len = v3->slen;
  }

  _awka_strconcat5(outvar,
		  p1, p1len, p2, p2len, p3, p3len,
		  NULL, 0, NULL, 0);

  return (outvar);
}

a_VAR *
awka_strconcat4( char keep, a_VAR *v1, a_VAR *v2, a_VAR *v3, a_VAR *v4 )
{
  char *p1, *p2, *p3, *p4;
  int p1len, p2len, p3len, p4len;
  a_VAR *outvar;

  /* create a variable & put the strings together */
  _awka_getstringvar;

  /* how long are the combined strings? */
  if (v1->type == a_VARARR) {
    p1 = awka_gets1(awka_tmp_dbl2var(awka_alength(v1)));
    p1len = strlen(p1);
  } else {
    p1 = awka_gets1(v1);
    p1len = v1->slen;
  }

  if (v2->type == a_VARARR) {
    p2 = awka_gets1(awka_tmp_dbl2var(awka_alength(v2)));
    p2len = strlen(p2);
  } else {
    p2 = awka_gets1(v2);
    p2len = v2->slen;
  }

  if (v3->type == a_VARARR) {
    p3 = awka_gets1(awka_tmp_dbl2var(awka_alength(v3)));
    p3len = strlen(p3);
  } else {
    p3 = awka_gets1(v3);
    p3len = v3->slen;
  }

  if (v4->type == a_VARARR) {
    p4 = awka_gets1(awka_tmp_dbl2var(awka_alength(v4)));
    p4len = strlen(p4);
  } else {
    p4 = awka_gets1(v4);
    p4len = v4->slen;
  }

  _awka_strconcat5(outvar,
		  p1, p1len, p2, p2len, p3, p3len, p4, p4len,
		  NULL, 0);

  return (outvar);
}

a_VAR *
awka_strconcat5( char keep, a_VAR *v1, a_VAR *v2, a_VAR *v3, a_VAR *v4, a_VAR *v5)
{
  char *p1, *p2, *p3, *p4, *p5;
  int p1len, p2len, p3len, p4len, p5len;
  a_VAR *outvar;

  /* create a variable & put the strings together */
  _awka_getstringvar;

  /* how long are the combined strings? */
  if (v1->type == a_VARARR) {
    p1 = awka_gets1(awka_tmp_dbl2var(awka_alength(v1)));
    p1len = strlen(p1);
  } else {
    p1 = awka_gets1(v1);
    p1len = v1->slen;
  }

  if (v2->type == a_VARARR) {
    p2 = awka_gets1(awka_tmp_dbl2var(awka_alength(v2)));
    p2len = strlen(p2);
  } else {
    p2 = awka_gets1(v2);
    p2len = v2->slen;
  }

  if (v3->type == a_VARARR) {
    p3 = awka_gets1(awka_tmp_dbl2var(awka_alength(v3)));
    p3len = strlen(p3);
  } else {
    p3 = awka_gets1(v3);
    p3len = v3->slen;
  }

  if (v4->type == a_VARARR) {
    p4 = awka_gets1(awka_tmp_dbl2var(awka_alength(v4)));
    p4len = strlen(p4);
  } else {
    p4 = awka_gets1(v4);
    p4len = v4->slen;
  }

  if (v5->type == a_VARARR) {
    p5 = awka_gets1(awka_tmp_dbl2var(awka_alength(v5)));
    p5len = strlen(p5);
  } else {
    p5 = awka_gets1(v5);
    p5len = v5->slen;
  }

  _awka_strconcat5(outvar,
		  p1, p1len, p2, p2len, p3, p3len, p4, p4len, p5, p5len);

  return (outvar);
}

/*
 * awka_match
 * awk 'match' function and (x ~ y)
 */
a_VAR *
awka_match( char keep, char fcall, a_VAR *va, a_VAR *rva, a_VAR *arr )
{
  char *start, *end, *ptr, tmp[25];
  a_VAR *outvar, *pvar, *pindex;
  awka_regexp *r;
  int i, nmatch = arr ? 20 : (int) fcall;
  static regmatch_t pmatch[20];

  /* create a variable */
  _awka_getdoublevar;
  memset(pmatch, 0, sizeof(pmatch));

  /* match the string */
  if (rva->type != a_VARREG)
    _awka_getreval(rva, __FILE__, __LINE__, _RE_MATCH);

  r = (awka_regexp *) rva->ptr;

  if (r->cant_be_null)
  {
    r = _awka_compile_regexp_MATCH(r->origstr, r->strlen);
    rva->ptr = (char *) r;
  }

  //rva->type = a_VARREG;
  ptr = awka_gets1(va);

  if (arr)
    awka_arrayclear( arr );

  if (awka_regexec(r, ptr, nmatch, pmatch, (fcall == TRUE ? REG_NEEDSTART : 0)))
  {
    /* failed to find RE */
    if (fcall == TRUE)
    {
      awka_setd(a_bivar[a_RSTART]) = 0;
      awka_setd(a_bivar[a_RLENGTH]) = -1;
    }
    outvar->dval = 0;

    return outvar;
  }

  /* found RE */
  start = ptr + pmatch->rm_so;
  end   = ptr + pmatch->rm_eo;

  if (arr)
  {
    awka_varinit( pindex );
    for (i=0; i<r->max_sub; i++)
    {
      if (pmatch[i].rm_so == pmatch[i].rm_eo)
        break;

      outvar->dval = i;
      pvar = awka_getarrayval( arr, outvar );
      awka_strncpy( pvar, ptr + pmatch[i].rm_so, pmatch[i].rm_eo - pmatch[i].rm_so );

      /* arr[i, start] = n */
      sprintf( tmp, "%i%sstart", i, awka_gets1(a_bivar[a_SUBSEP]) );
      awka_strscpy( pindex, tmp );
      pvar = awka_getarrayval( arr, pindex );
      sprintf( tmp, "%i", pmatch[i].rm_so + 1 );  /* 1-indexed string start */
      awka_strcpy( pvar, tmp );

      /* arr[i, length] = n */
      sprintf( tmp, "%i%slength", i, awka_gets1(a_bivar[a_SUBSEP]) );
      awka_strscpy( pindex, tmp );
      pvar = awka_getarrayval( arr, pindex );
      sprintf( tmp, "%i", pmatch[i].rm_eo - pmatch[i].rm_so );
      awka_strcpy( pvar, tmp );

    }
    awka_killvar( pindex );
  }

  outvar->dval = 1.0;

  if (fcall == TRUE)
  {
    awka_setd(a_bivar[a_RSTART]) = (start - ptr) + 1;
    awka_setd(a_bivar[a_RLENGTH]) = (end - start < 1 ? 1 : end - start);
    outvar->dval = (start - ptr) + 1;
  }

  return (outvar);
}

/*
 * awka_substr
 * awk builtin function 'substr'
 */
a_VAR *
awka_substr(char keep, a_VAR *var, double start, double end)
{
  a_VAR *outvar = NULL;
  register int len1, len2;
  register char *ptr;
  double a, b;

  /* check argument list */
  if ((a = start) < 1)
    a = 1;

  a = (double) ((int) a);

  /* create a variable */
  _awka_getstringvar;

  /* compute length of substring */
  ptr = awka_gets1(var);

  if (var->slen < a)
  {
    /* string not long enough for specified substring */
    outvar->slen = 0;
    if (!outvar->ptr)
      outvar->allc = malloc( &outvar->ptr, 1 );
    outvar->allc = 1;
    outvar->ptr[0] = '\0';
  }
  else
  {
    len1 = len2 = (var->slen - a) + 1;

    if ((int) end != INT_MAX)
    {
      if ((b = end) < 0)
        b = 0;

      b = (double) ((int) b);
      len2 = A_MIN(len1, b);
    }

    /* allocate space for substring */
    if (!outvar->ptr)
      outvar->allc = malloc( &outvar->ptr, len2 + 1 );
    else if (outvar->allc <= len2)
      outvar->allc = realloc( &outvar->ptr, len2 + 1 );

    /* copy it over */
    memcpy( outvar->ptr, ptr + ((int) a - 1), len2 );
    outvar->ptr[len2] = '\0';
    outvar->slen = len2;
  }

  /* get outa here */
  return (outvar);
}

struct re_res {
  char *match_bgn;
  char *match_end;
  char **startp;
  char **endp;
  int  max_sub;
  int  sub_allc;
};

/*
 * awka_sub
 * awk builtin functions 'sub' and 'gsub'
 */
a_VAR *
awka_sub(char keep, char gsub, int gensub, a_VAR *rva, a_VAR *sva, a_VAR *tva)
{
  a_VAR *outvar;
  char *start = NULL, *end=NULL, *ptr, *tptr, orig_type;
  static struct re_res *re_result = NULL;
  static char *tmp = NULL;
  static int m_alloc = 0, t_alloc = 0;
  register int i, zmatch = 0, match_no = 0, match_len = 0, amp = 0;
  register char *p;
  regmatch_t *pmatch;
  awka_regexp *r;

  _awka_getdoublevar;

  /* various initialisation stuff */
  if (tva == a_bivar[a_DOL0])
  {
    _rebuild0_now = FALSE;
    _rebuildn = TRUE;
  }
  else
    _awka_set_FW(tva);

  orig_type = tva->type;

  if (tva == rva)
  {
    awka_varcpy(tva, sva);

    if (orig_type == a_VARUNK)
      tva->type = orig_type;

    outvar->dval = 1;

    return outvar;
  }

  if (rva->type != a_VARREG)
  {
    r = _awka_compile_regexp_GSUB(awka_gets(rva), rva->slen);

    if (rva->ptr)
      free(rva->ptr);

    rva->ptr = (char *) r;
  }
  else
  {
    r = (awka_regexp *) rva->ptr;

    if (r->gsub != TRUE)
    {
      r = _awka_compile_regexp_GSUB(r->origstr, r->strlen);
      rva->ptr = (char *) r;
    }
  }

  rva->type = a_VARREG;

  if (!m_alloc)
  {
    m_alloc = 20;
    malloc( &re_result, 20 * sizeof(struct re_res) );

    for (i=0; i<m_alloc; i++)
    {
      re_result[i].sub_allc = 10;
      malloc( &re_result[i].startp, 10 * sizeof(char *) );
      malloc( &re_result[i].endp, 10 * sizeof(char *) );
    }
  }

  for (i=0; i<m_alloc; i++)
    re_result->max_sub = 0;

  /* find ampersands, used to insert matched text in substitution string */
  awka_gets(sva);

  if (strchr(sva->ptr, '&'))
  {
    for (ptr=sva->ptr; *ptr; ptr++)
      if (*ptr == '&')
      {
        if (ptr > sva->ptr && *(ptr-1) == '\\')
          continue;
        amp++;
      }
  }

  if (gensub && strchr(sva->ptr, '\\'))
  {
    for (ptr=sva->ptr; *ptr && *(ptr+1); ptr++)
      if (*ptr == '\\' && isdigit(*(ptr+1)))
      {
        if (ptr > sva->ptr && *(ptr-1) == '\\')
          continue;
        amp = (amp >= *(ptr+1)-'0' ? amp : *(ptr+1)-'0');
        ptr++;
      }
  }

  malloc(&pmatch, (amp+1) * sizeof(regmatch_t));

  awka_gets(tva);

starthere:
  if (!tva->ptr)
    awka_strcpy(tva, "");

  ptr = tva->ptr;
  outvar->dval = match_no = 0;

  /* first pass - find substitution points */
  do
  {
    if (gensub && !*ptr)
      break;

    /* if (!(awka_regexec(r, ptr, &start, &end, FALSE, sub))) */
    if (awka_regexec(r, ptr, amp+1, pmatch, REG_NEEDSTART))
      break;

    if (match_no >= m_alloc)
    {
      m_alloc *= 2;
      realloc( &re_result, m_alloc * sizeof(struct re_res) );

      for (i=m_alloc/2; i<m_alloc; i++)
      {
        re_result[i].sub_allc = 10;
        malloc( &re_result[i].startp, 10 * sizeof(char *) );
        malloc( &re_result[i].endp, 10 * sizeof(char *) );
      }
    }

    if (gensub)
    {
      if (r->max_sub > re_result[match_no].sub_allc)
      {
        re_result[match_no].sub_allc = r->max_sub + 10;
        realloc( &re_result[match_no].startp, (r->max_sub + 10) * sizeof(char *) );
        realloc( &re_result[match_no].endp, (r->max_sub + 10) * sizeof(char *) );
      }

      for (i=0; i<r->max_sub; i++)
      {
        re_result[match_no].startp[i] = ptr + pmatch[i].rm_so;
        re_result[match_no].endp[i]   = ptr + pmatch[i].rm_eo;
      }

      re_result[match_no].max_sub = r->max_sub;
    }

    re_result[match_no].match_bgn   = start = ptr + pmatch[0].rm_so;
    re_result[match_no++].match_end = end   = ptr + pmatch[0].rm_eo;

    if (!*ptr && match_no > 1 &&
        re_result[match_no-2].match_bgn == re_result[match_no-1].match_bgn &&
        re_result[match_no-2].match_end == re_result[match_no-1].match_end)
      match_no--;

    match_len = (start - end > match_len ? start - end : match_len);

    if (!*ptr || (gensub && gensub != -1 && !pmatch[0].rm_so && !pmatch[0].rm_eo) || gensub == -2)
      break;

    ptr = end;

    if (start == end && !zmatch)
    {
      zmatch = 1;
      goto starthere;
    }

    if (*ptr)
      ptr += zmatch;

  } while (gsub && !r->reganch);

  if (orig_type == a_VARUNK)
    tva->type = orig_type;

  if (!match_no)
  {
    free(pmatch);

    return outvar;
  }

  /* second pass - substitute away! */
  awka_gets(sva);

  if (!t_alloc)
  {
    if (amp)
      t_alloc = tva->slen + (match_no * (sva->slen+(amp*match_len)+1) * 2) + 1;
    else
      t_alloc = tva->slen + (match_no * (sva->slen+1) * 2) + 1;

    malloc( &tmp, t_alloc );
  }
  else if (amp && t_alloc < tva->slen + (match_no * (sva->slen+(amp*match_len)+1) * 2) + 1)
  {
    t_alloc = tva->slen + (match_no * (sva->slen+(amp*match_len)+1) * 2) + 1;
    realloc( &tmp, t_alloc );
  }
  else if (t_alloc < tva->slen + (match_no * (sva->slen+1) * 2) + 1)
  {
    t_alloc = tva->slen + (match_no * (sva->slen+1) * 2) + 1;
    realloc( &tmp, t_alloc );
  }

  tptr = tmp;
  ptr = tva->ptr;

  for (i=0; i<match_no; i++)
  {
    if (gensub > 1 && i != gensub-2)
      continue;

    if (re_result[i].match_bgn > ptr)
    {
      memcpy(tptr, ptr, re_result[i].match_bgn - ptr);
      tptr += re_result[i].match_bgn - ptr;
      *tptr = '\0';
    }

    if (sva->slen)
    {
      for (p=sva->ptr; p-sva->ptr < sva->slen; p++)
      {
        if (amp)
        {
          if (gensub)
          {
            if (p > sva->ptr && *(p-1) == '\\')
            {
              *(tptr-1) = *p;
              continue;
            }

            if (*p == '&' || (*p == '\\' && *(p+1) == '0'))
            {
              /* copy matched text to target string */
              memcpy(tptr, re_result[i].match_bgn,
                     re_result[i].match_end - re_result[i].match_bgn);
              tptr += re_result[i].match_end - re_result[i].match_bgn;

              if (*p == '\\')
                p++;
            }
            else if (*p == '\\' && isdigit(*(p+1)))
            {
              /* copy matching sub-expression to target string */
              int sub = atoi(p+1);

              if (sub > re_result[i].max_sub)
                continue;

              memcpy(tptr, re_result[i].startp[sub],
                     re_result[i].endp[sub] - re_result[i].startp[sub]);
              tptr += re_result[i].endp[sub] - re_result[i].startp[sub];
              p++;
            }
            else
            {
              *(tptr++) = *p;
              continue;
            }
          }
          else
          {
            if (*p == '\\' && *(p+1) == '&')
            {
              /* drop the backslash, save the literal ampersand */
              *(tptr++) = *(++p);
              continue;
            }

            if (*p != '&')
            {
              /* copy this character to target string */
              *(tptr++) = *p;
              continue;
            }

            /* copy matched text to target string */
            memcpy(tptr, re_result[i].match_bgn,
                   re_result[i].match_end - re_result[i].match_bgn);
            tptr += re_result[i].match_end - re_result[i].match_bgn;
          }

          if (*p == '\\')
            p++;
        }
        else
        {
          if (*p == '\\' && *(p+1) == '&')
          {
            /* drop the backslash, save the literal ampersand */
            *(tptr++) = *(++p);
            continue;
          }
          *(tptr++) = *p;
        }
      }
    }

    *tptr = '\0';
    ptr = re_result[i].match_end;
  } /* for */

  if (*ptr)
    strcpy(tptr, ptr);

  outvar->dval = match_no;
  awka_strcpy(tva, tmp);

  if (orig_type == a_VARUNK)
    tva->type = orig_type;

  free(pmatch);

  return (outvar);
}

a_VAR *
awka_gensub(char keep, a_VAR *rva, a_VAR *sva, a_VAR *hva, a_VAR *tva)
{
  a_VAR *outvar;
  char *p;
  register int i;

  _awka_getstringvar;
  awka_varcpy(outvar, tva);

  p = awka_gets1(hva);

  if (*p == 'G' || *p == 'g')
    awka_sub(keep, TRUE, -1, rva, sva, outvar);
  else {
    i = (unsigned int) atoi(p);

    if (!i)
      awka_sub(keep, TRUE, -2, rva, sva, outvar);
    else
      awka_sub(keep, TRUE, i+1, rva, sva, outvar);
  }

  return outvar;
}

/*
 * awka_tocase
 * awk builtin functions 'toupper', 'tolower' and extended function 'totitle'
 */
a_VAR *
awka_tocase( char keep, char which, a_VAR *var )
{
  a_VAR *outvar;
  register char *s, *ptr;

  /* create a variable */
  _awka_getstringvar;

  ptr = awka_gets1(var);

  if (var->slen)
  {
    awka_strcpy(outvar, ptr);
    s = outvar->ptr;

    switch (which)
    {
      case a_BI_TOUPPER:
        while (*s)
          if (islower(*s++))
            *(s-1) += - 'a' + 'A';
        break;

      case a_BI_TOLOWER:
        while (*s)
          if (isupper(*s++))
            *(s-1) += - 'A' + 'a';
        break;

      case a_BI_TOTITLE:
        if (*s)
        {
          if (islower(*s++))
            *(s-1) += - 'a' + 'A';
        }
        else
          break;

        while (*s)
        {
          if (islower(*s) && isspace(*(s-1)))
            *s += - 'a' + 'A';
          else if (isupper(*s))
            *s += - 'A' + 'a';
          s++;
        }
        break;
    } /* switch */
  }
  else
  {
    if (!outvar->ptr)
      outvar->allc = malloc( &outvar->ptr, 1 );

    outvar->slen = 0;
    outvar->ptr[0] = '\0';
  }

  /* get outa here */
  return (outvar);
}

/*
 * awka_system
 * awk builtin 'system' function
 */
a_VAR *
awka_system( char keep, a_VAR *va )
{
  a_VAR *outvar;
  register int i;
  register char *ptr;
  int pid;

  _awka_getdoublevar;

  /* flush io */
  for (i=0; i<_a_ioused; i++)
    if (_a_iostream[i].io & _a_IO_WRITE ||
        _a_iostream[i].io == _a_IO_APPEND)
      fflush(_a_iostream[i].fp);

  ptr = awka_gets1(va);

  /* outvar->dval = (double) system(ptr) / 256; */
  switch (pid = fork())
  {
    case -1:
      /* failed to fork - use system instead */
      outvar->dval = (double) system(ptr) / 256;
      break;

    case 0:
      execl(awka_shell, awka_shell, "-c", ptr, (char *) NULL);
      /* if here, process wouldn't start.  Again, use system() */
      fflush(stderr);

      _exit(system(ptr) / 256);

    default:
      outvar->dval = _awka_wait_pid(pid);
      break;
  }

  return (outvar);
}

/*
 * awka_trim
 * awk extended builtin function 'trim'
 */
a_VAR *
awka_trim(char keep, a_VARARG *va)
{
  a_VAR *outvar;
  register char *p, *q, *r;

  /* create a variable */
  _awka_getstringvar;

  /* compute length of substring */
  awka_strcpy(outvar, awka_gets1(va->var[0]));
  p = outvar->ptr;

  if (va->var[0]->slen)
  {
    if (va->used == 2)
    {
      r = awka_gets1(va->var[1]);

      while (*p)
      {
        for (q=r; *q; q++)
          if (*p == *q)
            break;

        if (*q)
          p++;
        else
          break;
      }
    }
    else
    {
      /* get rid of preceding whitespace */
      while (*p)
      {
        if (isspace(*p))
          p++;
        else
          break;
      }
    }
  }

  if (p > outvar->ptr)
  {
    outvar->slen -= (p - outvar->ptr);
    memmove(outvar->ptr, p, outvar->slen + 1);
  }

  if (outvar->slen)
  {
    p = (outvar->ptr + outvar->slen) - 1;

    if (va->used == 2)
    {
      r = awka_gets1(va->var[1]);

      while (p > outvar->ptr)
      {
        for (q=r; *q; q++)
          if (*p == *q)
            break;

        if (*q)
        {
          *p-- = '\0';
          outvar->slen--;
        }
        else
          break;
      }
    }
    else
    {
      /* remove trailing whitespace */
      while (p > outvar->ptr)
      {
        if (!(isspace(*p)))
          break;

        *p-- = '\0';
        outvar->slen--;
      }
    }
  }

  /* get outa here */
  return (outvar);
}

/*
 * awka_ltrim
 * awk extended builtin function 'ltrim'
 */
a_VAR *
awka_ltrim(char keep, a_VARARG *va)
{
  a_VAR *outvar;
  register char *p, *r, *q;

  /* create a variable */
  _awka_getstringvar;

  /* compute length of substring */
  awka_strcpy(outvar, awka_gets1(va->var[0]));
  p = outvar->ptr;

  if (va->var[0]->slen)
  {
    if (va->used == 2)
    {
      r = awka_gets1(va->var[1]);

      while (*p)
      {
        for (q=r; *q; q++)
          if (*p == *q)
            break;

        if (*q)
          p++;
        else
          break;
      }
    }
    else
    {
      /* get rid of preceding whitespace */
      while (*p)
      {
        if (isspace(*p))
          p++;
        else
          break;
      }
    }
  }

  if (p > outvar->ptr)
  {
    outvar->slen -= (p - outvar->ptr);
    memmove(outvar->ptr, p, outvar->slen + 1);
  }

  /* get outa here */
  return (outvar);
}

/*
 * awka_rtrim
 * awk extended builtin function 'rtrim'
 */
a_VAR *
awka_rtrim(char keep, a_VARARG *va)
{
  a_VAR *outvar;
  register char *p, *r, *q;

  /* create a variable */
  _awka_getstringvar;

  /* compute length of substring */
  awka_strcpy(outvar, awka_gets1(va->var[0]));
  p = outvar->ptr + outvar->slen - 1;

  if (outvar->slen)
  {
    if (va->used == 2)
    {
      r = awka_gets1(va->var[1]);

      while (p > outvar->ptr)
      {
        for (q=r; *q; q++)
          if (*p == *q)
            break;

        if (*q)
        {
          *p-- = '\0';
          outvar->slen--;
        }
        else
          break;
      }
    }
    else
    {
      /* remove trailing whitespace */
      while (p > outvar->ptr)
      {
        if (!(isspace(*p)))
          break;

        *p-- = '\0';
        outvar->slen--;
      }
    }
  }

  /* get outa here */
  return (outvar);
}

/*
 * awka_rand
 * awk 'rand' builtin function
 *
 * To avoid bad system implementations of rand(), I have used a
 * derivative of Park and Miller's Minimal Standard generator,
 * described in CACM, vol 31 (1988), pp 1192-1201.
 */

#if LONG_BIT < 64
typedef long randint;
#else
typedef int randint;
#endif

static randint _a_seed = 1;

#define _aQ   127773
#define _aA   16807
#define _aIM  2147483647
#define _aAM  (1.0/_aIM)
#define _aR   2836
#define _aMK  123459876

double
awka_rand()
{
  double ret;
  register randint c;

  /* get the random number */
  _a_seed ^= _aMK;
  c = _a_seed / _aQ;
  _a_seed = _aA * (_a_seed - c * _aQ) - _aR * c;

  if (_a_seed < 0)
    _a_seed += _aIM;

  ret = _aAM * _a_seed;
  _a_seed ^= _aMK;

  return ret;
}

/*
 * awka_srand
 * awk builtin 'srand' function
 */
a_VAR *
awka_srand( char keep, a_VARARG *va )
{
  a_VAR *outvar;

  _awka_checkbiargs( va, "awka_srand", _BI_SRAND );
  _awka_getdoublevar;

  if (va->used == 0)
    _a_seed = time((time_t *) 0);
  else
    _a_seed = (randint) awka_getd1(va->var[0]);

  while (_a_seed == _aMK)
  {
    /* cant have it equalling mask value - use a
       time call, again if necessary */
    _a_seed = time((time_t *) 0);
  }

  outvar->dval = (double) _a_seed;

  return (outvar);
}

/*
 * awka_left
 * awk builtin extended function 'left'
 */
a_VAR *
awka_left(char keep, a_VAR *va, a_VAR *vb)
{
  a_VAR *outvar;
  register char *ptr;

  /* check argument list */
  if (awka_getd1(vb) < 1)
    awka_error("runtime error: Second Argument must be >= 1 in call to Left, got %d.\n",(int) vb->dval);

  /* create a variable */
  _awka_getstringvar;

  /* compute length of substring */
  ptr = awka_gets1(va);

  if (va->slen <= vb->dval)
  {
    /* return the full string */
    awka_strcpy(outvar, ptr);
  }
  else
  {
    awka_setstrlen(outvar, vb->dval);
    memcpy(outvar->ptr, ptr, outvar->slen);
    outvar->ptr[outvar->slen] = '\0';
  }

  /* get outa here */
  return (outvar);
}

/*
 * awka_right
 * awk builtin extended function 'right'
 */
a_VAR *
awka_right(char keep, a_VAR *va, a_VAR *vb)
{
  a_VAR *outvar;
  register char *ptr;

  /* check argument list */
  if (awka_getd1(vb) < 1)
    awka_error("runtime error: Second Argument must be >= 1 in call to Right, got %d.\n",(int) vb->dval);

  /* create a variable */
  _awka_getstringvar;

  /* compute length of substring */
  ptr = awka_gets1(va);

  if (va->slen <= vb->dval)
  {
    /* return the full string */
    awka_strcpy(outvar, ptr);
  }
  else
  {
    awka_setstrlen(outvar, vb->dval);
    memcpy(outvar->ptr, ptr + (va->slen - outvar->slen), outvar->slen);
    outvar->ptr[outvar->slen] = '\0';
  }

  /* get outa here */
  return (outvar);
}

/*
 * awka_ascii
 * awk builtin extended function 'ascii'
 */
a_VAR *
awka_ascii(char keep, a_VARARG *va)
{
  a_VAR *outvar;
  register int i;
  register char *ptr;

  /* check argument list */
  _awka_checkbiargs( va, "awka_ascii", _BI_ASCII );

  if (va->used == 2)
    if (awka_getd1(va->var[1]) < 0)
      awka_error("runtime error: Second Argument must be >= 0 in call to Ascii, got %d.\n",(int) va->var[1]->dval);

  /* create a variable */
  _awka_getdoublevar;

  ptr = awka_gets1(va->var[0]);

  if (va->used == 2)
    i = A_MIN(va->var[0]->slen, va->var[1]->dval) - 1;
  else
    i = 0;

  outvar->dval = (double) ptr[i];

  /* get outa here */
  return (outvar);
}

/*
 * awka_char
 * awk builtin extended function 'char'
 */
a_VAR *
awka_char(char keep, a_VAR *va)
{
  a_VAR *outvar;

  /* create a variable */
  _awka_getstringvar;

  if (!outvar->ptr)
    outvar->allc = malloc( &outvar->ptr, 2 );
  else if (outvar->allc <= 1)
    outvar->allc = realloc( &outvar->ptr, 2 );

  outvar->ptr[0] = (char) ((int) awka_getd1(va) & 255);
  outvar->ptr[1] = '\0';
  outvar->slen = 1;

  /* get outa here */
  return (outvar);
}

/*
 * _awka_calctime
 * Calculates julian time from separate variables
 */
static time_t
_awka_calctime( a_VARARG *va )
{
  register int i;
  struct tm tme;

  tme.tm_isdst = tme.tm_year = tme.tm_mon = tme.tm_mday = tme.tm_hour = tme.tm_min = tme.tm_sec = 0;

  for (i=0; i<va->used; i++)
  {
    switch (i)
    {
      case 0:
        /* year */
        tme.tm_year = (int) awka_getd1(va->var[i]);

        if (tme.tm_year >= 1900)
          tme.tm_year -= 1900;
        else if (tme.tm_year > 136 || tme.tm_year < 0)
          tme.tm_year = 0;
        break;

      case 1:
        /* month */
        tme.tm_mon = (int) awka_getd1(va->var[i]);

        if (tme.tm_mon > 0) tme.tm_mon--;
        break;

      case 2:
        /* day */
        tme.tm_mday = (int) awka_getd1(va->var[i]);
        break;

      case 3:
        /* hour */
        tme.tm_hour = (int) awka_getd1(va->var[i]);

        if (tme.tm_hour > 0) tme.tm_hour--;
        break;

      case 4:
        /* minute */
        tme.tm_min = (int) awka_getd1(va->var[i]);
        break;

      case 5:
        /* seconds */
        tme.tm_sec = (int) awka_getd1(va->var[i]);
        break;
    }
  }

  return mktime(&tme);
}

/*
 * awka_time
 * awk extended function 'time'
 */
a_VAR *
awka_time( char keep, a_VARARG *va )
{
  a_VAR *outvar;
  time_t tt;

  _awka_checkbiargs( va, "awka_time", _BI_TIME );
  _awka_getdoublevar;

  if (va->used == 0)
    tt = time(NULL);
  else
  {
    tt = _awka_calctime(va);
    if (tt == -1) tt = 0;
  }

  outvar->dval = tt;
  return (outvar);
}

/*
 * awka_systime
 * gawk function systime
 */
a_VAR *
awka_systime( char keep )
{
  a_VAR *outvar;

  _awka_getdoublevar;

  outvar->dval = time(NULL);
  return (outvar);
}

/*
 * awka_localtime
 * awk extended function 'localtime'
 */
a_VAR *
awka_localtime( char keep, a_VARARG *va )
{
  a_VAR *outvar;
  time_t tt;
  char *p;
  register int i;

  _awka_checkbiargs( va, "awka_localtime", _BI_LOCALTIME );
  _awka_getstringvar;

  if (va->used == 0)
    tt = time(NULL);
  else
  {
    tt = (time_t) awka_getd1(va->var[0]);
    if (tt < 0) tt = 0;
  }

  p = asctime(localtime(&tt));
  i = strlen(p);

  if (p[i-1] == '\n')
    p[--i] = '\0';

  awka_strcpy(outvar, p);

  return (outvar);
}

/*
 * awka_gmtime
 * awk extended function 'gmtime'
 */
a_VAR *
awka_gmtime( char keep, a_VARARG *va )
{
  a_VAR *outvar;
  time_t tt;
  char *p;
  register int i;

  _awka_checkbiargs( va, "awka_gmtime", _BI_GMTIME );
  _awka_getstringvar;

  if (va->used == 0)
    tt = time(NULL);
  else
  {
    tt = (time_t) awka_getd1(va->var[0]);
    if (tt < 0) tt = 0;
  }

  p = asctime(gmtime(&tt));
  i = strlen(p);

  if (p[i-1] == '\n')
    p[--i] = '\0';

  if (!outvar->ptr)
    outvar->allc = malloc( &outvar->ptr, i+1 );
  else if (i >= outvar->allc)
    outvar->allc = realloc( &outvar->ptr, i+1 );

  memcpy(outvar->ptr, p, i+1);
  outvar->slen = i;

  return (outvar);
}

/*
 * awka_mktime
 * gawk function 'mktime' - turn a time string into a timestamp
 */
a_VAR *
awka_mktime( char keep, a_VARARG *va )
{
  a_VAR *outvar;
  struct tm the_time;
  long year;
  int month, day, hour, min, sec, dst = -1;
  int count;

  _awka_checkbiargs( va, "awka_mktime", _BI_MKTIME );
  _awka_getdoublevar;
  outvar->dval = -1.0;

  if (va->used < 1)
    return outvar;

  count = sscanf( awka_gets(va->var[0]),
                  "%ld %d %d %d %d %d %d",
                  &year, &month, &day,
                  &hour, &min, &sec, &dst );

  if (count < 6)
    return outvar;

  memset( &the_time, 0, sizeof(the_time) );
  the_time.tm_year  = year - 1900;
  the_time.tm_mon   = month - 1;
  the_time.tm_mday  = day;
  the_time.tm_hour  = hour;
  the_time.tm_min   = min;
  the_time.tm_sec   = sec;
  the_time.tm_isdst = dst;

  outvar->dval = (double) mktime( &the_time );

  return outvar;
}

/*
 * awka_strftime
 * gawk function 'strftime'
 */
a_VAR *
awka_strftime( char keep, a_VARARG *va )
{
  a_VAR *outvar;
  time_t tt;
  struct tm *tme;
  char *p, *fmt, buf[4096];
  static char def_fmt[] = "%a %b %d %H:%M:%S %Z %Y";
  register int fmtlen, bufallc = 4096, buflen;

  _awka_checkbiargs( va, "awka_strftime", _BI_STRFTIME );
  _awka_getstringvar;

  if (va->used < 2)
    tt = time(NULL);
  else
  {
    tt = (time_t) awka_getd1(va->var[1]);
    if (tt < 0) tt = 0;
  }

  if (va->used >= 1)
  {
    /* user-defined format */
    fmt = awka_gets1(va->var[0]);
    fmtlen = va->var[0]->slen;

    if (fmtlen == 0)
    {
      awka_strcpy(outvar, "");

      return (outvar);
    }
  }
  else
  {
    fmt = def_fmt;
    fmtlen = strlen(fmt);
  }

  tme = localtime(&tt);
  p = buf;

  while (1)
  {
    *p = '\0';
    buflen = strftime(p, bufallc, fmt, tme);

    if (buflen > 0 || bufallc >= 1024 * fmtlen)
      break;

    bufallc *= 2;

    if (p == buf)
      malloc( &p, bufallc);
    else
      realloc( &p, bufallc);
  }

  awka_strcpy(outvar, p);

  if (p != buf) free(p);

  return (outvar);
}

/*
 * awka_min
 * awk builtin (extended) function 'min'
 */
a_VAR *
awka_min(char keep, a_VARARG *va)
{
  a_VAR *outvar;
  register int i;

  /* check argument list */
  _awka_checkbiargs( va, "awka_min", _BI_MIN );

  /* create a variable */
  _awka_getdoublevar;

  /* find the smallest number */
  outvar->dval = awka_getd1(va->var[0]);

  for (i=1; i<va->used; i++)
    outvar->dval = (outvar->dval < awka_getd1(va->var[i])) ? outvar->dval : va->var[i]->dval;

  /* get outa here */
  return (outvar);
}

/*
 * awka_max
 * awk builtin (extended) function 'max'
 */
a_VAR *
awka_max(char keep, a_VARARG *va)
{
  a_VAR *outvar;
  register int i;

  /* check argument list */
  _awka_checkbiargs( va, "awka_max", _BI_MAX );

  /* create a variable */
  _awka_getdoublevar;

  /* find the smallest number */
  outvar->dval = awka_getd1(va->var[0]);

  for (i=1; i<va->used; i++)
    outvar->dval = (outvar->dval > awka_getd1(va->var[i])) ? outvar->dval : va->var[i]->dval;

  /* get outa here */
  return (outvar);
}

/*
 * _awka_formatstr
 * does the dirty work for printf & sprintf
 *
 * for sprintf, which == 0
 * for printf which == id of stream + 1.
 */
#define _a_fs_morefmtbuf(i) fmtallc = realloc(&fmtbuf, (i)*2)

#define _a_fs_checkbuf(len) \
  if ((bp + len) - buf >= bufallc) { \
    tlen = bp - buf; \
    bufallc = realloc( &buf, ((bp + len) - buf) + 1 ); \
    bp = buf + tlen; \
  }

#define _a_FSTYPE_SINT   1
#define _a_FSTYPE_UINT   2
#define _a_FSTYPE_FLOAT  3
#define _a_FSTYPE_CHAR   4
#define _a_FSTYPE_STRING 5

char *
_awka_formatstr(char which, a_VARARG *va)
{
  register char *cp, *bp, *lcp, char_arg = 0, arg_type = 0, *caller, *p1, *p2;
  register int i = 0, cur_arg = 0, done, sint_arg = 0, tlen;
  static char *buf = NULL, *fmtbuf = NULL, *str_arg, tmp[512], *cur_str = NULL;
  static int fmtallc = 0, cur_allc = 0, bufallc = 0;
  unsigned int uint_arg = 0;
  double dbl_arg = 0.0;

  if (!buf)
  {
    bufallc = malloc(&buf, _a_SPRINTF_BUFFER);
    fmtallc = malloc(&fmtbuf, 128);
  }

  if (which)
    caller = "printf";
  else
    caller = "sprintf";

  p1 = awka_gets1(va->var[cur_arg]);

  if (!cur_str)
    cur_allc = malloc( &cur_str, va->var[cur_arg]->slen + 100 );
  else if (va->var[cur_arg]->slen + 100 > cur_allc)
    cur_allc = realloc( &cur_str, va->var[cur_arg]->slen + 100 );

  strcpy(cur_str, p1);
  p1 = cp = lcp = cur_str;
  bp = buf;

  /* format & output string */
  while (*cp)
  {
    while (*cp && *cp != '%')
    {
      /* normal character in format string */
      cp++;
    }

    /*
    cp = strchr(cp, '%');
    */

    if (!*cp)
      break;

    /* if (!cp) break; */
    if (*(++cp) == '%')
    {
      cp++;
      lcp = cp;

      if (which)
        fputs("%", _a_iostream[which-1].fp);
      else
      {
        _a_fs_checkbuf(2);
        strcpy(bp, "%");
        bp++;
      }
      continue;
    }

    /* we have a '%' format marker */
    cur_arg++;

    if (cur_arg >= va->used)
      awka_error("%s: missing argument %d.\n",caller,cur_arg);

    /* swallow format options */
    done = 0;

    while (!done)
    {
      switch (*cp)
      {
        case '\0':
          awka_error("%s: incomplete symbol after %% specifier %d.\n",caller,cur_arg);
          break;

        case '-':  /* left alignment */
        case '+':  /* decimal +- */
        case ' ':  /* leading blank space */
        case '#':  /* increase precision */
        case '0':  /* leading zeros */
        case '.':  /* decimal point */
          break;

        case '*':  /* literal insertion of number parameter */
          sprintf(tmp, "%d%s", (int) awka_getd(va->var[cur_arg++]), cp+1);
          p1 = cp; p2 = tmp;

          while (*p2 != '\0')
            *p1++ = *p2++;

          *p1 = *p2;
          break;

        default:
          done = 1;
      }

      if (!done)
        cp++;
    }

    if (!*cp)
      awka_error("%s: incomplete symbol after %%, specifier %d.\n",caller,cur_arg);

    /* swallow minimum width specification */
    while (isdigit(*cp))
      cp++;

    if (*cp == '.')  /* precision */
    {
      if (*(++cp) == '\0')
        awka_error("%s: incomplete symbol after %%, specifier %d.\n",caller,cur_arg);
      else
        while (isdigit(*cp)) cp++;
    }

    /* format specifier - prepare argument for use */
    switch (*cp++)
    {
      case '\0':
        awka_error("%s: incomplete symbol after %%, specifier %d.\n",caller,cur_arg);
        break;

      case 'c':
        arg_type = _a_FSTYPE_CHAR;

        if (va->var[cur_arg]->type == a_VARSTR || va->var[cur_arg]->type == a_VARUNK)
        {
          i = atoi(va->var[cur_arg]->ptr);
          sprintf(tmp, "%d", i);

          if (i < 128 && i >= 0 && !strcmp(tmp, va->var[cur_arg]->ptr))
            char_arg = (char) i;
          else
            char_arg = va->var[cur_arg]->ptr[0];
        }
        else if (va->var[cur_arg]->type == a_VARDBL)
          char_arg = (char) ((int) va->var[cur_arg]->dval);
        else
          char_arg = *(awka_gets1(va->var[cur_arg]));
        break;

      case 'd':
      case 'i':
      case 'o':
      case 'x':
      case 'X':
        arg_type = _a_FSTYPE_SINT;
        sint_arg = (int) awka_getd1(va->var[cur_arg]);
        break;

      case 'u':
        arg_type = _a_FSTYPE_UINT;
        uint_arg = (unsigned int) awka_getd1(va->var[cur_arg]);
        break;

      case 'e':
      case 'E':
      case 'f':
      case 'g':
      case 'G':
        arg_type = _a_FSTYPE_FLOAT;
        dbl_arg = awka_getd1(va->var[cur_arg]);
        break;

      case 's':
        arg_type = _a_FSTYPE_STRING;
        str_arg = awka_gets1(va->var[cur_arg]);
        break;

      default:
        awka_error("%s: unknown format specification (%d) '%s'.\n",caller,cur_arg,awka_gets1(va->var[cur_arg]));
    }

    /* prepare format string */
    if (cp - lcp >= fmtallc - 1)
    {
      _a_fs_morefmtbuf(cp - lcp);
    }

    memcpy(fmtbuf, lcp, cp - lcp);
    fmtbuf[cp - lcp] = '\0';

    if (!which)
    {
      if (arg_type == _a_FSTYPE_STRING)
        i = (cp - lcp) + strlen(str_arg) + 1;
      else
        i = (cp - lcp) + 30;

      _a_fs_checkbuf(i);
    }


    switch (arg_type)
    {
      case _a_FSTYPE_SINT:
        if (which)
          fprintf(_a_iostream[which-1].fp, fmtbuf, sint_arg);
        else
          sprintf(bp, fmtbuf, sint_arg);
        break;

      case _a_FSTYPE_UINT:
        if (which)
          fprintf(_a_iostream[which-1].fp, fmtbuf, uint_arg);
        else
          sprintf(bp, fmtbuf, uint_arg);
        break;

      case _a_FSTYPE_FLOAT:
        if (which)
          fprintf(_a_iostream[which-1].fp, fmtbuf, dbl_arg);
        else
          sprintf(bp, fmtbuf, dbl_arg);
        break;

      case _a_FSTYPE_CHAR:
        if (which)
          fprintf(_a_iostream[which-1].fp, fmtbuf, char_arg);
        else
          sprintf(bp, fmtbuf, char_arg);
        break;

      case _a_FSTYPE_STRING:
        if (which)
          fprintf(_a_iostream[which-1].fp, fmtbuf, str_arg);
        else
          sprintf(bp, fmtbuf, str_arg);
        break;
    }


    if (!which)
    {
      i = strlen(bp);
      bp += i;
    }

    lcp = cp;
  }

  if (cp > lcp)
  {
    if (which)
      fputs(lcp, _a_iostream[which-1].fp);
    else
    {
      _a_fs_checkbuf( (cp - lcp) + 1 );
      strcpy(bp, lcp);
    }
  }

  return buf;
}

/*
 * awka_sprintf
 * awk builtin 'sprintf' function
 */
a_VAR *
awka_sprintf( char keep, a_VARARG *va )
{
  a_VAR *outvar;
  char *p;
  register int i;

  _awka_checkbiargs( va, "awka_sprintf", _BI_SPRINTF );
  _awka_getstringvar;

  p = _awka_formatstr(0, va);
  i = strlen(p);

  if (!outvar->ptr)
    outvar->allc = malloc( &outvar->ptr, i+1 );
  else if (i >= outvar->allc)
    outvar->allc = realloc( &outvar->ptr, i+1 );

  memcpy(outvar->ptr, p, i+1);
  outvar->slen = i;

  return (outvar);
}

/*
 * awka_printf
 * awk builtin 'printf' function
 */
void
awka_printf( char *output, int stream, int pipe, a_VARARG *va )
{
  register int i;
  char flag = _a_IO_WRITE;

  _awka_checkbiargs( va, "awka_printf", _BI_PRINTF );

  if (pipe == -1)
  {
    flag = _a_IO_APPEND;
    pipe = 0;
  }

  if (output)
  {
    for (i=0; i<_a_ioused; i++)
      if ((_a_iostream[i].io & _a_IO_WRITE ||
           _a_iostream[i].io == _a_IO_APPEND) &&
          _a_iostream[i].pipe == pipe &&
          !strcmp(_a_iostream[i].name, output))
        break;

    if (i == _a_ioused)
       i = _awka_io_addstream( output, flag, pipe );
  }
  else
    i = stream;

  if (_a_iostream[i].io == _a_IO_READ + _a_IO_WRITE &&
      _a_iostream[i].fp &&
      _a_iostream[i].lastmode != _a_IO_WRITE)
  {
    fflush(_a_iostream[i].fp);
    _a_iostream[i].lastmode = _a_IO_WRITE;
  }

  _awka_formatstr(i+1, va);
}

a_VAR *
_awka_print_concat( a_VARARG *va )
{
  register int len, oldlen, i=1, alloc, ofs_len;
  a_VAR *outvar;
  register char *ptr, *op, *ofs, keep = FALSE;

  ofs = awka_gets1(a_bivar[a_OFS]);
  ofs_len = a_bivar[a_OFS]->slen;

  _awka_getstringvar;

  ptr = awka_gets1P(va->var[0]);
  alloc = (va->var[0]->slen + ofs_len) * (va->used-1) + 1;

  if (!outvar->ptr)
    alloc = malloc( &outvar->ptr, alloc );
  else if (outvar->allc < alloc)
    alloc = realloc( &outvar->ptr, alloc );
  else
    alloc = outvar->allc;

  oldlen = len = va->var[0]->slen;
  memcpy(outvar->ptr, ptr, len+1);
  op = outvar->ptr + len;

  for (i=1; i<va->used; i++)
  {
    oldlen = len;
    ptr = awka_gets1P(va->var[i]);
    len += va->var[i]->slen + ofs_len;

    if (len >= alloc)
    {
      alloc = realloc( &outvar->ptr, alloc + len + ((va->used - i - 1) * 20) );
      op = outvar->ptr + oldlen;
    }

    if (ofs_len == 1)
      *op = *ofs;
    else
      memcpy(op, ofs, ofs_len);

    op += ofs_len;
    memcpy(op, ptr, va->var[i]->slen+1);
    op += va->var[i]->slen;
  }

  outvar->slen = len;
  outvar->allc = alloc;

  return (outvar);
}

/*
 * awka_print
 * awk builtin 'print' function
 */
void
awka_print( char *output, int stream, int pipe, a_VARARG *va )
{
  register int i, j;
  register char flag = _a_IO_WRITE;
  a_VAR *var;

  _awka_checkbiargs( va, "awka_print", _BI_PRINT );

  if (pipe == -1)
  {
    flag = _a_IO_APPEND;
    pipe = 0;
  }

  if (output)
  {
    for (i=0; i<_a_ioused; i++)
      if ((_a_iostream[i].io & _a_IO_WRITE ||
           _a_iostream[i].io == _a_IO_APPEND) &&
          _a_iostream[i].pipe == pipe &&
          !strcmp(_a_iostream[i].name, output))
        break;

    if (i == _a_ioused)
       i = _awka_io_addstream( output, flag, pipe );
  }
  else
    i = stream;

  if (_a_iostream[i].io == _a_IO_READ + _a_IO_WRITE &&
      _a_iostream[i].fp &&
      _a_iostream[i].lastmode != _a_IO_WRITE)
  {
    fflush(_a_iostream[i].fp);
    _a_iostream[i].lastmode = _a_IO_WRITE;
  }

  if (va->used > 1)
  {
    var = _awka_print_concat(va);
    fwrite(awka_gets(var), 1, var->slen, _a_iostream[i].fp);
  }
  else
  {
    if (va->var[0]->type == a_VARDBL)
    {
      if ((double) (j = (int) va->var[0]->dval) == va->var[0]->dval)
        fprintf(_a_iostream[i].fp, "%d", j);
      else
        fprintf(_a_iostream[i].fp, awka_gets(a_bivar[a_OFMT]), va->var[0]->dval);
    }
    else
    {
      awka_gets(va->var[0]);

      if (va->var[0]->slen == 1)
        putc( va->var[0]->ptr[0], _a_iostream[i].fp );
      else
        fwrite(va->var[0]->ptr, 1, va->var[0]->slen, _a_iostream[i].fp);
    }
  }

  awka_gets(a_bivar[a_ORS]);

  if (a_bivar[a_ORS]->slen == 1)
    putc( a_bivar[a_ORS]->ptr[0], _a_iostream[i].fp );
  else
    fwrite(a_bivar[a_ORS]->ptr, 1, a_bivar[a_ORS]->slen, _a_iostream[i].fp);
}

int
awka_setvarbyname( char *name, char *value )
{
  int i = 0;
  extern struct gvar_struct *_gvar;

  while (_gvar[i].name)
  {
    if (!strcmp(_gvar[i].name, name))
      break;

    i++;
  }

  if (!_gvar[i].name)
    return 0;   // not found
  else if (_gvar[i].var->type == a_VARARR)
    return -1;  // is array

  awka_strcpy(_gvar[i].var, value);
  _gvar[i].var->type = a_VARUNK;

  return 1;
}

/*
 * awka_getline
 * awk builtin 'getline' function
 */
a_VAR *
awka_getline( char keep, a_VAR *target, char *input, int pipe, char main )
{
  a_VAR *outvar;
  register int i = 0, fill_target = 1, new_file = FALSE;
  static int mlen = 100, from_filelist = TRUE, stream = -1;
  static char *file = NULL; //, *mfile = NULL;

  if (!file)
  {
    malloc( &file, mlen);
    file[0] = '\0';
    //malloc( &mfile, mlen);
    //mfile[0] = '\0';
    awka_setd(a_bivar[a_NR]) = 0;
  }

  _awka_getdoublevar;

  if (_awka_arg_change == TRUE)
    awka_parsecmdline(0);
  _awka_arg_change = FALSE;

  awka_forcestr(target);

  if (target == a_bivar[a_DOL0])
  {
    fill_target = _dol0_used;

    if (!main)
    {
      _rebuild0 = FALSE;
      _rebuildn = TRUE;
    }
  }

start:
  /* check to see if input filename is different from the last
     one we read from - if so, read a filename from the main
     filelist if necessary, and set file variable */
  if (main || !*input)
  {
    if (_awka_file_read == TRUE || !*input || strcmp(file, input))
    {
      /* reading from a file off the argument list */
      if (from_filelist == TRUE || _awka_curfile == -1)
      {
        _awka_curfile++;
        awka_setd(a_bivar[a_ARGIND]) = _awka_fileoffset + _awka_curfile;
      }

      if (_awka_curfile < awka_filein_no)
      {
        awka_setd(a_bivar[a_FNR]) = 0;

        do {
          char *equals;
          if (NULL != (equals = strchr(awka_filein[_awka_curfile], '=')))
          {
            /* var=value on commandline support (without -v) */
            *equals = '\0';
            if (awka_setvarbyname(awka_filein[_awka_curfile], equals+1)>0)
            {
              _awka_curfile++;
              awka_setd(a_bivar[a_ARGIND]) = _awka_fileoffset + _awka_curfile;
              continue;
            }
            else
              *equals = '=';
          }

          if (strlen(awka_filein[_awka_curfile]) >= mlen)
          {
            mlen = strlen(awka_filein[_awka_curfile])+1;
            realloc(&file, mlen);
          }

          strcpy(file, awka_filein[_awka_curfile]);
          awka_strcpy(a_bivar[a_FILENAME], file);
          new_file = TRUE;
          break;

        } while (_awka_curfile < awka_filein_no);
      }
      else
        file[0] = '\0';

      from_filelist = TRUE;
    }
  }
  else if (*file != *input || strcmp(file, input))
  {
    /* file specified */
    if ((i = strlen(input)) >= mlen)
    {
      mlen = i + 1;
      realloc(&file, mlen);
    }

    strcpy(file, input);
    from_filelist = FALSE;
    new_file = TRUE;
  }

  _awka_file_read = FALSE;

  /* find the input stream for this filename */
  if (file[0] && (new_file == TRUE || stream == -1))
  {
    for (i=0; i<_a_ioused; i++)
    {
      if ((_a_iostream[i].io & _a_IO_READ || _a_iostream[i].io == _a_IO_CLOSED || _a_iostream[i].io == _a_IO_EOF) &&
          *(_a_iostream[i].name) == *file && !strcmp(_a_iostream[i].name, file) &&
          (int) _a_iostream[i].pipe == pipe)
        break;
    }

    if (i == _a_ioused)
    {
      i = _awka_io_addstream(file, _a_IO_READ, pipe);

      if (_a_iostream[i].io == _a_IO_CLOSED)
      {
        /* error opening file */
        outvar->dval = -1;

        if (main == TRUE)
          awka_error("error reading from file \"%s\".\n",file);

        _a_iostream[i].name[0] = '\0';    // ensure reading the same file again results in the same failure to read
        goto getline_end;
      }
    }

    stream = i;
    _awka_file_stream = stream;  // track current stream globally. For seek, tell, etc.
  }

  /* read a line */
  if (stream < _a_ioused && file[0])
  {
    outvar->dval = (double) awka_io_readline(target, stream, fill_target);

    if (!outvar->dval)
    {
      /* failed to read - end of file */
      if (main)
      {
        _awka_file_read = TRUE;
        awka_fclose(stream);

        goto start;
      }

      _a_iostream[stream].io = _a_IO_EOF;
    }

    if (main)
    {
      awka_setd(a_bivar[a_FNR])++;
      awka_setd(a_bivar[a_NR])++;
    }
  }

  getline_end:
  target->type = a_VARUNK;

  return (outvar);
}

int
awka_getline_main()
{
  return awka_getline(
               a_TEMP,
               awka_dol0(a_DOL_GET),
               awka_gets(a_bivar[a_FILENAME]),
	       FALSE,
	       TRUE
	 )->dval > 0.0
         && awka_setNF(0);
}

a_VAR *
awka_fseek(char keep, a_VARARG *va )
{
  a_VAR *outvar;
  register int i;
  register char *ptr;

  _awka_getdoublevar;
  outvar->dval = -1;

  // XXX: use a_bivar[a_FILENAME] ?

  long offset = 0;
  int  origin = 0;

  if (va->used==1)
  {
    ptr = awka_gets1( a_bivar[a_FILENAME] );
    offset = (long) awka_getd( va->var[0] );
    origin = (offset > 0) ? SEEK_SET : SEEK_END;
  }
  else if (va->used==2)
  {
    ptr = awka_gets1( va->var[0] );
    offset = (long) awka_getd( va->var[1] );
    origin = (offset > 0) ? SEEK_SET : SEEK_END;
  }
  else if (va->used==3)
  {
    // TODO: use string to int conversion
    ptr = awka_gets1( va->var[0] );
    offset = (long) awka_getd( va->var[1] );
    origin = (int) awka_getd( va->var[2] );
    origin = SEEK_CUR;
  }

  /* fseek specific stream */
  for (i=0; i<_a_ioused; i++) {
    if (!strcmp(_a_iostream[i].name, ptr) && _a_iostream[i].io != _a_IO_CLOSED)
    {
      fseek(_a_iostream[i].fp, offset, origin);
      outvar->dval = 0;

      _a_iostream[i].current = _a_iostream[i].end = _a_iostream[i].buf ;

      break;
    }
  }

  return outvar;
}

a_VAR *
awka_ftell(char keep, a_VARARG *va )
{
  a_VAR *outvar;
  register int i;
  register char *ptr;

  _awka_getdoublevar;
  outvar->dval = -1;

  // XXX: use a_bivar[a_FILENAME] ?
  if (va->used == 0)
    ptr = awka_gets1(a_bivar[a_FILENAME]);
  else
    ptr = awka_gets1(va->var[0]);

  // i = _awka_file_stream;
  // if (!strcmp(_a_iostream[i].name, ptr) && _a_iostream[i].io != _a_IO_CLOSED)
  // {
  //     outvar->dval = ftell(_a_iostream[i].fp);
  // }

  /***************************************************************************
   * XXX: For performance reason, use abvoe code to avoid loop.
   *      Considering support of FILENAME, and low usage frequency, use below
   **************************************************************************/

  /* fseek specific stream */
  for (i=0; i<_a_ioused; i++) {
    if (!strcmp(_a_iostream[i].name, ptr) && _a_iostream[i].io != _a_IO_CLOSED)
    {
      outvar->dval = ftell(_a_iostream[i].fp);
      break;
    }
  }

  return outvar;
}

a_VAR *
awka_fsize(char keep, a_VARARG *va )
{
  a_VAR *outvar;
  register int i;
  register char *ptr;

  _awka_getdoublevar;
  outvar->dval = -1;

  if (va->used == 0)
    ptr = awka_gets1(a_bivar[a_FILENAME]);
  else
    ptr = awka_gets1(va->var[0]);


  /* fseek specific stream */
  for (i=0; i<_a_ioused; i++) {
    if (!strcmp(_a_iostream[i].name, ptr) && _a_iostream[i].io != _a_IO_CLOSED)
    {
      // TODO: use stat
      long pos = ftell(_a_iostream[i].fp);
      fseek(_a_iostream[i].fp, 0, SEEK_END);
      outvar->dval = ftell(_a_iostream[i].fp);
      fseek(_a_iostream[i].fp, pos, SEEK_SET);
      break;
    }
  }

  return outvar;
}

/*
 * awka_fflush
 * awk 'fflush' function
 */
a_VAR *
awka_fflush( char keep, a_VARARG *va )
{
  a_VAR *outvar;
  register int i;
  register char *ptr;

  _awka_checkbiargs( va, "awka_fflush", _BI_FFLUSH );
  _awka_getdoublevar;
  outvar->dval = 0;

  if (va->used)
  {
    outvar->dval = -1;
    ptr = awka_gets1(va->var[0]);

    if (ptr[0] == '\0')
    {
      /* flush all open streams */
      outvar->dval = 0;

      for (i=0; i<_a_ioused; i++)
        if (_a_iostream[i].io != _a_IO_CLOSED)
          fflush(_a_iostream[i].fp);
    }
    else
    {
      /* flush specific stream */
      for (i=0; i<_a_ioused; i++)
        if (!strcmp(_a_iostream[i].name, ptr) && _a_iostream[i].io != _a_IO_CLOSED)
        {
          fflush(_a_iostream[i].fp);
          outvar->dval = 0;
        }
    }
  }
  else
  {
    /* flush /dev/stdout */
    for (i=0; i<_a_ioused; i++)
      if (!strcmp(_a_iostream[i].name, "/dev/stdout"))
        fflush(_a_iostream[i].fp);
  }

  return (outvar);
}

/*
 * awka_close
 * awk 'close' function
 */
a_VAR *
awka_close( char keep, a_VARARG *va )
{
  a_VAR *outvar;
  register int i;
  register char *ptr;

  _awka_checkbiargs( va, "awka_close", _BI_CLOSE );
  _awka_getdoublevar;
  outvar->dval = -1;

  ptr = awka_gets1(va->var[0]);

  for (i=0; i<_a_ioused; i++)
    if (!strcmp(_a_iostream[i].name, ptr) &&
        _a_iostream[i].io & _a_IO_READ)
      break;

  if (i == _a_ioused)
    for (i=0; i<_a_ioused; i++)
      if (!strcmp(_a_iostream[i].name, ptr))
        break;

  if (i < _a_ioused)
    outvar->dval = (double) awka_fclose(i);

  return outvar;
}

int
awka_fclose( int i )
{
  int ret = -1, j;

  if (i >= _a_ioused){
    return ret;
  }

  _a_IOSTREAM *s = &_a_iostream[i];

  if (i < _a_ioused)
  {
    if (_a_iostream[i].io != _a_IO_CLOSED)
    {
      if (_a_iostream[i].fp)
      {
        fflush(_a_iostream[i].fp);

        if (_a_iostream[i].pipe == 1)
	{
          ret = pclose(_a_iostream[i].fp);
        }
	else if (_a_iostream[i].pipe == 2)
	{
          /* two-way process */
          switch (s->type)
	  {
            case AWKA_STREAM_SOCKET:
              fclose(_a_iostream[i].fp);
              break;

            default:
              awka_error("runtime error: Unsupported close of pipe = 2.\n");
          }
        }
	else
	{
          if (strcmp(_a_iostream[i].name, "/dev/stdout") &&
              strcmp(_a_iostream[i].name, "/dev/stderr"))
            fclose(_a_iostream[i].fp);
          ret = 0;
        }
      }

      if (_a_iostream[i].io & _a_IO_READ)
      {
        for (j=(_awka_curfile >= 0 ? _awka_curfile : 0); j<awka_filein_no; j++)
        {
          if (!strcmp(_a_iostream[i].name, awka_filein[j]))
            break;
        }

        if (j < awka_filein_no)
        {
          awka_filein_no--;
          free(awka_filein[j]);

          while (j < awka_filein_no)
          {
            awka_filein[j] = awka_filein[j+1];
            j++;
          }

          if (j == _awka_curfile)
            _awka_file_read = TRUE;
        }
      }

      _a_iostream[i].io = _a_IO_CLOSED;
      _a_iostream[i].fp = NULL;

      if (_a_iostream[i].buf)
        free(_a_iostream[i].buf);

      _a_iostream[i].buf = _a_iostream[i].current = _a_iostream[i].end = 0;
      _a_iostream[i].alloc = 0;
    }
  }

  return ret;
}

/*
 * awka_isarray
 * awk builtin extended function 'isarray'
 */
a_VAR *
awka_isarray(char keep, a_VARARG *va)
{
  a_VAR *outvar;

  /* create a variable */
  _awka_getdoublevar;

  if (va->used != 1)
    awka_error("runtime error: isarray expects only one parameter.\n");

  outvar->dval = (va->var[0]->type == a_VARARR);

  return (outvar);
}

/*
 * awka_typeof
 * awk builtin extended function 'typeof'
 */
a_VAR *
awka_typeof(char keep, a_VARARG *va)
{
  a_VAR *outvar;

  /* create a variable */
  _awka_getstringvar;

  if (va->used != 1)
    awka_error("runtime error: isarray expects only one parameter.\n");

  switch (va->var[0]->type)
  {
    case a_VARNUL:
      awka_strscpy(outvar, "untyped");
      break;

    case a_VARDBL:
      awka_strscpy(outvar, "number");
      break;

    case a_VARSTR:
      awka_strscpy(outvar, "string");
      break;

    case a_VARARR:
      awka_strscpy(outvar, "array");
      break;

    case a_VARREG:
      awka_strscpy(outvar, "regexp");
      break;

    case a_VARUNK:
      if (va->var[0]->type2 == a_DBLSET)
        awka_strscpy(outvar, "strnum");
      else if (va->var[0]->type2 == a_STRSET)
        awka_strscpy(outvar, "string");
      else
        awka_strscpy(outvar, "unassigned");
      break;

    default:
      awka_strscpy(outvar, "unassigned");
      break;
  }

  return (outvar);
}

/*
 * awka_length
 * awk builtin extended function 'length'
 */
double
awka_length(a_VAR *v)
{
  double outvar = 0.0;

  switch (v->type)
  {
    case a_VARUNK:
    case a_VARDBL:
    case a_VARREG:
      outvar = strlen(awka_gets1(v));
      break;

    case a_VARSTR:
      outvar = strlen(v->ptr);
      break;

    case a_VARARR:
      outvar = awka_alength(v);
      break;

    default:
      outvar = 0.0;
      break;
  }

  return (outvar);
}


/*
 * awka_patsplit
 * awk builtin extended function 'patsplit'
 * Split vstr into varr using vreg (or FPAT if vreg is NULL)
 * and store the separators in vsep if vsep is not NULL.
 *  vreg
 */
a_VAR *
awka_patsplit(a_VAR *vstr, a_VAR *varr, a_VAR *vreg, a_VAR *vsep)
{
  a_VAR *outvar, *fpat = vreg;
  awka_regexp *r;
  static regmatch_t pmatch;

  outvar = awka_tmp_dbl2var(0.0);
  awka_gets(vstr);

  if (!fpat)
  {
    fpat = awka_tmp_str2var(a_bivar[a_FPAT]->ptr);
  }
  if (fpat->type != a_VARREG)
    _awka_getreval(fpat, __FILE__, __LINE__, _RE_MATCH);

  r = (awka_regexp *) fpat->ptr;

  if (r->cant_be_null)
  {
    r = _awka_compile_regexp_MATCH(r->origstr, r->strlen);
    fpat->ptr = (char *) r;
  }

  outvar->dval = awka_arraysplitpat(vstr->ptr, varr, fpat, _split_max);

  if (!vsep)
  {
    ;
  }
}

int
awka_globline(const char *pattern)
{
  a_VAR      *_haystack;
  const char *s;
  int         s_size;
  int         ret;

  _haystack = awka_dol0(a_DOL_GET);
  ret = nstring_match(pattern, strlen(pattern), _haystack->ptr, _haystack->slen);

  return (ret == 0) ? 1 : 0;
}

