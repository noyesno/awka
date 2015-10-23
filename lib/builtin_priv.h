/*--------------------------------------------------*
 | builtin_priv.h                                   |
 | Header file for builtin.c, part of the Awka      |
 | Library, Copyright 1999, Andrew Sumner.          |
 | This file is covered by the GNU General Public   |
 | License (GPL).                                   |
 *--------------------------------------------------*/

#ifndef _BUILTIN_PRIV_H
#define _BUILTIN_PRIV_H

typedef struct {
  char *name;
  unsigned char min_args;
  unsigned char max_args;
} BI_VARARG;

#ifdef BUILTIN_HOME

BI_VARARG _a_bi_vararg[] = {
"strconcat", 2, 255,
"print", 0, 255,
"printf", 1, 255,
"getline", 3, 3,
"length", 0, 1,
"index", 2, 2,
"substr", 2, 3,
"sprintf", 1, 255,
"split", 2, 3,
"match", 2, 2,
"sub", 2, 3,
"tocase", 1, 1,
"sin", 1, 1,
"cos", 1, 1,
"atan2", 2, 2,
"log", 1, 1,
"exp", 1, 1,
"sqrt", 1, 1,
"srand", 0, 1,
"rand", 0, 0,
"close", 1, 1,
"system", 1, 1,
"fflush", 0, 1,
"trim", 1, 2,
"ltrim", 1, 2,
"rtrim", 1, 2,
"left", 2, 2,
"right", 2, 2,
"ascii", 1, 2,
"char", 1, 1,
"and", 2, 2,
"or", 2, 2,
"xor", 2, 2,
"compl", 1, 1,
"lshift", 2, 2,
"rshift", 2, 2,
"time", 0, 6,
"localtime", 0, 1,
"gmtime", 0, 1,
"mktime", 1, 1,
"min", 2, 255,
"max", 2, 255,
"int", 1, 1,
"systime", 0, 0,
"strftime", 0, 2,
"gensub", 3, 4
};

#ifdef _BUILTIN_H
#define _a_TYPE_VARARG 1
#define _a_TYPE_VAR    2

#define _a_SPRINTF_BUFFER 2048  /* compiled-in sprintf buffer size */
void _awka_sopen(_a_IOSTREAM *s, char flag);
#endif /* _BUILTIN_H */

#else
extern BI_VARARG _a_bi_vararg[];
#endif /* BUILTIN_HOME */

#define _BI_STRCONCAT 0
#define _BI_PRINT     1
#define _BI_PRINTF    2
#define _BI_GETLINE   3
#define _BI_LENGTH    4
#define _BI_INDEX     5
#define _BI_SUBSTR    6
#define _BI_SPRINTF   7
#define _BI_SPLIT     8
#define _BI_MATCH     9
#define _BI_SUB       10
#define _BI_TOCASE    11
#define _BI_SIN       12
#define _BI_COS       13
#define _BI_ATAN2     14
#define _BI_LOG       15
#define _BI_EXP       16
#define _BI_SQRT      17
#define _BI_SRAND     18
#define _BI_RAND      19
#define _BI_CLOSE     20
#define _BI_SYSTEM    21
#define _BI_FFLUSH    22
#define _BI_TRIM      23
#define _BI_LTRIM     24
#define _BI_RTRIM     25
#define _BI_LEFT      26
#define _BI_RIGHT     27
#define _BI_ASCII     28
#define _BI_CHAR      29
#define _BI_AND       30
#define _BI_OR        31
#define _BI_XOR       32
#define _BI_COMPL     33
#define _BI_LSHIFT    34
#define _BI_RSHIFT    35
#define _BI_TIME      36
#define _BI_LOCALTIME 37
#define _BI_GMTIME    38
#define _BI_MKTIME    39
#define _BI_MIN       40
#define _BI_MAX       41
#define _BI_INT       42
#define _BI_SYSTIME   43
#define _BI_STRFTIME  44
#define _BI_GENSUB    45
#define _BI_ALENGTH   46
#define _BI_ARGCOUNT  47
#define _BI_ARGVAL    48
#define _BI_ASORT     49

#endif
