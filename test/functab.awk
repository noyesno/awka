BEGIN {
    SORTTYPE = 6
	for(x in FUNCTAB) {
	        printf "%-10s -> %s\n", x, FUNCTAB[x]
    }
}
