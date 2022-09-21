/*------------------------------------------------------------*
 | garbage.c                                                  |
 | copyright 1999,  Andrew Sumner (andrewsumner@yahoo.com)    |
 |                                                            |
 | This is a source file for the awka package, a translator   |
 | of the AWK programming language to ANSI C.                 |
 |                                                            |
 | This library is free software; you can redistribute it     |
 | and/or modify it under the terms of the GNU General        |
 | Public License (GPL).                                      |
 |                                                            |
 | This library is distributed in the hope that it will be    |
 | useful, but WITHOUT ANY WARRANTY; without even the implied |
 | warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR    |
 | PURPOSE.                                                   |
 *------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "libawka.h"
#include "mem.h"

#define _IN_GARBAGE_C
#include "garbage.h"

int _a_gc_alloc = 0;

/* _max_base_gc controls the size of the BEGIN, MAIN & END bins */
/* _max_fn_gc specifies the size for function bins */
/* both of these are set to values in the translated code */
int _max_base_gc, _max_fn_gc;

_a_VARBIN *
_awka_gc_initvarbin(int binsize)
{
  _a_VARBIN *base, *prev, *this;
  int i;

  malloc( &base, sizeof(_a_VARBIN));
  awka_varinit(base->var);
  base->var->temp = TRUE;
  base->binsize = binsize;
  prev = base;
  for (i=1; i<binsize; i++)
  {
    malloc( &this, sizeof(_a_VARBIN));
    awka_varinit(this->var);
    this->var->temp = TRUE;
    prev->next = this;
    prev = this;
  }

  prev->next = base;
  return base;
}

void
_awka_gc_killvarbin(_a_VARBIN *base)
{
  _a_VARBIN *this, *b;
  int i, binsize;
  
  if (!base) return;
  binsize = base->binsize;
  b = base->next;
  base->binsize = 0;
  for (i=1; i<binsize; i++)
  {
    if (!b) return;
    if (!b->next) break;
    if (b->var && b->var->allc)
    {
      b->var->ptr[0] = '\0';
      free(b->var->ptr);
      b->var->ptr = NULL;
      free(b->var);
      b->var = NULL;
    }
    this = b;
    b = b->next;
    this->binsize = 0;
    b->next = NULL;
    free(this);
  }
  free(base);
}

_a_VABIN *
_awka_gc_initvabin(int binsize)
{
  _a_VABIN *base, *prev, *this;
  int i;

  malloc( &base, sizeof(_a_VABIN));
  malloc( &base->va, sizeof(a_VARARG));
  base->va->used = 0;
  base->binsize = binsize;
  prev = base;
  for (i=1; i<binsize; i++)
  {
    malloc( &this, sizeof(_a_VABIN));
    malloc( &this->va, sizeof(a_VARARG));
    memset( this->va, 0, sizeof(a_VARARG));
    this->binsize = 0;
    prev->next = this;
    prev = this;
  }

  prev->next = base;
  return base;
}

void
_awka_gc_killvabin(_a_VABIN *base)
{
  _a_VABIN *this, *b;
  int i, binsize;
  
  if (!base) return;
  binsize = base->binsize;
  b = base->next;
  for (i=1; i<binsize; i++)
  {
    if (!b) return;
    free(b->va);
    this = b;
    b = b->next;
    free(this);
  }
  free(base->va);
  free(base);
  base = NULL;
}

_a_STRBIN *
_awka_gc_initstrbin(int binsize)
{
  _a_STRBIN *base, *this, *prev;
  int i;

  malloc( &base, sizeof(_a_STRBIN));
  base->slen = malloc( &base->str, 16 );
  base->binsize = binsize;
  prev = base;
  for (i=1; i<binsize; i++)
  {
    malloc( &this, sizeof(_a_STRBIN));
    this->slen = malloc( &this->str, 16 );
    prev->next = this;
    prev = this;
  }

  prev->next = base;
  return base;
}

void
_awka_gc_killstrbin(_a_STRBIN *base)
{
  _a_STRBIN *this;
  int i, binsize;
  
  if (!base) return;
  binsize = base->binsize;
  for (i=0; i<binsize; i++)
  {
    if (!base) return;
    if (base->str)
      free(base->str);
    this = base;
    base = base->next;
    free(this);
  }
}

void
_awka_gc_init()
{
  register int i;

  _a_gc_alloc = 10;
  if (_a_v_gc == NULL)
  {
    malloc( &_a_v_gc, _a_gc_alloc * sizeof(_a_VARGC));
    malloc( &_a_vro_gc, _a_gc_alloc * sizeof(_a_VARGC));
    malloc( &_a_va_gc, _a_gc_alloc * sizeof(_a_VAGC));
    malloc( &_a_c_gc, _a_gc_alloc * sizeof(_a_STRGC));

    for (i=0; i<_a_gc_alloc; i++)
    {
      _a_v_gc[i].bin = NULL;
      _a_vro_gc[i].bin = NULL;
      _a_va_gc[i].bin = NULL;
      _a_c_gc[i].bin = NULL;
    }
  }

  _a_v_gc[0].bin = _awka_gc_initvarbin(_max_base_gc);
  _a_vro_gc[0].bin = _awka_gc_initvarbin(_max_base_gc);
  _a_va_gc[0].bin = _awka_gc_initvabin(_max_base_gc);
  _a_c_gc[0].bin = _awka_gc_initstrbin(_max_base_gc);
}

void
_awka_gc_kill()
{
  register int i;
  
  for (i=0; i<_a_gc_alloc; i++)
  {
    _awka_gc_killvarbin(_a_v_gc[i].bin);
    _awka_gc_killvarbin(_a_vro_gc[i].bin);
    _awka_gc_killvabin(_a_va_gc[i].bin);
    _awka_gc_killstrbin(_a_c_gc[i].bin);
  }
  
  free(_a_v_gc);
  free(_a_vro_gc);
  free(_a_va_gc);
  free(_a_c_gc);
  
  _a_v_gc = NULL;
  _a_vro_gc = NULL;
  _a_va_gc = NULL;
  _a_c_gc = NULL;
  _a_gc_alloc = 0;
}

void
_awka_gc_deeper()
{
  register int i;

  _a_gc_depth++;

  if (_a_gc_depth >= _a_gc_alloc)
  {
    i = _a_gc_alloc;
    _a_gc_alloc = _a_gc_depth + 10;

    realloc( &_a_v_gc, _a_gc_alloc * sizeof(_a_VARGC));
    realloc( &_a_vro_gc, _a_gc_alloc * sizeof(_a_VARGC));
    realloc( &_a_va_gc, _a_gc_alloc * sizeof(_a_VAGC));
    realloc( &_a_c_gc, _a_gc_alloc * sizeof(_a_STRGC));

    for (; i<_a_gc_alloc; i++)
    {
      _a_v_gc[i].bin = NULL;
      _a_vro_gc[i].bin = NULL;
      _a_va_gc[i].bin = NULL;
      _a_c_gc[i].bin = NULL;
    }
  }

  i = _a_gc_depth;

  if (!_a_v_gc[i].bin)
  {
    _a_v_gc[i].bin = _awka_gc_initvarbin(_max_fn_gc);
    _a_vro_gc[i].bin = _awka_gc_initvarbin(_max_fn_gc);
    _a_va_gc[i].bin = _awka_gc_initvabin(_max_fn_gc);
    _a_c_gc[i].bin = _awka_gc_initstrbin(_max_fn_gc);
  }
}
