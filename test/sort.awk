#
# test and show the various valid soring options for awka
#
# Arrays are created with non-sequential keys so that sorting is obvious
#
BEGIN {
	valid[0] = valid[2] = valid[4] = valid[6] = valid[8] = valid[10] = valid[12] = 1
	arr["d"] = "g" 
	arr["b"] = "z" 
	arr["u"] = "x" 
	arr["f"] = "m" 
	arr["p"] = "a"
	for (i=0; i<13; i++) {
		if(!(i in valid)) continue
		SORTTYPE = 0+i
		print "Str index SORTTYPE: "i
		for (x in arr) {
			print x" -> "arr[x]
		}
	#}
	}
	arr2[0] = "g" 
	arr2[5] = "z" 
	arr2[2] = "x" 
	arr2[3] = "m" 
	arr2[1] = "a"
	for (i=0; i<13; i++) {
		if(!(i in valid)) continue
		SORTTYPE = 0+i
		print "Num index SORTTYPE: "i
		for (x in arr2) {
			print x" -> "arr2[x]
		}
	#}
	}
}
