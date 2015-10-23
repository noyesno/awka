BEGIN{
  patfile = ARGV[1]

  while (getline $0 < patfile > 0)
  { if ($2 == ":")
    { $2 = ""
      $1 = ""
    }
    gsub (/ /, "", $0)
    print $0
  }
}
