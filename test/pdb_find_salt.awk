#!/usr/local/bin/gawk -f
#
# Identify salt bridges ...
#
# (c) Finn Drablos, 1996
#
BEGIN {
   err=0
   first = 1
   CUTOFF = 3.5
   atom_list["serial"]=0

   if (ARGC==1) {
      print "Usage: pdb_find_salt [+c <cutoff>] <pdb_file>"
      err=1
      exit
   }

   for (i=1;i<=ARGC;i++) {
      if (ARGV[i]=="+c") {
         CUTOFF=ARGV[i+1]+0
         ARGV[i]=""
         ARGV[i+1]=""
      }
   }
}

/^ATOM/ {
   atom_decode($0,atom_list)
   # First find out if this is the first N ...
   if (atom_list["name"]==" N  ") {
      if (first) {
         first = 0
         name = atom_list["name"] "_" atom_list["residue"] "_" \
            atom_list["chain"] "_" atom_list["number"] atom_list["insert"]
         plus_x[name] = atom_list["x"]
         plus_y[name] = atom_list["y"]
         plus_z[name] = atom_list["z"]
         next
      }
   }
   # ... or an O (it may be the last one) ...
   if (atom_list["name"]==" O  ") {
      O_name = atom_list["name"] "_" atom_list["residue"] "_" \
         atom_list["chain"] "_" atom_list["number"] atom_list["insert"]
      O_x = atom_list["x"]
      O_y = atom_list["y"]
      O_z = atom_list["z"]
      next
   }
   # ... or the last OXT.
   if (atom_list["name"]==" OXT") {
      minus_x[O_name] = O_x
      minus_y[O_name] = O_y
      minus_z[O_name] = O_z
      name = atom_list["name"] "_" atom_list["residue"] "_" \
         atom_list["chain"] "_" atom_list["number"] atom_list["insert"]
      minus_x[name] = atom_list["x"]
      minus_y[name] = atom_list["y"]
      minus_z[name] = atom_list["z"]
      first = 1
      next
   }

   # Now look for specific residues.
   if (( atom_list["residue"] == "ARG" && atom_list["name"] == " NH1" ) ||\
      ( atom_list["residue"] == "ARG" && atom_list["name"] == " NH2" ) ||\
      ( atom_list["residue"] == "HIS" && atom_list["name"] == " ND1" ) ||\
      ( atom_list["residue"] == "HIS" && atom_list["name"] == " NE2" ) ||\
      ( atom_list["residue"] == "HIS" && atom_list["name"] == " AD1" ) ||\
      ( atom_list["residue"] == "HIS" && atom_list["name"] == " AE1" ) ||\
      ( atom_list["residue"] == "HIS" && atom_list["name"] == " AD2" ) ||\
      ( atom_list["residue"] == "HIS" && atom_list["name"] == " AE2" ) ||\
      ( atom_list["residue"] == "LYS" && atom_list["name"] == " NZ " ))  {
      name = atom_list["name"] "_" atom_list["residue"] "_" \
         atom_list["chain"] "_" atom_list["number"] atom_list["insert"]
      plus_x[name] = atom_list["x"]
      plus_y[name] = atom_list["y"]
      plus_z[name] = atom_list["z"]
      next
   }

   if (( atom_list["residue"] == "ASP" && atom_list["name"] == " OD1" ) ||\
      ( atom_list["residue"] == "ASP" && atom_list["name"] == " OD2" ) ||\
      ( atom_list["residue"] == "GLU" && atom_list["name"] == " OE1" ) ||\
      ( atom_list["residue"] == "GLU" && atom_list["name"] == " OE2" )) {
      name = atom_list["name"] "_" atom_list["residue"] "_" \
         atom_list["chain"] "_" atom_list["number"] atom_list["insert"]
      minus_x[name] = atom_list["x"]
      minus_y[name] = atom_list["y"]
      minus_z[name] = atom_list["z"]
      next
   }
}

END  {
   if (err) exit
   print "# pdb_find_salt  Finn Drablos (c) 1996"
   print "# CUTOFF is",CUTOFF
   # Now start the real computation ...
   for (i in plus_x) {
      for (j in minus_x) {
         if (i != j) {
            tmp=dist(plus_x[i],plus_y[i],plus_z[i], \
               minus_x[j],minus_y[j],minus_z[j])
            if (tmp < CUTOFF) {
               printf "%-14s %-14s %7.2f\n", i, j, tmp
            }
         }
      }
   }
}

function dist(x1,y1,z1,x2,y2,z2) {
   return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2))
}

function atom_decode(line,atom_list) {
   atom_list["serial"]=substr(line,7,5)
   atom_list["atom"]=substr(line,13,2)
   atom_list["name"]=substr(line,13,4)
   atom_list["location"]=substr(line,17,1)
   atom_list["residue"]=substr(line,18,3)
   atom_list["chain"]=substr(line,22,1)
   atom_list["number"]=substr(line,23,4)
   atom_list["insert"]=substr(line,27,1)
   atom_list["x"]=substr(line,31,8)
   atom_list["y"]=substr(line,39,8)
   atom_list["z"]=substr(line,47,8)
   atom_list["occup"]=substr(line,55,6)
   atom_list["temp"]=substr(line,61,6)
}
