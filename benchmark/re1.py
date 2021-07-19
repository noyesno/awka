#!/usr/local/bin/python3
import re

base = 100000

s = "she sells seashells"

prog = re.compile("s \w")
result = prog.search(s)

#print(result.span()[0]+1)

for i in range(base):
  x1 = prog.search(s)
  x = x1.span()[0]+1

