BEGIN {
  print b += b += 1
  print b
  b = 6
  print b += b++
  print b
  print b += ++b
  print b
}
