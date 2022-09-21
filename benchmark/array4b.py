#!/usr/local/bin/python3

base = 8000

arr1 = {} 
for i in range(base):
    si = str(i)
    arr1[si] = base - i

for i in range(5,base):
    sind = str(i-1)
    si = str(i)
    arr1[sind] = arr1[si];

