/*------------------------------------------------------------*
 | translate.c                                                |
 | copyright 1999,  Andrew Sumner (andrewsumner@yahoo.com)    |
 |                                                            |
 | This is a source file for the awka package, a translator   |
 | of the AWK programming language to ANSI C.                 |
 |                                                            |
 | This program is free software; you can redistribute it     |
 | and/or modify it under the terms of the GNU General Public |
 | License as published by the Free Software Foundation;      |
 | either version 2 of the License, or any later version.     |
 |                                                            |
 | This program is distributed in the hope that it will be    |
 | useful, but WITHOUT ANY WARRANTY; without even the implied |
 | warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR    |
 | PURPOSE.  See the GNU General Public License for more      |
 | details.                                                   |
 |                                                            |
 | You should have received a copy of the GNU General Public  |
 | License along with this program; if not, write to the      |
 | Free Software Foundation, Inc., 675 Mass Ave, Cambridge,   |
 | MA 02139, USA.                                             |
 *------------------------------------------------------------*/

#define BUILTIN_HOME

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include <stdarg.h>
#include "../config.h"
#include "mem.h"
#include "awka_exe.h"
#include "msg.h"
#include "../patchlev.h"

struct ivar_idx ivar[] = {
 { "ARGC",        "a_bivar[a_ARGC]"        },
 { "ARGIND",      "a_bivar[a_ARGIND]"      },
 { "ARGV",        "a_bivar[a_ARGV]"        },
 { "CONVFMT",     "a_bivar[a_CONVFMT]"     },
 { "ENVIRON",     "a_bivar[a_ENVIRON]"     },
 { "FIELDWIDTHS", "a_bivar[a_FIELDWIDTHS]" },
 { "FILENAME",    "a_bivar[a_FILENAME]"    },
 { "FNR",         "a_bivar[a_FNR]"         },
 { "FPAT",        "a_bivar[a_FPAT]"        },
 { "FS",          "a_bivar[a_FS]"          },
 { "FUNCTAB",     "a_bivar[a_FUNCTAB]"     },
 { "NF",          "a_bivar[a_NF]"          },
 { "NR",          "a_bivar[a_NR]"          },
 { "OFMT",        "a_bivar[a_OFMT]"        },
 { "OFS",         "a_bivar[a_OFS]"         },
 { "ORS",         "a_bivar[a_ORS]"         },
 { "PROCINFO",    "a_bivar[a_PROCINFO]"    },
 { "RLENGTH",     "a_bivar[a_RLENGTH]"     },
 { "RS",          "a_bivar[a_RS]"          },
 { "RSTART",      "a_bivar[a_RSTART]"      },
 { "RT",          "a_bivar[a_RT]"          },
 { "SAVEWIDTHS",  "a_bivar[a_SAVEWIDTHS]"  },
 { "SORTTYPE",    "a_bivar[a_SORTTYPE]"    },
 { "SUBSEP",      "a_bivar[a_SUBSEP]"      }
};
#define IVAR_MAX 22

#define _a_LHS 0
#define _a_RHS 1

int push_func[END_CODE], assign_op[END_CODE], change_op[END_CODE];
int isarray(char *);

char *getdoublevalue(char *);
char *getstringvalue(char *);
int findvarname(awka_varname *, char *, int);
int findvaltype(char *);
void setvaltype(char *, char);
void setvaltype2(char *, char *);
void translate_section(int start, int end);
void preprocess();
void addvarname(awka_varname **, char *, char);
void laddvarname(awka_varname **, char *, char);
void addvarname_ref(awka_varname *, int, char, char *, unsigned int);
void initlvartypes();

extern char *awk_input_files;

int indent = 1;
int split_req = 0, split_max = 0, mode = NONE, dol0_used = 0, dol0_only = 1, env_used = 0;
int doln_set = 0, dol0_get = 0;
int var_allc = 0, var_used = 0, lvar_allc = 0, lvar_used = 0;
int litr_allc = 0, litr_used = 0;
int lits_allc = 0, lits_used = 0;
int litd_allc = 0, litd_used = 0;
awka_varname *varname = NULL, *lvarname = NULL;
char **litrname = NULL, **litsname = NULL, **litdname = NULL;
char **litr_val = NULL, **lits_val = NULL, **litd_val = NULL;
extern FILE *outfp;
extern int awka_main;
extern char *awka_main_func;
int cur_func = 0;
int al_count = 0;  /* array loop count */
int range_no = 0;
int max_call_args = 0;
char *int_argv = NULL;
char **functions = NULL;
int func_no = 0;
int which_side = _a_RHS;
int max_base_gc = 1, max_fn_gc = 1, cur_base_gc = 1, cur_fn_gc = 1;

static int getstringsize(const char *ps) {
  int n=0;
  char c;
  register const char *p = ps;

  while (c = *p++) {
    if (c=='\\' && *p && (1 || *p=='0' || *p=='n' || *p=='r' || *p=='t' || *p=='\"' || *p=='\\')) {
      p++;
      n++;
    } else {
      n++;
    }
  }
  return n;
}

char *
codeptr(int inst, int len)
{
  if (!progcode[inst].code)
  {
    progcode[inst].code_allc = 5;
    progcode[inst].code = (char **) malloc( 5 * sizeof(char *) );
  }
  else if (progcode[inst].code_used == progcode[inst].code_allc)
  {
    progcode[inst].code_allc += 5;
    progcode[inst].code = (char **) realloc( progcode[inst].code, 
                        progcode[inst].code_allc * sizeof(char *) );
  }
  progcode[inst].code[ progcode[inst].code_used ] = (char *) malloc(len+10);
  return progcode[inst].code[ progcode[inst].code_used++ ];
}

char *
code0ptr(int inst, int len)
{
  if (!progcode[inst].code0)
  {
    progcode[inst].code0_allc = 5;
    progcode[inst].code0 = (char **) malloc( 5 * sizeof(char *) );
  }
  else if (progcode[inst].code0_used == progcode[inst].code0_allc)
  {
    progcode[inst].code0_allc += 5;
    progcode[inst].code0 = (char **) realloc( progcode[inst].code0, 
                        progcode[inst].code0_allc * sizeof(char *) );
  }
  progcode[inst].code0[ progcode[inst].code0_used ] = (char *) malloc(len+10);
  return progcode[inst].code0[ progcode[inst].code0_used++ ];
}

void
revert_gc()
{
  cur_fn_gc = 1;
  cur_base_gc = 1;
} 

void 
check_gc()
{
  if (mode == FUNC)
  {
    cur_fn_gc++;
    max_fn_gc = (max_fn_gc < cur_fn_gc ? cur_fn_gc : max_fn_gc);
  }
  else
  {
    cur_base_gc++;
    max_base_gc = (max_base_gc < cur_base_gc ? cur_base_gc : max_base_gc);
  }
}

char *
autoindent(char *str)
{
  static char *x = NULL;
  static int alloc = 10;
  int i, new_indent = indent;

  if (!x)
  {
    x = (char *) malloc((indent * 2)+1);
    alloc = (indent * 2) + 1;
  }
  else if (alloc < (indent * 2) + 1)
  {
    x = (char *) realloc(x, (indent * 2)+1);
    alloc = (indent * 2) + 1;
  }

  i = strlen(str);

  if (i >= 1)
  {
    if (str[--i] == '\n' && i >= 1)
      i--;
    if ((str[i] == '}' && indent > 0) || str[0] == '}')
    {
      do {
        new_indent = --indent;
        i -= 2;  if (i < 0) break;
      } while (str[i] == '}');
    }
    else if (str[i] == '{')
      new_indent++;
  }

  if (indent < 0) indent = 0;
  memset(x, ' ', indent*2);
  x[(indent*2)] = '\0';

  indent = new_indent;

  return x;
}

int
findivar(char *c)
{
  int i = IVAR_MAX / 2, hi = IVAR_MAX, lo = 0;
  int x;

  while (1)
  {
    if (!(x = strcmp(ivar[i].name, c)))
      return i;
    else if (x > 0)
    {
      if (i == lo)
        return -1;
      else if (i-1 == lo)
      {
        if (!strcmp(ivar[lo].name, c))
          return lo;
        return -1;
      }
      hi = i;
      i = lo + ((hi - lo) / 2);
    }
    else
    {
      if (i == hi)
        return -1;
      else if (i+1 == hi)
      {
        if (!strcmp(ivar[hi].name, c))
          return hi;
        return -1;
      }
      lo = i;
      i = lo + ((hi - lo) / 2);
    }
  }
}

char *
buildstr(char *func, char *fmt, char *p, char context, char true_context, int inst, int prev)
{
  static char *tmp = NULL;
  char *dest;
  static int tmp_allc = 0;

  if (!tmp_allc)
  {
    tmp_allc = 1024;
    tmp = (char *) malloc(1024);
  }
  else if (strlen(p) + 100 > tmp_allc)
  {
    tmp_allc += strlen(p) + 100;
    tmp = (char *) realloc(tmp, tmp_allc);
  }

  if (true_context == _ROVAR && context != _STR)
    true_context = _VAR;

  switch (context)
  {
    case _UNK:
    case _NUL:
      if (true_context == _NUL)
        sprintf(tmp, "%s", p);
      else
        awka_error("%s error: Null or Unknown format for statement '%s', line %d.\n",func,p,progcode[inst].line);
      break;

    case _ROVAR:
    case _VAR:
      if (true_context == _DBL)
      {
        if (findvaltype(p) == _VALTYPE_NUM || (prev >= 0 && progcode[prev].ftype == 1))
          sprintf(tmp, "%s", getdoublevalue(p));
          /* sprintf(tmp, "%s->dval", p); */
        else
          sprintf(tmp, "awka_getd(%s)",p);
      }
      else if (true_context == _STR)
      {
        if (findvaltype(p) == _VALTYPE_STR || (prev >= 0 && progcode[prev].ftype == 2))
          sprintf(tmp, "%s", getstringvalue(p));
          /* sprintf(tmp, "%s->ptr", p); */
        else if (prev >= 0 && push_func[progcode[prev].op])
          sprintf(tmp, "awka_gets1(%s)", p);
        else
          sprintf(tmp, "awka_gets(%s)",p);
      }
      else if (true_context == _REG)
        sprintf(tmp, "awka_getre(%s)",p);
      else if (true_context == _TRU)
      {
        if (findvaltype(p) == _VALTYPE_NUM || (prev >= 0 && progcode[prev].ftype == 1))
          sprintf(tmp, "(%s)", getdoublevalue(p));
          /* sprintf(tmp, "(%s->dval)", p); */
        if (findvaltype(p) == _VALTYPE_STR || (prev >= 0 && progcode[prev].ftype == 2))
          // TODO: make "abc" a glob match
          // XXX: sprintf(tmp, "awka_strtrue(%s) /* 1 */", getstringvalue(p));
          sprintf(tmp, "awka_globline(%s)", getstringvalue(p));
          /* sprintf(tmp, "awka_strtrue(%s->ptr)", p); */
        else
          sprintf(tmp, "awka_vartrue(%s)",p);
      }
      else 
        sprintf(tmp, "%s", p);
      break;

   case _VARX:  /* special one used only for awka_a_test */
      if (true_context == _DBL)
      {
        if (findvaltype(p) == _VALTYPE_NUM || (prev >= 0 && progcode[prev].ftype == 1))
          sprintf(tmp, "%s", getdoublevalue(p));
          /* sprintf(tmp, "%s->dval", p); */
        else
          sprintf(tmp, "awka_getd(%s)",p);
      }
      else if (true_context == _STR)
      {
        if (findvaltype(p) == _VALTYPE_STR || (prev >= 0 && progcode[prev].ftype == 2))
          sprintf(tmp, "%s", getstringvalue(p));
          /* sprintf(tmp, "%s->ptr", p); */
        else if (prev >= 0 && push_func[progcode[prev].op])
          sprintf(tmp, "awka_gets1(%s)", p);
        else
          sprintf(tmp, "awka_gets(%s)",p);
      }
      else if (true_context == _REG)
        sprintf(tmp, "awka_getre(%s)",p);
      else if (true_context == _TRU)
      {
        if (findvaltype(p) == _VALTYPE_NUM || (prev >= 0 && progcode[prev].ftype == 1))
          sprintf(tmp, "(%s)", getdoublevalue(p));
          /* sprintf(tmp, "(%s->dval)", p); */
        if (findvaltype(p) == _VALTYPE_STR || (prev >= 0 && progcode[prev].ftype == 2))
          sprintf(tmp, "awka_strtrue(%s)", getstringvalue(p));
          /* sprintf(tmp, "awka_strtrue(%s->ptr)", p); */
        else
          sprintf(tmp, "(int) (%s)->slen != -1",p);
      }
      else 
        sprintf(tmp, "%s", p);
      break;

    case _DBL:
      if (true_context == _DBL || true_context == _TRU)
      {
        if (assign_op[progcode[prev].op])
          sprintf(tmp, "(%s)", p);
        else
          sprintf(tmp, "%s", p);
      }
      else if (true_context == _STR)
      {
        check_gc();
        sprintf(tmp, "awka_tmp_dbl2str(%s)", p);
      }
      else if (true_context == _VAR)
      {
        check_gc();
        sprintf(tmp, "awka_tmp_dbl2var(%s)", p);
      }
      else if (true_context == _REG)
        sprintf(tmp, "awka_getre(awka_tmp_dbl2var(%s))", p);
      else
      {
        if (assign_op[progcode[prev].op])
          sprintf(tmp, "(%s)", p);
        else
          sprintf(tmp, "%s", p);
      }
      break;

    case _TRU:
      if (true_context == _DBL || true_context == _TRU)
      {
        if (assign_op[progcode[prev].op])
          sprintf(tmp, "(%s)", p);
        else
          sprintf(tmp, "%s", p);
      }
      else if (true_context == _STR)
      {
        check_gc();
        sprintf(tmp, "awka_tmp_dbl2str(%s)", p);
      }
      else if (true_context == _VAR)
      {
        check_gc();
        sprintf(tmp, "awka_tmp_dbl2var(%s)", p);
      }
      else if (true_context == _REG)
        sprintf(tmp, "awka_getre(awka_tmp_dbl2var(%s))", p);
      else
      {
        if (assign_op[progcode[prev].op])
          sprintf(tmp, "(%s)", p);
        else
          sprintf(tmp, "%s", p);
      }
      break;

    case _STR:
      if (true_context == _DBL)
        sprintf(tmp, "strtod(%s, (char **) 0)", p);
      else if (true_context == _TRU)
        sprintf(tmp, "awka_strtrue(%s)", p);
      else if (true_context == _STR)
        sprintf(tmp, "%s", p);
      else if (true_context == _VAR)
      {
        check_gc();
        sprintf(tmp, "awka_tmp_str2var(%s)", p);
      }
      else if (true_context == _ROVAR)
      {
        check_gc();
        sprintf(tmp, "awka_ro_str2var(%s)", p);
      }
      else if (true_context == _REG)
        sprintf(tmp, "awka_getre(awka_tmp_str2var(%s))", p);
      else if (true_context == _TRU)
        sprintf(tmp, "awka_strtrue(%s)", p);
      else
        sprintf(tmp, "%s", p);
      break;

    case _REG:  /* variable that is a regular expression */
      if (true_context == _DBL)
        sprintf(tmp, "strtod((%s)->origstr, NULL)", p);
      else if (true_context == _TRU)
        sprintf(tmp, "awka_strtrue((%s)->origstr)", p);
      else if (true_context == _STR)
        sprintf(tmp, "(%s)->origstr", p);
      else if (true_context == _VAR)
      {
        check_gc();
        sprintf(tmp, "awka_tmp_re2var(%s)", p);
      }
      else if (true_context == _TRU)
        sprintf(tmp, "(%s != NULL ? 1 : 0)", p);
      else
        sprintf(tmp, "%s", p);
  }

  dest = (char *) malloc(strlen(tmp)+strlen(fmt)+10);
  sprintf(dest, fmt, tmp);
  return dest;
}

void
killcode(int inst)
{
  int k;

  if (progcode[inst].code) 
  {
    for (k=0; k<progcode[inst].code_used; k++)
      if (progcode[inst].code[k])
        free(progcode[inst].code[k]);
    free(progcode[inst].code);
    progcode[inst].code = NULL;
  }
  progcode[inst].code_used = 0;
  progcode[inst].code_allc = 0;
}

char *
test_previnst(int inst, int *earliest, char *context, char *n)
{
  char *ret = NULL;

  if (progcode[inst].done == TRUE) 
  { 
    if (progcode[inst].code_used > 1) 
      awka_error("internal error: '%s' at line %d cannot be included in a higher expression!\n",n,progcode[inst].line); 
    *earliest = progcode[inst].earliest; 
    *context = progcode[inst].context; 
    if (progcode[inst].code_used)
    {
      ret = progcode[inst].code[0];
      progcode[inst].code[0] = NULL; 
      progcode[inst].code_used = 0;
    }
    progcode[inst].func = NULL;
    return ret; 
  } 
  progcode[inst].done = TRUE;
  return NULL;
}

void
test_loop(int inst)
{
  int i = 0, j; 
  char *z, *x = (char *) malloc(100); 

  if (progcode[inst].endloop > 0) 
  { 
    for (j=0; j<progcode[inst].endloop; j++)
    {
      if (i) 
        sprintf(x, "%s }",x); 
      else 
        sprintf(x, "}"); 
      i = 1;
    }
    sprintf(x, "%s\n", x);
    z = code0ptr(inst, strlen(x)+1);
    strcpy(z, x);
    x[0] = '\0';
  } 
  if (progcode[inst].doloop == TRUE) 
  { 
    if (i) 
      sprintf(x, "%sdo\n",x); 
    else 
      sprintf(x, "do\n"); 
    z = code0ptr(inst, strlen(x)+1);
    strcpy(z, x);
    x[0] = '\0';
    z = code0ptr(inst, 5);
    strcpy(z, "{\n");
  } 
  if (progcode[inst].foreverloop == 1)
  {
    if (i) 
      sprintf(x, "%swhile (1)\n",x); 
    else 
      sprintf(x, "while (1)\n"); 
    z = code0ptr(inst, strlen(x)+1);
    strcpy(z, x);
    z = code0ptr(inst, 5);
    strcpy(z, "{\n");
  } 
  else if (progcode[inst].foreverloop == 2)
  {
    if (i) 
      sprintf(x, "%swhile (1);\n",x); 
    else 
      sprintf(x, "while (1);\n"); 
    z = code0ptr(inst, strlen(x)+1);
    strcpy(z, x);
    x[0] = '\0';
  } 
  free(x);
}

char *
awka_buildexpr(char *oper, char *p, char *q, int pprev, int qprev, char c1, char c2, int inst)
{
  char *ret = (char *) malloc(100 + strlen(getdoublevalue(q)) + strlen(getdoublevalue(p))
                                  + strlen(getstringvalue(q)) + strlen(getstringvalue(p))
                                  + strlen(q) + strlen(p));

  switch (c2)
  {
    case _VAR:
      if (c1 == _VAR)
      {
        if (findvaltype(q) == _VALTYPE_STR)
        {
          if (findvaltype(p) == _VALTYPE_STR || progcode[pprev].ftype == 2)
            sprintf(ret, "(strcmp(%s, %s) %s 0)", getstringvalue(q), getstringvalue(p), oper);
            /* sprintf(ret, "(strcmp(%s->ptr, %s->ptr) %s 0)", q, p, oper); */
          else
            sprintf(ret, "(strcmp(%s, awka_gets(%s)) %s 0)", getstringvalue(q), p, oper);
            /* sprintf(ret, "(strcmp(%s->ptr, awka_gets(%s)) %s 0)", q, p, oper); */
        }
        else if (findvaltype(p) == _VALTYPE_STR)
        {
          if (findvaltype(q) == _VALTYPE_STR || progcode[qprev].ftype == 2)
            sprintf(ret, "(strcmp(%s, %s) %s 0)", getstringvalue(q), getstringvalue(p), oper);
            /* sprintf(ret, "(strcmp(%s->ptr, %s->ptr) %s 0)", q, p, oper); */
          else
            sprintf(ret, "(strcmp(awka_gets(%s), %s) %s 0)", q, getstringvalue(p), oper);
            /* sprintf(ret, "(strcmp(awka_gets(%s), %s->ptr) %s 0)", q, p, oper); */
        }
        else if ((findvaltype(p) == _VALTYPE_NUM || progcode[pprev].ftype == 1) &&
                 (findvaltype(q) == _VALTYPE_NUM || progcode[qprev].ftype == 1))
          sprintf(ret, "%s %s %s", getdoublevalue(q), oper, getdoublevalue(p));
          /* sprintf(ret, "%s->dval %s %s->dval", q, oper, p); */
        else if ((findvaltype(p) == _VALTYPE_NUM || progcode[pprev].ftype == 1))
          sprintf(ret, "(awka_var2dblcmp(%s, %s) %s 0)", q, getdoublevalue(p), oper);
          /* sprintf(ret, "(awka_var2dblcmp(%s, %s->dval) %s 0)", q, p, oper); */
        else if ((findvaltype(q) == _VALTYPE_NUM || progcode[qprev].ftype == 1))
          sprintf(ret, "(awka_dbl2varcmp(%s, %s) %s 0)", getdoublevalue(q), p, oper);
          /* sprintf(ret, "(awka_dbl2varcmp(%s->dval, %s) %s 0)", q, p, oper); */
        else
          sprintf(ret, "(awka_varcmp(%s, %s) %s 0)", q, p, oper);
      }
      else if (c1 == _DBL)
      {
        if (findvaltype(q) == _VALTYPE_NUM || progcode[pprev].ftype == 1)
          sprintf(ret, "%s %s", getdoublevalue(q), oper);
          /* sprintf(ret, "%s->dval %s", q, oper); */
        else
          sprintf(ret, "awka_getd(%s) %s", q, oper);
        if (assign_op[progcode[pprev].op])
          sprintf(ret, "%s (%s)", ret, p);
        else
          sprintf(ret, "%s %s", ret, p);
      }
      else
      {
        if (findvaltype(q) == _VALTYPE_STR || progcode[qprev].ftype == 2)
          sprintf(ret, "(strcmp(%s, %s) %s 0)", getstringvalue(q), p, oper);
          /* sprintf(ret, "(strcmp(%s->ptr, %s) %s 0)", q, p, oper); */
        else if (push_func[progcode[qprev].op])
          sprintf(ret, "(strcmp(awka_gets1(%s), %s) %s 0)", q, p, oper);
        else
          sprintf(ret, "(strcmp(awka_gets(%s), %s) %s 0)", q, p, oper);
      }
      break;

    case _STR:
      if (c1 == _VAR)
      {
        if (findvaltype(p) == _VALTYPE_STR || progcode[pprev].ftype == 2)
          sprintf(ret, "(strcmp(%s, %s) %s 0)", q, getstringvalue(p), oper);
          /* sprintf(ret, "(strcmp(%s, %s->ptr) %s 0)", q, p, oper); */
        else if (push_func[progcode[pprev].op])
          sprintf(ret, "(strcmp(%s, awka_gets1(%s)) %s 0)", q, p, oper);
        else
          sprintf(ret, "(strcmp(%s, awka_gets(%s)) %s 0)", q, p, oper);
      }
      else if (c1 == _DBL)
      {
        sprintf(ret, "strtod(%s, NULL) %s %s", q, oper, p);
        if (assign_op[progcode[pprev].op])
          sprintf(ret, "%s (%s)", ret, p);
        else
          sprintf(ret, "%s %s", ret, p);
      }
      else
        sprintf(ret, "(strcmp(%s, %s) %s 0)", q, p, oper);
      break;

    case _DBL:
      if (assign_op[progcode[qprev].op])
        sprintf(ret, "(%s)", q);
      else
        sprintf(ret, "%s", q);
      if (c1 == _VAR)
      {
        if (findvaltype(q) == _VALTYPE_NUM || progcode[pprev].ftype == 1)
          sprintf(ret, "%s %s %s", ret, oper, getdoublevalue(p));
          /* sprintf(ret, "%s %s %s->dval", ret, oper, p); */
        else
          sprintf(ret, "%s %s awka_getd(%s)", ret, oper, p);
      }
      else if (c1 == _DBL)
      {
        if (assign_op[progcode[pprev].op])
          sprintf(ret, "%s %s (%s)", ret, oper, p);
        else
          sprintf(ret, "%s %s %s", ret, oper, p);
      }
      else
        sprintf(ret, "%s %s strtod(%s, NULL)", q, oper, p);
      break;
      
    default:
      awka_error("%s error: expecting var, str or dbl context for lside argument, line %d.\n",oper,progcode[inst].line);
  }
  return ret;
}

char *
awka_nullfunc(int inst, int *earliest, char *context)
{
  char *ret = NULL;

  if ((ret = test_previnst(inst, earliest, context, "nullfunc")) != NULL)
    return ret;
  test_loop(inst);

  *earliest = inst-1;
  *context = _NUL;
  return NULL;
}

char *
awka_a_cat(int inst, int *earliest, char *context)
{
  char *ret = NULL, *r2, *p, c1, *tmp, deref = FALSE;
  int prev = inst-1, i, j, ralloc = 0, k;

  if ((ret = test_previnst(inst, earliest, context, "a_cat")) != NULL)
    return ret;
  test_loop(inst);

  j = atoi(progcode[inst].val);
  if (j < 2) /* surely this cant happen */
    awka_error("a_cat error: expected at least 2 array subscripts, line %d.\n",progcode[inst].line);

  for (prev = inst-1, i=0; i<j; i++)
  {
    k = prev;
    p = (* progcode[prev].func)(prev, &prev, &c1);
    if (deref == TRUE && push_func[progcode[k].op] && strncmp(p, "_lit", 4))
      r2 = buildstr("a_cat", "awka_vardup(%s)", p, c1, _ROVAR, inst, k);
    else
      r2 = buildstr("a_cat", "%s", p, c1, _ROVAR, inst, k);
    if (change_op[progcode[k].op])
      deref = TRUE;
    free(p);

    if (!ralloc)
    {
      ret = (char *) malloc(strlen(r2)+20);
      strcpy(ret, r2);
      ralloc = 20;
    }
    else
    {
      k = strlen(ret) + strlen(r2) + 3;
      if (ralloc < k)
      {
        ralloc = k;
        ret = (char *) realloc(ret, k);
      }
      tmp = (char *) malloc(k);
      sprintf(tmp, "%s, %s", r2, ret);
      strcpy(ret, tmp);
      free(tmp);
    }
    free(r2);
  }

  *context = _VAR;
  *earliest = prev;
  return ret;
}

char *
awka_a_del(int inst, int *earliest, char *context)
{
  /* deletes an element in an array */
  char *ret, *p, *q, *r2, *r3, c1, c2;
  int prev, cat_prev, i;

  if ((ret = test_previnst(inst, earliest, context, "a_del")) != NULL)
    return ret;

  test_loop(inst);
  if (inst < 2)
    awka_error("a_del error: expected two prior opcodes, line %d.\n",progcode[inst].line);
  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  cat_prev = prev;
  q = (* progcode[prev].func)(prev, &prev, &c2);
  *earliest = prev;
  *context = _NUL;

  r2 = buildstr("a_del", "%s", p, c1, _VAR, inst, -1);
  r3 = buildstr("a_del", "%s", q, c2, _ROVAR, inst, -1);
  ret = (char *) malloc( strlen(r2) + strlen(r3) + 100 );
  if (progcode[cat_prev].op == A_CAT)
  {
    i = atoi(progcode[cat_prev].val);
    switch (i)
    {
      case 2:
        sprintf(ret, "awka_arraysearch(%s, awka_arg2(a_TEMP, %s), a_ARR_DELETE);\n",r2,r3);
        break;
      case 3:
        sprintf(ret, "awka_arraysearch(%s, awka_arg3(a_TEMP, %s), a_ARR_DELETE);\n",r2,r3);
        break;
      default:
        sprintf(ret, "awka_arraysearch(%s, awka_vararg(a_TEMP, %s, NULL), a_ARR_DELETE);\n",r2,r3);
        break;
    }
  }
  else
    sprintf(ret, "awka_arraysearch1(%s, %s, a_ARR_DELETE, 0);\n",r2,r3);

  free(r2);
  free(r3);
  free(p);
  free(q);
  return ret;
}

char *
awka_push(int inst, int *earliest, char *context)
{
  char *ret, *p, c1, *r2;
  int prev, i;

  if ((ret = test_previnst(inst, earliest, context, "push")) != NULL)
    return ret;
  test_loop(inst);
  *context = _VAR;

  switch (progcode[inst].op)
  {
    case _PUSHI:  /* fall through */
    case A_PUSHA: /* fall through */
    case _PUSHA:
      /* variable */
      if ((i = findivar(progcode[inst].val)) != -1)
      {
        /* its a builtin variable! */
        if (!strcmp(ivar[i].vname, "a_bivar[a_ARGC]") && progcode[inst].op != _PUSHI)
        {
          ret = (char *) malloc(15);
          strcpy(ret, "awka_argc()");
        }
        else if (!strcmp(ivar[i].vname, "a_bivar[a_ARGV]") && progcode[inst].op != _PUSHI)
        {
          ret = (char *) malloc(15);
          strcpy(ret, "awka_argv()");
        }
        else if (!strcmp(ivar[i].vname, "a_bivar[a_NF]"))
        {
          ret = (char *) malloc(20);
          if (progcode[inst].op == _PUSHI)
            strcpy(ret, "awka_NFget()");
          else
            strcpy(ret, "awka_NFset()");
          split_req = dol0_used = 1;
          split_max = INT_MAX;
          dol0_only = 0;
        }
        else
        {
          ret = (char *) malloc(strlen(ivar[i].vname)+1);
          strcpy(ret, ivar[i].vname);
          if (!strcmp(ret, "a_bivar[a_ENVIRON]"))
            env_used = 1;
        }
      }
      else
      {
        if (!strcmp(progcode[inst].val, "a_bivar[a_NF]"))
        {
          ret = (char *) malloc(20);
          if (progcode[inst].op == _PUSHI)
            strcpy(ret, "awka_NFget()");
          else
            strcpy(ret, "awka_NFset()");
          split_req = dol0_used = 1;
          split_max = INT_MAX;
          dol0_only = 0;
        }
        else
        {
          ret = (char *) malloc(strlen(progcode[inst].val)+1);
          strcpy(ret, progcode[inst].val);
          if ((i = findvarname(varname, ret, var_used)) > -1)
          {
            addvarname_ref(varname, i, ((progcode[inst].op == _PUSHI || progcode[inst].op == A_PUSHA) ? _VAR_REF : _VAR_SET), progcode[inst].file, progcode[inst].line);
            varname[i].type = (progcode[inst].op == A_PUSHA ? _VARTYPE_G_ARRAY : _VARTYPE_G_SCALAR);
          }
        }
      }
      *earliest = inst-1;
      break;
    case AE_PUSHA: /* fallthru */
    case AE_PUSHI:
      /* array element */
      check_gc();
      if (inst < 1)
        awka_error("ae_push error: expected a prior opcode, line %d.\n",progcode[inst].line);
      p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
      r2 = buildstr("ae_push", "%s", p, c1, _ROVAR, inst, -1);
      ret = (char *) malloc( strlen(progcode[inst].val) + strlen(r2) + 100 );
      if (!strcmp(progcode[inst].val, "a_bivar[a_ENVIRON]")) env_used = 1;
      if (progcode[inst-1].op == A_CAT)
      {
        i = atoi(progcode[inst-1].val);
        switch (i)
        {
          case 2:
            sprintf(ret, "awka_arraysearch(%s, awka_arg2(a_TEMP, %s), a_ARR_CREATE)",progcode[inst].val,r2);
            break;
          case 3:
            sprintf(ret, "awka_arraysearch(%s, awka_arg3(a_TEMP, %s), a_ARR_CREATE)",progcode[inst].val,r2);
            break;
          default:
            sprintf(ret, "awka_arraysearch(%s, awka_vararg(a_TEMP, %s, NULL), a_ARR_CREATE)",progcode[inst].val,r2);
            break;
        }
      }
      else
      {
        if (progcode[inst].op == AE_PUSHA)
          sprintf(ret, "awka_arraysearch1(%s, %s, a_ARR_CREATE, 1)",progcode[inst].val,r2);
	//else if (al_count)
        //  sprintf(ret, "awka_arraynextget(&_alh%i, _alp%i)", al_count-1, al_count-1);
	else
          sprintf(ret, "awka_getarrayval(%s, %s)",progcode[inst].val,r2);
      }
      if ((i = findvarname(varname, progcode[inst].val, var_used)) > -1)
      {
        addvarname_ref(varname, i, (progcode[inst].op == AE_PUSHI ? _VAR_REF : _VAR_SET), progcode[inst].file, progcode[inst].line);
        varname[i].type = _VARTYPE_G_ARRAY;
      }
      *earliest = prev;
      free(p);
      free(r2);
      break;
    case F_PUSHA:
      ret = (char *) malloc( strlen(progcode[inst].val) + 40 );
      if (progcode[inst].val[0] == '$')
      {
        /* field variable on lside */
        progcode[inst].val[0] = '0';
        i = atoi(progcode[inst].val);
        if (inst < prog_no-1 && (progcode[inst+1].op == _GSUB || progcode[inst+1].op == _SUB_BI || progcode[inst+2].op == _GETLINE))
        {
          if (i == 0)
            sprintf(ret, "awka_dol0(a_DOL_REBLDN)");
          else
            sprintf(ret, "awka_doln(%d, a_DOL_REBLDN)",i);
        }
        else
        {
          if (i == 0)
            sprintf(ret, "awka_dol0(a_DOL_SET)");
          else
            sprintf(ret, "awka_doln(%d, a_DOL_SET)",i);
        }
        if (i > 0)
        {
          split_req = 1;
          doln_set = 1;
          /* split_max = INT_MAX; */
          split_max = (split_max < i ? i : split_max);
          dol0_only = 0;
        }
        dol0_used = 1;
      }
      else
      {
        /* internal variable on lside */
        if (!strcmp(progcode[inst].val, "NF"))
        {
          sprintf(ret, "awka_NFset()");
          split_req = dol0_used = 1;
          split_max = INT_MAX;
          dol0_only = 0;
        }
        else
          sprintf(ret, "a_bivar[a_%s]",progcode[inst].val);
      }
      *earliest = inst-1;
      break;
    case F_PUSHI:
      /* field variable on rside */
      if (progcode[inst].val[0] == '$')
      {
        progcode[inst].val[0] = '0';
        i = atoi(progcode[inst].val);
        ret = (char *) malloc( strlen(progcode[inst].val) + 20 );
        if (i == 0)
          sprintf(ret, "awka_dol0(a_DOL_GET)");
        else
          sprintf(ret, "awka_doln(%d, a_DOL_GET)",i);
        if (i > 0)
        {
          split_req = 1;
          split_max = (split_max < i ? i : split_max);
          dol0_only = 0;
        }
        else
          dol0_get = 1;
        dol0_used = 1;
      }
      else
      {
        /* internal variable on lside */
        if (!strcmp(progcode[inst].val, "NF"))
        {
          sprintf(ret, "awka_NFget()");
          split_req = dol0_used = 1;
          dol0_only = 0;
          split_max = INT_MAX;
        }
        else
          sprintf(ret, "a_bivar[a_%s]",progcode[inst].val);
      }
      *earliest = inst-1;
      break;
    case _FE_PUSHA:
      /* field variable, referenced by another var (eg $n), on lside */
      if (inst < 1)
        awka_error("fe_pusha error: expected a prior opcode, line %d.\n",progcode[inst].line);
      p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
      ret = buildstr("fe_pusha", "awka_doln(%s, a_DOL_SET)", p, c1, _DBL, inst, inst-1);
      split_req = dol0_used = 1;
      dol0_only = 0;
      split_max = INT_MAX;
      *earliest = prev;
      free(p);
      break;
    case FE_PUSHI:
      /* field variable, referenced by another var (eg $n), on rside */
      if (inst < 1)
        awka_error("fe_pushi error: expected a prior opcode, line %d.\n",progcode[inst].line);
      p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
      ret = buildstr("fe_pushi", "awka_doln(%s, a_DOL_GET)", p, c1, _DBL, inst, inst-1);
      split_req = dol0_used = 1;
      dol0_only = 0;
      split_max = INT_MAX;
      *earliest = prev;
      free(p);
      break;
    case LA_PUSHA:
    case L_PUSHA: /* fallthru */
    case L_PUSHI: /* fallthru */
      /* local function variable on lside */
      ret = (char *) malloc(8 + strlen(progcode[inst].val));
      sprintf(ret, "_lvar[%d]",atoi(progcode[inst].val));
      *earliest = inst-1;
      if ((i = findvarname(lvarname, progcode[inst].val, lvar_used)) == -1)
      {
        i = lvar_used;
        laddvarname(&lvarname, progcode[inst].val, (progcode[inst].op == LA_PUSHA ? _VARTYPE_G_ARRAY : _VARTYPE_G_SCALAR));
      }
      addvarname_ref(lvarname, i, (progcode[inst].op == L_PUSHI ? _VAR_REF : _VAR_SET), progcode[inst].file, progcode[inst].line);
      lvarname[i].type = (progcode[inst].op == LA_PUSHA ? _VARTYPE_G_ARRAY : _VARTYPE_G_SCALAR);
      break;
    case LAE_PUSHA: /* fallthru */
    case LAE_PUSHI:
      /* local function var array element */
      check_gc();
      if (inst < 1)
        awka_error("lae_push error: expected a prior opcode, line %d.\n",progcode[inst].line);
      p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
      r2 = buildstr("lae_push", "%s", p, c1, _VAR, inst, inst-1);
      ret = (char *) malloc( strlen(progcode[inst].val) + strlen(r2) + 100 );
      if (progcode[inst-1].op == A_CAT)
      {
        i = atoi(progcode[inst-1].val);
        switch (i)
        {
          case 2:
            sprintf(ret, "awka_arraysearch(_lvar[%d], awka_arg2(a_TEMP, %s), a_ARR_CREATE)",atoi(progcode[inst].val),r2);
            break;
          case 3:
            sprintf(ret, "awka_arraysearch(_lvar[%d], awka_arg3(a_TEMP, %s), a_ARR_CREATE)",atoi(progcode[inst].val),r2);
            break;
          default:
            sprintf(ret, "awka_arraysearch(_lvar[%d], awka_vararg(a_TEMP, %s, NULL), a_ARR_CREATE)",atoi(progcode[inst].val),r2);
            break;
        }
      }
      else
      {
        if (progcode[inst].op == LAE_PUSHA)
          sprintf(ret, "awka_arraysearch1(_lvar[%d], %s, a_ARR_CREATE, 1)",atoi(progcode[inst].val),r2);
	//else if (al_count)
        //  sprintf(ret, "awka_arraynextget(&_alh%i, _alp%i)", al_count-1, al_count-1);
        else
          sprintf(ret, "awka_getarrayval(_lvar[%d], %s)",atoi(progcode[inst].val),r2);
      }
      if ((i = findvarname(lvarname, progcode[inst].val, lvar_used)) == -1)
      {
        i = lvar_used;
        laddvarname(&lvarname, progcode[inst].val, _VARTYPE_G_ARRAY);
      }
      addvarname_ref(lvarname, i, (progcode[inst].op == LAE_PUSHI ? _VAR_REF : _VAR_SET), progcode[inst].file, progcode[inst].line);
      lvarname[i].type = _VARTYPE_G_ARRAY;
      *earliest = prev;
      free(p);
      free(r2);
      break;
    case NF_PUSHI:
      ret = (char *) malloc(20);
      strcpy(ret, "awka_NFget()");
      split_req = dol0_used = 1;
      dol0_only = 0;
      split_max = INT_MAX;
      *earliest = inst-1;
      break;
    case _PUSHC:
      *earliest = inst-1;
      *context = _STR;
      if (progcode[inst].val[0] == '0')
      {
        ret = (char *) malloc(strlen(progcode[inst].arg)+1);
        strcpy(ret, progcode[inst].arg);
        *context = _VAR;
      }
      else if (!strncmp(progcode[inst].val, "repl", 4))
      {
        ret = (char *) malloc(strlen(progcode[inst].arg)+3);
        sprintf(ret, "\"%s\"", progcode[inst].arg);
      }
      else if (!strcmp(progcode[inst].val, "null"))
      {
        ret = (char *) malloc(3);
        strcpy(ret, "\"\"");
      }
      else if (!strcmp(progcode[inst].val, "space"))
      {
        ret = (char *) malloc(4);
        strcpy(ret, "\" \"");
      }
      else
        awka_error("pushc error: unsupported argument '%s', line %d.\n",progcode[inst].val,progcode[inst].line);
      break;
    case _PUSHD:
      ret = (char *) malloc(strlen(progcode[inst].val)+1);
      strcpy(ret, progcode[inst].val);
      *context = _VAR;
      /* *context = _DBL; */
      *earliest = inst-1;
      break;
    case _PUSHINT:
      ret = (char *) malloc(strlen(progcode[inst].val)+1);
      strcpy(ret, progcode[inst].val);
      *earliest = inst-1;
      *context = _DBL;
      break;
    case _PUSHS:
      /* string on rside of expr */
      ret = (char *) malloc(strlen(progcode[inst].val)+3);
      strcpy(ret, progcode[inst].val);
      /* sprintf(ret, "\"%s\"", progcode[inst].val); */
      *context = _VAR;
      /* *context = _STR; */
      *earliest = inst-1;
      break;
  }
  return ret;
}

char *
awka_a_test(int inst, int *earliest, char *context)
{
  /* test (i in j) */
  char *ret, *p, *q, c1, c2, *r2, *r3;
  int prev, prev2, i;

  if ((ret = test_previnst(inst, earliest, context, "a_test")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 2)
    awka_error("a_test error: expected two prior opcodes, line %d.\n",progcode[inst].line);
  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  prev2 = prev;
  q = (* progcode[prev].func)(prev, &prev, &c2);
  *earliest = prev;
  r2 = buildstr("a_test", "%s", p, c1, _VAR, inst, inst-1);
  r3 = buildstr("a_test", "%s", q, c2, _ROVAR, inst, prev2);
  ret = (char *) malloc( strlen(r2) + strlen(r3) + 90 );
  check_gc();
  if (progcode[inst-2].op == A_CAT)
  {
    i = atoi(progcode[inst-2].val);
    switch (i)
    {
      case 2:
        sprintf(ret, "awka_arraysearch(%s, awka_arg2(a_TEMP, %s), a_ARR_QUERY)",r2,r3);
        break;
      case 3:
        sprintf(ret, "awka_arraysearch(%s, awka_arg3(a_TEMP, %s), a_ARR_QUERY)",r2,r3);
        break;
      default:
        sprintf(ret, "awka_arraysearch(%s, awka_vararg(a_TEMP, %s, NULL), a_ARR_QUERY)",r2,r3);
        break;
    }
  }
  else
    sprintf(ret, "awka_arraysearch1(%s, %s, a_ARR_QUERY, 0)",r2,r3);
  *context = _VARX;
  free(p);
  free(q);
  free(r2);
  free(r3);
  return ret;
}

char *
awka_add(int inst, int *earliest, char *context)
{
  char *ret, *r2, *p, *q, c1, c2;
  int prev, prev2;

  if ((ret = test_previnst(inst, earliest, context, "add")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 2)
    awka_error("add error: expected two prior opcodes, line %d.\n",progcode[inst].line);
  p = (* progcode[inst-1].func)(inst-1, &prev2, &c1);
  q = (* progcode[prev2].func)(prev2, &prev, &c2);
  *earliest = prev;

  if (push_func[progcode[inst-1].op])
    ret = buildstr("add", "(%s + ", q, c2, _DBL, inst, prev2);
  else
    ret = buildstr("add", "((%s) + ", q, c2, _DBL, inst, prev2);
  if (push_func[progcode[prev2].op])
    r2 = buildstr("add", "%s)", p, c1, _DBL, inst, inst-1);
  else
    r2 = buildstr("add", "(%s))", p, c1, _DBL, inst, inst-1);
  ret = (char *) realloc(ret, strlen(ret) + strlen(r2) + 1);
  strcat(ret, r2);
  *context = _DBL;

  free(p);
  free(q);
  free(r2);
  return ret;
}

char *
awka_add_asg(int inst, int *earliest, char *context)
{
  char *ret, *r2, *p, *q, c1, c2;
  int prev;

  if ((ret = test_previnst(inst, earliest, context, "add_asg")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 2)
    awka_error("add_asg error: expected two prior opcodes, line %d.\n",progcode[inst].line);
  which_side = _a_RHS;
  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  which_side = _a_LHS;
  q = (* progcode[prev].func)(prev, &prev, &c2);
  which_side = _a_RHS;
  *earliest = prev;

  if (c2 != _VAR)
    awka_error("add_asg error: expecting var context for lside, line %d.\n",progcode[inst].line);

  ret = (char *) malloc( 50 + (strlen(q)*3) );
  /* sprintf(ret, "awka_vardblset(%s, awka_getd(%s) + ", q, q); */
  /* r2 = buildstr("add_asg", "%s)", p, c1, _DBL, inst, inst-1); */
  sprintf(ret, "awka_setd(%s) += ", q); 
  setvaltype(q, _VALTYPE_NUM);
  r2 = buildstr("add_asg", "%s", p, c1, _DBL, inst, inst-1); 
  ret = (char *) realloc(ret, strlen(ret) + strlen(r2) + 2);
  sprintf(ret, "%s%s",ret,r2);
  *context = _DBL;

  free(p);
  free(q);
  free(r2);
  return ret;
}

char *
awka_aloop(int inst, int *earliest, char *context)
{
  /* ends 'for (bugs in NT)' loop */
  /* - how do you end a forever loop??? 8-) */
  char *ret;
  int i;
  if ((ret = test_previnst(inst, earliest, context, "aloop")) != NULL)
    return ret;
  test_loop(inst);

  i = *((int *) progcode[inst].arg);
  ret = (char *) malloc(100);
  sprintf(ret, "awka_alistfree(&_alh%d);\n",i);
  free(progcode[inst].arg);
  *earliest = inst-1;
  *context = _NUL;
  return ret;
}

char *
awka_assign(int inst, int *earliest, char *context)
{
  char *ret, *p, *q, c1, c2, *r2;
  int prev, prev2;

  if ((ret = test_previnst(inst, earliest, context, "assign")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 2)
    awka_error("assign error: expected two prior opcodes, line %d.\n",progcode[inst].line);
  which_side = _a_RHS;
  p = (* progcode[inst-1].func)(inst-1, &prev2, &c1);
  which_side = _a_LHS;
  q = (* progcode[prev2].func)(prev2, &prev, &c2);
  which_side = _a_RHS;
  *earliest = prev;

  if (c2 != _VAR)
    awka_error("assign error: expecting var context for lside, line %d.\n",progcode[inst].line);
  r2 = buildstr("assign", "%s", q, c2, _VAR, inst, prev2);
  ret = (char *) malloc(100 + strlen(r2) + strlen(getdoublevalue(p))
                            + strlen(getstringvalue(p)) + strlen(p));

  switch (c1) 
  {
    case _VAR:
    case _VARX:
      *context = _VAR;
      if (findvaltype(p) == _VALTYPE_NUM || progcode[inst-1].ftype == 1)
      {
        /* sprintf(ret, "awka_vardblset(%s, %s->dval)", r2, p);  */
        sprintf(ret, "awka_vardblset(%s, %s)", r2, getdoublevalue(p)); 
        setvaltype(r2, _VALTYPE_NUM);
        *context = _DBL;
      }
      else if (findvaltype(p) == _VALTYPE_STR || progcode[inst-1].ftype == 2)
      {
        if (0 && !strcmp(r2, "a_bivar[a_FS]")) {
           sprintf(ret, "awka_NFget(); awka_strcpy(%s, %s) /* 1 */", r2, getstringvalue(p));
        } else {
           const char *string_value = getstringvalue(p);
           if (strcmp(r2, "a_bivar[a_FS]") == 0 && string_value[0] == '\"') {
             sprintf(ret, "awka_strncpy(%s, %s, %d) /* 1 */", r2, string_value, getstringsize(string_value)-2);
	   } else if (strcmp(string_value, "\"\\0\"") == 0) {
             sprintf(ret, "awka_strncpy(%s, %s, 1) /* 1 */", r2, string_value);
           } else {
             sprintf(ret, "awka_strcpy(%s, %s) /* 1 */", r2, string_value);
           }
        }
        /* sprintf(ret, "awka_strcpy(%s, %s->ptr)", r2, p); */
        setvaltype(r2, _VALTYPE_STR);
        *context = _STR;
      }
      else
      {
        sprintf(ret, "awka_varcpy(%s, %s)", r2, p);
        setvaltype2(r2, p);
      }
      break;
    case _DBL:
    case _TRU:
      /* sprintf(ret, "awka_setd(%s) = %s", r2, p);  */
      sprintf(ret, "awka_vardblset(%s, %s)", r2, p); 
      setvaltype(r2, _VALTYPE_NUM);
      *context = _DBL;
      break;
    case _STR:
      if (0 && !strcmp(r2, "a_bivar[a_FS]")) {
        sprintf(ret, "awka_NFget(); awka_strcpy(%s, %s) /* 2 */", r2, p);
      } else {
        sprintf(ret, "awka_strcpy(%s, %s) /* 2 */", r2, p);
      }
      setvaltype(r2, _VALTYPE_STR);
      *context = _STR;
      break;
    default:
      awka_error("assign error: expecting var, str or dbl for rside, line %d.\n",progcode[inst].line);
  }

  free(p);
  free(q);
  free(r2);
  return ret;
}

char *
awka_call(int inst, int *earliest, char *context)
{
  char *ret, *p, c1, *r2, *r3, deref = FALSE;
  int prev = inst-1, prev2, i, j, len;

  if ((ret = test_previnst(inst, earliest, context, "call")) != NULL)
    return ret;
  test_loop(inst);

  check_gc();
  if (progcode[inst].op == _CALL)
    j = atoi(progcode[inst].arg);
  else
  {
    if (progcode[inst-1].op == _PUSHINT)
    {
      p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
      j = atoi(p);
      free(p);
    }
    else
      j = _a_bi_vararg[progcode[inst].varidx].min_args;
  }
  if (inst < j)
    awka_error("call error: expecting %d prior opcodes, line %d.\n",j,progcode[inst].line);

  len = strlen(progcode[inst].val);

  for (i=1; i<=j; i++)
  {
    if (i == 1)
    {
      prev2 = prev;
      p = (* progcode[prev].func)(prev, &prev, &c1);
      r2 = buildstr("call", "%s", p, c1, _ROVAR, inst, prev2);
      ret = (char *) malloc(strlen(r2)+4);
      sprintf(ret, "%s", r2);
      if (change_op[progcode[prev2].op]) deref = TRUE;
    }
    else
    {
      prev2 = prev;
      p = (* progcode[prev].func)(prev, &prev, &c1);
      if (deref == TRUE && push_func[progcode[prev2].op] && strncmp("_lit", p, 4))
        r2 = buildstr("call", "awka_vardup(%s)", p, c1, _ROVAR, inst, prev2);
      else
        r2 = buildstr("call", "%s", p, c1, _ROVAR, inst, prev2);
      if (change_op[progcode[prev2].op])
        deref = TRUE;
      r3 = (char *) malloc(len + strlen(r2) + strlen(ret) + 10);
      sprintf(r3, "%s, %s", r2, ret);
      ret = (char *) realloc(ret, strlen(r3)+1);
      strcpy(ret, r3);
      free(r3);
    }

    free(r2);
    free(p);
  }
  if (j)
  {
    r3 = (char *) malloc(strlen(progcode[inst].val) + strlen(ret) + 50);
    switch (j)
    {
      case 1:
        sprintf(r3, "%s_fn(awka_arg1(a_TEMP, %s))", progcode[inst].val, ret); break;
      case 2:
        sprintf(r3, "%s_fn(awka_arg2(a_TEMP, %s))", progcode[inst].val, ret); break;
      case 3:
        sprintf(r3, "%s_fn(awka_arg3(a_TEMP, %s))", progcode[inst].val, ret); break;
      default:
        sprintf(r3, "%s_fn(awka_vararg(a_TEMP, %s, NULL))", progcode[inst].val, ret);
    }
    ret = (char *) realloc(ret, strlen(r3)+1);
    strcpy(ret, r3);
    free(r3);
  }
  else
  {
    ret = (char *) malloc(strlen(progcode[inst].val)+50);
    sprintf(ret, "%s_fn(awka_arg0(a_TEMP))", progcode[inst].val);
  }

  *context = _VAR;
  *earliest = prev;
  return ret;
}

char *
awka_cat2(int inst, int *earliest, char *context, int *elements, char deref)
{
  char *ret, *r1, *r2;
  int len, prev2 = progcode[inst].prev2;

  *context = _ROVAR;
  *earliest = progcode[inst].earliest;
  *elements = *elements + 1;

  if (progcode[inst].code_used)
  {
    free(progcode[inst].code[0]);
    progcode[inst].code[0] = NULL;
    progcode[inst].code_used = 0;
  }
  progcode[inst].func = NULL;

  if (deref == TRUE && push_func[progcode[inst-1].op] && strncmp(progcode[inst].val, "_lit", 4))
    r2 = buildstr("cat", "awka_vardup(%s)", progcode[inst].val, _ROVAR, _ROVAR, inst, inst-1);
  else
    r2 = progcode[inst].val;
  if (change_op[progcode[inst-1].op]) deref = TRUE;

  if (progcode[inst].prevcat)
    r1 = awka_cat2(progcode[inst].prevcat, earliest, context, elements, deref);
  else
  {
    if (deref == TRUE && push_func[progcode[prev2].op] && strncmp(progcode[inst].arg, "_lit", 4))
      r1 = buildstr("cat", "awka_vardup(%s)", progcode[inst].arg, _ROVAR, _ROVAR, inst, prev2);
    else
      r1 = progcode[inst].arg;
  }

  len = strlen(r1) + strlen(r2) + 4;
  ret = (char *) malloc(len);
  sprintf(ret, "%s, %s", r1, r2);
  return ret;
}

char *
awka_cat(int inst, int *earliest, char *context)
{
  char *ret, *r2, *r3, *p, *q, c1, c2, deref = FALSE;
  int prev, prev2, elements = 2;

  if ((ret = test_previnst(inst, earliest, context, "cat")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 2)
    awka_error("cat error: expected two prior opcodes, line %d.\n",progcode[inst].line);

  check_gc();
  progcode[inst].prevcat = 0;
  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  progcode[inst].prev2 = prev2 = prev;

  if (change_op[progcode[inst-1].op])
    deref = TRUE;

  if (progcode[prev].op == _CAT)
  {
    progcode[inst].prevcat = prev;
    q = awka_cat2(prev, &prev, &c2, &elements, deref);
  }
  else
    q = (* progcode[prev].func)(prev, &prev, &c2);

  *earliest = prev;
  if (push_func[progcode[prev2].op] && change_op[progcode[inst-1].op] && strncmp("_lit", q, 4))
    progcode[inst].arg = r2 = buildstr("cat", "awka_vardup(%s)", q, c2, _ROVAR, inst, inst-1);
  else
    progcode[inst].arg = r2 = buildstr("cat", "%s", q, c2, _ROVAR, inst, inst-1);
  progcode[inst].val = r3 = buildstr("cat", "%s", p, c1, _ROVAR, inst, prev2);
  
  ret = (char *) malloc(strlen(r3) + strlen(r2) + 100);
  switch (elements)
  {
    case 2:
      sprintf(ret, "awka_strconcat2(a_TEMP, %s, %s)", r2, r3); break;
    case 3:
      sprintf(ret, "awka_strconcat3(a_TEMP, %s, %s)", r2, r3); break;
    case 4:
      sprintf(ret, "awka_strconcat4(a_TEMP, %s, %s)", r2, r3); break;
    case 5:
      sprintf(ret, "awka_strconcat5(a_TEMP, %s, %s)", r2, r3); break;
    default:
      sprintf(ret, "awka_strconcat(a_TEMP, awka_vararg(a_TEMP, %s, %s, NULL))", r2, r3);
  }
  *context = _VAR;

  free(p);
  free(q);
  return ret;
}

char *
awka_del_a(int inst, int *earliest, char *context)
{
  char *ret, *p, c1;
  int prev;

  if ((ret = test_previnst(inst, earliest, context, "del_a")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 1)
    awka_error("del_a error: expected a prior opcode, line %d.\n",progcode[inst].line);
  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  *earliest = prev;
  *context = _NUL;
  ret = buildstr("del_a", "awka_arrayclear(%s);\n", p, c1, _VAR, inst, inst-1);
  free(p);
  return ret;
}

char *
awka_div(int inst, int *earliest, char *context)
{
  char *ret, *p, *q, *r2, c1, c2;
  int prev, prev2;

  if ((ret = test_previnst(inst, earliest, context, "div")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 2)
    awka_error("div error: expected two prior opcodes, line %d.\n",progcode[inst].line);
  p = (* progcode[inst-1].func)(inst-1, &prev2, &c1);
  q = (* progcode[prev2].func)(prev2, &prev, &c2);
  *earliest = prev;

  ret = buildstr("div", "awka_div(%s, ", q, c2, _DBL, inst, prev2);
  r2 = buildstr("div", "%s)", p, c1, _DBL, inst, inst-1);
  ret = (char *) realloc(ret, strlen(ret) + strlen(r2) + 1);
  strcat(ret, r2);
  *context = _DBL;

  free(p);
  free(q);
  free(r2);
  return ret;
}

char *
awka_div_asg(int inst, int *earliest, char *context)
{
  char *ret, *p, *q, *r2, c1, c2;
  int prev;

  if ((ret = test_previnst(inst, earliest, context, "div_asg")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 2)
    awka_error("div_asg error: expected two prior opcodes, line %d.\n",progcode[inst].line);
  which_side = _a_RHS;
  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  which_side = _a_LHS;
  q = (* progcode[prev].func)(prev, &prev, &c2);
  which_side = _a_RHS;
  *earliest = prev;

  if (c2 != _VAR)
    awka_error("div_asg error: expecting var context for lside, line %d.\n",progcode[inst].line);

  ret = (char *) malloc( 50 + (strlen(q)*3) );
  /*
  sprintf(ret, "awka_vardblset(%s, awka_getd(%s) / ", q, q);
  r2 = buildstr("div_asg", "awka_dnotzero(%s))", p, c1, _DBL, inst, inst-1);
  */
  sprintf(ret, "awka_setd(%s) /= ", q); 
  setvaltype(q, _VALTYPE_NUM);
  r2 = buildstr("div_asg", "awka_dnotzero(%s)", p, c1, _DBL, inst, inst-1);

  ret = (char *) realloc(ret, strlen(ret) + strlen(r2) + 2);
  sprintf(ret, "%s%s",ret,r2);
  *context = _DBL;

  free(p);
  free(q);
  free(r2);
  return ret;
}

char *
awka_eq(int inst, int *earliest, char *context)
{
  char *ret, *p, *q, c1, c2;
  int prev, prev2, pprev, qprev;

  if ((ret = test_previnst(inst, earliest, context, "eq")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 2)
    awka_error("eq error: expected two prior opcodes, line %d.\n",progcode[inst].line);
  pprev = inst-1;
  p = (* progcode[inst-1].func)(inst-1, &prev2, &c1);
  qprev = prev2;
  q = (* progcode[prev2].func)(prev2, &prev, &c2);
  *earliest = prev;

  ret = (char *) malloc(100 + strlen(getdoublevalue(q)) + strlen(getdoublevalue(p))
                             + strlen(getstringvalue(q)) + strlen(getstringvalue(p))
                             + strlen(q) + strlen(p));

  switch (c2)
  {
    case _VAR:
      if (c1 == _VAR)
      {
        if ((findvaltype(q) == _VALTYPE_STR || progcode[prev2].ftype == 2) &&
            findvaltype(p) != _VALTYPE_NUM)
        {
          if (findvaltype(p) == _VALTYPE_STR || progcode[inst-1].ftype == 2)
            /* sprintf(ret, "!strcmp(%s->ptr, %s->ptr)", q, p); */
            sprintf(ret, "!strcmp(%s, %s)", getstringvalue(q), getstringvalue(p));
          else
            /* sprintf(ret, "!strcmp(%s->ptr, awka_gets(%s))", q, p); */
            sprintf(ret, "!strcmp(%s, awka_gets(%s))", getstringvalue(q), p);
        }
        else if ((findvaltype(p) == _VALTYPE_STR || progcode[inst-1].ftype == 2) 
                 && findvaltype(p) != _VALTYPE_NUM)
          /* sprintf(ret, "!strcmp(awka_gets(%s), %s->ptr)", q, p); */
          sprintf(ret, "!strcmp(awka_gets(%s), %s)", q, getstringvalue(p));
        else if ((findvaltype(p) == _VALTYPE_NUM || progcode[pprev].ftype == 1) &&
                 (findvaltype(q) == _VALTYPE_NUM || progcode[qprev].ftype == 1))
          sprintf(ret, "%s == %s", getdoublevalue(q), getdoublevalue(p));
          /* sprintf(ret, "%s->dval == %s->dval", q, p); */
        else if (findvaltype(p) == _VALTYPE_NUM || progcode[pprev].ftype == 1)
          sprintf(ret, "(awka_var2dblcmp(%s, %s) == 0)", q, getdoublevalue(p));
          /* sprintf(ret, "(awka_var2dblcmp(%s, %s->dval) == 0)", q, p); */
        else if (findvaltype(q) == _VALTYPE_NUM || progcode[qprev].ftype == 1)
          sprintf(ret, "(awka_dbl2varcmp(%s, %s) == 0)", getdoublevalue(q), p);
          /* sprintf(ret, "(awka_dbl2varcmp(%s->dval, %s) == 0)", q, p);*/
        else
          sprintf(ret, "!awka_varcmp(%s, %s)", q, p);
      }
      else if (c1 == _DBL)
      {
        if (findvaltype(q) == _VALTYPE_NUM || progcode[inst-1].ftype == 1)
        {
          if (assign_op[progcode[inst-1].op])
            sprintf(ret, "%s == (%s)", getdoublevalue(q), p);
            /* sprintf(ret, "%s->dval == (%s)", q, p); */
          else
            sprintf(ret, "%s == %s", getdoublevalue(q), p);
            /* sprintf(ret, "%s->dval == %s", q, p); */
        }
        else
          sprintf(ret, "!awka_var2dblcmp(%s, %s)", q, p);
      }
      else
      {
        if (findvaltype(q) == _VALTYPE_STR || progcode[prev2].ftype == 2)
          sprintf(ret, "!strcmp(%s, %s)", getstringvalue(q), p);
          /* sprintf(ret, "!strcmp(%s->ptr, %s)", q, p); */
        else if (push_func[progcode[prev2].op])
          sprintf(ret, "!strcmp(awka_gets1(%s), %s)", q, p);
        else
          sprintf(ret, "!strcmp(awka_gets(%s), %s)", q, p);
      }
      break;

    case _STR:
      if (c1 == _VAR)
      {
        if (findvaltype(p) == _VALTYPE_STR || progcode[inst-1].ftype == 2)
          sprintf(ret, "!strcmp(%s, %s)", q, getstringvalue(p));
          /* sprintf(ret, "!strcmp(%s, %s->ptr)", q, p); */
        else if (push_func[progcode[inst-1].op])
          sprintf(ret, "!strcmp(%s, awka_gets1(%s))", q, p);
        else
          sprintf(ret, "!strcmp(%s, awka_gets(%s))", q, p);
      }
      else if (c1 == _DBL)
        sprintf(ret, "!strcmp(%s, awka_tmp_dbl2str(%s))", q, p);
      else
        sprintf(ret, "!strcmp(%s, %s)", q, p);
      break;

    case _DBL:
    case _TRU:
      if (c1 == _VAR)
      {
        if (findvaltype(p) == _VALTYPE_NUM || progcode[prev2].ftype == 1)
        {
          if (assign_op[progcode[prev2].op])
            sprintf(ret, "(%s) == %s", q, getdoublevalue(p));
            /* sprintf(ret, "(%s) == %s->dval", q, p); */
          else
            sprintf(ret, "%s == %s", q, getdoublevalue(p));
            /* sprintf(ret, "%s == %s->dval", q, p); */
        }
        else
          sprintf(ret, "!awka_dbl2varcmp(%s, %s)", q, p);
      }
      else if (c1 == _DBL)
      {
        if (assign_op[progcode[prev2].op])
          sprintf(ret, "(%s) ==", q);
        else
          sprintf(ret, "%s ==", q);
        if (assign_op[progcode[inst-1].op])
          sprintf(ret, "%s (%s)", ret, p);
        else
          sprintf(ret, "%s %s", ret, p);
      }
      else
      {
        check_gc();
        sprintf(ret, "!strcmp(awka_tmp_dbl2str(%s), %s)", q, p);
      }
      break;
      
    default:
      awka_error("eq error: expecting var, str or dbl context for lside argument, line %d.\n",progcode[inst].line);
  }

  *context = _DBL;
  free(p);
  free(q);
  return ret;
}

char *
awka_exit(int inst, int *earliest, char *context)
{
  char *ret, *p, c1;
  int prev, i;

  if ((ret = test_previnst(inst, earliest, context, "exit")) != NULL)
    return ret;
  revert_gc();
  test_loop(inst);
  if (inst < 1)
    awka_error("exit error: expected a prior opcode, line %d.\n",progcode[inst].line);
  if (al_count)
  {
    for (i=0; i<al_count; i++)
    {
      p = codeptr(inst, 50);
      sprintf(p, "awka_alistfreeall(&_alh%d);\n",i);
    }
  }
  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  ret = buildstr("exit", "awka_exit_val = %s;  return;\n", p, c1, _DBL, inst, inst-1);
  free(p);
  *earliest = prev;
  *context = _NUL;

  return ret;
}

char *
awka_exit0(int inst, int *earliest, char *context)
{
  char *ret, *p;
  int i;

  if ((ret = test_previnst(inst, earliest, context, "exit0")) != NULL)
    return ret;
  revert_gc();
  test_loop(inst);
  if (al_count)
  {
    for (i=0; i<al_count; i++)
    {
      p = codeptr(inst, 50);
      sprintf(p, "awka_alistfreeall(&_alh%d);\n",i);
    }
  }
  ret = (char *) malloc( 60 );
  if (awka_main) {
    if (progcode[inst].op == _CLEANUP)
      strcpy(ret, "return ((awka_exit_val < 0) ? 0 : awka_exit_val);\n");
    else
      strcpy(ret, "awka_exit((awka_exit_val < 0) ? 0 : awka_exit_val);\n");
  }
  else if (mode == BEGIN) {
      strcpy(ret, "awka_exit_val = 0;\n  return;\n");
  }
  else {
    if (end_used == TRUE && mode != END)
      strcpy(ret, "awka_exit_val = 0;\n  return;\n");
    else
      strcpy(ret, "awka_exit((awka_exit_val < 0) ? 0 : awka_exit_val);\n");
  }
  *earliest = inst-1;
  *context = _NUL;

  return ret;
}

char *
awka_jnz(int inst, int *earliest, char *context)
{
  /* a do/while, a while or a for loop */
  char *p, c1, c2, *ret, *r2, *r3=NULL, *r;
  int prev, i=0, prev2;

  if ((ret = test_previnst(inst, earliest, context, "jnz")) != NULL)
    return ret;
  test_loop(inst);
  revert_gc();
  if (inst == 0)
    awka_error("jnz error: expecting a prior opcode.\n");

  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  r2 = buildstr("jnz", "%s", p, c1, _TRU, inst, inst-1);

  if (progcode[inst].ljumpfrom != -1)
  {
    free(p); prev2 = prev;
    p = (* progcode[prev].func)(prev, &prev, &c2);
    r3 = buildstr("jnz", "%s", p, c2, _TRU, inst, prev2);
  }

  if (progcode[progcode[inst].jumpto].doloop == FALSE)
  {
    /* while loop */
    i = progcode[inst].jumpto;
    if (r3)
    {
      r = code0ptr(i, strlen(r2) + strlen(r3) + 20);
      sprintf(r, "while (%s%s)\n",r3,r2);
      ret = (char *) malloc(3);
      strcpy(ret, "{\n");
      free(r3);
    }
    else
    {
      r = code0ptr(i, strlen(r2) + 20);
      sprintf(r, "while (%s)\n",r2);
      ret = (char *) malloc(3);
      strcpy(ret, "{\n");
    }
    r = code0ptr(i, strlen(ret)+1);
    strcpy(r, ret);
    free(ret);
    ret = (char *) malloc(4);
    sprintf(ret, "}\n");
  }
  else
  {
    /* do/while loop */
    if (r3)
    {
      ret = (char *) malloc(strlen(r2) + strlen(r3) + 20);
      sprintf(ret, "} while (%s%s);\n",r3,r2);
      free(r3);
    }
    else
    {
      ret = (char *) malloc( strlen(r2) + 20);
      sprintf(ret, "} while (%s);\n",r2);
    }
  }
  free(p); free(r2);
    
  *earliest = prev;
  *context = _NUL;
  return ret;
}

char *
awka_jz(int inst, int *earliest, char *context)
{
  /* an if statement, qmark, or a range pattern */
  char *ret, *p, *q, c1, c2, *r2, *r3, *r;
  int prev, prev2;

  if ((ret = test_previnst(inst, earliest, context, "jz")) != NULL)
    return ret;
  revert_gc();
  test_loop(inst);
  if (inst == 0)
    awka_error("jz error: expecting a prior opcode.\n");

  *context = _NUL;
  if (progcode[inst].op == _STOP || progcode[inst].op == _QMARK)
    *context = _TRU;

  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  if (progcode[inst].ljumpfrom != -1)
  {
    prev2 = prev;
    q = (* progcode[prev].func)(prev, &prev, &c2);
    r2 = buildstr("jz", "%s", p, c1, _TRU, inst, inst-1);
    r3 = buildstr("jz", "%s", q, c2, _TRU, inst, prev2);
    ret = (char *) malloc( strlen(r2) + strlen(r3) + 15 );
    if (progcode[inst].op == _JZ)
    {
      r = codeptr(inst, strlen(r2) + strlen(r3) + 15);
      sprintf(r, "if (%s%s)\n", r3, r2);
      strcpy(ret, "{\n");
    }
    else
    {
      sprintf(ret, "(%s%s)", r3, r2);
      *context = _TRU;
    }
    free(r2); free(r3); free(q);
  }
  else
  {
    if (progcode[inst].op == _JZ)
    {
      ret = buildstr("jz", "if (%s)\n",p, c1, _TRU, inst, inst-1);
      r = codeptr(inst, strlen(ret)+1);
      strcpy(r, ret);
      strcpy(ret, "{\n");
    }
    else
      ret = buildstr("qmark", "(%s)",p, c1, _TRU, inst, inst-1);
    *context = _TRU;
  }

  *earliest = prev;

  free(p);
  return ret;
}

char *
awka_ljnz(int inst, int *earliest, char *context)
{
  char *ret, *p, c1;
  int prev;

  if ((ret = test_previnst(inst, earliest, context, "ljnz")) != NULL)
    return ret;
  test_loop(inst);
  revert_gc();
  if (inst < 2)
    awka_error("ljnz error: expected at least two prior opcodes, line %d.\n",progcode[inst].line);
  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  *earliest = prev;

  ret = buildstr("ljnz", "(%s) || ", p, c1, _TRU, inst, inst-1);
  *context = _TRU;
  free(p);
  return ret;
}

char *
awka_ljz(int inst, int *earliest, char *context)
{
  char *ret, *p, c1;
  int prev;

  if ((ret = test_previnst(inst, earliest, context, "ljz")) != NULL)
    return ret;
  test_loop(inst);
  revert_gc();
  if (inst < 2)
    awka_error("ljz error: expected at least two prior opcodes, line %d.\n",progcode[inst].line);
  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  *earliest = prev;

  ret = buildstr("ljz", "(%s) && ", p, c1, _TRU, inst, inst-1);
  *context = _TRU;
  free(p);
  return ret;
}

char *
awka_expr(int inst, int *earliest, char *context)
{
  char *ret, *p, *q, c1, c2;
  int prev, prev2;

  if ((ret = test_previnst(inst, earliest, context, "lt")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 2)
    awka_error("expr error: expected two prior opcodes, line %d.\n",progcode[inst].line);
  p = (* progcode[inst-1].func)(inst-1, &prev2, &c1);
  q = (* progcode[prev2].func)(prev2, &prev, &c2);
  *earliest = prev;

  switch (progcode[inst].op)
  {
    case _GT:
      ret = awka_buildexpr(">", p, q, inst-1, prev2, c1, c2, inst); break;
    case _GTE:
      ret = awka_buildexpr(">=", p, q, inst-1, prev2, c1, c2, inst); break;
    case _LT:
      ret = awka_buildexpr("<", p, q, inst-1, prev2, c1, c2, inst); break;
    case _LTE:
      ret = awka_buildexpr("<=", p, q, inst-1, prev2, c1, c2, inst); break;
    case _EQ:
      ret = awka_buildexpr("==", p, q, inst-1, prev2, c1, c2, inst); break;
    case _NEQ:
      ret = awka_buildexpr("!=", p, q, inst-1, prev2, c1, c2, inst); break;
  }

  *context = _DBL;
  free(p);
  free(q);
  return ret;
}

char *
awka_match(int inst, int *earliest, char *context)
{
  /* call to match function */
  char *ret, *p, *q, *r, c1, c2, c3, *r2=NULL, *r3=NULL, *r4=NULL;
  int i, j, prev2, prev3, prev = inst-1;

  if ((ret = test_previnst(inst, earliest, context, "match")) != NULL)
    return ret;

  test_loop(inst);
  check_gc();

  if (progcode[inst].op == _MATCH1 ||
      progcode[inst].op == _MATCH0)
  {
    if (inst < 2)
      awka_error("match1 error: expected a prior opcode, line %d.\n",progcode[inst].line);
    r3 = (char *) malloc(strlen(progcode[inst].arg) + 3);
    strcpy(r3, progcode[inst].arg);
  }

  switch (progcode[inst].op)
  {
    case _MATCH0:
      ret = (char *) malloc( strlen(r3) + 100);
      sprintf(ret, "awka_match(a_TEMP, FALSE, awka_dol0(a_DOL_GET), %s, NULL)",r3);
      setvaltype(r3, _VALTYPE_RE);
      dol0_used = 1;
      break;

    case _MATCH1:
      q = (* progcode[inst-1].func)(inst-1, &prev, &c2);
      r2 = buildstr("match1", "%s", q, c2, _VAR, inst, inst-1);
      ret = (char *) malloc( strlen(r2) + strlen(r3) + 70);
      sprintf(ret, "awka_match(a_TEMP, FALSE, %s, %s, NULL) /* 1 */",r2,r3);
      setvaltype(r3, _VALTYPE_RE);
      free(q);
      break;

    case _MATCH2:
      p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
      prev2 = prev;
      q = (* progcode[prev].func)(prev, &prev, &c2);
      r2 = buildstr("match", "%s", q, c2, _VAR, inst, inst-1);
      r3 = buildstr("match", "%s", p, c1, _VAR, inst, prev2);
      ret = (char *) malloc( strlen(r2) + strlen(r3) + 70);
      sprintf(ret, "awka_match(a_TEMP, TRUE, %s, %s, NULL)",r2,r3);
      setvaltype(r3, _VALTYPE_RE);
      free(q); free(p);
      break;

    case _MATCH:
    case _BUILTIN:
      if (inst < 3)
        awka_error("match error: expected at least three prior opcodes, line %d.\n",progcode[inst].line);

      if (progcode[inst-1].op != _PUSHINT)
        awka_error("printf error: expecting prior pushint opcode, got %d, line %d.\n",progcode[inst-1].op,progcode[inst].line);

      if (progcode[inst-1].val == NULL)
      {
        if (progcode[inst-1].arg == NULL)
          awka_error("printf error: expected 'pushint number' as prior opcode, line %d.\n",progcode[inst].line);
        j = atoi(progcode[inst-1].arg);
      }
      else
        j = atoi(progcode[inst-1].val); 
      progcode[inst-1].func = NULL;

      if (j == 3)
      {
        p = (* progcode[inst-2].func)(inst-2, &prev, &c1);
        prev2 = prev;
        q = (* progcode[prev].func)(prev, &prev, &c2);
        prev3 = prev;
        r = (* progcode[prev].func)(prev, &prev, &c3);
        r4 = buildstr("match", "%s", p, c1, _NUL, inst, inst-1);
        r3 = buildstr("match", "%s", q, c2, _VAR, inst, prev2);
        r2 = buildstr("match", "%s", r, c3, _VAR, inst, prev3);
        ret = (char *) malloc( strlen(r2) + strlen(r3) + strlen(r4) + 70);
        sprintf(ret, "awka_match(a_TEMP, TRUE, %s, %s, %s)",r2,r3,r4);
        setvaltype(r3, _VALTYPE_RE);
        free(r);
      }
      else
      {
        p = (* progcode[inst-2].func)(inst-2, &prev, &c1);
        prev2 = prev;
        q = (* progcode[prev].func)(prev, &prev, &c2);
        r4 = buildstr("match", "%s", q, c1, _VAR, inst, inst-1);
        r3 = buildstr("match", "%s", p, c2, _VAR, inst, prev2);
        ret = (char *) malloc( strlen(r3) + strlen(r4) + 70);
        sprintf(ret, "awka_match(a_TEMP, TRUE, %s, %s, NULL)",r4,r3);
        setvaltype(r4, _VALTYPE_RE);
      }
      free(q); free(p);
      break;
  }

  if (r2) free(r2);
  if (r3) free(r3);
  if (r4) free(r4);

  *earliest = prev;
  *context = _VAR;
  return ret;
}

char *
awka_mod(int inst, int *earliest, char *context)
{
  char *ret, *p, *q, *r2, c1, c2;
  int prev, prev2;

  if ((ret = test_previnst(inst, earliest, context, "mod")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 2)
    awka_error("mod error: expected two prior opcodes, line %d.\n",progcode[inst].line);
  p = (* progcode[inst-1].func)(inst-1, &prev2, &c1);
  q = (* progcode[prev2].func)(prev2, &prev, &c2);
  *earliest = prev;

  ret = buildstr("mod", "fmod(%s, ", q, c2, _DBL, inst, prev2);
  r2 = buildstr("mod", "%s)", p, c1, _DBL, inst, inst-1);

  ret = (char *) realloc(ret, strlen(ret) + strlen(r2) + 1);
  strcat(ret, r2);
  *context = _DBL;

  free(p);
  free(q);
  free(r2);
  return ret;
}

char *
awka_mod_asg(int inst, int *earliest, char *context)
{
  char *ret, *p, *q, *r2, c1, c2;
  int prev, prev2;

  if ((ret = test_previnst(inst, earliest, context, "mod_asg")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 2)
    awka_error("mod_asg error: expected two prior opcodes, line %d.\n",progcode[inst].line);
  which_side = _a_RHS;
  p = (* progcode[inst-1].func)(inst-1, &prev2, &c1);
  which_side = _a_LHS;
  q = (* progcode[prev2].func)(prev2, &prev, &c2);
  which_side = _a_RHS;
  *earliest = prev;

  if (c2 != _VAR)
    awka_error("mod_asg error: expecting var context for lside, line %d.\n",progcode[inst].line);

  ret = (char *) malloc( 50 + (strlen(q)*2) );

  /* sprintf(ret, "awka_setd(%s) = fmod(awka_getd(%s), ", q, q);  */
  sprintf(ret, "awka_vardblset(%s, fmod(awka_getd(%s), ", q, q); 
  setvaltype(q, _VALTYPE_NUM);
  r2 = buildstr("mod_asg", "%s)", p, c1, _DBL, inst, inst-1);
  ret = (char *) realloc(ret, strlen(ret) + strlen(r2) + 2);
  sprintf(ret, "%s%s)",ret,r2);
  *context = _DBL;
  free(p);
  free(q);
  free(r2);
  return ret;
}

char *
awka_mul(int inst, int *earliest, char *context)
{
  char *ret, *p, *q, *r2, c1, c2;
  int prev, prev2;

  if ((ret = test_previnst(inst, earliest, context, "mul")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 2)
    awka_error("mul error: expected two prior opcodes, line %d.\n",progcode[inst].line);
  p = (* progcode[inst-1].func)(inst-1, &prev2, &c1);
  q = (* progcode[prev2].func)(prev2, &prev, &c2);
  *earliest = prev;

  if (push_func[progcode[prev2].op] && push_func[progcode[inst-1].op])
    ret = buildstr("mul", "(%s * ", q, c2, _DBL, inst, prev2);
  else
    ret = buildstr("mul", "((%s) * ", q, c2, _DBL, inst, prev2);
  if (push_func[progcode[inst-1].op])
    r2 = buildstr("mul", "%s)", p, c1, _DBL, inst, inst-1);
  else
    r2 = buildstr("mul", "(%s))", p, c1, _DBL, inst, inst-1);

  ret = (char *) realloc(ret, strlen(ret) + strlen(r2) + 1);
  strcat(ret, r2);
  *context = _DBL;

  free(p);
  free(q);
  free(r2);
  return ret;
}

char *
awka_mul_asg(int inst, int *earliest, char *context)
{
  char *ret, *p, *q, *r2, c1, c2;
  int prev;

  if ((ret = test_previnst(inst, earliest, context, "mul_asg")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 2)
    awka_error("mul_asg error: expected two prior opcodes, line %d.\n",progcode[inst].line);
  which_side = _a_RHS;
  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  which_side = _a_LHS;
  q = (* progcode[prev].func)(prev, &prev, &c2);
  which_side = _a_RHS;
  *earliest = prev;

  if (c2 != _VAR)
    awka_error("mul_asg error: expecting var context for lside, line %d.\n",progcode[inst].line);

  ret = (char *) malloc( 50 + (strlen(q)*2) );
  /*
  sprintf(ret, "awka_vardblset(%s, awka_getd(%s) * ", q, q);
  r2 = buildstr("mul_asg", "%s)", p, c1, _DBL, inst, inst-1);
  */
  sprintf(ret, "awka_setd(%s) *= ", q); 
  setvaltype(q, _VALTYPE_NUM);
  r2 = buildstr("mul_asg", "%s", p, c1, _DBL, inst, inst-1);

  ret = (char *) realloc(ret, strlen(ret) + strlen(r2) + 2);
  sprintf(ret, "%s%s",ret,r2);
  *context = _DBL;

  free(p);
  free(q);
  free(r2);
  return ret;
}

char *
awka_neq(int inst, int *earliest, char *context)
{
  char *ret, *p, *q, c1, c2;
  int prev, prev2, pprev, qprev;

  if ((ret = test_previnst(inst, earliest, context, "neq")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 2)
    awka_error("neq error: expected two prior opcodes, line %d.\n",progcode[inst].line);
  pprev = inst-1;
  p = (* progcode[inst-1].func)(inst-1, &prev2, &c1);
  qprev = prev2;
  q = (* progcode[prev2].func)(prev2, &prev, &c2);
  *earliest = prev;

  ret = (char *) malloc(100 + strlen(getdoublevalue(q)) + strlen(getdoublevalue(p))
                            + strlen(getstringvalue(q)) + strlen(getstringvalue(p))
                            + strlen(q) + strlen(p));
  
  switch (c2)
  {
    case _VAR:
      if (c1 == _VAR)
      {
        if (findvaltype(p) == _VALTYPE_STR && findvaltype(q) == _VALTYPE_STR)
          sprintf(ret, "strcmp(%s, %s)",getstringvalue(q), getstringvalue(p));
          /* sprintf(ret, "strcmp(%s->ptr, %s->ptr)",q, p); */
        else if ((findvaltype(p) == _VALTYPE_NUM || progcode[pprev].ftype == 1) &&
                 (findvaltype(q) == _VALTYPE_NUM || progcode[qprev].ftype == 1))
          sprintf(ret, "%s != %s", getdoublevalue(q), getdoublevalue(p)); 
          /* sprintf(ret, "%s->dval != %s->dval", q, p); */
        else if ((findvaltype(p) == _VALTYPE_NUM || progcode[pprev].ftype == 1))
          sprintf(ret, "(awka_var2dblcmp(%s, %s) != 0)", q, getdoublevalue(p));
          /* sprintf(ret, "(awka_var2dblcmp(%s, %s->dval) != 0)", q, p); */
        else if ((findvaltype(q) == _VALTYPE_NUM || progcode[qprev].ftype == 1))
          sprintf(ret, "(awka_dbl2varcmp(%s, %s) != 0)", getdoublevalue(q), p);
          /* sprintf(ret, "(awka_dbl2varcmp(%s->dval, %s) != 0)", q, p); */
        else if (findvaltype(q) == _VALTYPE_STR)
          sprintf(ret, "strcmp(%s, awka_gets(%s))", getstringvalue(q), p);
          /* sprintf(ret, "strcmp(%s->ptr, awka_gets(%s))", q, p); */
        else if (findvaltype(p) == _VALTYPE_STR)
          sprintf(ret, "strcmp(awka_gets(%s), %s)", q, getstringvalue(p));
          /* sprintf(ret, "strcmp(awka_gets(%s), %s->ptr)", q, p); */
        else
          sprintf(ret, "awka_varcmp(%s, %s)", q, p);
      }
      else if (c1 == _DBL)
      {
        if (findvaltype(q) == _VALTYPE_NUM)
        {
          if (assign_op[progcode[inst-1].op])
            sprintf(ret, "%s != (%s)", getdoublevalue(q), p);
            /* sprintf(ret, "%s->dval != (%s)", q, p); */
          else
            sprintf(ret, "%s != %s", getdoublevalue(q), p);
            /* sprintf(ret, "%s->dval != %s", q, p); */
        }
        else
          sprintf(ret, "awka_var2dblcmp(%s, %s)", q, p);
      }
      else
      {
        if (findvaltype(q) == _VALTYPE_STR)
          sprintf(ret, "strcmp(%s, %s)", getstringvalue(q), p);
          /* sprintf(ret, "strcmp(%s->ptr, %s)", q, p); */
        else if (push_func[progcode[prev2].op])
          sprintf(ret, "strcmp(awka_gets1(%s), %s)", q, p);
        else
          sprintf(ret, "strcmp(awka_gets(%s), %s)", q, p);
      }
      break;

    case _STR:
      if (c1 == _VAR)
      {
        if (findvaltype(p) == _VALTYPE_STR)
          sprintf(ret, "strcmp(%s, %s)", q, getstringvalue(p));
          /* sprintf(ret, "strcmp(%s, %s->ptr)", q, p); */
        else if (push_func[progcode[inst-1].op])
          sprintf(ret, "strcmp(%s, awka_gets1(%s))", q, p);
        else
          sprintf(ret, "strcmp(%s, awka_gets(%s))", q, p);
      }
      else if (c1 == _DBL)
      {
        check_gc();
        sprintf(ret, "strcmp(%s, awka_tmp_dbl2str(%s))", q, p);
      }
      else
        sprintf(ret, "strcmp(%s, %s)", q, p);
      break;

    case _DBL:
    case _TRU:
      if (c1 == _VAR)
      {
        if (findvaltype(p) == _VALTYPE_NUM || progcode[prev2].ftype == 1)
        {
          if (assign_op[progcode[prev2].op])
            sprintf(ret, "(%s) != %s", q, getdoublevalue(p));
            /* sprintf(ret, "(%s) != %s->dval", q, p); */
          else
            sprintf(ret, "%s != %s", q, getdoublevalue(p));
            /* sprintf(ret, "%s != %s->dval", q, p); */
        }
        else
          sprintf(ret, "awka_dbl2varcmp(%s, %s)", q, p);
      }
      else if (c1 == _DBL)
      {
        if (assign_op[progcode[prev2].op])
          sprintf(ret, "(%s) !=", q);
        else
          sprintf(ret, "%s !=", q);
        if (assign_op[progcode[inst-1].op])
          sprintf(ret, "%s (%s)", ret, p);
        else
          sprintf(ret, "%s %s", ret, p);
      }
      else
      {
        check_gc();
        sprintf(ret, "strcmp(awka_tmp_dbl2str(%s), %s)", q, p);
      }
      break;
      
    default:
      awka_error("neq error: expecting var, str or dbl context for lside argument, line %d.\n",progcode[inst].line);
  }

  *context = _DBL;
  free(p);
  free(q);
  return ret;
}

char *
awka_next(int inst, int *earliest, char *context)
{
  char *ret, *p;
  int i;

  if ((ret = test_previnst(inst, earliest, context, "next")) != NULL)
    return ret;
  test_loop(inst);
  revert_gc();
  if (al_count)
  {
    for (i=0; i<al_count; i++)
    {
      p = codeptr(inst, 50);
      sprintf(p, "awka_alistfreeall(&_alh%d);\n",i);
    }
  }
  ret = (char *) malloc(30);
  if (mode == MAIN)
    strcpy(ret, "goto nextrec;\n");
  else
    strcpy(ret, "longjmp(context, 1);\n");
  *context = _NUL;
  *earliest = inst-1;
  return ret;
}

char *
awka_nextfile(int inst, int *earliest, char *context)
{
  char *ret, *p;
  int i;

  if ((ret = test_previnst(inst, earliest, context, "next")) != NULL)
    return ret;
  test_loop(inst);
  revert_gc();
  ret = codeptr(inst, 40);
  sprintf(ret, "_awka_file_read = TRUE;\n");
  if (al_count)
  {
    for (i=0; i<al_count; i++)
    {
      p = codeptr(inst, 50);
      sprintf(p, "awka_alistfreeall(&_alh%d);\n",i);
    }
  }
  ret = (char *) malloc(30);
  if (mode == MAIN)
    strcpy(ret, "goto nextrec;\n");
  else
    strcpy(ret, "longjmp(context, 1);\n");
  *context = _NUL;
  *earliest = inst-1;
  return ret;
}

char *
awka_not(int inst, int *earliest, char *context)
{
  char *ret, *p, c;
  int prev;

  if ((ret = test_previnst(inst, earliest, context, "not")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 1)
    awka_error("not error: expected a prior opcode, line %d.\n",progcode[inst].line);
  p = (* progcode[inst-1].func)(inst-1, &prev, &c);
  *earliest = prev;
  *context = _DBL;

  ret = buildstr("not", "!(%s)",p, c, _TRU, inst, inst-1);
  free(p);
  return ret;
}

char *
awka_ol_gl(int inst, int *earliest, char *context)
{
  /* end of main while loop in MAIN function */
  char *ret, *r2;
  int i;

  if ((ret = test_previnst(inst, earliest, context, "ol_gl")) != NULL)
    return ret;
  test_loop(inst);
  revert_gc();

  r2 = codeptr(inst, 20);
  strcpy(r2, "nextrec:;\n");
  for (i=0; i<range_no; i++)
  {
    r2 = codeptr(inst, 50);
    sprintf(r2, "_range%d = (_range%d == 2 ? 0 : _range%d);\n",i,i,i);
  }
  r2 = codeptr(inst, 4);
  strcpy(r2, "}\n");
  ret = (char *) malloc(4);
  strcpy(ret, "}");
  *earliest = inst-1;
  *context = _NUL;
  return ret;
}

char *
awka_pop(int inst, int *earliest, char *context)
{
  char *ret, *r2;
  int i;

  if ((ret = test_previnst(inst, earliest, context, "pop")) != NULL)
    return ret;
  revert_gc();

  switch (progcode[inst].op)
  {
    case _BREAK:
      test_loop(inst);
      ret = (char *) malloc(10);
      strcpy(ret, "break;\n");
      break;

    case _POP:
      ret = (char *) malloc(10 + (4 * progcode[inst].endloop));
      if (progcode[inst].endloop)
      {
        r2 = codeptr(inst, 3);
        strcpy(r2, ";\n");
        strcpy(ret, "}\n");
        for (i=1; i<progcode[inst].endloop; i++)
          sprintf(ret, "%s}\n",ret);
      }
      else
        strcpy(ret, ";\n");
      progcode[inst].endloop = 0;
  }

  *earliest = inst-1;
  *context = _NUL;
  return ret;
}

char *
awka_post_dec(int inst, int *earliest, char *context)
{
  char *ret, *p, c;
  int prev;

  if ((ret = test_previnst(inst, earliest, context, "post_dec")) != NULL)
  {
    if (which_side == _a_LHS)
    {
      p = (char *) malloc( 20 + strlen(ret) );
      sprintf(p, "awka_setd(%s)--", progcode[inst].arg); 
      setvaltype(p, _VALTYPE_NUM);
      free(ret);
      ret = p;
    }
    return ret;
  }
  test_loop(inst);
  if (inst < 1)
    awka_error("post_dec error: expected a prior opcode, line %d.\n",progcode[inst].line);
  p = (* progcode[inst-1].func)(inst-1, &prev, &c);
  *earliest = prev;

  if (c != _VAR)
    awka_error("post_dec error: expecting var context for rside, line %d.\n",progcode[inst].line);

  ret = (char *) malloc( 20 + strlen(p) );
  if (findvaltype(p) == _VALTYPE_NUM &&
     (inst + 1 >= prog_no || progcode[inst+1].op == _POP))
    sprintf(ret, "awka_pod(%s)", p);
  else
    sprintf(ret, "awka_postdec(%s)", p);
  if (progcode[inst].arg) free(progcode[inst].arg);
  progcode[inst].arg = p;
  *context = _DBL;
  return ret;
}

char *
awka_post_inc(int inst, int *earliest, char *context)
{
  char *ret, *p, c;
  int prev;

  if ((ret = test_previnst(inst, earliest, context, "post_inc")) != NULL)
  {
    if (which_side == _a_LHS)
    {
      p = (char *) malloc( 20 + strlen(ret) );
      sprintf(p, "awka_setd(%s)++", progcode[inst].arg); 
      setvaltype(p, _VALTYPE_NUM);
      free(ret);
      ret = p;
    }
    return ret;
  }
  test_loop(inst);
  if (inst < 1)
    awka_error("post_inc error: expected a prior opcode, line %d.\n",progcode[inst].line);
  p = (* progcode[inst-1].func)(inst-1, &prev, &c);
  *earliest = prev;

  if (c != _VAR)
    awka_error("post_inc error: expecting var context for rside, line %d.\n",progcode[inst].line);

  ret = (char *) malloc( 20 + strlen(p) );
  if (findvaltype(p) == _VALTYPE_NUM &&
     (inst + 1 >= prog_no || progcode[inst+1].op == _POP))
    sprintf(ret, "awka_poi(%s)", p);
  else
    sprintf(ret, "awka_postinc(%s)", p); 
  if (progcode[inst].arg) free(progcode[inst].arg);
  progcode[inst].arg = p;
  *context = _DBL;
  return ret;
}

char *
awka_pow(int inst, int *earliest, char *context)
{
  char *ret, *p, *q, *r2, c1, c2;
  int prev, prev2;

  if ((ret = test_previnst(inst, earliest, context, "pow")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 2)
    awka_error("pow error: expected two prior opcodes, line %d.\n",progcode[inst].line);
  p = (* progcode[inst-1].func)(inst-1, &prev2, &c1);
  q = (* progcode[prev2].func)(prev2, &prev, &c2);
  *earliest = prev;

  ret = buildstr("pow", "pow(%s, ", q, c2, _DBL, inst, prev2);
  r2 = buildstr("pow", "%s)", p, c1, _DBL, inst, inst-1);
  ret = (char *) realloc(ret, strlen(ret) + strlen(r2) + 1);
  strcat(ret, r2);
  *context = _DBL;

  free(p);
  free(q);
  free(r2);
  return ret;
}

char *
awka_pow_asg(int inst, int *earliest, char *context)
{
  char *ret, *p, *q, *r2, c1, c2;
  int prev, prev2;

  if ((ret = test_previnst(inst, earliest, context, "pow_asg")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 2)
    awka_error("pow_asg error: expected two prior opcodes, line %d.\n",progcode[inst].line);
  which_side = _a_RHS;
  p = (* progcode[inst-1].func)(inst-1, &prev2, &c1);
  which_side = _a_LHS;
  q = (* progcode[prev2].func)(prev2, &prev, &c2);
  which_side = _a_RHS;
  *earliest = prev;

  if (c2 != _VAR)
    awka_error("pow_asg error: expecting var context for lside, line %d.\n",progcode[inst].line);

  ret = (char *) malloc( 50 + (strlen(q)*2) );
  /* sprintf(ret, "awka_setd(%s) = pow(awka_getd(%s), ", q, q); */
  sprintf(ret, "awka_vardblset(%s, pow(awka_getd(%s), ", q, q);
  setvaltype(q, _VALTYPE_NUM);
  r2 = buildstr("pow_asg", "%s))", p, c1, _DBL, inst, inst-1);
  ret = (char *) realloc(ret, strlen(ret) + strlen(r2) + 1);
  strcat(ret, r2);
  *context = _DBL;
  free(p);
  free(q);
  free(r2);
  return ret;
}

char *
awka_pre_dec(int inst, int *earliest, char *context)
{
  char *ret, *p, c;
  int prev;

  if ((ret = test_previnst(inst, earliest, context, "pre_dec")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 1)
    awka_error("pre_dec error: expected a prior opcode, line %d.\n",progcode[inst].line);
  p = (* progcode[inst-1].func)(inst-1, &prev, &c);
  *earliest = prev;

  if (c != _VAR)
    awka_error("pre_dec error: expecting var context for rside, line %d.\n",progcode[inst].line);

  ret = (char *) malloc( 15 + strlen(p) );
  if (findvaltype(p) == _VALTYPE_NUM &&
     (inst + 1 >= prog_no || progcode[inst+1].op == _POP))
    sprintf(ret, "awka_prd(%s)", p);
  else
    sprintf(ret, "--awka_setd(%s)", p); 
  setvaltype(p, _VALTYPE_NUM);
  free(p);
  *context = _DBL;
  return ret;
}

char *
awka_pre_inc(int inst, int *earliest, char *context)
{
  char *ret, *p, c;
  int prev;

  if ((ret = test_previnst(inst, earliest, context, "pre_inc")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 1)
    awka_error("pre_inc error: expected a prior opcode, line %d.\n",progcode[inst].line);
  p = (* progcode[inst-1].func)(inst-1, &prev, &c);
  *earliest = prev;

  if (c != _VAR)
    awka_error("pre_inc error: expecting var context for rside, line %d.\n",progcode[inst].line);

  ret = (char *) malloc( 15 + strlen(p) );
  if (findvaltype(p) == _VALTYPE_NUM &&
     (inst + 1 >= prog_no || progcode[inst+1].op == _POP))
    sprintf(ret, "awka_pri(%s)", p);
  else
    sprintf(ret, "++awka_setd(%s)", p); 
  setvaltype(p, _VALTYPE_NUM);
  free(p);
  *context = _DBL;
  return ret;
}

char *
awka_printf(int inst, int *earliest, char *context)
{
  char *ret=NULL, *p=NULL, c1, *r2=NULL, *r3=NULL, *r4=NULL, deref = FALSE;
  int prev, i, j, prev2;

  if ((ret = test_previnst(inst, earliest, context, "printf")) != NULL)
    return ret;
  revert_gc();
  test_loop(inst);
  if (inst == 0)
    awka_error("printf error: expected at least one prior opcode.\n");
  *context = _VAR;
  *earliest = inst-2;
  prev = inst-2;
  ret = (char *) malloc( 300 );
  if (progcode[inst].op == _PRINTF)
    sprintf(ret, "awka_printf(");
  else
    sprintf(ret, "awka_print(");

  if (progcode[inst-1].op != _PUSHINT)
    awka_error("printf error: expecting prior pushint opcode, got %d, line %d.\n",progcode[inst-1].op,progcode[inst].line);
  if (progcode[inst-1].val == NULL)
  {
    if (progcode[inst-1].arg == NULL)
      awka_error("printf error: expected 'pushint number' as prior opcode, line %d.\n",progcode[inst].line);
    j = atoi(progcode[inst-1].arg);
  }
  else
    j = atoi(progcode[inst-1].val); 

  if (j < 0)
  {
    /* we have an output stream specified */
    p = (* progcode[inst-2].func)(inst-2, &prev, &c1);
    r2 = buildstr("printf", "%s", p, c1, _STR, inst, inst-2);
    if (strlen(r2) + strlen(ret) >= 300)
      ret = (char *) realloc(ret, strlen(r2) + strlen(ret) + 50 );
    sprintf(ret, "%s%s, 0",ret, r2);
    free(r2);
    free(p);
    *earliest = prev;

    if (j == -1 || j == -2)
    {
      /* file */
      sprintf(ret, "%s, %d", ret, j+1);
    }
    else if (j == -6)
    {
      /* coprocess */
      sprintf(ret, "%s, 2", ret);
    }
    else
    {
      /* pipe */
      sprintf(ret, "%s, 1", ret);
    }

    if (progcode[prev].op != _PUSHINT)
      awka_error("printf error: expecting pushint on opcode at line %d, printf at line %d.\n",progcode[prev].line,progcode[inst].line);

    p = (* progcode[prev].func)(prev, &prev, &c1);
    j = atoi(p);
    free(p);
    for (i=1; i<=3; i++)
    {
      progcode[inst-i].done = TRUE;
      killcode(inst-i);
    }
  }
  else
  {
    /* default output stream (stdout) */
    sprintf(ret, "%sNULL, 0, 0", ret);
    progcode[inst-1].done = TRUE;
    killcode(inst-1);
  }

  if (prev < j)
    awka_error("printf error: expected at least %d prior opcodes, line %d\n",j,progcode[inst].line);

  if (progcode[inst].op == _PRINT)
  {
    switch (j)
    {
      case 0:
        sprintf(ret, "%s, awka_arg1(a_TEMP, awka_dol0(a_DOL_GET)));\n", ret);
        dol0_used = 1;
        *earliest = prev;
        return ret;
      case 1:
        sprintf(ret, "%s, awka_arg1(a_TEMP", ret);
        break;
      case 2:
        sprintf(ret, "%s, awka_arg2(a_TEMP", ret);
        break;
      case 3:
        sprintf(ret, "%s, awka_arg3(a_TEMP", ret);
        break;
      default:
        sprintf(ret, "%s, awka_vararg(a_TEMP", ret);
        break;
    }
  }
  else
    sprintf(ret, "%s, awka_vararg(a_TEMP", ret);

  for (i=0; i<j; i++)
  {
    prev2 = prev;
    p = (* progcode[prev].func)(prev, &prev, &c1);
    if (i == j-1 && progcode[inst].op == _PRINTF)
    {
      if (deref == TRUE && push_func[progcode[prev2].op] && strncmp("_lit", p, 4))
        r2 = buildstr("printf", "awka_vardup(%s)", p, c1, _VAR, inst, inst-1);
      else
        r2 = buildstr("printf", "%s", p, c1, _VAR, inst, inst-1);
    }
    else
    {
      if (deref == TRUE && push_func[progcode[prev2].op] && strncmp("_lit", p, 4))
        r2 = buildstr("printf", "awka_vardup(%s)", p, c1, _ROVAR, inst, inst-1);
      else
        r2 = buildstr("printf", "%s", p, c1, _ROVAR, inst, inst-1);
    }
    if (change_op[progcode[prev2].op]) 
      deref = TRUE;
    if (i == 0)
    {
      r3 = (char *) malloc(strlen(r2)+1);
      strcpy(r3, r2);
    }
    else
    {
      r4 = (char *) malloc(strlen(r3) + strlen(r2) + 3);
      sprintf(r4, "%s, %s", r2, r3);
      r3 = (char *) realloc(r3, strlen(r4) + 1);
      strcpy(r3, r4);
      free(r4);
    }
    free(r2);
    free(p);
  }
  if (j)
  {
    ret = (char *) realloc(ret, strlen(ret) + strlen(r3) + 22 );
    sprintf(ret, "%s, %s", ret, r3);
    free(r3);
  }
  
  p = (char *) malloc(strlen(ret) + 12);
  if (progcode[inst].op == _PRINTF || j > 3)
    sprintf(p, "%s, NULL));\n",ret);
  else
    sprintf(p, "%s));\n",ret);
  free(ret);
  *earliest = prev;
  return p;
}

char *
awka_getline(int inst, int *earliest, char *context)
{
  char *ret, *p, c1, *r2=NULL, *r3=NULL;
  int prev, j, pipe = 1, prev2;

  if ((ret = test_previnst(inst, earliest, context, "getline")) != NULL)
    return ret;
  check_gc();
  test_loop(inst);
  if (inst <= 2)
    awka_error("getline error: expected at least two prior opcodes.\n");
  *context = _VAR;
  *earliest = inst-3;
  prev = inst-3;

  if (progcode[inst-1].val == NULL)
  {
    if (progcode[inst-1].arg == NULL)
      awka_error("printf error: expected 'pushint number' as prior opcode, line %d.\n",progcode[inst].line);
    j = atoi(progcode[inst-1].arg);
  }
  else
    j = atoi(progcode[inst-1].val); 
  progcode[inst-1].done = TRUE;
  killcode(inst-1);

  switch (j)
  {
    /* make r2 equal target variable, and r3 the input stream */
    case -7: /* coprocess */
      pipe = 2;
      p = (* progcode[inst-2].func)(inst-2, &prev2, &c1);
      r2 = buildstr("getline", "%s", p, c1, _VAR, inst, inst-2);
      free(p);
      p = (* progcode[prev2].func)(prev2, &prev, &c1);
      r3 = buildstr("getline", "%s", p, c1, _STR, inst, prev2);
      free(p);
      break;

    case -5: /* filename - different order to others for some reason */
      pipe = 0;
      p = (* progcode[inst-2].func)(inst-2, &prev2, &c1);
      r3 = buildstr("getline", "%s", p, c1, _STR, inst, inst-2);
      free(p);
      p = (* progcode[prev2].func)(prev2, &prev, &c1);
      r2 = buildstr("getline", "%s", p, c1, _VAR, inst, prev2);
      free(p);
      break;

    case -4: /* pipe */
      p = (* progcode[inst-2].func)(inst-2, &prev2, &c1);
      r2 = buildstr("getline", "%s", p, c1, _VAR, inst, inst-2);
      free(p);
      p = (* progcode[prev2].func)(prev2, &prev, &c1);
      r3 = buildstr("getline", "%s", p, c1, _STR, inst, prev2);
      free(p);
      break;

    case 0: /* FILENAME input */
      pipe = 0;
      r3 = (char *) malloc(32);
      strcpy(r3, "awka_gets(a_bivar[a_FILENAME])");
      prev = inst-2;
      p = (* progcode[prev].func)(prev, &prev, &c1);
      r2 = buildstr("getline", "%s", p, c1, _VAR, inst, inst-2);
      free(p);
      break;

    default:
      awka_error("getline error: unexpected pushint value %d, line %d\n",j,progcode[inst-1].line);
  }

  ret = (char *) malloc(strlen(r2) + strlen(r3) + 140);
  if (j)
    sprintf(ret, "awka_getline(a_TEMP, %s, %s, %d, FALSE)", r2, r3, pipe);
  else
    sprintf(ret, "awka_getline(a_TEMP, %s, %s, %d, TRUE)", r2, r3, pipe);

  free(r2);
  free(r3);

  *earliest = prev;
  return ret;
}

char *
awka_alength(int inst, int *earliest, char *context)
{
  char *ret, *p, c1;
  int prev = -1;

  if ((ret = test_previnst(inst, earliest, context, "alength")) != NULL)
    return ret;
  test_loop(inst);

  if (inst < 2)
    awka_error("alength error: expected a prior opcode.\n");
  *context = _DBL;

  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  if (c1 != _VAR) 
    awka_error("alength error: expecting a variable as argument.\n");

  ret = (char *) malloc(strlen(p) + 20);
  sprintf(ret, "awka_alength(%s)", p);
  free(p);

  *earliest = prev;
  return ret;
}

char *
awka_asort(int inst, int *earliest, char *context)
{
  char *ret, *p, *p2, c1;
  int prev = -1, args = 0;

  if ((ret = test_previnst(inst, earliest, context, "asort")) != NULL)
    return ret;
  test_loop(inst);

  if (inst < 3)
    awka_error("asort error: expected a prior opcode.\n");
  *context = _DBL;

  if (progcode[inst-1].op != _PUSHINT)
    awka_error("asort error: expecting prior pushint opcode, got %d, line %d.\n",progcode[inst-1].op,progcode[inst].line);
  if (progcode[inst-1].val == NULL)
  {
    if (progcode[inst-1].arg == NULL)
      awka_error("asort error: expected 'pushint number' as prior opcode, line %d.\n",progcode[inst].line);
    args = atoi(progcode[inst-1].arg);
  }
  else
    args = atoi(progcode[inst-1].val); 
  progcode[inst-1].func = NULL;

  p = (* progcode[inst-2].func)(inst-2, &prev, &c1);
  if (c1 != _VAR) 
    awka_error("asort error: expecting a variable as argument.\n");

  if (args > 1)
  {
    p2 = (* progcode[prev].func)(prev, &prev, &c1);
    if (c1 != _VAR) 
      awka_error("asort error: expecting a variable as argument.\n");
    ret = (char *) malloc(strlen(p) + strlen(p2) + 20);
    sprintf(ret, "awka_asort(%s, %s)",p,p2);
    free(p);
    free(p2);
  }
  else
  {
    ret = (char *) malloc(strlen(p) + 20);
    sprintf(ret, "awka_asort(%s, NULL)", p);
    free(p);
  }

  *earliest = prev;
  return ret;
}

char *
awka_split(int inst, int *earliest, char *context)
{
  char *ret, *p, c1, *r2, *r3;
  int prev = inst-1, prev2;

  if ((ret = test_previnst(inst, earliest, context, "split")) != NULL)
    return ret;
  test_loop(inst);
  if (inst <= 2)
    awka_error("split error: expected at least two prior opcodes.\n");
  *context = _DBL;
  
  if (!strcmp(progcode[inst-1].val, "space"))
  {
    r3 = (char *) malloc(30);
    strcpy(r3, "awka_ro_str2var(\" \")");
    prev = inst-2;
  }
  else if (!strcmp(progcode[inst-1].val, "null"))
  {
    r3 = (char *) malloc(30);
    strcpy(r3, "awka_ro_str2var(\"\")");
    prev = inst-2;
  }
  else if (!strcmp(progcode[inst-1].val, "@fs_shadow"))
  {
    r3 = (char *) malloc(5);
    strcpy(r3, "NULL");
    prev = inst-2;
  }
  else if (progcode[inst-1].val[0] == '0')
  {
    r3 = (char *) malloc(strlen(progcode[inst-1].arg)+1);
    strcpy(r3, progcode[inst-1].arg);
    prev = inst-2;
  }
  else
  {
    p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
    r3 = buildstr("split", "%s", p, c1, _VAR, inst, inst-1);
    free(p);
  }
  progcode[inst-1].done = TRUE;
  killcode(inst-1);

  prev2 = prev;
  p = (* progcode[prev].func)(prev, &prev, &c1);
  r2 = buildstr("split", "%s", p, c1, _VAR, inst, prev2);
  free(p);

  prev2 = prev;
  p = (* progcode[prev].func)(prev, &prev, &c1);
  ret = buildstr("split", "awka_arraysplitstr(%s", p, c1, _STR, inst, prev2);
  free(p);

  ret = (char *) realloc(ret, strlen(ret) + strlen(r2) + strlen(r3) + 30);
  sprintf(ret, "%s, %s, %s, INT_MAX, FALSE)",ret,r2,r3);

  *earliest = prev;
  free(r2);
  free(r3);
  return ret;
}

char *
awka_range(int inst, int *earliest, char *context)
{
  char *ret, *p, *r2, *r3, c1;
  int prev = inst-1, prev2;

  if ((ret = test_previnst(inst, earliest, context, "range")) != NULL)
    return ret;
  test_loop(inst);

  p = (* progcode[prev].func)(prev, &prev2, &c1);
  r3 = buildstr("range", "%s", p, c1, _TRU, inst, inst-1);
  free(p);
  p = (* progcode[prev2].func)(prev2, &prev, &c1);
  r2 = buildstr("range", "%s", p, c1, _TRU, inst, prev2);
  free(p);

  p = codeptr(inst, 30 + strlen(r2));
  sprintf(p, "if (_range%d == 0 && %s)\n", range_no, r2);
  p = codeptr(inst, 50);
  sprintf(p, "  _range%d = 1;\n", range_no);
  p = codeptr(inst, 40 + strlen(r3));
  sprintf(p, "else if (_range%d == 1 && %s)\n", range_no, r3);
  p = codeptr(inst, 50);
  sprintf(p, "  _range%d = 2;\n", range_no);
  p = codeptr(inst, 50);
  sprintf(p, "if (_range%d == 0)\n", range_no++);

  ret = (char *) malloc(15 + strlen(progcode[inst].val));
  sprintf(ret, "  goto __%s;\n",progcode[inst].val);

  *context = _NUL;
  *earliest = prev;
  return ret;
}

char *
awka_ret(int inst, int *earliest, char *context)
{
  char *ret, *p, c1;
  int prev, i;

  if ((ret = test_previnst(inst, earliest, context, "ret")) != NULL)
    return ret;
  revert_gc();
  test_loop(inst);
  if (inst < 1)
    awka_error("ret error: expected a prior opcode, line %d.\n",progcode[inst].line);
  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  *earliest = prev;
  *context = _NUL;
  ret = buildstr("ret", "awka_varcpy(_ret, %s);\n", p, c1, _VAR, inst, inst-1);
  free(p);

  if (al_count)
  {
    for (i=0; i<al_count; i++)
    {
      p = codeptr(inst, 50);
      sprintf(p, "awka_alistfreeall(&_alh%d);\n",i);
    }
  }
  p = codeptr(inst, strlen(ret)+1);
  strcpy(p, ret);
  p = codeptr(inst, 30 + strlen(progcode[cur_func].val));
  sprintf(p, "_awka_retfn(_fn_idx);\n");
  strcpy(ret, "return _ret;\n");

  return ret;
}

char *
awka_ret0(int inst, int *earliest, char *context)
{
  char *ret = NULL, *p;
  int i;

  if ((ret = test_previnst(inst, earliest, context, "ret0")) != NULL)
    return ret;
  test_loop(inst);
  revert_gc();

  if (inst > 0 && progcode[inst].jumpto != -1)
    if (progcode[inst-1].op == _RET)
    {
      ret = (char *) malloc(3);
      strcpy(ret, ";\n");
    }

  if (mode == FUNC && !ret)
  {
    if (al_count)
    {
      for (i=0; i<al_count; i++)
      {
        p = codeptr(inst, 50);
        sprintf(p, "awka_alistfreeall(&_alh%d);\n",i);
      }
    }
    /* p = codeptr(inst, 30);
    strcpy(p, "awka_killvar(_ret);\n"); */
    p = codeptr(inst, 40 + strlen(progcode[cur_func].val));
    sprintf(p, "_awka_retfn(_fn_idx);\n");
    ret = (char *) malloc( 20 );
    strcpy(ret, "return _ret;\n");
  }

  *earliest = inst-1;
  *context = _NUL;
  return ret;
}

char *
awka_set_al(int inst, int *earliest, char *context)
{
  /* starts a 'for (pigs in mud)' loop */
  char *ret, *p, *q, c1, c2, *r2, *r3;
  int prev, i, prev2;

  if ((ret = test_previnst(inst, earliest, context, "set_al")) != NULL)
    return ret;
  test_loop(inst);
  revert_gc();
  if (inst < 2)
    awka_error("set_al error: expected two prior opcodes, line %d.\n",progcode[inst].line);
  p = (* progcode[inst-1].func)(inst-1, &prev2, &c1);
  q = (* progcode[prev2].func)(prev2, &prev, &c2);
  *earliest = prev;

  r2 = buildstr("set_al", "%s", p, c1, _VAR, inst, inst-1);
  r3 = buildstr("set_al", "%s", q, c2, _VAR, inst, prev2);
  free(p);
  free(q);

  p = codeptr(inst, 100 + strlen(r2));
  sprintf(p, "for (_alp%d=0, awka_arrayloop(&_alh%d, %s, 0);\n", al_count, al_count, r2);
  p = codeptr(inst, 100 + strlen(r3));
  sprintf(p, "    (_alp%d = awka_arraynext(%s, &_alh%d, _alp%d)) > 0;)\n", al_count, r3, al_count, al_count);
  ret = (char *) malloc(3);
  strcpy(ret, "{\n");
  i = al_count++;

  progcode[progcode[inst].jumpto].arg = (char *) malloc(sizeof(int));
  memcpy(progcode[progcode[inst].jumpto].arg, &i, sizeof(int));

  free(r3);
  free(r2);
  *context = _NUL;
  return ret;
}

char *
awka_sub(int inst, int *earliest, char *context)
{
  char *ret, *r2, *p, *q, c1, c2;
  int prev, prev2;

  if ((ret = test_previnst(inst, earliest, context, "sub")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 2)
    awka_error("sub error: expected two prior opcodes, line %d.\n",progcode[inst].line);
  p = (* progcode[inst-1].func)(inst-1, &prev2, &c1);
  q = (* progcode[prev2].func)(prev2, &prev, &c2);
  *earliest = prev;

  if (push_func[progcode[inst-1].op])
    ret = buildstr("sub", "(%s - ", q, c2, _DBL, inst, prev2);
  else
    ret = buildstr("sub", "((%s) - ", q, c2, _DBL, inst, prev2);
  if (push_func[progcode[prev2].op])
    r2 = buildstr("sub", "%s)", p, c1, _DBL, inst, inst-1);
  else
    r2 = buildstr("sub", "(%s))", p, c1, _DBL, inst, inst-1);

  ret = (char *) realloc(ret, strlen(ret) + strlen(r2) + 1);
  strcat(ret, r2);
  *context = _DBL;

  free(p);
  free(q);
  free(r2);
  return ret;
}

char *
awka_sub_asg(int inst, int *earliest, char *context)
{
  char *ret, *r2, *p, *q, c1, c2;
  int prev;

  if ((ret = test_previnst(inst, earliest, context, "sub_asg")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 2)
    awka_error("sub_asg error: expected two prior opcodes, line %d.\n",progcode[inst].line);
  which_side = _a_RHS;
  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  which_side = _a_LHS;
  q = (* progcode[prev].func)(prev, &prev, &c2);
  which_side = _a_RHS;
  *earliest = prev;

  if (c2 != _VAR)
    awka_error("sub_asg error: expecting var context for lside, line %d.\n",progcode[inst].line);

  ret = (char *) malloc( 50 + (strlen(q)*2) );
  /*
  sprintf(ret, "awka_vardblset(%s, awka_getd(%s) - ", q, q);
  r2 = buildstr("sub_asg", "%s)", p, c1, _DBL, inst, inst-1);
  */
  sprintf(ret, "awka_setd(%s) -= ", q); 
  setvaltype(q, _VALTYPE_NUM);
  r2 = buildstr("sub_asg", "%s", p, c1, _DBL, inst, inst-1);

  ret = (char *) realloc(ret, strlen(ret) + strlen(r2) + 1);
  strcat(ret, r2);
  *context = _DBL;
  free(p);
  free(q);
  free(r2);
  return ret;
}

char *
awka_test(int inst, int *earliest, char *context)
{
  /* this indicates a multi-part test statement */
  char *ret, *p, *q, c1, c2, *r2, *r3;
  int prev, prev2;

  if ((ret = test_previnst(inst, earliest, context, "test")) != NULL)
    return ret;
  test_loop(inst);
  if (inst < 1)
    awka_error("test error: expected at least one prior opcode, line %d.\n",progcode[inst].line);

  if (progcode[inst].ljumpfrom != -1 && progcode[inst-1].op == _TEST)
  {
    /* ljz or ljnz jump to here - indicates parentheses */
    p = (* progcode[progcode[inst].ljumpfrom].func)(progcode[inst].ljumpfrom, &prev, &c1);
    *earliest = prev;
    q = (* progcode[inst-1].func)(inst-1, &prev2, &c2);

    r3 = buildstr("test", "%s", p, c1, _TRU, inst, progcode[inst].ljumpfrom);
    r2  = buildstr("test", "%s", q, c2, _TRU, inst, inst-1);

    ret = (char *) malloc( strlen(r3) + strlen(r2) + 4 );
    sprintf(ret, "(%s %s)", r3, r2);
    free(r2);
    free(q);
    free(p);
  }
  else
  {
    /* simple test statement, possibly with predecessors */
    p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
    *earliest = prev;
    ret = buildstr("test", "%s", p, c1, _TRU, inst, inst-1);
    free(p);

    if (prev > 1)
    {
      if ((progcode[prev].op == _LJNZ || progcode[prev].op == _LJZ) && 
          progcode[prev].done == FALSE) 
          /* progcode[prev].printed == FALSE) */
      {
        prev2 = prev;
        p = (* progcode[prev].func)(prev, &prev, &c1);
        r2 = (char *) malloc( strlen(ret)+1 );
        strcpy(r2, ret);
        ret = buildstr("test", "(%s", p, c1, _TRU, inst, prev2);
        free(p);
        ret = (char *) realloc( ret, strlen(ret) + strlen(r2) + 20 );
        sprintf(ret, "%s %s)",ret, r2);
        free(r2);
        *earliest = prev;
      }
    }
  }

  if ((progcode[prev].op == _LJNZ || progcode[prev].op == _LJZ) &&
      progcode[prev].printed == FALSE && progcode[prev].jumpto == inst+1 &&
      progcode[inst+1].op != _TEST && progcode[inst+1].op != _LJZ &&
      progcode[inst+1].op != _LJNZ && progcode[inst+1].op != _JZ &&
      progcode[inst+1].op != _JNZ && progcode[inst+1].op != _QMARK)
  {
    p = (* progcode[prev].func)(prev, &prev, &c1);
    r2 = (char *) malloc((strlen(p) + strlen(ret) + 5));
    sprintf(r2, "(%s %s)", p, ret);
    ret = r2;
    free(p);
    *earliest = prev;
  }

  *context = _TRU;
  return ret;
}

char *
awka_uminus(int inst, int *earliest, char *context)
{
  char *ret, *p, c;
  int prev;

  if ((ret = test_previnst(inst, earliest, context, "uminus")) != NULL)
    return ret;
  test_loop(inst);
  if (inst == 0)
    awka_error("uminus error: expected at least one prior opcode.\n");
  p = (* progcode[inst-1].func)(inst-1, &prev, &c);
  *earliest = prev;

  ret = buildstr("uminus", "-(%s)", p, c, _DBL, inst, inst-1);
  *context = _DBL;
  free(p);
  
  return ret;
}

char *
awka_uplus(int inst, int *earliest, char *context)
{
  char *ret, *p, c;
  int prev;

  if ((ret = test_previnst(inst, earliest, context, "uplus")) != NULL)
    return ret;
  test_loop(inst);
  if (inst == 0)
    awka_error("uplus error: expected at least one prior opcode.\n");
  p = (* progcode[inst-1].func)(inst-1, &prev, &c);
  *earliest = prev;

  ret = buildstr("uplus", "+(%s)", p, c, _DBL, inst, inst-1);
  *context = _DBL;
  free(p);
  
  return ret;
}

char *
awka_gensub(int inst, int *earliest, char *context)
{
  char *ret, *p, c1, *r2, *r3, *r4, *r1;
  int prev, prev2;

  if ((ret = test_previnst(inst, earliest, context, "gensub")) != NULL)
    return ret;
  check_gc();
  test_loop(inst);
  if (inst < 3)
    awka_error("gensub error: expected at least three prior opcodes, line %d.\n",progcode[inst].line);

  /* first prior - the variable to be acted on */
  p = (* progcode[inst-1].func)(inst-1, &prev2, &c1);
  r4 = buildstr("gensub", "%s", p, c1, _ROVAR, inst, inst-1);
  free(p);

  /* second - what-to-find flag */
  p = (* progcode[prev2].func)(prev2, &prev, &c1);
  r3 = buildstr("gensub", "%s", p, c1, _ROVAR, inst, prev2);
  free(p);

  /* third - the replacement string */
  p = (* progcode[prev].func)(prev, &prev2, &c1);
  r2 = buildstr("gensub", "%s", p, c1, _ROVAR, inst, prev2);
  free(p);

  /* fourth - the regular expression */
  p = (* progcode[prev2].func)(prev2, &prev, &c1);
  r1 = buildstr("gensub", "%s", p, c1, _VAR, inst, prev2);
  setvaltype(r1, _VALTYPE_RE);
  free(p);

  ret = (char *) malloc(strlen(r1) + strlen(r2) + strlen(r3) + strlen(r4) + 64);
  sprintf(ret, "awka_gensub(a_TEMP, %s, %s, %s, %s)", r1, r2, r3, r4);

  free(r1); free(r2); free(r3); free(r4);
  *earliest = prev;
  *context = _VAR;
  return ret;
}

char *
awka_gsub(int inst, int *earliest, char *context)
{
  char *ret, *p, c1, *r2, *r3, *r4;
  int prev, prev2;

  if ((ret = test_previnst(inst, earliest, context, "gsub")) != NULL)
    return ret;
  test_loop(inst);
  check_gc();
  if (inst < 3)
    awka_error("gsub error: expected at least three prior opcodes, line %d.\n",progcode[inst].line);

  /* first prior - the variable to be acted on */
  p = (* progcode[inst-1].func)(inst-1, &prev2, &c1);
  r4 = buildstr("gsub", "%s", p, c1, _VAR, inst, inst-1);
  free(p);

  /* second - the replacement string */
  p = (* progcode[prev2].func)(prev2, &prev, &c1);
  r3 = buildstr("gsub", "%s", p, c1, _ROVAR, inst, prev2);
  free(p);

  /* third - the regular expression */
  prev2 = prev;
  p = (* progcode[prev].func)(prev, &prev, &c1);
  r2 = buildstr("gsub", "%s", p, c1, _VAR, inst, prev2);
  setvaltype(r2, _VALTYPE_RE);
  free(p);

  ret = (char *) malloc(strlen(r2) + strlen(r3) + strlen(r4) + 64);
  if (progcode[inst].op == _GSUB)
    sprintf(ret, "awka_sub(a_TEMP, TRUE, FALSE, %s, %s, %s)", r2, r3, r4);
  else
    sprintf(ret, "awka_sub(a_TEMP, FALSE, FALSE, %s, %s, %s)", r2, r3, r4);

  free(r2); free(r3); free(r4);
  *earliest = prev;
  *context = _VAR;
  return ret;
}

char *
awka_patsplit(int inst, int *earliest, char *context)
{
  /* patsplit(stringToSplit, resultArray [, fieldpatternRE [, separatorsFound]])
   * 
   */

  char *ret, *p, c1, *r2, *r3, *r4, *r5;
  int prev, prev2, prev3;

  if ((ret = test_previnst(inst, earliest, context, "patsplit")) != NULL)
    return ret;

  test_loop(inst);
  check_gc();

  if (inst < 2)
    awka_error("patsplit error: expected at least two prior opcodes, line %d.\n",progcode[inst].line);
  if (inst > 4)
    awka_error("patsplit error: expected at most four prior opcodes, line %d.\n",progcode[inst].line);

  /* first prior - the string variable to be acted on */
  p = (* progcode[inst-1].func)(inst-1, &prev2, &c1);
  r5 = buildstr("patsplit", "%s", p, c1, _VAR, inst, inst-1);
  setvaltype(r5, _VALTYPE_STR);
  free(p);

  /* second - the output resultant array of splits */
  p = (* progcode[prev2].func)(prev2, &prev, &c1);
  r4 = buildstr("patsplit", "%s", p, c1, _ROVAR, inst, prev2);
  free(p);

  /* third - the optional field pattern regular expression */
  if (inst >= 3)
  {
    prev2 = prev;
    p = (* progcode[prev].func)(prev, &prev, &c1);
    r3 = buildstr("patsplit", "%s", p, c1, _VAR, inst, prev2);
    setvaltype(r2, _VALTYPE_RE);
    free(p);
  }
  else
  {
    r3 = (char *) malloc(5);
    sprintf(r3, "NULL");
  }

  /* fourth - the optional output array containing the separators */
  if (inst == 4)
  {
    prev2 = prev;
    p = (* progcode[prev].func)(prev, &prev, &c1);
    r2 = buildstr("patsplit", "%s", p, c1, _VAR, inst, prev2);
    free(p);
  }
  else
  {
    r2 = (char *) malloc(5);
    sprintf(r2, "NULL");
  }

  ret = (char *) malloc(strlen(r2) + strlen(r3) + strlen(r4) + strlen(r5) + 64);
  sprintf(ret, "awka_patsplit(%s, %s, %s, %s)", r5, r4, r3, r2);

  free(r2); free(r3); free(r4); free(r5);
  *earliest = prev;
  *context = _VAR;
  return ret;
}

char *
awka_close(int inst, int *earliest, char *context)
{
  char *ret, *p, c1;
  int prev;

  if ((ret = test_previnst(inst, earliest, context, "close")) != NULL)
    return ret;
  revert_gc();
  test_loop(inst);
  if (inst == 0)
    awka_error("close error: expected at least one prior opcodes.\n");

  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  ret = buildstr("close", "awka_close(a_TEMP, awka_vararg(a_TEMP, %s, NULL))", p, c1, _VAR, inst, inst-1);
  free(p);

  *earliest = prev;
  *context = _VAR;
  return ret;
}

char *
awka_goto(int inst, int *earliest, char *context)
{
  char *ret;

  if ((ret = test_previnst(inst, earliest, context, "goto")) != NULL)
    return ret;
  test_loop(inst);
  revert_gc();
  ret = (char *) malloc(15 + strlen(progcode[inst].val));
  sprintf(ret, "goto __%d;\n",atoi(progcode[inst].val));
  *context = _NUL;
  *earliest = inst-1;
  return ret;
}

char *
awka_colon(int inst, int *earliest, char *context)
{
  char *ret, *p, *r2, *r3, *r4, c1;
  int prev, prev2;

  if ((ret = test_previnst(inst, earliest, context, "colon")) != NULL)
    return ret;
  test_loop(inst);
  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  r4 = buildstr("colon", "%s", p, c1, _VAR, inst, inst-1);
  free(p);
  prev2 = prev;
  p = (* progcode[prev].func)(prev, &prev, &c1);
  r3 = buildstr("colon", "%s", p, c1, _VAR, inst, prev2);
  free(p);
  prev2 = prev;
  p = (* progcode[prev].func)(prev, &prev, &c1);
  r2 = buildstr("colon", "%s", p, c1, _NUL, inst, prev2);
  free(p);

  ret = (char *) malloc(strlen(r2) + strlen(r3) + strlen(r4) + 20);
  sprintf(ret, "(%s ? %s : %s)", r2, r3, r4);
  *earliest = prev;
  *context = _VAR;

  free(r2); free(r3); free(r4);
  return ret;
}

char *
awka_tocase(int inst, int *earliest, char *context)
{
  char *ret, *p, c1, *r2;
  int prev;

  if ((ret = test_previnst(inst, earliest, context, "tocase")) != NULL)
    return ret;
  test_loop(inst);
  check_gc();

  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  r2 = buildstr("tocase", "%s", p, c1, _NUL, inst, inst-1);
  ret = (char *) malloc(strlen(r2) + 70);
  switch (progcode[inst].op)
  {
    case _TOLOWER:
      sprintf(ret, "awka_tocase(a_TEMP, a_BI_TOLOWER, %s)",r2);
      break;
    case _TOTITLE:
      sprintf(ret, "awka_tocase(a_TEMP, a_BI_TOTITLE, %s)",r2);
      break;
    case _TOUPPER:
      sprintf(ret, "awka_tocase(a_TEMP, a_BI_TOUPPER, %s)",r2);
      break;
  }
  free(r2);
  free(p);
  *context = _VAR;
  *earliest = prev;
  return ret;
}

char *
awka_index(int inst, int *earliest, char *context)
{
  char *ret, *p, c1, *r2, *r3;
  int prev2, prev;

  if ((ret = test_previnst(inst, earliest, context, "index")) != NULL)
    return ret;
  test_loop(inst);

  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  r2 = buildstr("index", "%s", p, c1, _ROVAR, inst, inst-1);
  free(p);
  prev2 = prev;
  p = (* progcode[prev].func)(prev, &prev, &c1);
  r3 = buildstr("index", "%s", p, c1, _VAR, inst, prev2);
  free(p);

  ret = (char *) malloc(strlen(r2) + strlen(r3) + 70);
  sprintf(ret, "awka_index(%s, %s)",r3,r2);
  free(r3);
  free(r2);

  *earliest = prev;
  *context = _DBL;
  return ret;
}

char *
awka_substr(int inst, int *earliest, char *context)
{
  char *ret, *p, c1, *r2, *r3, *r4;
  int prev, n_arg, prev2;

  if ((ret = test_previnst(inst, earliest, context, "substr")) != NULL)
    return ret;
  test_loop(inst);
  check_gc();

  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  n_arg = atoi(p);
  free(p);

  if (n_arg == 3)
  {
    prev2 = prev;
    p = (* progcode[prev].func)(prev, &prev, &c1);
    r4 = buildstr("substr", "%s", p, c1, _DBL, inst, prev2);
  }
  else
  {
    r4 = (char *) malloc(20);
    strcpy(r4, "(double) INT_MAX");
  }
  prev2 = prev;
  p = (* progcode[prev].func)(prev, &prev, &c1);
  r3 = buildstr("substr", "%s", p, c1, _DBL, inst, prev2);
  free(p);
  prev2 = prev;
  p = (* progcode[prev].func)(prev, &prev, &c1);
  r2 = buildstr("substr", "%s", p, c1, _ROVAR, inst, prev2);
  free(p);

  ret = (char *) malloc(strlen(r2) + strlen(r3) + strlen(r4) + 70);
  sprintf(ret, "awka_substr(a_TEMP, %s, %s, %s)",r2,r3,r4);
  free(r4);
  free(r3);
  free(r2);

  *earliest = prev;
  *context = _VAR;
  return ret;
}

char *
awka_bi_length(int inst, int *earliest, char *context)
{
  char *ret, *p, c1, *r2;
  int prev, n_arg, prev2;
  char *(*func2)(int, int *, char *);

  if ((ret = test_previnst(inst, earliest, context, "length")) != NULL)
    return ret;
  test_loop(inst);

  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  n_arg = atoi(p);
  free(p);
  if (n_arg)
  {
    prev2 = prev;
    func2 = progcode[prev].func;
    p = (* progcode[prev].func)(prev, &prev, &c1);
    r2 = buildstr("bi_length", "%s", p, c1, _VAR, inst, prev2);
    ret = (char *) malloc(strlen(r2) + 30);
    sprintf(ret, "awka_length(%s)",r2);
    free(r2);
    free(p);
  }
  else
  {
    ret = (char *) malloc(50);
    strcpy(ret, "awka_length0");
    dol0_used = 1;
    dol0_only = 0;
  }

  *earliest = prev;
  *context = _DBL;
  return ret;
}

char *
awka_math(int inst, int *earliest, char *context)
{
  char *ret, *p, c1, *r2, *r3 = NULL;
  int prev, prev2;

  if ((ret = test_previnst(inst, earliest, context, "math")) != NULL)
    return ret;
  test_loop(inst);

  p = (* progcode[inst-1].func)(inst-1, &prev, &c1);
  r2 = buildstr("math", "%s", p, c1, _DBL, inst, inst-1);
  if (progcode[inst].op == _ATAN2 || progcode[inst].op == _HYPOT ||
	progcode[inst].op == _MMOD || progcode[inst].op == _MPOW)
  {
    free(p); prev2 = prev;
    p = (*progcode[prev].func)(prev, &prev, &c1);
    r3 = buildstr("math", "%s", p, c1, _DBL, inst, prev2);
    ret = (char *) malloc(strlen(r2) + strlen(r3) + 50);
  }
  else
    ret = (char *) malloc(strlen(r2) + 50);

  switch (progcode[inst].op)
  {
    case 0:  /* Not found! */
      awka_error("math error: unknown function %s\n",code[progcode[inst].op-1].name);
      break;
    case _ABS:
      sprintf(ret, "fabs(%s)",r2);
      break;
    case _ATAN2:
    case _HYPOT:
    case _MPOW:
      sprintf(ret, "%s(%s, %s)",code[progcode[inst].op-1].name,r3,r2);
      break;
    case _MMOD:
      sprintf(ret, "fmod(%s, %s)",r3,r2);
      break;
    case a_INT:
      sprintf(ret, "(int) (%s)",r2);
      break;
    default:  /* single parameter math function */
      sprintf(ret, "%s(%s)",code[progcode[inst].op-1].name,r2);
      break;
  }
  free(r2);
  free(p);
  if (r3) free(r3);
  *context = _DBL;
  *earliest = prev;
  return ret;
}

char *
awka_argval(int inst, int *earliest, char *context)
{
  char *ret, *p, c1, *r2, *r3 = NULL;
  int prev, prev2, i, j, deref = FALSE;

  if ((ret = test_previnst(inst, earliest, context, "argcount")) != NULL)
    return ret;
  test_loop(inst);
  check_gc();

  progcode[inst-1].done = TRUE;
  killcode(inst-1);
  j = atoi(progcode[inst-1].val); 
  if (inst <= j-1)
    awka_error("argval error: expected at least %d prior opcodes.\n",j+1);
  prev = inst-2;

  ret = (char *) malloc(8);
  ret[0] = '\0';

  for (i=0; i<j; i++)
  {
    prev2 = prev;
    p = (* progcode[prev].func)(prev, &prev, &c1);
    if (deref == TRUE && push_func[progcode[prev2].op] && strncmp("_lit", p, 4))
      r2 = buildstr("argval", "awka_vardup(%s)", p, c1, _VAR, inst, prev2);
    else
      r2 = buildstr("argval", "%s", p, c1, _VAR, inst, prev2);
    free(p);
    if (change_op[progcode[prev2].op])
      deref = TRUE;
    r3 = (char *) malloc(strlen(ret) + strlen(r2)*2 + 90);
    if (ret[0] != '\0')
    {
      if (i == j-1)
      {
        p = buildstr("argval", "%s", r2, c1, _DBL, inst, prev2);
        sprintf(r3, "_lvar[(int) %s - 1], (int) %s, va->used, awka_vararg(a_TEMP, %s",p,p,ret);
        free(p);
      }
      else
        sprintf(r3, "%s, %s",r2,ret);
    }
    else
      sprintf(r3, "%s", r2);
    ret = (char *) realloc(ret, strlen(r3) + 50);
    strcpy(ret, r3);
    free(r2); free(r3);
  }

  if (j == 1)
  {
    p = buildstr("argval", "(int) %s", ret, c1, _DBL, inst, prev2);
    r3 = (char *) malloc( strlen(p) * 2 + 80 );
    sprintf(r3, "awka_argval(_fn_idx, _lvar[%s - 1], %s, va->used, awka_arg0(a_TEMP))",p,p);
    free(p);
  }
  else
  {
    r3 = (char *) malloc(strlen(ret) + 80);
    sprintf(r3, "awka_argval(_fn_idx, %s, NULL))",ret);
  }
  free(ret);

  if (mode != FUNC)
    strcpy(r3, "awka_argval(-1, NULL, -1, -1, awka_arg0(a_TEMP))");

  *context = _VAR;
  *earliest = prev;

  return r3;
}

char *
awka_argcount(int inst, int *earliest, char *context)
{
  char *ret = NULL;

  if ((ret = test_previnst(inst, earliest, context, "argcount")) != NULL)
    return ret;
  test_loop(inst);

  ret = (char *) malloc(16);
  if (mode == FUNC)
    strcpy(ret, "va->used");
  else
    strcpy(ret, "0");

  *context = _DBL;
  *earliest = inst-1;

  return ret;
}

char *
awka_builtin(int inst, int *earliest, char *context)
{
  /* this handles any builtins that dont need their own argument formats */
  char *ret, *p, c1, *r2, *r3, deref = FALSE;
  int prev2, prev = inst-1, i, j;

  if ((ret = test_previnst(inst, earliest, context, "builtin")) != NULL)
    return ret;
  test_loop(inst);

  if (inst == 0)
    awka_error("builtin error: expected at least one prior opcode, line %d.\n", progcode[inst].line);
  *context = _VAR;

  ret = (char *) malloc(50);
  if (progcode[inst].op == _RAND)
  {
    /* takes no arguments */
    strcpy(ret, "awka_rand()");
    *earliest = inst-1;
    *context = _DBL;
    return ret;
  }
  ret[0] = '\0';
  check_gc();

  if (progcode[inst].varidx > -1 &&
      _a_bi_vararg[progcode[inst].varidx].min_args == 
      _a_bi_vararg[progcode[inst].varidx].max_args)
  {
    j = _a_bi_vararg[progcode[inst].varidx].min_args;
    switch (progcode[inst].op)
    {
      case _SYSTIME:
            strcpy(ret, "awka_systime(a_TEMP)");
                *earliest = inst-1;
                return ret;

      case _SYSTEM:
      case _CHAR:
      case _COMPL:
        p = (* progcode[prev].func)(prev, &prev, &c1);
        r2 = buildstr("builtin", "%s", p, c1, _VAR, inst, inst-1);
        i = 20 + strlen(r2) + strlen(code[progcode[inst].op-1].name);
        if (i > 50)
          ret = (char *) realloc(ret, i+1);
        if (progcode[inst].op == _COMPL)
          sprintf(ret, "awka_%s(%s)",code[progcode[inst].op-1].name, r2);
        else
          sprintf(ret, "awka_%s(a_TEMP, %s)",code[progcode[inst].op-1].name, r2);
        free(p); free(r2);
        *earliest = prev;
        if (progcode[inst].op == _COMPL)
          *context = _DBL;
        return ret;

      case _LEFT:
      case _RIGHT:
      case _LSHIFT:
      case _RSHIFT:
      case _AND:
      case _OR:
      case _XOR:
        p = (* progcode[prev].func)(prev, &prev, &c1);
        r2 = buildstr("builtin", "%s", p, c1, _VAR, inst, inst-1);
        free(p);
        prev2 = prev;
        p = (* progcode[prev].func)(prev, &prev, &c1);
        r3 = buildstr("builtin", "%s", p, c1, _VAR, inst, prev2);
        i = 20 + strlen(r2) + strlen(r3) + strlen(code[progcode[inst].op-1].name);
        if (i > 50)
          ret = (char *) realloc(ret, i+1);
        if (progcode[inst].op != _LEFT && progcode[inst].op != _RIGHT)
          sprintf(ret, "awka_%s(%s, %s)",code[progcode[inst].op-1].name, r3, r2);
        else
          sprintf(ret, "awka_%s(a_TEMP, %s, %s)",code[progcode[inst].op-1].name, r3, r2);
        free(p); free(r2); free(r3);
        *earliest = prev;
        if (progcode[inst].op != _LEFT && progcode[inst].op != _RIGHT)
          *context = _DBL;
        return ret;
    }
  }
  else
  {
    if (progcode[inst-1].op == _PUSHINT)
    {
      progcode[inst-1].done = TRUE;
      killcode(inst-1);
      j = atoi(progcode[inst-1].val); 
      if (inst <= j-1)
        awka_error("builtin error: expected at least %d prior opcodes.\n",j+1);
      prev = inst-2;
    }
    else
    {
      j = atoi(progcode[inst].arg);
      if (inst <= j-1)
        awka_error("builtin error: expected at least %d prior opcodes.\n",j+1);
      prev = inst-1;
    }
  }

  for (i=0; i<j; i++)
  {
    prev2 = prev;
    p = (* progcode[prev].func)(prev, &prev, &c1);
    if (deref == TRUE && push_func[progcode[prev2].op] && strncmp("_lit", p, 4))
      r2 = buildstr("builtin", "awka_vardup(%s)", p, c1, _VAR, inst, prev2);
    else
      r2 = buildstr("builtin", "%s", p, c1, _VAR, inst, prev2);
    if (change_op[progcode[prev2].op])
      deref = TRUE;
    r3 = (char *) malloc(strlen(ret) + strlen(r2) + 12);
    if (ret[0] != '\0')
      sprintf(r3, "%s, %s",r2,ret);
    else
      strcpy(r3, r2);
    ret = (char *) realloc(ret, strlen(r3) + 12);
    strcpy(ret, r3);
    free(r2); free(r3);
    free(p);
  }
  *earliest = prev;
  r3 = (char *) malloc( strlen(ret) + strlen(code[progcode[inst].op-1].name) + 60 );
  if (j)
  {
    switch (j)
    {
      case 1:
        sprintf(r3, "awka_%s(a_TEMP, awka_arg1(a_TEMP, %s))",code[progcode[inst].op-1].name,ret);
        break;
      case 2:
        sprintf(r3, "awka_%s(a_TEMP, awka_arg2(a_TEMP, %s))",code[progcode[inst].op-1].name,ret);
        break;
      case 3:
        sprintf(r3, "awka_%s(a_TEMP, awka_arg3(a_TEMP, %s))",code[progcode[inst].op-1].name,ret);
        break;
      default:
        sprintf(r3, "awka_%s(a_TEMP, awka_vararg(a_TEMP, %s, NULL))",code[progcode[inst].op-1].name,ret);
    }
  }
  else
    sprintf(r3, "awka_%s(a_TEMP, awka_arg0(a_TEMP))",code[progcode[inst].op-1].name);
  ret = (char *) realloc(ret, strlen(r3) + 1);
  strcpy(ret, r3);
  free(r3);

  return ret;
}

void
awka_function(int cur, int end)
{
  int i, lvar_no, argval = FALSE;
  char *p, isarr[256];

  revert_gc();
  memset(isarr, 0, 256);
  if (mode != NONE && mode != MAIN)
    fprintf(outfp, "\n}\n\n");
  else if (mode == MAIN)
    fprintf(outfp, "\n\n");
  mode = FUNC;
  cur_func = cur;
  if (!progcode[cur].val)
    awka_error("error: expecting function name, line %d.\n",progcode[cur].line);
  p = code0ptr(cur, 100 + strlen(progcode[cur].val));
  sprintf(p, "\na_VAR *\n%s_fn( a_VARARG *va )\n{\n",progcode[cur].val);
  indent = 1;
  for (i=cur+1, lvar_no=0; i<end; i++)
  {
    switch (progcode[i].op)
    {
      case L_PUSHA:
      case L_PUSHI:
        lvar_no = Max(lvar_no, atoi(progcode[i].val)+1);
        if (progcode[i].op == L_PUSHA && isarr[atoi(progcode[i].val)] == 0)
          isarr[atoi(progcode[i].val)] = 1;
        else if (i > 0 && progcode[i].op == L_PUSHI && isarr[atoi(progcode[i].val)] == 0)
        {
          if (progcode[i-1].op == _GSUB || progcode[i-1].op == _SUB_BI)
            isarr[atoi(progcode[i].val)] = 1;
          else if (i < prog_no - 1 && progcode[i+1].op == _MATCH || progcode[i+1].op == _MATCH0 ||
                  progcode[i+1].op == _MATCH1 || progcode[i+1].op == _MATCH2)
            isarr[atoi(progcode[i].val)] = 1;
        }
        break;
        
      case _ARGVAL:
        argval = TRUE;
        break;

      case LAE_PUSHA:
      case LAE_PUSHI:
      case LA_PUSHA:
        lvar_no = Max(lvar_no, atoi(progcode[i].val)+1);
        isarr[atoi(progcode[i].val)] = 2;
        break;
        
      case _FUNCTION:
      case _BEGIN:
      case _END:
      case _MAIN:
        i = end;
        break;
    }
  }
  p = codeptr(cur, 50);
  sprintf(p, "a_VAR *_ret = NULL;\n");
  p = codeptr(cur, 50);
  sprintf(p, "static int _fn_idx = -1;\n");
  if (argval && !lvar_no)
    lvar_no = max_call_args;
  if (lvar_no)
  {
    p = codeptr(cur, 10);
    sprintf(p, "int _i;\n");
    p = codeptr(cur, 40);
    sprintf(p, "a_VAR *_lvar[%d];\n\n",lvar_no);
    p = codeptr(cur, (lvar_no * 3) + 40);
    sprintf(p, "static int _type[%d] = {", lvar_no);
    sprintf(p, "%s%d", p, isarr[0]);
    for (i=1; i<lvar_no; i++)
      sprintf(p, "%s, %d", p, isarr[i]);
    sprintf(p, "%s};\n",p);
  }

  p = codeptr(cur, 40);
  sprintf(p, "if (_fn_idx == -1)\n");
  p = codeptr(cur, 100 + strlen(progcode[cur].val));
  sprintf(p, "  _fn_idx = _awka_registerfn(\"%s\",%d);\n", progcode[cur].val,lvar_no);
  p = codeptr(cur, 50);
  sprintf(p, "_ret = _awka_addfncall(_fn_idx);\n");
  if (lvar_no)
  {
    p = codeptr(cur, 50);
    sprintf(p, "for (_i=0; _i<%d; _i++)\n", lvar_no);
    p = codeptr(cur, 5);
    sprintf(p, "{\n");
    p = codeptr(cur, 50);
    sprintf(p, "if (va->used <= _i)\n");
    p = codeptr(cur, 100);
    sprintf(p, "  _lvar[_i] = awka_fn_varinit(_fn_idx, _i, _type[_i]);\n");
    p = codeptr(cur, 20);
    sprintf(p, "else\n");
    p = codeptr(cur, 20);
    sprintf(p, "{\n");
    p = codeptr(cur, 50);
    sprintf(p, "switch (va->var[_i]->type)\n");
    p = codeptr(cur, 20);
    sprintf(p, "{\n");
    p = codeptr(cur, 50);
    sprintf(p, "case a_VARARR:\n");
    p = codeptr(cur, 100);
    sprintf(p, "_lvar[_i] = va->var[_i];\n");
    p = codeptr(cur, 50);
    sprintf(p, "break;\n\n");
    p = codeptr(cur, 50);
    sprintf(p, "default:\n");
    p = codeptr(cur, 100);
    sprintf(p, "_lvar[_i] = awka_fn_varinit(_fn_idx, _i, _type[_i]);\n");
    p = codeptr(cur, 100);
    strcpy(p,  "if (!_type[_i] && !va->var[_i]->temp)\n");
    p = codeptr(cur, 50);
    strcpy(p,  "  awka_lcopy(_lvar[_i], va->var[_i]);\n");
    p = codeptr(cur, 10);
    strcpy(p,  "else\n");
    p = codeptr(cur, 200);
    strcpy(p,  "  awka_varcpy(_lvar[_i], va->var[_i]);\n");
    p = codeptr(cur, 20);
    sprintf(p, "}\n");
    p = codeptr(cur, 20);
    sprintf(p, "}\n");
    p = codeptr(cur, 20);
    sprintf(p, "}\n\n");
  }

  progcode[cur].func = awka_nullfunc;
}

void
translate_section(int start, int end)
{
  int cur;
  char *x, *y;

  al_count = 0;
  for (cur=start; cur<end; cur++)
  {
    if (progcode[cur].func)
    {
      if (progcode[cur].done != FALSE)
        awka_error("internal error: program opcode %d(%d) executed already !?\n",cur,progcode[cur].minst);
      x = (* progcode[cur].func)(cur, &progcode[cur].earliest, &progcode[cur].context);
      if (x)
      {
        y = codeptr(cur, strlen(x)+1);
        strcpy(y, x);
        free(x);
      }
    }
  }
}

void
print_section(int start, int end)
{
  int i, j=0;

  for (i=start; i<end; i++)
  {
    if (progcode[i].done != TRUE &&
        progcode[i].func != NULL)
      awka_error("opcode at line %d not parsed!\n",progcode[i].line);

    if (progcode[i].label == TRUE)
      fprintf(outfp, "%s__%d:;\n",autoindent(" "), progcode[i].minst);
    progcode[i].label = FALSE;

    for (j=0; j<progcode[i].code0_used; j++)
      fprintf(outfp,"%s%s",autoindent(progcode[i].code0[j]),progcode[i].code0[j]);

    progcode[i].printed = TRUE;
    
    if (progcode[i].func == NULL)
      continue;

    for (j=0; j<progcode[i].code_used-1; j++)
      fprintf(outfp,"%s%s",autoindent(progcode[i].code[j]),progcode[i].code[j]);

    if (progcode[i].code_used)
    {
      /*
      if (i == end-1 && (mode == BEGIN || mode == END) && 
          progcode[i].op == _EXIT0 && progcode[i-1].op != _JZ)
        continue;
      if (i == end-2 && (mode == BEGIN || mode == END) && 
          progcode[i].op == _EXIT && progcode[i-1].op != _JZ)
        continue;
        */
      if (i == end-1 && mode == END && 
          progcode[i].op == _EXIT0 && progcode[i-1].op != _JZ)
        continue;
      if (i == end-2 && mode == END && 
          progcode[i].op == _EXIT && progcode[i-1].op != _JZ)
        continue;
      if (progcode[i].op == _POP)
        fprintf(outfp,"%s",progcode[i].code[j]);
      else
        fprintf(outfp,"%s%s",autoindent(progcode[i].code[j]),progcode[i].code[j]);
    }
  }
}

void
declare_alist(int inst)
{
  int i;
  char *p;

  if (al_count)
  {
    for (i=0; i<al_count; i++)
    {
      p = code0ptr(inst, 100);
      sprintf(p, "a_ListHdr _alh%d;\n", i);
      p = code0ptr(inst, 100);
      sprintf(p, "int _alp%d;\n", i);
    }
    for (i=0; i<al_count; i++)
    {
      p = codeptr(inst, 100);
      sprintf(p, "_alh%d.allc = _alh%d.used = 0; _alh%d.list = NULL;\n", i, i, i);
    }
  }
}

void
declare_range(int inst)
{
  int i;
  char *p;

  if (range_no)
  {
    for (i=0; i<range_no; i++)
    {
      p = code0ptr(inst, 100);
      sprintf(p, "static int _range%d = 0;\n", i);
    }
    p = code0ptr(inst, 5);
    sprintf(p, "\n");
  }
}

char **
process_argv(int *argc, char *int_argv)
{
  register char *p = int_argv, *p2, *p3, *p4;
  int count = 0, in_string = 0;
  char **argv;

  while (*p)
  {
    while (*p && *p == ' ') p++;
    if (!*p) break;

    count++;

    while (*p && *p != ' ') p++;
  }

  if (!count) return NULL;
  argv = (char **) malloc(count * sizeof(char *));
  count = 0;
  p = int_argv;

  while (*p)
  {
    while (*p && *p == ' ') p++;
    if (!*p) break;

    p2 = p;
    /* while (*p2 && *p2 != ' ') p2++; */
    while (*p2)
    {
      if (!in_string && *p2 == '"' && (p2 == int_argv || *(p2-1) != '\\'))
        in_string = 1;
      else if (in_string && *p2 == '"' && *(p2-1) != '\\')
        in_string = 0;

      if (*p2 == ' ' && !in_string)
        break;

      p2++;
    }

    argv[count] = (char *) malloc( (p2 - p) * 2 );
    p3 = p;
    p4 = argv[count];
    count++;

    while (p3 < p2)
    {
      if (*p3 == '"' && p3 > p && *(p3-1) != '\\')
        ;
      else
      {
        *p4 = *p3; ++p4;
      }
      p3++;
    }
    *p4 = '\0';

    if (!*p2)
      break;
    
    p = p2+1;
  }

  *argc = count;
  return argv;
}


/*
 * translate
 * - main controlling function for generating code
 */
void
translate()
{
  int i, cur = 0, prev = 0;
  static char preproc = FALSE;
  char *p, **argv;
  int argc;
  extern int incf_used;
  extern char **incfile;

  buf[0] = '\0';
  memset(push_func, 0, END_CODE * sizeof(int));
  push_func[AE_PUSHI] = push_func[F_PUSHI] = 1;
  push_func[FE_PUSHI] = push_func[L_PUSHI] = push_func[LAE_PUSHI] = 1;
  push_func[_PUSHI] = push_func[_PUSHC] = push_func[_PUSHD] = push_func[_PUSHS] = 1;

  memset(assign_op, 0, END_CODE * sizeof(int));
  assign_op[_ASSIGN] = assign_op[F_ASSIGN] = assign_op[F_ADD_ASG] = assign_op[F_SUB_ASG] = 1;
  assign_op[F_DIV_ASG] = assign_op[F_MUL_ASG] = assign_op[F_POW_ASG] = assign_op[F_MOD_ASG] = 1;
  assign_op[_DIV_ASG] = assign_op[_MUL_ASG] = assign_op[_POW_ASG] = assign_op[_MOD_ASG] = 1;
  assign_op[_ADD_ASG] = assign_op[_SUB_ASG] = 1;

  memset(change_op, 0, END_CODE * sizeof(int));
  change_op[_ASSIGN] = change_op[F_ASSIGN] = change_op[F_ADD_ASG] = change_op[F_SUB_ASG] = 1;
  change_op[F_DIV_ASG] = change_op[F_MUL_ASG] = change_op[F_POW_ASG] = change_op[F_MOD_ASG] = 1;
  change_op[_DIV_ASG] = change_op[_MUL_ASG] = change_op[_POW_ASG] = change_op[_MOD_ASG] = 1;
  change_op[_ADD_ASG] = change_op[_SUB_ASG] = change_op[_MATCH0] = change_op[_MATCH1] = 1;
  change_op[_MATCH] = change_op[_MATCH2] = change_op[_GSUB] = change_op[_SUB_BI] = change_op[_CALL] = 1;
  change_op[_PRE_INC] = change_op[_PRE_DEC] = change_op[_POST_INC] = change_op[_POST_DEC] = 1;
  change_op[F_PRE_INC] = change_op[F_PRE_DEC] = change_op[F_POST_INC] = change_op[F_POST_DEC] = 1;
  
  fprintf(outfp,"/* This file generated by AWKA %s */\n\n", AWKAVERSION);
  fprintf(outfp,"#include <libawka.h>\n");
  fprintf(outfp,"#include <setjmp.h>\n");

  for (i=0; i<incf_used; i++)
    fprintf(outfp, "#include <%s>\n", incfile[i]);
  fprintf(outfp, "\n");
  
  if (int_argv)
  {
    fprintf(outfp, "extern int _int_argc;\n");
    fprintf(outfp, "extern char ** _int_argv;\n");
  }
  fprintf(outfp, "static jmp_buf context;\n");
  fprintf(outfp,"static int awka_exit_val = -1;\n");

  if (preproc == FALSE)
  {
    preprocess();
    preproc = TRUE;
  }
        
  /* look for main starting points */
  for (cur=0; cur<prog_no; cur++)
  {
    if (progcode[cur].done == TRUE) 
      continue;

    switch (progcode[cur].op)
    {
      case _FUNCTION:
        if (cur > 0)
        {
          translate_section(prev, cur);
          if (al_count > 0) declare_alist(prev);
          if (range_no > 0 && mode == MAIN) declare_range(prev);
          print_section(prev, cur);
        }
        revert_gc();
        initlvartypes();
        awka_function(cur, prog_no);
        indent = 0;
        /* progcode[cur].done = TRUE; */
        prev = cur;
        break;

      case _BEGIN:
        begin_used = TRUE;
        if (cur > 0)
        {
          translate_section(prev, cur);
          if (al_count > 0) declare_alist(prev);
          if (range_no > 0 && mode == MAIN) declare_range(prev);
          print_section(prev, cur);
        }
        if (mode != NONE)
          fprintf(outfp, "\n}\n\n");
        mode = BEGIN;
        revert_gc();
        fprintf(outfp, "\nstatic void\nBEGIN()\n{\n");
        indent = 1;
        prev = cur;
        progcode[cur].func = awka_nullfunc;
        /* progcode[cur].done = TRUE; */
        break;

      case _END:
        end_used = TRUE;
        if (cur > 0)
        {
          translate_section(prev, cur);
          if (al_count > 0) declare_alist(prev);
          if (range_no > 0 && mode == MAIN) declare_range(prev);
          print_section(prev, cur);
        }
        if (mode != NONE)
          fprintf(outfp, "\n}\n\n");
        mode = END;
        revert_gc();
        fprintf(outfp, "\nstatic void\nEND()\n{\n");
        indent = 1;
        prev = cur;
        progcode[cur].func = awka_nullfunc;
        /* progcode[cur].done = TRUE; */
        break;

      case _MAIN:
        main_used = TRUE;
        if (cur > 0)
        {
          translate_section(prev, cur);
          if (al_count > 0) declare_alist(prev);
          print_section(prev, cur);
        }
        if (mode != NONE)
          fprintf(outfp, "\n}\n\n");
        mode = MAIN;
        revert_gc();
        fprintf(outfp, "\nstatic void\nMAIN()\n{\n");
	if (begin_used)       /* exit MAIN() if BEGIN() has requested an exit */
	{
          p = code0ptr(cur, 40);
          sprintf(p, "if (awka_exit_val >= 0)\n    return;\n\n");
	}
        p = code0ptr(cur, 15);
        sprintf(p, "int i = 0;\n");
        p = code0ptr(cur, 50);
        sprintf(p, "if (*(awka_gets(a_bivar[a_FILENAME])) == '\\0')\n");
        p = code0ptr(cur, 50);
        sprintf(p, "  awka_strcpy(a_bivar[a_FILENAME], \"\");\n");
        p = code0ptr(cur, 25);
        sprintf(p, "i = setjmp(context);\n");
        p = code0ptr(cur, 50);
        sprintf(p, "while (awka_getline_main())\n  {\n");
        //p = code0ptr(cur, 170);
        //sprintf(p, "while (awka_getline(a_TEMP, awka_dol0(a_DOL_GET), awka_gets(a_bivar[a_FILENAME]), FALSE, TRUE)->dval > 0 && awka_setNF())\n  {\n");
        indent = 1;
        prev = cur;
        progcode[cur].func = awka_nullfunc;
        break;
    }
  }

  if (prev < cur)
  {
    translate_section(prev, cur);
    if (al_count > 0) declare_alist(prev);
    if (range_no > 0 && mode == MAIN) declare_range(prev);
    print_section(prev, cur);
  }
  if (mode == BEGIN || mode == FUNC || mode == END)
    fprintf(outfp, "\n}\n\n");

  fprintf(outfp,"\n\n");
  fprintf(outfp,"int\n");
  if (awka_main)
    fprintf(outfp,"%s(int argc, char *argv[])\n{\n",awka_main_func);
  else
    fprintf(outfp,"main(int argc, char *argv[])\n{\n");
  
  fprintf(outfp,"  awka_config_max_base_gc(%d);\n",max_base_gc+1);
  fprintf(outfp,"  awka_config_max_fn_gc(%d);\n\n",max_fn_gc+1);
  fprintf(outfp,"  awka_split_req(0); awka_split_max(INT_MAX);\n\n");

  for (i=0; i<var_used; i++)
    fprintf(outfp,"  awka_varinit(%s);\n",varname[i].name);
  fprintf(outfp,"\n");

  for (i=0; i<litd_used; i++)
    fprintf(outfp,"  awka_varinit(_litd%d_awka); awka_setd(_litd%d_awka) = %s;\n",i,i,litd_val[i]);
  
  for (i=0; i<lits_used; i++) {
    const char *string_value = lits_val[i];
    fprintf(outfp,"  awka_varinit(_lits%d_awka); awka_strncpy(_lits%d_awka, \"%s\", %d);\n",i, i, string_value, getstringsize(string_value));
  }
  
  for (i=0; i<litr_used; i++)
    fprintf(outfp,"  awka_varinit(_litr%d_awka); awka_strcpy(_litr%d_awka, \"%s\"); awka_getre(_litr%d_awka);\n",i,i,litr_val[i],i);
  
  fprintf(outfp,"\n  if (!_lvar) {\n");
  fprintf(outfp,"    malloc( &_lvar, %d * sizeof(a_VAR *) );\n",litd_used+lits_used+litr_used+1);
  for (i=0; i<litd_used; i++)
    fprintf(outfp, "    _lvar[%d] = _litd%d_awka;\n", i, i);
  for (i=0; i<lits_used; i++)
    fprintf(outfp, "    _lvar[%d] = _lits%d_awka;\n", i+litd_used, i);
  for (i=0; i<litr_used; i++)
    fprintf(outfp, "    _lvar[%d] = _litr%d_awka;\n", i+litd_used+lits_used, i);
  fprintf(outfp, "    _lvar[%d] = NULL;\n", i+lits_used+litd_used);
  fprintf(outfp, "  }\n");
  
  fprintf(outfp,"\n  awka_register_gvar(%d, NULL, NULL);\n", var_used);
  for (i=0; i<var_used; i++)
  {
    fprintf(outfp,"  awka_register_gvar(%d, \"%s\", %s);\n",i,varname[i].name,varname[i].name);
    if (isarray(varname[i].name))
    {
/*       fprintf(outfp,"  free(%s->ptr); %s->ptr = NULL;\n",varname[i],varname[i]); */
      fprintf(outfp,"  %s->type = a_VARARR;\n",varname[i].name);
      varname[i].type = _VARTYPE_G_ARRAY;
    }
  }
  
  fprintf(outfp, "  awka_register_fn(%d, NULL, NULL);\n", func_no);
  for (i=0; i<func_no; i++)
  {
    fprintf(outfp, "  awka_register_fn(%d, \"%s\", %s_fn);\n", i, functions[i], functions[i]);
  }

  if (int_argv)
  {
    argv = process_argv(&argc, int_argv);
    if (argv)
    {
      fprintf(outfp, "  _int_argc = %d;\n",argc);
      fprintf(outfp, "  malloc( &_int_argv, %d * sizeof(char *));\n",argc);
      for (i=0; i<argc; i++)
        fprintf(outfp, "  _int_argv[%d] = \"%s\";\n",i,argv[i]);
      fprintf(outfp, "\n");
    }
  }

  if (env_used == 1)
    fprintf(outfp,"\n  awka_env_used(1);\n\n");
  
  fprintf(outfp,"  awka_init(argc, argv, \"%s\", \"%s\", \"%s\");\n", AWKAVERSION, DATE_STRING, awk_input_files);

  if (functab_used)
    fprintf(outfp,"  awka_init_functab();\n\n");

  if (split_max != 0 && split_max != INT_MAX)
  {
    if (dol0_get && doln_set)
      fprintf(outfp,"\n  _split_max = %d;",INT_MAX);
    else
      fprintf(outfp,"\n  _split_max = %d;",split_max);
  }
  if (split_req == 1)
    fprintf(outfp,"\n  awka_split_req(1);");
  if (dol0_used == 1)
    fprintf(outfp,"\n  awka_dol0_used(1);");
  if (dol0_only == 1)
    fprintf(outfp,"\n  awka_dol0_only(1);");
  fprintf(outfp, "\n\n");

  if (begin_used)
    fprintf(outfp,"  BEGIN();\n");
  if (main_used)
    fprintf(outfp,"  MAIN();\n");
  if (end_used)
    fprintf(outfp,"  END();\n");

  fprintf(outfp, "\n");
  fprintf(outfp,"\n  if (_lvar) free(_lvar);\n");
  for (i=0; i<litd_used; i++)
    fprintf(outfp,"  free(_litd%d_awka);\n",i);
  
  for (i=0; i<lits_used; i++)
    fprintf(outfp,"  free(_lits%d_awka->ptr); free(_lits%d_awka);\n",i,i);
  
  for (i=0; i<litr_used; i++)
    fprintf(outfp,"  awka_killvar(_litr%d_awka); free(_litr%d_awka);\n",i,i);
  fprintf(outfp, "\n");
  
  if (!awka_main)
    fprintf(outfp,"  awka_exit((awka_exit_val < 0) ? 0 : awka_exit_val);\n");
  else
    fprintf(outfp,"  awka_cleanup();\n");
  fprintf(outfp,"}\n");
  
  if (warning_msg & MSG_LIST)
    msg_print_list(varname, var_used);
  if (warning_msg & MSG_SETnREF)
    msg_print_setnref(varname, var_used);
  if (warning_msg & MSG_REFnSET)
    msg_print_refnset(varname, var_used);
  
}
