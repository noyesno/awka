/*------------------------------------------------------------*
 | awka.c                                                     |
 | copyright 1999,  Andrew Sumner                             |
 |                                                            |
 | This is a source file for the awka package, a translator   |
 | of the AWK programming language to ANSI C.                 |
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
#include <time.h>
#include <stdarg.h>

#define AWKA_MAIN
#define TEMPBUFF_GOES_HERE

#include "../config.h"
#include "awka.h"
#include "awka_exe.h"
#include "mem.h"

#define strdup_cat(s, suffix) strcat(strcpy(malloc(strlen(s)+strlen(suffix)+1), s), suffix)

char ** _arraylist = NULL;
int _array_no = 0, _array_allc = 0;
struct _fargs *_farglist = NULL;
int _farg_no = 0, _farg_allc = 0;
struct _fcalls *_fcalllist = NULL;
int _fcall_no = 0, _fcall_allc = 0;

struct pc *progcode = NULL;
FILE *outfp;
int curop_id, prog_allc, prog_no, curinst, curminst;
char buf[4096], *curarg, *curval;
char begin_used=FALSE, main_used=FALSE, end_used=FALSE, functab_used=FALSE;

extern int var_used;
extern awka_varname *varname;
extern char *uoutfile;
extern char awka_exe, awka_tmp, awka_comp, awka_comp_static;
extern int exe_argc;
extern char **exe_argv;
extern char **incdir, **libfile, **libdir;
extern int incd_used, libf_used, libd_used;

int awka_main = 0;
char *awka_main_func = NULL;
char **vardeclare = NULL;
int vdec_no = 0, vdec_allc = 0;

void initialize(int, char **);

void
awka_error(char *fmt, ...)
{
  va_list args;

  va_start( args, fmt );
  vfprintf( stderr, fmt, args );
  va_end( args );

  exit(1);
}

void
awka_warning(char *fmt, ...)
{
  va_list args;

  va_start( args, fmt );
  vfprintf( stderr, fmt, args );
  va_end( args );
}

int
add2arraylist(char *str)
{
  int i;
  char *nstr;

  nstr = (char *) malloc(strlen(str)+5);
  sprintf(nstr, "%s_awk", str);

  for (i=0; i<_array_no; i++)
    if (!strcmp(_arraylist[i], nstr))
      break;

  if (i == _array_no || _array_no == 0)
  {
    if (!_array_allc)
    {
      _array_allc = 5;
      _arraylist = (char **) malloc(_array_allc * sizeof(char **));
    }
    else if (_array_no == _array_allc)
    {
      _array_allc += 5;
      _arraylist = (char **) realloc(_arraylist, _array_allc * sizeof(char **));
    }

    _arraylist[_array_no] = (char *) malloc(strlen(nstr)+1);
    strcpy(_arraylist[_array_no], nstr);
    _array_no++;
    free(nstr);
    return 1;
  }
  free(nstr);
  return 0;
}

int
isarray(char *var)
{
  int i;

  for (i=0; i<_array_no; i++)
  {
    if (!strcmp(_arraylist[i], var))
      return 1;
  }
  return 0;
}

/*
-----------------------------------------------------------------------------
    | awka_comp | awka_exe | awka_main | awka_tmp
-----------------------------------------------------------------------------
 -c | FALSE     | FALSE    | TRUE      | FALSE    # translate
 -x | FALSE     | TRUE     | FALSE     | FALSE    # translate, compile, execute
 -X | TRUE      | FALSE    | FALSE     | FALSE    # translate, compile
 -t | -         | -        | -         | TRUE     # use temproray file
-----------------------------------------------------------------------------
*/

static char* tmpfile_prefix(const char *pattern) {
    char tmp_file[512];
    strcpy(tmp_file, pattern);
    int fd = mkstemp(tmp_file);
    char *file_path = strdup_cat(tmp_file, "");
    close(fd);
    return file_path;
}

static int awka_output_file(int awka_tmp, char *uoutfile, char **c_file, char **outfile) {
  char *outfile_prefix = NULL;
  char *outfile_suffix;

#if defined(__CYGWIN32__) || defined(__CYGWIN__) || defined(__DJGPP__)
  outfile_suffix = ".exe";
#else
  outfile_suffix = ".out";
#endif

  if (awka_tmp) {
    outfile_prefix = tmpfile_prefix("./awka-XXXXXX");
  } else if (!uoutfile) {
    outfile_prefix = strdup_cat("./awka-app", "");
  } else {
    outfile_prefix = strdup_cat(uoutfile, "");
  }

  *c_file = strdup_cat(outfile_prefix, ".c");

  if (awka_tmp) {
    *outfile = strdup_cat(outfile_prefix, outfile_suffix);
  } else if (uoutfile) {
    *outfile = uoutfile;
  } else {
    *outfile = strdup_cat(outfile_prefix, outfile_suffix);
  }

  if (awka_tmp)
    remove(outfile_prefix);  /* remove blank file from tmpfile_prefix() */

  free(outfile_prefix);

  return 1;
}

static char* awka_build_compile_command(char *c_file, char *outfile) {
    char *cmd;
    int incd_len=0, libd_len=0, libf_len=0;

    for (int i=0; i<libd_used; i++)
      libd_len += strlen(libdir[i])+3;
    for (int i=0; i<libf_used; i++)
      libf_len += strlen(libfile[i])+3;
    for (int i=0; i<incd_used; i++)
      incd_len += strlen(incdir[i])+3;

    cmd = (char *) malloc( strlen(c_file) + strlen(awka_INCDIR) + strlen(awka_LIBDIR) + strlen(awka_CC) + strlen(awka_CFLAGS) + strlen(awka_MATHLIB) + strlen(awka_SOCKET_LIBS) + strlen(outfile) + incd_len + libd_len + libf_len + 35 + 512);

    sprintf(cmd, "%s %s %s", awka_CC, awka_CFLAGS, c_file);

    for (int i=0; i<incd_used; i++)
      sprintf(cmd, "%s -I%s", cmd, incdir[i]);
    for (int i=0; i<libd_used; i++)
      sprintf(cmd, "%s -L%s", cmd, libdir[i]);

    sprintf(cmd, "%s -I%s -L%s", cmd, awka_INCDIR, awka_LIBDIR);

    for (int i=0; i<libf_used; i++)
      sprintf(cmd, "%s -l%s", cmd, libfile[i]);

    if (strlen(awka_SOCKET_LIBS))
      sprintf(cmd, "%s %s", cmd, awka_SOCKET_LIBS);

    if (strlen(awka_MATHLIB))
      sprintf(cmd, "%s %s -o %s", cmd, awka_MATHLIB, outfile);
    else
      sprintf(cmd, "%s -o %s", cmd, outfile);

    if (awka_comp_static) {
      sprintf(cmd, "%s -l:libawka.a", cmd);
    } else {
      sprintf(cmd, "%s -lawka", cmd);
    }
    return cmd;
}

static int awka_do_compile(char *c_file, char *outfile) {
    char *cmd;
    cmd = awka_build_compile_command(c_file, outfile);

    if (0) {
      fprintf(stderr, "%s\n", cmd);
    }

    int ret = system(cmd);

    free(cmd);

    FILE *fp;
    if (!(fp = fopen(outfile, "r")))
    {
      if (awka_tmp)
        remove(c_file);
      return 0;
    }
    fclose(fp);
    return 1;
}

static int awka_do_exec(char *outfile) {
  int len = strlen(outfile);
  for (int i=0; i<exe_argc; i++)
    len += strlen(exe_argv[i]) + 2;

  char *cmd = (char *) malloc(len + 1);
  strcpy(cmd, outfile);
  for (int i=0; i<exe_argc; i++) {
    strcat(cmd, " ");
    strcat(cmd, exe_argv[i]);
  }

  int ret = system(cmd);

  free(cmd);
  return 1;
}

int main(int argc, char *argv[])
{
  int i = 1, len;
  char *c_file, *tmp;
  char *outfile = NULL;
  int ret;

  progcode = (struct pc *) malloc(20 * sizeof(struct pc));
  prog_allc = 20;
  prog_no = 0;

  initialize(argc, argv);

  parse();

  if (!prog_no) {
    awka_error("Sorry, program was not parsed successfully.\n");
    return -1;
  }

  if (awka_comp) awka_tmp = FALSE;

  int do_translate = 1;
  int do_compile   = awka_exe || awka_comp ;
  int do_exec      = awka_exe;

  if (do_translate) {
    //% where to write translated code
    outfp = stdout;
    if (do_compile) {
      awka_output_file(awka_tmp, uoutfile, &c_file, &outfile);
      outfp = fopen(c_file, "w");
      if (!outfp) {
        awka_error("Failed to open %s to write.\n", c_file);
        return -1;
      }
    }
    translate();       //% -- transtate Awk script to C code
    fclose(outfp);
  }

  if (do_compile) {
    int compile_succ = awka_do_compile(c_file, outfile);
    if (!compile_succ) {
      awka_error("Awka error: compile failed.\n");
      return -1;
    }
  }

  if (do_exec) {
    awka_do_exec(outfile);
  }

  if ( do_compile || do_exec ) {
    if (awka_tmp) {
      remove(outfile);
    }
    remove(c_file);

    free(c_file);
    free(outfile);
  }

  return 0;
}
