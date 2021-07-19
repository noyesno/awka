sub rec {
  $a = $_[0];
  $b = $_[1];
  if ($a > $b) {
    return ($a+1);
  } else {
    return rec($a+1, $b)
  }
}

$base = 20;
$x = 0;
for ($i=1; $i<$base; $i++) {
    $x += rec(1, 400);
}
#print $x."\n";
