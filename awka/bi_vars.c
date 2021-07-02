/*------------------------------------------------------------*
 | bi_vars.c                                                  |
 | copyright 1999,  Andrew Sumner                             |
 | copyright 1991,  Michael D. Brennan                        |
 |                                                            |
 | This is a source file for the awka package, a translator   |
 | of the AWK programming language to ANSI C.                 |
 |                                                            |
 | The file is a modified version of bi_vars.c from Mawk,     |
 | an implementation of the AWK processing language,          |
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

/* bi_vars.c */

#include "awka.h"
#include "symtype.h"
#include "bi_vars.h"
#include "field.h"
#include "init.h"
#include "memory.h"

/* the builtin variables */
CELL  bi_vars[NUM_BI_VAR] ;

/* the order here must match the order in bi_vars.h */

static char *bi_var_names[NUM_BI_VAR] = {
"NR" ,
"FNR" ,
"ARGC" ,
"FILENAME" ,
"OFS" ,
"ORS" ,
"RLENGTH" ,
"RSTART" ,
"SORTTYPE" ,
"SUBSEP" ,
"FIELDWIDTHS" ,
"SAVEWIDTHS" ,
"ARGIND" ,
"RT" ,
"PROCINFO" ,
"NF" ,
"FS" ,
"RS" ,
"OFMT" ,
"CONVFMT" ,
"FUNCTAB"
} ;

/* insert the builtin vars in the hash table */

void  bi_vars_init()
{ register int i ;
  register SYMTAB *s ;

  
  for ( i = 0 ; i < NUM_BI_VAR ; i++ )
  { if (!strcmp(bi_var_names[i], "PROCINFO"))
      continue;
    if (!strcmp(bi_var_names[i], "FUNCTAB"))
      continue;
    s = insert( bi_var_names[i] ) ;
    s->type = i <= 1 ? ST_NR : ST_VAR ; 
    s->stval.cp = bi_vars + i ;
    /* bi_vars[i].type = 0 which is C_NOINIT */
  }

  s = insert("ENVIRON") ;
  s->type = ST_ENV ;

  /* set defaults */

  FILENAME->type = C_STRING ;
  FILENAME->ptr = (PTR) new_STRING( "" ) ; 

  SORTTYPE->type = C_DOUBLE ;
  SORTTYPE->dval = 0.0 ;

  OFS->type = C_STRING ;
  OFS->ptr = (PTR) new_STRING( " " ) ;
  
  ORS->type = C_STRING ;
  ORS->ptr = (PTR) new_STRING( "\n" ) ;

  SUBSEP->type = C_STRING ;
  SUBSEP->ptr =  (PTR) new_STRING( "\034" ) ;

  NF->type = C_DOUBLE ;
  NF->dval = 0.0 ;

  FIELDWIDTHS->type = C_DOUBLE ;
  FIELDWIDTHS->dval = 0.0 ;

  SAVEWIDTHS->type = C_DOUBLE ;
  SAVEWIDTHS->dval = 0.0 ;

  ARGIND->type = C_DOUBLE ;
  ARGIND->dval = 0.0 ;

  RS->type = C_STRING ;
  RS->ptr = (PTR) new_STRING("\n") ;
  /* rs_shadow already set */

  RT->type = C_STRING ;
  RT->ptr = (PTR) new_STRING("\n") ;

  FS->type = C_STRING ;
  FS->ptr = (PTR) new_STRING(" ") ;
  /* fs_shadow is already set */

  OFMT->type = C_STRING ;
  OFMT->ptr = (PTR) new_STRING("%.6g") ;

  CONVFMT->type = C_STRING ;
  CONVFMT->ptr = OFMT->ptr ;
  string(OFMT)->ref_cnt++ ;

  NR->type = FNR->type = C_DOUBLE ;

  s = insert("PROCINFO") ;
  s->type = ST_ARRAY ;
  s->stval.array = new_ARRAY() ;
  /* PROCINFO->stval.array = new_ARRAY(); */
  /* dval is already 0.0 */

  s = insert("FUNCTAB") ;
  s->type = ST_ARRAY ;
  s->stval.array = new_ARRAY() ;

}
