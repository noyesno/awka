{
  num=split($0, rec, /;/)
  printf("%s\n", num)
  for(x in rec) print i++","rec[x]
}
