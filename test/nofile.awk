BEGIN {
	print (getline < "nofile")
	close("nofile")
	print (getline < "nofile")
	close("nofile")
	print (getline < "nofile")
	close("nofile")
}
