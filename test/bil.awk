# contributed by chris proctor

BEGIN{FS=" *|,|:"}
{
  print $0
  i=0
  while (i<=NF){
    print $i "  i=$" i
    i++
  }       
}       

