# tests to ensure conversion to & from split arrays,
# and growth of empty split arrays, all works!
BEGIN {
  abc()
  abc()
  abc()
}
function abc(array) {
  split("",array)
  x = array[6]
  x = array[9]
  x = array[7]
  x = array["hello"]
  print x
}

