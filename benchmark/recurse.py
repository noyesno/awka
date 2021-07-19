def rec(x, y):
  a = x
  b = y
  if (a > b):
    return a+1
  else:
    return rec(a+1, b)

base = 20
x = 0
for i in range(1, base):
    x += rec(1, 400)
#print(x, '\n')
