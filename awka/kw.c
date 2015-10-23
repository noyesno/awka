/*------------------------------------------------------------*
 | kw.c                                                       |
 | copyright 1991,  Michael D. Brennan                        |
 |                                                            |
 | This is a source file for the awka package, a translator   |
 | of the AWK programming language to ANSI C.                 |
 |                                                            |
 | The file is a borrowed version of kw.c from                |
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

/* kw.c */


#include "awka.h"
#include "symtype.h"
#include "parse.h"
#include "init.h"


static struct kw
{
   char *text ;
   short kw ;
}
keywords[] =
{

   "print", PRINT,
   "printf", PRINTF,
   "do", DO,
   "while", WHILE,
   "for", FOR,
   "break", BREAK,
   "continue", CONTINUE,
   "if", IF,
   "else", ELSE,
   "in", IN,
   "delete", DELETE,
   "split", SPLIT,
   "match", MATCH_FUNC, 
   "BEGIN", a_BEGIN,
   "END", a_END,
   "exit", EXIT,
   "abort", ABORT,
   "next", NEXT,
   "nextfile", NEXTFILE,
   "return", RETURN,
   "getline", GETLINE,
   "sub", SUB,
   "gsub", GSUB,
   "gensub", GENSUB,
   "func", FUNCTION,
   "function", FUNCTION,
   "alength", ALENGTH_FUNC,
   "asort", ASORT_FUNC,
   (char *) 0, 0
} ;

/* put keywords in the symbol table */
void
kw_init()
{
   register struct kw *p = keywords ;
   register SYMTAB *q ;

   while (p->text)
   {
      q = insert(p->text) ;
      q->type = ST_KEYWORD ;
      q->stval.kw = p++->kw ;
   }
}

/* find a keyword to emit an error message */
char *
find_kw_str(kw_token)
   int kw_token ;
{
   struct kw *p ;

   for (p = keywords; p->text; p++)
      if (p->kw == kw_token)  
        return p->text ;
   /* search failed */
   return (char *) 0 ;
}
