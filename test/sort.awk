BEGIN {
	arr["d"] = "g" 
	arr["b"] = "z" 
	arr["u"] = "x" 
	arr["f"] = "m" 
	arr["p"] = "a"
	for (i=0; i<13; i++) {
		#if(i==0 || i==1 || i==2 || i==4 || i==6 || i==8 || i==10 || i==12) {
		SORTTYPE = 0+i
		print "####################\nSORTTYPE: "i
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
		#if(i==0 || i==1 || i==2 || i==4 || i==6 || i==8 || i==10 || i==12) {
		SORTTYPE = 0+i
		print "@@@@@@@@@@@@@@@@@@@@\nSORTTYPE: "i
		for (x in arr2) {
			print x" -> "arr2[x]
		}
	#}
	}
}
