BEGIN{
  getline < ARGV[1]
  for (e=9; e>=0; e--)
    stimpatbitlist[e] = $(10-e)
    
  getline < ARGV[2]
  for (d=1; d<=length($0); d++)
  {
    pattern[d] = "1"
  }
  for (e=9; e>= 0; e--)
  {
    x = e""
    print pattern[x]
  }
}

