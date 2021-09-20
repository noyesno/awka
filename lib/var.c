/*------------------------------------------------------------*
 | var.c                                                      |
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

#define _VAR_C
#define _IN_LIBRARY

#ifndef TRUE
#  define TRUE  1
#  define FALSE 0
#endif
char _awka_arg_change = FALSE;

#include "libawka.h"
#include "number.h"
#include "garbage.h"

struct _fnvar {
  a_VAR **var;
  char *status;
  int used;
  int prevused;
  int allc;
};

struct _fn {
  char *fn;
  struct _fnvar *fnvar;
  int push;
  int allc;
  int nvar;
} *_awka_fn = NULL;

int _awka_fn_allc = 0, _awka_fn_used = 0;

static a_VAR * bivarProcInfoFS = NULL;
static a_VAR * bivarProcInfoFPAT = NULL;

#define _awka_update_procinfo(v,s) \
  if ( (v) == a_bivar[a_FS] )\
  {\
    if (!bivarProcInfoFS)\
      bivarProcInfoFS = awka_getarrayval( a_bivar[a_PROCINFO], awka_tmp_str2var("FS") );\
    strncpy( bivarProcInfoFS->ptr, "FS", 2 );\
    bivarProcInfoFS->slen = 2;\
  }\
  else if ( (v) == a_bivar[a_FPAT] )\
  {\
    if (!bivarProcInfoFS)\
      bivarProcInfoFS = awka_getarrayval( a_bivar[a_PROCINFO], awka_tmp_str2var("FS") );\
    strncpy( bivarProcInfoFS->ptr, "FPAT", 4 );\
    bivarProcInfoFS->slen = 4;\
  }\
  else if ( (v) == a_bivar[a_FIELDWIDTHS] || (v) == a_bivar[a_SAVEWIDTHS] )\
  {\
    if (!bivarProcInfoFS)\
      bivarProcInfoFS = awka_getarrayval( a_bivar[a_PROCINFO], awka_tmp_str2var("FS") );\
    strncpy( bivarProcInfoFS->ptr, "FIELDWIDTHS", 11 );\
    /* sized to 12 chars originally in  init.c */ \
    bivarProcInfoFS->slen = 11;\
  }\
  else if ( (v)->type == a_VARSTR && strncmp((v)->ptr, "RE_SYNTAX_", 10) == 0)\
  {\
    _awka_set_re_syntax((s));\
  }

a_VAR *
awka_argval(int fn_idx, a_VAR *var, int arg_no, int arg_count, a_VARARG *va)
{
  a_VAR *ret;

  arg_no--;
  if (fn_idx == -1 || arg_no >= arg_count || arg_no < 0)
  {
    _awka_tmpvar(ret);
    awka_killvar(ret);
    return ret;
  }

  if (va->used == 0 || var->type != a_VARARR)
    return var;

  if (va->used == 1)
    return awka_arraysearch1( var,
                              va->var[0],
                              a_ARR_CREATE,
                              TRUE );

  return awka_arraysearch( var,
                           va,
                           a_ARR_CREATE );
}

int
_awka_registerfn(char *fn, int nvar)
{
  int i, j;

  for (i=0; i<_awka_fn_used; i++)
    if (!strcmp(_awka_fn[i].fn, fn))
      return i;

  if (i == _awka_fn_used)
  {
    if (_awka_fn_allc == 0)
    {
      _awka_fn_allc = 10;
      malloc( &_awka_fn, 10 * sizeof(struct _fn));
    }
    else if (_awka_fn_used == _awka_fn_allc)
    {
      _awka_fn_allc += 10;
      realloc( &_awka_fn, _awka_fn_allc * sizeof(struct _fn));
    }

    _awka_fn_used++;
    malloc( &_awka_fn[i].fn, strlen(fn)+1);
    strcpy(_awka_fn[i].fn, fn);
    _awka_fn[i].allc = 10;
    _awka_fn[i].push = 0;
    _awka_fn[i].nvar = nvar;
    malloc( &_awka_fn[i].fnvar, 10 * sizeof(struct _fnvar));
    for (j = 0; j<_awka_fn[i].allc; j++)
    {
      _awka_fn[i].fnvar[j].var = NULL;
      _awka_fn[i].fnvar[j].status = NULL;
      _awka_fn[i].fnvar[j].used = 0;
      _awka_fn[i].fnvar[j].prevused = 0;
      _awka_fn[i].fnvar[j].allc = 0;
    }
  }

  return i;
}

/*
 * addfnvar - this nonsense registers local variables
 *            within user functions.
 */
void
_awka_addfnvar(int i, int var_idx, a_VAR *var, int type)
{
  register int j;

  j = _awka_fn[i].push-1;
 _awka_fn[i].fnvar[j].used = (_awka_fn[i].fnvar[j].used <= var_idx ?
                              var_idx + 1 : _awka_fn[i].fnvar[j].used);
  _awka_fn[i].fnvar[j].var[var_idx] = var;
  _awka_fn[i].fnvar[j].prevused = _awka_fn[i].fnvar[j].used;
  _awka_fn[i].fnvar[j].status[var_idx] = type;
  if (type == 2)
    var->type = a_VARARR;
}

a_VAR *
_awka_usefnvar(int i, int var_idx)
{
  register int j = _awka_fn[i].push-1;
  a_VAR *var;

  if (_awka_fn[i].fnvar[j].prevused > var_idx)
  {
    var = _awka_fn[i].fnvar[j].var[ var_idx ];
    _awka_fn[i].fnvar[j].used = (_awka_fn[i].fnvar[j].used <= var_idx ?
                                 var_idx + 1 : _awka_fn[i].fnvar[j].used);
    return var;
  }
  else
    return NULL;
}

/*
 * awka_addfncall - denotes a call to a registered function, allocating
 *                  space if necessary for its local variables.
 */
a_VAR *
_awka_addfncall(int i)
{
  register int j, k;
  a_VAR *ret;

  _awka_tmpvar(ret);  /* bugfix for eiso */
  _awka_gc_deeper();

  if (_awka_fn[i].push == _awka_fn[i].allc)
  {
    _awka_fn[i].allc += 10;
    realloc( &_awka_fn[i].fnvar, _awka_fn[i].allc * sizeof(struct _fnvar));
    for (j = _awka_fn[i].push; j<_awka_fn[i].allc; j++)
    {
      _awka_fn[i].fnvar[j].var = NULL;
      _awka_fn[i].fnvar[j].used = 0;
      _awka_fn[i].fnvar[j].prevused = 0;
      _awka_fn[i].fnvar[j].allc = 0;
    }
  }

  j = _awka_fn[i].push;
  _awka_fn[i].push++;

  if (_awka_fn[i].fnvar[j].allc == 0 && _awka_fn[i].nvar)
  {
    _awka_fn[i].fnvar[j].allc = _awka_fn[i].nvar;
    malloc( &_awka_fn[i].fnvar[j].var, _awka_fn[i].nvar * sizeof(a_VAR *));
    malloc( &_awka_fn[i].fnvar[j].status, _awka_fn[i].nvar);
    for (k=0; k<_awka_fn[i].nvar; k++)
      _awka_fn[i].fnvar[j].var[k] = NULL;
  }

  if (ret->ptr)
  {
    if (ret->type == a_VARREG)
    {
      _awka_re2null(ret);
    }
    else
    {
      ret->ptr[0] = '\0';
      ret->slen = 0;
    }
  }
  else
    ret->type = a_VARNUL;

  return ret;
}

/*
 * awka_retfn - this frees local variables in user-functions upon
 *              a return.
 */
void
_awka_retfn(int i)
{
  register int j, k;
  a_VAR *var;

  if (_awka_fn[i].push == 0)
    return;

  _awka_fn[i].push--;
  j = _awka_fn[i].push;

  for (k=0; k<_awka_fn[i].fnvar[j].used; k++)
  {
    var = _awka_fn[i].fnvar[j].var[k];
    if (!var) continue;
    if (var->ptr)
    {
      /* if this is a local function var, manage its memory */
      if (!_awka_fn[i].fnvar[j].status[k])
      {
        if (!var->allc)
          var->ptr = NULL;
        else
          awka_killvar(var);
      }
      else if (var->type == a_VARARR)
      {
        awka_arrayclear(var);
        free(var->ptr);
        var->ptr = NULL;
        var->allc = 0;
      }
      else
      {
        awka_gets1(var);
        var->ptr[0] = '\0';
      }
    }

    if (var->type == a_VARDBL)
    {
      /* if (var->ptr) awka_killvar(var); */
      var->type = a_VARNUL;
    }
    var->slen = 0;
    var->dval = 0.0;
    var->type2 = 0;
  } /* for */

  _a_gc_depth--;
  _awka_fn[i].fnvar[j].used = 0;
}

void
awka_killvar( a_VAR *v )
{
  if (!v) return;

  if (v->ptr)
  {
    if (v->type == a_VARARR)
    {
      awka_arrayclear(v);
      if (v->allc)
        free(v->ptr);
    }
    else if (v->type == a_VARREG)
    {
      regfree((awka_regexp *) v->ptr);
    }
    else
      if (v->allc)
        free(v->ptr);
  }

  v->ptr = NULL;
  v->dval = 0.0;
  v->allc = v->slen = v->type2 = 0;
  v->type = a_VARNUL;
}

void
_awka_re2s( a_VAR *v )
{
  awka_regexp *r;

  if (v->type != a_VARREG) return;
  r = (awka_regexp *) v->ptr;
  malloc( &v->ptr, (v->slen = r->strlen) + 1 );
  memcpy(v->ptr, r->origstr, v->slen+1);
  v->type = a_VARSTR;
  v->allc = v->slen+1;
  v->type2 = 0;
}

void
_awka_re2null( a_VAR *v )
{
  if (v->type != a_VARREG)
    return;

  v->type = a_VARNUL;
  v->allc = v->slen = 0;
  v->type2 = 0;
  v->ptr = NULL;
}

double
awka_postinc( a_VAR *v )
{
  double d = awka_setd(v);
  v->dval++;
  return d;
}

double
awka_postdec( a_VAR *v )
{
  double d = awka_setd(v);
  v->dval--;
  return d;
}

a_VAR *
_awka_getdval( a_VAR *v, char *file, int line )
{
  switch (v->type)
  {
    case a_VARREG:
      _awka_re2s(v);
      /* fall through */
    case a_VARSTR:
    case a_VARUNK:
      if (v->type2 == (char) -1)
        v->dval = 0;
      else if (v->ptr)
        v->dval = strtod(v->ptr, NULL);
      else
        v->dval = 0;
      break;
    case a_VARNUL:
      v->dval = 0.0;
      break;
    case a_VARARR:
      awka_error("runtime error: awka_getd in file %s, line %d - %s\n", file,line,"array used as scalar");
  }

  if (v->type == a_VARDBL && v->type2 != (char) -1)
    v->type2 = a_DBLSET;

  return v;
}

a_VAR *
_awka_setdval( a_VAR *v, char *file, int line )
{
  if (v->type == a_VARREG)
    _awka_re2null(v);

  _awka_set_FW(v);

  v->type2 = 0;
  if (v->type == a_VARSTR || v->type == a_VARUNK)
  {
    if (v->ptr)
    {
      v->dval = strtod(v->ptr, NULL);
      free(v->ptr);
    }
    v->ptr = NULL;
    v->slen = 0;
    v->allc = 0;
    v->type = a_VARDBL;
    return v;
  }
  else if (v->type == a_VARNUL)
  {
    v->type = a_VARDBL;
    v->dval = 0.0;
    /* v->ptr = NULL; */
    return v;
  }
  else
  {
    /* array */
    awka_error("runtime error: awka_setd in file %s, line %d - %s\n", file,line,"array used as scalar");
  }

  /* dead code - have to check this... */
  if (_awka_setdoln == TRUE)
    _awka_setdol0_len = TRUE;

  if (v == a_bivar[a_DOL0])
  {
    _rebuild0_now = FALSE;
    _rebuildn = TRUE;
  }

  return v;
}

char *
_awka_getsval( a_VAR *v, char ofmt, char *file, int line )
{
  char varbuf[256], *ptr = NULL;
  register int i;

  switch (v->type)
  {
    case a_VARDBL:
      i = (int) (v->dval);
      if ((double) i == v->dval)
      {
        v->slen =  sprintf(varbuf, "%d", i);
      }
      else
      {
        if (ofmt)
          v->slen = sprintf(varbuf, awka_gets1(a_bivar[a_OFMT]), v->dval);
        else
          v->slen = sprintf(varbuf, awka_gets1(a_bivar[a_CONVFMT]), v->dval);
      }
      if (!v->ptr || (v->temp == 2 && v->allc <= v->slen))
        v->allc = malloc( &v->ptr, v->slen + 1 );
      else if (v->allc <= v->slen)
        v->allc = realloc( &v->ptr, v->slen + 1 );

      memcpy(v->ptr, varbuf, v->slen+1);
      v->type2 = (ofmt ? 0 : a_STRSET);
      return v->ptr;

    case a_VARSTR:
    case a_VARUNK:
      v->allc = malloc( &v->ptr, 8 );
      ptr = v->ptr;
      v->slen = 0;
      v->ptr[0] = '\0';
      return ptr;

    case a_VARREG:
      if (v->ptr)
      {
        _awka_re2s(v);
        v->type = a_VARSTR;
        return v->ptr;
      }
      v->dval = 0.0;
      v->type = a_VARNUL;
      /* fall thru */
    case a_VARNUL:
      ptr = _awka_tmpstr(1);
      ptr[0] = '\0';
      v->slen = 0;
      return ptr;

    case a_VARARR:
      awka_error("runtime error: awka_gets in file %s, line %d - array used as scalar.\n",file,line);

    default:
      awka_error("runtime error: awka_gets in file %s, line %d - unexpected type value (%d).\n",file,line,v->type);
  }

  return ptr;  /* can't get here anyway */
}

awka_regexp *
_awka_getreval( a_VAR *v, char *file, int line, char type )
{
  awka_regexp *r = NULL;
  switch (v->type)
  {
    case a_VARDBL:
      awka_gets1(v);
      break;

    case a_VARNUL:
      v->allc = malloc( &v->ptr, 1 );
      v->ptr[0] = '\0';
      v->slen = 0;
      break;

    case a_VARARR:
      awka_error("runtime error: awka_getre in file %s, line %d - %s\n", file,line,"array used as scalar");
  }

  if (!v->ptr)
  {
    v->allc = malloc( &v->ptr, 1 );
    v->slen = 0;
    v->ptr[0] = '\0';
  }

  switch (type)
  {
    case _RE_SPLIT:
      r = _awka_compile_regexp_SPLIT(v->ptr, v->slen); break;
    case _RE_MATCH:
      r = _awka_compile_regexp_MATCH(v->ptr, v->slen); break;
    case _RE_GSUB:
      r = _awka_compile_regexp_GSUB(v->ptr, v->slen); break;
  }

  if (!r)
    awka_error("runtime error: Regular Expression failed to compile, file %s line %d\n",file,line);

  free(v->ptr);

  v->ptr = (char *) r;
  v->type = a_VARREG;

  return r;
}

char **
awka_setsval( a_VAR *v, char *file, int line )
{
  if (v->type == a_VARARR)
    awka_error("runtime error: awka_sets in file %s, line %d - %s\n", file,line,"array used as scalar");
  else if (v->type == a_VARREG)
    _awka_re2null(v);
  else if (v->ptr)
  {
    free(v->ptr);
    v->ptr = NULL;
  }

  v->slen = 0;
  v->allc = 0;
  v->type2 = 0;
  v->type = a_VARSTR;

  return &(v->ptr);
}

a_VAR *
awka_strdcpy( a_VAR *v, double d )
{
  char tmp[256];
  int i = (int) d, slen = 0;

  if ((double) i == d)
    slen = sprintf(tmp, "%d", i);
  else
    slen = sprintf(tmp, awka_gets1(a_bivar[a_CONVFMT]), d);

  if (v->type == a_VARSTR || v->type == a_VARUNK)
  {
    if (!v->ptr)
      v->allc = malloc( &v->ptr, slen+1 );
    else if (slen >= v->allc)
      v->allc = realloc( &v->ptr, slen+1 );
  }
  else
    v->allc = malloc( &v->ptr, slen+1 );

  v->slen = slen;
  memcpy(v->ptr, tmp, slen+1);
  v->type = a_VARSTR;

  return v;
}

a_VAR *
awka_strscpy( a_VAR *v, char *s )
{
  register int i = strlen(s);

  if (v->type == a_VARSTR || v->type == a_VARUNK)
  {
    if (!v->ptr)
      v->allc = malloc( &v->ptr, i+1 );
    else if (v->allc < i)
      v->allc = realloc( &v->ptr, i+1 );
  }
  else
    v->allc = malloc( &v->ptr, i+1 );

  v->slen = i;
  memcpy(v->ptr, s, i+1);
  v->type = a_VARSTR;

  return v;
}

a_VAR *
awka_vardup( a_VAR *v )
{
  a_VAR *ret;

  _awka_tmpvar(ret);
  awka_varcpy(ret, v);

  return ret;
}

double
awka_vardblset( a_VAR *v, double d )
{
  if (v->type == a_VARARR)
    awka_error("runtime error: awka_vardblset - %s\n", "array used as scalar");

  _awka_set_FW(v);

  if (v->type == a_VARREG)
    _awka_re2null(v);

  _awka_set_FW(v);

  v->type2 = 0;
  if (v->type == a_VARSTR || v->type == a_VARUNK)
  {
    if (v->ptr)
      free(v->ptr);
    v->ptr = NULL;
    v->slen = 0;
    v->allc = 0;
  }

  v->type = a_VARDBL;
  v->dval = d;

  if (_awka_setdoln == TRUE)
    _awka_setdol0_len = TRUE;

  if (v == a_bivar[a_DOL0])
  {
    _rebuild0_now = FALSE;
    _rebuildn = TRUE;
  }

  return v->dval;
}

a_VAR *
awka_varcpy( a_VAR *va, a_VAR *vb )
{
  int prev_len = -1;
  register char *ptr;
  register unsigned int allc;

  if (vb->type == a_VARARR || va->type == a_VARARR)
    awka_error("runtime error: awka_varcpy - %s\n", "array used as scalar");

  _awka_set_FW(va);

  if (va == vb)
  {
    if (vb->type == a_VARNUL)
      va->type = a_VARUNK;

    return va;
  }

  va->dval = vb->dval;
  va->type2 = vb->type2;

  switch (vb->type)
  {
    case a_VARSTR:
    case a_VARUNK:
      prev_len = va->slen;
      if (vb->temp == 1)
      { /* don't need vb beyond this statement */
        /* swap pointers - this is very efficient */
        if (va->type == a_VARREG)
          _awka_re2null(va);

        ptr = va->ptr;
        allc = va->allc;
        va->ptr = vb->ptr;
        va->allc = vb->allc;
        va->slen = vb->slen;
        vb->ptr = ptr;

        if (ptr)
          ptr[0] = '\0';

        vb->type2 = 0;
        vb->slen = 0;
        vb->allc = allc;
        vb->dval = 0;
      }
      else
      {
        /* we need both va & vb, memcpy is necessary */
        awka_forcestr(va);

        if (va->ptr && va->allc <= vb->slen)
          va->allc = realloc( &va->ptr, vb->slen+1 );
        else if (!va->ptr)
          va->allc = malloc( &va->ptr, vb->slen+1 );

        if (!vb->ptr)
        {
          va->ptr[0] = '\0';  /* null valued string type */
          vb->slen = 0;
        }
        else
          memcpy(va->ptr, vb->ptr, vb->slen+1);

        va->slen = vb->slen;
      }

      va->type = vb->type;
      va->type2 = vb->type2;

      if (va->slen)
        _awka_update_procinfo(va,va->ptr);

      break;

    case a_VARREG:
      if (va->ptr)
        awka_killvar(va);

      va->ptr = vb->ptr;
      break;

    case a_VARDBL:
      if (vb->ptr && vb->type2 == a_STRSET)
      {
        if (!va->ptr || !va->allc)
          va->allc = malloc( &va->ptr, vb->slen+1 );
        else if (va->ptr && va->allc <= vb->slen)
          va->allc = realloc( &va->ptr, vb->slen+1 );

        memcpy(va->ptr, vb->ptr, vb->slen+1);
        va->slen = vb->slen;
      }
      break;
  } /* switch */

  va->type = vb->type;

  if (_awka_setdoln == TRUE)
  {
    if (prev_len != -1)
    {
      _awka_dol0_len -= prev_len;
      _awka_dol0_len += vb->slen;
      _awka_setdoln = _awka_setdol0_len = FALSE;
    }
    else
      _awka_setdol0_len = TRUE;
  }

  if (va == a_bivar[a_DOL0])
  {
    _rebuild0_now = FALSE;
    _rebuildn = TRUE;
  }


  return va;
}

void
_awka_checkunk(a_VAR *va)
{
  if (va->type2 == 0 && va->ptr)
  {
    if (!isalpha(va->ptr[0]) &&
        _awka_isnumber(va->ptr) == TRUE)
    {
      if (va->type != a_VARDBL)
        va->type2 = a_DBLSET;

      va->dval = strtod(va->ptr, NULL);
    }
    else {
      va->dval = 0.0;
      va->type2 = 0;
    }
    /*
    else
      va->type2 = -1;
      */    /* commented in 0.5.10 as causing grief */
  }
}

double
awka_varcmp( a_VAR *va, a_VAR *vb )
{
  int i;

  if (vb->type == a_VARARR || va->type == a_VARARR)
    awka_error("runtime error: awka_varcmp", "array used as scalar");

  if (va == vb)
    return 0;

  if (va->type == a_VARUNK && va->type2 == 0 && va->ptr)
    _awka_checkunk(va);

  if (vb->type == a_VARUNK && vb->type2 == 0 && vb->ptr)
    _awka_checkunk(vb);

  if ((va->type <= a_VARDBL ||
      (va->type == a_VARUNK && va->type2 == a_DBLSET)) &&
      (vb->type <= a_VARDBL ||
      (vb->type == a_VARUNK && vb->type2 == a_DBLSET)))
  {
    /* double comparison */
    if (va->dval == vb->dval)
      return 0;
    else
      return ((va->dval < vb->dval) ? -1 : 1);
  }

  i = strcmp(awka_gets1(va), awka_gets1(vb));

  return ((i == 0) ? 0 : ((i < 0) ? -1 : 1));
}

int
awka_vartrue( a_VAR *v )
{
  if (v->type == a_VARSTR && v->ptr)
  {
    if (v->ptr[0] != '\0')
      return 1;
    else
      return 0;
  }

  if (v->type == a_VARDBL && v->dval != 0.0)
    return 1;

  if (v->type == a_VARUNK)
  {
    if (v->ptr && v->ptr[0] != '\0' && strcmp(v->ptr, "0"))
      return 1;
    if (v->type2 == a_DBLSET && v->dval != 0.0)
      return 1;
  }

  if (v->type == a_VARREG)
    return 1;

  return 0;
}

double
awka_var2dblcmp( a_VAR *va, double d )
{
  int i;

  if (va->type == a_VARARR)
    awka_error("runtime error: awka_var2dblcmp", "array used as scalar");

  if (va->type == a_VARUNK && va->type2 == 0 && va->ptr)
    _awka_checkunk(va);

  if (va->type == a_VARDBL || (va->type == a_VARUNK && va->type2 == a_DBLSET))
    return (va->dval == d ? 0 : (va->dval < d ? -1 : 1));

  if (!(i = strcmp(awka_gets1(va), awka_tmp_dbl2str(d))))
    return 0;

  return ((i < 0) ? -1 : 1);
}

double
awka_dbl2varcmp( double d, a_VAR *va )
{
  int i;

  if (va->type == a_VARARR)
    awka_error("runtime error: awka_var2dblcmp", "array used as scalar");

  if (va->type2 == 0 && va->ptr && va->type == a_VARUNK)
    _awka_checkunk(va);

  if (va->type == a_VARDBL || (va->type == a_VARUNK && va->type2 == a_DBLSET))
  {
    i = (d == va->dval ? 0 : (d < va->dval ? -1 : 1));
    return (double) i;
  }

  if (!(i = strcmp(awka_tmp_dbl2str(d), awka_gets1(va))))
    return 0;

  return ((i < 0) ? -1 : 1);
}

int
awka_nullval( char *s )
{
  double d;
  char *p;

  d = strtod(s, NULL);

  if (!d)
  {
    p = s + (strlen(s)-1);

    while ((*p == ' ' || *p == '\t') && p > s)
      p--;

    p++;
    *p = '\0';
    p = s;

    while (*p == ' ' || *p == '\t')
      p++;

    while (*p)
    {
      if (isalpha(*p) ||
         (ispunct(*p) && *p != '.') ||
         (isdigit(*p) && *p != '0'))
        break;

      p++;
    }

    if (*p == '\0')
      return 1;
  }

  return 0;
}


a_VAR *
awka_tmp_dbl2var(double d)
{
  a_VAR *v;

  _awka_tmpvar(v);

  if (v->ptr && v->type == a_VARREG)
    _awka_re2null(v);

  /* v->ptr = NULL; */
  v->type = a_VARDBL;
  v->slen = 0;
  /* v->allc = 0; */
  v->dval = d;
  v->type2 = 0;

  return v;
}

a_VAR *
awka_ro_str2var(char *c)
{
  a_VAR *v;
  int i = strlen(c);

  _awka_tmpvar_ro(v);

  v->type = a_VARSTR;
  v->ptr = c;
  v->slen = i;
  v->allc = 0;
  v->dval = 0.0;
  v->type2 = 0;

  return v;
}

a_VAR *
awka_tmp_str2var(char *c)
{
  a_VAR *v;
  int i = strlen(c);

  _awka_tmpvar(v);

  if (v->type == a_VARSTR || v->type == a_VARUNK || v->type == a_VARREG)
  {
    if (v->type == a_VARREG)
      _awka_re2null(v);

    if (v->allc <= i)
      v->allc = realloc( &v->ptr, i+1 );
    else if (!v->ptr)
      v->allc = malloc( &v->ptr, i+1 );
  }
  else
  {
    if (v->ptr) free(v->ptr);
    v->allc = malloc( &v->ptr, i+1 );
  }

  v->type = a_VARSTR;
  memcpy(v->ptr, c, i+1);
  v->slen = i;
  v->dval = 0.0;
  v->type2 = 0;

  return v;
}

a_VAR *
awka_tmp_re2var(awka_regexp *r)
{
  a_VAR *v;

  _awka_tmpvar(v);

  if (v->ptr)
    awka_killvar(v);

  v->ptr = (char *) r;
  v->type = a_VARREG;
  v->slen = 0;
  v->allc = 0;
  v->dval = 0.0;
  v->type2 = 0;

  return v;
}

char *
awka_tmp_dbl2str(double d)
{
  char *s, tmp[25];   /* max chars for %d on 64 bit machine should be 23 */
  int i = (int) d;
  int len;

  if ((double) i == d)
    len = sprintf(tmp, "%d", i);
  else
    len = sprintf(tmp, awka_gets1(a_bivar[a_CONVFMT]), d);

  i = len + 1;
  len = i + (32 - (i % 32));

  _awka_tmpvar_c(s, len);
  memcpy(s, tmp, i);

  return s;
}

char *
awka_strcpy(a_VAR *v, char *s)
{
  register int _slen = strlen(s)+1;

  _awka_set_FW(v);

  if (v->type == a_VARREG)
    _awka_re2s(v);

  if (v->type != a_VARSTR && v->type != a_VARUNK)
    awka_setsval(v, __FILE__, __LINE__);

  if (v->ptr && v->allc <= _slen)
    v->allc = realloc( (void **) &v->ptr, _slen );
  else if (!v->ptr)
    v->allc = malloc( (void **) &v->ptr, _slen );

  v->slen = _slen-1;
  memcpy(v->ptr, s, _slen);
  v->type = a_VARSTR;
  v->type2 = 0;

  if (v == a_bivar[a_DOL0])
  {
#ifdef MEM_DEBUG
    _rebuild0_now = FALSE;
    _rebuildn = TRUE;
#else
    _rebuild0_now = _rebuild0 = FALSE;
    _rebuildn = _awka_setdol0_len = TRUE;
#endif
  }

  _awka_update_procinfo(v,s);

  return v->ptr;
}


char *
awka_strncpy(a_VAR *v, char *s, int _slen)
{
  a_VAR *tmpv = NULL;

  _awka_set_FW(v);

  if (v->type == a_VARREG)
    _awka_re2s(v);

  if (v->type != a_VARSTR && v->type != a_VARUNK)
    awka_setsval(v, __FILE__, __LINE__);

  if (v->ptr && v->allc <= _slen+1)
    v->allc = realloc( (void **) &v->ptr, _slen+1 );
  else if (!v->ptr)
    v->allc = malloc( (void **) &v->ptr, _slen+1 );

  v->slen = _slen;
  memcpy(v->ptr, s, _slen);
  v->ptr[_slen] = '\0';
  v->type = a_VARSTR;
  v->type2 = 0;

  _awka_update_procinfo(v,s);

  return v->ptr;
}

