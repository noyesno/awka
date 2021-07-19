
/********************************************
code.h
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/


/* $Log: code.h,v $
 * Revision 1.5  1995/06/18  19:42:15  mike
 * Remove some redundant declarations and add some prototypes
 *
 * Revision 1.4  1994/12/13  00:13:01  mike
 * delete A statement to delete all of A at once
 *
 * Revision 1.3  1993/12/01  14:25:06  mike
 * reentrant array loops
 *
 * Revision 1.2  1993/07/22  00:04:01  mike
 * new op code _LJZ _LJNZ
 *
 * Revision 1.1.1.1  1993/07/03  18:58:10  mike
 * move source to cvs
 *
 * Revision 5.3  1993/01/14  13:11:11  mike
 * code2() -> xcode2()
 *
 * Revision 5.2  1993/01/07  02:50:33  mike
 * relative vs absolute code
 *
 * Revision 5.1  1991/12/05  07:59:07  brennan
 * 1.1 pre-release
 *
*/


/*  code.h  */

#ifndef  CODE_H
#define  CODE_H

#ifndef _AWKA_EXE_H

#include "memory.h"

#define  PAGESZ        512
        /* number of code instructions allocated at one time */
#define  CODEWARN        16

/* coding scope */
#define   SCOPE_MAIN    0
#define   SCOPE_BEGIN   1  
#define   SCOPE_END     2
#define   SCOPE_FUNCT   3


typedef struct {
INST *base, *limit, *warn, *ptr ;
} CODEBLOCK ;

extern CODEBLOCK active_code ;
extern CODEBLOCK *main_code_p, *begin_code_p, *end_code_p ;

extern INST *main_start, *begin_start, *end_start  ;
extern unsigned main_size, begin_size  ;
extern INST *execution_start ;
extern INST *next_label ;  /* next statements jump to here */
extern int dump_code_flag ;

#define code_ptr  active_code.ptr
#define code_base active_code.base
#define code_warn active_code.warn
#define code_limit active_code.limit
#define code_offset (code_ptr-code_base)

#define INST_BYTES(x) (sizeof(INST)*(unsigned)(x))

extern  CELL  eval_stack[] ;
extern int exit_code ;


static void code1(int x) {
  code_ptr -> lineno = token_lineno ;
  code_ptr -> file = pfile_name ;
  code_ptr++ -> op = x ;
}

/* shutup picky compilers */
#define  code2(x,p)  xcode2(x,(PTR)(p))

void  PROTO(xcode2, (int, PTR)) ;
void  PROTO(code2op, (int, int)) ;
INST *PROTO(code_shrink, (CODEBLOCK*, unsigned*)) ;
void  PROTO(code_grow, (void)) ;
void  PROTO(set_code, (void)) ;
void  PROTO(be_setup, (int)) ;
void  PROTO(dump_code, (void)) ;

#endif  /* _AWKA_EXE_H */

/*  the machine opcodes  */
/* to avoid confusion with a ptr FE_PUSHA must have op code 0 */
/* unfortunately enums are less portable than defines */

/* The order needs to match awka_exe.h 
 * and sorted alphabetically,
 * but the order in bi_func is not specific 
 */

#define FE_PUSHA     0

#define _ABORT       1
#define _ABORT0      2
#define A_CAT        3
#define A_DEL        4
#define A_PUSHA      5
#define A_TEST       6
#define _ADD         7
#define _ADD_ASG     8
#define AE_PUSHA     9
#define AE_PUSHI    10
#define ALOOP       11
#define _ASSIGN     12
#define _BREAK      13
#define _BUILTIN    14
#define _CALL       15
#define _CAT        16
#define _CLEANUP    17
#define _COLON      18
#define _CONTINUE   19
#define DEL_A       20
#define _DIV        21
#define _DIV_ASG    22
#define _EQ         23
#define _ELSE       24
#define _EXIT       25
#define _EXIT0      26
#define F_ADD_ASG   27
#define F_ASSIGN    28
#define F_DIV_ASG   29
#define F_MOD_ASG   30
#define F_MUL_ASG   31
#define F_POST_DEC  32
#define F_POST_INC  33
#define F_POW_ASG   34
#define F_PRE_DEC   35
#define F_PRE_INC   36
#define F_PUSHA     37
#define F_PUSHI     38
#define F_SUB_ASG   39
#define _FE_PUSHA   40
#define FE_PUSHI    41
#define _HALT       42
#define _GOTO       43
#define _GT         44
#define _GTE        45
#define _JMAIN      46
#define _JMP        47
#define _JNZ        48
#define _JZ         49
#define L_PUSHA     50
#define L_PUSHI     51
#define LA_PUSHA    52
#define LAE_PUSHA   53
#define LAE_PUSHI   54
#define _LJNZ       55
#define _LJZ        56
#define _LT         57
#define _LTE        58
#define _MATCH      59
#define _MATCH0     60
#define _MATCH1     61
#define _MATCH2     62
#define _MOD        63
#define _MOD_ASG    64
#define _MUL        65
#define _MUL_ASG    66
#define _NEQ        67
#define _NEXT       68
#define _NEXTFILE   69
#define NF_PUSHI    70
#define _NOT        71
#define OL_GL       72
#define OL_GL_NR    73
#define _OMAIN      74
#define _POP        75
#define POP_AL      76
#define _POST_DEC   77
#define _POST_INC   78
#define _POW        79
#define _POW_ASG    80
#define _PRE_DEC    81
#define _PRE_INC    82
#define _PRINT      83
#define _PRINTF     84
#define _PUSHA      85
#define _PUSHC      86
#define _PUSHD      87
#define _PUSHI      88
#define _PUSHINT    89
#define _PUSHS      90
#define _QMARK      91
#define _RANGE      92
#define _RET        93
#define _RET0       94
#define SET_ALOOP   95
#define _STOP       96
#define _SUB        97
#define _SUB_ASG    98
#define _TEST       99
#define _UMINUS     100
#define _UPLUS      101
#define _ABS        102
#define _ACOS       103
#define _ACOSH      104
#define _ALENGTH    105
#undef _AND  /* this shuts-up the cygnus compiler */
#define _AND        106
#define _ARGCOUNT   107
#define _ARGVAL     108
#define _ASCII      109
#define _ASIN       110
#define _ASINH      111
#define _ASORT      112
#define _ATAN       113
#define _ATAN2      114
#define _ATANH      115
#define _CEIL       116
#define _CHAR       117
#define _CLOSE      118
#define _COMPL      119
#define _COS        120
#define _COSH       121
#define _ERF        122
#define _ERFC       123
#define _EXP        124
#define _EXP2       125
#define _FFLUSH     126
#define _FLOOR      127
#define _GENSUB     128
#define _GETLINE    129
#define a_GMTIME    130
#define _GSUB       131
#define _HYPOT      132
#define _INDEX      133
#define a_INT       134
#define _ISARRAY    135
#define _LEFT       136
#define _LENGTH     137
#define _LGAMMA     138
#define a_LOCALTIME 139
#define _LOG        140
#define _LOG10      141
#define _LOG2       142
#define _LSHIFT     143
#define _LTRIM      144
#define _MAX        145
#define _MIN        146
#define a_MKTIME    147
#define _MMOD       148   // C Math style mod(x,y) rather than (x % y)
#define _OR         149
#define _MPOW       150   // C Math style pow(x,y) rather than (x ^ y)  or (x ** y)
#define _RAND       151
#define _RIGHT      152
#define _ROUND      153
#define _RSHIFT     154
#define _RTRIM      155
#define _SIN        156
#define _SINH       157
#define _SPLIT      158
#define _SPRINTF    159
#define _SQRT       160
#define _SRAND      161
#define a_STRFTIME  162
#define _SUB_BI     163
#define _SUBSTR     164
#define _SYSTEM     165
#define _SYSTIME    166
#define _TAN        167
#define _TANH       168
#define _TGAMMA     169
#define a_TIME      170
#define _TOLOWER    171
#define _TOTITLE    172
#define _TOUPPER    173
#define _TRIM       174
#define _TRUNC      175
#define _TYPEOF     176


#define _XOR        177  // was 150
#define _FSIZE      178
#define _FSEEK      179
#define _FTELL      180

#define CODE_MIN      0
#define CODE_MAX    100

#define BI_MIN      101
// # define BI_MAX 149     // TODO: use 153 ?  Is original 149 a bug?
#define BI_MAX      177


#define _FUNCTION   181   // was 154
#define _BEGIN      182
#define _MAIN       183
#define _END        184
#define END_CODE    185

#endif  /* CODE_H */
