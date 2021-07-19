# If you know python, and reckon this script could be written
# to run faster, by all means do so & forward a copy to me.
#
# I'd hate to think that the woeful speed this script runs
# at was as fast as Python can manage.  Otherwise it is yet
# another example of why Awk should be used for processing
# data! ;-)
import os

base = 15000
v3 = "qwerty qwerty qwerty qwerty qwerty qwerty"
v4 = "quincy quincy quincy quincy quincy quincy"
nr = 0

f = open("io.txt", "w")
for i in range(base):
    if (i % 2) == 0:
        f.write(v3+"\n")
    else:
        f.write(v4+"\n")
f.close()

f = open("io.txt", "r")
lines = f.readlines()
for S in lines:
  fields = S.split(" ")
  x = fields[3]
  if nr < 2:
      nr = nr
      #print(x)
  nr = nr + 1
f.close()

#os.system("rm -f io.txt")

