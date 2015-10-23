/*------------------------------------------------------------*
 | msg.h                                                      |
 | copyright 1991,  Andrew Sumner                             |
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


#ifndef  _MSG_H
#define  _MSG_H   

#include "awka_exe.h"

#define MSG_LIST              1
#define MSG_SETnREF           2
#define MSG_REFnSET           4
#define MSG_GLOBinFUNC        8
#define MSG_GLOBoinFUNC      16
#define MSG_VARDECLARE       32
#define MSG_ASGNasTRUTH      64

extern int warning_msg;

void msg_print_list(awka_varname *varname, int var_used);
void msg_print_setnref(awka_varname *varname, int var_used);
void msg_print_refnset(awka_varname *varname, int var_used);

#endif
