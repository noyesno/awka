# sort
#  type\tname\ttime
# by type|time
#
BEGIN {
  FS = "\t"
  OFS = FS
  OFMT = "%0.02f"
}
!/PERL|PYTHON|AWKA|MAWK|GAWK|TCL/ {next}
{
	# sum and count the time values for each type,name combination
	type = $1
	name = $2
	time = $3
	split($3, T, " ")
	tsum[type","name] += 10*T[1]
	nr[type","name] += 1
	typ[type] = type
}
END {
	for(tn in tsum) {
		split(tn, X, ",")
		# times array - use asorti to sort indexes by time
		av = tsum[tn] / nr[tn] / 10
		t[av","X[1]","X[2]] = 1
	}
	asort(typ,typsorted)
	asorti(t, tkeys)
	for(y in typsorted) {
		if(y>1) print ""
		print toupper(typsorted[y])":"
		j = 1
		for(x in tkeys) {
			split(tkeys[x], XX, ",")
			if(typsorted[y] == XX[2]) {
				#print XX[2]","t[tkeys[x]]","XX[1]
				printf("%d: %-10s %*.2f\n", j++, XX[3], 6, 10*XX[1])
			}
		}
	}
}
