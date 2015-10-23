BEGIN {
  while (getline<"manpage">0)
  {
    i++;
    if (i > 20) break
    for (j=1; j<=NF; j++)
      arr[$j] = $j
  }

  count = asort(arr, arr2)

  for (i=1; i<=count; i++)
    print i": "arr2[i]

  count = asort(arr)

  for (i=1; i<=count; i++)
    print i": "arr[i]
}
