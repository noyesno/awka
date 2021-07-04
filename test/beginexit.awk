# test that END is called when exit from BEGIN section

BEGIN { print dummy(1); legit(); exit }

function dummy(arg)
{
	return arg
}

function legit(         scratch)
{
	split("1 2 3", scratch)
	return ""
}
END { print "done" }
END { print "done again" }
