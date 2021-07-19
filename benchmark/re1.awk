# MAWK does not handle \w word boundaries ??
#
BEGIN {
  #print match("she sells seashells", /s \w/) 
  x = match("she sells seashells", /s \w/)

  if(!base) base = 100000

  for(i=0; i<base; i++) {
    x = match("she sells seashells", /s \w/) 
  }
}
