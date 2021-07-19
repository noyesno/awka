/*------------------------------------------------------------*
 | da.c                                                       |
 | copyright 1999,  Andrew Sumner                             |
 | copyright 1991,  Michael D. Brennan                        |
 |                                                            |
 | This is a source file for the awka package, a translator   |
 | of the AWK programming language to ANSI C.                 |
 |                                                            |
 | The file is a heavily modified version of da.c from        |
 | Mawk, an implementation of the AWK processing language,    |
 | distributed by Michael Brennan under the GPL.              |
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
 *-----------------------------------------------------------*/

/*  da.c  */
/*  disassemble code */

#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <stdarg.h>

#include  "awka.h"
#include  "awka_exe.h"

#include  "bi_funct.h"
#include  "repl.h"
#include  "field.h"

#define isoctal(x)  ((x)>='0'&&(x)<='7')
#define isdigit(x)  ((x)>='8'&&(x)<='9')

static char *PROTO(find_bi_name, (PF_CP)) ;

char **cfunc = NULL;
int cfunc_no = 0;
int cfunc_allc = 0;

static struct sc
{
   char op ;
   char *name ;
} simple_code[] =

{
   _STOP, "stop",
   FE_PUSHA, "fe_pusha",
   FE_PUSHI, "fe_pushi",
   A_TEST, "a_test",
   A_DEL, "a_del",
   DEL_A, "del_a",
   POP_AL, "pop_al",
   _POP, "pop",
   _ADD, "add",
   _SUB, "sub",
   _MUL, "mul",
   _DIV, "div",
   _MOD, "mod",
   _POW, "pow",
   _NOT, "not",
   _UMINUS, "uminus",
   _UPLUS, "uplus",
   _TEST, "test",
   _CAT, "cat",
   _ASSIGN, "assign",
   _ADD_ASG, "add_asg",
   _SUB_ASG, "sub_asg",
   _MUL_ASG, "mul_asg",
   _DIV_ASG, "div_asg",
   _MOD_ASG, "mod_asg",
   _POW_ASG, "pow_asg",
   NF_PUSHI, "nf_pushi",
   F_ASSIGN, "f_assign",
   F_ADD_ASG, "f_add_asg",
   F_SUB_ASG, "f_sub_asg",
   F_MUL_ASG, "f_mul_asg",
   F_DIV_ASG, "f_div_asg",
   F_MOD_ASG, "f_mod_asg",
   F_POW_ASG, "f_pow_asg",
   _POST_INC, "post_inc",
   _POST_DEC, "post_dec",
   _PRE_INC, "pre_inc",
   _PRE_DEC, "pre_dec",
   F_POST_INC, "f_post_inc",
   F_POST_DEC, "f_post_dec",
   F_PRE_INC, "f_pre_inc",
   F_PRE_DEC, "f_pre_dec",
   _EQ, "eq",
   _NEQ, "neq",
   _LT, "lt",
   _LTE, "lte",
   _GT, "gt",
   _GTE, "gte",
   _MATCH2, "match2",
   _ABORT, "abort",
   _ABORT0, "abort0",
   _EXIT, "exit",
   _EXIT0, "exit0",
   _CLEANUP, "exit0",
   _NEXT, "next",
   _NEXTFILE, "next",
   _RET, "ret",
   _RET0, "ret0",
   _OMAIN, "omain",
   _JMAIN, "jmain",
   OL_GL, "ol_gl",
   OL_GL_NR, "ol_gl_nr",
   _HALT, (char *) 0
} ;

static char bslash[256];
static char bslash_set = 0;

void
set_bslash()
{
  memset(bslash, 0, 256);

  bslash['r'] |= 2;
  bslash['t'] |= 2;
  bslash['n'] |= 2;

  bslash['0'] |= 2;

  /* bslash['"'] |= 2; */
  /* bslash['\\'] |= 2;  */

  bslash_set = 1;
}

char *
fixbackslashes(char *str, int which)
{
  char *p = str, *q = str, *r;
  size_t len;

  /*
   * double-up (most) backslashes.
   * The aim here is basically to protect things
   * from the C compiler.  Smart parsing of backslashes
   * for regexps and printfs is left to the library.
   */
   /*
    *  Basically, we try to convert regular expression to a C string format.
    *  -----------------------------------------------------------------
    *  /\\/  => "\\\\"        # insert '\'
    *  /\n|\r|\t/             # no change
    *  /\7/  => "\\7"         # insert '\'
    *  /\./  => "\\."         # insert '\'
    *  /\8/  => "8"           # remove '\'
    *  -----------------------------------------------------------------
    */

  while (*p)
  {
    switch (which)
    {
      case _MATCH:
      case _MATCH0:
      case _MATCH1:
      case _MATCH2:
      case _PUSHC:
        if (*p == '\\')
        {
          q = p+1;
          if ( isdigit(*q) ) {
            /* remove double backslash */
            q = p;
            r = q+1;
            do {
              *(q++) = *(r++);
            } while (*q);
          } else if ((*q == '\\') || !(bslash[*q] & 2)) {
            /* insert double backslash */
            while (*(q++))
              ;
            r = q-1;
            do {
              *(q--) = *(r--);
            } while (r > p);
            *(++p) = '\\';
          }
        }
        p++;
        break;

      default:
        if (*p == '\\')
        {
          q = p+1;
          if (!(bslash[*q] & 2) && *q)
          {
            /* double backslash */
            while (*(q++))
              ;
            r = q-1;
            do {
              *(q--) = *(r--);
            } while (r > p);
            *(++p) = '\\';
          }
          else if (*q == '\\')
            p++;
        }
        else if (*p == '\n')  /* unescape '\n' */
        {
          q = p+1;
          while (*(q++))
            ;
          r = q-1;
          do {
            *(q--) = *(r--);
          } while (r > p);
          *(p++) = '\\';
          *p = 'n';
        }
        p++;
    }
  }

  /* now protect double-quotes */
  p = str;
  len = strlen(str);
  while (*p)
  {
    if (*p == '\\' && (*(p+1) == '\\' || *(p+1) == '"'))
    {
      p += 2;
      continue;
    }
    if (*p == '"')
    {
      len++;
      q = str + len;
      r = q - 1;
      do {
        *(q--) = *(r--);
      } while (r >= p);
      *(p++) = '\\';
    }
    p++;
  }

  /* protect any trailing backslash */
  len = strlen(str);
  p = str;
  while (*p)
  {
    if (*p == '\\' && !*(p+1))
    {
      str[len] = '\\';
      str[len+1] = '\0';
      break;
    }
    if (*p == '\\' && *(p+1) == '\\')
      p++;
    p++;
  }
 
  return str;
}

void
double_backslashes(char *str)
{
  register char *p, *q;
  int len;

  if (!*str) return;
  /* double those backslashes again!!! */
  len = strlen(str);
  p = str + len;
  q = str;
  while (q - str < len)
    *(p++) = *(q++);
  *p = '\0';
  p = str + len;
  q = str;

  do {
    *(q++) = *p;
    if (*p == '\\' && *(p+1) == '\\')
    {
      p++;
      *(q++) = '\\';
      *(q++) = '\\';
      *(q++) = '\\';
    }
    p++;
  } while (*p);
  *q = '\0';
  return;
}

int
findbi(char *c)
{
  int i = BI_MIN + (((BI_MAX-1) - BI_MIN) / 2), hi = BI_MAX, lo = BI_MIN;
  int x;

  while (1)
  {
    if (!(x = strcmp(code[i].name, c)))
      return code[i].op;
    else if (x > 0)
    {
      if (i == lo)
        return 0;
      else if (i-1 == lo)
      {
        if (!strcmp(code[lo].name, c))
          return code[lo].op;
        return 0;
      }
      hi = i;
      i = lo + ((hi - lo) / 2);
    }
    else
    {
      if (i == hi)
        return 0;
      else if (i+1 == hi)
      {
        if (!strcmp(code[hi].name, c))
          return code[hi].op;
        return 0;
      }
      lo = i;
      i = lo + ((hi - lo) / 2);
    }
  }
}

int
findop(char *c)
{
  int i = (CODE_MAX - CODE_MIN) / 2, hi = CODE_MAX, lo = CODE_MIN;
  int x;

  while (1)
  {
    if (!(x = strcmp(code[i].name, c)))
      return code[i].op;
    else if (x > 0)
    {
      if (i == lo)
        return findbi(c);
      else if (i-1 == lo)
      {
        if (!strcmp(code[lo].name, c))
          return code[lo].op;
        return findbi(c);
      }
      hi = i;
      i = lo + ((hi - lo) / 2);
    }
    else
    {
      if (i == hi)
        return findbi(c);
      else if (i+1 == hi)
      {
        if (!strcmp(code[hi].name, c))
          return code[hi].op;
        return findbi(c);
      }
      lo = i;
      i = lo + ((hi - lo) / 2);
    }
  }
}

/* here's where mawk really becomes awka */
void
awka_insertop(int op, char *cval, char *carg, int minst, char *file, int line)
{
  char *val = NULL, *arg = NULL;

  if (cval)
  {
    val = (char *) malloc((strlen(cval)*2)+10);
    strcpy(val, cval);
  }
  if (carg)
  {
    arg = (char *) malloc((strlen(carg)*2)+10);
    strcpy(arg, carg);
  }

  if (prog_no == prog_allc)
  {
    prog_allc += 40;
    progcode = (struct pc *) realloc(progcode, prog_allc * sizeof(struct pc));
  }
  
  if (op == _BUILTIN)
  {
    if (!(op = findop(val)))
      return;
    free(val); val = NULL;
  }

  progcode[prog_no].op = op;
  progcode[prog_no].inst = prog_no;
  progcode[prog_no].minst = minst;
  progcode[prog_no].line = line;
  progcode[prog_no].file = file;
  progcode[prog_no].val = val;
  progcode[prog_no].arg = arg;
  progcode[prog_no].endloop = 0;
  progcode[prog_no].foreverloop = 0;
  progcode[prog_no].doloop = FALSE;
  progcode[prog_no].label = FALSE;
  progcode[prog_no].jumpfrom = progcode[prog_no].jumpto = -1;
  progcode[prog_no].ljumpfrom = -1;
  progcode[prog_no].prevcat = progcode[prog_no].prev2 = -1;
  progcode[prog_no].context = _NUL;
  progcode[prog_no].code = NULL;
  progcode[prog_no].code0 = NULL;
  progcode[prog_no].code_used = progcode[prog_no].code_allc = 0;
  progcode[prog_no].code0_used = progcode[prog_no].code0_allc = 0;
  progcode[prog_no].done = FALSE;
  progcode[prog_no].printed = FALSE;
  progcode[prog_no].earliest = -1;
  progcode[prog_no].ftype = code[op-1].ftype;

  if (op >= CODE_MIN && op <= BI_MAX)
  {
    progcode[prog_no].func = code[op-1].func;
    progcode[prog_no].pop = code[op-1].pop;
    progcode[prog_no].varidx = code[op-1].varidx;
  }
  else
  {
    progcode[prog_no].func = NULL;
    progcode[prog_no].pop = FALSE;
    progcode[prog_no].varidx = -1;
  }

  if (val && op == _PUSHS)
  {
    AWKA_DEBUG("1 progcode[prog_no].val=\"%s\" %ld\n", progcode[prog_no].val, strlen(progcode[prog_no].val));
    progcode[prog_no].val = fixbackslashes(progcode[prog_no].val, op);
    AWKA_DEBUG("2 progcode[prog_no].val=\"%s\" %ld\n", progcode[prog_no].val, strlen(progcode[prog_no].val));
    /* double_backslashes(progcode[prog_no].val); */
  }

  if (arg)
  {
    if (!strncmp(val, "repl", 4))
    {
      progcode[prog_no].arg = fixbackslashes(progcode[prog_no].arg, _PUSHS);
      /* double_backslashes(progcode[prog_no].arg); */
    }
    else
      progcode[prog_no].arg = fixbackslashes(progcode[prog_no].arg, op);
  }
  prog_no++;
}

void
da(start, fp)
   INST *start ;
   FILE *fp ;
{
   CELL *cp ;
   register INST *p = start ;
   char *name, *tmp, *tmp1, *tmp2, *file ;
   int minst, line, op, i;

   if (!cfunc)
   {
     cfunc = (char **) malloc(20 * sizeof(char *));
     cfunc_allc = 20;
   }

   tmp1 = (char *) malloc(256);
   tmp2 = (char *) malloc(256);
   if (!bslash_set) set_bslash();

   while (p->op != _HALT)
   {
      /* print the relative code address (label) */
      minst = p - start;
      line = p->lineno;
      file = p->file;
      op = p++->op;

      switch (op)
      {

          case _PUSHC:
             cp = (CELL *) p++->ptr ;
             switch (cp->type)
             {
                case C_RE:
                    sprintf(tmp1, "0x%lx", (long) cp->ptr);
                    /* sprintf(tmp2, "/%s/", re_uncompile(cp->ptr)); */
                    sprintf(tmp2, "%s", re_uncompile(cp->ptr));
                    if (dump) fprintf(stderr,"%d:\tpushc     \t\t%s\t%d\n",minst,tmp2,line);
                    awka_insertop(_PUSHC, tmp1, tmp2, minst, file, line);
                    break ;

                case C_SPACE:
                    if (dump) fprintf(stderr,"%d:\tpushc     \t\tspace split\t%d\n",minst,line);
                    awka_insertop(_PUSHC, "space", "split", minst, file, line);
                    break ;

                case C_SNULL:
                    if (dump) fprintf(stderr,"%d:\tpushc     \t\tnull split\t%d\n",minst,line);
                    awka_insertop(_PUSHC, "null", "split", minst, file, line);
                    break ;

                case C_REPL:
                    sprintf(tmp2, "%s", repl_uncompile(cp));
                    if (dump) fprintf(stderr,"%d:\tpushc     \t\trepl %s\t%d\n",minst,tmp2,line);
                    awka_insertop(_PUSHC, "repl", tmp2, minst, file, line);
                    break ;

                case C_REPLV:
                    sprintf(tmp2, "%s", repl_uncompile(cp));
                    if (dump) fprintf(stderr,"%d:\tpushc     \t\treplv %s\t%d\n",minst,tmp2,line);
                    awka_insertop(_PUSHC, "replv", tmp2, minst, file, line);
                    break ;

                default:
                    awka_insertop(_PUSHC, "WEIRD", NULL, minst, file, line);
                    break ;
             }
             break ;

          case _PUSHD:
             /* I'm unhappy with this, but at some point the double has
                to be converted to a string to be printed to the C output.
                Does anyone know a means of safely ensuring the correct
                precision is carried through? */
             sprintf(tmp1, "%.25g", *(double *) p++->ptr);
             if (dump) fprintf(stderr,"%d:\tpushd     \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(_PUSHD, tmp1, NULL, minst, file, line);
             break ;
          case _PUSHS:
             {
                STRING *sval = (STRING *) p++->ptr ;
                if (dump) fprintf(stderr,"%d:\tpushs     \t\t%s\t%d\n",minst,sval->str,line);
                awka_insertop(_PUSHS, sval->str, NULL, minst, file, line);
                break ;
             }
          // TODO: diff glob pattern and regexp pattern
          case _MATCH0:
             sprintf(tmp1, "0x%lx", (long) p->ptr);
             sprintf(tmp2, "/%s/", re_uncompile(p->ptr));
             if (dump) fprintf(stderr,"%d:\tmatch0    \t\t%s\t%d\n",minst,tmp2,line);
             awka_insertop(op, tmp1, tmp2, minst, file, line);
             p++ ;
             break;

          case _MATCH1:
             sprintf(tmp1, "0x%lx", (long) p->ptr);
             sprintf(tmp2, "/%s/", re_uncompile(p->ptr));
             if (dump) fprintf(stderr,"%d:\tmatch1    \t\t%s\t%d\n",minst,tmp2,line);
             awka_insertop(op, tmp1, tmp2, minst, file, line);
             p++ ;
             break ;

          case _PUSHA:
             tmp  = reverse_find(ST_VAR, &p->ptr);
	     /* hack: allow array as a var w*/
	     if (strcmp(tmp, "unknown") == 0) {
                tmp = reverse_find(ST_ARRAY, &p->ptr);
	     }
	     p++;
             if (dump) fprintf(stderr,"%d:\tpusha     \t\t%s\t%d\n",minst,tmp,line);
             awka_insertop(op,  tmp, NULL, minst, file, line);
             break ;

          case _PUSHI:
             cp = (CELL *) p++->ptr ;
             if (cp == field)
             {
                if (dump) fprintf(stderr,"%d:\tpushi     \t\t$0\t%d\n",minst,line);
                awka_insertop(op,  "$0", NULL, minst, file, line);
             }
             else if (cp == &fs_shadow)
             {
                if (dump) fprintf(stderr,"%d:\tpushi     \t\t@fs_shadow\t%d\n",minst,line);
                awka_insertop(op,  "@fs_shadow", NULL, minst, file, line);
             }
             else
             {
                /* if ((unsigned long) cp > (unsigned long) NF && (unsigned long) cp <= (unsigned long) LAST_PFIELD)  */
                if (cp > NF && cp <= LAST_PFIELD) 
                    name = reverse_find(ST_FIELD, &cp) ;
                else name = reverse_find(ST_VAR, &cp) ;

		if (strcmp(name, "unknown") == 0) {
                   name = reverse_find(ST_ARRAY, &cp) ;
		}

                if (dump) fprintf(stderr,"%d:\tpushi     \t\t%s\t%d\n",minst,name,line);
                awka_insertop(op, name, NULL, minst, file, line);
             }
             break ;

          case L_PUSHA:
             sprintf(tmp1, "%d", p++->op);
             if (dump) fprintf(stderr,"%d:\tl_pusha   \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             break ;

          case L_PUSHI:
             sprintf(tmp1, "%d", p++->op);
             if (dump) fprintf(stderr,"%d:\tl_pushi   \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             break ;

          case LAE_PUSHI:
             sprintf(tmp1, "%d", p++->op);
             if (dump) fprintf(stderr,"%d:\tlae_pushi \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             break ;

          case LAE_PUSHA:
             sprintf(tmp1, "%d", p++->op);
             if (dump) fprintf(stderr,"%d:\tlae_pusha \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             break ;

          case LA_PUSHA:
             sprintf(tmp1, "%d", p++->op);
             if (dump) fprintf(stderr,"%d:\tla_pusha  \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             break ;

          case F_PUSHA:
             cp = (CELL *) p++->ptr ;
             if (cp >= NF && cp <= LAST_PFIELD)
             {
                tmp = reverse_find(ST_FIELD, &cp);
                if (dump) fprintf(stderr,"%d:\tf_pusha   \t\t%s\t%d\n",minst,tmp,line);
                awka_insertop(op, tmp, NULL, minst, file, line);
             }
             else
             {
                sprintf(tmp1, "$%d", field_addr_to_index(cp)) ;
                if (dump) fprintf(stderr,"%d:\tf_pusha   \t\t%s\t%d\n",minst,tmp1,line);
                awka_insertop(op, tmp1, NULL, minst, file, line);
             }
             break ;

          case F_PUSHI:
             p++ ;
             sprintf(tmp1, "$%d", p++->op);
             if (dump) fprintf(stderr,"%d:\tf_pushi   \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             break ;

          case AE_PUSHA:
             tmp = reverse_find(ST_ARRAY, &p++->ptr);
             if (dump) fprintf(stderr,"%d:\tae_pusha  \t\t%s\t%d\n",minst,tmp,line);
             awka_insertop(op, tmp, NULL, minst, file, line);
             break ;

          case AE_PUSHI:
             tmp = reverse_find(ST_ARRAY, &p++->ptr);
             if (dump) fprintf(stderr,"%d:\tae_pushi  \t\t%s\t%d\n",minst,tmp,line);
             awka_insertop(op, tmp, NULL, minst, file, line);
             break ;

          case A_PUSHA:
             tmp = reverse_find(ST_ARRAY, &p++->ptr);
             if (dump) fprintf(stderr,"%d:\ta_pusha   \t\t%s\t%d\n",minst,tmp,line);
             awka_insertop(op, tmp, NULL, minst, file, line);
             break ;

          case _PUSHINT:
             sprintf(tmp1, "%d", p++->op);
             if (dump) fprintf(stderr,"%d:\tpushint   \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             break ;

          case _BUILTIN:
             if (dump) fprintf(stderr,"%d:\tbuiltin   \t\t%s\t%d\n",minst,find_bi_name((PF_CP) p++->ptr),line);
             awka_insertop(op, find_bi_name((PF_CP) p++->ptr), NULL, minst, file, line);
             break ;

          case _PRINT:
             if (dump) fprintf(stderr,"%d:\t%-10s\t\t\t%d\n",minst,((PF_CP) p++->ptr == bi_printf ? "printf" : "print"),line);
             awka_insertop((PF_CP) p++->ptr == bi_printf ? _PRINTF : _PRINT, NULL, NULL, minst, file, line);
             break ;

          case _JMP:
             sprintf(tmp1, "%d", (int) ((p - start) + p->op));
             if (dump) fprintf(stderr,"%d:\tjmp       \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             p++ ;
             break ;

          case _GOTO:
             sprintf(tmp1, "%d", (int) ((p - start) + p->op));
             if (dump) fprintf(stderr,"%d:\tgoto      \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             p++ ;
             break ;

          case _BREAK:
             sprintf(tmp1, "%d", (int) ((p - start) + p->op));
             if (dump) fprintf(stderr,"%d:\tbreak     \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             p++ ;
             break ;

          case _ELSE:
             sprintf(tmp1, "%d", (int) ((p - start) + p->op));
             if (dump) fprintf(stderr,"%d:\telse      \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             p++ ;
             break ;

          case _COLON:
             sprintf(tmp1, "%d", (int) ((p - start) + p->op));
             if (dump) fprintf(stderr,"%d:\tcolon     \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             p++ ;
             break ;

          case _QMARK:
             sprintf(tmp1, "%d", (int) ((p - start) + p->op));
             if (dump) fprintf(stderr,"%d:\tqmark     \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             p++ ;
             break ;

          case _JNZ:
             sprintf(tmp1, "%d", (int) ((p - start) + p->op));
             if (dump) fprintf(stderr,"%d:\tjnz       \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             p++ ;
             break ;

          case _JZ:
             sprintf(tmp1, "%d", (int) ((p - start) + p->op));
             if (dump) fprintf(stderr,"%d:\tjz      \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             p++ ;
             break ;

          case _LJZ:
             sprintf(tmp1, "%d", (int) ((p - start) + p->op));
             if (dump) fprintf(stderr,"%d:\tljz       \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             p++ ;
             break ;

          case _LJNZ:
             sprintf(tmp1, "%d", (int) ((p - start) + p->op));
             if (dump) fprintf(stderr,"%d:\tljnz       \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             p++ ;
             break ;

          case SET_ALOOP:
             sprintf(tmp1, "%d", (int) ((p - start) + p->op));
             if (dump) fprintf(stderr,"%d:\tset_aloop \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             p++ ;
             break ;

          case ALOOP:
             sprintf(tmp1, "%d", (int) ((p - start) + p->op));
             if (dump) fprintf(stderr,"%d:\taloop     \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             p++ ;
             break ;

          case  A_CAT :
             sprintf(tmp1, "%d", p++->op);
             if (dump) fprintf(stderr,"%d:\ta_cat     \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             break ;

          case _CALL:
             sprintf(tmp1, "%d", p[1].op);
             if (dump) fprintf(stderr,"%d:\tcall      \t\t%s\t%d\n",minst,((FBLOCK *) p->ptr)->name,line);
             awka_insertop(op, ((FBLOCK *) p->ptr)->name, tmp1, minst, file, line);
             for (i=0; i<cfunc_no; i++)
               if (!strcmp(cfunc[i], ((FBLOCK *) p->ptr)->name))
                 break;

             if (i == cfunc_no)
             {
               if (cfunc_no++ == cfunc_allc)
               {
                 cfunc_allc *= 2;
                 cfunc = (char **) realloc(cfunc, cfunc_allc * sizeof(char *));
               }
               cfunc[i] = (char *) malloc(strlen(((FBLOCK *) p->ptr)->name)+1);
               strcpy(cfunc[i],((FBLOCK *) p->ptr)->name);
             }

             p += 2 ;
             break ;

          case _RANGE:
             sprintf(tmp1, "%d %d %d",
                  (int) (p - start + p[1].op),
                  (int) (p - start + p[2].op),
                  (int) (p - start + p[3].op));
             if (dump) fprintf(stderr,"%d:\trange     \t\t%s\t%d\n",minst,tmp1,line);
             awka_insertop(op, tmp1, NULL, minst, file, line);
             p += 4;
             break ;

          default:
             {
                struct sc *q = simple_code ;
                int k = (p - 1)->op ;

                while (q->op != _HALT && q->op != k)
                  q++ ;

                if (q->op == FE_PUSHA)
                {
                  if (dump) fprintf(stderr,"%d:\tfe_pusha  \t\t\t%d\n",minst,line);
                  awka_insertop(_FE_PUSHA, NULL, NULL, minst, file, line);
                }
                else
                {
                  if (dump) fprintf(stderr,"%d:\t%-10s\t\t\t%d\n",minst,q->name,line);
                  awka_insertop(q->op, NULL, NULL, minst, file, line);
                }
             }
             break ;
      }
   }
   fflush(fp) ;
   if (tmp1) free(tmp1);
   if (tmp2) free(tmp2);
}

static struct
{
   PF_CP action ;
   char *name ;
}
special_cases[] =
{
   bi_split, "split",
   bi_match, "match",
   bi_getline, "getline",
   bi_sub, "sub_bi",
   bi_gsub, "gsub",
   (PF_CP) 0, (char *) 0
} ;

static char *
find_bi_name(p)
   PF_CP p ;
{
   BI_REC *q ;
   int i ;
   static char *tmp = "alength", *tmp2 = "asort", *tmp3 = "length";

   if (p == bi_alength) return tmp;
   if (p == bi_asort) return tmp2;
   if (p == bi_length) return tmp3;

   for (q = bi_funct; q->name; q++)
   {
      if (q->fp == p)
      {
         /* found */
         return q->name ;
      }
   }
   /* next check some special cases */
   for (i = 0; special_cases[i].action; i++)
   {
      if (special_cases[i].action == p)    return special_cases[i].name ;
   }

   return "unknown builtin" ;
}

static struct fdump
{
   struct fdump *link ;
   FBLOCK *fbp ;
} *fdump_list ;          /* linked list of all user functions */

void
add_to_fdump_list(fbp)
   FBLOCK *fbp ;
{
   struct fdump *p = (struct fdump *) malloc(sizeof(struct fdump)) ;
   p->fbp = fbp ;
   p->link = fdump_list ;  fdump_list = p ;
}

void
fdump()
{
   register struct fdump *p, *q = fdump_list ;
   int old_cfunc_no = 0, i, j, dfunc_allc = 20, dfunc_no = 0;
   char **dfunc = (char **) malloc(20 * sizeof(char *));

   while (old_cfunc_no < cfunc_no)
   {
     old_cfunc_no = cfunc_no;
     q = fdump_list;
     while (q)
     {
       p = q ;
       q = p->link ;
       j = strlen(p->fbp->name);
       if (p->fbp->name[j-1] == '(')
         j--;
       
       for (i=0; i<cfunc_no; i++)
       {
         if (!strncmp(cfunc[i], p->fbp->name, j) &&
            (cfunc[i][j] == '(' || cfunc[i][j] == '\0'))
           break;
       }

       if (i < cfunc_no)
       {
         for (j=0; j<dfunc_no; j++)
           if (cfunc[i] == dfunc[j])
             break;

         if (j == dfunc_no)
         {
           awka_insertop(_FUNCTION, p->fbp->name, NULL, 0, NULL, 0);
           if (dump) fprintf(stderr, "function %s\n",p->fbp->name);
           da(p->fbp->code, stdout) ;
           dfunc[dfunc_no++] = cfunc[i];

           if (dfunc_no == dfunc_allc)
           {
             dfunc_allc *= 2;
             dfunc = (char **) realloc(dfunc, dfunc_allc * sizeof(char *));
           }
         }
       }
     }
   }

   free(dfunc);
}

