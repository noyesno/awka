/*------------------------------------------------------------*
 | error.c                                                    |
 | copyright 1991,  Michael D. Brennan                        |
 |                                                            |
 | This is a source file for the awka package, a translator   |
 | of the AWK programming language to ANSI C.                 |
 |                                                            |
 | The file is a borrowed version of error.c from             |
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

#include  "awka.h"
#include  "scan.h"
#include  "bi_vars.h"
#include  "vargs.h"


#ifndef  EOF
#define  EOF  (-1)
#endif

static void  PROTO( rt_where, (void) ) ;
static void  PROTO( missing, (int, char *, int) ) ;
static char *PROTO( type_to_str, (int) ) ;


#ifdef  NO_VFPRINTF
#define  vfprintf  simple_vfprintf
#endif


/* for run time error messages only */
unsigned rt_nr , rt_fnr ;

static struct token_str  {
short token ;
char *str ; }  token_str[] = {
EOF , "end of file" ,
NL , "end of line",
SEMI_COLON , ";" ,
LBRACE , "{" ,
RBRACE , "}" ,
SC_FAKE_SEMI_COLON, "}",
LPAREN , "(" ,
RPAREN , ")" ,
LBOX , "[",
RBOX , "]",
QMARK , "?",
COLON , ":",
OR, "||",
AND, "&&",
ASSIGN , "=" ,
ADD_ASG, "+=",
SUB_ASG, "-=",
MUL_ASG, "*=",
DIV_ASG, "/=",
MOD_ASG, "%=",
POW_ASG, "^=",
EQ  , "==" ,
NEQ , "!=",
LT, "<" ,
LTE, "<=" ,
GT, ">",
GTE, ">=" ,
MATCH, string_buff,
PLUS , "+" ,
MINUS, "-" ,
MUL , "*" ,
DIV, "/"  , 
MOD, "%" ,
POW, "^" ,
NOT, "!" ,
COMMA, "," ,
INC_or_DEC , string_buff ,
DOUBLE  , string_buff ,
STRING_  , string_buff ,
ID  , string_buff ,
FUNCT_ID  , string_buff ,
BUILTIN  , string_buff ,
IO_OUT , string_buff ,
IO_IN, "<" ,
PIPE, "|" ,
DOLLAR, "$" ,
FIELD, "$" ,
COPROCESS, "|&" ,
0, (char *) 0 } ;

/* if paren_cnt >0 and we see one of these, we are missing a ')' */
static int missing_rparen[] =
{ EOF, NL, SEMI_COLON, SC_FAKE_SEMI_COLON, RBRACE, 0 } ;

/* ditto for '}' */
static int missing_rbrace[] =
{ EOF, a_BEGIN, a_END , 0 } ;

static void missing( c, n , ln)
  int c ;
  char *n ;
  int ln ;
{ char *s0, *s1 ;

  if ( pfile_name )
  { s0 = pfile_name ; s1 = ": " ; }
  else s0 = s1 = "" ;

  errmsg(0, "%s%sline %u: missing %c near %s" ,s0, s1, ln, c, n) ; 
}  

void  yyerror(s)
  char *s ; /* we won't use s as input 
  (yacc and bison force this).
  We will use s for storage to keep lint or the compiler
  off our back */
{ struct token_str *p ;
  int *ip ;

  s = (char *) 0 ;

  for ( p = token_str ; p->token ; p++ )
      if ( current_token == p->token )
      { s = p->str ; break ; }

  if ( ! s )  /* search the keywords */
  {
     s = find_kw_str(current_token) ;
  }


  if ( s )
  {
    if ( paren_cnt )
        for( ip = missing_rparen ; *ip ; ip++)
          if ( *ip == current_token )
          { missing(')', s, token_lineno) ;
            paren_cnt = 0 ;
            goto done ;
          }

    if ( brace_cnt )
        for( ip = missing_rbrace ; *ip ; ip++)
          if ( *ip == current_token )
          { missing('}', s, token_lineno) ;
            brace_cnt = 0 ;
            goto done ;
          }

    compile_error("syntax error at or near %s", s) ;

  }
  else  /* special cases */
  switch ( current_token )
  {
    case UNEXPECTED :
            unexpected_char() ; 
            goto done ;

    case BAD_DECIMAL :
            compile_error(
              "syntax error in decimal constant %s",
              string_buff ) ;
            break ;

    case RE :
            compile_error(
            "syntax error at or near /%s/", 
            string_buff ) ;
            break ;

    default :
            compile_error("syntax error") ;
            break ;
  }
  return ;

done :
  if ( ++compile_error_count == MAX_COMPILE_ERRORS ) exit(2) ;
}


/* generic error message with a hook into the system error 
   messages if errnum > 0 */

void  errmsg VA_ALIST2(int , errnum, char *, format)
  va_list args ;

  fprintf(stderr, "%s: " , progname) ;

  VA_START2(args, int, errnum, char *, format) ;
  vfprintf(stderr, format, args) ;
  va_end(args) ;

  if ( errnum > 0 ) fprintf(stderr, " (%s)" , strerror(errnum) ) ;

  fprintf( stderr, "\n") ;
}

void  compile_error  VA_ALIST(char *, format)
  va_list args ;
  char *s0, *s1 ;

  /* with multiple program files put program name in
     error message */
  if ( pfile_name )
  { s0 = pfile_name ; s1 = ": " ; }
  else
  { s0 = s1 = "" ; }

  fprintf(stderr, "%s: %s%sline %u: " , progname, s0, s1,token_lineno) ;
  VA_START(args, char *, format) ;
  vfprintf(stderr, format, args) ;
  va_end(args) ;
  fprintf(stderr, "\n") ;
  if ( ++compile_error_count == MAX_COMPILE_ERRORS ) exit(2) ;
}

void  rt_error VA_ALIST( char *, format)
  va_list args ;

  fprintf(stderr, "%s: run time error: " , progname ) ;
  VA_START(args, char *, format) ;
  vfprintf(stderr, format, args) ;
  va_end(args) ;
  putc('\n',stderr) ;
  rt_where() ;
  exit(2) ;
}


void bozo(s)
  char *s ;
{ 
  errmsg(0, "bozo: %s" , s) ; 
  exit(3) ;
}

void overflow(s, size)
  char *s ; unsigned size ;
{ 
  errmsg(0 , "program limit exceeded: %s size=%u", s, size) ;
  exit(2) ; 
}


/* print as much as we know about where a rt error occured */

static void rt_where()
{
  if ( FILENAME->type != C_STRING ) cast1_to_s(FILENAME) ;

  fprintf(stderr, "\tFILENAME=\"%s\" FNR=%u NR=%u\n", 
    string(FILENAME)->str, rt_fnr, rt_nr) ;
}

/* run time */
void rt_overflow(s, size)
  char *s ; unsigned size ;
{ 
  errmsg(0 , "program limit exceeded: %s size=%u", s, size) ;
  rt_where() ;
  exit(2) ;
}

void 
unexpected_char()
{ int c = yylval.ival ;

  fprintf(stderr, "%s: %u: ", progname, token_lineno) ;
  if ( c > ' ' && c < 127 )
      fprintf(stderr, "unexpected character '%c'\n" , c) ;
  else
      fprintf(stderr, "unexpected character 0x%02x\n" , c) ;
}

static char *type_to_str( type )
  int type ;
{ char *retval ;

  switch( type )
  {
    case  ST_VAR :  retval = "variable" ; break ;
    case  ST_ARRAY :  retval = "array" ; break ;
    case  ST_FUNCT :  retval = "function" ; break ;
    case  ST_LOCAL_VAR : retval = "local variable" ; break ;
    case  ST_LOCAL_ARRAY : retval = "local array" ; break ;
    case  ST_BUILTIN : retval = "builtin function (used as a variable)" ; break ;
    default : bozo("type_to_str") ;
          
  }
  return retval ;
}

/* emit an error message about a type clash */
void type_error(p)
  SYMTAB *p ;
{ compile_error("illegal reference to %s %s", 
    type_to_str(p->type) , p->name) ;
}



#ifdef  NO_VFPRINTF

/* a minimal vfprintf  */
int simple_vfprintf( fp, format, argp)
  FILE *fp ;
  char *format ; 
  va_list  argp ;
{ 
  char *q , *p, *t ;
  int l_flag ;
  char xbuff[64] ;

  q = format ;
  xbuff[0] = '%' ;

  while ( *q != 0 )
  { 
    if ( *q != '%' )
    {
      putc(*q, fp) ; q++ ; continue ;
    }

    /* mark the start with p */
    p = ++q ;  t = xbuff + 1 ;

    if ( *q == '-' )  *t++ = *q++ ;
    while ( scan_code[*(unsigned char*)q] == SC_DIGIT ) *t++ = *q++ ;
    if ( *q == '.' )
    { *t++ = *q++ ;
      while ( scan_code[*(unsigned char*)q] == SC_DIGIT ) *t++ = *q++ ;
    }

    if ( *q == 'l' )  { l_flag = 1 ; *t++ = *q++ ; }
    else l_flag = 0 ;

    
    *t = *q++ ; t[1] = 0 ;

    switch( *t )
    {
      case 'c' :  
      case 'd' :
      case 'o' :
      case 'x' :
      case 'u' :
           if ( l_flag )  fprintf(fp, xbuff, va_arg(argp,long) ) ;
           else  fprintf(fp, xbuff, va_arg(argp, int)) ;
           break ;

      case  's' :
           fprintf(fp, xbuff, va_arg(argp, char*)) ;
           break ;

      case  'g' :
      case  'f' :
           fprintf(fp, xbuff, va_arg(argp, double)) ;
           break ;

      default:
           putc('%', fp) ; 
           q = p ;
           break ;
    }
  }
  return 0 ; /* shut up */
}

#endif  /* USE_SIMPLE_VFPRINTF */


