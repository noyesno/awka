#!/usr/local/bin/python3

base = 8000

arr1 = {} 
for i in range(base):
    arr1[str(i)] = base - i

for i in range(5,base):
    arr1[str(i-1)] = arr1[str(i)];

