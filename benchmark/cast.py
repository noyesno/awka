base = 18000

v1 = 3
v3 = "qwerty qwerty qwerty qwerty qwerty qwerty"
v4 = "."
v5 = "-"
v2 = "53.24928745"

for i in range(base):
  xx = "v1->ival = %d, v2->ptr = %s\n" % (v1,v2);
  x1 = v1 + float(v2);
  x2 = v1 + float(v2) + x1;

  x3 = (v1+float(v2))-v1;
  x4 = v3+str(v1)+v4+v2+v3;
  #x5 = (v5+str(v1)+v4+v2+v3) - v2;

