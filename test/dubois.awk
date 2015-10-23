# contributed by John H DuBois III

BEGIN {
  c = d = "d"

  x = ((a == "b") || (c == "d"))
  print x
  x = ("x" in y || "z" in y)
  print x
  s = "a" || (c && d)
  print x
}
