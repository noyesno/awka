BEGIN {
  a[1] = "aaaaaaa"; a[2] = "bbbbb"; a[3] = "cccccc"; a[4] = "zzzzzzzzzzzzz"
  s = "hello awka"
  $0 = "The rain in Spain"

  if (tst(s) != 10) print "tst(s) error"
  if (tst($0) != 17) print "tst($0) error"
  if (tst(a) != 4) print "tst(a) error"
  if (tst(a[3]) != 6) print "tst(a[3]) error"

  if (length(s) != 10) print "length(s) error"
  if (length() != 17) print "length() error"
  if (length(a[3]) != 6) print "length(a[3]) error"
  if (length != 17) print "length error"
  if (length(a) != alength(a)) print "length(a) != alength(a) error"

  print length, length()
  print length(s), alength(a)
  print length(a[1]), length(a)
  print length(), length(a), length
}

function tst(x) {
  # test length as a lvalue, and in a loop
  while(length(x) > 0) { break; }
  for (i = 0; i < length(x); i++) { continue; }
  return length(x)
}
