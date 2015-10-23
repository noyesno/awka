BEGIN {
  string1 = "\\\"\\\n"
  bslash = "\\"
  quote = "\""
  nl = "\n"
  string2 = bslash quote bslash nl
  print string1
  print string2

  printf "%s", nl
}
