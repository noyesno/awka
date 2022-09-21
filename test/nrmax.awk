# split input by char to test Field Splitting limits 
# MAX_SPLIT is (FBANKSZ-1) and must be divisible by 3
BEGIN {
  FS = ""
}
{
  print "NF:  " NF 
  for(i=1; i <= 10; i++)
    print "$" i ": " $i 
  for(i=NF-10; i <= NF; i++)
    print "$" i ": " $i 
}
