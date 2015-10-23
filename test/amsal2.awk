BEGIN{
  pdef_out = ARGV[1]
  stimsignal = 0
  while (getline < pdef_out > 0)
  { stimsignal++
    stimsiglist[stimsignal, "level"] = $2
  }
  for (d=1; d<=stimsignal; d++)
    print d, stimsignal
}
