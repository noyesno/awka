/*------------------------------------------------------------*\
 | scan.c                                                     |
 | copyright 1999,  Andrew Sumner                             |
 | copyright 1991,  Michael D. Brennan                        |
 |                                                            |
 | This is a source file for the awka package, a translator   |
 | of the AWK programming language to ANSI C.                 |
 |                                                            |
 | The file is a borrowed version of scan.c from              |
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
\*------------------------------------------------------------*/

#include  "awka.h"
#include  "msg.h"
#include  "scan.h"
#include  "memory.h"
#include  "field.h"
#include  "init.h"
#include  "fin.h"
#include  "repl.h"
#include  "code.h"
#include  "bi_funct.h"

#include <unistd.h>

#ifndef          NO_FCNTL_H
#include  <fcntl.h>
#endif

#include  "files.h"

#define CMT_BUF_SZ  128

/* static functions */
static void   PROTO(scan_fillbuff, (void)) ;
static void   PROTO(scan_open, (void)) ;
static int    PROTO(slow_next, (void)) ;
static unsigned char   PROTO(next, (void)) ;
static void   PROTO(un_next, (void)) ;
static void   PROTO(eat_comment, (void)) ;
static void   PROTO(eat_semi_colon, (void)) ;
static double PROTO(collect_decimal, (int, int *)) ;
static int    PROTO(collect_string, (void)) ;
static int    PROTO(collect_RE, (void)) ;


/*-----------------------------
  program file management
 *----------------------------*/

char *pfile_name ;
STRING *program_string = (STRING *) 0 ;
PFILE *pfile_list ;
static unsigned char *buffer ;
unsigned char *buffp = NULL ;
 /* unsigned so it works with 8 bit chars */
static int program_fd = 0 ;
static int eof_flag = 0 ;

extern char **vardeclare ;
extern int vdec_no, vdec_allc ;

int _true_re = 0 ;

/* position used for error reporting */
int line_pos = 0 ;

void PROTO(init_extbi, (void)) ;

/*
 * +------------------+------------+------------+----------------+-----------------------------------+
 * | Inout Type       | program_fd | pfile_name | cmdine_program | eof_flag (1)                      |
 * +------------------+------------+------------+----------------+-----------------------------------+
 * |                  |            |            |                |                                   |
 * | inline script    |     -1     |  <null>    | <script text>  | read files if any                 |
 * |                  |            |            |                |                                   |
 * | read from stdin  |      0     |   "-"      |    <null>      | finished                          |
 * |                  |            |            |                |                                   |
 * | file(s)          |    > 0     |  <null>    |    <null>      | read next file (from pfile_list)  |
 * |                  |            |            |                |                                   |
 * +------------------+------------+------------+----------------+-----------------------------------+
 */
void
scan_init(cmdline_program)
   char *cmdline_program ;
{
   buffp = buffer = (unsigned char *) zmalloc(BUFFSZ + 1) ;

   if (cmdline_program)
   {
      program_fd = -1 ;                 /* command line program */
      program_string = new_STRING0(strlen(cmdline_program) + 1) ;
      strcpy(program_string->str, cmdline_program) ;
      /* simulate file termination */
      program_string->str[program_string->len - 1] = '\n' ;
      buffp = (unsigned char *) program_string->str ;
      eof_flag = 1 ;
   }
   else             /* program from file[s] */
   {
      scan_open() ;
      scan_fillbuff() ;
   }

#ifdef OS2  /* OS/2 "extproc" is similar to #! */
   if (strnicmp(buffp, "extproc ", 8) == 0)
     eat_comment() ;
#endif
   eat_nl() ;       /* scan to first token */
   if (next() == 0)
   {
      /* no program */
      exit(0) ;
   }

   un_next() ;

}

static void
scan_open()         /* open pfile_name */
{
   if (!pfile_list) 
   {
      if (program_fd <= 0) 
         return ;

      errmsg(0, "missing file list.") ;
      exit(2) ;
   }

   PFILE *q ;

   pfile_name = pfile_list->fname ;
   q = pfile_list ;
   pfile_list = pfile_list->link ;
   ZFREE(q) ;

   if (!pfile_name) 
   {
      errmsg(errno, "file name is missing from the file list.") ;
      exit(2) ;
   } 
   else if (pfile_name[0] == '-' && pfile_name[1] == 0)
   {
      /* read from stdin (piped input from previous command) */
      program_fd = 0 ;
      pfile_name = (char *) 0 ;
      /* fill_scanbuff will open and read stdin */
   }
   else if ((program_fd = open(pfile_name, O_RDONLY, 0)) == -1)
   {
      errmsg(errno, "cannot open %s.", pfile_name) ;
      exit(2) ;
   }
}

void
scan_cleanup()
{
   //if (program_fd >= 0 || pfile_name[0] == '-')
      zfree(buffer, BUFFSZ + 1) ;
   //else
   if (program_fd < 0)
      free_STRING(program_string) ;

   if (program_fd > 0)
      close(program_fd) ;

   /* redefine SPACE as [ \t\n] */

   scan_code['\n'] = posix_space_flag && rs_shadow.type != SEP_MLR
      ? SC_UNEXPECTED : SC_SPACE ;
   scan_code['\f'] = SC_UNEXPECTED ;         /*value doesn't matter */
   scan_code['\013'] = SC_UNEXPECTED ;         /* \v not space */
   scan_code['\r'] = SC_UNEXPECTED ;
}

static inline unsigned char
next()
{
  line_pos++ ; 
  return (*buffp ? *buffp++ : slow_next()) ;
}

static inline void
un_next() { buffp--; line_pos--; }

/*--------------------------------
  global variables shared by yyparse() and yylex()
  and used for error messages too
 *-------------------------------*/

int current_token = -1 ;
unsigned token_lineno = 1 ;
unsigned compile_error_count ;
int NR_flag ;                         /* are we tracking NR */
int paren_cnt ;
int brace_cnt ;
int print_flag ;                 /* changes meaning of '>' */
int getline_flag ;                 /* changes meaning of '<' */


/*----------------------------------------
 file reading functions
 next() and un_next(c) are macros in scan.h

 *---------------------*/

static unsigned lineno = 1 ;


static void
scan_fillbuff()
{
   unsigned r ;
   int c, i = BUFFSZ-1 ;
   FILE *f = NULL ;

   if (program_fd < 0)    /* inline script */
   {
      buffer[0] = '\0' ;
      buffp = buffer ;
      return ;
   }

   if (program_fd == 0)   /* read from stdin */
   {
      if (eof_flag) 
         return ;
      /* reading from stdin    ** not portable to windows OS */
      if (program_fd == 0 && !(f = fopen("/dev/stdin", "r")))
      {
         errmsg( (i=ferror(f)), "unable to open stdin for reading") ;
         exit(i) ;
      }
      buffp = buffer ;
      while ((c = fgetc(f)) != EOF) {
         *buffp++ = (char) c ;
         if (!--i) break ;
      }
      fclose(f) ;
      *buffp++ = '\n' ;
      *buffp = '\0' ;
      buffp = buffer ;
      eof_flag = 1 ;
      return ;
   }

   r = fillbuff(program_fd, (char *) buffer, BUFFSZ) ;
   if (r < BUFFSZ)
   {
      eof_flag = 1 ;
      /* make sure eof is terminated */
      buffer[r] = '\n' ;
      buffer[r + 1] = 0 ;
   }
}

/* read one character -- slowly */
static int
slow_next()
{

   while (*buffp == 0)
   {
      if (!eof_flag)
      {
         buffp = buffer ;
         scan_fillbuff() ;
      }
      else if (pfile_list)   /* open another program file */
      {
         PFILE *q ;

         if (program_fd > 0)  
         {
            close(program_fd) ;
            eof_flag = 0 ;
         }

         pfile_name = pfile_list->fname ;
         q = pfile_list ;
         pfile_list = pfile_list->link ;
         ZFREE(q) ;

         if (pfile_name[0] == '-' && pfile_name[1] == 0)
         {
            /* read from stdin (piped input from previous command) */
            program_fd = 0 ;
            eof_flag = 0 ;
            pfile_name = (char *) 0 ;
            buffp = buffer ;
            buffer[0] = 0 ;
            /* fill_scanbuff will open and read stdin */
         }
         else if ((program_fd = open(pfile_name, O_RDONLY, 0)) == -1)
         {
            errmsg(errno, "cannot open %s.", pfile_name) ;
            exit(2) ;
         }
        // scan_open() ;
         token_lineno = lineno = 1 ;
         line_pos = 0 ;
         scan_fillbuff() ;
         buffp = buffer ;
      }
      else  break /* real eof */ ;
   }

   return *buffp++ ;                 /* note can un_next() , eof which is zero */
}

static void
eat_comment()
{
   register int c ;
   
   if (warning_msg & MSG_VARDECLARE)
   {
      char keyword[] = "VDECL: ", buf[CMT_BUF_SZ] ;
      int i = 0, j = strlen(keyword) ;

      /* see if its a var_declare comment */
      do {
         c = next() ;
      } while (c == ' ' || c == '\t') ;

      for (i=0; i<j; i++)
      {
         if (c != keyword[i]) break ;
         c = next() ;
      }

      if (i < j)
      {
         if (c == '\n')
         {
            un_next() ;
            return ;
         }
      }
      else
      {
         /* got the keyword - anything from here on is a variable */
         while (c != '\0' && c != '\n')
         {
            /* navigate to the next word */
            while (c == ' ' || c == '\t') c = next() ;
            if (c == '\n' || c == '\0') { line_pos = 0 ; return ; }

            i = 0 ;
            while (c != ' ' && c != '\t' && c != '\n' && c != '\0')
            {
               buf[i++] = c ;
               if (i == CMT_BUF_SZ-1) break ;
               c = next() ;
            }
            buf[i] = '\0' ;

            if (vardeclare == NULL)
            {
               vdec_allc = 10 ;
               vardeclare = (char **) malloc( 10 * sizeof(char *) ) ;
            }
            else if (vdec_allc == vdec_no)
            {
               vdec_allc += 10 ;
               vardeclare = (char **) realloc( vardeclare, vdec_allc * sizeof(char *) ) ;
            }

            for (j=0; j<vdec_no; j++)
               if (!strcmp(vardeclare[j], buf))
                  break ;

            if (j == vdec_no)
            {
               vardeclare[vdec_no] = (char *) malloc(i + 1) ;
               memcpy(vardeclare[vdec_no], buf, i+1) ;
               vdec_no++ ;
            }
         }

         return ;
      }
   }

   while ((c = next()) != '\n' && scan_code[c]) ;
   un_next() ;
}

/* this is how we handle extra semi-colons that are
   now allowed to separate pattern-action blocks

   A proof that they are useless clutter to the language:
   we throw them away
*/

static void
eat_semi_colon()
/* eat one semi-colon on the current line */
{
   register int c ;

   while (scan_code[c = next()] == SC_SPACE) ;
   if (c != ';')  un_next() ;
}

void
eat_nl()                        /* eat all space including newlines */
{
   while (1)
      switch (scan_code[next()])
      {
         case SC_COMMENT:
            eat_comment() ;
            break ;

         case SC_NL:
	    line_pos = 0 ;
            lineno++ ;
            /* fall thru  */

         case SC_SPACE:
            break ;

         case SC_ESCAPE:
            /* bug fix - surprised anyone did this,
               a csh user with backslash dyslexia.(Not a joke)
            */
            {
               register unsigned c ;

               while (scan_code[c = next()] == SC_SPACE) ;
               if (c == '\n')
	       {
		  line_pos = 0 ;
                  token_lineno = ++lineno ;
	       }
               else if (c == 0)  
               {
                  un_next() ;
                  return ;
               }
               else /* error */
               {
                  un_next() ;
                  /* can't un_next() twice so deal with it */
                  yylval.ival = '\\' ;
                  unexpected_char() ;
                  if ( ++compile_error_count == MAX_COMPILE_ERRORS )
                     exit(2) ;
                  return ;
               }
            }
            break ;
             
         default:
            un_next() ;
            return ;
      }
}

int
yylex()
{
   register int c, funct, nexts ;
   static int prev_token = -1 ;

   token_lineno = lineno ;

reswitch:

   switch (scan_code[c = next()])
   {
      case 0:
         ct_ret(EOF) ;

      case SC_SPACE:
         goto reswitch ;

      case SC_COMMENT:
         eat_comment() ;
         goto reswitch ;

      case SC_NL:
         lineno++ ;
         eat_nl() ;
         ct_ret(NL) ;

      case SC_ESCAPE:
         while (scan_code[c = next()] == SC_SPACE) ;
         if (c == '\n')
         {
            token_lineno = ++lineno ;
	        line_pos = 0 ;
            goto reswitch ;
         }

         if (c == 0)  ct_ret(EOF) ;
         un_next() ;
         yylval.ival = '\\' ;
         ct_ret(UNEXPECTED) ;


      case SC_SEMI_COLON:
         eat_nl() ;
         ct_ret(SEMI_COLON) ;

      case SC_LBRACE:
         eat_nl() ;
         brace_cnt++ ;
         ct_ret(LBRACE) ;

      case SC_PLUS:
         switch (next())
         {
            case '+':
               yylval.ival = '+' ;
               string_buff[0] =
                  string_buff[1] = '+' ;
               string_buff[2] = 0 ;
               ct_ret(INC_or_DEC) ;

            case '=':
               ct_ret(ADD_ASG) ;

            default:
               un_next() ;
               ct_ret(PLUS) ;
         }

      case SC_MINUS:
         switch (next())
         {
            case '-':
               yylval.ival = '-' ;
               string_buff[0] =
                  string_buff[1] = '-' ;
               string_buff[2] = 0 ;
               ct_ret(INC_or_DEC) ;

            case '=':
               ct_ret(SUB_ASG) ;

            default:
               un_next() ;
               ct_ret(MINUS) ;
         }

      case SC_COMMA:
         eat_nl() ;
         ct_ret(COMMA) ;

      case SC_MUL:
	 // * or *= or **=
         //test1_ret('=', MUL_ASG, MUL) ;
         switch (next())
         {
            case '*': // **
               yylval.ival = '*' ;
               string_buff[0] =
                 string_buff[1] = '*' ;
               string_buff[2] = 0 ;
	           if (c = next() == '=')
	           {
		          // **=
                  yylval.ival = '=' ;
                  string_buff[2] = '=' ;
                  string_buff[3] = 0 ;
		          ct_ret(POW_ASG) ;
	           }
               yylval.ival = c ;
               string_buff[2] = c ;
               string_buff[3] = 0 ;
	           ct_ret(UNEXPECTED) ;

            case '=':  // *=
               ct_ret(MUL_ASG) ;

            default:  // *
               un_next() ;
               ct_ret(MUL) ;
         }


      case SC_DIV:
         {
            static int can_precede_div[] =
            {DOUBLE, STRING_, RPAREN, ID, D_ID, RE, RBOX, FIELD,
             GETLINE, INC_or_DEC, -1} ;

            int *p = can_precede_div ;

            do
            {
               if (*p == current_token)
               {
                  if (*p != INC_or_DEC)
                  {
                      test1_ret('=', DIV_ASG, DIV) ;
                  }
                  else if (next() == '=')
                      /* x++ /= ... cannot be div-assign */
                  {
                      un_next() ;
                      ct_ret(collect_RE()) ;
                  }
                  else
                  {
                      un_next() ;
                      ct_ret(DIV) ;
                  }
               }
            }
            while (*++p != -1) ;

            ct_ret(collect_RE()) ;
         }

      case SC_MOD:
         test1_ret('=', MOD_ASG, MOD) ;

      case SC_POW:
         test1_ret('=', POW_ASG, POW) ;

      case SC_LPAREN:
         paren_cnt++ ;
         ct_ret(LPAREN) ;

      case SC_RPAREN:
         if (--paren_cnt < 0)
         {
            string_buff[0] = ')' ;
            string_buff[1] = 0 ;
            compile_error("extra ')'") ;
            paren_cnt = 0 ;
            goto reswitch ;
         }

         ct_ret(RPAREN) ;

      case SC_LBOX:
         string_buff[0] = '[' ;
         string_buff[1] = 0 ;
         ct_ret(LBOX) ;

      case SC_RBOX:
         string_buff[0] = ']' ;
         string_buff[1] = 0 ;
         ct_ret(RBOX) ;

      case SC_MATCH:
         string_buff[0] = '~' ;
         string_buff[1] = 0 ;
         yylval.ival = 1 ;
         ct_ret(MATCH) ;

      case SC_EQUAL:
         test1_ret('=', EQ, ASSIGN) ;

      case SC_NOT:                /* !  */
         if ((c = next()) == '~')
         {
            string_buff[0] = '!' ;
            string_buff[1] = '~' ;
            string_buff[2] = 0 ;
            yylval.ival = 0 ;
            ct_ret(MATCH) ;
         }
         else if (c == '=')  ct_ret(NEQ) ;

         un_next() ;
         ct_ret(NOT) ;


      case SC_LT:                /* '<' */
         if (next() == '=')  ct_ret(LTE) ;
         else  un_next() ;

         if (getline_flag)
         {
            getline_flag = 0 ;
            ct_ret(IO_IN) ;
         }
         else  ct_ret(LT) ;

      case SC_GT:                /* '>' */
         if (print_flag && paren_cnt == 0)
         {
            print_flag = 0 ;
            /* there are 3 types of IO_OUT
               -- build the error string in string_buff */
            string_buff[0] = '>' ;
            if (next() == '>')
            {
               yylval.ival = F_APPEND ;
               string_buff[1] = '>' ;
               string_buff[2] = 0 ;
            }
            else
            {
               un_next() ;
               yylval.ival = F_TRUNC ;
               string_buff[1] = 0 ;
            }
            return prev_token = current_token = IO_OUT ;
         }

         test1_ret('=', GTE, GT) ;

      case SC_OR:
         if ((c = next()) == '|')
         {
            eat_nl() ;
            ct_ret(OR) ;
         }
         else if (c == '&')
         {
           eat_nl() ;
           if (print_flag && paren_cnt == 0)
           {
             print_flag = 0 ;
             yylval.ival = COP_OUT ;
             string_buff[0] = '|' ;
             string_buff[1] = '&' ;
             string_buff[2] = 0 ;
             ct_ret(COPROCESS_OUT) ;
           }
           ct_ret(COPROCESS) ;
         }
         else
         {
            un_next() ;

            if (print_flag && paren_cnt == 0)
            {
               print_flag = 0 ;
               yylval.ival = PIPE_OUT ;
               string_buff[0] = '|' ;
               string_buff[1] = 0 ;
               ct_ret(IO_OUT) ;
            }
            else  ct_ret(PIPE) ;
         }

      case SC_AND:
         if (next() == '&')
         {
            eat_nl() ;
            ct_ret(AND) ;
         }
         else
         {
            un_next() ;
            yylval.ival = '&' ;
            ct_ret(UNEXPECTED) ;
         }

      case SC_QMARK:
         ct_ret(QMARK) ;

      case SC_COLON:
         ct_ret(COLON) ;

      case SC_RBRACE:
         if (--brace_cnt < 0)
         {
            compile_error("extra '}'") ;
            eat_semi_colon() ;
            brace_cnt = 0 ;
            goto reswitch ;
         }

         if ((c = current_token) == NL || c == SEMI_COLON
             || c == SC_FAKE_SEMI_COLON || c == RBRACE)
         {
            /* if the brace_cnt is zero , we've completed
               a pattern action block. If the user insists
               on adding a semi-colon on the same line
               we will eat it.        Note what we do below:
               physical law -- conservation of semi-colons */

            if (brace_cnt == 0)
               eat_semi_colon() ;
            eat_nl() ;
            ct_ret(RBRACE) ;
         }

         /* supply missing semi-colon to statement that
             precedes a '}' */
         brace_cnt++ ;
         un_next() ;
         current_token = SC_FAKE_SEMI_COLON ;
         return prev_token = SEMI_COLON ;

      case SC_DIGIT:
      case SC_DOT:
         {
            double d ;
            int flag ;
            static double double_zero = 0.0 ;
            static double double_one = 1.0 ;

            if ((d = collect_decimal(c, &flag)) == 0.0)
            {
               if (flag)  ct_ret(flag) ;
               else  yylval.ptr = (PTR) & double_zero ;
            }
            else if (d == 1.0)
            {
               yylval.ptr = (PTR) & double_one ;
            }
            else
            {
               yylval.ptr = (PTR) ZMALLOC(double) ;
               *(double *) yylval.ptr = d ;
            }
            ct_ret(DOUBLE) ;
         }

      case SC_DOLLAR:                /* '$' */
         {
            double d ;
            int flag ;

            while (scan_code[c = next()] == SC_SPACE) ;
            if (scan_code[c] != SC_DIGIT &&
                scan_code[c] != SC_DOT)
            {
               un_next() ;
               ct_ret(DOLLAR) ;
            }

            /* compute field address at compile time */
            if ((d = collect_decimal(c, &flag)) == 0.0)
            {
               if (flag)
                  ct_ret(flag) ; /* an error */
               else
                  yylval.cp = &field[0] ;
            }
            else
            {
               if (d > MAX_FIELD)
               {
                  compile_error(
                     "$%g exceeds maximum field(%d)", d, MAX_FIELD) ;
                  d = MAX_FIELD ;
               }
               yylval.cp = field_ptr((int) d) ;
            }

            ct_ret(FIELD) ;
         }

      case SC_DQUOTE:
         return prev_token = current_token = collect_string() ;

      case SC_IDCHAR:                /* collect an identifier */
         {
            unsigned char *p =
            (unsigned char *) string_buff + 1 ;
            SYMTAB *stp ;

            string_buff[0] = c ;

            while (
                     (c = scan_code[*p++ = next()]) == SC_IDCHAR ||
                     c == SC_DIGIT) ;

            funct = 0 ;
            nexts = 1 ;
            if (c == SC_SPACE)
            {
               *(p-1) = 0 ;
               if (bi_funct_find(string_buff))
               {
                 while ((scan_code[c = next()]) == SC_SPACE)
                    nexts++ ;
                 if (c == '(')
                    funct = TRUE ;
               }
            }
            else if (*(p-1) == '(')
               funct = 1 ;
            while (nexts--)
               un_next() ;
            *--p = 0 ;

            switch ((stp = find(string_buff, funct))->type)
            {
               case ST_NONE:
                  _st_none:
                  /* check for function call before defined */
                  if (next() == '(')
                  {
                     stp = find(string_buff, 2) ;
                     stp->type = ST_FUNCT ;
                     stp->stval.fbp = (FBLOCK *)
                         zmalloc(sizeof(FBLOCK)) ;
                     stp->stval.fbp->name = stp->name ;
                     stp->stval.fbp->code = (INST *) 0 ;
                     yylval.fbp = stp->stval.fbp ;
                     current_token = FUNCT_ID ;
                  }
                  else
                  {
                     yylval.stp = stp ;
                     current_token =
                        current_token == DOLLAR ? D_ID : ID ;
                  }
                  un_next() ;
                  break ;

               case ST_NR:
                  NR_flag = 1 ;
                  stp->type = ST_VAR ;
                  /* fall thru */

               case ST_VAR:
               case ST_ARRAY:
               case ST_LOCAL_NONE:
               case ST_LOCAL_VAR:
               case ST_LOCAL_ARRAY:
                  yylval.stp = stp ;
                  current_token =
                     current_token == DOLLAR ? D_ID : ID ;
                  break ;

               case ST_ENV:
                  stp->type = ST_ARRAY ;
                  stp->stval.array = new_ARRAY() ;
                  /* load_environ(stp->stval.array) ; */
                  yylval.stp = stp ;
                  current_token =
                     current_token == DOLLAR ? D_ID : ID ;
                  break ;

               case ST_FUNCT:
                  yylval.fbp = stp->stval.fbp ;
                  current_token = FUNCT_ID ;
                  break ;

               case ST_KEYWORD:
                  current_token = stp->stval.kw ;
                  break ;

               case ST_BUILTIN:
                  /* if (current_token == FUNCTION) goto _st_none ; */
                  if (funct == 0)
                     goto _st_none ;
                  yylval.bip = stp->stval.bip ;
                  /* if (is_ext_builtin(string_buff) && prev_token == FUNCTION)  */
                  if (is_ext_builtin(string_buff))
                  {
                     stp->type = ST_NONE ;
                     stp->name = (char *) malloc(strlen(string_buff)+1) ;
                     strcpy(stp->name, string_buff) ;
                     yylval.stp = stp ;
                     if (prev_token == FUNCTION)
                        current_token = ID ;
                     else
                        current_token = FUNCT_ID ;
                  }
                  else
                     current_token = BUILTIN ;
                  break ;

               case ST_LENGTH:
                  yylval.bip = stp->stval.bip ;

                  /* check for length alone, this is an ugly
                         hack */
                  while (scan_code[c = next()] == SC_SPACE) ;
                  if (c == '(') {
                     c = next() ;
                     if (c == ')') {
                        /* length() parsed using LENGTH0 rule */
                        current_token = LENGTH0 ;
                        break ;
                     }
                     c = '(' ;
                     un_next() ;
                  }
                  un_next() ;

                  current_token = c == '(' ? LENGTH : LENGTH0 ;

                  break ;

               case ST_FIELD:
                  yylval.cp = stp->stval.cp ;
                  current_token = FIELD ;
                  break ;

               default:
                  bozo("find returned bad st type.") ;
            }
            return prev_token = current_token ;
         }


      case SC_UNEXPECTED:
         yylval.ival = c & 0xff ;
         ct_ret(UNEXPECTED) ;
   }
   return prev_token = 0 ;                         /* never get here make lint happy */
}

/* collect a decimal constant in temp_buff.
   Return the value and error conditions by reference */

static double
collect_decimal(c, flag)
   int c ;
   int *flag ;
{
   register unsigned char *p = (unsigned char *) string_buff + 1 ;
   unsigned char *endp ;
   double d ;

   *flag = 0 ;
   /* memset(string_buff, 0, MIN_SPRINTF) ; */
   string_buff[0] = c ;

   if (c == '.')
   {
      if (scan_code[*p++ = next()] != SC_DIGIT)
      {
         *flag = UNEXPECTED ;
         yylval.ival = '.' ;
         return 0.0 ;
      }
   }
   else
   {
      while (scan_code[*p++ = next()] == SC_DIGIT) ;
      if (p[-1] != '.')
      {
         un_next() ;
         *p = 0 ;
         p-- ;
      }
   }

   /* get rest of digits after decimal point */
   while (scan_code[*p++ = next()] == SC_DIGIT) ;

   /* check for exponent */
   if (p[-1] != 'e' && p[-1] != 'E')
   {
      un_next() ;
      *--p = 0 ;
   }
   else         /* get the exponent */
   {
      if (scan_code[*p = next()] != SC_DIGIT &&
          *p != '-' && *p != '+' && *p != ')' && *p != ' ')
      {
         *++p = 0 ;
         *flag = BAD_DECIMAL ;
         return 0.0 ;
      }
      else  /* get the rest of the exponent */
      {
         p++ ;
         while (scan_code[*p++ = next()] == SC_DIGIT) ;
         un_next() ;
         *--p = 0 ;
      }
   }

   errno = 0 ;                         /* check for overflow/underflow */
   d = strtod(string_buff, (char **) &endp) ;

#ifndef         STRTOD_UNDERFLOW_ON_ZERO_BUG
   if (errno)  compile_error("%s : decimal %sflow", string_buff,
                    d == 0.0 ? "under" : "over") ;
#else /* ! sun4 bug */
   if (errno && d != 0.0)
      compile_error("%s : decimal overflow", string_buff) ;
#endif

   if (endp < p)
   {
      *flag = BAD_DECIMAL ;
      return 0.0 ;
   }
   return d ;
}

/*----------  process escape characters ---------------*/

static char hex_val['f' - 'A' + 1] =
{
   10, 11, 12, 13, 14, 15, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   10, 11, 12, 13, 14, 15} ;

#define isoctal(x)  ((x)>='0'&&(x)<='7')

#define         hex_value(x)        hex_val[(x)-'A']

#define ishex(x) (scan_code[x] == SC_DIGIT ||\
                  'A' <= (x) && (x) <= 'f' && hex_value(x))

static int PROTO(octal, (char **)) ;
static int PROTO(hex, (char **)) ;

/* process one , two or three octal digits
   moving a pointer forward by reference */
static int
octal(start_p)
   char **start_p ;
{
   register char *p = *start_p ;
   register unsigned x ;

   x = *p++ - '0' ;
   if (isoctal(*p))
   {
      x = (x << 3) + *p++ - '0' ;
      if (isoctal(*p))
         x = (x << 3) + *p++ - '0' ;
   }
   *start_p = p ;
   return x & 0xff ;
}

/* process one or two hex digits
   moving a pointer forward by reference */

static int
hex(start_p)
   char **start_p ;
{
   register unsigned char *p = (unsigned char *) *start_p ;
   register unsigned x ;
   unsigned t ;

   if (scan_code[*p] == SC_DIGIT)
      x = *p++ - '0' ;
   else
      x = hex_value(*p++) ;

   if (scan_code[*p] == SC_DIGIT)  x = (x << 4) + *p++ - '0' ;
   else if ('A' <= *p && *p <= 'f' && (t = hex_value(*p)))
   {
      x = (x << 4) + t ;
      p++ ;
   }

   *start_p = (char *) p ;
   return x ;
}

#define   ET_END   9

static struct
{
   char in, out ;
}
escape_test[ET_END + 1] =
{
   'n',  '\n' ,
   't',  '\t' ,
   'f',  '\f' ,
   'b',  '\b' ,
   'r',  '\r' ,
   'a',  '\07' ,
   'v',  '\013' ,
   '\\', '\\' ,
   '\"', '\"' ,
   0,    0
} ;


/* process the escape characters in a string, in place . */

char *
rm_escape(s)
   char *s ;
{
   register char *p, *q ;
   char *t ;
   int i ;

   q = p = s ;

   while (*p)
   {
      if (*p == '\\')
      {
         escape_test[ET_END].in = *++p ; /* sentinal */
         i = 0 ;
         while (escape_test[i].in != *p)  i++ ;

         if (i != ET_END)        /* in table */
         {
            p++ ;
            *q++ = escape_test[i].out ;
         }
         else if (isoctal(*p))
         {
            t = p ;
            int c = octal(&t) ;
            if ( c==0 && (t-p)==1 ) {
              *q++ = *(p-1) ;
              *q++ = *p++ ;
            } else {
              *q++ = c ;
               p = t ;
            }
         }
         else if (*p == 'x' && ishex(*(unsigned char *) (p + 1)))
         {
            t = p + 1 ;
            *q++ = hex(&t) ;
            p = t ;
         }
         else if (*p == 0)        /* can only happen with command line assign */
            *q++ = '\\' ;
         else  /* not an escape sequence */
         {
            /* *q++ = '\\' ; */
            *q++ = *p++ ;
         }
      }
      else
         *q++ = *p++ ;
   }

   *q = 0 ;
   return s ;
}

static int
collect_string()
{
   register unsigned char *p = (unsigned char *) string_buff ;
   int c ;
   int e_flag = 0 ;                 /* on if have an escape char */

   while (1)
      switch (scan_code[*p++ = next()])
      {
         case SC_DQUOTE:        /* done */
            *--p = 0 ;
            goto out ;

         case SC_NL:
            p[-1] = 0 ;
            /* fall thru */

         case 0:                /* unterminated string */
            compile_error(
                            "runaway string constant \"%.10s ...",
                            string_buff, token_lineno) ;
            exit(2) ;

         case SC_ESCAPE:
            if ((c = next()) == '\n')
            {
               p-- ;
               lineno++ ;
	           line_pos = 0 ;
            }
            else if (c == 0)  un_next() ;
            else
            {
               *p++ = c ;
               e_flag = 1 ; /* this was commented by me, not in mawk */
            }

            break ;

         default:
            break ;
      }

out:
   AWKA_DEBUG("token string 1 = \"%s\"\n", string_buff) ;
   unsigned char *s = e_flag ? rm_escape(string_buff) : string_buff ; 
   AWKA_DEBUG("token string 2 = \"%s\"\n", s) ;
   yylval.ptr = (PTR) new_STRING(s) ;
   return STRING_ ;
}


static int
collect_RE()
{
   register unsigned char *p = (unsigned char *) string_buff ;
   int c, paren=0, box=0 ;
   STRING *sval ;

   while (1)
      switch (scan_code[*p++ = next()])
      {
         case SC_LPAREN:
            paren++ ;
            break ;

         case SC_RPAREN:
            if (--paren < 0)
               paren=0 ;
            break ;

         case SC_LBOX:
            box = 1 ;
            break ;

         case SC_RBOX:
            if (--box < 0)
               box=0 ;
            break ;

         case SC_DIV:                /* done */
            if (!box && !paren)
            {
               *--p = 0 ;
               goto out ;
            }
            break ;

         case SC_NL:
            p[-1] = 0 ;
            /* fall thru */

         case 0:                /* unterminated re */
            compile_error(
                            "runaway regular expression /%.10s ...",
                            string_buff, token_lineno) ;
            exit(2) ;

         case SC_ESCAPE:
            switch (c = next())
            {
               case '/':
                  _true_re = 1 ;
                  p[-1] = '/' ;
                  break ;

               case '\n':
                  p-- ;
                  break ;

               case 0:
                  un_next() ;
                  break ;

               default:
                  *p++ = c ;
                  break ;
            }
            break ;
      }

out:
   /* now we've got the RE, so compile it */
   sval = new_STRING(string_buff) ;
   yylval.ptr = re_compile(sval) ;
   free_STRING(sval) ;
   return RE ;
}

/*--------
  target is big enough to hold size + 1 chars
  on exit the back of the target is zero terminated
 *--------------*/
unsigned
fillbuff(fd, target, size)
   int fd ;
   register char *target ;
   unsigned size ;
{
   register int r ;
   unsigned entry_size = size ;

   while (size)
      switch (r = read(fd, target, size))
      {
         case -1:
            errmsg(errno, "read error") ;
            exit(2) ;

         case 0:
            goto out ;

         default:
            target += r ;
            size -= r ;
            break ;
      }

out :
   *target = 0 ;
   return entry_size - size ;
}
