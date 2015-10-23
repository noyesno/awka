BEGIN{
  tmp="cox.in"
  while(getline<tmp>0){
    sub("=",".",$2)
    print $1, $2
  }
  close(tmp)
}
