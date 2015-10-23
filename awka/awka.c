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
/* #include <unistd.h> */

#define AWKA_MAIN
#define TEMPBUFF_GOES_HERE

#include "../config.h"
#include "awka.h"
#include "awka_exe.h"
#include "mem.h"

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
char begin_used=FALSE, main_used=FALSE, end_used=FALSE;

extern int var_used;
extern awka_varname *varname;
extern char *uoutfile;
extern char awka_exe, awka_tmp, awka_comp;
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

int
main(int argc, char *argv[])
{
  int i = 1, len;
  char *c_file, *tmp;

  progcode = (struct pc *) malloc(20 * sizeof(struct pc));
  prog_allc = 20;
  prog_no = 0;

  initialize(argc, argv);
  parse();

  if (awka_comp) awka_tmp = FALSE;

  if (awka_exe || awka_comp)
  {
    if (awka_tmp)
    {
      tmp = (char *) tempnam("./","");
      c_file = (char *) malloc(strlen(tmp) + 3);
      sprintf(c_file, "%s.c",tmp);
      free(tmp);
      if (!(outfp = fopen(c_file, "w")))
        awka_error("Failed to open temporary output file.\n");
    }
    else
    {
      if (uoutfile)
      {
        c_file = (char *) malloc(strlen(uoutfile)+3);
        sprintf(c_file, "%s.c", uoutfile);
      }
      else
      {
        c_file = (char *) malloc(11);
        strcpy(c_file, "awka_out.c");
      }
      if (!(outfp = fopen(c_file, "w")))
        awka_error("Failed to open awka_out.c in current directory.\n");
    }
  }
  else
    outfp = stdout;

  if (!prog_no) 
    awka_error("Sorry, program was not parsed successfully.\n");

  translate();

  if (awka_exe || awka_comp)
  {
    FILE *fp;
    char *cmd;
    char *outfile;
    int incd_len=0, libd_len=0, libf_len=0;

    if (awka_tmp)
      outfile = (char *) tempnam("./", "");
    else if (uoutfile)
      outfile = uoutfile;
    else
    {
      outfile = (char *) malloc(15);
#if defined(__CYGWIN32__) || defined(__DJGPP__)
      strcpy(outfile, "./awka_out.exe");
#else
      strcpy(outfile, "./awka.out");
#endif
    }

    fclose(outfp);

    for (i=0; i<libd_used; i++)
      libd_len += strlen(libdir[i])+3;
    for (i=0; i<libf_used; i++)
      libf_len += strlen(libfile[i])+3;
    for (i=0; i<incd_used; i++)
      incd_len += strlen(incdir[i])+3;

    cmd = (char *) malloc( strlen(c_file) + strlen(awka_INCDIR) + strlen(awka_LIBDIR) + strlen(awka_CC) + strlen(awka_CFLAGS) + strlen(awka_MATHLIB) + strlen(awka_SOCKET_LIBS) + strlen(outfile) + incd_len + libd_len + libf_len + 35 );

    sprintf(cmd, "%s %s %s -I%s -L%s -lawka", awka_CC, awka_CFLAGS, c_file, awka_INCDIR, awka_LIBDIR);

    for (i=0; i<incd_used; i++)
      sprintf(cmd, "%s -I%s", cmd, incdir[i]);
    for (i=0; i<libd_used; i++)
      sprintf(cmd, "%s -L%s", cmd, libdir[i]);
    for (i=0; i<libf_used; i++)
      sprintf(cmd, "%s -l%s", cmd, libfile[i]);

    if (strlen(awka_SOCKET_LIBS))
      sprintf(cmd, "%s %s", cmd, awka_SOCKET_LIBS);

    if (strlen(awka_MATHLIB))
      sprintf(cmd, "%s %s -o %s", cmd, awka_MATHLIB, outfile);
    else
      sprintf(cmd, "%s -o %s", cmd, outfile);

    system(cmd);

    if (!(fp = fopen(outfile, "r")))
    {
      if (awka_tmp)
        unlink(c_file);
      awka_error("Awka error: compile failed.\n");
    }
    fclose(fp);

    if (awka_exe)
    {
      len = strlen(outfile);
      for (i=0; i<exe_argc; i++)
        len += strlen(exe_argv[i]) + 2;

      cmd = (char *) realloc(cmd, len + 1);
      strcpy(cmd, outfile);
      for (i=0; i<exe_argc; i++)
      {
        strcat(cmd, " ");
        strcat(cmd, exe_argv[i]);
      }

      system(cmd);
    }

    if (awka_tmp)
    {
      unlink(outfile);
      unlink(c_file);
    }
  }

  return 0;
}
