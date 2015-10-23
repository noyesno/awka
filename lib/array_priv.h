/*--------------------------------------------------*
 | array_priv.h                                     |
 | Header file for array.c, part of the Awka        |
 | Library, Copyright 1999, Andrew Sumner.          |
 | This file is covered by the GNU General Public   |
 | License (GPL).                                   |
 *--------------------------------------------------*/

#ifndef _ARRAY_PRIV_H
#define _ARRAY_PRIV_H

#define _a_SPLT_BASESTR  NULL
#define _a_SPLT_LOCALSTR (char *) 1

/* _a_HSH_MAXDEPTH is the average number of nodes in each slot of a
   hash array required for it to double its size.  The higher the number,
   the fewer times the array will double, but average search speeds will
   be slower.  I have generally found 7 to be a good compromise. */
/* #define _a_HSH_MAXDEPTH 4  */
#define _a_HSH_MAXDEPTH 4

/* This must turn on all bits up to the given value - ie ((n ^ 2) - 1).
   The higher the number the more memory will be used initially, but 
   doubling will be postponed longer, and the array will be more sparse
   providing better performance before it fills up. */
#define _a_HASHMASK 63

#define _a_ARR_INT 1
#define _a_ARR_STR 2

#endif
