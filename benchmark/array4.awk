# Integer keys (or indexes) rather than string keys
BEGIN {
  if (!base)
    base = 15000

  for (i=0; i<base; i++)
    arr1[i] = base - i

  for (i=5; i<base; i++)
    arr1[i-1] = arr1[i]
}
