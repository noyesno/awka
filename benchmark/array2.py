#!/usr/local/bin/python3

base = 50000

X = {}
for i in range(50):
  X[i] = str(i) + "." + str(i)

for j in range(base): 
  for i in range(49):
    X[i] = X[i+1]
  X[i] = X[0]

