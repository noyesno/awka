
/********************************************
files.h
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*$Log: files.h,v $
 * Revision 1.3  1996/01/14  17:14:11  mike
 * flush_all_output()
 *
 * Revision 1.2  1994/12/11  22:14:13  mike
 * remove THINK_C #defines.  Not a political statement, just no indication
 * that anyone ever used it.
 *
 * Revision 1.1.1.1  1993/07/03  18:58:13  mike
 * move source to cvs
 *
 * Revision 5.2  1992/12/17  02:48:01  mike
 * 1.1.2d changes for DOS
 *
 * Revision 5.1  1991/12/05  07:59:18  brennan
 * 1.1 pre-release
 *
*/

#ifndef   FILES_H
#define   FILES_H

/* IO redirection types */
#define  COP_IN         (-7)
#define  COP_OUT        (-6)
#define  F_IN           (-5)
#define  PIPE_IN        (-4)
#define  PIPE_OUT       (-3)
#define  F_APPEND       (-2)
#define  F_TRUNC        (-1)
#define  IS_OUTPUT(type)  ((type)>=PIPE_OUT)

extern char *shell ; /* for pipes and system() */

PTR  PROTO(file_find, (STRING *, int)) ;
int  PROTO(file_close, (STRING *)) ;
int  PROTO(file_flush, (STRING *)) ;
void PROTO(flush_all_output, (void)) ;
PTR  PROTO(get_pipe, (char *, int, int *) ) ;
int  PROTO(wait_for, (int) ) ;
void  PROTO( close_out_pipes, (void) ) ;

#if  HAVE_FAKE_PIPES
void PROTO(close_fake_pipes, (void)) ;
int  PROTO(close_fake_outpipe, (char *,int)) ;
char *PROTO(tmp_file_name, (int, char*)) ;
#endif

#if MSDOS
int  PROTO(DOSexec, (char *)) ;
int  PROTO(binmode, (void)) ;
void PROTO(set_binmode, (int)) ;
void PROTO(enlarge_output_buffer, (FILE*)) ;
#endif


#endif
