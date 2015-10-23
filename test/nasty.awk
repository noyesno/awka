BEGIN{
a="aaaaa"
a=a a #10
a=a a #20
a=a a #40
a=a a #80
a=a a #160
a=a a # i.e. a is long enough

#f()
a=a"\n"f() # this causes the trouble
print a # guess the result
}

function f()
{
gsub(/a/, "123", a)
return "X"
}
