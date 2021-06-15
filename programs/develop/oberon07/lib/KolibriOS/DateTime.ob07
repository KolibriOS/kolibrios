(*
    Copyright 2016, 2018 Anton Krotov

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*)

MODULE DateTime;

IMPORT KOSAPI;

CONST ERR* = -7.0E5;

PROCEDURE Encode*(Year, Month, Day, Hour, Min, Sec: INTEGER): REAL;
VAR d, i: INTEGER; M: ARRAY 14 OF CHAR; Res: REAL;
BEGIN
  Res := ERR;
  IF (Year >= 1) & (Year <= 9999) & (Month >= 1) & (Month <= 12) &
    (Day >= 1) & (Day <= 31) & (Hour >= 0) & (Hour <= 23) &
    (Min >= 0) & (Min <= 59) & (Sec >= 0) & (Sec <= 59) THEN
    M := "_303232332323";
    IF (Year MOD 4 = 0) & (Year MOD 100 # 0) OR (Year MOD 400 = 0) THEN
      M[2] := "1"
    END;
    IF Day <= ORD(M[Month]) - ORD("0") + 28 THEN
      DEC(Year);
      d := Year * 365 + (Year DIV 4) - (Year DIV 100) + (Year DIV 400) + Day - 693594;
      FOR i := 1 TO Month - 1 DO
        d := d + ORD(M[i]) - ORD("0") + 28
      END;
      Res := FLT(d) + FLT(Hour * 3600000 + Min * 60000 + Sec * 1000) / 86400000.0
    END
  END
  RETURN Res
END Encode;

PROCEDURE Decode*(Date: REAL; VAR Year, Month, Day, Hour, Min, Sec: INTEGER): BOOLEAN;
VAR Res, flag: BOOLEAN; d, t, i: INTEGER; M: ARRAY 14 OF CHAR;

  PROCEDURE MonthDay(n: INTEGER; VAR d, Month: INTEGER; M: ARRAY OF CHAR): BOOLEAN;
  VAR Res: BOOLEAN;
  BEGIN
    Res := FALSE;
    IF d > ORD(M[n]) - ORD("0") + 28 THEN
      d := d - ORD(M[n]) + ORD("0") - 28;
      INC(Month);
      Res := TRUE
    END
    RETURN Res
  END MonthDay;

BEGIN
  IF (Date >= -693593.0) & (Date < 2958466.0) THEN
    d := FLOOR(Date);
    t := FLOOR((Date - FLT(d)) * 86400000.0);
    d := d + 693593;
    Year := 1;
    Month := 1;
    WHILE d > 0 DO
      d := d - 365 - ORD((Year MOD 4 = 0) & (Year MOD 100 # 0) OR (Year MOD 400 = 0));
      INC(Year)
    END;
    IF d < 0 THEN
      DEC(Year);
      d := d + 365 + ORD((Year MOD 4 = 0) & (Year MOD 100 # 0) OR (Year MOD 400 = 0))
    END;
    INC(d);
    M := "_303232332323";
    IF (Year MOD 4 = 0) & (Year MOD 100 # 0) OR (Year MOD 400 = 0) THEN
      M[2] := "1"
    END;
    i := 1;
    flag := TRUE;
    WHILE flag & (i <= 12) DO
      flag := MonthDay(i, d, Month, M);
      INC(i)
    END;
    Day := d;
    Hour := t DIV 3600000;
    t := t MOD 3600000;
    Min := t DIV 60000;
    t := t MOD 60000;
    Sec := t DIV 1000;
    Res := TRUE
  ELSE
    Res := FALSE
  END
  RETURN Res
END Decode;

PROCEDURE Now*(VAR Year, Month, Day, Hour, Min, Sec, Msec: INTEGER);
VAR date, time: INTEGER;
BEGIN
  date  := KOSAPI.sysfunc1(29);
  time  := KOSAPI.sysfunc1(3);

  Year  := date MOD 16;
  date  := date DIV 16;
  Year  := (date MOD 16) * 10 + Year;
  date  := date DIV 16;

  Month := date MOD 16;
  date  := date DIV 16;
  Month := (date MOD 16) * 10 + Month;
  date  := date DIV 16;

  Day := date MOD 16;
  date  := date DIV 16;
  Day := (date MOD 16) * 10 + Day;
  date  := date DIV 16;

  Hour  := time MOD 16;
  time  := time DIV 16;
  Hour  := (time MOD 16) * 10 + Hour;
  time  := time DIV 16;

  Min := time MOD 16;
  time  := time DIV 16;
  Min := (time MOD 16) * 10 + Min;
  time  := time DIV 16;

  Sec := time MOD 16;
  time  := time DIV 16;
  Sec := (time MOD 16) * 10 + Sec;
  time  := time DIV 16;

  Year := Year + 2000;
  Msec := 0
END Now;

END DateTime.