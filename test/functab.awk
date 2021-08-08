BEGIN {
    SORTTYPE = 8 
	for(x in FUNCTAB) {
	        printf "%-10s -> %s\n", x, FUNCTAB[x]
    }
}
