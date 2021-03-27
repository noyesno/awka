#ifndef _DEBUG_H
#define _DEBUG_H

// #define DEBUG 0

#if defined(DEBUG) && DEBUG
  #define AWKA_DEBUG(format, ...) fprintf(stderr, "awka-debug: (%s:%d) " format "\n", __FILE__, __LINE__, __VA_ARGS__)
#else
  #define AWKA_DEBUG(format, ...)
#endif


#endif
