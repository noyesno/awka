base = 20000
for i in range(base):
  x = "hello%d-%d" % ( ((i + (30 - (i % 30)))-30), ((i + (30 - (i % 30)))-1) )
