BEGIN { 
  if (!base) base = 500
  FS="," 
  line[1] = "a,b,c,d"
  line[2] = "a,\"b\",c,d"
  line[3] = "a,\",commas,surrounded,\",d"
  line[4] = "a,\"a fine \"\"quoted\"\" and ,comma, word \"\"doubledquoted\"\"\",d"
  line[5] = "\"one\",\"two\",\"three\",\"\"\"four\"\"\""
  line[6] = "\"one\",\"\",empty,,field,\"\"\"\",or,,more"
  line[7] = "an \"invalid\" line,will come out how you'd expect"
  line[8] = "another,\",invalid one (two fields; second one's closing quote lost)"

  for (i=0; i<base; i++)
  {
    #while (getline dol0<"data.s1">0)
    #while (getline<"data.s1">0)
    for (j=0; j<8; j++)
    {
      fs = FS
      dol0 = line[j]
      nf = split(dol0, F, FS)
      oldf = newf = 0
      while ( oldf < nf ) {
      #while ( oldf < NF ) {
        f[++newf] = F[++oldf]
        #f[++newf] = $(++oldf)
        if ( f[newf] ~ /^"/ ) {
          while ( gsub(/"/, "\"", f[newf]) % 2 && oldf < NF ) {
            f[newf] = f[newf] fs F[++oldf]
            #f[newf] = f[newf] fs $(++oldf)
          }
          sub(/^"/, "", f[newf])
          sub(/"$/, "", f[newf])
          gsub(/""/, "\"", f[newf])
        }
        n = length(f[newf])
      }
    }
  }
}
