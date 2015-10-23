/*-------------------------------------------------*
 | mem.h                                           |
 | Memory-based defines & functions, part of the   |
 | Awka Library, Copyright 1999, Andrew Sumner.    |
 | This file is covered by the GNU Library         |
 | General Public License - see file LGPL for more |
 | details.                                        |
 *-------------------------------------------------*/

#ifndef _MEM_H
#define _MEM_H

#include <stdarg.h>

void awka_error(char *fmt, ...);

static INLINE void *
awka_malloc(size_t size, char *file, int line)
{
  void *ptr;
  if (!(ptr = malloc(size)))
    awka_error("Memory Error - Failed to allocate %d bytes, file %s line %d.\n",size,file,line);
  return ptr;
}

static INLINE void *
awka_realloc(void *oldptr, size_t size, char *file, int line)
{
  void *ptr;
  if (!(ptr = realloc(oldptr, size)))
    awka_error("Memory Error - Failed to reallocate ptr %p to %d bytes, file %s line %d.\n",oldptr,size,file,line);
  return ptr;
}

static INLINE void 
awka_free(void *ptr, char *file, int line)
{
  if (!ptr)
  {
    awka_error("Memory Error - Free of Null ptr, file %s, line %d.\n",file,line);
    return;
  }
  free(ptr);
}

#define malloc(size)       awka_malloc(size, __FILE__, __LINE__)
#define realloc(ptr, size) awka_realloc(ptr, size, __FILE__, __LINE__)
#define free(ptr)          awka_free(ptr, __FILE__, __LINE__)

#endif
