# Contributed by Phil Magson <philm@netmap.com.au>

BEGIN { 
  FS=OFS="\t"
  pnts = 1
  team = 5
  type = 6
  name = 7
  types["BAT"]=BAT=5
  types["ALR"]=ALR=2
  types["WKT"]=WKT=1
  types["BWL"]=BWL=4
  tIdx["AUS"]  =AUS  =1
  tIdx["BDESH"]=BDESH=2
  tIdx["ENG"]  =ENG  =3
  tIdx["IND"]  =IND  =4
  tIdx["KENYA"]=KENYA=5
  tIdx["NZ"]   =NZ   =6
  tIdx["PAK"]  =PAK  =7
  tIdx["RSA"]  =RSA  =8
  tIdx["SCOT"] =SCOT =9
  tIdx["SL"]   =SL   =10
  tIdx["WI"]   =WI   =11
  tIdx["ZIM"]  =ZIM  =12
  for (i in tIdx) teams[tIdx[i]] = i

  while (getline<"loop.in">0)
  { 
    whom[$team,$type] = $name
    if ($type=="BAT") { bBAT[tIdx[$team]] = $pnts; continue }
    if ($type=="ALR") { bALR[tIdx[$team]] = $pnts; continue }
    if ($type=="WKT") { bWKT[tIdx[$team]] = $pnts; continue }
    if ($type=="BWL") { bBWL[tIdx[$team]] = $pnts; continue }
  }

# now lets process them

  for (wk1 in bWKT) 
  {
    for (ar1=1;ar1<=11;ar1++)     
      if (ar1!=wk1)
        for (ar2=ar1+1;ar2<=12;ar2++) 
          if (ar2!=wk1)
            for (bw1=1;bw1<=9; bw1++)     
              if (bw1!=wk1&&bw1!=ar1&&bw1!=ar2)
                for (bw2=bw1+1;bw2<=10;bw2++) 
                  if (bw2!=wk1&&bw2!=ar1&&bw2!=ar2)
                    for (bw3=bw2+1;bw3<=11;bw3++) 
                      if (bw3!=wk1&&bw3!=ar1&&bw3!=ar2)
                        for (bw4=bw3+1;bw4<=12;bw4++) 
                          if (bw4!=wk1&&bw4!=ar1&&bw4!=ar2)
                            for (ba1=1;ba1<=8; ba1++)     
                              if (ba1!=wk1&&ba1!=ar1&&ba1!=ar2&&ba1!=bw1&&ba1!=bw2&&ba1!=bw3&&ba1!=bw4)
                                for (ba2=ba1+1;ba2<=9; ba2++) 
                                  if (ba2!=wk1&&ba2!=ar1&&ba2!=ar2&&ba2!=bw1&&ba2!=bw2&&ba2!=bw3&&ba2!=bw4)
                                    for (ba3=ba2+1;ba3<=10;ba3++) 
                                      if (ba3!=wk1&&ba3!=ar1&&ba3!=ar2&&ba3!=bw1&&ba3!=bw2&&ba3!=bw3&&ba3!=bw4)
                                        for (ba4=ba3+1;ba4<=11;ba4++) 
                                          if (ba4!=wk1&&ba4!=ar1&&ba4!=ar2&&ba4!=bw1&&ba4!=bw2&&ba4!=bw3&&ba4!=bw4)
              for (ba5=ba4+1;ba5<=12;ba5++) 
                if (ba5!=wk1&&ba5!=ar1&&ba5!=ar2&&ba5!=bw1&&ba5!=bw2&&ba5!=bw3&&ba5!=bw4)
                {
                  # valid team!!!
                  tot =     bWKT[wk1]+ bALR[ar1]+ bALR[ar2]+ bBAT[ba1]+ bBAT[ba2]+ bBAT[ba3]+ bBAT[ba4]+ bBAT[ba5]+ bBWL[bw1]+ bBWL[bw2]+ bBWL[bw3]+ bBWL[bw4]
                  if (tot>=best) {
                    best = tot
                    bwk1 = wk1
                    bar1 = ar1; bar2 = ar2
                    bba1 = ba1; bba2 = ba2; bba3 = ba3; bba4 = ba4; bba5 = ba5
                    bbw1 = bw1; bbw2 = bw2; bbw3 = bw3; bbw4 = bw4
                  }
                }
  }
  
  exit

  print ""
  print "and the best World Cup 99 cricket team is..."
  print "POSITION     ","TEAM","PTS","PLAYER"
  print "Batsman      ",teams[bba1],bBAT[bba1],whom[teams[bba1],"BAT"]
  print "Batsman      ",teams[bba2],bBAT[bba2],whom[teams[bba2],"BAT"]
  print "Batsman      ",teams[bba3],bBAT[bba3],whom[teams[bba3],"BAT"]
  print "Batsman      ",teams[bba4],bBAT[bba4],whom[teams[bba4],"BAT"]
  print "Batsman      ",teams[bba5],bBAT[bba5],whom[teams[bba5],"BAT"]
  print "All-rounder  ",teams[bar1],bALR[bar1],whom[teams[bar1],"ALR"]
  print "All-rounder  ",teams[bar2],bALR[bar2],whom[teams[bar2],"ALR"]
  print "Wicket-keeper",teams[bwk1],bWKT[bwk1],whom[teams[bwk1],"WKT"]
  print "Bowler       ",teams[bbw1],bBWL[bbw1],whom[teams[bbw1],"BWL"]
  print "Bowler       ",teams[bbw2],bBWL[bbw2],whom[teams[bbw2],"BWL"]
  print "Bowler       ",teams[bbw3],bBWL[bbw3],whom[teams[bbw3],"BWL"]
  print "Bowler       ",teams[bbw4],bBWL[bbw4],whom[teams[bbw4],"BWL"]
}
