# If you know perl, and reckon this script could be written
# to run faster, by all means do so & forward a copy to me.
#
# I'd hate to think that the woeful speed this script runs
# at was as fast as Perl can manage.  Otherwise it is yet
# another example of why Awk should be used for processing
# data! ;-)

$base = 1500000;
$v3 = "qwerty qwerty qwerty qwerty qwerty qwerty";
$v4 = "quincy quincy quincy quincy quincy quincy";
$nr = 0;
$x2 = 2;

open(OUTPUT, '>io.txt');
for ($i=0; $i<$base; $i++)
{
  if (!($i % 2)) {
    print(OUTPUT $v3,"\n");
  } else {
    print(OUTPUT $v4,"\n");
  }
}
close(OUTPUT);
open(INPUT, '<io.txt');

while (<INPUT>)
{
  chop;
  # the '3' parameter in the call to split below is really
  # giving perl an unfair boost - the .awk example doesn't
  # have it!
  @fields = split(" ", $_, 3);
  $x = $fields[3];
}

system("rm -f io.txt");

