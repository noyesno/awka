BEGIN {
  RS = 3
}

{ print NR, $0 }

