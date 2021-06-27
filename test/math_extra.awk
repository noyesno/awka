# Non-Standard math functions that match what is available in C libraries
#
# This can not be tested in other AWK versions, as most of these functions are only in AWKA
#
BEGIN {
  OFMT="%.15g"
  print "tan(0.5):      ", sin(0.5)/cos(0.5)
  print "tan(0.5):      ", tan(0.5)
  print "asinh(0.5):    ", asinh(0.5)
  print "atanh(0.5):    ", atanh(0.5)
  print "atan2(10,-10): ", atan2(10,-10)
  print "erf(0.5):      ", erf(0.5)
  print "erfc(0.5):     ", erfc(0.5)
  print "hypot(2,3):    ", hypot(2,3)
  print "tgamma(0.5):   ", tgamma(0.5)
  print "ceil(2.5):     ", ceil(2.5)
  print "floor(2.5):    ", floor(2.5)
  print "trunc(2.5):    ", trunc(2.5)
  print "round(2.5):    ", round(2.5)
  print "mod(27,5):     ", mod(27,5), (27 % 5)
  print "pow(5,3):      ", pow(5,3) , (5 ^ 3)
  print xor(0,0), xor(0,1), xor(1,0), xor(1,1)
}
