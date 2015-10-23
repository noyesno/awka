
/********************************************
jmp.h
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/* $Log: jmp.h,v $
 * Revision 1.2  1995/04/21  14:20:19  mike
 * move_level variable to fix bug in arglist patching of moved code.
 *
 * Revision 1.1.1.1  1993/07/03  18:58:15  mike
 * move source to cvs
 *
 * Revision 5.2  1993/01/09  19:03:44  mike
 * code_pop checks if the resolve_list needs relocation
 *
 * Revision 5.1  1991/12/05  07:59:24  brennan
 * 1.1 pre-release
 *
*/

#ifndef   JMP_H
#define   JMP_H

void  PROTO(BC_new, (void) ) ;
void  PROTO(BC_insert, (int, INST*) ) ;
void  PROTO(BC_clear, (INST *, INST *) ) ;
void  PROTO(code_push, (INST *, unsigned, int, FBLOCK*) ) ;
unsigned  PROTO(code_pop, (INST *) ) ;
void  PROTO(code_jmp, (int, INST *) ) ;
void  PROTO(patch_jmp, (INST *) ) ;

extern int code_move_level ;
   /* used to as one part of unique identification of context when
      moving code.  Global for communication with parser.
   */

#endif  /* JMP_H  */

