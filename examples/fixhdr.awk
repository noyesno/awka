function readfile(filename) {
  while (getline<filename>0) {
    nf = split($0, R, " ")
    if (R[1] == "<include" && nf >= 2)
      readfile(R[2])
    else
      print $0
  }
}
{
  if ($1 == "<include" && NF >= 2)
    readfile($2)
  else
    print $0
}
