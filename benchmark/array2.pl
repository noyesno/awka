#!/usr/local/bin/perl

$base = 800;

for ($i=0; $i<50; $i++) {
    $X{$i} = $i.".".$i;
}

for ($j=0; $j<$base; $j++) {
  for ($i=0; $i<49; $i++) {
    $X{$i} = $X{$i+1}
  }
  $X{$i} = $X{0}
}
