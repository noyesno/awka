BEGIN {
  str = "\\a\\b\\c"
  x = str
  gsub( "\\\\", "\\\\", x)
  print x

  x = str
  gsub(/\\\\/, "\\\\", x)
  print x
}
