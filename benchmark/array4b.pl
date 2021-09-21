#!/usr/local/bin/perl

$base = 8000;

for ($i=0; $i<$base; $i++) {
	$si = "".$i;
	$arr1{$si} = $base - $i;
}
for ($i=5; $i<$base; $i++) {
	$ind = "".($i-1);
	$si = "".$i;
	$arr1{$ind} = $arr1{$si};
}
