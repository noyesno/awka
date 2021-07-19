BEGIN {
  if (!base) base = 18000

  v1 = 3
  v3 = "qwerty qwerty qwerty qwerty qwerty qwerty"
  v4 = "."
  v5 = "-"
  v2 = "53.24928745"

  for (i=0; i<base; i++)
  {
    sprintf(xx, "v1->ival = %d, v2->ptr = %s\n",v1,v2)
    x1 = v1 + v2
    x2 = v1 + v2 + x1

    x3 = (v1+v2)-v1
    x4 = v3 v1 v4 v2 v3
    x5 = (v5 v1 v4 v2 v3) - v2
  }
}
