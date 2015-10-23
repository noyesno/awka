
/********************************************
types.h
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/


/* $Log: types.h,v $
 * Revision 1.3  1993/07/15  23:56:18  mike
 * general cleanup
 *
 * Revision 1.2  1993/07/04  12:52:15  mike
 * start on autoconfig changes
 *
 * Revision 5.1  1991/12/05  07:59:39  brennan
 * 1.1 pre-release
 *
*/


/*  types.h  */

#ifndef  MAWK_TYPES_H
#define  MAWK_TYPES_H

#include  "sizes.h"


/*  CELL  types  */

#define  C_NOINIT                0
#define  C_DOUBLE                1
#define  C_STRING                2
#define  C_STRNUM                3
#define  C_MBSTRN                4 
        /*could be STRNUM, has not been checked */
#define  C_RE                    5
#define  C_SPACE                 6
        /* split on space */
#define  C_SNULL                 7
        /* split on the empty string  */
#define  C_REPL                  8
        /* a replacement string   '\&' changed to &  */
#define  C_REPLV                 9
        /* a vector replacement -- broken on &  */
#define  NUM_CELL_TYPES         10

/* these defines are used to check types for two
   CELLs which are adjacent in memory */

#define  TWO_NOINITS  (2*(1<<C_NOINIT))
#define  TWO_DOUBLES  (2*(1<<C_DOUBLE))
#define  TWO_STRINGS  (2*(1<<C_STRING))
#define  TWO_STRNUMS  (2*(1<<C_STRNUM))
#define  TWO_MBSTRNS  (2*(1<<C_MBSTRN))
#define  NOINIT_AND_DOUBLE  ((1<<C_NOINIT)+(1<<C_DOUBLE))
#define  NOINIT_AND_STRING  ((1<<C_NOINIT)+(1<<C_STRING))
#define  NOINIT_AND_STRNUM  ((1<<C_NOINIT)+(1<<C_STRNUM))
#define  DOUBLE_AND_STRING  ((1<<C_DOUBLE)+(1<<C_STRING))
#define  DOUBLE_AND_STRNUM  ((1<<C_STRNUM)+(1<<C_DOUBLE))
#define  STRING_AND_STRNUM  ((1<<C_STRING)+(1<<C_STRNUM))
#define  NOINIT_AND_MBSTRN  ((1<<C_NOINIT)+(1<<C_MBSTRN))
#define  DOUBLE_AND_MBSTRN  ((1<<C_DOUBLE)+(1<<C_MBSTRN))
#define  STRING_AND_MBSTRN  ((1<<C_STRING)+(1<<C_MBSTRN))
#define  STRNUM_AND_MBSTRN  ((1<<C_STRNUM)+(1<<C_MBSTRN))

typedef  struct {
unsigned len ;
unsigned short ref_cnt ;
char str[2] ;
} STRING ;

/* number of bytes more than the characters to store a
   string */
#define  STRING_OH   (sizeof(STRING)-1)


typedef  struct cell {
short type ;
short vcnt ; /* only used if type == C_REPLV   */
PTR   ptr ;
double  dval ;
}  CELL ;


/* all builtins are passed the evaluation stack pointer and
   return its new value, here is the type */

#ifndef  NO_PROTOS
typedef CELL *(*PF_CP)(CELL *) ;
#else
typedef CELL *(*PF_CP)() ;
#endif

/* an element of code (instruction) */
typedef  struct {  /* yeah, yeah, a kludge */
char *file ;
int lineno ;
int  op ;
PTR  ptr ;
}  INST ;

#endif  /* MAWK_TYPES_H */
