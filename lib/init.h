/*--------------------------------------------------*
 | init.h                                           |
 | Header file for init.c, part of the Awka         |
 | Library, Copyright 1999, Andrew Sumner.          |
 | This file is covered by the GNU General Public   |
 | License (GPL).                                   |
 *--------------------------------------------------*/

#ifndef _INIT_H
#define _INIT_H

void awka_init(int argc, char *argv[], char *, char *);
void _awka_kill_ivar();
void _awka_kill_gvar();
void _awka_kill_fn();

#if defined(__cplusplus)
extern "C" {
#endif

struct awka_fn_struct {
  char *name;
  a_VAR * (*fn)( a_VARARG *);
};

#if defined(__cplusplus)
}
#endif

struct gvar_struct {
  char *name;
  a_VAR *var;
};

void     awka_initgvar(int, char *, a_VAR *);
#ifndef _INIT_C
extern char **awka_filein;
extern int awka_filein_no;
extern struct awka_fn_struct *_awkafn;
#endif

static INLINE char
awka_setNF()
{
  _rebuildn = TRUE;
  return 1;
}


#endif
