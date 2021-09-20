BEGIN { 
  print match("abc", /(a).(c)/, ar); 
  print length(ar)
  if(1 in ar)
    print "ar[1]: " ar[1] " start: " ar["1\034start"] " len: " ar["1\034length"]; 
  #for(k in ar) 
  #  print k, ar[k] 
  print "-----------------"
  print match("abc", /a.c/, ar); 
  print length(ar)
  if(1 in ar)
    print "ar[1]: " ar[1] " start: " ar["1\034start"] " len: " ar["1\034length"]; 
  #for(k in ar) 
  #  print k, ar[k] 
  print "-----------------"
  print match("abc", /(x).(z)/, ar); 
  print length(ar)
  if(1 in ar)
    print "ar[1]: " ar[1] " start: " ar["1\034start"] " len: " ar["1\034length"]; 
  #for(k in ar) 
  #  print k, ar[k] 
  print "-----------------"
}
