#define UNEXPECTED 257
#define BAD_DECIMAL 258
#define NL 259
#define SEMI_COLON 260
#define LBRACE 261
#define RBRACE 262
#define LBOX 263
#define RBOX 264
#define COMMA 265
#define IO_OUT 266
#define COPROCESS_OUT 267
#define ASSIGN 268
#define ADD_ASG 269
#define SUB_ASG 270
#define MUL_ASG 271
#define DIV_ASG 272
#define MOD_ASG 273
#define POW_ASG 274
#define QMARK 275
#define COLON 276
#define OR 277
#define AND 278
#define IN 279
#define MATCH 280
#define EQ 281
#define NEQ 282
#define LT 283
#define LTE 284
#define GT 285
#define GTE 286
#define CAT 287
#define GETLINE 288
#define PLUS 289
#define MINUS 290
#define MUL 291
#define DIV 292
#define MOD 293
#define NOT 294
#define UMINUS 295
#define IO_IN 296
#define PIPE 297
#define COPROCESS 298
#define POW 299
#define INC_or_DEC 300
#define DOLLAR 301
#define FIELD 302
#define LPAREN 303
#define RPAREN 304
#define DOUBLE 305
#define STRING_ 306
#define RE 307
#define ID 308
#define D_ID 309
#define FUNCT_ID 310
#define BUILTIN 311
#define LENGTH 312
#define LENGTH0 313
#define PRINT 314
#define PRINTF 315
#define SPLIT 316
#define MATCH_FUNC 317
#define SUB 318
#define GSUB 319
#define GENSUB 320
#define ALENGTH_FUNC 321
#define ASORT_FUNC 322
#define DO 323
#define WHILE 324
#define FOR 325
#define BREAK 326
#define CONTINUE 327
#define IF 328
#define ELSE 329
#define DELETE 330
#define a_BEGIN 331
#define a_END 332
#define EXIT 333
#define ABORT 334
#define NEXT 335
#define NEXTFILE 336
#define RETURN 337
#define FUNCTION 338
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
typedef union{
CELL *cp ;
SYMTAB *stp ;
int  start ; /* code starting address as offset from code_base */
PF_CP  fp ;  /* ptr to a (print/printf) or (sub/gsub) function */
BI_REC *bip ; /* ptr to info about a builtin */
FBLOCK  *fbp  ; /* ptr to a function block */
ARG2_REC *arg2p ;
CA_REC   *ca_p  ;
int   ival ;
PTR   ptr ;
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */
extern YYSTYPE yylval;
