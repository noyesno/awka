INTRODUCTION
------------

Awka comprises a translator of the AWK programming language to ANSI-C,
and a library against which translated programs may be linked.

version 0.7.x

Instructions on installing Awka may be found in the file INSTALL.TXT.


SO WHAT IS AWKA?
----------------
It is two products :-

 *  A Translator - awka - is a seriously hacked version of mawk, with 
    additional code to output ANSI-C code.  This is not a recommended way of
    producing a program, but (a) it's only a translator, and (b) time was too
    short not to use mawk as a starting point.

 *  The Library - libawka.a - is my own creation, and is I believe(hope) much
    better designed than the translator.  Most development time was spent on
    the library, ensuring that code size and execution time were the best I
    could produce. 

To my mind the most important reason for a translator is that AWK programs
are limited to what the interpreter provides, whereas C code can be compiled
with other code into a larger application.

The use of C allows other products, such as Tk, to be used to provide GUI 
front-ends and the like.  It also allows you to migrate stable functions from
AWK to C, and still have them available to call as functions from with the
AWK language.

Many people have expressed the following requirements for an Awk-to-C
translator; it must allow distribution of an executable, and it should deliver
improved performance through removal of the interpreter.

Awka provides the method for creating an executable.  Increased performance,
however, faced major issues:-

 (a) It is a common assumption that compiled code will automatically be 
     faster than interpreted, but this is not particularly true of AWK.  
     AWK is a relatively terse language (part of its appeal) - most things 
     translate directly into calls to 'library' functions in the (compiled)
     interpreter.  This means that Awka is unlikely to be much quicker than
     the fastest interpreter Mawk, and sometimes Mawk is faster.

     Only with larger AWK programs will there be any benefit from compiling;
     even then it may not be significant.  You are also depending on how well
     your C compiler optimises code on your platform, something over which I
     have no control. 

 (b) Variables are typeless in AWK, so they must also be typeless in the
     compiled code.  Thus one of the main areas where savings could be made -
     use of C native data types - is denied to a translator.  The Awka library
     uses macros and inline functions where possible to allow AWK-style
     type-casting without losing speed, and the translator cuts corners by
     referencing the contents of variables directly whenever it can.

 (c) Many AWK functions have variable numbers of arguments.  An interpreter
     can create efficient internal parameter structures at parse time, but
     C code must resort to vararg calls, which are not particularly quick.

Having said all this, Awka seems to be at least competitive with Mawk, and on
some (but by no means all) occasions it is faster.  The one area where Awka
really falls behind Mawk is with recursive function calls, where Mawk approaches
C in speed, but Awka is closer to Gawk.

Some AWK enthusiasts may be opposed to the idea of a compiler, holding to a
philosophical view that AWK scripts should always be made available in source
form to customers.  I feel that although this may be appropriate most of the 
time, sometimes it is not.  With Awka, the author of a script now has more 
options to decide how they want their work distributed, and I think freedom of
choice is a good thing.


WHAT PLATFORMS ARE SUPPORTED?
-----------------------------
Awka is designed to operate under a flavour of Unix.  It should compile and
install correctly on pretty much any machine supporting configure scripts,
Makefiles and with an ANSI C compiler.

Neither the Awka library or the generated C code is thread-safe.


LICENSING
---------
The Awka TRANSLATOR contains portions of code derived from Mawk, and 
therefore shares Mawk's GPL license, a copy of which is provided.

Prior to release 0.7.5, the LIBRARY was distributed under the Lesser Public
License.  This changed to the GPL in 0.7.5 due to the inclusion of some
code from Gawk to provide inet and coprocessing capabilities.

A copy of the GPL is included in this distribution.  If you plan to use the 
Awka package you should definitely read this document and become familiar with 
its terms.

Please note that the code generated from your awk script by the TRANSLATOR
is always owned by you - I do not consider it to be 'derived from' the
TRANSLATOR's code, so it does not come under GPL.


FEEDBACK
--------
Encouragement, bug reports (patches are welcome), suggestions, bribes and
constructive criticism may be sent to andrewsumner@yahoo.com.  I don't
always have time to read my mail or to reply promptly, but I will do my best.

The Awka homepage is at:
   http://awka.sourceforge.net

The most current source release will always be posted here, and new releases
are announced in comp.lang.awk and at freshmeat.net.


HOW AWKA CAME TO BE
-------------------
Awka was developed in response to several posts to comp.lang.awk asking 
whether a free awk translator/compiler existed - clearly there was a 
need for such a tool.

Responses to these queries usually pointed to TAWK, apparently a fine product;
however it is not free, and is not available for many platforms.  From what
I can gather Tawk produces assembler or machine-language output which it then
executes, rather than generating portable C code.

I found some references to an ancient tool called CAWK, which is apparently
'Compiled AWK', however I could find nothing about it - certainly if it
exists it is not available for general distribution.

There is also awk2c, which creates C code to link to gawk sourcecode, however
this was incomplete, with no development taking place.  It also appeared to
be subject to the GPL, so distribution of binaries without sourcecode may not 
have been possible.  As it used native C datatypes it would have been difficult
to integrate it with a library providing the builtin C functions.

Finally there was awkcc, an AT&T product that is essentially similar to Awka 
but costs $2,000 for a sublicense (or did when I last checked).

  "Implementors of the AWK language have shown a consistent lack of 
   imagination when naming their programs."
                                              - Michael Brennan

Over time I had been creating a library of awk-like functions, which I had
called libawka (Awka stood for Awk Archive).  I extended this to make it a more
complete coverage of the language, including pervasive casting of variable
types.

I then tackled the task of writing a translator - something I had little time
to do properly.  In order to reduce the size of the task I decided to use Mawk
as a starting point.  I tried to remove as much execution code as possible,
leaving code necessary for parsing the AWK language.  I converted the Mawk
internal opcode structure to a format I could use for translation purposes,
then crunched out a translation module.

I preferred Mawk over Gawk as I found it easier to track variable types in 
complex statements using Mawk's reverse-polish, assembler-like opcode structure.
At the time I was not aware of various Mawk parser bugs and restrictions, and
these caused sufficient problems to make me attempt using Gawk's parser, however 
this proved too complex, so I persevered with Mawk's.  In retrospect it would 
have been preferable to use Gawk as a starting point, and potentially one day
Awka may move to using Gawk's language parser.

I borrowed Gawk's extensive test suite and, after much effort removing various 
bugs and incorrect logic (from Awka not Gawk), I ensured that Awka now passes 
the tests.  The test suite is being extended as new bugs are discovered and fixed.

A major enhancement introduced in version 0.7.0 is the ability to write your own
C functions, and have them available in the AWK language as if they were builtin
functions.  See the awka-elm manpage for more details about this, as it really
frees the AWK language in a way that hasn't been done before.


Andrew Sumner, August 2000.
