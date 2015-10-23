/*------------------------------------------------------------*
 | code.c                                                     |
 | copyright 1999,  Andrew Sumner                             |
 | copyright 1991,  Michael D. Brennan                        |
 |                                                            |
 | This is a source file for the awka package, a translator   |
 | of the AWK programming language to ANSI C.                 |
 |                                                            |
 | The file is a modified version of code.c from Mawk,        |
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

/*  code.c  */

#include "awka.h"
#include "code.h"
#include "init.h"
#include "jmp.h"
#include "field.h"

static CODEBLOCK *PROTO(new_code, (void)) ;

CODEBLOCK active_code ;

CODEBLOCK *main_code_p, *begin_code_p, *end_code_p ;

INST *begin_start, *main_start, *end_start, *next_label ;
unsigned begin_size, main_size ;
extern int awka_main;

INST *execution_start = 0 ;

void awka_insertop(int, char *, char *, int, int);

/* grow the active code */
void
code_grow()
{
   unsigned oldsize = code_limit - code_base ;
   unsigned newsize = PAGESZ + oldsize ;
   unsigned delta = code_ptr - code_base ;

   if (code_ptr > code_limit)  bozo("CODEWARN is too small") ;

   code_base = (INST *)
      zrealloc(code_base, INST_BYTES(oldsize),
               INST_BYTES(newsize)) ;
   code_limit = code_base + newsize ;
   code_warn = code_limit - CODEWARN ;
   code_ptr = code_base + delta ;
}

/* shrinks executable code that's done to its final size */
INST *
code_shrink(p, sizep)
   CODEBLOCK *p ;
   unsigned *sizep ;
{

   unsigned oldsize = INST_BYTES(p->limit - p->base) ;
   unsigned newsize = INST_BYTES(p->ptr - p->base) ;
   INST *retval ;

   *sizep = newsize ;

   retval = (INST *) zrealloc(p->base, oldsize, newsize) ;
   ZFREE(p) ;
   return retval ;
}


/* code an op and a pointer in the active_code */
void
xcode2(op, ptr)
   int op ;
   PTR ptr ;
{
   register INST *p = code_ptr + 2 ;

   if (p >= code_warn)
   {
      code_grow() ;
      p = code_ptr + 2 ;
   }

   p[-2].op = op ;
   p[-1].ptr = ptr ;
   p[-2].lineno = p[-1].lineno = token_lineno ;
   p[-2].file = p[-1].file = pfile_name ;
   code_ptr = p ;
}

/* code two ops in the active_code */
void
code2op(x, y)
   int x, y ;
{
   register INST *p = code_ptr + 2 ;

   if (p >= code_warn)
   {
      code_grow() ;
      p = code_ptr + 2 ;
   }

   p[-2].op = x ;
   p[-1].op = y ;
   p[-2].lineno = p[-1].lineno = token_lineno ;
   p[-2].file = p[-1].file = pfile_name ;
   code_ptr = p ;
}

void
code_init()
{
   main_code_p = new_code() ;

   active_code = *main_code_p ;
   code1(_OMAIN) ;
}

/* final code relocation
   set_code() as in set concrete */
void
set_code()
{
   /* set the main code which is active_code */
   if (end_code_p || code_offset > 1)
   {
      int gl_offset = code_offset ;
      extern int NR_flag ;

      if (NR_flag)  code2op(OL_GL_NR, _HALT) ;
      else  code2op(OL_GL, _HALT) ;

      *main_code_p = active_code ;
      main_start = code_shrink(main_code_p, &main_size) ;
      next_label = main_start + gl_offset ;
      execution_start = main_start ;
   }
   else         /* only BEGIN */
   {
      zfree(code_base, INST_BYTES(PAGESZ)) ;
      ZFREE(main_code_p) ;
   }

   /* set the END code */
   if (end_code_p)
   {
      unsigned dummy ;

      active_code = *end_code_p ;
      if (awka_main)
        code2op(_CLEANUP, _HALT) ;
      else
        code2op(_EXIT0, _HALT) ;
      *end_code_p = active_code ;
      end_start = code_shrink(end_code_p, &dummy) ;
   }

   /* set the BEGIN code */
   if (begin_code_p)
   {
      active_code = *begin_code_p ;
      if (main_start)  code2op(_JMAIN, _HALT) ;
      else
      {
         if (awka_main)
            code2op(_CLEANUP, _HALT) ;
         else
            code2op(_EXIT0, _HALT) ;
      }
      *begin_code_p = active_code ;
      begin_start = code_shrink(begin_code_p, &begin_size) ;

      execution_start = begin_start ;
   }

   if ( ! execution_start )
   {
      /* program had functions but no pattern-action bodies */
      execution_start = begin_start = (INST*) zmalloc(2*sizeof(INST)) ;
      if (!awka_main)
        execution_start[0].op = _EXIT0 ;
      else
        execution_start[0].op = _CLEANUP ;
      execution_start[1].op = _HALT  ;
   }
}

void
dump_code()
{
   if (begin_start)  
   { awka_insertop(_BEGIN, NULL, NULL, 0, 0);
     if (dump) fprintf(stderr,"BEGIN\n");
     da(begin_start, stdout) ; 
     if (dump) fprintf(stderr,"\n");
   }
   if (end_start)  
   { awka_insertop(_END, NULL, NULL, 0, 0);
     if (dump) fprintf(stderr,"END\n");
     da(end_start, stdout) ; 
     if (dump) fprintf(stderr,"\n");
   }
   if (main_start)  
   { awka_insertop(_MAIN, NULL, NULL, 0, 0);
     if (dump) fprintf(stderr,"MAIN\n");
     da(main_start, stdout) ;
     if (dump) fprintf(stderr,"\n");
   }
   fdump() ;                         /* dumps all user functions */
   if (dump) exit(0);
}


static CODEBLOCK *
new_code()
{
   CODEBLOCK *p = ZMALLOC(CODEBLOCK) ;

   p->base = (INST *) zmalloc(INST_BYTES(PAGESZ)) ;
   p->limit = p->base + PAGESZ ;
   p->warn = p->limit - CODEWARN ;
   p->ptr = p->base ;

   return p ;
}

/* moves the active_code from MAIN to a BEGIN or END */

void
be_setup(scope)
   int scope ;
{
   *main_code_p = active_code ;

   if (scope == SCOPE_BEGIN)
   {
      if (!begin_code_p)  begin_code_p = new_code() ;
      active_code = *begin_code_p ;
   }
   else
   {
      if (!end_code_p)        end_code_p = new_code() ;
      active_code = *end_code_p ;
   }
}
