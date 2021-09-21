set base 800;

for {set i 0} {$i<50} {incr i} {
  set X($i) "$i.$i"
}

for {set j 0} {$j<$base} {incr j} {
  for {set i 0} {$i<49} {incr i} {
    set X($i) X([expr $i+1])
  }
  set X($i) X(0)
}
