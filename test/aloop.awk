function test_aloop(depth, x) {
  print "test_aloop("depth", "x")"
  if (depth == 3) return x;
  for (i in a)
  {
    if (i >= x) 
    {
      continue;
    }
    print depth,i
    j = test_aloop(depth+1, i)
  }
  return x
}

BEGIN {
  a[1] = "1"
  a[2] = "2"
  a[3] = "3"
  a[4] = "4"

  test_aloop(1, 4)
  print "ok"
}
