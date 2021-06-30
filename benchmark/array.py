#!/usr/local/bin/perl

base = 3000

arr1 = {}
for i in range(base):
    x = "x" + str(i)
    arr1[x] = base - i

arr2 = {}
for i in range(base):
    arr2[(i, base-i)] = base - i

for x in arr1:
    y = str(x) + str(arr1[x])

for i in range(base):
    if arr2[(i, base-i)] != base - i:
      print("Error ", i, ", ", base-i, " = ", arr2[(i,base-i)])
      quit()
