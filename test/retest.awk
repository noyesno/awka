BEGIN {
  string = "+ a Sample $str1ng."

  if (match(string, /+/))
  {
    print RSTART, RLENGTH
    print "ok 1"
  }
  else
    print "fail 1"

  if (match(string, /Sampl/))
  {
    print RSTART, RLENGTH
    print "ok 2"
  }
  else
    print "fail 2"

  if (match(string, /./))
  {
    print RSTART, RLENGTH
    print "ok 3"
  }
  else
    print "fail 3"

  if (match(string, /\./))
  {
    print RSTART, RLENGTH
    print "ok 4"
  }
  else
    print "fail 4"

  if (match(string, /\\./))
  {
    print RSTART, RLENGTH
    print "fail 4a"
  }
  else
    print "ok 4a"

  if (match(string, /[Ss]/))
  {
    print RSTART, RLENGTH
    print "ok 5"
  }
  else
    print "fail 5"

  if (match(string, /\$/))
  {
    print RSTART, RLENGTH
    print "ok 6"
  }
  else
    print "fail 6"

  if (match(string, /pel|ple/))
  {
    print RSTART, RLENGTH
    print "ok 7"
  }
  else
    print "fail 7"

  if (match(string, /(am)+p[Ll]/))
  {
    print RSTART, RLENGTH
    print "ok 8"
  }
  else
    print "fail 8"

  string = "hello\rbyebye"
  if (match(string, /o\rb/))
  {
    print RSTART, RLENGTH
    print "ok 9"
  }
  else
    print "fail 9"

  if (match(string, "o\rb"))
  {
    print RSTART, RLENGTH
    print "ok 10"
  }
  else
    print "fail 10"
}

