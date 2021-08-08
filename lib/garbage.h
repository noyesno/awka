/*--------------------------------------------------*
 | garbage.h                                        |
 | Garbage collection routines, part of the Awka    |
 | Library, Copyright 1999, Andrew Sumner.          |
 | This file is covered by the GNU General Public   |
 | License (GPL).                                   |
 *--------------------------------------------------*/

#ifndef _GARBAGE_H
#define _GARBAGE_H

#define _a_BINSIZE 30

typedef struct _a_VARBIN _a_VARBIN;
struct _a_VARBIN {
  _a_VARBIN *next;
  a_VAR *var;
  int binsize;
};

typedef struct {
  _a_VARBIN *bin;
} _a_VARGC;

typedef struct _a_VABIN _a_VABIN;
struct _a_VABIN {
  _a_VABIN *next;
  a_VARARG *va;
  int binsize;
};

typedef struct {
  _a_VABIN *bin;
} _a_VAGC;

typedef struct _a_STRBIN _a_STRBIN;
struct _a_STRBIN {
  _a_STRBIN *next;
  char *str;
  int slen;
  int binsize;
};

typedef struct {
  _a_STRBIN *bin;
} _a_STRGC;


#ifndef _IN_GARBAGE_C
extern _a_VARGC *_a_v_gc;
extern _a_VARGC *_a_vro_gc;
extern _a_VAGC *_a_va_gc;
extern _a_STRGC *_a_c_gc;
extern unsigned int _a_gc_depth;
#else
_a_VARGC *_a_v_gc=NULL;
_a_VARGC *_a_vro_gc=NULL;
_a_VAGC *_a_va_gc=NULL;
_a_STRGC *_a_c_gc=NULL;
unsigned int _a_gc_depth = 0;
#endif

void _awka_gc_kill();
void _awka_gc_init();
void _awka_gc_deeper();

/*
 * Garbage collection macros
 */

#define _awka_tmpvar(p) \
  (p) = _a_v_gc[_a_gc_depth].bin->var; \
  if ((p)->type == a_VARREG) { \
    (p)->type = a_VARNUL; \
    if ((p)->ptr) free((p)); \
    (p)->ptr = NULL; (p)->allc = 0; \
  } \
  _a_v_gc[_a_gc_depth].bin = _a_v_gc[_a_gc_depth].bin->next

#define _awka_tmpvar_ro(p) \
  (p) = _a_vro_gc[_a_gc_depth].bin->var; \
  if ((p)->type == a_VARREG) { \
    (p)->type = a_VARNUL; \
    if ((p)->ptr) free((p)); \
    (p)->ptr = NULL; (p)->allc = 0; \
  } \
  _a_vro_gc[_a_gc_depth].bin = _a_vro_gc[_a_gc_depth].bin->next

#define _awka_tmpvar_a(p) \
  (p) = _a_va_gc[_a_gc_depth].bin->va; \
  _a_va_gc[_a_gc_depth].bin = _a_va_gc[_a_gc_depth].bin->next

#define _awka_tmpvar_c(p, len) \
  if (_a_c_gc[_a_gc_depth].bin->slen < (len)) \
    _a_c_gc[_a_gc_depth].bin->slen = realloc( &_a_c_gc[_a_gc_depth].bin->str, (len)); \
  (p) = _a_c_gc[_a_gc_depth].bin->str; \
  _a_c_gc[_a_gc_depth].bin = _a_c_gc[_a_gc_depth].bin->next

static INLINE char *
_awka_tmpstr(int len)
{
  register char *p;

  len = len + (32 - (len % 32));
  _awka_tmpvar_c(p, len);
  return p;
}

#endif
