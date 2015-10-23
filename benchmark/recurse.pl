sub rec {
  $a = $_[0];
  $b = $_[1];
  if ($a > $b) {
    return ($a+1);
  } else {
    return rec($a+1, $b)
  }
}

$base = 2000;
for ($i=0; $i<$base; $i++) {
    $x += rec(1, 400);
}
