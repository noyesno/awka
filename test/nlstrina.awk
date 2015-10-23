# Contributed by Eiso AB
BEGIN {
        v=""
        ta[v]++
        if ( v in ta) print "a",v,++ta[v],ta[v]
	print "b",v,++ta[v],ta[v]
        for( i in ta) print "c",++c,i,ta[i]
} 
