{ gsub(/^.*PatternBit= */, "", $0)
  maxstimpatbit = ($0 > maxstimpatbit? $0 : maxstimpatbit)
  print $0, maxstimpatbit
}
