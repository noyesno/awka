BEGIN { FS="," }
{ print "1"$0; n = csvsplit($0); print n } # no output for timing tests

function csvsplit(  n, newf, oldf, fs) {
  fs = FS
  oldf = newf = 0
  while ( oldf < NF ) {
    f[++newf] = $(++oldf)
    if ( f[newf] ~ /^"/ ) {
      while ( gsub(/"/, "\"", f[newf]) % 2 ) {
        if ( oldf >= NF ) {
          if ((getline) > 0) {
            print NR":"$0"."
            oldf = 0
            fs = "\n"
          }
          else break
        } 
        else fs = FS
        f[newf] = f[newf] fs $(++oldf)
      }
      sub(/^"/, "", f[newf])
      sub(/"$/, "", f[newf])
      gsub(/""/, "\"", f[newf])
    }
    n = length(f[newf])
  }
  return newf
}
