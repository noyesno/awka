def abc(x, y, z):
  a = x
  b = y
  c = z
  return ("%d%d%d" % (a, b, c))

def defn(x, y, z):
  d = x
  e = y
  f = z
  return abc(d, e, f)

base = 20000
for i in range(base):
    x = defn(12435345, 93840932, 98921)
    #x = defn("this", "that", "the next thing");
