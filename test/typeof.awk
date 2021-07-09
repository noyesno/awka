BEGIN {
  svar = "Hello world"
  ivar = 3
  dvar = 3.1415926
  avar["key1"] = "val1"
  avar["key2"] = uvar  # unknown
  u[1]
  $1 = "Hello"
  $2 = 3.1415926
  s2 = $1
  dvar2 = $2
  print "s: "typeof(svar) " i: " typeof(ivar) " d: " typeof(dvar) " a: " typeof(avar) " u: " typeof(uvar) " u: " typeof(u[1]) " s: " typeof($1) " d: " typeof($2) " s: " typeof(s2) " d: " typeof(dvar2) " u: " typeof(unkvar)
}
