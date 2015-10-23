BEGIN {
  Mi = Mj = Mij = -0;

  p = q = 1 / 3
  printf "%.4g\n", log((1-q)/(1-p))

  Mi = q * log(q/p) / log(2)
  if ((1-p)>0 && (1-q)>0) 
    Mj = (1-q) * log( (1-q)/(1-p))/log(2);
  else
    Mj = 0

  printf "%.4g %.4g\n",Mi,Mj
  Mij = Mi + Mj
  printf "%9.6f\n", Mij
}

