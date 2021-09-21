# string keys rather than integer
BEGIN {
  if (!base)
    base = 8000

  for (i=0; i<base; i++) {
    ind = ""i
    arr1[ind] = base - i
  }

  for (i=5; i<base; i++) {
    ind1 = ""(i-1)
    ind = ""i
    arr1[ind1] = arr1[ind]
  }
}
