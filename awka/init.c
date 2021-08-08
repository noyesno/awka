/*------------------------------------------------------------*
 | init.c                                                     |
 | copyright 1999,  Andrew Sumner                             |
 | copyright 1991,  Michael D. Brennan                        |
 |                                                            |
 | This is a source file for the awka package, a translator   |
 | of the AWK programming language to ANSI C.                 |
 |                                                            |
 | The file is a modified version of init.c from              |
 | Mawk, an implementation of the AWK processing language,    |
 | distributed by Michael Brennan under the GPL.              |
 |                                                            |
 | This program is free software; you can redistribute it     |
 | and/or modify it under the terms of the GNU General Public |
 | License as published by the Free Software Foundation;      |
 | either version 2 of the License, or any later version.     |
 |                                                            |
 | This program is distributed in the hope that it will be    |
 | useful, but WITHOUT ANY WARRANTY; without even the implied |
 | warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR    |
 | PURPOSE.  See the GNU General Public License for more      |
 | details.                                                   |
 |                                                            |
 | You should have received a copy of the GNU General Public  |
 | License along with this program; if not, write to the      |
 | Free Software Foundation, Inc., 675 Mass Ave, Cambridge,   |
 | MA 02139, USA.                                             |
 *-----------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* init.c */
#include "awka.h"
#include "msg.h"
#include "code.h"
#include "memory.h"
#include "symtype.h"
#include "init.h"
#include "bi_vars.h"
#include "field.h"

static void PROTO(_process_cmdline, (int, char **)) ;
static void PROTO(set_ARGV, (int, char **, int)) ;
static void PROTO(_bad_option, (char *)) ;
static void PROTO(_no_program, (void)) ;

extern void PROTO(print_version, (void)) ;
extern int  PROTO(is_cmdline_assign, (char *)) ;

extern int  awka_main ;
extern char *awka_main_func ;
extern char *int_argv ;

static char  *quote_escape(char *) ;

PFILE dummy ;                 /* starts linked list of filenames */

char  *progname, *uoutfile = NULL ;
short interactive_flag = 0 ;
char  awka_exe = FALSE, awka_comp = FALSE ;
char  awka_tmp = FALSE ;
char  awka_comp_debug = FALSE, awka_comp_static = FALSE ;
int   exe_argc = 0 ;
char  **exe_argv = NULL ;
char  **incfile = NULL, **incdir = NULL, **libfile = NULL, **libdir = NULL ;
int   incf_allc = 0, incf_used = 0, incd_allc = 0, incd_used = 0, libf_allc = 0, libf_used = 0, libd_allc = 0, libd_used = 0 ;
int   dump = 0, warning_msg = 0 ;
char  *awk_input_files = NULL ;      /* used for version reporting */

#ifndef         SET_PROGNAME
#define         SET_PROGNAME() \
   {char *p = strrchr(argv[0],'/') ;\
    progname = p ? p+1 : argv[0] ; }
#endif

void
initialize(argc, argv)
int argc ; char **argv ;
{

   SET_PROGNAME() ;

   bi_vars_init() ;                 /* load the builtin variables */
   bi_funct_init() ;                 /* load the builtin functions */
   kw_init() ;                         /* load the keywords */
   field_init() ;

   _process_cmdline(argc, argv) ;

   code_init() ;
}

int dump_code_flag ;                 /* if on dump internal code */
short posix_space_flag ;

#ifdef         DEBUG
int dump_RE ;                         /* if on dump compiled REs  */
#endif

static void
_process_help_message()
{
  fprintf(stderr,"\nusage: awka [-c fn] [-X] [-x -t] [-w flags] [-f filename] program_string [--] [exe-args]\n") ;
  fprintf(stderr,"       awka [-h] [-v]\n\n") ;
  fprintf(stderr,"    -f      AWK Program file(s)\n") ;
  fprintf(stderr,"    -c fn   Awka will generate a 'fn' function rather\n") ;
  fprintf(stderr,"            than a main function\n") ;
  fprintf(stderr,"    -x      Translates, compiles and executes the program,\n") ;
#if defined(__CYGWIN32__) || defined(__CYGWIN__)
  fprintf(stderr,"            and will produce the awka-app.exe file\n") ;
#else
  fprintf(stderr,"            and will produce the awka-app.out file\n") ;
#endif
  fprintf(stderr,"            in the current directory.\n") ;
  fprintf(stderr,"    -t      If -x is specified, the temporary C and executable\n") ;
  fprintf(stderr,"            files will be deleted following execution.\n") ;
  fprintf(stderr,"    -X      Translates, compiles (not execute) the program, and leaves\n") ;
#if defined(__CYGWIN32__) || defined(__CYGWIN__)
  fprintf(stderr,"            the 'awka-app.exe' executable file in the current directory.\n") ;
#else
  fprintf(stderr,"            the 'awka-app.out' executable file in the current directory.\n") ;
#endif
  fprintf(stderr,"    -o fil  If -x used, this will create an executable called 'fil'\n") ;
#if defined(__CYGWIN32__) || defined(__CYGWIN__)
  fprintf(stderr,"            instead of the default 'awka_out.exe'\n") ;
#else
  fprintf(stderr,"            instead of the default 'awka.out'\n") ;
  fprintf(stderr,"\n") ;
  fprintf(stderr,"    -I dir  Compiler include directory.\n") ;
  fprintf(stderr,"    -i fil  Compiler include file.\n") ;
  fprintf(stderr,"    -L dir  Compiler link library directory.\n") ;
  fprintf(stderr,"    -l fil  Compiler link library file.\n") ;
  fprintf(stderr,"    -s      Statically link libawka (default is dynamic)\n") ;
  //fprintf(stderr,"    -D      Dump extra information then exit\n") ;
  fprintf(stderr,"\n") ;
#endif
  fprintf(stderr,"    --      If -x specified, all arguments after this point\n") ;
  fprintf(stderr,"            will be passed to the compiled executable.\n") ;
  fprintf(stderr,"    -a str  The executable command-line arguments in 'str' will\n") ;
  fprintf(stderr,"            be hard-coded in the translated C output.\n") ;
  fprintf(stderr,"    -w flg  Prints various warnings to stderr, useful in debugging\n") ;
  fprintf(stderr,"            large, complex programs.  The argument can contain the\n") ;
  fprintf(stderr,"            following characters:-\n") ;
  fprintf(stderr,"       'a'  print a list of all global variables & their usage\n") ;
  fprintf(stderr,"       'b'  warn about variables set but not referenced.\n") ;
  fprintf(stderr,"       'c'  warn about variables referenced but not set.\n") ;
  fprintf(stderr,"       'd'  report global vars used in any function.\n") ;
  fprintf(stderr,"       'e'  report global vars used in just one function.\n") ;
  fprintf(stderr,"       'f'  require global variables to be listed in a VDECL comment\n") ;
  fprintf(stderr,"       'g'  warn about assignments used as truth expressions\n") ;
  fprintf(stderr,"    -v      Prints version information\n\n") ;
  exit(0) ;
}


static void
_bad_option(
      char *s)
{
   errmsg(0, "not an option: %s", s) ;
   exit(EXIT_ERR_BAD) ;
}

static void
_no_program()
{
   exit(EXIT_ERR_NO) ;
}

static void
_add_input_filename(
      char *fn)
{
   if (!*awk_input_files)
      sprintf(awk_input_files, "%s", fn) ;
   else
      sprintf(awk_input_files, "%s %s", awk_input_files, fn) ;
}

static void
_add_pfile(
      char *fn)
{
  register PFILE *pf, *tpf ;

   pf = (PFILE *) ZMALLOC(PFILE) ;
   pf->fname = fn ;
   pf->link = (PFILE *) 0 ;

   tpf = &dummy ;
   while (tpf->link) {
     tpf = tpf->link ;
   }
   tpf->link = pf ;
}

static void
_process_scan_init(
	char *awk_input_files,
	char *awk_input_text)
{
   char *p = NULL ;

  /* scan each file and inline awk text */
   pfile_list = dummy.link ;

   if (*awk_input_text)
   { /* process any inline scripts */

      /* add tp filename list for awka version reporting */
      _add_input_filename(p = quote_escape(awk_input_text)) ;
      free(p) ;
      scan_init(awk_input_text) ;

   } else {
      /* files only */
      pfile_name = NULL ;
      scan_init((char *) 0) ;
   }
}

/*
 * _process_cmdline_dsh
 *
 * handles choices once a - has been identified
 *
 * cases with no break exit the program
 *
 */
static void
_process_cmdline_dash(
      int *argc, char **argv,
      int *i, int *j, int *nextarg,
      char *optarg,
      char *awk_input_text)
{
   char *p = NULL ;

   switch (argv[*i][*j])
   {
      case 'v':
         print_version() ;

      case 'f':
         /* first file goes in pfile_name ; any more go
            on a list */
	 optarg = argv[++(*i)] ;
         if (!pfile_name)
            pfile_name = optarg ;

         /* process the files after all names are collected */
         _add_input_filename(optarg) ;
	 _add_pfile(optarg) ;
         break ;

      case 'c':
         awka_main = 1 ;
         awka_exe = awka_comp = awka_tmp = FALSE ;

         if (argv[*i+1][0] == '-')
         {
           fprintf(stderr,"Command Line Error: Expecting function-name after -c argument.\n") ;
           exit(EXIT_ERR_c) ;
         }

         awka_main_func = argv[*i+1] ;
         break ;

      case 'w':    /* warning messages */
         if (argv[++(*i)][0] == '-')
         {
           fprintf(stderr,"Command Line Error: Expecting message flags 'abcdefg' after -w argument.\n") ;
           exit(EXIT_ERR_w) ;
         }

         for (p=argv[*i]; *p; p++)
         {
           switch (*p)
           {
             case 'a':
               warning_msg |= MSG_LIST; break ;
             case 'b':
               warning_msg |= MSG_SETnREF; break ;
             case 'c':
               warning_msg |= MSG_REFnSET; break ;
             case 'd':
               warning_msg |= MSG_GLOBinFUNC; break ;
             case 'e':
               warning_msg |= MSG_GLOBoinFUNC; break ;
             case 'f':
               warning_msg |= MSG_VARDECLARE; break ;
             case 'g':
               warning_msg |= MSG_ASGNasTRUTH; break ;
           }
         }
         break ;

      case 'i':  /* include file */
         if (!incf_allc)
         {
           incf_allc = 8 ;
           incfile = (char **) malloc( incf_allc * sizeof(char *) ) ;
         }
         else if (incf_used == incf_allc)
         {
           incf_allc *= 2 ;
           incfile = (char **) realloc( incfile, incf_allc * sizeof(char *) ) ;
         }

         incfile[incf_used] = (char *) malloc( strlen(argv[++(*i)])+1 ) ;
         strcpy(incfile[incf_used++], argv[*i]) ;
         break ;

      case 'I':  /* include directory */
         if (!incd_allc)
         {
           incd_allc = 8 ;
           incdir = (char **) malloc( incd_allc * sizeof(char *) ) ;
         }
         else if (incd_used == incd_allc)
         {
           incd_allc *= 2 ;
           incdir = (char **) realloc( incdir, incd_allc * sizeof(char *) ) ;
         }

         incdir[incd_used] = (char *) malloc( strlen(argv[++(*i)])+1 ) ;
         strcpy(incdir[incd_used++], argv[*i]) ;
         break ;

      case 'l':  /* library */
         if (!libf_allc)
         {
           libf_allc = 8 ;
           libfile = (char **) malloc( libf_allc * sizeof(char *) ) ;
         }
         else if (libf_used == libf_allc)
         {
           libf_allc *= 2 ;
           libfile = (char **) realloc( libfile, libf_allc * sizeof(char *) ) ;
         }

         libfile[libf_used] = (char *) malloc( strlen(argv[++(*i)])+1 ) ;
         strcpy(libfile[libf_used++], argv[*i]) ;
         break ;

      case 'L':  /* library directory */
         if (!libd_allc)
         {
           libd_allc = 8 ;
           libdir = (char **) malloc( libd_allc * sizeof(char *) ) ;
         }
         else if (libd_used == libd_allc)
         {
           libd_allc *= 2 ;
           libdir = (char **) realloc( libdir, libd_allc * sizeof(char *) ) ;
         }

         libdir[libd_used] = (char *) malloc( strlen(argv[++(*i)])+1 ) ;
         strcpy(libdir[libd_used++], argv[*i]) ;
         break ;

      case 's': // -s | -static
         awka_comp_static = TRUE ;
         break ;

      case 'X':
         awka_comp = TRUE ;
         awka_exe  = FALSE ;
         awka_main = FALSE ;
         break ;

      case 'x':
         awka_comp = FALSE ;
         awka_exe  = TRUE ;
         awka_main = FALSE ;
         break ;

      case 't':
         awka_tmp = TRUE ;
         break ;

      case 'D':
         dump = TRUE ;
         break ;

      case 'a':
         if (++(*i) >= *argc)
         {
           fprintf(stderr,"Command Line Error: Expecting filename after -a argument.\n") ;
           exit(EXIT_ERR_a) ;
         }

         int_argv = (char *) malloc(strlen(argv[*i])+1) ;
         strcpy(int_argv, argv[*i]) ;
         break ;

      case 'o':
         if (++(*i) >= *argc)
         {
           fprintf(stderr,"Command Line Error: Expecting filename after -o argument.\n") ;
           exit(EXIT_ERR_o) ;
         }

         uoutfile = (char *) malloc(strlen(argv[*i])+1) ;
         strcpy(uoutfile, argv[*i]) ;
         break ;

      case 'u':
      case 'h':
	 _process_help_message() ;

      default:
         _bad_option(argv[*i]) ;

   } /* switch */

   *nextarg = ++(*i) ;
}

static void
_process_cmdline(argc, argv)
   int argc ;
   char **argv ;
{
   int i, j, nextarg ;
   char *optarg, *p, *tptr, *awk_input_text ;
   dummy.link = (PFILE *) 0 ;

   awk_input_files = (char *) malloc(2048) ;
   awk_input_files[0] = '\0' ;
   awk_input_text = (char *) malloc(1024) ;
   awk_input_text[0] = '\0' ;

   for (i = 1; i < argc ; i = nextarg)
   {
      if (argv[i][0] == '-' && argv[i][1] == 0)        /* -  alone (read from stdin) */
      {
         //if (!pfile_name) _no_program() ;
	 _add_input_filename(argv[i]) ;
	 _add_pfile(argv[i]) ;

	 nextarg = ++i ;
	 continue ;
      }
      if (argv[i][0] != '-')      /* inline script */
      {
	 /* if ends in ".awk" then is missing the -f */
	 if (strcmp(argv[i] + strlen(argv[i]) - 4, ".awk") == 0)
	 {
           errmsg(0, "expected a quoted inline script, or  -f %s.", argv[i]) ;
	   exit(EXIT_ERR_NOf) ;
	 }

	 if (!*awk_input_text)
	   sprintf(awk_input_text, "%s", argv[i]) ;
	 else
	   sprintf(awk_input_text, "%s %s", awk_input_text, argv[i]) ;

         nextarg = i + 1 ;
	 if (i >= argc)
           goto no_more_opts ;

	 continue ;
      }
      /* safe to look at argv[i][2] */

      j = 1 ;
      if (argv[i][j] == '-' && argv[i][0] == '-') j++ ;

      if (argv[i][j+1] == 0)
      {
         if (i == argc - 1 && argv[i][j] != '-' &&
             argv[i][j] != 'u' && argv[i][j] != 'v' &&
             argv[i][j] != 'h')
         {
            /* there are no more arguments - does this option need one? */
            if (strchr("IiLlaocfxwt-", argv[i][j]))
            {
               errmsg(0, "option %s lacks argument", argv[i]) ;
               exit(EXIT_ERR_MISS) ;
            }
            _bad_option(argv[i]) ;
         }
         else if (strchr("-DxtXs", argv[i][j]))
         {
           nextarg = i + 1 ;
         }
         else
         {
           optarg = argv[i + 1] ;
           nextarg = i + 2 ;
         }
      }
      else if (!strcmp(argv[i], "--"))
      {
        if (!awka_exe)
        {
          fprintf(stderr,"Need a \"-x\" argument for \"--\" to make any sense.\n") ;
          exit(EXIT_ERR_NOx) ;
        }
        if (i == 0)  _no_program() ;

        goto exe_opts ;
      }
      else  /* argument glued to option mb */
      {
         if (argv[i][j-1] == '-')
         {
           optarg = &argv[i][j] ;

           nextarg = i + 1 ;

           if (argv[i][j] != 'h' && argv[i][j] != 'v' && argv[i][j] != 'u')
             j++ ;
         }
         else
         {
           optarg = &argv[i][j] ;

           nextarg = i + 1 ;
         }
      }

      _process_cmdline_dash( &argc, argv, &i, &j, &nextarg,
           optarg, awk_input_text) ;

   }

 no_more_opts:
   _process_scan_init(awk_input_files, awk_input_text) ;
   i++ ;

 exe_opts:
   /* if (i < argc-1 && !strcmp(argv[i+1], "--")) */
   if (i < argc-1 && !strcmp(argv[i], "--"))
   {
     /* i += 2; */
     i++ ;
     exe_argc = argc - i ;
     exe_argv = (char **) malloc(exe_argc * sizeof(char *)) ;
     for (j=0; i<argc; i++, j++)
     {
        exe_argv[j] = (char *) malloc(strlen(argv[i])+1) ;
        strcpy(exe_argv[j], argv[i]) ;
     }
   }
}

