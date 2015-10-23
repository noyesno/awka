# contributed by Eiso AB

BEGIN {
        q=rec(5,4);print "q"q
        q=rec(5,4);print "q"q
   
        q=fun(5,4) ; print "'"q"'"
        q=fun(5) ; print "'"q"'"
}


function rec(a,  b) {
        printf "rec :%s :%s\n",a,b
        b++
        printf "rec %s %s\n",a,b
          
        if ( a < 10 ) return rec(a+b)
        else return a
}

function fun(a,b) {
        printf "fun :%s :%s",a,b
        return a""b
}
