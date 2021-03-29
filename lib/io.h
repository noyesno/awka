/*--------------------------------------------------*
 | io.h                                             |
 | Header file for io.c, part of the Awka           |
 | Library, Copyright 1999, Andrew Sumner.          |
 | This file is covered by the GNU General Public   |
 | License (GPL).                                   |
 *--------------------------------------------------*/

#ifndef _IO_H
#define _IO_H

#define A_BUFSIZ 16384 

#define _a_IO_CLOSED 0
#define _a_IO_READ   1
#define _a_IO_WRITE  2
#define _a_IO_APPEND 4
#define _a_IO_EOF    8

enum AwkaStreamType {
  AWKA_STREAM_UNKNOWN,
  AWKA_STREAM_FILE,
  AWKA_STREAM_PIPE,
  AWKA_STREAM_SOCKET
};

typedef enum AwkaStreamType AwkaStreamType;

typedef struct {
  AwkaStreamType type;
  char *name;       /* name of output file or device */
  FILE *fp;         /* file pointer */
  int   fd;         /* file descriptor */
  char *buf;        /* input buffer */
  char *current;    /* where up to in buffer */
  char *end;        /* end of data in buffer */
  int alloc;        /* size of input buffer */
  char io;          /* input or output stream flag */
  char pipe;        /* true/false */
  char lastmode;    /* for |& this records whether stream was last read from
                       or written to */
  char interactive; /* whether from a /dev/xxx stream or not */
} _a_IOSTREAM;

#ifndef _IO_C
extern _a_IOSTREAM *_a_iostream;
extern int _a_ioallc, _a_ioused;
#endif

int _awka_io_addstream( char *name, char flag, int pipe );
int awka_io_readline( a_VAR *var, int, int );
void awka_exit(double);
void awka_cleanup();
int _awka_wait_pid(int);

#endif
