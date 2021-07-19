#!/usr/local/bin/python3
import csv

FS = ' ';                # set field separator

FS = SEP = "\t";
pnts = 0;
team = 4;
type = 5;
name = 6;
types = {}
types['BAT'] = BAT = 5;
types['ALR'] = ALR = 2;
types['WKT'] = WKT = 1;
types['BWL'] = BWL = 4;
tIdx = {}
tIdx['AUS'] = AUS = 1;
tIdx['BDESH'] = BDESH = 2;
tIdx['ENG'] = ENG = 3;
tIdx['IND'] = IND = 4;
tIdx['KENYA'] = KENYA = 5;
tIdx['NZ'] = NZ = 6;
tIdx['PAK'] = PAK = 7;
tIdx['RSA'] = RSA = 8;
tIdx['SCOT'] = SCOT = 9;
tIdx['SL'] = SL = 10;
tIdx['WI'] = WI = 11;
tIdx['ZIM'] = ZIM = 12;
teams = {}
bBAT = {}
bALR = {}
bWKT = {}
bBWL = {}
whom = {}
best = 0
for k in tIdx.keys():
    teams[tIdx[k]] = k
with open('loop.in') as lp:
    creadr = csv.reader(lp, delimiter='\t')
    for flds in creadr:
        whom[(flds[team],flds[type])] = flds[name]
        if flds[type] == 'BAT':
            bBAT[tIdx[flds[team]]] = int(flds[pnts])
            next
        if flds[type] == 'ALR':
            bALR[tIdx[flds[team]]] = int(flds[pnts])
            next
        if flds[type] == 'WKT':
            bWKT[tIdx[flds[team]]] = int(flds[pnts])
            next
        if flds[type] == 'BWL':
            bBWL[tIdx[flds[team]]] = int(flds[pnts])
            next

# now lets process them

for wk1 in bWKT.keys():
    for ar1 in range(1,12):
        if ar1 != wk1:        #???
            for ar2 in range(ar1 + 1, 13):
                if ar2 != wk1:        #???
                    for bw1 in range(1, 10):
                        if bw1 != wk1 and bw1 != ar1 and bw1 != ar2:        #???        #???        #???
                            for bw2 in range(bw1 + 1, 11):
                                if bw2 != wk1 and bw2 != ar1 and bw2 != ar2:        #???
                                    for bw3 in range(bw2 + 1, 12):
                                        if bw3 != wk1 and bw3 != ar1 and bw3 != ar2:        #???
                                            for bw4 in range(bw3 + 1, 13):
                                                if bw4 != wk1 and bw4 != ar1 and bw4 != ar2:        #???
                                                    for ba1 in range(1, 9): 
                                                        if ba1 != wk1 and ba1 != ar1 and ba1 != ar2 and ba1 != bw1 and ba1 != bw2 and ba1 != bw3 and ba1 != bw4:        #???
                                                            for ba2 in range(ba1 + 1, 10):
                                                                if ba2 != wk1 and ba2 != ar1 and ba2 != ar2 and ba2 != bw1 and ba2 != bw2 and ba2 != bw3 and ba2 != bw4:
                                                                    for ba3 in range(ba2 + 1, 11):
                                                                       if ba3 != wk1 and ba3 !=  ar1 and ba3 != ar2 and ba3 != bw1 and ba3 != bw2 and ba3 != bw3 and ba3 != bw4:
                                                                          for ba4 in range(ba3 + 1, 12):
                                                                             if ba4 != wk1 and ba4 != ar1 and ba4 != ar2 and ba4 != bw1 and ba4 != bw2 and ba4 != bw3 and ba4 != bw4:
                                                                                for ba5 in range(ba4 + 1, 13):
                                                                                   if ba5 != wk1 and ba5 != ar1 and ba5 != ar2 and ba5 != bw1 and ba5 != bw2 and ba5 != bw3 and ba5 !=  bw4:
                                                                                      # valid team!!!
                                                                                      tot = bWKT[wk1] + bALR[ar1] + bALR[ar2] + bBAT[ba1] + bBAT[ba2] + bBAT[ba3] + bBAT[ba4] + bBAT[ba5] + bBWL[bw1] + bBWL[bw2] + bBWL[bw3] + bBWL[bw4];
                                                                                      if tot >= best:
                                                                                             best = tot
                                                                                             bwk1 = wk1
                                                                                             bar1 = ar1
                                                                                             bar2 = ar2
                                                                                             bba1 = ba1
                                                                                             bba2 = ba2
                                                                                             bba3 = ba3
                                                                                             bba4 = ba4
                                                                                             bba5 = ba5
                                                                                             bbw1 = bw1
                                                                                             bbw2 = bw2
                                                                                             bbw3 = bw3
                                                                                             bbw4 = bw4

exit();

print('')
print('and the best World Cup 99 cricket team is...')
print('POSITION     ', '\t', 'TEAM', '\t', 'PTS', '\t', 'PLAYER')
print('Batsman      ', '\t', teams[bba1], '\t', bBAT[bba1], '\t', whom[(teams[bba1], 'BAT')])
print('Batsman      ', '\t', teams[bba2], '\t', bBAT[bba2], '\t', whom[(teams[bba2], 'BAT')])
print('Batsman      ', '\t', teams[bba3], '\t', bBAT[bba3], '\t', whom[(teams[bba3], 'BAT')])
print('Batsman      ', '\t', teams[bba4], '\t', bBAT[bba4], '\t', whom[(teams[bba4], 'BAT')])
print('Batsman      ', '\t', teams[bba5], '\t', bBAT[bba5], '\t', whom[(teams[bba5], 'BAT')])
print('All-rounder  ', '\t', teams[bar1], '\t', bALR[bar1], '\t', whom[(teams[bar1], 'ALR')])
print('All-rounder  ', '\t', teams[bar2], '\t', bALR[bar2], '\t', whom[(teams[bar2], 'ALR')])
print('Wicket-keeper', '\t', teams[bwk1], '\t', bWKT[bwk1], '\t', whom[(teams[bwk1], 'WKT')])
print('Bowler       ', '\t', teams[bbw1], '\t', bBWL[bbw1], '\t', whom[(teams[bbw1], 'BWL')])
print('Bowler       ', '\t', teams[bbw2], '\t', bBWL[bbw2], '\t', whom[(teams[bbw2], 'BWL')])
print('Bowler       ', '\t', teams[bbw3], '\t', bBWL[bbw3], '\t', whom[(teams[bbw3], 'BWL')])
print('Bowler       ', '\t', teams[bbw4], '\t', bBWL[bbw4], '\t', whom[(teams[bbw4], 'BWL')])

