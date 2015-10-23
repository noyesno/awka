/*------------------------------------------------------------*
 | memory.c                                                   |
 | copyright 1991,  Michael D. Brennan                        |
 |                                                            |
 | This is a source file for the awka package, a translator   |
 | of the AWK programming language to ANSI C.                 |
 |                                                            |
 | The file is a borrowed version of memory.c from            |
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

/* memory.c */

#include "awka.h"
#include "memory.h"

static STRING *PROTO(xnew_STRING, (unsigned)) ;


STRING null_str =
{0, 1, ""} ;

static STRING *
xnew_STRING(len)
   unsigned len ;
{
   STRING *sval = (STRING *) zmalloc(len + STRING_OH) ;

   sval->len = len ;
   sval->ref_cnt = 1 ;
   return sval ;
}

/* allocate space for a STRING */

STRING *
new_STRING0(len)
   unsigned len ;
{
   if (len == 0)
   {
      null_str.ref_cnt++ ;
      return &null_str ;
   }
   else
   {
      STRING *sval = xnew_STRING(len) ;
      sval->str[len] = 0 ;
      return sval ;
   }
}

/* convert char* to STRING* */

STRING *
new_STRING(s)
   char *s ;
{

   if (s[0] == 0)
   {
      null_str.ref_cnt++ ;
      return &null_str ;
   }
   else
   {
      STRING *sval = xnew_STRING(strlen(s)) ;
      strcpy(sval->str, s) ;
      return sval ;
   }
}


#ifdef         DEBUG

void
DB_free_STRING(sval)
   register STRING *sval ;
{
   if (--sval->ref_cnt == 0)  zfree(sval, sval->len + STRING_OH) ;
}

#endif
