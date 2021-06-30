set base 3000;

for {set i 0} {$i<$base} {incr i} {
  set x "$i"
  set arr1($x) [expr $base - $i]
}

for {set i 0} {$i<$base} {incr i} {
  set arr2($i,[expr $base - $i]) [expr $base - $i]
}

set sid [array startsearch arr1]
set x [array nextelement arr1 $sid]
while {$x != ""} {
  set y "$x$arr1($x)"
  set x [array nextelement arr1 $sid]
}

for {set i 0} {$i<$base} {incr i} {
  if {$arr2($i,[expr $base-$i]) != [expr $base - $i]} {
    puts "Error"
    exit
  }
}
