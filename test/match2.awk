{ 
  match($0, /(fo+).+(bar*)/, arr)
  print arr[1], arr[2]
  print arr[1, "start"], arr[1, "length"]
  print arr[2, "start"], arr[2, "length"]
}
