# Test using getline reading in the BEGIN section
# Reading one value results in ++c being run twice
# and the output is "2"

BEGIN {
  system("echo 1 > _f.out")
  while ((getline a[++c] < "_f.out") > 0) { }
  print c
}
