#!/usr/local/bin/perl

$base = 100000;

$s = "she sells seashells";
#if($s =~ /s \w/) { print @-; print "\n"; } 
$x = $s =~ /s \w/;

for($i=0; $i<$base; $i++) {
  if($s =~ /s \w/) {
    $x = @-;
  }
}
