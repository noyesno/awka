dnl 
dnl custom awka macros for autoconf
dnl 
dnl **********  look for sockets library *****************
dnl 
dnl Borrowed from Gawk-3.1.0
dnl
AC_DEFUN(AWKA_AC_LIB_SOCKETS, [
awka_have_sockets=no
SOCKET_DEFINE=0
# Check for system-dependent location of socket libraries

SOCKET_LIBS=
if test "$ISC" = yes; then
  SOCKET_LIBS="-lnsl_s -linet"
else
  # Martyn.Johnson@cl.cam.ac.uk says this is needed for Ultrix, if the X
  # libraries were built with DECnet support.  And karl@cs.umb.edu says
  # the Alpha needs dnet_stub (dnet does not exist).
  #
  # ADR: Is this needed just for sockets???
#  AC_CHECK_LIB(dnet, dnet_ntoa, [SOCKET_LIBS="$SOCKET_LIBS -ldnet"])
#  if test $ac_cv_lib_dnet_ntoa = no; then
#    AC_CHECK_LIB(dnet_stub, dnet_ntoa,
#                                                                                                   [SOCKET_LIBS="$SOCKET_LIBS -ldnet_stub"])
#  fi                
    
  # msh@cis.ufl.edu says -lnsl (and -lsocket) are needed for his 386/AT,
  # to get the SysV transport functions.
  # chad@anasazi.com says the Pyramid MIS-ES running DC/OSx (SVR4)
  # needs -lnsl.
  # The nsl library prevents programs from opening the X display
  # on Irix 5.2, according to dickey@clark.net.
  AC_CHECK_FUNC(gethostbyname)
  if test $ac_cv_func_gethostbyname = no; then
    AC_CHECK_LIB(nsl, gethostbyname, SOCKET_LIBS="$SOCKET_LIBS -lnsl")
  fi

  # lieder@skyler.mavd.honeywell.com says without -lsocket,
  # socket/setsockopt and other routines are undefined under SCO ODT
  # 2.0.  But -lsocket is broken on IRIX 5.2 (and is not necessary
  # on later versions), says simon@lia.di.epfl.ch: it contains
  # gethostby* variants that don't use the nameserver (or something).
  # -lsocket must be given before -lnsl if both are needed.
  # We assume that if connect needs -lnsl, so does gethostbyname.
  AC_CHECK_FUNC(connect)
  if test $ac_cv_func_connect = no; then
    AC_CHECK_LIB(socket, connect, SOCKET_LIBS="-lsocket $SOCKET_LIBS"
                                  awka_have_sockets=yes, ,
                                           $SOCKET_LIBS)
  else
    awka_have_sockets=yes
  fi
fi                                                                                           
if test "${awka_have_sockets}" = "yes"
then
        SOCKET_DEFINE=1
        AC_MSG_CHECKING([where to find the socket library calls])
        case "${SOCKET_LIBS}" in
        ?*)     awka_lib_loc="${SOCKET_LIBS}" ;;
        *)      awka_lib_loc="the standard library" ;;
        esac
        AC_MSG_RESULT([${awka_lib_loc}])                                                     
        AC_DEFINE(HAVE_SOCKETS)
fi
echo "X awka_SOCKET_LIBS \"$SOCKET_LIBS\"" >> defines.out
AC_SUBST(SOCKET_LIBS)dnl
])dnl 
dnl
dnl **********  look for math library *****************
dnl
define(ANDREW, andrewsumner@yahoo.com)
dnl
define(LOOK_FOR_MATH_LIBRARY,[
if test "${MATHLIB+set}" != set  ; then
AC_CHECK_LIB(m,log,[MATHLIB=-lm ; LIBS="$LIBS -lm"],
[# maybe don't need separate math library
AC_CHECK_FUNC(log, log=yes)
if test "$log$" = yes
then
   MATHLIB='' # evidently don't need one
else
   AC_MSG_ERROR(
Cannot find a math library. You need to set MATHLIB in config.user)
fi])dnl
fi
echo "X awka_MATHLIB \"$MATHLIB\"" >> defines.out
AC_SUBST(MATHLIB)])dnl
dnl
dnl *********  utility macros **********************
dnl
dnl  I can't get AC_DEFINE_NOQUOTE to work so give up
define([XDEFINE],[AC_DEFINE($1)
echo  X '$1' 'ifelse($2,,1,$2)' >> defines.out])dnl
define([XXDEFINE],
[echo  X '$1' '$2' >> defines.out])dnl
dnl
dnl
dnl We want #define NO_STRERROR
dnl instead of #define HAVE_STRERROR
dnl
dnl
define([XADD_NO],[NO_[$1]])dnl 
define([ADD_NO], [XADD_NO(translit($1, a-z. , A-Z_))])dnl
define([HEADER_CHECK],[AC_CHECK_HEADER($1, ,XDEFINE(ADD_NO($1)))])dnl
define([FUNC_CHECK],[AC_CHECK_FUNC($1, ,XDEFINE(ADD_NO($1)))])dnl
dnl
dnl how to repeat a macro on a list of args
dnl (probably won't work if the args are expandable
dnl
define([REPEAT_IT],
[ifelse($#,1,[$1],$#,2,[$1($2)],
[$1($2) 
REPEAT_IT([$1],
builtin(shift,builtin(shift,$*)))])])dnl

define([CHECK_HEADERS],[REPEAT_IT([HEADER_CHECK],$*)])dnl
define([CHECK_FUNCTIONS],[REPEAT_IT([FUNC_CHECK],$*)])dnl
dnl
dnl ******* find size_t ********************
dnl
define([SIZE_T_CHECK],[
  [if test "$size_t_defed" != 1 ; then]
   AC_CHECK_HEADER($1,size_t_header=ok)
   [if test "$size_t_header" = ok ; then]
   AC_TRY_COMPILE([
#include <$1>],
[size_t *n ;
], [size_t_defed=1;
XXDEFINE($2,1)
echo getting size_t from '<$1>'])
[fi;fi]])dnl
define(WHERE_SIZE_T,
[SIZE_T_CHECK(stddef.h,SIZE_T_STDDEF_H)
SIZE_T_CHECK(sys/types.h,SIZE_T_TYPES_H)])dnl
dnl
dnl  **********  .exe extension ******************
dnl
define(EXE_EXTENSION,
[AC_MSG_CHECKING(for .exe extension)
AC_TRY_COMPILE(
[int this() { return __CYGWIN32__; }] ,
[return this();],cygnus=".exe",cygnus=no)
SHARED_LIB=''
SHAREDFLAG=''
if test "$cygnus" = no
then
  AC_TRY_COMPILE(
  [int this() { return __CYGWIN__; }] ,
  [return this();],cygnus=".exe",cygnus=no)
fi
if test "$cygnus" = no
then
  AC_TRY_COMPILE(
  [int this() { return __DJGPP__; }] ,
  [return this();],djgpp=".exe",djgpp=no)
  AC_MSG_RESULT($djgpp)
  if test "$djgpp" = no
  then
    EXE=''
    if test "$GCC" = yes
    then
      SHARED_LIB='libawka.so'
      SHAREDFLAG='-fPIC'
    fi
  else
    EXE=$djgpp
  fi
else
  AC_MSG_RESULT($cygnus)
  EXE=$cygnus
fi
AC_SUBST(SHARED_LIB)
AC_SUBST(SHAREDFLAG)
AC_SUBST(EXE)])dnl
dnl
dnl  **********  check compiler ******************
dnl
define(COMPILER_ATTRIBUTES,
[AC_MSG_CHECKING(compiler supports void*)
AC_TRY_COMPILE(
[char *cp ;
void *foo() ;] ,
[cp = (char*)(void*)(int*)foo() ;],void_ptr=yes,void_ptr=no)
AC_MSG_RESULT($void_ptr)
test "$void_ptr" = no && XXDEFINE(NO_VOID_PTR,1)
AC_MSG_CHECKING(compiler groks prototypes)
AC_TRY_COMPILE(,[int x(char*);],protos=yes,protos=no)
AC_MSG_RESULT([$protos])
test "$protos" = no && XXDEFINE(NO_PROTOS,1)
AC_C_CONST
test "$ac_cv_c_const" = no && XXDEFINE(const)])dnl
dnl
dnl
dnl
dnl  **********  check inline ******************
dnl
define(GOT_INLINE,
[AC_MSG_CHECKING(checking for inline)
AC_TRY_COMPILE(
[static __inline__ int this() { return 0; }] ,
[return this();],inline="__inline__",inline=no)
if test "$inline" = no
then
AC_TRY_COMPILE(
[static __inline int this() { return 0; }] ,
[return this();],inline="__inline",inline=no)
if test "$inline" = no
then
AC_TRY_COMPILE(
[static inline int this() { return 0; }] ,
[return this();],inline="inline",inline=no)
AC_MSG_RESULT($inline)
if test "$inline" = no
then
  XXDEFINE(INLINE)
else
  XXDEFINE(INLINE,inline)
fi
else
  AC_MSG_RESULT($inline)
  XXDEFINE(INLINE,__inline)
fi
else
  AC_MSG_RESULT($inline)
  XXDEFINE(INLINE,__inline__)
fi])dnl
dnl
dnl
dnl
dnl **********  which awk ***********
define(WHICH_AWK,
[AC_CHECK_PROGS(AWK, gawk mawk nawk awk)
if test "$AWK" = ""
then
   AC_MSG_ERROR(
Cannot find an awk.  Correct your PATH, or install an awk if necessary.)
fi
])dnl
dnl
dnl **********  which shell ***********
define(WHICH_SH,
[AC_CHECK_PROGS(SH, ash sh bash ksh tcsh csh)
if test "$SH" = ""
then
   AC_MSG_ERROR(
Cannot find a shell program.  Correct your PATH, or install sh if necessary.)
fi
])dnl
dnl
dnl **********  which yacc ***********
define(WHICH_YACC,
[AC_CHECK_PROGS(YACC, byacc bison yacc)
test "$YACC" = bison && YACC='bison -y'])dnl
dnl
dnl **********  which ar ***********
define(WHICH_AR,
[AC_CHECK_PROGS(AR, ar gar)])dnl
dnl
dnl **********  which ranlib ***********
define(WHICH_RANLIB,
[AC_CHECK_PROGS(RANLIB, ranlib touch echo)])dnl
dnl
dnl **********  which cmp ***********
define(WHICH_CMP,
[AC_TRY_COMPILE(
[int this() { return __CYGWIN32__; }] ,
[return this();],cmp="diff",cmp="cmp")
if test "$cmp" = "cmp"
then
  AC_TRY_COMPILE(
  [int this() { return __CYGWIN__; }] ,
  [return this();],cmp="diff",cmp="cmp")
fi
if test "$cmp" = "cmp"
then
  AC_TRY_COMPILE(
  [int this() { return __DJGPP__; }] ,
  [return this();],cmp="diff",cmp="cmp")
fi
AC_CHECK_PROGS(CMP, $cmp)])dnl
dnl
dnl *************  header and footer for config.h *******************
dnl
define(CONFIG_H_HEADER,
[cat<<'EOF'
/* config.h -- generated by configure */
#ifndef CONFIG_H
#define CONFIG_H

EOF])dnl
define(CONFIG_H_TRAILER,
[cat<<'EOF'

#define HAVE_REAL_PIPES 1
/* #define NO_BIN_CHARS */
/* #define MEM_DEBUG */
#endif /* CONFIG_H */
EOF])dnl
dnl
dnl *************  build config.h ***********************
define(DO_CONFIG_H,
[# output config.h
rm -f config.h
(
CONFIG_H_HEADER
echo "X awka_CC \"$CC\"" >> defines.out
echo "X awka_CFLAGS \"$CFLAGS\"" >> defines.out
echo "X awka_shell \"$SH\"" >> defines.out
[if [ "$SOCKET_DEFINE" = "1" ]; then echo "#define HAVE_SOCKETS 1"; fi];
[if [ "$HAVE_PORTALS" = "1" ]; then echo "#define HAVE_PORTALS 1"; fi];
[if [ "${ac_cv_header_sys_socket_h}" = "yes" ]; then echo "#define HAVE_SYS_SOCKET_H"; fi];
[if [ "${ac_cv_header_unistd_h}" = "yes" ]; then echo "#define HAVE_UNISTD_H"; fi];
[if [ "${ac_cv_header_netdb_h}" = "yes" ]; then echo "#define HAVE_NETDB_H"; fi];
[if [ "${ac_cv_header_netinet_in_h}" = "yes" ]; then echo "#define HAVE_NETINET_IN_H"; fi];
[if [ "${ac_cv_header_fcntl_h}" = "yes" ]; then echo "#define HAVE_FCNTL_H"; fi];
[sed 's/^X/#define/' defines.out]
CONFIG_H_TRAILER
) | sed 's/SYS\//SYS_/' | sed 's/INET\//INET_/' | sed 's/NO_"/NO_/' | sed 's/_H"/_H/' | tee config.h
rm defines.out])dnl
dnl
dnl
dnl *************** [sf]printf checks needed for print.c ***********
dnl
dnl sometimes fprintf() and sprintf() are not proto'ed in
dnl stdio.h
define(FPRINTF_IN_STDIO,
[AC_EGREP_HEADER([[[^v]]fprintf],stdio.h,,XDEFINE(NO_FPRINTF_IN_STDIO))
AC_EGREP_HEADER([[[^v]]sprintf],stdio.h,,XDEFINE(NO_SPRINTF_IN_STDIO))])dnl
dnl
dnl  **************************************************
dnl  C program to compute MAX__INT and MAX__LONG
dnl  if looking at headers fails
define([MAX__INT_PROGRAM],
[[#include <stdio.h>
int main()
{ int y ; long yy ;
  FILE *out ;

    if ( !(out = fopen("maxint.out","w")) ) exit(1) ;
    /* find max int and max long */
    y = 0x1000 ;
    while ( y > 0 ) y *= 2 ;
    fprintf(out,"X MAX__INT 0x%x\n", y-1) ;
    yy = 0x1000 ;
    while ( yy > 0 ) yy *= 2 ;
    fprintf(out,"X MAX__LONG 0x%lx\n", yy-1) ;
    exit(0) ;
    return 0 ;
 }]])dnl
dnl
dnl *** Try to find a definition of MAX__INT from limits.h else compute***
dnl
define(FIND_OR_COMPUTE_MAX__INT,
[AC_CHECK_HEADER(limits.h,limits_h=yes)
if test "$limits_h" = yes ; then :
else
AC_CHECK_HEADER(values.h,values_h=yes)
   if test "$values_h" = yes ; then
   AC_TRY_RUN(
[#include <values.h>
#include <stdio.h>
int main()
{   FILE *out = fopen("maxint.out", "w") ;
    if ( ! out ) exit(1) ;
    fprintf(out, "X MAX__INT 0x%x\n", MAXINT) ;
    fprintf(out, "X MAX__LONG 0x%lx\n", MAXLONG) ;
    exit(0) ; return(0) ;
}
], maxint_set=1,[MAX_INT_ERRMSG])
   fi
if test "$maxint_set" != 1 ; then 
# compute it  --  assumes two's complement
AC_TRY_RUN(MAX__INT_PROGRAM,:,[MAX_INT_ERRMSG])
fi
cat maxint.out >> defines.out ; rm -f maxint.out
fi ;])dnl
dnl
define(MAX_INT_ERRMSG,
[AC_MSG_ERROR(C program to compute maxint and maxlong failed.
Please send bug report to ANDREW.)])dnl
dnl
dnl **********  input config.user ******************
define(GET_USER_DEFAULTS,
[cat < /dev/null > defines.out
test -f config.user && . ./config.user
NOTSET_THEN_DEFAULT(MANEXT,1)
NOTSET_THEN_DEFAULT(BUILDLIB,a)
echo "$USER_DEFINES" >> defines.out])
dnl
dnl ************************************************
dnl
define([NOTSET_THEN_DEFAULT],
[test "[$]{$1+set}" = set || $1="$2"
AC_SUBST($1)])dnl
dnl
dnl ******************  sysV and solaris fpe checks ***********
dnl  
define(LOOK_FOR_FPE_SIGINFO,
[AC_CHECK_FUNC(sigaction, sigaction=1)
AC_CHECK_HEADER(siginfo.h,siginfo_h=1)
if test "$sigaction" = 1 && test "$siginfo_h" = 1 ; then
   XDEFINE(SV_SIGINFO)
else
   AC_CHECK_FUNC(sigvec,sigvec=1)
   if test "$sigvec" = 1 && ./fpe_check phoney_arg >> defines.out ; then :
   else XDEFINE(NOINFO_SIGFPE)
   fi
fi])
dnl
dnl
dnl ******** AC_PROG_CC with defaultout -g to cflags **************
dnl 
AC_DEFUN([PROG_CC_NO_MINUS_G_NONSENSE],
[AC_BEFORE([$0], [AC_PROG_CPP])dnl
AC_CHECK_PROG(CC, gcc, gcc, cc)
dnl
AC_MSG_CHECKING(whether we are using GNU C)
AC_CACHE_VAL(ac_cv_prog_gcc,
[dnl The semicolon is to pacify NeXT's syntax-checking cpp.
cat > conftest.c <<EOF
#ifdef __GNUC__
  yes;
#endif
EOF
if ${CC-cc} -E conftest.c 2>&AC_FD_CC | egrep yes >/dev/null 2>&1; then
  ac_cv_prog_gcc=yes
  GCC=yes
else
  ac_cv_prog_gcc=no
  GCC=no
fi])dnl
AC_MSG_RESULT($ac_cv_prog_gcc)
rm -f conftest*
])dnl
dnl
