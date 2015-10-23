{
  if($0 in a)
  {
    printf("line %d has been seen before at line %d\n",  NR, a[$0])
    repeat_count += 1
  }
  else
  {
    a[$0] = NR
  }
  count += 1
}
END {
  printf("%d %f%%\n", repeat_count, repeat_count / count * 100)
}
