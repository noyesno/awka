#!/usr/local/bin/tcl

set base 15000

set v3 "qwerty qwerty qwerty qwerty qwerty qwerty"
set v4 "quincy quincy quincy quincy quincy quincy"

set io_txt [open "io.txt" "w"]

for {set i 0} {$i<$base} {incr i} {
  if {!($i % 2)} {
      puts $io_txt $v3
  } else {
      puts $io_txt $v4
  }
}
close $io_txt

set io_txt [open "io.txt" "r"]

set nr 0
while {[gets $io_txt line]>-1} {
  set arr [split $line " "]
  set x [lindex $arr 2]
  if {$nr < 2} {
      #puts $x
  }
  incr nr
}
close $io_txt

#system "rm -f io.txt"
file delete "io.txt"

