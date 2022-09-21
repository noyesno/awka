function rec(a, b) {
  if (a > b)
    return a+1;
  else
    return rec(a+1, b)
}

BEGIN {
  base = 20
  for (i=1; i<base; i++)
    x += rec(1,400)
  #print x
}

