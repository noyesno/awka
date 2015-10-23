# contributed by Eiso AB

BEGIN { 
        line = "a b c d "
        nw=split(line,word)
        for ( i=1 ; i<=nw ; i++ ) printf " '%s'",word[i] ; print ""
        line = "a b "
        nw=split(line,word)
        for ( i=1 ; i<=nw ; i++ ) printf " '%s'",word[i] ; print ""
        for ( i in word ) printf " '%s'",word[i] ; print ""
        for ( i=1 ; i<=nw+2 ; i++ ) if (word[i]) printf " '%s'",word[i] ; print "\n"
}

