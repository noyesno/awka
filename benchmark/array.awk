# The aim of this program is to test loading and retrieval of array elements.
# Note that on 32-bit systems the amount of memory used should be less.
# To reduce the amount of memory used, set base to a smaller number.  This
# will also have the effect of making the test run faster.

BEGIN {
  if (!base)
    base = 3000

  for (i=0; i<base; i++)
  {
    x = "x"i
    arr1[x] = base - i
  }

  for (i=0; i<base; i++)
    arr2[i, base-i] = base - i

  for (x in arr1)
    y = x "" arr1[x]

  for (i=0; i<base; i++)
    if (arr2[i, base-i] != base - i)
    {
      print "Error "i", "base-i" = "arr2[i,base-i]
      exit
    }
}
