function abc(a, b, c) {
  #a = a" abc"
  #b = b" bce"
  #c = c" cde"
  return (a b c)
}

function def(d, e, f) {
  #d = d " 123"
  #e = e " 456"
  #f = f " 789"
  return abc(d, e, f)
}

BEGIN {
  if (!base) base = 20000

  for (i=0; i<base; i++)
    x = def(12435345, 93840932, 98921)
    #x = def("this", "that", "the next thing")
}
