
/********************************************
sizes.h
copyright 1991, 1992.  Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/* $Log: sizes.h,v $
 * Revision 1.8  1995/10/14  22:09:51  mike
 * getting MAX__INT from values.h didn't really work since the value was
 * unusable in an #if MAX__INT <= 0x7fff
 * at least it didn't work under sunos -- so use of values.h is a goner
 *
 * Revision 1.7  1995/06/18  19:17:51  mike
 * Create a type Int which on most machines is an int, but on machines
 * with 16bit ints, i.e., the PC is a long.  This fixes implicit assumption
 * that int==long.
 *
 * Revision 1.6  1994/10/10  01:39:01  mike
 * get MAX__INT from limits.h or values.h
 *
 * Revision 1.5  1994/10/08  19:15:53  mike
 * remove SM_DOS
 *
 * Revision 1.4  1994/09/25  23:00:49  mike
 * remove #if 0
 *
 * Revision 1.3  1993/07/15  23:56:15  mike
 * general cleanup
 *
 * Revision 1.2  1993/07/04  12:52:13  mike
 * start on autoconfig changes
 *
 * Revision 5.3  1992/12/17  02:48:01  mike
 * 1.1.2d changes for DOS
 *
 * Revision 5.2  1992/08/27  03:20:08  mike
 * patch2: increase A_HASH_PRIME
 *
 * Revision 5.1  1991/12/05  07:59:35  brennan
 * 1.1 pre-release
 *
*/

/*  sizes.h  */

#ifndef  SIZES_H
#define  SIZES_H

#ifndef  MAX__INT
#include <limits.h>
#define  MAX__INT  INT_MAX
#define  MAX__LONG LONG_MAX
#endif   /* MAX__INT */

#if  MAX__INT <= 0x7fff
#define  SHORT_INTS
#define  INT_FMT "%ld"
typedef  long Int ;
#define  Max_Int MAX__LONG
#else
#define  INT_FMT "%d"
typedef  int Int ;
#define  Max_Int  MAX__INT
#endif

#define EVAL_STACK_SIZE  256  /* initial size , can grow */
/* number of fields at startup, must be a power of 2 
   and FBANK_SZ-1 must be divisible by 3! */
#define  FBANK_SZ        256
#define  FB_SHIFT          8   /* lg(FBANK_SZ) */
#define  NUM_FBANK        128   /* see MAX_FIELD below */


#define  MAX_SPLIT        (FBANK_SZ-1)   /* needs to be divisble by 3*/
#define  MAX_FIELD        (NUM_FBANK*FBANK_SZ - 1)

#define  MIN_SPRINTF        400


#define  BUFFSZ         4096
  /* starting buffer size for input files, grows if 
     necessary */

#define  HASH_PRIME  53
#define  A_HASH_PRIME 199


#define  MAX_COMPILE_ERRORS  5 /* quit if more than 4 errors */

#endif   /* SIZES_H */
