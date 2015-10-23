# contributed by chris proctor (bil@linuxstart.com)

/^#include/ { 
  while (getline x <$2>0) 
    print x; 
  next 
} 
{ print }
