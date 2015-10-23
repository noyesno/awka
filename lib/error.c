/*------------------------------------------------------------*
 | error.c                                                    |
 | copyright 1999,  Andrew Sumner (andrewsumner@yahoo.com)    |
 |                                                            |
 | This is a source file for the awka package, a translator   |
 | of the AWK programming language to ANSI C.                 |
 |                                                            |
 | This library is free software; you can redistribute it     |
 | and/or modify it under the terms of the GNU General        |
 | Public License (GPL).                                      |
 |                                                            |
 | This library is distributed in the hope that it will be    |
 | useful, but WITHOUT ANY WARRANTY; without even the implied |
 | warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR    |
 | PURPOSE.                                                   |
 *------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define _IN_LIBRARY
#include "libawka.h"
#include "varg.h"

int _print_mem = 0;

/* uncomment this for debugging purposes
#define a_DUMP_ON_ERROR
*/

void
awka_error( char *fmt, ... )
{
  va_list args;

  va_start( args, fmt );
  vfprintf( stderr, fmt, args );
  va_end( args );

#if defined(MEM_DEBUG) || defined(a_DUMP_ON_ERROR)
  fprintf(stderr,"Awka Library now dumping core as a_DUMP_ON_ERROR is defined...\n");
  raise(SIGSEGV);
#endif

  exit(1);
}


#ifndef a_DUMP_ON_ERROR

void
awka_parachute(int sig)
{
  signal(sig, SIG_DFL);

  fprintf(stderr, "Fatal signal ");
  switch (sig) {
    case SIGSEGV:
      fprintf(stderr, "\"Segmentation Fault\"");
      break;

#ifdef SIGBUS
#if SIGBUS != SIGSEGV
    case SIGBUS:
      fprintf(stderr, "\"Bus Error\"");
      break;
#endif
#endif

#ifdef SIGFPE
    case SIGFPE:
      fprintf(stderr,"\"Floating Point Exception\"");
      break;
#endif

#ifdef SIGQUIT
    case SIGQUIT:
      fprintf(stderr,"\"Keyboard Quit\"");
      exit(-sig);
      break;
#endif

#ifdef SIGPIPE
    case SIGPIPE:
      fprintf(stderr,"\"Broken Pipe\"");
      exit(-sig);
      break;
#endif

    default:
      fprintf(stderr,"# %d", sig);
      break;
  }

  fprintf(stderr, " (trapped by awka_parachute).\n");
  fprintf(stderr, "Please report this to the author of Awka, andrewsumner@yahoo.com\n");
  exit(-sig);
}

void
awka_init_parachute(char *file, int line)
{
  int i;
  int fatal_signals[] = {
    SIGSEGV,
#ifdef SIGBUS
    SIGBUS,
#endif
#ifdef SIGFPE
    SIGFPE,
#endif
#ifdef SIGQUIT
    SIGQUIT,
#endif
#ifdef SIGPIPE
    SIGPIPE,
#endif
    0
  };
  void (*ohandler)(int);

  for (i=0; fatal_signals[i]; i++)
  {
    ohandler = signal(fatal_signals[i], awka_parachute);
    if (ohandler != SIG_DFL)
      signal(fatal_signals[i], ohandler);
  }

#ifdef SIGALRM
  {
    struct sigaction action, oaction;

    memset( &action, 0, sizeof(action) );
    action.sa_handler = SIG_IGN;
    sigaction(SIGALRM, &action, &oaction);

    if (oaction.sa_handler != SIG_DFL)
      sigaction(SIGALRM, &oaction, NULL);
  }
#endif

  return;
}

#endif
