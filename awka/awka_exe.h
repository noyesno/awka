/*-------------------------------------------------*
 | array.h                                         |
 | Header file for awka.c, part of the awka        |
 | program, Copyright 1999, Andrew Sumner.         |
 | This file is covered by the GNU General         |
 | Public License - see file GPL for more details. |
 *-------------------------------------------------*/

#ifndef _AWKA_EXE_H
#define _AWKA_EXE_H

#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Min(a, b) (((a) < (b)) ? (a) : (b))
#ifndef TRUE
#  define TRUE  1
#  define FALSE 0
#endif

#include "../lib/builtin_priv.h"
#include "code.h"

char *       awka_a_cat(int, int *, char *);
char *       awka_a_del(int, int *, char *);
char *        awka_push(int, int *, char *); 
char *      awka_a_test(int, int *, char *);
char *         awka_add(int, int *, char *);
char *     awka_add_asg(int, int *, char *);
char *       awka_aloop(int, int *, char *);
char *      awka_assign(int, int *, char *);
char *         awka_cat(int, int *, char *);
char *        awka_call(int, int *, char *);
char *       awka_del_a(int, int *, char *);
char *         awka_div(int, int *, char *);
char *     awka_div_asg(int, int *, char *);
char *          awka_eq(int, int *, char *);
char *        awka_exit(int, int *, char *);
char *       awka_exit0(int, int *, char *);
char *   awka_f_add_asg(int, int *, char *);
char *    awka_f_assign(int, int *, char *);
char *   awka_f_div_asg(int, int *, char *);
char *   awka_f_mod_asg(int, int *, char *);
char *   awka_f_mul_asg(int, int *, char *);
char *  awka_f_post_dec(int, int *, char *);
char *  awka_f_post_inc(int, int *, char *);
char *   awka_f_pow_asg(int, int *, char *);
char *   awka_f_pre_dec(int, int *, char *);
char *   awka_f_pre_inc(int, int *, char *);
char *   awka_f_sub_asg(int, int *, char *);
char *          awka_gt(int, int *, char *);
char *         awka_gte(int, int *, char *);
char *         awka_jnz(int, int *, char *);
char *          awka_jz(int, int *, char *);
char *        awka_ljnz(int, int *, char *);
char *         awka_ljz(int, int *, char *);
char *          awka_lt(int, int *, char *);
char *         awka_lte(int, int *, char *);
char *       awka_match(int, int *, char *);
char *      awka_match0(int, int *, char *);
char *       awka_match(int, int *, char *);
char *         awka_mod(int, int *, char *);
char *     awka_mod_asg(int, int *, char *);
char *         awka_mul(int, int *, char *);
char *     awka_mul_asg(int, int *, char *);
char *         awka_neq(int, int *, char *);
char *        awka_next(int, int *, char *);
char *    awka_nextfile(int, int *, char *);
char *         awka_not(int, int *, char *);
char *       awka_ol_gl(int, int *, char *);
char *         awka_pop(int, int *, char *);
char *    awka_post_dec(int, int *, char *);
char *    awka_post_inc(int, int *, char *);
char *         awka_pow(int, int *, char *);
char *     awka_pow_asg(int, int *, char *);
char *     awka_pre_dec(int, int *, char *);
char *     awka_pre_inc(int, int *, char *);
char *      awka_printf(int, int *, char *);
char *       awka_range(int, int *, char *);
char *         awka_ret(int, int *, char *);
char *        awka_ret0(int, int *, char *);
char *      awka_set_al(int, int *, char *);
char *         awka_sub(int, int *, char *);
char *     awka_sub_asg(int, int *, char *);
char *        awka_test(int, int *, char *);
char *      awka_uminus(int, int *, char *);
char *       awka_uplus(int, int *, char *);
char *     awka_builtin(int, int *, char *);
char *     awka_getline(int, int *, char *);
char *       awka_split(int, int *, char *);
char *        awka_gsub(int, int *, char *);
char *       awka_close(int, int *, char *);
char *        awka_goto(int, int *, char *);
char *      awka_tocase(int, int *, char *);
char *    awka_nullfunc(int, int *, char *);
char *        awka_math(int, int *, char *);
char *       awka_index(int, int *, char *);
char *       awka_colon(int, int *, char *);
char *   awka_bi_length(int, int *, char *);
char *      awka_substr(int, int *, char *);
char *       awka_asort(int, int *, char *);
char *     awka_alength(int, int *, char *);
char *        awka_expr(int, int *, char *);
char *    awka_argcount(int, int *, char *);
char *      awka_argval(int, int *, char *);
char *      awka_gensub(int, int *, char *);

#define _VAR_REF  0
#define _VAR_SET  1
#define _VARTYPE_G_SCALAR  1
#define _VARTYPE_G_ARRAY   2
#define _VARTYPE_L_SCALAR  3
#define _VARTYPE_L_ARRAY   4

#define _VALTYPE_UNK       0
#define _VALTYPE_NUM       1
#define _VALTYPE_STR       2
#define _VALTYPE_RE        4

typedef struct {
  char *name;
  unsigned int *usage;
  unsigned int *lines;
  char **files;
  unsigned line_no;
  unsigned line_allc;
  char type;
  char valtype;    /* the primary type of data held by this var */
  char valtype2;   /* if its been cast to another type */
} awka_varname;

struct a_sc
{
   char *name;
   char *(*func)(int, int *, char *);
   int op;
   int varidx;
   char pop;
   char ftype;
};

#ifdef AWKA_MAIN
/* to add new builtins
 * insert Bukltins alphabettically (at >= BI_MIN)
 * Re-arrange the order of awka_exe.h defines, then renumber
 * Adjust BI_MIN and BI_MAX
 */
struct a_sc code[] = {
{ "abort",       awka_exit,         _ABORT,       -1,             TRUE,  0, },
{ "abort0",      awka_exit0,        _ABORT0,      -1,             TRUE,  0, },
{ "a_cat",       awka_a_cat,        A_CAT,        -1,             FALSE, 0, },
{ "a_del",       awka_a_del,        A_DEL,        -1,             TRUE,  0, },
{ "a_pusha",     awka_push,         A_PUSHA,      -1,             FALSE, 0, },
{ "a_test",      awka_a_test,       A_TEST,       -1,             FALSE, 0, },
{ "add",         awka_add,          _ADD,         -1,             FALSE, 0, },
{ "add_asg",     awka_add_asg,      _ADD_ASG,     -1,             FALSE, 0, },
{ "ae_pusha",    awka_push,         AE_PUSHA,     -1,             FALSE, 0, },
{ "ae_pushi",    awka_push,         AE_PUSHI,     -1,             FALSE, 0, },
{ "aloop",       awka_aloop,        ALOOP,        -1,             TRUE,  0, },
{ "assign",      awka_assign,       _ASSIGN,      -1,             FALSE, 0, },
{ "break",       awka_pop,          _BREAK,       -1,             TRUE,  0, },
{ "builtin",     awka_nullfunc,     _BUILTIN,     -1,             FALSE, 0, },
{ "call",        awka_call,         _CALL,        -1,             FALSE, 0, },
{ "cat",         awka_cat,          _CAT,         -1,             FALSE, 0, },
{ "cleanup",     awka_exit0,        _CLEANUP,     -1,             TRUE,  0, },
{ "colon",       awka_colon,        _COLON,       -1,             FALSE, 0, },
{ "continue",    awka_pop,          _CONTINUE,    -1,             TRUE,  0, },
{ "del_a",       awka_del_a,        DEL_A,        -1,             TRUE,  0, },
{ "div",         awka_div,          _DIV,         -1,             FALSE, 0, },
{ "div_asg",     awka_div_asg,      _DIV_ASG,     -1,             FALSE, 0, },
{ "eq",          awka_eq,           _EQ,          -1,             FALSE, 0, },
{ "else",        awka_jz,           _ELSE,        -1,             TRUE,  0, },
{ "exit",        awka_exit,         _EXIT,        -1,             TRUE,  0, },
{ "exit0",       awka_exit0,        _EXIT0,       -1,             TRUE,  0, },
{ "f_add_asg",   awka_add_asg,      F_ADD_ASG,    -1,             FALSE, 0, },
{ "f_assign",    awka_assign,       F_ASSIGN,     -1,             FALSE, 0, },
{ "f_div_asg",   awka_div_asg,      F_DIV_ASG,    -1,             FALSE, 0, },
{ "f_mod_asg",   awka_mod_asg,      F_MOD_ASG,    -1,             FALSE, 0, },
{ "f_mul_asg",   awka_mul_asg,      F_MUL_ASG,    -1,             FALSE, 0, },
{ "f_post_dec",  awka_post_dec,     F_POST_DEC,   -1,             FALSE, 0, },
{ "f_post_inc",  awka_post_inc,     F_POST_INC,   -1,             FALSE, 0, },
{ "f_pow_asg",   awka_pow_asg,      F_POW_ASG,    -1,             FALSE, 0, },
{ "f_pre_dec",   awka_pre_dec,      F_PRE_DEC,    -1,             FALSE, 0, },
{ "f_pre_inc",   awka_pre_inc,      F_PRE_INC,    -1,             FALSE, 0, },
{ "f_pusha",     awka_push,         F_PUSHA,      -1,             FALSE, 0, },
{ "f_pushi",     awka_push,         F_PUSHI,      -1,             FALSE, 0, },
{ "f_sub_asg",   awka_sub_asg,      F_SUB_ASG,    -1,             FALSE, 0, },
{ "fe_pusha",    awka_push,         _FE_PUSHA,    -1,             FALSE, 0, },
{ "fe_pushi",    awka_push,         FE_PUSHI,     -1,             FALSE, 0, },
{ "halt",        NULL,              _HALT,        -1,             FALSE, 0, },
{ "goto",        awka_goto,         _GOTO,        -1,             TRUE,  0, },
{ "gt",          awka_expr,         _GT,          -1,             FALSE, 0, },
{ "gte",         awka_expr,         _GTE,         -1,             FALSE, 0, },
{ "jmain",       awka_nullfunc,     _JMAIN,       -1,             TRUE,  0, },
{ "jmp",         awka_nullfunc,     _JMP,         -1,             FALSE, 0, },
{ "jnz",         awka_jnz,          _JNZ,         -1,             TRUE,  0, },
{ "jz",          awka_jz,           _JZ,          -1,             TRUE,  0, },
{ "l_pusha",     awka_push,         L_PUSHA,      -1,             FALSE, 0, },
{ "l_pushi",     awka_push,         L_PUSHI,      -1,             FALSE, 0, },
{ "la_pusha",    awka_push,         LA_PUSHA,     -1,             FALSE, 0, },
{ "lae_pusha",   awka_push,         LAE_PUSHA,    -1,             FALSE, 0, },
{ "lae_pushi",   awka_push,         LAE_PUSHI,    -1,             FALSE, 0, },
{ "ljnz",        awka_ljnz,         _LJNZ,        -1,             FALSE, 0, },
{ "ljz",         awka_ljz,          _LJZ,         -1,             FALSE, 0, },
{ "lt",          awka_expr,         _LT,          -1,             FALSE, 0, },
{ "lte",         awka_expr,         _LTE,         -1,             FALSE, 0, },
{ "match",       awka_match,        _MATCH,       -1,             FALSE, 1, },
{ "match0",      awka_match,        _MATCH0,      -1,             FALSE, 1, },
{ "match1",      awka_match,        _MATCH1,      -1,             FALSE, 1, },
{ "match2",      awka_match,        _MATCH2,      -1,             FALSE, 1, },
{ "mod",         awka_mod,          _MOD,         -1,             FALSE, 0, },
{ "mod_asg",     awka_mod_asg,      _MOD_ASG,     -1,             FALSE, 0, },
{ "mul",         awka_mul,          _MUL,         -1,             FALSE, 0, },
{ "mul_asg",     awka_mul_asg,      _MUL_ASG,     -1,             FALSE, 0, },
{ "neq",         awka_neq,          _NEQ,         -1,             FALSE, 0, },
{ "next",        awka_next,         _NEXT,        -1,             TRUE,  0, },
{ "nextfile",    awka_nextfile,     _NEXTFILE,    -1,             TRUE,  0, },
{ "nf_pushi",    awka_push,         NF_PUSHI,     -1,             FALSE, 0, },
{ "not",         awka_not,          _NOT,         -1,             FALSE, 0, },
{ "ol_gl",       awka_ol_gl,        OL_GL,        -1,             TRUE,  0, },
{ "ol_gl_nr",    awka_ol_gl,        OL_GL_NR,     -1,             TRUE,  0, },
{ "omain",       awka_nullfunc,     _OMAIN,       -1,             TRUE,  0, },
{ "pop",         awka_pop,          _POP,         -1,             TRUE,  0, },
{ "pop_al",      awka_nullfunc,     POP_AL,       -1,             TRUE,  0, },
{ "post_dec",    awka_post_dec,     _POST_DEC,    -1,             FALSE, 0, },
{ "post_inc",    awka_post_inc,     _POST_INC,    -1,             FALSE, 0, },
{ "pow",         awka_pow,          _POW,         -1,             FALSE, 0, },
{ "pow_asg",     awka_pow_asg,      _POW_ASG,     -1,             FALSE, 0, },
{ "pre_dec",     awka_pre_dec,      _PRE_DEC,     -1,             FALSE, 0, },
{ "pre_inc",     awka_pre_inc,      _PRE_INC,     -1,             FALSE, 0, },
{ "print",       awka_printf,       _PRINT,       -1,             TRUE,  0, },
{ "printf",      awka_printf,       _PRINTF,      -1,             TRUE,  0, },
{ "pusha",       awka_push,         _PUSHA,       -1,             FALSE, 0, },
{ "pushc",       awka_push,         _PUSHC,       -1,             FALSE, 0, },
{ "pushd",       awka_push,         _PUSHD,       -1,             FALSE, 0, },
{ "pushi",       awka_push,         _PUSHI,       -1,             FALSE, 0, },
{ "pushint",     awka_push,         _PUSHINT,     -1,             FALSE, 0, },
{ "pushs",       awka_push,         _PUSHS,       -1,             FALSE, 0, },
{ "qmark",       awka_jz,           _QMARK,       -1,             FALSE, 0, },
{ "range",       awka_range,        _RANGE,       -1,             FALSE, 0, },
{ "ret",         awka_ret,          _RET,         -1,             TRUE,  0, },
{ "ret0",        awka_ret0,         _RET0,        -1,             TRUE,  0, },
{ "set_al",      awka_set_al,       SET_ALOOP,    -1,             TRUE,  0, },
{ "stop",        awka_jz,           _STOP,        -1,             TRUE,  0, },
{ "sub",         awka_sub,          _SUB,         -1,             FALSE, 0, },
{ "sub_asg",     awka_sub_asg,      _SUB_ASG,     -1,             FALSE, 0, },
{ "test",        awka_test,         _TEST,        -1,             FALSE, 0, },
/* END of OP CODES */

{ "uminus",      awka_uminus,       _UMINUS,      -1,             FALSE, 0, },
{ "uplus",       awka_uplus,        _UPLUS,       -1,             FALSE, 0, },
{ "abs",         awka_math,         _ABS,         _BI_ABS,        FALSE, 0, },
{ "acos",        awka_math,         _ACOS,        _BI_ACOS,       FALSE, 0, },
{ "acosh",       awka_math,         _ACOSH,       _BI_ACOSH,      FALSE, 0, },
{ "alength",     awka_alength,      _ALENGTH,     _BI_ALENGTH,    FALSE, 1, },
{ "and",         awka_builtin,      _AND,         _BI_AND,        FALSE, 1, },
{ "argcount",    awka_argcount,     _ARGCOUNT,    _BI_ARGCOUNT,   FALSE, 1, },
{ "argval",      awka_argval,       _ARGVAL,      _BI_ARGVAL,     FALSE, 1, },
{ "ascii",       awka_builtin,      _ASCII,       _BI_ASCII,      FALSE, 1, },
{ "asin",        awka_math,         _ASIN,        _BI_ASIN,       FALSE, 0, },
{ "asinh",       awka_math,         _ASINH,       _BI_ASINH,      FALSE, 0, },
{ "asort",       awka_asort,        _ASORT,       _BI_ASORT,      FALSE, 1, },
{ "atan",        awka_math,         _ATAN,        _BI_ATAN,       FALSE, 0, },
{ "atan2",       awka_math,         _ATAN2,       _BI_ATAN2,      FALSE, 0, },
{ "atanh",       awka_math,         _ATANH,       _BI_ATANH,      FALSE, 0, },
{ "ceil",        awka_math,         _CEIL,        _BI_CEIL,       FALSE, 0, },
{ "char",        awka_builtin,      _CHAR,        _BI_CHAR,       FALSE, 2, },
{ "close",       awka_close,        _CLOSE,       _BI_CLOSE,      TRUE,  1, },
{ "compl",       awka_builtin,      _COMPL,       _BI_COMPL,      FALSE, 1, },
{ "cos",         awka_math,         _COS,         _BI_COS,        FALSE, 0, },
{ "cosh",        awka_math,         _COSH,        _BI_COSH,       FALSE, 0, },
{ "erf",         awka_math,         _ERF,         _BI_ERF,        FALSE, 0, },
{ "erfc",        awka_math,         _ERFC,        _BI_ERFC,       FALSE, 0, },
{ "exp",         awka_math,         _EXP,         _BI_EXP,        FALSE, 0, },
{ "exp2",        awka_math,         _EXP2,        _BI_EXP2,       FALSE, 0, },
{ "fflush",      awka_builtin,      _FFLUSH,      _BI_FFLUSH,     TRUE,  0, },
{ "floor",       awka_math,         _FLOOR,       _BI_FLOOR,      FALSE, 0, },
{ "gensub",      awka_gensub,       _GENSUB,      _BI_GENSUB,     FALSE, 2, },
{ "getline",     awka_getline,      _GETLINE,     _BI_GETLINE,    FALSE, 1, },
{ "gmtime",      awka_builtin,      a_GMTIME,     _BI_GMTIME,     FALSE, 2, },
{ "gsub",        awka_gsub,         _GSUB,        _BI_SUB,        FALSE, 1, },
{ "hypot",       awka_math,         _HYPOT,       _BI_HYPOT,      FALSE, 0, },
{ "index",       awka_index,        _INDEX,       _BI_INDEX,      FALSE, 1, },
{ "int",         awka_math,         a_INT,        _BI_INT,        FALSE, 0, },
{ "isarray",     awka_builtin,      _ISARRAY,     _BI_ISARRAY,    FALSE, 0, },
{ "left",        awka_builtin,      _LEFT,        _BI_LEFT,       FALSE, 2, },
{ "length",      awka_bi_length,    _LENGTH,      _BI_LENGTH,     FALSE, 1, },
{ "lgamma",      awka_math,         _LGAMMA,      _BI_LGAMMA,     FALSE, 0, },
{ "localtime",   awka_builtin,      a_LOCALTIME,  _BI_LOCALTIME,  FALSE, 2, },
{ "log",         awka_math,         _LOG,         _BI_LOG,        FALSE, 0, },
{ "log10",       awka_math,         _LOG10,       _BI_LOG10,      FALSE, 0, },
{ "log2",        awka_math,         _LOG2,        _BI_LOG2,       FALSE, 0, },
{ "lshift",      awka_builtin,      _LSHIFT,      _BI_LSHIFT,     FALSE, 1, },
{ "ltrim",       awka_builtin,      _LTRIM,       _BI_LTRIM,      FALSE, 2, },
{ "max",         awka_builtin,      _MAX,         _BI_MAX,        FALSE, 1, },
{ "min",         awka_builtin,      _MIN,         _BI_MIN,        FALSE, 1, },
{ "mktime",      awka_builtin,      a_MKTIME,     _BI_MKTIME,     FALSE, 1, },
{ "mod",         awka_math,         _MMOD,        _BI_MMOD,       FALSE, 0, },
{ "or",          awka_builtin,      _OR,          _BI_OR,         FALSE, 1, },
{ "pow",         awka_math,         _MPOW,        _BI_MPOW,       FALSE, 0, },
{ "rand",        awka_builtin,      _RAND,        _BI_RAND,       FALSE, 1, },
{ "right",       awka_builtin,      _RIGHT,       _BI_RIGHT,      FALSE, 2, },
{ "round",       awka_math,         _ROUND,       _BI_ROUND,      FALSE, 0, },
{ "rshift",      awka_builtin,      _RSHIFT,      _BI_RSHIFT,     FALSE, 1, },
{ "rtrim",       awka_builtin,      _RTRIM,       _BI_RTRIM,      FALSE, 2, },
{ "sin",         awka_math,         _SIN,         _BI_SIN,        FALSE, 0, },
{ "sinh",        awka_math,         _SINH,        _BI_SINH,       FALSE, 0, },
{ "split",       awka_split,        _SPLIT,       _BI_SPLIT,      FALSE, 1, },
{ "sprintf",     awka_builtin,      _SPRINTF,     _BI_SPRINTF,    FALSE, 2, },
{ "sqrt",        awka_math,         _SQRT,        _BI_SQRT,       FALSE, 0, },
{ "srand",       awka_builtin,      _SRAND,       _BI_SRAND,      FALSE, 0, },
{ "strftime",    awka_builtin,      a_STRFTIME,   _BI_STRFTIME,   FALSE, 2, },
{ "sub_bi",      awka_gsub,         _SUB_BI,      _BI_SUB,        FALSE, 1, },
{ "substr",      awka_substr,       _SUBSTR,      _BI_SUBSTR,     FALSE, 2, },
{ "system",      awka_builtin,      _SYSTEM,      _BI_SYSTEM,     FALSE, 1, },
{ "systime",     awka_builtin,      _SYSTIME,     _BI_SYSTIME,    FALSE, 1, },
{ "tan",         awka_math,         _TAN,         _BI_TAN,        FALSE, 0, },
{ "tanh",        awka_math,         _TANH,        _BI_TANH,       FALSE, 0, },
{ "tgamma",      awka_math,         _TGAMMA,      _BI_TGAMMA,     FALSE, 0, },
{ "time",        awka_builtin,      a_TIME,       _BI_TIME,       FALSE, 1, },
{ "tolower",     awka_tocase,       _TOLOWER,     _BI_TOCASE,     FALSE, 2, },
{ "totitle",     awka_tocase,       _TOTITLE,     _BI_TOCASE,     FALSE, 2, },
{ "toupper",     awka_tocase,       _TOUPPER,     _BI_TOCASE,     FALSE, 2, },
{ "trim",        awka_builtin,      _TRIM,        _BI_TRIM,       FALSE, 2, },
{ "trunc",       awka_math,         _TRUNC,       _BI_TRUNC,      FALSE, 0, },
{ "typeof",      awka_builtin,      _TYPEOF,      _BI_TYPEOF,     FALSE, 0, },
{ "xor",         awka_builtin,      _XOR,         _BI_XOR,        FALSE, 1  },
{ "fsize",       awka_builtin,      _FSIZE,       -1,             FALSE, 1, },
{ "fseek",       awka_builtin,      _FSEEK,       -1,             FALSE, 1, },
{ "ftell",       awka_builtin,      _FTELL,       -1,             FALSE, 1, }
,{ NULL,          awka_nullfunc,     0,            0,              FALSE, 0  }
};

struct a_sc ext_funcs[] = {
{ "alength",     awka_alength,      _ALENGTH,     _BI_ALENGTH,    FALSE, 1, },
{ "and",         awka_builtin,      _AND,         _BI_AND,        FALSE, 1, },
{ "argcount",    awka_argcount,     _ARGCOUNT,    _BI_ARGCOUNT,   FALSE, 1, },
{ "argval",      awka_argval,       _ARGVAL,      _BI_ARGVAL,     FALSE, 1, },
{ "ascii",       awka_builtin,      _ASCII,       _BI_ASCII,      FALSE, 1, },
{ "asort",       awka_asort,        _ASORT,       _BI_ASORT,      FALSE, 1, },
{ "char",        awka_builtin,      _CHAR,        _BI_CHAR,       FALSE, 2, },
{ "compl",       awka_builtin,      _COMPL,       _BI_COMPL,      FALSE, 1, },
{ "gmtime",      awka_builtin,      a_GMTIME,     _BI_GMTIME,     FALSE, 2, },
{ "isarray",     awka_builtin,      _ISARRAY,     _BI_ISARRAY,    FALSE, 0, },
{ "left",        awka_builtin,      _LEFT,        _BI_LEFT,       FALSE, 2, },
{ "localtime",   awka_builtin,      a_LOCALTIME,  _BI_LOCALTIME,  FALSE, 2, },
{ "lshift",      awka_builtin,      _LSHIFT,      _BI_LSHIFT,     FALSE, 1, },
{ "ltrim",       awka_builtin,      _LTRIM,       _BI_LTRIM,      FALSE, 2, },
{ "max",         awka_builtin,      _MAX,         _BI_MAX,        FALSE, 1, },
{ "min",         awka_builtin,      _MIN,         _BI_MIN,        FALSE, 1, },
{ "mktime",      awka_builtin,      a_MKTIME,     _BI_MKTIME,     FALSE, 1, },
{ "or",          awka_builtin,      _OR,          _BI_OR,         FALSE, 1, },
{ "right",       awka_builtin,      _RIGHT,       _BI_RIGHT,      FALSE, 2, },
{ "rshift",      awka_builtin,      _RSHIFT,      _BI_RSHIFT,     FALSE, 1, },
{ "rtrim",       awka_builtin,      _RTRIM,       _BI_RTRIM,      FALSE, 2, },
{ "strftime",    awka_builtin,      a_STRFTIME,   _BI_STRFTIME,   FALSE, 2, },
{ "systime",     awka_builtin,      _SYSTIME,     _BI_SYSTIME,    FALSE, 1, },
{ "time",        awka_builtin,      a_TIME,       _BI_TIME,       FALSE, 1, },
{ "totitle",     awka_tocase,       _TOTITLE,     _BI_TOCASE,     FALSE, 2, },
{ "trim",        awka_builtin,      _TRIM,        _BI_TRIM,       FALSE, 2, },
{ "typeof",      awka_builtin,      _TYPEOF,      _BI_TYPEOF,     FALSE, 0, },
{ "xor",         awka_builtin,      _XOR,         _BI_XOR,        FALSE, 1, },
{ "fsize",       awka_builtin,      _FSIZE,       -1,             FALSE, 1, },
{ "fseek",       awka_builtin,      _FSEEK,       -1,             FALSE, 1, },
{ "ftell",       awka_builtin,      _FTELL,       -1,             FALSE, 1, },
{ NULL,          awka_nullfunc,     0,            0,              FALSE, 0  }
};
#else
extern struct a_sc code[];
extern struct a_sc ext_funcs[];
#endif

#define NONE 0
#define BEGIN 1
#define MAIN 2
#define END 3
#define FUNC 4

#define _NUL 0
#define _VAR 1
#define _DBL 2
#define _STR 3
#define _REG 4
#define _UNK 5
#define _TRU 6
#define _ARR 7
#define _ROVAR 8
#define _VARX 9

struct pc
{
   char *(*func)(int, int *, char *);
   char *val ;
   char *arg ;
   char **code0 ;
   char **code ;
   char *file ;
   int code0_used;
   int code_used;
   int code0_allc;
   int code_allc;
   int inst ;
   int line ;
   int minst ;
   int ljumpfrom ;
   int jumpfrom ;
   int jumpto ;
   int op ;
   int earliest ;
   int endloop ;
   int varidx ;
   int prevcat;
   int prev2;
   char done ;
   char foreverloop ;
   char doloop ;
   char context ;
   char label ;
   char pop ;
   char printed ;
   char ftype ;
};

struct ivar_idx
{
  char *name;
  char *vname;
};

struct _fargs
{
  char *func;
  int arg_no;
  char array;
};

struct _fcalls
{
  char *calledfunc;
  char *callingfunc;
  char *var_calling;
  int arg_calling;
  int arg_called;
};

void readprog();
void translate();
int add2arraylist(char *);
int add2fargs(char *, char, int);
void add2fcalls(char *, char *, char *, int, int);

#ifndef AWKA_MAIN
extern struct pc *progcode;
extern FILE *fp;
extern int curop_id, prog_allc, prog_no, curinst, curminst;
extern char buf[4096], *curarg, *curval;
extern char begin_used, main_used, end_used, functab_used;
extern int mode;
#endif

#endif
