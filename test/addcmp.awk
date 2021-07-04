BEGIN {
  a[0] = 50
  a[1] = "a"
  a[2] = "test string"
  a[3] = SUBSEP

  for (i=0; i<4; i++) print a[i] + 0 == a[i]
  print i + 0 == i
  print "s" + 0 == "s"
}
