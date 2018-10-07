/*--------------------------------------------------*
 | mem.h                                            |
 | Memory-based defines & functions, part of the    |
 | Awka Library, Copyright 1999, Andrew Sumner.     |
 | This file is covered by the GNU General Public   |
 | License (GPL).                                   |
 *--------------------------------------------------*/

#ifndef _MEM_H
#define _MEM_H

#ifndef _ERROR_H
void awka_error( char *fmt, ... );
#endif

extern int _print_mem;

#ifdef MEM_DEBUG
#define A_PROT_SIZE 64
size_t awka_malloc(void **ptr, size_t size, char *file, int line);
size_t awka_realloc(void **oldptr, size_t size, char *file, int line);
void awka_free(void *ptr, char *file, int line);
#else

static size_t
awka_malloc(void **ptr, size_t size, char *file, int line)
{
  size = size + (16 - (size % 16));

  if (!(*ptr = malloc(size)))
    awka_error("Memory Error - Failed to allocate %d bytes, file %s line %d.\n",size,file,line);

  /*
fprintf(stderr,"m %p %s %d %u\n",*ptr,file,line,size); 
*/

  return size;
}

static size_t
awka_realloc(void **oldptr, size_t size, char *file, int line)
{
  void *ptr = *oldptr;

  size = size + (16 - (size % 16));

  if (!ptr)
    return awka_malloc(oldptr, size, file, line);

  if (!(ptr = realloc(ptr, size)))
    awka_error("Memory Error - Failed to reallocate ptr %p to %d bytes, file %s line %d.\n",*oldptr,size,file,line);

  /*
if (ptr != *oldptr)
{
  fprintf(stderr,"f %p %s %d\n",*oldptr,file,line);
  fprintf(stderr,"m %p %s %d %u\n",ptr,file,line,size);
}
*/

  *oldptr = ptr;

  return size;
}

static void 
awka_free(void *ptr, char *file, int line)
{
  if (!ptr)
  {
    awka_error("Memory Error - Free of Null ptr, file %s, line %d.\n",file,line);
    return;
  }

  /*
fprintf(stderr,"f %p %s %d\n",ptr,file,line); 
*/

  free(ptr);
}

#endif /* MEM_DEBUG */

#define malloc(ptr, size)  awka_malloc((void **) ptr, size, __FILE__, __LINE__)
#define realloc(ptr, size) awka_realloc((void **) ptr, size, __FILE__, __LINE__)
#define free(ptr)          awka_free(ptr, __FILE__, __LINE__)

#endif
