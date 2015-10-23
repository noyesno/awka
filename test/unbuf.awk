BEGIN {
  RS = "e"
  while (getline<INFILE>0)
    print "<"$0">"
  close(INFILE)

  getline<"/dev/stdin"; print "["$0"]"
  getline<"/dev/stdin"; print "["$0"]"
  getline<"/dev/stdin"; print "["$0"]"

  RS = "[ld]in"
  while (getline<INFILE>0)
    print "<<"$0">>"

  getline<"/dev/stdin"; print "[["$0"]]"
  getline<"/dev/stdin"; print "[["$0"]]"
  getline<"/dev/stdin"; print "[["$0"]]"
}
