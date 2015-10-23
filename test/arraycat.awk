BEGIN {
  longstr  = "this is quite a long string, dont you think?"
  shortstr = "shorty"

  arr1[longstr, shortstr] = 223
  print arr1[longstr, shortstr]
  arr2[shortstr, longstr] = 224
  print arr2[shortstr, longstr]
}

