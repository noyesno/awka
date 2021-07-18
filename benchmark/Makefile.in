# Generated automatically from Makefile.in by configure.
#
#
SHELL = /bin/sh

AWKA = ../awka/awka
GAWK = gawk
MAWK = mawk
PERL = perl
PYTHON = python3
TCL = tclsh

AWKAV = $(AWKA) -v | head -n2
GAWKV = $(GAWK) -V | head -n1
MAWKV = $(MAWK) -W Version 2>&1 | head -n1 
PERLV = $(PERL) -v | head -n2 | tail -n1
PYTHONV = $(PYTHON) -V
TCLV = echo 'puts [ concat "TCL" " " [info patchlevel]]' | $(TCL) -

TIME = /usr/bin/time
CMP = cmp
CC  = gcc -O3
EXE =
MLIB = -lm
SLIB =

MATHLIB = $(MLIB) $(SLIB)
CFLAGS =  -L ../lib -I../lib ../lib/libawka.a $(MATHLIB)

srcdir = .

# run a number of times to average the result
SEQCMD = seq 5 | xargs -I{}

srcdir = .

###############

bench:   start array array2 array3 array4 array4b array4c array4d io cast expr function loop recurse \
         split parsecsv regexp re1 re2 re2a end

###############

versions::
	@date
	@echo "--------- Versions ----------" 
	@$(AWKAV) 
	@$(MAWKV)
	@$(GAWKV)
	@$(PERLV) 
	@$(PYTHONV)
	@$(TCLV)
	@echo "--------- Results -----------" 

start::
	@echo "Generating benchmarks..."
	@date 
	@echo "-----------------------------" 

array array2 array3 array4 array4b array4c array4d io::
	@echo $@ 
	@$(AWKA) -f $@.awk >x.c
	@$(CC) x.c $(CFLAGS)
	@$(SEQCMD) $(TIME) -f "$@\tAWKA\t%e sec" ./a.out 2>>$@ares.out
	@$(SEQCMD) $(TIME) -f "$@\tMAWK\t%e sec" $(MAWK) -f $@.awk 2>>$@mres.out
	@$(SEQCMD) $(TIME) -f "$@\tGAWK\t%e sec" $(GAWK) -f $@.awk 2>>$@gres.out
	@$(SEQCMD) $(TIME) -f "$@\tPERL\t%e sec" $(PERL) $@.pl 2>>$@plres.out
	@$(SEQCMD) $(TIME) -f "$@\tPYTHON\t%e sec" $(PYTHON) $@.py 2>>$@pyres.out
	@$(SEQCMD) $(TIME) -f "$@\tTCL \t%e sec"  $(TCL) $@.tcl 2>>$@tres.out


cast expr function recurse split re2 re2a::
	@echo $@
	@$(AWKA) -f $@.awk >x.c
	@$(CC) x.c $(CFLAGS)
	@$(SEQCMD) $(TIME) -f "$@\tAWKA\t%e sec" ./a.out 2>>$@ares.out
	@$(SEQCMD) $(TIME) -f "$@\tMAWK\t%e sec" $(MAWK) -f $@.awk 2>>$@mres.out
	@$(SEQCMD) $(TIME) -f "$@\tGAWK\t%e sec" $(GAWK) -f $@.awk 2>>$@gres.out
	@$(SEQCMD) $(TIME) -f "$@\tPERL\t%e sec" $(PERL) $@.pl 2>>$@plres.out
	@$(SEQCMD) $(TIME) -f "$@\tPYTHON\t%e sec" $(PYTHON) $@.py 2>>$@pyres.out

re1::
	@echo $@
	@$(AWKA) -f $@.awk >x.c
	@$(CC) x.c $(CFLAGS)
	@$(SEQCMD) $(TIME) -f "$@\tAWKA\t%e sec" ./a.out 2>>$@ares.out
	@# $(SEQCMD) $(TIME) -f "$@\tMAWK\t%e sec" $(MAWK) -f $@.awk 2>>$@mres.out
	@$(SEQCMD) $(TIME) -f "$@\tGAWK\t%e sec" $(GAWK) -f $@.awk 2>>$@gres.out
	@$(SEQCMD) $(TIME) -f "$@\tPERL\t%e sec" $(PERL) $@.pl 2>>$@plres.out
	@$(SEQCMD) $(TIME) -f "$@\tPYTHON\t%e sec" $(PYTHON) $@.py 2>>$@pyres.out

loop::
	@echo $@
	@$(AWKA) -f $@.awk >x.c
	@$(CC) x.c $(CFLAGS)
	@seq 2 | $(TIME) -f "$@\tAWKA\t%e sec" ./a.out 2>>$@ares.out
	@seq 2 | $(TIME) -f "$@\tMAWK\t%e sec" $(MAWK) -f $@.awk 2>>$@mres.out
	@seq 2 | $(TIME) -f "$@\tGAWK\t%e sec" $(GAWK) -f $@.awk 2>>$@gres.out
	@seq 2 | $(TIME) -f "$@\tPERL\t%e sec" $(PERL) $@.pl 2>>$@plres.out
	@seq 2 | $(TIME) -f "$@\tPYTHON\t%e sec" $(PYTHON) $@.py 2>>$@pyres.out


parsecsv regexp::
	@echo $@
	@$(AWKA) -f $@.awk >x.c
	@$(CC) x.c $(CFLAGS)
	@$(SEQCMD) $(TIME) -f "$@\tAWKA\t%e sec" ./a.out 2>>$@ares.out
	@$(SEQCMD) $(TIME) -f "$@\tMAWK\t%e sec" $(MAWK) -f $@.awk 2>>$@mres.out
	@$(SEQCMD) $(TIME) -f "$@\tGAWK\t%e sec" $(GAWK) -f $@.awk 2>>$@gres.out

end::
	@echo "Finished benchmarking!"
	@cat *res.out | $(GAWK) -f lib/benchmarksort.awk 


###############


results: versions
	@cat *res.out | $(GAWK) -f lib/benchmarksort.awk 


clean:
	rm -fr _* core x.c a.out *res.out

distclean: clean
	rm -f Makefile

maintainer-clean: distclean

