/********************************************
init.h
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/* $Log: init.h,v $
 * Revision 1.2  1995/06/18  19:42:18  mike
 * Remove some redundant declarations and add some prototypes
 *
 * Revision 1.1.1.1  1993/07/03  18:58:14  mike
 * move source to cvs
 *
 * Revision 5.1  1991/12/05  07:59:22  brennan
 * 1.1 pre-release
 *
*/

/* init.h  */


#ifndef  INIT_H
#define  INIT_H

#include "symtype.h"


#define  EXIT_ERR_NO	1
#define  EXIT_ERR_c	2
#define  EXIT_ERR_w	3
#define  EXIT_ERR_BAD   4
#define  EXIT_ERR_a	5
#define  EXIT_ERR_o	6
#define  EXIT_ERR_MISS  7
#define  EXIT_ERR_NOx   8
#define  EXIT_ERR_NOf   9

/* nodes to link file names for multiple
   -f option */

typedef struct pfile {
struct pfile *link ;
char *fname ;
} PFILE ;

extern PFILE *pfile_list ;

extern char *sprintf_buff, *sprintf_limit ;


void  PROTO( initialize, (int, char **) ) ;
void  PROTO( code_init, (void) ) ;
void  PROTO( code_cleanup, (void) ) ;
void  PROTO( compile_cleanup, (void) ) ;
void  PROTO( scan_init, ( char *) ) ;
void  PROTO( bi_vars_init, (void) ) ;
void  PROTO( bi_funct_init, (void) ) ;
void  PROTO( print_init, (void) ) ;
void  PROTO( kw_init, (void) ) ;
void  PROTO( field_init, (void) ) ;
void  PROTO( fpe_init, (void) ) ;
void  PROTO( load_environ, (ARRAY)) ;
void  PROTO( set_stderr, (void)) ;

static inline char *
quote_escape(char *s) {
  /* hard limited to 5 pairs of quotes */
  register char *p = s;
  int i = strlen(s);
  char *tstr = (char *) malloc(i+10); 

  strncpy(tstr, s, i);
  p = tstr;
  while (*p) {
    if (*p == '\\') {
      memmove(p+1, p, strlen(p));
      *(p++) = '\\';
    }
    if (*p++ == '"') {
      memmove(p, p-1, strlen(p-1));
      *((p++)-1) = '\\';
    }
  }
  return tstr;
}

#endif   /* INIT_H  */
