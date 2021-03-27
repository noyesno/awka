BEGIN { RS = "" }
{ print "---", NR, "---"; print }
