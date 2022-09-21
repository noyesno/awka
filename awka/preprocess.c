/*------------------------------------------------------------*
 | preprocess.c                                               |
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include <stdarg.h>
#include "awka.h"
#include "msg.h"
#include "../config.h"
#include "mem.h"
#include "memory.h"
#include "awka_exe.h"

extern struct ivar_idx ivar[];
extern int indent;
extern int split_req, split_max, mode, dol0_used;
extern int var_allc, var_used;
extern int lvar_allc, lvar_used;
extern int litr_allc, litr_used;
extern int lits_allc, lits_used;
extern int litd_allc, litd_used;
extern awka_varname *varname;
extern char **litrname, **litsname, **litdname;
extern char **litr_val, **lits_val, **litd_val;
extern FILE *outfp;
extern int awka_main;
extern char *awka_main_func;
extern int cur_func;
extern int al_count;
extern int array_sort;
extern int range_no;
extern int max_call_args;
extern char **vardeclare;
extern int vdec_no;
extern char **functions;
extern int func_no;
char *lvar_type = NULL;

void awka_warning(char *, ...);
int findivar(char *);

int
is_ext_builtin(name)
  char *name;
{
  struct a_sc *fp;

  fp = ext_funcs;
  while (fp->name)
  {
    if (!strcmp(fp->name, name))
      return 1;
    fp++;
  }

  return 0;
}

void
moveprog(int k, int i)
{
  progcode[i].op = progcode[k].op;
  progcode[i].pop = progcode[k].pop;
  progcode[i].func = progcode[k].func;
  progcode[i].val = progcode[k].val;
  progcode[i].arg = progcode[k].arg;
  progcode[i].minst = progcode[k].minst;
  progcode[i].inst = progcode[k].inst;
  progcode[i].line = progcode[k].line;
  progcode[i].jumpfrom = progcode[k].jumpfrom;
  progcode[i].jumpto = progcode[k].jumpto;
  progcode[i].earliest = progcode[k].earliest;
  progcode[i].done = progcode[k].done;
  progcode[i].endloop = progcode[k].endloop;
  progcode[i].doloop = progcode[k].doloop;
  progcode[i].context = progcode[k].context;
  progcode[i].label = progcode[k].label;
  progcode[i].varidx = progcode[k].varidx;
  progcode[i].ftype = progcode[k].ftype;
  progcode[i].file = progcode[k].file;
}

int
findvarname(awka_varname *vname, char *name, int used)
{
  int i;

  if (!vname) return -1;
  for (i=0; i<used; i++)
    if (!strcmp(vname[i].name, name))
      return i;
  return -1;
}

void
initlvartypes()
{
  int i;

  if (!lvar_type)
    lvar_type = (char *) malloc(128);
  memset(lvar_type, _VALTYPE_UNK, 128);
}

void
setlvartype(int idx, char flag)
{
  if (idx > 127) return;

  lvar_type[idx] = flag;
}

int
getlvartype(int idx)
{
  if (idx > 127) return _VALTYPE_UNK;
  return (int) lvar_type[idx];
}

int
findvaltype(char *name)
{
  int i;

  if (!strncmp(name, "_litd", 5))
    return _VALTYPE_NUM;
  
  if (!strncmp(name, "_lits", 5))
    return _VALTYPE_STR;

  if (!strncmp(name, "_lvar[", 6))
    return getlvartype(atoi(name+6));

  if (!strcmp(name, "a_bivar[a_NF]") || 
      !strcmp(name, "a_bivar[a_NR]") ||
      !strcmp(name, "awka_NFget()"))
    return _VALTYPE_NUM;

  if ((i = findvarname(varname, name, var_used)) == -1)
    return _VALTYPE_UNK;

  if (varname[i].valtype == _VALTYPE_UNK || 
      varname[i].valtype >= _VALTYPE_NUM + _VALTYPE_STR)
    return _VALTYPE_UNK;

  return varname[i].valtype;
}

char *
getstringvalue(char *name)
{
  int i;
  char *p = name, *buf;
  static char *buf1 = NULL, *buf2 = NULL;
  static int alloc = 0;
  static char last = 0;

  if (!strncmp(name, "_lits", 5))
  {
    i = atoi(name+5);
    p = lits_val[i];
  }
  
  i = strlen(p);
  if (alloc == 0)
  {
    alloc = i + 6;
    alloc += 16 - (alloc % 16);
    buf1 = (char *) malloc(alloc);
    buf2 = (char *) malloc(alloc);
  }
  else if ((i+6) >= alloc)
  {
    alloc = i + 6;
    alloc += 16 - (alloc % 16);
    buf1 = (char *) realloc(buf1, alloc);
    buf2 = (char *) realloc(buf2, alloc);
  }

  if (last == 0)
  {
    buf = buf1;
    last = 1;
  }
  else
  {
    buf = buf2;
    last = 0;
  }

  if (p == name)
    sprintf(buf, "%s->ptr", p);
  else
    sprintf(buf, "\"%s\"", p);
  return buf;
}

char *
getdoublevalue(char *name)
{
  int i;
  char *p, *buf;
  static char *buf1 = NULL, *buf2 = NULL;
  static int alloc = 0;
  static char last = 0;

  if (!strncmp(name, "_litd", 5))
  {
    i = atoi(name+5);
    if (i < litd_used && i >= 0)
      return litd_val[i];
  }
  
  i = strlen(name);
  if (alloc == 0)
  {
    alloc = i + 7;
    alloc += 16 - (alloc % 16);
    buf1 = (char *) malloc(alloc);
    buf2 = (char *) malloc(alloc);
  }
  else if ((i+7) >= alloc)
  {
    alloc = i + 7;
    alloc += 16 - (alloc % 16);
    buf1 = (char *) realloc(buf1, alloc);
    buf2 = (char *) realloc(buf2, alloc);
  }

  if (last == 0)
  {
    buf = buf1;
    last = 1;
  }
  else
  {
    buf = buf2;
    last = 0;
  }

  sprintf(buf, "%s->dval", name);
  return buf;
}

void
setvaltype(char *name, char flag)
{
  int i;

  if (*name == '(' || !strncmp(name, "_lit", 4))
    return;

  if (!strncmp(name, "_lvar[", 6))
  {
    setlvartype(atoi(name+6), flag);
    return;
  }

  if ((i = findvarname(varname, name, var_used)) == -1)
    return;

  varname[i].valtype |= flag;
}

void
setvaltype2(char *n1, char *n2)
{
  int i, j, type, local = -1;

  if (*n2 == '(') return;
  if (*n1 == '(' || !strncmp(n1, "_lit", 4))
    return;

  if (!strncmp(n1, "_lvar[", 6))
  {
    local = atoi(n1+6);
    if (local > 127) return;
  }
  else if ((i = findvarname(varname, n1, var_used)) == -1)
    return;
  if (!strncmp(n2, "_lvar[", 6))
  {
    if ((type = atoi(n2+6)) > 127)
      return;
    type = lvar_type[type];
  }
  else if ((j = findvarname(varname, n2, var_used)) != -1)
    type = varname[j].valtype;
  else
    return;

  if (local != -1)
    lvar_type[local] = type;
  else
    varname[i].valtype |= type;
}

void
addvarname(awka_varname **vname, char *name, char type)
{
  int i = var_used;
  awka_varname *pvname = *vname;

  if (!pvname)
  {
    var_allc = 10;
    pvname = (awka_varname *) malloc(var_allc * sizeof(awka_varname));
  }
  else if (++var_used == var_allc)
  {
    var_allc += 10;
    pvname = (awka_varname *) realloc(pvname, var_allc * sizeof(awka_varname));
  }
  pvname[i].name = (char *) malloc(strlen(name)+1);
  strcpy(pvname[i].name, name);
  pvname[i].usage = (unsigned int *) malloc( 16 * sizeof(int) );
  pvname[i].lines = (unsigned int *) malloc( 16 * sizeof(int) );
  pvname[i].files = (char **) malloc( 16 * sizeof(char *) );
  pvname[i].line_no = 0;
  pvname[i].line_allc = 16;
  pvname[i].type = _VARTYPE_G_SCALAR;
  pvname[i].valtype = _VALTYPE_UNK;
  fprintf(outfp, "a_VAR *%s = NULL;\n",pvname[i].name); 
  *vname = pvname;
}

void
laddvarname(awka_varname **vname, char *name, char type)
{
  int i = lvar_used;
  awka_varname *pvname = *vname;

  if (!pvname)
  {
    lvar_allc = 10;
    pvname = (awka_varname *) malloc(lvar_allc * sizeof(awka_varname));
  }
  else if (++lvar_used == var_allc)
  {
    lvar_allc += 10;
    pvname = (awka_varname *) realloc(pvname, lvar_allc * sizeof(awka_varname));
  }
  pvname[i].name = (char *) malloc(strlen(name)+1);
  strcpy(pvname[i].name, name);
  pvname[i].usage = (unsigned int *) malloc( 16 * sizeof(int) );
  pvname[i].lines = (unsigned int *) malloc( 16 * sizeof(int) );
  pvname[i].files = (char **) malloc( 16 * sizeof(char *) );
  pvname[i].line_no = 0;
  pvname[i].line_allc = 16;
  pvname[i].type = _VARTYPE_G_SCALAR;
  /*fprintf(outfp, "a_VAR *%s = NULL;\n",pvname[i].name); */
  *vname = pvname;
}

void
addvarname_ref(awka_varname *vname, int i, char op, char *file, unsigned int line)
{
  int j;
  j = vname[i].line_no++;
  if (vname[i].line_no >= vname[i].line_allc)
  {
    vname[i].line_allc *= 2;
    vname[i].lines = (unsigned int *) realloc(vname[i].lines, vname[i].line_allc * sizeof(int));
    vname[i].usage = (unsigned int *) realloc(vname[i].usage, vname[i].line_allc * sizeof(int));
    vname[i].files = (char **) realloc(vname[i].files, vname[i].line_allc * sizeof(char *));
  }
  vname[i].lines[j] = line;
  vname[i].usage[j] = op;
  vname[i].files[j] = file;
}

char *
fix_litsval( char *str )
{
  static char *output = NULL;
  char *p, *q;
  int len;
  static int alloc = 0;

  if (!str) return "";
  len = strlen(str);

  if (!alloc)
  {
    output = (char *) malloc( len * 2 + 1 );
    alloc = len * 2 + 1;
  }
  else if (alloc < (len * 2 + 1))
  {
    alloc = len * 2 + 1;
    output = (char *) realloc( output, alloc );
  }

  p = output;
  q = str;

  while (*q) {
    if (*q == '\r')
    {
      *p = '\\'; ++p;
      *p = 'r'; ++p; ++q;
    }
    else
    {
      *p = *q;
      p++; q++;
    }
  }
  *p = '\0';

  return output;
}

void
preprocess()
{
  int cur = 0;
  int i, j, j2, j3, k, p;
  char *x, *q;
  int func_allc = 10;
  struct a_sc *ebp;

  functions = (char **) malloc( 10 * sizeof(char *) );
  varname = (awka_varname *) malloc( 10 * sizeof(awka_varname) );
  var_allc = 10;

  /* PREPROCESSING - identify variables & code jumps */
  for (cur=0; cur<prog_no; cur++)
  {
    switch (progcode[cur].op)
    {
      case _END:
        end_used = TRUE;
        break;

      case _BEGIN:
        begin_used = TRUE;
        break;

      case _MAIN:
        main_used = TRUE;
        break;

      case _PUSHI:
        if (!strcmp(progcode[cur].val, "@fs_shadow")) break;
        if (progcode[cur].val[0] == '$')
        {
          progcode[cur].op = F_PUSHI;
          progcode[cur].func = code[F_PUSHI-1].func;
          break;
        }
      case A_PUSHA:
      case AE_PUSHA:
      case AE_PUSHI:
        if (progcode[cur].op != _PUSHI)
          add2arraylist(progcode[cur].val);
      case _PUSHA:
        if ((i = findivar(progcode[cur].val)) != -1)
        {
          if (strcmp(ivar[i].vname, "a_bivar[a_FUNCTAB]") == 0)
            functab_used = TRUE;

          if (!strcmp(ivar[i].vname, "a_bivar[a_ARGV]") && progcode[cur].op != AE_PUSHI)
          {
            progcode[cur].val = (char *) malloc(15);
            strcpy(progcode[cur].val, "awka_argv()");
          }
          else
          {
            progcode[cur].val = (char *) malloc(strlen(ivar[i].vname)+1);
            strcpy(progcode[cur].val, ivar[i].vname);
          }
          break;
        }
        /* adding _awk on end avoids conflicts with system defined names */
        x = (char *) malloc(strlen(progcode[cur].val) + 6);
        sprintf(x, "%s_awk", progcode[cur].val);
        free(progcode[cur].val);
        progcode[cur].val = x;

        for (i=0; i<var_used; i++)
          if (!strcmp(varname[i].name, progcode[cur].val))
            break;

        if (i == var_used)
          addvarname(&varname, progcode[cur].val, _VARTYPE_G_SCALAR);
        break;

      case _MATCH0:
      case _MATCH1:
      case _PUSHC:
        if (progcode[cur].val[0] != '0')
          break;

        for (i=0; i<litr_used; i++)
          if (!strcmp(litr_val[i], progcode[cur].arg))
            break;

        if (i == litr_used)
        {
          if (litr_allc == 0)
          {
            litr_allc = 20;
            litrname = (char **) malloc(20 * sizeof(char *));
            litr_val = (char **) malloc(20 * sizeof(char *));
          }
          else if (litr_used == litr_allc)
          {
            litr_allc += 20;
            litrname = (char **) realloc(litrname, litr_allc * sizeof(char *));
            litr_val = (char **) realloc(litr_val, litr_allc * sizeof(char *));
          }

          litrname[i] = (char *) malloc(20);
          sprintf(litrname[i], "_litr%d_awka", i);
          /* litr_val[i] = progcode[cur].arg; */
          litr_val[i] = (char *) malloc(strlen(progcode[cur].arg)*2);
          strcpy(litr_val[i], fix_litsval(progcode[cur].arg));
          litr_used++;
        }
        progcode[cur].arg = (char *) malloc(20);
        strcpy(progcode[cur].arg, litrname[i]);
        break;


      case _PUSHD:
        for (i=0; i<litd_used; i++)
          if (!strcmp(litd_val[i], progcode[cur].val))
            break;

        if (i == litd_used)
        {
          if (litd_allc == 0)
          {
            litd_allc = 20;
            litdname = (char **) malloc(20 * sizeof(char *));
            litd_val = (char **) malloc(20 * sizeof(char *));
          }
          else if (litd_used == litd_allc)
          {
            litd_allc += 20;
            litdname = (char **) realloc(litdname, litd_allc * sizeof(char *));
            litd_val = (char **) realloc(litd_val, litd_allc * sizeof(char *));
          }

          litdname[i] = (char *) malloc(20);
          sprintf(litdname[i], "_litd%d_awka", i);
          litd_val[i] = (char *) malloc(strlen(progcode[cur].val)+1);
          strcpy(litd_val[i], progcode[cur].val);
          litd_used++;
        }

        progcode[cur].val = (char *) malloc(20);
        strcpy(progcode[cur].val, litdname[i]);
        break;

      case _PUSHS:
        for (i=0; i<lits_used; i++)
          if (!strcmp(lits_val[i], progcode[cur].val))
            break;

        if (i == lits_used)
        {
          if (lits_allc == 0)
          {
            lits_allc = 20;
            litsname = (char **) malloc(20 * sizeof(char *));
            lits_val = (char **) malloc(20 * sizeof(char *));
          }
          else if (lits_used == lits_allc)
          {
            lits_allc += 20;
            litsname = (char **) realloc(litsname, lits_allc * sizeof(char *));
            lits_val = (char **) realloc(lits_val, lits_allc * sizeof(char *));
          }

          litsname[i] = (char *) malloc(20);
          sprintf(litsname[i], "_lits%d_awka", i);
          lits_val[i] = (char *) malloc((strlen(progcode[cur].val)*2)+1);
          strcpy(lits_val[i], fix_litsval(progcode[cur].val));
          lits_used++;
        }

        progcode[cur].val = (char *) malloc(20);
        strcpy(progcode[cur].val, litsname[i]);
        break;

      case _LJNZ:
      case _LJZ:
        j = atoi(progcode[cur].val);
        progcode[cur].jumpto = -1;
        if (j > progcode[cur].minst)
        {
          for (i=cur+1; i<prog_no; i++)
          {
            if (progcode[i].minst > j ||
                progcode[i].minst < progcode[cur].minst)
              break;
            if (progcode[i].minst == j)
            {
              progcode[i].ljumpfrom = cur;
              progcode[cur].jumpto = i;
              break;
            }
          }
        }
        else
        {
          for (i=cur-1; i>=0; i--)
          {
            if (progcode[i].minst < j ||
                progcode[i].minst > progcode[cur].minst)
              break;
            if (progcode[i].minst == j)
            {
              progcode[i].ljumpfrom = cur;
              progcode[cur].jumpto = i;
              break;
            }
          }
        }
        if (progcode[cur].jumpto == -1)
          awka_error("parse error: lj[n]z target not found, line %d.\n",progcode[cur].line);
        break;

      case _JMP:
        j = atoi(progcode[cur].val);
        progcode[cur].jumpto = -1;
        if (j > progcode[cur].minst)
        {
          for (i=cur+1; i<prog_no; i++)
          {
            if (progcode[i].minst < progcode[cur].minst)
              break;
            if (progcode[i].minst >= j && progcode[i].op == _JNZ)
            {
              progcode[i].jumpfrom = cur;
              progcode[i].jumpto = cur+1;
              progcode[cur].jumpto = i;
              break;
            }
          }
        }
        else
        {
          for (i=cur; i>=0; i--)
          {
            if (progcode[i].minst > progcode[cur].minst)
              break;
            if (progcode[i].minst == j)
            {
              progcode[i].jumpfrom = cur;
              progcode[i].jumpto = cur+1;
              progcode[cur].jumpto = i;
              if (i == cur)
                progcode[i].foreverloop = 2;
              else
              {
                progcode[i].foreverloop = 1;
                progcode[cur].endloop++;
              }
              break;
            }
          }
        }
        if (progcode[cur].jumpto == -1 && progcode[cur].op == _JMP)
          awka_error("parse error: jmp target not found, line %d.\n",progcode[cur].line);
        break;

      case SET_ALOOP:
      case _JZ:
      case _QMARK:
        j = atoi(progcode[cur].val);
        progcode[cur].jumpto = -1;
        if (j > progcode[cur].minst)
        {
          for (i=cur+1; i<prog_no; i++)
          {
            if (progcode[i].minst > j ||
                progcode[i].minst < progcode[cur].minst)
              break;
            if (progcode[i].minst == j)
            {
              progcode[cur].jumpto = i;
              if (progcode[cur].op != _QMARK)
                progcode[i].endloop++;
              break;
            }
          }
        }
        else
        {
          for (i=cur-1; i>=0; i--)
          {
            if (progcode[i].minst < j ||
                progcode[i].minst > progcode[cur].minst)
              break;
            if (progcode[i].minst == j)
            {
              progcode[cur].jumpto = i;
              if (progcode[cur].op != _QMARK)
                progcode[i].endloop++;
              break;
            }
          }
        }
        if (progcode[cur].jumpto == -1)
          awka_error("parse error: set_al/jz/qmark target not found, line %d.\n",progcode[cur].line);
        if (progcode[cur].op != SET_ALOOP)
        {
          if (progcode[cur].op == _QMARK)
          {
            j = atoi(progcode[i-1].val);
            k = progcode[i-1].minst;
            for (p=i; p<prog_no; p++)
            {
              if (progcode[p].minst > j ||
                  progcode[p].minst < k)
                break;
              if (progcode[p].minst == j)
              {
                for (k=i; k<p; k++)
                  moveprog(k, k-1);
                k--;
                progcode[k].op = _COLON;
                progcode[k].func = code[_COLON-1].func;
                break;
              }
            }
          }
          else if (progcode[i-1].op == _ELSE)
          {
            /* wimping out of a proper method */
            progcode[i-1].op = _GOTO;
            progcode[i-1].func = code[_GOTO-1].func;
          }
          progcode[cur].jumpto = -1;
        }
        break;

      case _JNZ:
        /* a while or a do/while statement */
        j = atoi(progcode[cur].val);
        if (progcode[cur].jumpto == -1 || progcode[cur].jumpfrom == -1)
        {
          progcode[cur].jumpto = -1;
          if (j > progcode[cur].minst)
          {
            for (i=cur+1; i<prog_no; i++)
            {
              if (progcode[i].minst > j ||
                  progcode[i].minst < progcode[cur].minst)
                break;
              if (progcode[i].minst == j)
              {
                progcode[cur].jumpto = i;
                break;
              }
            }
          }
          else
          {
            for (i=cur-1; i>=0; i--)
            {
              if (progcode[i].minst < j ||
                  progcode[i].minst > progcode[cur].minst)
                break;
              if (progcode[i].minst == j)
              {
                progcode[cur].jumpto = i;
                break;
              }
            }
          }
          if (progcode[cur].jumpto == -1)
            awka_error("parse error: jnz target not found, line %d.\n",progcode[cur].line);
          /* progcode[cur].jumpto = -1; */
          if (cur > 0)
            if (progcode[cur-1].op == _JMP)
              break;  /* while loop */
          progcode[i].doloop = TRUE;
        }
        break;

      case _GOTO:
        j = atoi(progcode[cur].val);
        if (j > progcode[cur].minst)
        {
          for (i=cur+1; i<prog_no; i++)
          {
            if (progcode[i].minst > j ||
                progcode[i].minst < progcode[cur].minst)
              break;
            if (progcode[i].minst == j)
            {
              progcode[i].label = TRUE;
              break;
            }
          }
        }
        else
        {
          for (i=cur-1; i>=0; i--)
          {
            if (progcode[i].minst < j ||
                progcode[i].minst > progcode[cur].minst)
              break;
            if (progcode[i].minst == j)
            {
              progcode[i].label = TRUE;
              break;
            }
          }
        }
        break;

      case _CALL:
        i = strlen(progcode[cur].val);
        if (progcode[cur].val[i-1] == '(')
          progcode[cur].val[i-1] = '\0';
        j = atoi(progcode[cur].arg);
        max_call_args = (max_call_args > j ? max_call_args : j);
        break;

      case _FUNCTION:
        i = strlen(progcode[cur].val);
        if (progcode[cur].val[i-1] == '(')
          progcode[cur].val[i-1] = '\0';
        for (i=0; i<func_no; i++)
          if (!strcmp(functions[i], progcode[cur].val))
            break;

        if (i == func_no)
        {
          if (func_no == func_allc)
          {
            func_allc *= 2;
            functions = (char **) realloc(functions, func_allc * sizeof(char *));
          }
          functions[i] = (char *) malloc(strlen(progcode[cur].val)+1);
          strcpy(functions[i], progcode[cur].val);
          func_no++;
        }
        break;
    }

  }

  /* another pass to decide whether calls are to functions or to 
     extended builtins. */
  for (cur=0; cur<prog_no; cur++)
  {
    /* find extended builtins & see if the're locals */
    for (ebp = ext_funcs; ebp->name != NULL; ebp++)
      if (ebp->op == progcode[cur].op)
        break;

    if (ebp->name)
    {
      for (i=0; i<func_no; i++)
        if (!strcmp(functions[i], ebp->name))
          break;

      if (i < func_no)
      {
        progcode[cur].func = awka_call;
        progcode[cur].op = _CALL;
        progcode[cur].pop = FALSE;
        progcode[cur].val = functions[i];
        if (cur == 0 || progcode[cur-1].inst != _PUSHINT)
        {
          progcode[cur].arg = (char *) malloc(20);
          sprintf(progcode[cur].arg, "%d", _a_bi_vararg[progcode[cur].varidx].min_args);
        }
        progcode[cur].varidx = -1;
      }
      continue;
    }

    /* find locals & see if the're extended builtins */
    if (progcode[cur].op == _CALL)
    {
      for (i=0; i<func_no; i++)
        if (!strcmp(functions[i], progcode[cur].val))
          break;

      if (i == func_no)
      {
        /* local definition don't exist - is it an extended func? */
        for (ebp = ext_funcs; ebp->name != NULL; ebp++)
          if (!strcmp(progcode[cur].val, ebp->name))
          {
            progcode[cur].op = ebp->op;
            progcode[cur].varidx = code[ebp->op-1].varidx;
            progcode[cur].pop = code[ebp->op-1].pop;
            progcode[cur].func = code[ebp->op-1].func;
            break;
          }
      }
    }

  }

  /* set up range patterns */
  for (cur=prog_no-1; cur>=0; cur--)
  {
    if (progcode[cur].op == _RANGE)
    {
      j = atoi(progcode[cur].val);
      k = progcode[cur].endloop;
      q = progcode[cur].val;
      while (*q && *q != ' ') q++;
      if (!(*q))
        awka_error("parse error: range value not set correctly, line %d.\n",progcode[cur].line);
      j2 = atoi(++q);
      while (*q && *q != ' ') q++;
      if (!(*q))
        awka_error("parse error: range value not set correctly, line %d.\n",progcode[cur].line);
      j3 = atoi(q);

      /* find end of patterns */
      for (i=cur+1; i<prog_no; i++)
      {
        if (progcode[i].minst > j2)
          awka_error("parse error: can't find range target (1), line %d.\n", progcode[cur].line);
        if (progcode[i].minst == j2)
          break;
      }
      if (i == prog_no)
        awka_error("parse error: can't find range target (2), line %d.\n", progcode[cur].line);
      j2 = i-1;

      /* find jumpto opcode */
      for (i=j2+1; i<prog_no; i++)
      {
        if (progcode[i].minst > j3)
          awka_error("parse error: can't find range target (3), line %d.\n", progcode[cur].line);
        if (progcode[i].minst == j3)
          break;
      }
      if (i == prog_no)
        awka_error("parse error: can't find range target (4), line %d.\n", progcode[cur].line);
      j3 = i;
      progcode[i].label = TRUE;
      progcode[cur+1].label = progcode[cur].label;
      progcode[cur+1].minst = progcode[cur].minst;

      /* move range opcode to end of patterns */
      for (i=cur+1; i<=j2; i++)
        moveprog(i, i-1);
      progcode[--i].op = _RANGE;
      progcode[i].val = (char *) malloc(20);
      sprintf(progcode[i].val, "%d", progcode[j3].minst);
      progcode[i].arg = NULL;
      progcode[i].func = awka_range;
      progcode[i].jumpto = j3;
      progcode[i].endloop = k;
    }
  }

  if (warning_msg & MSG_VARDECLARE)
  {
    for (i=0; i<var_used; i++)
    {
      for (j=0; j<vdec_no; j++)
        if (!strcmp(varname[i].name, vardeclare[j]))
          break;
      if (j == vdec_no)
        awka_warning("global variable '%s' not declared in VDECL comment.\n",varname[i].name);
    }
  }

  fprintf(outfp, "\nstatic a_VAR **_lvar;\n");

  if (litd_used)
  {
    fprintf(outfp, "static a_VAR *_litd0_awka=NULL");
    for (i=1; i<litd_used; i++)
      fprintf(outfp, ", *_litd%d_awka=NULL",i);
    fprintf(outfp, ";\n");
  }

  if (lits_used)
  {
    fprintf(outfp, "static a_VAR *_lits0_awka=NULL");
    for (i=1; i<lits_used; i++)
      fprintf(outfp, ",\n\t*_lits%d_awka=NULL",i);
    fprintf(outfp, ";\n");
  }

  if (litr_used)
  {
    fprintf(outfp, "static a_VAR *_litr0_awka=NULL");
    for (i=1; i<litr_used; i++)
      fprintf(outfp, ", *_litr%d_awka=NULL",i);
    fprintf(outfp, ";\n");
  }

  for (i=0; i<func_no; i++)
    fprintf(outfp, "static a_VAR * %s_fn(a_VARARG *);\n",functions[i]);
  if (begin_used)
    fprintf(outfp, "static void BEGIN();\n");
  if (main_used)
    fprintf(outfp, "static void MAIN();\n");
  if (end_used)
    fprintf(outfp, "static void END();\n");
}

