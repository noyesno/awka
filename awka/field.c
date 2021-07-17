/*------------------------------------------------------------*
 | field.c                                                    |
 | copyright 1991,  Michael D. Brennan                        |
 |                                                            |
 | This is a source file for the awka package, a translator   |
 | of the AWK programming language to ANSI C.                 |
 |                                                            |
 | The file is a borrowed version of field.c from             |
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

/* field.c */

#include "awka.h"
#include "field.h"
#include "init.h"
#include "memory.h"
#include "scan.h"
#include "bi_vars.h"
#include "repl.h"

CELL field[FBANK_SZ + NUM_PFIELDS] ;

CELL *fbank[NUM_FBANK] =
{field} ;

static int max_field = MAX_SPLIT ;         /* maximum field actually created*/

static void PROTO(build_field0, (void)) ;
static void PROTO(load_pfield, (char *, CELL *)) ;



/* a description of how to split based on RS.
   If RS is changed, so is rs_shadow */
SEPARATOR rs_shadow =
{SEP_CHAR, '\n'} ;
/* a splitting CELL version of FS */
CELL fs_shadow =
{C_SPACE} ;
int nf ;
 /* nf holds the true value of NF.  If nf < 0 , then
     NF has not been computed, i.e., $0 has not been split
  */


static void
load_pfield(name, cp)
   char *name ;
   CELL *cp ;
{
   SYMTAB *stp ;

   stp = insert(name) ; stp->type = ST_FIELD ;
   stp->stval.cp = cp ;
}

/* initialize $0 and the pseudo fields */
void
field_init()
{
   field[0].type = C_STRING ;
   field[0].ptr = (PTR) & null_str ;
   null_str.ref_cnt++ ;
   return;
}


void
split_field0()
{
}

/* construct field[0] from the other fields */

static void
build_field0()
{


#ifdef DEBUG
   if (nf < 0)        bozo("nf <0 in build_field0") ;
#endif

   cell_destroy(field + 0) ;

   if (nf == 0)
   {
      field[0].type = C_STRING ;
      field[0].ptr = (PTR) & null_str ;
      null_str.ref_cnt++ ;
   }
   else if (nf == 1)
   {
      cellcpy(field, field + 1) ;
   }
   else
   {
      CELL c ;
      STRING *ofs, *tail ;
      unsigned len ;
      register CELL *cp ;
      register char *p, *q ;
      int cnt ;
      CELL **fbp, *cp_limit ;


      cast1_to_s(cellcpy(&c, OFS)) ;
      ofs = (STRING *) c.ptr ;
      cast1_to_s(cellcpy(&c, field_ptr(nf))) ;
      tail = (STRING *) c.ptr ;
      cnt = nf - 1 ;

      len = cnt * ofs->len + tail->len ;

      fbp = fbank ; cp_limit = field + FBANK_SZ ;
      cp = field + 1 ;

      while (cnt-- > 0)
      {
         if (cp->type < C_STRING)
         {                        /* use the string field temporarily */
            if (cp->type == C_NOINIT)
            {
               cp->ptr = (PTR) & null_str ;
               null_str.ref_cnt++ ;
            }
            else  /* its a double */
            {
               Int ival ;
               char xbuff[260] ;

               ival = d_to_I(cp->dval) ;
               if (ival == cp->dval)  sprintf(xbuff, INT_FMT, ival) ;
               else  sprintf(xbuff, string(CONVFMT)->str, cp->dval) ;

               cp->ptr = (PTR) new_STRING(xbuff) ;
            }
         }

         len += string(cp)->len ;

         if (++cp == cp_limit)
         {
            cp = *++fbp ;
            cp_limit = cp + FBANK_SZ ;
         }

      }

      field[0].type = C_STRING ;
      field[0].ptr = (PTR) new_STRING0(len) ;

      p = string(field)->str ;

      /* walk it again , putting things together */
      cnt = nf-1 ; fbp = fbank ;
      cp = field+1 ; cp_limit = field + FBANK_SZ ;
      while (cnt-- > 0)
      {
         memcpy(p, string(cp)->str, string(cp)->len) ;
         p += string(cp)->len ;
         /* if not really string, free temp use of ptr */
         if (cp->type < C_STRING)  free_STRING(string(cp)) ;
         if (++cp == cp_limit)
         {
            cp = *++fbp ;
            cp_limit = cp + FBANK_SZ ;
         }
         /* add the separator */
         q = ofs->str ;
	 while ( *q )
            *p++ = *q++ ;
      }
      /* tack tail on the end */
      memcpy(p, tail->str, tail->len) ;

      /* cleanup */
      free_STRING(tail) ; free_STRING(ofs) ;
   }
}

int
field_addr_to_index(cp)
   CELL *cp ;
{
   CELL **p = fbank ;

   while (
            cp < *p || cp >= *p + FBANK_SZ)
      p++ ;

   return ((p - fbank) << FB_SHIFT) + (cp - *p) ;
}

/*------- more than 1 fbank needed  ------------*/

/*
  compute the address of a field with index
  > MAX_SPLIT
*/

CELL *
slow_field_ptr(i)
   register int i ;
{

   if (i > max_field)
   {
      int j ;

      if (i > MAX_FIELD)
         rt_overflow("maximum number of fields", MAX_FIELD) ;

      j = 1 ;
      while (fbank[j])        j++ ;

      do
      {
         fbank[j] = (CELL *) zmalloc(sizeof(CELL) * FBANK_SZ) ;
         memset(fbank[j], 0, sizeof(CELL) * FBANK_SZ) ;
         j++ ;
         max_field += FBANK_SZ ;
      }
      while (i > max_field);
   }

   return &fbank[i >> FB_SHIFT][i & (FBANK_SZ - 1)] ;
}


