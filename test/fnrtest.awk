# test changing FNR for MULTIPLE read files

BEGIN {
	print "Starting FNR: "FNR" NR: "NR
}
{
	if (FNR == 1) { 
		prev = FILENAME
		print "File: "FILENAME
		print "Onchange FNR: "FNR" NR: "NR
	}
	print "FNR: "FNR" NR: "NR
}
END {
	print "Ending FNR: "FNR" NR: "NR
}
