BEGIN {
   k=0;
   scalefilefound=0;
   problems=0;
   #scalefile = "hh.in"

# read scale file
   fileok=(getline < scalefile);
   while( fileok > 0 )
   {
      scalefilefound=1;
      # print "x "$0;
      if(NF==4 && $1!=";") {
         k++;
      }

      fileok=(getline < scalefile);
   }

   if(scalefilefound==0) problems=1;
   close(scalefile);

   print "noofcolors: "k;

}

