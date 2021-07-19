function rec(a, b) {
  if (a > b)
    return a+1;
  else
    return rec(a+1, b)
}

BEGIN {
  for (i=1; i<20; i++)
    x += rec(1,400)
  #print x
}

