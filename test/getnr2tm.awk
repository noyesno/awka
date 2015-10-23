function process(w) {
   if(w in ws) {
      printf " : found\n"; lc[p " " w]++; rc[w " " n]++; }
   }
BEGIN {IGNORECASE=1;
      }
/^/ {if(NR % 10 ==0)print "processing line " NR;
     process($1); nlines++;
    }
END {p=w; w=n; n="";
     if(w)process(w); t=1; print NR " lines in " t " sec: " NR+0 " lines/sec;  nlines=" nlines;
    }
#
