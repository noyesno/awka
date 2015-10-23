
/********************************************
memory.h
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/


/* $Log: memory.h,v $
 * Revision 1.1.1.1  1993/07/03  18:58:17  mike
 * move source to cvs
 *
 * Revision 5.2  1993/01/01  21:30:48  mike
 * split new_STRING() into new_STRING and new_STRING0
 *
 * Revision 5.1  1991/12/05  07:59:28  brennan
 * 1.1 pre-release
 *
*/


/*  memory.h  */

#ifndef  MEMORY_H
#define  MEMORY_H

#include "zmalloc.h"


STRING *PROTO(new_STRING, (char*)) ;
STRING *PROTO(new_STRING0, (unsigned)) ;

#ifdef   DEBUG
void  PROTO( DB_free_STRING , (STRING *) ) ;

#define  free_STRING(s)  DB_free_STRING(s)

#else

#define  free_STRING(sval)   if ( --(sval)->ref_cnt == 0 ) \
                                zfree(sval, (sval)->len+STRING_OH) ; else
#endif


#endif   /* MEMORY_H */
