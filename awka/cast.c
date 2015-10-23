/*------------------------------------------------------------*
 | cast.c                                                     |
 | copyright 1999,  Andrew Sumner                             |
 | copyright 1991,  Michael D. Brennan                        |
 |                                                            |
 | This is a source file for the awka package, a translator   |
 | of the AWK programming language to ANSI C.                 |
 |                                                            |
 | The file is a modified version of cast.c from Mawk,        |
 | an implementation of the AWK processing language,          |
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

/*  cast.c  */

#include "awka.h"
#include "field.h"
#include "memory.h"
#include "scan.h"
#include "repl.h"

int mpow2[NUM_CELL_TYPES] =
{1, 2, 4, 8, 16, 32, 64, 128, 256, 512} ;

void
cast1_to_d(cp)
   register CELL *cp ;
{
   switch (cp->type)
   {
      case C_NOINIT:
         cp->dval = 0.0 ;
         break ;

      case C_DOUBLE:
         return ;

      case C_MBSTRN:
      case C_STRING:
         {
            register STRING *s = (STRING *) cp->ptr ;

#if FPE_TRAPS_ON                /* look for overflow error */
            errno = 0 ;
            cp->dval = strtod(s->str, (char **) 0) ;
            if (errno && cp->dval != 0.0)        /* ignore underflow */
               rt_error("overflow converting %s to double", s->str) ;
#else
            cp->dval = strtod(s->str, (char **) 0) ;
#endif
            free_STRING(s) ;
         }
         break ;

      case C_STRNUM:
         /* don't need to convert, but do need to free the STRING part */
         free_STRING(string(cp)) ;
         break ;


      default:
         bozo("cast on bad type") ;
   }
   cp->type = C_DOUBLE ;
}

void
cast2_to_d(cp)
   register CELL *cp ;
{
   register STRING *s ;

   switch (cp->type)
   {
      case C_NOINIT:
         cp->dval = 0.0 ;
         break ;

      case C_DOUBLE:
         goto two ;
      case C_STRNUM:
         free_STRING(string(cp)) ;
         break ;

      case C_MBSTRN:
      case C_STRING:
         s = (STRING *) cp->ptr ;

#if FPE_TRAPS_ON                /* look for overflow error */
         errno = 0 ;
         cp->dval = strtod(s->str, (char **) 0) ;
         if (errno && cp->dval != 0.0)        /* ignore underflow */
            rt_error("overflow converting %s to double", s->str) ;
#else
         cp->dval = strtod(s->str, (char **) 0) ;
#endif
         free_STRING(s) ;
         break ;

      default:
         bozo("cast on bad type") ;
   }
   cp->type = C_DOUBLE ;

 two:cp++ ;

   switch (cp->type)
   {
      case C_NOINIT:
         cp->dval = 0.0 ;
         break ;

      case C_DOUBLE:
         return ;
      case C_STRNUM:
         free_STRING(string(cp)) ;
         break ;

      case C_MBSTRN:
      case C_STRING:
         s = (STRING *) cp->ptr ;

#if FPE_TRAPS_ON                /* look for overflow error */
         errno = 0 ;
         cp->dval = strtod(s->str, (char **) 0) ;
         if (errno && cp->dval != 0.0)        /* ignore underflow */
            rt_error("overflow converting %s to double", s->str) ;
#else
         cp->dval = strtod(s->str, (char **) 0) ;
#endif
         free_STRING(s) ;
         break ;

      default:
         bozo("cast on bad type") ;
   }
   cp->type = C_DOUBLE ;
}

void
cast1_to_s(cp)
   register CELL *cp ;
{
   register Int lval ;
   char xbuff[260] ;

   switch (cp->type)
   {
      case C_NOINIT:
         null_str.ref_cnt++ ;
         cp->ptr = (PTR) & null_str ;
         break ;

      case C_DOUBLE:

         lval = d_to_I(cp->dval) ;
         if (lval == cp->dval)        sprintf(xbuff, INT_FMT, lval) ;
         else  sprintf(xbuff, string(CONVFMT)->str, cp->dval) ;

         cp->ptr = (PTR) new_STRING(xbuff) ;
         break ;

      case C_STRING:
         return ;

      case C_MBSTRN:
      case C_STRNUM:
         break ;

      default:
         bozo("bad type on cast") ;
   }
   cp->type = C_STRING ;
}

void
cast2_to_s(cp)
   register CELL *cp ;
{
   register Int lval ;
   char xbuff[260] ;

   switch (cp->type)
   {
      case C_NOINIT:
         null_str.ref_cnt++ ;
         cp->ptr = (PTR) & null_str ;
         break ;

      case C_DOUBLE:

         lval = d_to_I(cp->dval) ;
         if (lval == cp->dval)        sprintf(xbuff, INT_FMT, lval) ;
         else  sprintf(xbuff, string(CONVFMT)->str, cp->dval) ;

         cp->ptr = (PTR) new_STRING(xbuff) ;
         break ;

      case C_STRING:
         goto two ;

      case C_MBSTRN:
      case C_STRNUM:
         break ;

      default:
         bozo("bad type on cast") ;
   }
   cp->type = C_STRING ;

two:
   cp++ ;

   switch (cp->type)
   {
      case C_NOINIT:
         null_str.ref_cnt++ ;
         cp->ptr = (PTR) & null_str ;
         break ;

      case C_DOUBLE:

         lval = d_to_I(cp->dval) ;
         if (lval == cp->dval)        sprintf(xbuff, INT_FMT, lval) ;
         else  sprintf(xbuff, string(CONVFMT)->str, cp->dval) ;

         cp->ptr = (PTR) new_STRING(xbuff) ;
         break ;

      case C_STRING:
         return ;

      case C_MBSTRN:
      case C_STRNUM:
         break ;

      default:
         bozo("bad type on cast") ;
   }
   cp->type = C_STRING ;
}

void
cast_to_RE(cp)
   register CELL *cp ;
{
   register PTR p ;
   /* char *s, *s1, *s2; */

   if (cp->type < C_STRING)  cast1_to_s(cp) ;

   /* preserve backslashes */
   /*
   for (s = string(cp)->str; s - string(cp)->str < string(cp)->len; s++)
   {
     if (*s == '\\' && *(s+1) == '\\')
     {
       s1 = s; s2 = s+1;
       for (; s2 - string(cp)->str < string(cp)->len; s1++, s2++)
         *s1 = *s2;
       *s1 = 0;
       string(cp)->len--;
     }
   }
   */
      
   p = re_compile(string(cp)) ;
   free_STRING(string(cp)) ;
   cp->type = C_RE ;
   cp->ptr = p ;
}

void
cast_for_split(cp)
   register CELL *cp ;
{
   static char meta[] = "^$.*+?|[]()" ;
   static char xbuff[] = "\\X" ;
   int c ;
   unsigned len ;

   if (cp->type < C_STRING)  cast1_to_s(cp) ;

   if ((len = string(cp)->len) == 1)
   {
      if ((c = string(cp)->str[0]) == ' ')
      {
         free_STRING(string(cp)) ;
         cp->type = C_SPACE ;
         return ;
      }
      else if (strchr(meta, c))
      {
         xbuff[1] = c ;
         free_STRING(string(cp)) ;
         cp->ptr = (PTR) new_STRING(xbuff) ;
      }
   }
   else if (len == 0)
   {
      free_STRING(string(cp)) ;
      cp->type = C_SNULL ;
      return ;
   }

   cast_to_RE(cp) ;
}

/* input: cp-> a CELL of type C_MBSTRN (maybe strnum)
   test it -- casting it to the appropriate type
   which is C_STRING or C_STRNUM
*/

void
check_strnum(cp)
   CELL *cp ;
{
   char *test ;
   register unsigned char *s, *q ;

   cp->type = C_STRING ;         /* assume not C_STRNUM */
   s = (unsigned char *) string(cp)->str ;
   q = s + string(cp)->len ;
   while (scan_code[*s] == SC_SPACE)  s++ ;
   if (s == q)        return ;

   while (scan_code[q[-1]] == SC_SPACE)         q-- ;
   if (scan_code[q[-1]] != SC_DIGIT &&
       q[-1] != '.')
      return ;

   switch (scan_code[*s])
   {
      case SC_DIGIT:
      case SC_PLUS:
      case SC_MINUS:
      case SC_DOT:

#if FPE_TRAPS_ON
         errno = 0 ;
         cp->dval = strtod((char *) s, &test) ;
         /* make overflow pure string */
         if (errno && cp->dval != 0.0)        return ;
#else
         cp->dval = strtod((char *) s, &test) ;
#endif

         if ((char *) q <= test)  cp->type = C_STRNUM ;
         /*  <= instead of == , for some buggy strtod
                 e.g. Apple Unix */
   }
}

/* cast a CELL to a replacement cell */

void
cast_to_REPL(cp)
   register CELL *cp ;
{
   register STRING *sval ;

   if (cp->type < C_STRING)  cast1_to_s(cp) ;
   sval = (STRING *) cp->ptr ;

   cellcpy(cp, repl_compile(sval)) ;
   free_STRING(sval) ;
}


/* convert a double to Int (this is not as simple as a
   cast because the results are undefined if it won't fit).
   Truncate large values to +Max_Int or -Max_Int
   Send nans to -Max_Int
*/

Int
d_to_I(d)
   double d;
{
   if (d >= Max_Int)        return Max_Int ;
   if (d > -Max_Int)        return (Int) d ;
   return -Max_Int ;
}

/* does not assume target was a cell, if so
   then caller should have made a previous
   call to cell_destroy  */

CELL *
cellcpy(target, source)
   register CELL *target, *source ;
{
   switch (target->type = source->type)
   {
      case C_NOINIT:
      case C_SPACE:
      case C_SNULL:
         break ;

      case C_DOUBLE:
         target->dval = source->dval ;
         break ;

      case C_STRNUM:
         target->dval = source->dval ;
         /* fall thru */

      case C_REPL:
      case C_MBSTRN:
      case C_STRING:
         string(source)->ref_cnt++ ;
         /* fall thru */

      case C_RE:
         target->ptr = source->ptr ;
         break ;

      case C_REPLV:
         replv_cpy(target, source) ;
         break ;

      default:
         bozo("bad cell passed to cellcpy()") ;
         break ;
   }
   return target ;
}

