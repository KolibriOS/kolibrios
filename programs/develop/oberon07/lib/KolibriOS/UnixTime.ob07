(*
    BSD 2-Clause License

    Copyright (c) 2018-2019, Anton Krotov
    All rights reserved.
*)

MODULE UnixTime;


VAR

    days: ARRAY 12, 31, 2 OF INTEGER;


PROCEDURE init;
VAR
    i, j, k, n0, n1: INTEGER;
BEGIN

    FOR i := 0 TO 11 DO
        FOR j := 0 TO 30 DO
            days[i, j, 0] := 0;
            days[i, j, 1] := 0;
        END
    END;

    days[ 1, 28, 0] := -1;

    FOR k := 0 TO 1 DO
        days[ 1, 29, k] := -1;
        days[ 1, 30, k] := -1;
        days[ 3, 30, k] := -1;
        days[ 5, 30, k] := -1;
        days[ 8, 30, k] := -1;
        days[10, 30, k] := -1;
    END;

    n0 := 0;
    n1 := 0;
    FOR i := 0 TO 11 DO
        FOR j := 0 TO 30 DO
            IF days[i, j, 0] = 0 THEN
                days[i, j, 0] := n0;
                INC(n0)
            END;
            IF days[i, j, 1] = 0 THEN
                days[i, j, 1] := n1;
                INC(n1)
            END
        END
    END

END init;


PROCEDURE time* (year, month, day, hour, min, sec: INTEGER): INTEGER;
    RETURN ((year - 1970) * 365 + days[month - 1, day - 1, ORD(year DIV 4 = 0)] + (year - 1969) DIV 4) * 86400 + hour * 3600 + min * 60 + sec
END time;


BEGIN
    init
END UnixTime.