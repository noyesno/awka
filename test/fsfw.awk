#
# test that PROCINFO["FS"] updates as
# changes are made to 
# FIELDWIDTHS, FS and SAVEWIDTHS
#
BEGIN {
  FIELDWIDTHS = "13 2 9 9 9 4 3 6 20"
  OFS = "~"
}
{
  if(!once) {
    once = 1
    print PROCINFO["FS"]
    FIELDWIDTHS = ""
    print PROCINFO["FS"]
    FS = " "
    #SAVEWIDTHS = "13 2 9 9 9 4 3 6 20"
    print PROCINFO["FS"]
    FS = " "
    print PROCINFO["FS"]
    FIELDWIDTHS = "13 2 9 9 9 4 3 6 20"
  }
  print $5,$9
  print $0"."
}
END {
  print ""
  $1 = "we will fight them on the beaches"
  print $0"."
  SAVEWIDTHS = FIELDWIDTHS
  $1 = "we will fight them on the beaches"
  print $0"."
  FIELDWIDTHS = "13 2 9 9"
  SAVEWIDTHS = "13 2 9 9"
  $1 = "we will fight them on the beaches"
  print $0"."
  FIELDWIDTHS = "13 2 9 9 9 4 3 6 20"
  SAVEWIDTHS = FIELDWIDTHS" 3 4 5"
  FIELDWIDTHS = "13 2 9 9 9 4 3 6 20 3 4 5"
  $1 = "we will fight them on the beaches"
  print $0"."
}
