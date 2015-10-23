# From Gawk Manual modified by bug fix and removal of punctuation
# Record every word which is used at least once
BEGIN {
  count = 0
}
{
    for (i = 1; i <= NF; i++) {
        tmp = tolower($i)
        if (0 != (pos = match(tmp, /([a-z]|-)+/)))
        {
            key = substr(tmp, pos, RLENGTH)
            if (!(key in used))
            {
              used[substr(tmp, pos, RLENGTH)] = 1
              used_i[count] = substr(tmp, pos, RLENGTH)
              count++
            }
        }
    }
}

#Find a number of distinct words longer than 10 characters
END {
    print count, "words"
    lng_count = 0
    for (i=0; i<count; i++)
        if (length(used_i[i]) > 10) {
            print used_i[i]
            lng_count++
        }
    print lng_count, "long words"
}
