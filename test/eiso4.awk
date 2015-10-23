$5==4  {
        tmp=sprintf("%5i %5i %5i %5i %5i",$1,$2,$3,$4,$5)
        $1=$2=$3=$4=$5=""
        #a=$0    # uncomment this and it runs ok
        $0=tmp"|"$0 # in the awka version $1-5 are not yet updated at this point
}
{print}

