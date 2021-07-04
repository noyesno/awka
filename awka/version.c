/*------------------------------------------------------------*
 | version.c                                                  |
 | copyright 1999,  Andrew Sumner                             |
 | copyright 1991,  Michael D. Brennan                        |
 |                                                            |
 | This is a source file for the awka package, a translator   |
 | of the AWK programming language to ANSI C.                 |
 |                                                            |
 | The file is a modified version of version.c from           |
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

#include "awka.h"
#include "../patchlev.h"

#define         VERSION_STRING         \
  "\nawka %s%s, %s\nCopyright (C) Andrew Sumner\nsome sections Copyright Michael T. Brennan\n"

#ifndef DOS_STRING
#define DOS_STRING        ""
#endif

/* print VERSION and exit */
void
print_version()
{

   printf(VERSION_STRING, AWKAVERSION, DOS_STRING, DATE_STRING) ;
   printf("\nhttp://awka.sourceforge.net\n\n");
   fflush(stdout) ;
   exit(0) ;
}

