/*--------------------------------------------------*
 | number.h                                         |
 | Header file for numerical routines, part of the  |
 | Awka Library, Copyright 1999, Andrew Sumner.     |
 | This file is covered by the GNU General Public   |
 | License (GPL).                                   |
 *--------------------------------------------------*/

#ifndef _NUMBER_H
#define _NUMBER_H

static int
_awka_isnumber( char *s )
{
  register char *p = s, dot = 0, exp = 0;

  /* initial spaces */
  while (*p == ' ') p++;
  if (!*p) return FALSE;

  /* unary +- */
  if (*p == '+' || *p == '-') p++;
  if (*p == '.') { p++; dot = 1; }
  if (!isdigit(*p++)) return FALSE;

  start:
  /* numbers */
  while (isdigit(*p++));
  if (!*--p)
    return TRUE;
  if (*p == 'e')
  {
    /* exponential notation */
    if (exp) return FALSE;
    exp = dot = 1;
    if (*(p+1) == '-' || *(p+1) == '+') p++;
    if (!isdigit(*++p)) return FALSE;
    goto start;
  }
  if (*p == '.')
  {
    if (dot) return FALSE;
    dot = 1;
    p++;
    goto start;
  }
  if (*p == ' ')
  {
    while (*++p == ' ');
    if (!*p) return TRUE;
  }
  return FALSE;
}

#endif
