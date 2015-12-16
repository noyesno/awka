#################################################################
#  User-Configurable parameters...
#################################################################

# Location of tcl.h & tk.h
TCLINCDIR = /usr/include

# Location of tcl distribution (contains init.tcl)
TCLDISTDIR = /usr/lib/tcl8.0

# Location of tcl distribution (contains tk.tcl)
TKDISTDIR = /usr/lib/tk8.0

# Location of libawka.h
LIBAWKA_H = /usr/local/include

# Preferred destination for installing awkatk.a & awkatk.so
LIBINSTALLDIR = /usr/local/lib

# Preferred destination for installing awkatk.h
INCINSTALLDIR = /usr/local/include

# Build targets.  Some systems (like Cygnus) won't support shared object libraries
TARGETS = libawkatk.a libawkatk.so
# TARGETS = libawkatk.a
# TARGETS = libawkatk.so

# Preferred C compiler
CC = gcc
# CC = cc

# C compiler flags.
CFLAGS = -O

# Archive creation program
AR = ar

# AWK interpreter
AWK = awk

#################################################################
#  Nothing user-configurable below this point
#################################################################

all: checkver $(TARGETS)
  
libawkatk.a: awkatk.o
	$(AR) -uvr libawkatk.a awkatk.o
	
libawkatk.so: awkatk.o
	$(CC) -shared $(CFLAGS) awkatk.o -o libawkatk.so
	
checkver: awkaver tclver tkver

awkaver::
	@rm -f _all_ok
	@$(AWK) -vversion="0.7.1" -vpattern="AWKAVERSION" -f checkver.awk $(LIBAWKA_H)/libawka.h 
	@if [ ! -f _all_ok ]; then echo "ERROR: Awka version must be >= 0.7.1"; fi
	@rm _all_ok
	
tclver::
	@rm -f _all_ok
	@$(AWK) -vversion="8.0" -vpattern="TCL_VERSION" -f checkver.awk $(TCLINCDIR)/tcl.h 
	@if [ ! -f _all_ok ]; then echo "ERROR: Tcl version must be >= 8.0"; fi
	@rm _all_ok
	
tkver::
	@rm -f _all_ok
	@$(AWK) -vversion="8.0" -vpattern="TK_VERSION" -f checkver.awk $(TCLINCDIR)/tk.h 
	@if [ ! -f _all_ok ]; then echo "ERROR: Tk version must be >= 8.0"; fi
	@rm _all_ok
	

awkatk.o: awkatk.c awkatk.h
	$(CC) -c $(CFLAGS) -DAWKATK_USE_TK=0 -DINIT_TCL_FILE="\"$(TCLDISTDIR)/init.tcl\"" -DTK_TCL_FILE="\"$(TKDISTDIR)/tk.tcl\"" -I. -I$(LIBAWKA_H) -I$(TCLINCDIR) awkatk.c -o awkatk.o

install: $(TARGETS)
	if [ ! -d $(LIBINSTALLDIR) ]; then mkdir -p $(LIBINSTALLDIR); fi
	if [ ! -d $(INCINSTALLDIR) ]; then mkdir -p $(INCINSTALLDIR); fi
	if [ -f libawkatk.a ]; then cp libawkatk.a $(LIBINSTALLDIR); chmod a+r $(LIBINSTALLDIR)/libawkatk.a; fi
	if [ -f libawkatk.so ]; then cp libawkatk.so $(LIBINSTALLDIR); chmod a+r $(LIBINSTALLDIR)/libawkatk.so; fi
	cp awkatk.h $(INCINSTALLDIR); chmod a+r $(INCINSTALLDIR)/awkatk.h

clean:
	rm -rf *.a *.so *.o core

