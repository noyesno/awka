/*------------------------------------------------------------*
 | zmalloc.c                                                  |
 | copyright 1991,  Michael D. Brennan                        |
 |                                                            |
 | This is a source file for the awka package, a translator   |
 | of the AWK programming language to ANSI C.                 |
 |                                                            |
 | The file is a borrowed version of zmalloc.c from           |
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

/*  zmalloc.c  */
#include  "awka.h"
#include  "zmalloc.h"



/*
  zmalloc() gets mem from malloc() in CHUNKS of 2048 bytes
  and cuts these blocks into smaller pieces that are multiples
  of eight bytes.  When a piece is returned via zfree(), it goes
  on a linked linear list indexed by its size.        The lists are
  an array, pool[].

  E.g., if you ask for 22 bytes with p = zmalloc(22), you actually get
  a piece of size 24.  When you free it with zfree(p,22) , it is added
  to the list at pool[2].
*/

#define POOLSZ            16

#define         CHUNK                256
 /* number of blocks to get from malloc */

static void PROTO(out_of_mem, (void)) ;


static void
out_of_mem()
{
   static char out[] = "out of memory" ;

   /* I don't think this will ever happen */
   compile_error(out) ; exit(2) ; 
}


typedef union zblock
{
   char dummy[ZBLOCKSZ] ;
   union zblock *link ;
} ZBLOCK ;

/* ZBLOCKS of sizes 1, 2, ... 16
   which is bytes of sizes 8, 16, ... , 128
   are stored on the linked linear lists in
   pool[0], pool[1], ... , pool[15]
*/

static ZBLOCK *pool[POOLSZ] ;

/* zmalloc() is a macro in front of bmalloc "BLOCK malloc" */

PTR
bmalloc(blocks)
   register unsigned blocks ;
{
   register ZBLOCK *p ;
   static unsigned amt_avail ;
   static ZBLOCK *avail ;

   if (blocks > POOLSZ)
   {
      p = (ZBLOCK *) malloc(blocks << ZSHIFT) ;
      if (!p)  out_of_mem() ;
      return (PTR) p ;
   }

   if (p = pool[blocks - 1])
   {
      pool[blocks - 1] = p->link ;
      return (PTR) p ;
   }

   if (blocks > amt_avail)
   {
      if (amt_avail != 0)        /* free avail */
      {
         avail->link = pool[--amt_avail] ;
         pool[amt_avail] = avail ;
      }

      if (!(avail = (ZBLOCK *) malloc(CHUNK * ZBLOCKSZ)))
      {
         /* if we get here, almost out of memory */
         amt_avail = 0 ;
         p = (ZBLOCK *) malloc(blocks << ZSHIFT) ;
         if (!p)  out_of_mem() ;
         return (PTR) p ;
      }
      else  amt_avail = CHUNK ;
   }

   /* get p from the avail pile */
   p = avail ; avail += blocks ; amt_avail -= blocks ; 
   return (PTR) p ;
}

void
bfree(p, blocks)
   register PTR p ;
   register unsigned blocks ;
{

   if (blocks > POOLSZ)         free(p) ;
   else
   {
      ((ZBLOCK *) p)->link = pool[--blocks] ;
      pool[blocks] = (ZBLOCK *) p ;
   }
}

PTR
zrealloc(p, old_size, new_size)
   register PTR p ;
   unsigned old_size, new_size ;
{
   register PTR q ;

   if (new_size > (POOLSZ << ZSHIFT) &&
       old_size > (POOLSZ << ZSHIFT))
   {
      if (!(q = realloc(p, new_size)))        out_of_mem() ;
   }
   else
   {
      q = zmalloc(new_size) ;
      memcpy(q, p, old_size < new_size ? old_size : new_size) ;
      zfree(p, old_size) ;
   }
   return q ;
}



#ifndef         __GNUC__
/* pacifier for Bison , this is really dead code */
PTR
alloca(sz)
   unsigned sz ;
{
   /* hell just froze over */
   exit(100) ;
   return (PTR) 0 ;
}
#endif
