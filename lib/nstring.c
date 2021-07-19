/*------------------------------------------------------------*
 | nstring.c                                                  |
 | Copyright 2018,  Sean Zhang (noyesno.net@gmail.com)        |
 |                                                            |
 | This is a source file for string with a length.            |
 | This file is covered by the BSD License.                   |
 *------------------------------------------------------------*/

#ifndef TEST_MAIN
  #include "nstring.h"
#else
  #include <stdio.h>
  #include <string.h>
#endif

int nstring_match(
  const char *pattern, int pattern_size,
  const char *string, int string_size)
{
  #ifdef TEST_MAIN
  printf("\tdebug: %.*s | %.*s\n", pattern_size, pattern, string_size, string);
  #endif

  const char *p = pattern;
  const char *s = string;
  int s_size = string_size;
  int p_size = pattern_size;

  while (p_size>0) {
    int stop = 0;

    switch (*p) {
      case '?' :
        if ( s_size>0 ) {
          s++; s_size--;
          p++; p_size--;
        } else {
          return 0;
        }
        break;
      case '*' :
        if ( p_size==1 ) {
          return 1;
        }
        stop++;
        break;
      default:
        if ( s_size>0 && p[0]==s[0] ) {
          s++; s_size--;
          p++; p_size--;
        } else {
          return 0;
        }
    }

    if (p_size==0) break;

    switch (p[p_size-1]) {
      case '?' :
        if ( s_size>0 ) {
          s_size--;
          p_size--;
        } else {
          return 0;
        }
        break;
      case '*' :
        if ( p_size==1 ) {
          return 1;
        }
        stop++;
        break;
      default :
        if ( s_size>0 && p[p_size-1]==s[s_size-1] ) {
          s_size--;
          p_size--;
        } else {
          return 0;
        }
    }

    if ( stop==2 ) {
        int match = 0;
        for (int i=0; i<s_size; i++) {
          match = nstring_match(p+1, p_size-1, s+i, s_size-i);
          if ( match ) {
            break;
          }
        }
        return match;
    }
  }

  return (s_size==0) ? 1 : 0;
}


#ifdef TEST_MAIN

int main(int argc, char *argv[]){
  const char *pattern;
  const char *string;
  if ( argc==3 ) {
    pattern = argv[1];
    string  = argv[2];
    int match = nstring_match(pattern, strlen(pattern), string, strlen(string));
    printf("%d %s %s\n", match, pattern, string);
    return 0;
  }

  const char *test_case[] = {
    "abc*123",   "abcd1234",  "0",
    "abc*123?",  "abcd1234",  "1",
    "abc*123?",  "abcd12345", "0",
    "abc*123?",  "abcd1239234", "0",
    "abc*123*",  "abcd12345", "1",
    "abc*123",   "abc",       "0",
    NULL, NULL
  };

  for (int i=0; 1 ; i += 3 ) {
    const char *pattern = test_case[i];
    const char *string  = test_case[i+1];
    const char *result  = test_case[i+2];

    if ( !(pattern && string) ) break;

    printf("+++ #%d nstring_match %s %s\n", (i/3+1), pattern, string);

    int match = nstring_match(pattern, strlen(pattern), string, strlen(string));

    printf("=== %s %d # nstring_match \"%s\" \"%s\"\n",
      (result[0]-0x30)==match?"PASS":"FAIL",
      match, pattern, string );
  }
  return 0;
}

#endif
