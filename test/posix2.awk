# the following program should print all OKs.

NR == 1 {
        num = 0
        str = "0e2"

        print ++test ": " (        (str == "0e2")        ? "OK" : "OOPS" )
        print ++test ": " (        ("0e2" != 0)        ? "OK" : "OOPS" )
        print ++test ": " (        ("0" != $2)        ? "OK" : "OOPS" )
        print ++test ": " (        ("0e2" == $1)        ? "OK" : "OOPS" )

        print ++test ": " (        (0 == "0")        ? "OK" : "OOPS" )
        print ++test ": " (        (0 == num)        ? "OK" : "OOPS" )
        print ++test ": " (        (0 != $2)        ? "OK" : "OOPS" )
        print ++test ": " (        (0 == $1)        ? "OK" : "OOPS" ) # 8

        print ++test ": " (        ($1 != "0")        ? "OK" : "OOPS" )
        print ++test ": " (        ($1 == num)        ? "OK" : "OOPS" ) # 10
        print ++test ": " (        ($2 != 0)        ? "OK" : "OOPS" )
        print ++test ": " (        ($2 != $1)        ? "OK" : "OOPS" )
        print ++test ": " (        ($3 == 0)        ? "OK" : "OOPS" )
        print ++test ": " (        ($3 == $1)        ? "OK" : "OOPS" )
        print ++test ": " (        ($2 != $4)        ? "OK" : "OOPS"        ) # 15
}
{
        a = "+2"
        b = 2
        if (NR % 2)
                c = a + b
        print ++test ": " (        (a != b)        ? "OK" : "OOPS" ) # 16 and 22

        d = "2a"
        b = 2
        if (NR % 2)
                c = d + b
        print ++test ": " (        (d != b)        ? "OK" : "OOPS" )

        print ++test ": " (        (d + 0 == b)        ? "OK" : "OOPS" ) #18

        e = "2"
        print ++test ": " (        (e == b "")        ? "OK" : "OOPS" )

        a = "2.13"
        print ++test ": " (        (a == 2.13)        ? "OK" : "OOPS" )

        a = "2.130000"
        print ++test ": " (        (a != 2.13)        ? "OK" : "OOPS" )

        if (NR == 2) {
                CONVFMT = "%.6f"
                print ++test ": " (        (a == 2.13)        ? "OK" : "OOPS" )
        }
}
