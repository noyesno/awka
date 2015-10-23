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

static void PROTO(process_cmdline, (int, char **)) ;
static void PROTO(set_ARGV, (int, char **, int)) ;
static void PROTO(bad_option, (char *)) ;
static void PROTO(no_program, (void)) ;

extern void PROTO(print_version, (void)) ;
extern int PROTO(is_cmdline_assign, (char *)) ;

extern int awka_main;
extern char *awka_main_func;

char *progname, *uoutfile = NULL ;
extern char *int_argv;
short interactive_flag = 0 ;
char awka_exe = FALSE, awka_comp = FALSE;
char awka_tmp = FALSE;
int exe_argc = 0;
char **exe_argv = NULL;
char **incfile = NULL, **incdir = NULL, **libfile = NULL, **libdir = NULL;
int incf_allc = 0, incf_used = 0, incd_allc = 0, incd_used = 0, libf_allc = 0, libf_used = 0, libd_allc = 0, libd_used = 0;
int dump = 0, warning_msg = 0;

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

   process_cmdline(argc, argv) ;

   code_init() ;
}

int dump_code_flag ;                 /* if on dump internal code */
short posix_space_flag ;

#ifdef         DEBUG
int dump_RE ;                         /* if on dump compiled REs  */
#endif


static void
bad_option(s)
   char *s ;
{
   errmsg(0, "not an option: %s", s) ; exit(2) ; 
}

static void
no_program()
{
   exit(0) ;
}

static void
process_cmdline(argc, argv)
   int argc ;
   char **argv ;
{
   int i, j, nextarg ;
   char *optarg, *p ;
   PFILE dummy ;                 /* starts linked list of filenames */
   PFILE *tail = &dummy ;

   for (i = 1; i < argc && argv[i][0] == '-'; i = nextarg)
   {
      if (argv[i][1] == 0)        /* -  alone */
      {
         if (!pfile_name) no_program() ;
         break ;                 /* the for loop */
      }
      /* safe to look at argv[i][2] */

      j = 1;
      if (argv[i][j] == '-' && argv[i][0] == '-') j++;

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
               exit(2) ;
            }
            bad_option(argv[i]) ;
         }
         else if (strchr("-DxtX", argv[i][j]))
         {
           nextarg = i + 1;
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
          fprintf(stderr,"Need a \"-x\" argument for \"--\" to make any sense.\n");
          exit(0);
        }
        if (pfile_name)
          scan_init((char *) 0);
        else
        {
          if (i == 0)  no_program() ;
          scan_init(argv[i-1]) ;
        }
        goto exe_opts;
      }
      else  /* argument glued to option mb */
      {
         if (argv[i][j-1] == '-')
         {
           optarg = &argv[i][j] ;
           nextarg = i + 1 ;
           if (argv[i][j] != 'h' && argv[i][j] != 'v' && argv[i][j] != 'u')
             j++;
         }
         else
         {
           optarg = &argv[i][j] ;
           nextarg = i + 1 ;
         }
      }

      switch (argv[i][j])
      {
         case 'v':
            print_version() ;
            break ;

         case 'f':
            /* first file goes in pfile_name ; any more go
               on a list */
            if (!pfile_name)  pfile_name = optarg ;
            else
            {
               tail = tail->link = ZMALLOC(PFILE) ;
               tail->fname = optarg ;
            }
            break ;

         case 'c':
            awka_main = 1;
            awka_exe = awka_comp = awka_tmp = FALSE;
            if (argv[i+1][0] == '-')
            {
              fprintf(stderr,"Command Line Error: Expecting function-name after -c argument.\n");
              exit(1);
            }
            awka_main_func = argv[i+1];
            break;

         case 'w':    /* warning messages */
            if (argv[i+1][0] == '-')
            {
              fprintf(stderr,"Command Line Error: Expecting message flags 'abcdefg' after -w argument.\n");
              exit(1);
            }
            for (p=argv[i+1]; *p; p++)
            {
              switch (*p) {
                case 'a':
                  warning_msg |= MSG_LIST; break;
                case 'b':
                  warning_msg |= MSG_SETnREF; break;
                case 'c':
                  warning_msg |= MSG_REFnSET; break;
                case 'd':
                  warning_msg |= MSG_GLOBinFUNC; break;
                case 'e':
                  warning_msg |= MSG_GLOBoinFUNC; break;
                case 'f':
                  warning_msg |= MSG_VARDECLARE; break;
                case 'g':
                  warning_msg |= MSG_ASGNasTRUTH; break;
              }
            }
            break;

         case 'i':  /* include file */
            if (!incf_allc)
            {
              incf_allc = 8;
              incfile = (char **) malloc( incf_allc * sizeof(char *) );
            }
            else if (incf_used == incf_allc)
            {
              incf_allc *= 2;
              incfile = (char **) realloc( incfile, incf_allc * sizeof(char *) );
            }

            incfile[incf_used] = (char *) malloc( strlen(argv[++i])+1 );
            strcpy(incfile[incf_used++], argv[i]);
            break;

         case 'I':  /* include directory */
            if (!incd_allc)
            {
              incd_allc = 8;
              incdir = (char **) malloc( incd_allc * sizeof(char *) );
            }
            else if (incd_used == incd_allc)
            {
              incd_allc *= 2;
              incdir = (char **) realloc( incdir, incd_allc * sizeof(char *) );
            }

            incdir[incd_used] = (char *) malloc( strlen(argv[++i])+1 );
            strcpy(incdir[incd_used++], argv[i]);
            break;

         case 'l':  /* library */
            if (!libf_allc)
            {
              libf_allc = 8;
              libfile = (char **) malloc( libf_allc * sizeof(char *) );
            }
            else if (libf_used == libf_allc)
            {
              libf_allc *= 2;
              libfile = (char **) realloc( libfile, libf_allc * sizeof(char *) );
            }

            libfile[libf_used] = (char *) malloc( strlen(argv[++i])+1 );
            strcpy(libfile[libf_used++], argv[i]);
            break;

         case 'L':  /* library directory */
            if (!libd_allc)
            {
              libd_allc = 8;
              libdir = (char **) malloc( libd_allc * sizeof(char *) );
            }
            else if (libd_used == libd_allc)
            {
              libd_allc *= 2;
              libdir = (char **) realloc( libdir, libd_allc * sizeof(char *) );
            }

            libdir[libd_used] = (char *) malloc( strlen(argv[++i])+1 );
            strcpy(libdir[libd_used++], argv[i]);
            break;

         case 'X':
            awka_comp = TRUE;
            awka_exe = FALSE;
            awka_main = FALSE;
            break;

         case 'x':
            awka_exe = TRUE;
            awka_comp = FALSE;
            awka_main = FALSE;
            break;

         case 't':
            awka_tmp = TRUE;
            break;

         case 'D':
            dump = TRUE;
            break;

         case 'a':
            if (++i >= argc)
            {
              fprintf(stderr,"Command Line Error: Expecting filename after -a argument.\n");
              exit(1);
            }
            int_argv = (char *) malloc(strlen(argv[i])+1);
            strcpy(int_argv, argv[i]);
            break;
            
         case 'o':
            if (++i >= argc)
            {
              fprintf(stderr,"Command Line Error: Expecting filename after -o argument.\n");
              exit(1);
            }
            uoutfile = (char *) malloc(strlen(argv[i])+1);
            strcpy(uoutfile, argv[i]);
            break;

         case 'u':
         case 'h':
            fprintf(stderr,"\nusage: awka [-c fn] [-X] [-x -t] [-w flags] [-f filename] program_string [--] [exe-args]\n");
            fprintf(stderr,"       awka [-h] [-v]\n\n");
            fprintf(stderr,"    -c fn  Awka will generate a 'fn' function rather\n");
            fprintf(stderr,"           than a main function\n");
            fprintf(stderr,"    -x     Translates, compiles then executes program\n");
            fprintf(stderr,"    -t     If -x specified, the temporary C and executable\n");
            fprintf(stderr,"           files will be deleted following execution.\n");
            fprintf(stderr,"           Without this argument, -x will produce awka_out.c\n");
#ifdef __CYGWIN32__
            fprintf(stderr,"           and awka_out.exe in the current directory.\n");
#else
            fprintf(stderr,"           and awka.out in the current directory.\n");
#endif
            fprintf(stderr,"    -X     This will create the C file awka_out.c and compile\n");
#ifdef __CYGWIN32__
            fprintf(stderr,"           an executable 'awka_out.exe'.  Awka will stop after\n");
#else
            fprintf(stderr,"           an executable 'awka.out'.  Awka will stop after\n");
#endif
            fprintf(stderr,"           the compile is complete.\n");
            fprintf(stderr,"    -o fil If -x used, this will create an executable called 'fil'\n");
#ifdef __CYGWIN32__
            fprintf(stderr,"           instead of the default 'awka_out.exe'\n");
#else
            fprintf(stderr,"           instead of the default 'awka.out'\n");
#endif
            fprintf(stderr,"    -f     AWK Program file(s)\n");
            fprintf(stderr,"    --     If -x specified, all arguments after this point\n");
            fprintf(stderr,"           will be passed to the compiled executable.\n");
            fprintf(stderr,"    -a str The executable command-line arguments in 'str' will\n");
            fprintf(stderr,"           be hard-coded in the translated C output.\n");
            fprintf(stderr,"    -w flg Prints various warnings to stderr, useful in debugging\n");
            fprintf(stderr,"           large, complex programs.  The argument can contain the\n");
            fprintf(stderr,"           following characters:-\n");
            fprintf(stderr,"       'a' print a list of all global variables & their usage\n");
            fprintf(stderr,"       'b' warn about variables set but not referenced.\n");
            fprintf(stderr,"       'c' warn about variables referenced but not set.\n");
            fprintf(stderr,"       'd' report global vars used in any function.\n");           
            fprintf(stderr,"       'e' report global vars used in just one function.\n");           
            fprintf(stderr,"       'f' require global variables to be listed in a VDECL comment\n");
            fprintf(stderr,"       'g' warn about assignments used as truth expressions\n");
            fprintf(stderr,"    -v     Prints version information\n\n");
            exit(0);

         default:
            bad_option(argv[i]) ;
      }
   }

 no_more_opts:

   tail->link = (PFILE *) 0 ;
   pfile_list = dummy.link ;

   if (pfile_name)
      scan_init((char *) 0) ;
   else         /* program on command line */
   {
      if (i == argc)  no_program() ;
      scan_init(argv[i]) ;
/* #endif  */
   }
   i++;

 exe_opts:
   /* if (i < argc-1 && !strcmp(argv[i+1], "--")) */
   if (i < argc-1 && !strcmp(argv[i], "--"))
   {
     /* i += 2; */
     i++;
     exe_argc = argc - i;
     exe_argv = (char **) malloc(exe_argc * sizeof(char *));
     for (j=0; i<argc; i++, j++)
     {
        exe_argv[j] = (char *) malloc(strlen(argv[i])+1);
        strcpy(exe_argv[j], argv[i]);
     }
   }
}

