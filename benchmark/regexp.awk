# Contributed by Eiso AB <eiso@chem.rug.nl>

BEGIN {
  if (!base) base = 1500

  Switch["123"] = " abc "
  Switch["82"] = " def "
  Switch["985"] = " ghi "
  Switch["20"] = " jkl "
  Switch["1098"] = " mno "
  Switch["3874"] = " pqr "
  Switch["272"] = " stu "

  Switch_R["123"] = " 123 "
  Switch_R["82"] = " 82 "
  Switch_R["985"] = " 985 "
  Switch_R["20"] = " 20 "
  Switch_R["1098"] = " 1098 "
  Switch_R["3874"] = " 3874 "
  Switch_R["272"] = " 272 "

  for (i=0; i<base; i++)
  {
    s1 = s2 = s3 = " 123 82 985 20 1098 3874 272 "

    for (j in Switch)
    {
      # Manually doing a gsub
      while (match(s1, j))
        s1 = substr(s1, 1, RSTART-1) Switch[j] substr(s1, RSTART+RLENGTH)

      # Use gsub
      gsub(j, Switch[j], s2)

      # gsub, and prevent RE recompile
      gsub(Switch_R[j], Switch[j], s3)

      #print Switch[j] " :: " Switch_R[j]
    }
  }
}
