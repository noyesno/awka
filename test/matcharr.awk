#
# test output of gawk-compatible match() that sets an optional array
#  that is populated with match start/length data
#
# Requires sorting of output to compare with gawk output
#
BEGIN { 
  print match("cbabc", /(a)(b(c))/, arr); 
  for(k in arr) 
    print k, arr[k] 
}
