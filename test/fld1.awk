# Test the comparison of numerical and string inputs and checks the 
# type setting of input fields
#
# The expected input is lines containing either a string or a numeric value
#
{
  print ($1 + 0) == $1
}