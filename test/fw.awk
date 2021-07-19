#
# this test of FIELDWIDTHS and SAVEWIDTHS
#
# SAVEWIDTH is Awka specific, so the output
# will be different for Gawk/Mawk/...
#
BEGIN {
  FIELDWIDTHS = "13 2 9 9 9 4 3 6 20"
  OFS = "~"
}
{
  print $5
}
END {
  $1 = "we will fight them on the beaches"
  print $0"."
  SAVEWIDTHS = FIELDWIDTHS
  $1 = "we will fight them on the beaches"
  print $0"."
  SAVEWIDTHS = "13 2 9 9"
  $1 = "we will fight them on the beaches"
  print $0"."
  SAVEWIDTHS = FIELDWIDTHS" 3 4 5"
  $1 = "we will fight them on the beaches"
  print $0"."
}
