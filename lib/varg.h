/*--------------------------------------------------*
 | varg.h                                           |
 | Part of the Awka Library,                        |
 | Copyright (c) 1999, Andrew Sumner.               |
 | This file is covered by the GNU General Public   |
 | License (GPL).                                   |
 *--------------------------------------------------*/

#ifndef _VARG_H
#define _VARG_H

#ifdef     NO_PROTOS
#  ifndef    NO_STDARG_H   
#    define    NO_STDARG_H  1
#  endif
#endif

#if     NO_STDARG_H
#  include <varargs.h>
#else  /* have stdarg.h */
#  include <stdarg.h>
#endif

#endif
