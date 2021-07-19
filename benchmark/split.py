import sys, re, os

base = 2000

str = "Newcastle Knights are the best there is"

print("space split", file=sys.stderr)

for i in range(base):
  S = str.split(" ")
  x = len(S)
  for j in range(x):
    s = S[j]

print("single char", file=sys.stderr)

for i in range(base):
  S = str.split("a")
  x = len(S)
  for j in range(x):
    s = S[j]

print("regex", file=sys.stderr)

f = open("x.tmp", "w")
for i in range(base):
  S = re.split(" |t", str)
  x = len(S)
  for j in range(x):
    s = S[j]
  f.write(str + "\n")
f.close()

print("field split", file=sys.stderr)

f = open("x.tmp", "r")
lines = f.readlines()
for S in lines:
  fields = S.split(" ")
  NF = len(fields)
  for i in range(NF):
    s = fields[i]
f.close()

os.system("rm -f x.tmp")

