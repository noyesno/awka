# test that if no '\0' in the file, it reads the whole file
BEGIN { RS = "\0" }
{ print; exit }
