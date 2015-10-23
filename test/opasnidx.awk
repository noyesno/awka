BEGIN { 
  b = c = 1; 
  a[b] = 2; 
  a[b++] += 1; 
  print b, a[c]
  
  # the next lines reveal an awka bug as per 0.7.2
  # b = 1;
  # a[b++] += a[b];
  # print b, a[c]
}
