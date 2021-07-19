#
# currently Awka is different to Gawk - Awka identifies "uvar" as untyped rather than unassigned
#
BEGIN {
  svar = "Hello world"
  ivar = 3
  dvar = 3.1415926
  avar["key1"] = "val1"
  uvar
  avar["key2"] = uvar  # unknown
  u[1]
  $1 = "Hello"
  $2 = 3.1415926
  s2 = $1
  dvar2 = $2
  x=x
  print "svar: "typeof(svar) "\nivar: " typeof(ivar) "\ndvar: " typeof(dvar) "\navar: " typeof(avar) "\nuvar: " typeof(uvar) "\nu[1]: " typeof(u[1]) "\n$1: " typeof($1) "\n$2: " typeof($2) "\ns2: " typeof(s2) "\ndvar2: " typeof(dvar2) "\nunkvar: " typeof(unkvar) "\nx: " typeof(x)
}
