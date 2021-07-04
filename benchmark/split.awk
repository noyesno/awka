BEGIN {
  if (!base) base = 2000

  str = "Newcastle Knights are the best there is"

  print "space split" >"/dev/stderr"
  for (i=0; i<base; i++)
  {
    x = split(str, S, " ")
    for (j=1; j<=x; j++)
      s = S[j]
  }

  print "single char" >"/dev/stderr"
  for (i=0; i<base; i++)
  {
    x = split(str, S, "a")
    for (j=1; j<=x; j++)
      s = S[j]
  }

  print "regex" >"/dev/stderr"
  for (i=0; i<base; i++)
  {
    x = split(str, S, " |t")
    for (j=1; j<=x; j++)
      s = S[j]
    print str >"x.tmp"
  }
  close("x.tmp")
  ARGV[1] = "x.tmp"
  ARGC++
  print "field split" >"/dev/stderr"
}
{
  for (i=1; i<=NF; i++)
    s = $i
}
END {
  cmd = "rm -f x.tmp"
  system(cmd)
}
