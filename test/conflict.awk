function min(a, b)
{
  return (a < b ? a : b) + 3
}

BEGIN {

  trim[15] = 3
  right = "this number " left("should be equality in gender", 15)

  right = right " to " min(trim[15], 2) + max(trim[15], 3)

  print right
}
  
