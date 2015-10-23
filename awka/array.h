/*
array.h 
copyright 1991-96, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
*/

/*
This file was generated with the command

   notangle -R'"array.h"' array.w > array.h

Notangle is part of Norman Ramsey's noweb literate programming package
available from CTAN(ftp.shsu.edu).

It's easiest to read or modify this file by working with array.w.
*/

#ifndef ARRAY_H
#define ARRAY_H 1
typedef struct array {
   PTR ptr ;  /* What this points to depends on the type */
   unsigned size ; /* number of elts in the table */
   unsigned limit ; /* Meaning depends on type */
   unsigned hmask ; /* bitwise and with hash value to get table index */
   short type ;  /* values in AY_NULL .. AY_SPLIT */
} *ARRAY ;

#define AY_NULL         0
#define AY_INT          1
#define AY_STR          2
#define AY_SPLIT        4

#define NO_CREATE  0
#define CREATE     1

#define new_ARRAY()  ((ARRAY)memset(ZMALLOC(struct array),0,sizeof(struct array)))

CELL* PROTO(array_find, (ARRAY,CELL*,int)) ;
void  PROTO(array_delete, (ARRAY,CELL*)) ;
void  PROTO(array_load, (ARRAY,int)) ;
void  PROTO(array_clear, (ARRAY)) ;
STRING** PROTO(array_loop_vector, (ARRAY,unsigned*)) ;
CELL* PROTO(array_cat, (CELL*,int)) ;

#endif /* ARRAY_H */

