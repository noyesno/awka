# forcing int keys (acting like array indexes)
BEGIN {
  if (!base)
    base = 15000

  for (i=0; i<base; i++) {
    arr1[0+i] = base - i
  }

  for (i=5; i<base; i++) {
    arr1[0+i-1] = arr1[0+i]
  }

}
