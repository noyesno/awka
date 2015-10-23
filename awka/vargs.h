
/********************************************
vargs.h
copyright 1992 Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
$Log: vargs.h,v $
 * Revision 1.4  1994/12/14  14:36:54  mike
 * sometimes stdarg.h exists, but depending on compiler flags it is
 * unusable -- assume NO_PROTOS => NO_STDARG_H
 *
 * Revision 1.3  1994/10/08  19:18:38  mike
 * trivial change
 *
 * Revision 1.2  1993/07/04  12:52:19  mike
 * start on autoconfig changes
 *
 * Revision 1.1.1.1  1993/07/03  18:58:22  mike
 * move source to cvs
 *
 * Revision 1.1  1992/10/02  23:23:41  mike
 * Initial revision
 *
*/

/* provides common interface to <stdarg.h> or <varargs.h> 
   only used for error messages
*/

#ifdef     NO_PROTOS
#ifndef    NO_STDARG_H   
#define    NO_STDARG_H  1
#endif
#endif

#if     NO_STDARG_H
#include <varargs.h>

#ifndef  VA_ALIST

#define  VA_ALIST(type, arg)  (va_alist) va_dcl { type arg ;
#define  VA_ALIST2(t1,a1,t2,a2) (va_alist) va_dcl { t1 a1 ; t2 a2 ;

#endif

#define  VA_START(p,type, last)  va_start(p) ;\
                                 last = va_arg(p,type)


#define  VA_START2(p,t1,a1,t2,a2)  va_start(p) ;\
                                  a1 = va_arg(p,t1);\
                                  a2 = va_arg(p,t2)

#else  /* have stdarg.h */
#include <stdarg.h>

#ifndef  VA_ALIST
#define  VA_ALIST(type, arg)  (type arg, ...) {
#define  VA_ALIST2(t1,a1,t2,a2)  (t1 a1,t2 a2,...) {
#endif

#define  VA_START(p,type,last)   va_start(p,last)

#define  VA_START2(p,t1,a1,t2,a2)  va_start(p,a2)

#endif

