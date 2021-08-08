BEGIN { FPAT="([^,]*)|(\"([^\"]|\"\")+\"[^,]*)" }
{
  for (i=1; i<=NF; i++) printf "field #%d: %s\n", i, $(i)
  printf "---------------------------\n"
}
