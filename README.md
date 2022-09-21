# Awka

![C/C++ CI](https://github.com/noyesno/awka/workflows/C/C++%20CI/badge.svg)

![](docs/awka.svg)

This is a fork and enhanced version of [Awka](http://awka.sourceforge.net/index.html).


> Awka - Open Source, AWK to C Conversion 
>
> Awka is not an interpreter like Gawk, Mawk or Nawk, but instead it converts the program to ANSI-C, then compiles this using gcc or a native C compiler to create a binary executable. 
>
> -- http://awka.sourceforge.net/index.html


  * Awka has two main parts: a translator and a library.
  * `awka` translate a [Awk](https://www.gnu.org/software/gawk/) script into C code
  * `libawka` is used as link library to compile the generated C code.
  
### Why compile Awk script into C program?
 
  * C program probably run faster.
  * "AWK programs are limited to what the interpreter provides".
  * "C code can be compiled with other code into a larger application."
  * "migrate stable functions from AWK to C" to impove performance.
  * Reuse existing C functions in Awk script.
  * A way of protect your Awk script.
   
Actually, the mixing use of Awk and C provide provide an interesting way of writing programs.
   
## Revive Awka

Awka is an "old" project. Its last change 0.7.5 was on June 20 2001.

And to fill my usage requirement, some of my update to the code go much further than traditional "Awk".

### What's new in "Revive Awka"?

  * Fix bugs in awka-0.7.5
  * Add `libawka` builtin function `fseek` and `ftell` to increase performance.
  * Add glob style matching, which shold be faster than regular expression in many case.
  * Lazy calling of line split function to increase performance.
  * Use `nstring` other than NULL terminated C string to avoid string copy.

### What to do next?

  * Continue bug fixing.
  * Continue `libawka` performance improvement.
  * Support flexibl synax in the "Awk" script. 
    * For example, add direct Tcl language support.
  * Make it thread safe to support processing multiple files in parallel.
  
