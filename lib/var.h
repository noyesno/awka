/*--------------------------------------------------*
 | var.h                                            |
 | Header file for var.c, part of the Awka          |
 | Library, Copyright 1999, Andrew Sumner.          |
 | This file is covered by the GNU General Public   |
 | License (GPL).                                   |
 *--------------------------------------------------*/

#ifndef _VAR_H
#define _VAR_H

#define a_VARNUL 0  /* vars not set to anything yet */
#define a_VARDBL 1  /* vars set to double */
#define a_VARSTR 2  /* vars set to string */
#define a_VARARR 4  /* array variables */
#define a_VARREG 5  /* regular expressions */
#define a_VARUNK 6  /* contents of a split array & return from getline */

#define a_DBLSET 7  /* indicates string var with double value set */
#define a_STRSET 8  /* indicates double var with string value set */

/* builtin variables  (align with ivar[] in init.c) */
#define a_ARGC        0
#define a_ARGIND      1
#define a_ARGV        2
#define a_CONVFMT     3
#define a_ENVIRON     4
#define a_FIELDWIDTHS 5
#define a_FILENAME    6
#define a_FNR         7
#define a_FPAT        8
#define a_FS          9
#define a_FUNCTAB     10
#define a_NF          11
#define a_NR          12
#define a_OFMT        13
#define a_OFS         14
#define a_ORS         15
#define a_PROCINFO    16
#define a_RLENGTH     17
#define a_RS          18
#define a_RSTART      19
#define a_RT          20
#define a_SAVEWIDTHS  21
#define a_SORTTYPE    22
#define a_SUBSEP      23
#define a_DOL0        24
#define a_DOLN        25

#define a_BIVARS      26

#define _RE_SPLIT   0
#define _RE_MATCH   1
#define _RE_GSUB    2

#define VARARGSZ    256

typedef struct {
  double dval;          /* double value, set when type & a_DBLVAR */
  char * ptr;           /* string value, except with array vars */
  unsigned int slen;    /* length of ptr as returned by strlen */
  unsigned int allc;    /* space mallocated for ptr */
  char type;            /* records current cast of variable */
  char type2;           /* for string vars, TRUE if dval is set */
  char temp;            /* TRUE if a temporary variable */
} a_VAR;

typedef struct {
  a_VAR *var[VARARGSZ];
  int used;
} a_VARARG;

void          awka_killvar( a_VAR * );
a_VAR *       _awka_getdval( a_VAR *, char *, int );
a_VAR *       _awka_setdval( a_VAR *, char *, int );
char *        _awka_getsval( a_VAR *, char, char *, int );
char **       awka_setsval( a_VAR *, char *, int );
awka_regexp * _awka_getreval( a_VAR *, char *, int, char );
awka_regexp * _awka_compile_regexp_SPLIT(char *str, unsigned len);
awka_regexp * _awka_compile_regexp_MATCH(char *str, unsigned len);
awka_regexp * _awka_compile_regexp_GSUB(char *str, unsigned len);
a_VAR *       awka_strdcpy( a_VAR *, double d );
a_VAR *       awka_strscpy( a_VAR *, char *s );
double        awka_vardblset( a_VAR *, double d );
a_VAR *       awka_varcpy( a_VAR *, a_VAR * );
int           awka_vartrue( a_VAR *v );
double        awka_varcmp( a_VAR *, a_VAR * );
double        awka_dbl2varcmp( double, a_VAR * );
double        awka_var2dblcmp( a_VAR *, double );
int           awka_nullval( char * );
double        awka_str2pow( char * );
void          awka_parsecmdline(int first);
a_VAR *       awka_tmp_dbl2var(double);
char *        awka_tmp_dbl2str(double);
a_VAR *       awka_tmp_str2var(char *);
a_VAR *       awka_ro_str2var(char *);
a_VAR *       awka_tmp_str2revar(char *);
a_VAR *       awka_tmp_re2var(awka_regexp *r);
void          _awka_re2s(a_VAR *);
void          _awka_re2null(a_VAR *);
a_VAR *       awka_vardup(a_VAR *);
double        awka_postinc(a_VAR *);
double        awka_postdec(a_VAR *);

#ifndef _INIT_C
extern a_VAR *a_bivar[a_BIVARS];
#else
a_VAR *a_bivar[a_BIVARS];
#endif

#ifndef _VAR_C
extern char _awka_arg_change;
#endif

#ifndef _ARRAY_C
extern char fs_or_fw, _awka_setdol0_len;
extern char _rebuild0, _rebuildn, _rebuild0_now;
#endif

extern void _awka_set_re_syntax(char *);

static INLINE a_VAR * awka_NFget();
#define _awka_set_FW(v) \
  if ((v) == a_bivar[a_FS]) { \
    fs_or_fw = 0; awka_NFget(); \
  } else if ((v) == a_bivar[a_FIELDWIDTHS]) {\
    fs_or_fw = 1; awka_NFget(); \
  } else if ((v) == a_bivar[a_FPAT]) {\
    fs_or_fw = 2; awka_NFget(); \
  }

static int
awka_isadbl( char *s, int len )
{
  register char *p = s, dot = FALSE;

  /* while (*p == ' ') p++; */
  for ( ;*p; p++)
  {
    if (*p == '.')
    {
      if (dot == TRUE)
        return FALSE;
      dot = TRUE;
      continue;
    }
    if (*p == ' ') break;
    if (!isdigit(*p)) return FALSE;
  }

  if (!*p) return TRUE;

  /* while (*p == ' ') p++; */
  if (*p) return FALSE;
  return TRUE;
}

static a_VAR *
awka_getdval( a_VAR *v, char *file, int line )
{
  if (v->type == a_VARDBL || v->type2 == a_DBLSET) 
    return v;
  return _awka_getdval(v, file, line);
}

static a_VAR *
awka_setdval( a_VAR *v, char *file, int line )
{
  v->type2 = 0;
  if (v->type == a_VARDBL)
    return v;
 
  return _awka_setdval(v, file, line);
}

static char *
awka_getsval( a_VAR *v, char ofmt, char *file, int line )
{
  if (v->type == a_VARSTR || v->type == a_VARUNK ||
     (v->type == a_VARDBL && v->type2 == a_STRSET))
    return v->ptr;
  return _awka_getsval(v, ofmt, file, line );
}

static char *
awka_getsvalP( a_VAR *v, char ofmt, char *file, int line )
{
  if (v->type == a_VARSTR || v->type == a_VARUNK)
    return v->ptr;
  return _awka_getsval(v, ofmt, file, line );
}

static awka_regexp *
awka_getreval( a_VAR *v, char *file, int line )
{
  v->type2 = 0;
  if (v->type == a_VARREG && v->ptr)
    return (awka_regexp *) v->ptr;
  return _awka_getreval(v, file, line, _RE_MATCH);
}

static void
awka_forcestr( a_VAR *v )
{
  v->type2 = 0;
  if (v->type != a_VARSTR && v->type != a_VARUNK)
    awka_setsval(v, __FILE__, __LINE__);
  else
    v->type = a_VARSTR;
}

static a_VAR *
awka_argv()
{
  _awka_arg_change = TRUE;
  return a_bivar[a_ARGV];
}

static a_VAR *
awka_argc()
{
  _awka_arg_change = TRUE;
  return a_bivar[a_ARGC];
}

static void
awka_lcopy(a_VAR *va, a_VAR *vb)
{
  va->ptr = vb->ptr;
  va->slen = vb->slen;
  va->allc = 0;
  va->dval = vb->dval;
  va->type = vb->type;
  va->type2 = vb->type2;
}

#define awka_getd(v)  (awka_getdval((v),__FILE__,__LINE__)->dval) 
#define awka_setd(v)  (awka_setdval((v),__FILE__,__LINE__)->dval)
#define awka_gets(v)  (awka_getsval((v),0,__FILE__,__LINE__)) 
#define awka_getsP(v) (awka_getsvalP((v),1,__FILE__,__LINE__)) 
#define awka_sets(v)  (*(awka_setsval((v),__FILE__,__LINE__)))
#define awka_getre(v) (awka_getreval((v),__FILE__,__LINE__))

#define awka_gets1(v) ((v)->ptr && ((v)->type == a_VARSTR || (v)->type == a_VARUNK) ? (v)->ptr : _awka_getsval((v),0,__FILE__,__LINE__)) 
#define awka_gets1P(v) ((v)->ptr && ((v)->type == a_VARSTR || (v)->type == a_VARUNK) ? (v)->ptr : _awka_getsval((v),1,__FILE__,__LINE__)) 
#define awka_getd1(v) ((v)->type == a_VARDBL || (v)->type2 == a_DBLSET ? (v)->dval : _awka_getdval((v),__FILE__,__LINE__)->dval)

#define awka_poi(v) (((v)->type == a_VARDBL && (v)->type2 != a_STRSET) ? ((v)->dval)++ : awka_postinc(v))
#define awka_pri(v) (((v)->type == a_VARDBL && (v)->type2 != a_STRSET) ? ++(v)->dval : ++awka_setd(v))
#define awka_pod(v) (((v)->type == a_VARDBL && (v)->type2 != a_STRSET) ? ((v)->dval)-- : awka_postdec(v))
#define awka_prd(v) (((v)->type == a_VARDBL && (v)->type2 != a_STRSET) ? --(v)->dval : --awka_setd(v))

#define awka_varinit(v) { \
       malloc( (void **) &(v), sizeof(a_VAR) ); \
       memset( (v), 0, sizeof(a_VAR) ); }
     /*  (v)->dval = 0.0; \
       (v)->temp = (v)->type2 = 0; \
       (v)->type = a_VARNUL; \
       (v)->slen = (v)->allc = 0; \
       (v)->ptr = NULL; }
*/
a_VAR *  awka_argval(int, a_VAR *, int, int, a_VARARG *);
void    _awka_addfnvar(int, int, a_VAR *, int);
a_VAR * _awka_usefnvar(int, int);
a_VAR * _awka_addfncall(int);
void    _awka_retfn(int);
int     _awka_registerfn(char *, int);

static a_VAR *
awka_fn_varinit(int fn_idx, int var_idx, int type)
{
  a_VAR *var;

  if (!(var = _awka_usefnvar(fn_idx, var_idx)))
  {
    awka_varinit(var);
    var->temp = 2;
    _awka_addfnvar(fn_idx, var_idx, var, type);
  }
  return var;
}

char *
awka_strcpy(a_VAR *v, char *s);

char *
awka_strncpy(a_VAR *v, char *s, int _slen);
/* see var.c */

static void
awka_setstrlen(a_VAR *v, register int slen)
{
  _awka_set_FW(v);
  slen++;
  if (v->type == a_VARREG)
    _awka_re2s(v);
  if (v->type != a_VARSTR && v->type != a_VARUNK)
    awka_setsval(v, __FILE__, __LINE__);

  if (v->ptr && v->allc < slen)
    v->allc = realloc( (void **) &v->ptr, slen );
  else if (!(v)->ptr)
    v->allc = malloc( (void **) &v->ptr, slen );
  v->slen = slen - 1;
  v->type2 = 0;
}

#define awka_strtrue(s) ((s)[0] != '\0' ? 1 : 0)

static double
_awka_dnotzero(double d, char *file, int line) 
{
  if (d == 0.0)
    awka_error("Math Error: Divide By Zero, file %s line %d.\n",file,line); 
  return d;
}

#define awka_dnotzero(d) _awka_dnotzero(d, __FILE__, __LINE__)

static double
awka_div(double d1, double d2)
{
  if (d2 == 0.0)
    awka_error("Math Error: Divide By Zero");
  return d1 / d2;
}

#endif
