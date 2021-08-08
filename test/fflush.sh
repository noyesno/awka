#! /bin/sh
CC="$1"
MATHLIB="$2"

../awka/awka 'BEGIN{print "1st";fflush("/dev/stdout");print "2nd"|"cat"}' >x.c
$CC x.c -L../lib -I../lib ../lib/libawka.a $MATHLIB -o xx
./xx

../awka/awka 'BEGIN{print "1st";fflush("/dev/stdout");print "2nd"|"cat"}' >x.c
$CC x.c -L../lib -I../lib ../lib/libawka.a $MATHLIB -o xx
./xx | cat

../awka/awka 'BEGIN{print "1st";fflush("/dev/stdout");close("/dev/stdout");print "2nd"|"cat"}' >x.c
$CC x.c -L../lib -I../lib ../lib/libawka.a $MATHLIB -o xx
./xx | cat

../awka/awka 'BEGIN{print "1st";fflush("/dev/stdout");print "2nd"|"cat";close("cat")}' >x.c
$CC x.c -L../lib -I../lib ../lib/libawka.a $MATHLIB -o xx
./xx | cat

../awka/awka 'BEGIN{print "1st";fflush("/dev/stdout");print "2nd"|"cat";close("cat")}' >x.c
$CC x.c -L../lib -I../lib ../lib/libawka.a $MATHLIB -o xx
./xx | cat

../awka/awka 'BEGIN{print "1st";fflush("/dev/stdout");print "2nd"|"cat";close("cat")}' >x.c
$CC x.c -L../lib -I../lib ../lib/libawka.a $MATHLIB -o xx
./xx | cat

../awka/awka 'BEGIN{print "1st";fflush("/dev/stdout");print "2nd"|"sort"}' >x.c
$CC x.c -L../lib -I../lib ../lib/libawka.a $MATHLIB -o xx
./xx | cat

../awka/awka 'BEGIN{print "1st";fflush("/dev/stdout");print "2nd"|"sort";close("sort")}' >x.c
$CC x.c -L../lib -I../lib ../lib/libawka.a $MATHLIB -o xx
./xx | cat
