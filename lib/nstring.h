/*--------------------------------------------------*
 | nstring.h                                        |
 | Header file for nstring.c, string with length.   |
 | This file is covered by the BSD License.         |
 *--------------------------------------------------*/

#ifndef _NSTRING_H
#define _NSTRING_H

// Do glob match of string. Return 1 on match, 0 othewise.
int nstring_match(
  const char *pattern, int pattern_size,
  const char *string, int string_size);

#endif

