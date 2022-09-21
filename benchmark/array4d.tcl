set base 15000;

for {set i 0} {$i<$base} {incr i} {
  set arr1($i) [expr ($base - $i)]
}

for {set i 5} {$i<$base} {incr i} {
  set arr1([expr ($i - 1)]) $arr1($i)
}
