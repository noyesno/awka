BEGIN {
  str = "1 2 3 4 5"
  nw = split(str, word)
  for (i=1; i<=nw; i++) print "word["i"] = "word[i]
  print "###---###"
  for (i in word) print "word["i"] = "word[i]
}
