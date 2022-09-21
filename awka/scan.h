
/********************************************
scan.h
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/


/* $Log: scan.h,v $
 * Revision 1.3  1995/06/18  19:42:26  mike
 * Remove some redundant declarations and add some prototypes
 *
 * Revision 1.2  1994/09/23  00:20:06  mike
 * minor bug fix: handle \ in eat_nl()
 *
 * Revision 1.1.1.1  1993/07/03  18:58:20  mike
 * move source to cvs
 *
 * Revision 5.1  1991/12/05  07:59:33  brennan
 * 1.1 pre-release
 *
*/


/* scan.h  */

#ifndef  SCAN_H_INCLUDED
#define  SCAN_H_INCLUDED   1

#include <stdio.h>

#ifndef   MAKESCAN
#include  "symtype.h"
#include  "parse.h"
#endif

extern  char scan_code[256] ;

/*  the scan codes to compactify the main switch */

#define  SC_SPACE               1
#define  SC_NL                  2
#define  SC_SEMI_COLON          3
#define  SC_FAKE_SEMI_COLON     4
#define  SC_LBRACE              5
#define  SC_RBRACE              6
#define  SC_QMARK               7
#define  SC_COLON               8
#define  SC_OR                  9
#define  SC_AND                10
#define  SC_PLUS               11
#define  SC_MINUS              12
#define  SC_MUL                13
#define  SC_DIV                14
#define  SC_MOD                15
#define  SC_POW                16
#define  SC_LPAREN             17
#define  SC_RPAREN             18
#define  SC_LBOX               19
#define  SC_RBOX               20
#define  SC_IDCHAR             21
#define  SC_DIGIT              22
#define  SC_DQUOTE             23
#define  SC_ESCAPE             24
#define  SC_COMMENT            25
#define  SC_EQUAL              26
#define  SC_NOT                27
#define  SC_LT                 28
#define  SC_GT                 29
#define  SC_COMMA              30
#define  SC_DOT                31
#define  SC_MATCH              32
#define  SC_DOLLAR             33
#define  SC_UNEXPECTED         34

#ifndef  MAKESCAN

void  PROTO(eat_nl, (void) ) ;

/* in error.c */
void  PROTO( unexpected_char, (void) ) ;

#define  ct_ret(x)  return current_token = (x)

#define  test1_ret(c,x,d)  if ( next() == (c) ) ct_ret(x) ;\
                           else { un_next() ; ct_ret(d) ; }

#define  test2_ret(c1,x1,c2,x2,d)   switch ( next() )\
                                   { case c1: ct_ret(x1) ;\
                                     case c2: ct_ret(x2) ;\
                                     default: un_next() ;\
                                              ct_ret(d) ; }
#endif  /* ! MAKESCAN  */
#endif
