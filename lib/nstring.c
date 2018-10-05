/*------------------------------------------------------------*
 | nstring.c                                                  |
 | Copyright 2018,  Sean Zhang (noyesno.net@gmail.com)        |
 |                                                            |
 | This is a source file for string with a length.            |
 | This file is covered by the BSD License.                   |
 *------------------------------------------------------------*/

#include "nstring.h"

int nstring_match(
  const char *pattern, int pattern_size,
  const char *string, int string_size)
{
  const char *p = pattern;
  const char *s = string;
  int s_size = string_size;
  int p_size = pattern_size;

  while(p_size>0){
    switch(*p){
      case '?' :
        if( s_size>0 ){
          s++; s_size--;
        }else{
          return 0;
        }
        break;
      case '*' :
        if( p_size==1 ){
          return 1;
        }
        int match = 0;
        for(int i=0; i<s_size; i++){
          match = nstring_match(p+1, p_size-1, s+i, s_size-i); 
          if( match ){
            break;
          }
        }
        return match; 
      default:
        if( s_size>0 && p[0]==s[0] ){
          s++; s_size--;
        } else {
          return 0;
        }
    }
    p++; p_size--;
  }

  return s_size==0?1:0;
}
