# Most of these tests provided by Eiso AB 

BEGIN {
    q = "abcdefg"
    print gensub(/.*([bc]).*(ef).*/,"{\\1}[]{\\2}","",q)
    print gensub(/.*([bc]).*(ef).*/,"{\\1}[]{\\2}","","abcdefg")
    print gensub(/([^,]*),([^,]*),?([^,]*)/,"[\\3:\\2:\\1]","g","a,b,c,d,e,f")
    rex="([a-z]):([a-z]):([a-z]):([a-z])"
    print match("a:b:c:d",/([a-z]):[a-z]:[a-z]:[a-z]/),RSTART,RLENGTH
    print
    gensub(/([a-z]):([a-z]):([a-z]):([a-z])/,"[\\0]\\1-\\2-\\3-\\4","g","a:b:c:d")
    print gensub(/([a-z]*)([A-Z]*)([0-9]+)/,"\\1,\\2,\\3","","xY9")
    print gensub(/([a-z]*)([A-Z]*)([0-9]*)/,"\\1,\\2,\\3","","xYz")
    print gensub(/([0-9]*)/,"a\\1b","","xYz")
    print gensub(/a*/,":","g","abca")
    print gensub(/a*/,":","","abca")

    print so = "a Klevit, R achox E."

    print so1=gensub(/(.*) ([A-Z]+)( [^ ]*)? ?/,"1:\\1 2:\\2 3:\\3","",so)

    ruler="0..../....1..../....2..../....3..../....4..../...."
    conf ="S Peins, 40-4. Eby: Rean, Vean. ESCOM: Leiden,Neth"

    print conf

    vs=""
    # vs="g"

    rex="(.*)Eby:" ; gs="\\1"
    print (conf~rex)
    print gensub(rex,gs,vs,conf)
    print gensub(/ean/,"EAN",0,conf)
    print gensub(/ean/,"EAN",1,conf)
    print gensub(/ean/,"EAN",2,conf)

    rex=".* ([0-9]+\-[0-9]+)\. Eby:.*" ; gs="\\1"
    print (conf~rex)
    print gensub(rex,gs,vs,conf)

    rex = ".*Eby: *(.*)( [a-zA-Z]+):.*" ; gs="\\1"
    print (conf~rex)
    print gensub(rex,gs,vs,conf)

    rex = ".*Eby:(.*)( [a-zA-Z]+):(.*)" ; gs="\\2"
    print match(conf,rex), RSTART, RLENGTH
    print gensub(rex,gs,vs,conf)

    rex = ".*Eby:(.*)( [a-zA-Z]+):(.+)" ; gs="\\3"
    print (conf~rex)
    print gensub(rex,gs,vs,conf)

}


