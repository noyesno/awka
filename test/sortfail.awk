BEGIN {
  for(i=0; i<5; i++) arr[i] = ""sprintf("%c",65+i)
  for (x in arr) print x " -> " arr[x]
  asort(arr, arr2)
  for (x in arr2) print x " -> " arr2[x]
}