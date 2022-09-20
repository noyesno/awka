#!/usr/local/bin/perl
eval 'exec /usr/local/bin/perl -S $0 ${1+"$@"}'
    if $running_under_some_shell;
                        # this emulates #! processing on NIH machines.
                        # (remove #! line above if indigestible)

eval '$'.$1.'$2;' while $ARGV[0] =~ /^([A-Za-z_0-9]+=)(.*)/ && shift;
                        # process any FOO=bar switches

open(LOOP_IN, 'loop.in') || die 'Cannot open file "loop.in".';

# Contributed by Phil Magson <philm@netmap.com.au>

#local $[ = 1;                    # set array base to 1
$FS = ' ';                # set field separator
$, = ' ';                 # set output field separator
$\ = "\n";                # set output record separator

$FS = $, = "\t";
$pnts = 0;
$team = 4;
$type = 5;
$name = 6;
$types{'BAT'} = $BAT = 4;
$types{'ALR'} = $ALR = 1;
$types{'WKT'} = $WKT = 0;
$types{'BWL'} = $BWL = 3;
$tIdx{'AUS'} = $AUS = 0;
$tIdx{'BDESH'} = $BDESH = 1;
$tIdx{'ENG'} = $ENG = 2;
$tIdx{'IND'} = $IND = 3;
$tIdx{'KENYA'} = $KENYA = 4;
$tIdx{'NZ'} = $NZ = 5;
$tIdx{'PAK'} = $PAK = 6;
$tIdx{'RSA'} = $RSA = 7;
$tIdx{'SCOT'} = $SCOT = 8;
$tIdx{'SL'} = $SL = 9;
$tIdx{'WI'} = $WI = 10;
$tIdx{'ZIM'} = $ZIM = 11;
foreach $i (keys %tIdx) {
    $teams{$tIdx{$i}} = $i;
}
while (($_ = &Getline2('LOOP_IN'),$getline_ok) > 0) {
    $whom{$Fld[$team], $Fld[$type]} = $Fld[$name];
    if ($Fld[$type] eq 'BAT') {
        $bBAT{$tIdx{$Fld[$team]}} = $Fld[$pnts];
        next;
    }
    if ($Fld[$type] eq 'ALR') {
        $bALR{$tIdx{$Fld[$team]}} = $Fld[$pnts];
        next;
    }
    if ($Fld[$type] eq 'WKT') {
        $bWKT{$tIdx{$Fld[$team]}} = $Fld[$pnts];
        next;
    }
    if ($Fld[$type] eq 'BWL') {
        $bBWL{$tIdx{$Fld[$team]}} = $Fld[$pnts];
        next;
    }
}

# now lets process them

foreach $wk1 (keys %bWKT) {
    for ($ar1 = 0; $ar1 < 11; $ar1++) {
        if ($ar1 != $wk1) {        #???
            for ($ar2 = $ar1 + 1; $ar2 < 12; $ar2++) {
                if ($ar2 != $wk1) {        #???
                    for ($bw1 = 0; $bw1 < 9; $bw1++) {
                        if ($bw1 != $wk1 && $bw1 != $ar1 && $bw1 != $ar2) {        #???        #???        #???
                            for ($bw2 = $bw1 + 1; $bw2 < 10; $bw2++) {
                                if ($bw2 != $wk1 && $bw2 != $ar1 && $bw2 != $ar2) {        #???
                                    for ($bw3 = $bw2 + 1; $bw3 < 11; $bw3++) {
                                        if ($bw3 != $wk1 && $bw3 != $ar1 && $bw3 != $ar2) {        #???
                                            for ($bw4 = $bw3 + 1; $bw4 < 12; $bw4++) {
                                                if ($bw4 != $wk1 && $bw4 != $ar1 && $bw4 != $ar2) {        #???
                                                    for ($ba1 = 0; $ba1 < 8; $ba1++) {
                                                        if ($ba1 != $wk1 && $ba1 != $ar1 && $ba1 != $ar2 && $ba1 != $bw1 && $ba1 != $bw2 && $ba1 != $bw3 && $ba1 != $bw4) {        #???
                                                            for ($ba2 = $ba1 + 1; $ba2 < 9; $ba2++) {
                                                                if ($ba2 !=  $wk1 && $ba2 != $ar1  && $ba2 != $ar2 && $ba2 != $bw1 && $ba2 != $bw2 && $ba2 != $bw3 && $ba2 != $bw4) {
                                                                    for ($ba3 = $ba2 + 1; $ba3 < 10; $ba3++) {
                                                                        if ($ba3 != $wk1 && $ba3 !=  $ar1 && $ba3 != $ar2 && $ba3 != $bw1 && $ba3 != $bw2 && $ba3 != $bw3 && $ba3 != $bw4) {
                                                                            for ($ba4 = $ba3 + 1; $ba4 < 11; $ba4++) {
                                                                                if ($ba4 != $wk1 && $ba4 != $ar1 && $ba4 != $ar2 && $ba4 != $bw1 && $ba4 != $bw2 && $ba4 != $bw3 && $ba4 != $bw4) {
                                                                                    for ($ba5 = $ba4 + 1; $ba5 < 12; $ba5++) {
                                                                                        if ($ba5 != $wk1 && $ba5 != $ar1 && $ba5 != $ar2 && $ba5 != $bw1 && $ba5 != $bw2 && $ba5 != $bw3 && $ba5 !=  $bw4) {
                                                                                            # valid team!!!
                                                                                            $tot = $bWKT{$wk1} + $bALR{$ar1} + $bALR{$ar2} + $bBAT{$ba1} + $bBAT{$ba2} + $bBAT{$ba3} + $bBAT{$ba4} + $bBAT{$ba5} + $bBWL{$bw1} + $bBWL{$bw2} + $bBWL{$bw3} + $bBWL{$bw4};
                                                                                            if ($tot >= $best) {
                                                                                                $best = $tot;
                                                                                                $bwk1 = $wk1;
                                                                                                $bar1 = $ar1;
                                                                                                $bar2 = $ar2;
                                                                                                $bba1 = $ba1;
                                                                                                $bba2 = $ba2;
                                                                                                $bba3 = $ba3;
                                                                                                $bba4 = $ba4;
                                                                                                $bba5 = $ba5;
                                                                                                $bbw1 = $bw1;
                                                                                                $bbw2 = $bw2;
                                                                                                $bbw3 = $bw3;
                                                                                                $bbw4 = $bw4;
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

exit;

print '';
print 'and the best World Cup 99 cricket team is...';
print 'POSITION     ', 'TEAM', 'PTS', 'PLAYER';
print 'Batsman      ', $teams{$bba1}, $bBAT{$bba1}, $whom{$teams{$bba1}, 'BAT'};
print 'Batsman      ', $teams{$bba2}, $bBAT{$bba2}, $whom{$teams{$bba2}, 'BAT'};
print 'Batsman      ', $teams{$bba3}, $bBAT{$bba3}, $whom{$teams{$bba3}, 'BAT'};
print 'Batsman      ', $teams{$bba4}, $bBAT{$bba4}, $whom{$teams{$bba4}, 'BAT'};
print 'Batsman      ', $teams{$bba5}, $bBAT{$bba5}, $whom{$teams{$bba5}, 'BAT'};
print 'All-rounder  ', $teams{$bar1}, $bALR{$bar1}, $whom{$teams{$bar1}, 'ALR'};
print 'All-rounder  ', $teams{$bar2}, $bALR{$bar2}, $whom{$teams{$bar2}, 'ALR'};
print 'Wicket-keeper', $teams{$bwk1}, $bWKT{$bwk1}, $whom{$teams{$bwk1}, 'WKT'};
print 'Bowler       ', $teams{$bbw1}, $bBWL{$bbw1}, $whom{$teams{$bbw1}, 'BWL'};
print 'Bowler       ', $teams{$bbw2}, $bBWL{$bbw2}, $whom{$teams{$bbw2}, 'BWL'};
print 'Bowler       ', $teams{$bbw3}, $bBWL{$bbw3}, $whom{$teams{$bbw3}, 'BWL'};
print 'Bowler       ', $teams{$bbw4}, $bBWL{$bbw4}, $whom{$teams{$bbw4}, 'BWL'};

sub Getline2 {
    ($fh) = @_;
    if ($getline_ok = (($_ = <$fh>) ne '')) {
        @Fld = split($FS, $_, 9999);
    }
    $_;
}
