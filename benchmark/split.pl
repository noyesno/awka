$base = 2000;

$str = "Newcastle Knights are the best there is";

print STDERR "space split\n";

for ($i=0; $i<$base; $i++) {
  @S = split(" ", $str);
  $x = @S;
  for ($j=1; $j<=$x; $j++) {
    $s = $S[$j];
  }
}

print STDERR "single char\n";

for ($i=0; $i<$base; $i++)
{
  @S = split("a", $str);
  $x = @S;
  for ($j=1; $j<=$x; $j++) {
    $s = $S[$j];
  }
}

print STDERR "regex\n";
open(OUTPUT, '>x.tmp');

for ($i=0; $i<$base; $i++)
{
  @S = split(/ |t/, $str);
  $x = @S;
  for ($j=1; $j<=$x; $j++) {
    $s = $S[$j];
  }
  print(OUTPUT $str,"\n");
}
close(OUTPUT);

open(INPUT, '<x.tmp');
print STDERR "field split\n";

while (<INPUT>)
{
  chop;
  @fields = split(" ");
  $NF = @fields;
  for ($i=1; $i<=$NF; $i++) {
    $s = $fields[$i];
  }
}

close(INPUT);

system("rm -f x.tmp");
