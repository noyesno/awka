/*------------------------------------------------------------*
 | print.c                                                    |
 | copyright 1991,  Michael D. Brennan                        |
 |                                                            |
 | This is a source file for the awka package, a translator   |
 | of the AWK programming language to ANSI C.                 |
 |                                                            |
 | The file is a modified version of print.c from             |
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

#include "awka.h"
#include "bi_vars.h"
#include "bi_funct.h"
#include "memory.h"
#include "field.h"
#include "scan.h"
#include "files.h"

static void PROTO(print_cell, (CELL *, FILE *)) ;
static STRING *PROTO(do_printf, (FILE *, char *, unsigned, CELL *)) ;
static void PROTO(bad_conversion, (int, char *, char *)) ;
static void PROTO(write_error,(void)) ;

/* prototyping fprintf() or sprintf() is a loser as ellipses will
   always cause problems with ansi compilers depending on what
   they've already seen,
   but we need them here and sometimes they are missing
*/

#ifdef NO_FPRINTF_IN_STDIO
int PROTO(fprintf, (FILE *, const char *,...)) ;
#endif
#ifdef NO_SPRINTF_IN_STDIO
int PROTO(sprintf, (char *, const char *,...)) ;
#endif

/* this can be moved and enlarged  by -W sprintf=num  */
char *sprintf_buff = string_buff ;
char *sprintf_limit = string_buff + SPRINTF_SZ ;

/* Once execute() starts the sprintf code is (belatedly) the only
   code allowed to use string_buff  */

static void
print_cell(p, fp)
   register CELL *p ;
   register FILE *fp ;
{
   int len ;

   switch (p->type)
   {
      case C_NOINIT:
         break ;
      case C_MBSTRN:
      case C_STRING:
      case C_STRNUM:
         switch (len = string(p)->len)
         {
            case 0:
               break ;
            case 1:
               putc(string(p)->str[0], fp) ;
               break ;

            default:
               fwrite(string(p)->str, 1, len, fp) ;
         }
         break ;

      case C_DOUBLE:
         {
            Int ival = d_to_I(p->dval) ;

            /* integers print as "%[l]d" */
            if ((double) ival == p->dval)  fprintf(fp, INT_FMT, ival) ;
            else  fprintf(fp, string(OFMT)->str, p->dval) ;
         }
         break ;

      default:
         bozo("bad cell passed to print_cell") ;
   }
}

CELL *
bi_print(sp)
   CELL *sp ;                         /* stack ptr passed in */
{
   return sp ;
}

/*---------- types and defs for doing printf and sprintf----*/
#define         PF_C                0        /* %c */
#define         PF_S                1        /* %s */
#define         PF_D                2        /* int conversion */
#define         PF_F                3        /* float conversion */

/* for switch on number of '*' and type */
#define         AST(num,type)        ((PF_F+1)*(num)+(type))

/* some picky ANSI compilers go berserk without this */
#ifdef NO_PROTOS
typedef int (*PRINTER) () ;
#else
typedef int (*PRINTER) (PTR, const char *,...) ;
#endif

/*-------------------------------------------------------*/

static void
bad_conversion(cnt, who, format)
   int cnt ;
   char *who, *format ;
{
   rt_error("improper conversion(number %d) in %s(\"%s\")",
            cnt, who, format) ;
}

CELL *
bi_printf(sp)
   register CELL *sp ;
{
   return --sp ;
}

CELL *
bi_sprintf(sp)
   CELL *sp ;
{
   return sp ;
}


static void 
write_error()
{
   errmsg(errno, "write failure") ;
   exit(2) ;
}
