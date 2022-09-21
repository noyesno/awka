/*--------------------------------------------------*
 | array.h                                          |
 | Header file for array.c, part of the Awka        |
 | Library, Copyright 1999, Andrew Sumner.          |
 | This file is covered by the GNU General Public   |
 | License (GPL).                                   |
 *--------------------------------------------------*/

#ifndef _ARRAY_H
#define _ARRAY_H

#define GROWSZ  16

#define a_ARR_TYPE_NULL  0
#define a_ARR_TYPE_SPLIT 1
#define a_ARR_TYPE_HSH   2

#define a_ARR_CREATE 1
#define a_ARR_QUERY  2
#define a_ARR_DELETE 3

typedef struct _hshnode a_HSHNode;
struct _hshnode {
  a_HSHNode *next;
  char *key;
  a_VAR *var;
  unsigned int hval;
  char type;
  char shadow;
};

typedef struct {
  a_HSHNode **node;
  int type;
  int base;
  int nodeno;
  int id;
} a_List;

typedef struct {
  a_List *list;
  int allc;
  int used;
} a_ListHdr;

typedef struct {
  char *str;
  double *delem;
  char **pelem;
  int *lelem;
  char *dset;
  int alloc;
  int elem;
  int dalloc;
} _a_Subscript;

typedef struct {
  a_HSHNode **slot;
  _a_Subscript *subscript;
  a_HSHNode *last;
  char *splitstr;
  int nodeno; 
  int nodeallc;  /* used with split arrays to monitor allocated size */
  int splitallc;
  int base;      /* for split arrays */
  unsigned int hashmask;   /* current number of hash slots */
  char type;
  char flag;
  char fill_1;
  char fill_2;
} _a_HSHarray;

void         awka_arraycreate( a_VAR *var, char type );
void         awka_arrayclear( a_VAR *var );
a_VAR *      awka_arraysearch1( a_VAR *v, a_VAR *element, char create, int set );
a_VAR *      awka_arraysearch( a_VAR *v, a_VARARG *va, char create );
double       awka_arraysplitstr( char *str, a_VAR *v, a_VAR *fs, int max, char );
int          awka_arrayloop( a_ListHdr *ah, a_VAR *v, char );
int          awka_arraynext( a_VAR *v, a_ListHdr *, int );
a_VAR *      awka_arraynextget( a_ListHdr *, int );
void         awka_alistfree( a_ListHdr * );
void         awka_alistfreeall( a_ListHdr * );
a_VAR *      awka_doln(int, int);
a_VAR *      _awka_dol0(int);
double       awka_asort( a_VAR *src, a_VAR *dst );
double       awka_arraysplitpat( char *str, a_VAR *v, a_VAR *fpat, int max );

#ifndef _ARRAY_C
extern char _awka_setdoln;
extern int  _awka_dol0_len;
extern char _dol0_only;
#endif

extern int _split_req;
extern int _split_max;
static inline void awka_split_req(int v) { _split_req = v; }
static inline void awka_split_max(int v) { _split_max = v; }

static INLINE a_VAR *
awka_NFget()
{
  /* noyesno: delayed split from awka_setNF() */
  if (_rebuildn)
  {
    awka_setd(a_bivar[a_NF]) = awka_arraysplitstr(awka_gets1(a_bivar[a_DOL0]), a_bivar[a_DOLN], NULL, _split_max, TRUE);
    _rebuildn = FALSE;
  }
  return a_bivar[a_NF];
}

static INLINE a_VAR *
awka_NFset()
{
  awka_NFget();
  _rebuild0_now = _rebuild0 = _awka_setdoln = TRUE;
  _rebuildn = FALSE;
  return a_bivar[a_NF];
}

static INLINE int
awka_alength( a_VAR *v )
{
  if (v->type != a_VARARR)
    awka_error("Runtime Error: Scalar used as Array when passed to alength()\n");
  if (!v->ptr) return 0;
  return ((_a_HSHarray *) v->ptr)->nodeno;
}

static INLINE a_VAR *
awka_dol0(int set)
{
  if (_dol0_only)
    return a_bivar[a_DOL0];
  else
    return _awka_dol0(set);
}

static INLINE a_VAR *
awka_getarrayval( a_VAR *v, a_VAR *key)
{
  return awka_arraysearch1( v, key, a_ARR_CREATE, 0 );
}
#endif
