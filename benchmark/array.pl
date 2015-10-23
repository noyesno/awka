#!/usr/local/bin/perl

$base = 300000;

for ($i=0; $i<$base; $i++) {
	$x = $i."";
	$arr1{$x} = $base - $i;
}

for ($i=0; $i<$base; $i++) {
	$arr2{$i,$base-$i} = $base - $i;
}

foreach $x (@arr1) {
	$y = $x."".$arr1{$x};
}

for ($i=0; $i<$base; $i++) {
	if ($arr2{$i, $base-$i} != $base - $i) {
		print "Error\n";
		exit
	}
}

