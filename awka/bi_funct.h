
/********************************************
bi_funct.h
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/


/* $Log: bi_funct.h,v $
 * Revision 1.2  1994/12/11  22:10:15  mike
 * fflush
 *
 * Revision 1.1.1.1  1993/07/03  18:58:08  mike
 * move source to cvs
 *
 * Revision 5.1  1991/12/05  07:59:03  brennan
 * 1.1 pre-release
 *
*/

#ifndef  BI_FUNCT_H
#define  BI_FUNCT_H  1

#include "symtype.h"

extern BI_REC  bi_funct[] ;

void PROTO(bi_init, (void) ) ;

/* builtin string functions */
CELL *PROTO( bi_print, (CELL *) ) ;
CELL *PROTO( bi_printf, (CELL *) ) ;
CELL *PROTO( bi_length, (CELL *) ) ;
CELL *PROTO( bi_index, (CELL *) ) ;
CELL *PROTO( bi_substr, (CELL *) ) ;
CELL *PROTO( bi_sprintf, (CELL *) ) ;
CELL *PROTO( bi_split, (CELL *) ) ;
CELL *PROTO( bi_match, (CELL *) ) ;
CELL *PROTO( bi_getline, (CELL *) ) ;
CELL *PROTO( bi_sub, (CELL *) ) ;
CELL *PROTO( bi_gsub, (CELL *) ) ;
CELL *PROTO( bi_gensub, (CELL *) ) ;
CELL *PROTO( bi_toupper, (CELL*) ) ;
CELL *PROTO( bi_tolower, (CELL*) ) ;

/* builtin arith functions */
CELL *PROTO( bi_sin, (CELL *) ) ;
CELL *PROTO( bi_cos, (CELL *) ) ;
CELL *PROTO( bi_atan2, (CELL *) ) ;
CELL *PROTO( bi_log, (CELL *) ) ;
CELL *PROTO( bi_exp, (CELL *) ) ;
CELL *PROTO( bi_int, (CELL *) ) ;
CELL *PROTO( bi_sqrt, (CELL *) ) ;
CELL *PROTO( bi_srand, (CELL *) ) ;
CELL *PROTO( bi_rand, (CELL *) ) ;
/* extra math functions */
CELL *PROTO( bi_tan, (CELL *) ) ;
CELL *PROTO( bi_cosh, (CELL *) ) ;
CELL *PROTO( bi_sinh, (CELL *) ) ;
CELL *PROTO( bi_tanh, (CELL *) ) ;
CELL *PROTO( bi_acos, (CELL *) ) ;
CELL *PROTO( bi_asin, (CELL *) ) ;
CELL *PROTO( bi_atan, (CELL *) ) ;
CELL *PROTO( bi_atan2, (CELL *) ) ;
CELL *PROTO( bi_acosh, (CELL *) ) ;
CELL *PROTO( bi_asinh, (CELL *) ) ;
CELL *PROTO( bi_atanh, (CELL *) ) ;
CELL *PROTO( bi_hypot, (CELL *) ) ;
CELL *PROTO( bi_log10, (CELL *) ) ;
CELL *PROTO( bi_log2, (CELL *) ) ;
CELL *PROTO( bi_exp2, (CELL *) ) ;
CELL *PROTO( bi_pow, (CELL *) ) ;
CELL *PROTO( bi_ceil, (CELL *) ) ;
CELL *PROTO( bi_floor, (CELL *) ) ;
CELL *PROTO( bi_round, (CELL *) ) ;
CELL *PROTO( bi_trunc, (CELL *) ) ;
CELL *PROTO( bi_abs, (CELL *) ) ;
CELL *PROTO( bi_mod, (CELL *) ) ;
CELL *PROTO( bi_erf, (CELL *) ) ;
CELL *PROTO( bi_erfc, (CELL *) ) ;
CELL *PROTO( bi_lgamma, (CELL *) ) ;
CELL *PROTO( bi_tgamma, (CELL *) ) ;

/* other builtins */
CELL *PROTO( bi_close, (CELL *) ) ;
CELL *PROTO( bi_system, (CELL *) ) ;
CELL *PROTO( bi_fflush, (CELL *) ) ;

/* more string functions */
CELL *PROTO( bi_trim, (CELL*) ) ;
CELL *PROTO( bi_ltrim, (CELL*) ) ;
CELL *PROTO( bi_rtrim, (CELL*) ) ;
CELL *PROTO( bi_left, (CELL*) ) ;
CELL *PROTO( bi_right, (CELL*) ) ;
CELL *PROTO( bi_totitle, (CELL*) ) ;
CELL *PROTO( bi_ascii, (CELL *) ) ;
CELL *PROTO( bi_char, (CELL *) ) ;

/* bit operations */
CELL *PROTO( bi_and, (CELL *) ) ;
CELL *PROTO( bi_or, (CELL *) ) ;
CELL *PROTO( bi_xor, (CELL *) ) ;
CELL *PROTO( bi_compl, (CELL *) ) ;
CELL *PROTO( bi_lshift, (CELL *) ) ;
CELL *PROTO( bi_rshift, (CELL *) ) ;

/* date functions */
CELL *PROTO( bi_time, (CELL *) ) ;
CELL *PROTO( bi_localtime, (CELL *) ) ;
CELL *PROTO( bi_gmtime, (CELL *) ) ;
CELL *PROTO( bi_mktime, (CELL *) ) ;
CELL *PROTO( bi_systime, (CELL *) ) ;
CELL *PROTO( bi_strftime, (CELL *) ) ;

/* useful for performance */
CELL *PROTO( bi_min, (CELL *) ) ;
CELL *PROTO( bi_max, (CELL *) ) ;

CELL *PROTO( bi_asort, (CELL *) ) ;
CELL *PROTO( bi_alength, (CELL *) ) ;
CELL *PROTO( bi_argcount, (CELL *) ) ;
CELL *PROTO( bi_argval,   (CELL *) ) ;

CELL *PROTO( bi_isarray, (CELL *) ) ;
CELL *PROTO( bi_typeof,  (CELL *) ) ;

char bi_funct_find(char *) ;

#endif  /* BI_FUNCT_H  */

