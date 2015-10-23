/*------------------------------------------------------------*
 | msg.c                                                      |
 | copyright 2000,  Andrew Sumner                             |
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

/*  cast.c  */

#include "awka.h"
#include "awka_exe.h"

void
msg_sort_swap(awka_varname *varname, int i, int j)
{
  awka_varname tmp;
  
  tmp.name = varname[i].name;
  tmp.usage = varname[i].usage;
  tmp.lines = varname[i].lines;
  tmp.files = varname[i].files;
  tmp.line_no = varname[i].line_no;
  tmp.line_allc = varname[i].line_allc;
  tmp.type = varname[i].type;
  
  varname[i].name = varname[j].name;
  varname[i].usage = varname[j].usage;
  varname[i].lines = varname[j].lines;
  varname[i].files = varname[j].files;
  varname[i].line_no = varname[j].line_no;
  varname[i].line_allc = varname[j].line_allc;
  varname[i].type = varname[j].type;
  
  varname[j].name = tmp.name;
  varname[j].usage = tmp.usage;
  varname[j].lines = tmp.lines;
  varname[j].files = tmp.files;
  varname[j].line_no = tmp.line_no;
  varname[j].line_allc = tmp.line_allc;
  varname[j].type = tmp.type;
}

void
msg_sort_varname(awka_varname *varname, int hi)
{
  unsigned i, j, ln, rn;

  while (hi > 1)
  {
    msg_sort_swap(varname, 0, hi/2);
    for (i = 0, j = hi; ; )
    {
      do
        if (--j > hi) { j++; break; }
      while (strcmp(varname[j].name, varname[0].name) > 0);

      do
        if (++i >= hi) { i--; break; }
      while (i < j && strcmp(varname[i].name, varname[0].name) < 0);

      if (i >= j)
        break;
      msg_sort_swap(varname, i, j);
    }
    msg_sort_swap(varname, j, 0);
    ln = j;
    rn = hi - ++j;
    if (ln < rn)
    {
      msg_sort_varname(varname, ln);
      varname += j;
      hi = rn;
    }
    else
    {
      msg_sort_varname(varname + j, rn);
      hi = ln;
    }
  }
}

void
msg_print_list(awka_varname *varname, int var_used)
{
  int i, j, set, get;
  char tmp[128];
  msg_sort_varname(varname, var_used);

  if (var_used)
  {
    fprintf(stderr,"Global Variable Listing...\n\n");
    fprintf(stderr,"  VAR-NAME                       USED   READ   SET\n");
    for (i=0; i<var_used; i++)
    {
      j = strlen(varname[i].name);
      strncpy(tmp, varname[i].name, j-4);
      tmp[j-4] = '\0';
      for (set=0,get=0,j=0; j<varname[i].line_no; j++)
        if (varname[i].usage[j] == _VAR_SET)
          set++;
        else
          get++;
      fprintf(stderr,"  %-30s %-6d %-6d %d\n",tmp, varname[i].line_no, get, set);
    }
    fprintf(stderr,"\n");
  }
}

void
msg_print_setnref(awka_varname *varname, int var_used)
{
  int i, j, set;
  char tmp[128];
  msg_sort_varname(varname, var_used);

  if (var_used)
  {
    fprintf(stderr,"Global Variables Set but not Referenced...\n\n");
    for (i=0; i<var_used; i++)
    {
      j = strlen(varname[i].name);
      strncpy(tmp, varname[i].name, j-4);
      tmp[j-4] = '\0';
      for (set=0,j=0; j<varname[i].line_no; j++)
        if (varname[i].usage[j] == _VAR_SET)
          set++;
        else
          break;
      if (j < varname[i].line_no) continue;
      fprintf(stderr,"  %s\n", tmp);
      for (set=0,j=0; j<varname[i].line_no; j++)
        fprintf(stderr,"    line %d\n",varname[i].lines[j]);
      fprintf(stderr,"\n");
    }
    fprintf(stderr,"\n");
  }
}

void
msg_print_refnset(awka_varname *varname, int var_used)
{
  int i, j, get;
  char tmp[128];
  msg_sort_varname(varname, var_used);

  if (var_used)
  {
    fprintf(stderr,"Global Variables Referenced but not Set...\n\n");
    for (i=0; i<var_used; i++)
    {
      j = strlen(varname[i].name);
      strncpy(tmp, varname[i].name, j-4);
      tmp[j-4] = '\0';
      for (get=0,j=0; j<varname[i].line_no; j++)
        if (varname[i].usage[j] == _VAR_SET)
          break;
        else
          get++;
      if (j < varname[i].line_no) continue;
      fprintf(stderr,"  %s\n", tmp);
      for (get=0,j=0; j<varname[i].line_no; j++)
        fprintf(stderr,"    line %d\n",varname[i].lines[j]);
      fprintf(stderr,"\n");
    }
    fprintf(stderr,"\n");
  }
}
