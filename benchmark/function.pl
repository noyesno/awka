sub abc {
  $a = $_[0];
  $b = $_[1];
  $c = $_[2];
  return ($a.$b.$c);
}

sub def {
  $d = $_[0];
  $e = $_[1];
  $f = $_[2];
  return abc($d, $e, $f);
}

$base = 20000;
for ($i=0; $i<$base; $i++) {
    $x = def(12435345, 93840932, 98921);
    #$x = def("this", "that", "the next thing");
}
