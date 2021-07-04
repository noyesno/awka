# test that END is called when exit from MAIN section,
# and that exit code is set
# When multi-row input file is provided, 
# the two END print statements are only
# called once due to the exit() in MAIN.

BEGIN { print dummy(1); legit(); }

function dummy(arg)
{
	return arg
}

function legit(         scratch)
{
	split("1 2 3", scratch)
	return ""
}
{
	print "Main"
	exit(2);
}
END { print "done" }
END { print "done again" }
