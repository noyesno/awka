# The aim of this program is to test the speed with which large amounts
# of io are handled, in association with field splitting.
# Make sure you have about 100mb of free disk space before you run this test.

BEGIN {
  if (!base)
    base = 15000
  #print "base = "base >"/dev/stderr"

  v3 = "qwerty qwerty qwerty qwerty qwerty qwerty"
  v4 = "quincy quincy quincy quincy quincy quincy"

  for (i=0; i<base; i++)
  {
    if (!(i % 2))
      print v3 >"io.txt"
    else
      print v4 >"io.txt"
  }
  close("io.txt")

  nr = 0
  while (getline<"io.txt">0)
  {
    x = $3
    if (nr++ < 2) {}
      #print x
  }

  system("rm -f io.txt");
}

