BEGIN{
  listfile = ARGV[1]

  stimsignal = 1
  while (getline < listfile > 0)
  {
    basename = $1
    bitnum = $2

    if (basename in stimbitwidth)
    { 
      if (int(stimbitwidth[basename]) <= int(bitnum))
        stimbitwidth[basename] = int(bitnum)
      print basename, bitnum
    } 
    else
    { 
      stimbitwidth[basename] = int(bitnum)
      stimsiglist[stimsignal++, "name"] = basename
      print basename, bitnum, "*** new in database ***"
    }
  }
  stimsignal--

  print "total new entries: ", stimsignal
}

