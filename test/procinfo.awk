# gawk has "identifiers" as a subarray of PROCINFO
# Awka can not handle subarrays like  "arr[x][y]"  nor as  "for x in arr[y]"
BEGIN {
	for(x in PROCINFO) {
		#if(x == "identifiers") { continue }
        # GAWK printing of identifiers
		#if(x == "identifiers") { print x" @@ " PROCINFO[x]; continue}
		#if(x == "identifiers" && length(PROCINFO[x])) {
		#	for(y in PROCINFO[x]) {
		#		printf "%-15s -> %-15s -> %s\n", x, y, PROCINFO[x][y] 
		#	}
		#	continue
		#}

	    printf "%-25s -> %s\n", x, PROCINFO[x]
    }
}