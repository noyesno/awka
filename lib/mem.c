/*------------------------------------------------------------*
 | mem.c                                                      |
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
#include <string.h>

#include "error.h"

size_t awka_malloc(void **ptr, size_t size, char *file, int line);
size_t awka_realloc(void **oldptr, size_t size, char *file, int line);
void   awka_free(void *ptr, char *file, int line);

void   awka_mprotect(char *, size_t);
void   awka_mfree(char *, char *file, int line);
int    awka_mcheck(char *, char *file, int line);
void   awka_mtest(char *);
void   awka_mtestall();

#define A_PROT_SIZE 64
char mem_char = 0xff, free_char = 0xfe, allc_char = 0xfd;

typedef struct MemAllc MemAllc;
struct MemAllc {
  MemAllc *next;
  char *ptr;
  size_t size;
};

MemAllc *mem_base = NULL;

void 
awka_mprotect(char *ptr, size_t size)
{
  MemAllc *pmem;

  pmem = (MemAllc *) malloc(sizeof(MemAllc));
  pmem->next = mem_base;
  mem_base = pmem;

  pmem->ptr = ptr;
  pmem->size = size;
  size -= A_PROT_SIZE * 2;

  memset(ptr, mem_char, A_PROT_SIZE);
  ptr += A_PROT_SIZE;
  memset(ptr, allc_char, size);
  ptr += size;
  memset(ptr, mem_char, A_PROT_SIZE); 
}

void
awka_mtestall()
{
  MemAllc *pmem;
  char *sptr;
  register int i;

  pmem = mem_base;
  while (pmem)
  {
    sptr = pmem->ptr;
    for (i=0; i<A_PROT_SIZE; i++)
      if (*sptr++ != mem_char)
        awka_error("Mem_Debug: Corruption in leading block for ptr %p, offset %d\n",pmem->ptr, i);
    sptr = pmem->ptr + (pmem->size - A_PROT_SIZE);
    for (i=0; i<A_PROT_SIZE; i++)
      if (*sptr++ != mem_char)
        awka_error("Mem_Debug: Corruption in trailing block for ptr %p, offset %d\n",pmem->ptr, i);
    pmem = pmem->next;
  }

}

void
awka_mtest(char *ptr)
{
  MemAllc *pmem;
  char *sptr;
  register int i;

  pmem = mem_base;
  ptr -= A_PROT_SIZE;
  while (pmem)
  {
    if (pmem->ptr == ptr) break;
    pmem = pmem->next;
  }

  if (!pmem)
    awka_error("Mem_Debug: (test): Internal error in allocation structure looking for ptr %p.\n", ptr);

  sptr = ptr;
  for (i=0; i<A_PROT_SIZE; i++)
    if (*sptr++ != mem_char)
      awka_error("Mem_Debug: Corruption in leading block for ptr %p, offset %d\n", ptr, i);
  sptr = ptr + (pmem->size - A_PROT_SIZE);
  for (i=0; i<A_PROT_SIZE; i++)
    if (*sptr++ != mem_char)
      awka_error("Mem_Debug: Corruption in trailing block for ptr %p, offset %d\n", ptr, i);
}

void
awka_mfree(char *ptr, char *file, int line)
{
  MemAllc *pmem, *prevmem = NULL;
  char *sptr;
  register int i;
  int size;

  pmem = mem_base;
  while (pmem)
  {
    if (pmem->ptr == ptr) break;
    prevmem = pmem;
    pmem = pmem->next;
  }

  if (!pmem)
    awka_error("Mem_Debug: (free): Internal error in allocation structure looking for ptr %p # %s:%d\n", ptr, file, line);

  size = pmem->size;

  sptr = pmem->ptr;
  for (i=0; i<A_PROT_SIZE; i++)
    if (*sptr++ != mem_char)
      awka_error("Mem_Debug: Corruption in leading block for ptr %p, offset %d # %s:%d\n", ptr, i, file, line);
  sptr = pmem->ptr + (pmem->size - A_PROT_SIZE);
  for (i=0; i<A_PROT_SIZE; i++)
    if (*sptr++ != mem_char)
      awka_error("Mem_Debug: Corruption in trailing block for ptr %p, offset %d # %s:%d\n", ptr, i, file, line);

  memset(ptr, free_char, pmem->size); 

  if (prevmem)
    prevmem->next = pmem->next;
  else
    mem_base = pmem->next;
  free(pmem);
}

int
awka_mcheck(char *ptr, char *file, int line)
{
  MemAllc *pmem, *prevmem = NULL;
  char *sptr;
  register int i;
  int size;

  pmem = mem_base;
  while (pmem)
  {
    if (pmem->ptr == ptr) break;
    prevmem = pmem;
    pmem = pmem->next;
  }

  if (!pmem)
    awka_error("Mem_Debug: (free): Internal error in allocation structure looking for ptr %p.\n",ptr);

  if (prevmem)
  {
    prevmem->next = pmem->next;
    pmem->next = mem_base;
    mem_base = pmem;
  }

  return pmem->size;
}

/*
 * The MEM_DEBUG versions of awka_malloc, awka_realloc and awka_free
 * follow...
 */

size_t
awka_malloc(void **ptr, size_t size, char *file, int line)
{
  size += A_PROT_SIZE * 2;

  if (!(*ptr = malloc(size)))
    awka_error("Memory Error - Failed to allocate %d bytes, file %s line %d.\n", size, file, line);

/**
fprintf(stderr,"m %p %s %d %u\n",*ptr,file,line,size); 
**/

  awka_mprotect((char *) *ptr, size);
  *ptr += A_PROT_SIZE;
  size -= A_PROT_SIZE * 2;

  return size;
}

size_t
awka_realloc(void **oldptr, size_t size, char *file, int line)
{
  void *ptr = *oldptr;
  size_t oldsize;

  if (!ptr)
    return awka_malloc(oldptr, size, file, line);

  *oldptr -= A_PROT_SIZE;
  size = awka_malloc(&ptr, size, file, line);
  ptr -= A_PROT_SIZE;
  size += A_PROT_SIZE * 2;

/**
fprintf(stderr,"f %p %s %d\n",*oldptr,file,line);
fprintf(stderr,"m %p %s %d %u\n",ptr,file,line,size);
/**/

  oldsize = awka_mcheck((char *) *oldptr, file, line);
  if (size < oldsize) oldsize = size;
  memcpy(ptr, *oldptr, oldsize);
  awka_mfree((char *) *oldptr, file, line);
  free(*oldptr);
  ptr += A_PROT_SIZE;
  size -= A_PROT_SIZE * 2;

  *oldptr = ptr;

  return size;
}

void 
awka_free(void *ptr, char *file, int line)
{
  if (!ptr)
  {
    awka_error("Memory Error - Free of Null ptr, file %s, line %d.\n", file, line);
    return;
  }

  ptr -= A_PROT_SIZE;
  awka_mfree((char *) ptr, file, line);

/**
fprintf(stderr,"f %p %s %d\n",ptr,file,line); 
/**/

  free(ptr);
}

