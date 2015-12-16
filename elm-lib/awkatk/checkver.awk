$0 ~ pattern { gsub(/"/,"",$3); if ($3 >= version) print "ok" >"_all_ok" }
