/*------------------------------------------------------------*
 | jmp.c                                                      |
 | copyright 1991,  Michael D. Brennan                        |
 |                                                            |
 | This is a source file for the awka package, a translator   |
 | of the AWK programming language to ANSI C.                 |
 |                                                            |
 | The file is a borrowed version of jmp.c from               |
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

/* this module deals with back patching jumps, breaks and continues,
   and with save and restoring code when we move code.
   There are three stacks.  If we encounter a compile error, the
   stacks are frozen, i.e., we do not attempt error recovery
   on the stacks
*/


#include "awka.h"
#include "symtype.h"
#include "jmp.h"
#include "code.h"
#include "sizes.h"
#include "init.h"
#include "memory.h"

#define error_state  (compile_error_count>0)


/*---------- back patching jumps  ---------------*/

typedef struct jmp
{
   struct jmp *link ;
   int source_offset ;
}
JMP ;

static JMP *jmp_top ;

void
code_jmp(jtype, target)
   int jtype ;
   INST *target ;
{
   if (error_state)  return ;

   /* WARNING: Don't emit any code before using target or
     relocation might make it invalid */

   if (target)        code2op(jtype, target - (code_ptr + 1)) ;
   else
   {
      register JMP *p = ZMALLOC(JMP) ;

      /* stack for back patch */
      code2op(jtype, 0) ;
      p->source_offset = code_offset - 1 ;
      p->link = jmp_top ;
      jmp_top = p ;
   }
}

void
patch_jmp(target)                /* patch a jump on the jmp_stack */
   INST *target ;
{
   register JMP *p ;
   register INST *source ;         /* jmp starts here */

   if (!error_state)
   {
#ifdef        DEBUG
      if (!jmp_top)  bozo("jmp stack underflow") ;
#endif

      p = jmp_top ; jmp_top = p->link ;
      source = p->source_offset + code_base ;
      source->op = target - source ;

      ZFREE(p) ;
   }
}


/*-- break and continue -------*/

typedef struct bc
{
   struct bc *link ;                 /* stack as linked list */
   int type ;                         /* 'B' or 'C' or mark start with 0 */
   int source_offset ;                 /* position of _JMP  */
}
BC ;

static BC *bc_top ;



void
BC_new()                        /* mark the start of a loop */
{
   BC_insert(0, (INST *) 0) ;
}

void
BC_insert(type, address)
int type ; INST *address ;
{
   register BC *p ;

   if (error_state)  return ;

   if (type && !bc_top)
   {
      compile_error("%s statement outside of loop",
                    type == 'B' ? "break" : "continue") ;

      return ;
   }
   else
   {
      p = ZMALLOC(BC) ;
      p->type = type ;
      p->source_offset = address - code_base ;
      p->link = bc_top ;
      bc_top = p ;
   }
}


/* patch all break and continues for one loop */
void
BC_clear(B_address, C_address)
   INST *B_address, *C_address ;
{
   register BC *p, *q ;
   INST *source ;

   if (error_state)  return ;

   p = bc_top ;
   /* pop down to the mark node */
   while (p->type)
   {
      source = code_base + p->source_offset ;
      source->op = (p->type == 'B' ? B_address : C_address)
         - source ;

      q = p ; p = p->link ; ZFREE(q) ;
   }
   /* remove the mark node */
   bc_top = p->link ;
   ZFREE(p) ;
}

/*-----         moving code --------------------------*/

/* a stack to hold some pieces of code while
   reorganizing loops .
*/

typedef struct mc
{                                /* mc -- move code */
   struct mc *link ;
   INST *code ;                         /* the save code */
   unsigned len ;                 /* its length */
   int scope ;                         /* its scope */
   int move_level ;              /* size of this stack when coded */
   FBLOCK *fbp ;                 /* if scope FUNCT */
   int offset ;                         /* distance from its code base */
}
MC ;

static MC *mc_top ;
int code_move_level = 0 ; /* see comment in jmp.h */

#define         NO_SCOPE        -1
 /* means relocation of resolve list not needed */

void
code_push(code, len, scope, fbp)
   INST *code ;
   unsigned len ;
   int scope ;
   FBLOCK *fbp ;
{
   register MC *p ;

   if (!error_state)
   {
      p = ZMALLOC(MC) ;
      p->len = len ;
      p->link = mc_top ;
      mc_top = p ;

      if (len)
      {
         p->code = (INST *) zmalloc(sizeof(INST) * len) ;
         memcpy(p->code, code, sizeof(INST) * len) ;
      }
      if (!resolve_list)  p->scope = NO_SCOPE ;
      else
      {
         p->scope = scope ;
         p->move_level = code_move_level ;
         p->fbp = fbp ;
         p->offset = code - code_base ;
      }
   }
   code_move_level++ ;
}

/* copy the code at the top of the mc stack to target.
   return the number of INSTs moved */

unsigned
code_pop(target)
   INST *target ;
{
   register MC *p ;
   unsigned len ;
   int target_offset ;

   if (error_state)  return 0 ;

#ifdef        DEBUG
   if (!mc_top)         bozo("code_pop: memory copy underflow") ;
#endif

   p = mc_top ; mc_top = p->link ;
   len = p->len ;

   while (target + len >= code_warn)
   {
      target_offset = target - code_base ;
      code_grow() ;
      target = code_base + target_offset ;
   }

   if (len)
   {
      memcpy(target, p->code, len * sizeof(INST)) ;
      zfree(p->code, len * sizeof(INST)) ;
   }

   if (p->scope != NO_SCOPE)
   {
      target_offset = target - code_base ;
      relocate_resolve_list(p->scope, p->move_level, p->fbp,
                         p->offset, len, target_offset - p->offset) ;
   }

   ZFREE(p) ;
   code_move_level-- ;
   return len ;
}
