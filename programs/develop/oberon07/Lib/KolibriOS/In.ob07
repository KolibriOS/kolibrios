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

MODULE In;

IMPORT sys := SYSTEM, ConsoleLib;

TYPE

  STRING = ARRAY 260 OF CHAR;

VAR

  Done* : BOOLEAN;

PROCEDURE digit(ch: CHAR): BOOLEAN;
  RETURN (ch >= "0") & (ch <= "9")
END digit;

PROCEDURE CheckInt(s: STRING; VAR first, last: INTEGER; VAR neg: BOOLEAN; Point: BOOLEAN): BOOLEAN;
VAR i: INTEGER;
BEGIN
  i := 0;
  neg := FALSE;
  WHILE (s[i] <= 20X) & (s[i] # 0X) DO
    INC(i)
  END;
  IF s[i] = "-" THEN
    neg := TRUE;
    INC(i)
  ELSIF s[i] = "+" THEN
    INC(i)
  END;
  first := i;
  WHILE digit(s[i]) DO
    INC(i)
  END;
  last := i
  RETURN ((s[i] <= 20X) OR (Point & (s[i] = "."))) & digit(s[first])
END CheckInt;

PROCEDURE IsMinInt(str: STRING; pos: INTEGER): BOOLEAN;
VAR i: INTEGER; min: STRING;
BEGIN
  i := 0;
  min := "2147483648";
  WHILE (min[i] # 0X) & (str[i] # 0X) & (min[i] = str[i + pos]) DO
    INC(i)
  END
  RETURN i = 10
END IsMinInt;

PROCEDURE StrToInt(str: STRING; VAR err: BOOLEAN): INTEGER;
CONST maxINT = 7FFFFFFFH;
VAR i, n, res: INTEGER; flag, neg: BOOLEAN;
BEGIN
  res := 0;
  flag := CheckInt(str, i, n, neg, FALSE);
  err := ~flag;
  IF flag & neg & IsMinInt(str, i) THEN
    flag := FALSE;
    neg := FALSE;
    res := 80000000H
  END;
  WHILE flag & digit(str[i]) DO
    IF res > maxINT DIV 10 THEN
      err := TRUE;
      flag := FALSE;
      res := 0
    ELSE
      res := res * 10;
      IF res > maxINT - (ORD(str[i]) - ORD("0")) THEN
        err := TRUE;
        flag := FALSE;
        res := 0
      ELSE
        res := res + (ORD(str[i]) - ORD("0"));
        INC(i)
      END
    END
  END;
  IF neg THEN
    res := -res
  END
  RETURN res
END StrToInt;

PROCEDURE Space(s: STRING): BOOLEAN;
VAR i: INTEGER;
BEGIN
  i := 0;
  WHILE (s[i] # 0X) & (s[i] <= 20X) DO
    INC(i)
  END
  RETURN s[i] = 0X
END Space;

PROCEDURE CheckReal(s: STRING; VAR n: INTEGER; VAR neg: BOOLEAN): BOOLEAN;
VAR i: INTEGER; Res: BOOLEAN;
BEGIN
  Res := CheckInt(s, n, i, neg, TRUE);
  IF Res THEN
    IF s[i] = "." THEN
      INC(i);
      WHILE digit(s[i]) DO
        INC(i)
      END;
      IF (s[i] = "D") OR (s[i] = "E") OR (s[i] = "d") OR (s[i] = "e") THEN
        INC(i);
        IF (s[i] = "+") OR (s[i] = "-") THEN
          INC(i)
        END;
        Res := digit(s[i]);
        WHILE digit(s[i]) DO
          INC(i)
        END
      END
    END
  END
  RETURN Res & (s[i] <= 20X)
END CheckReal;

PROCEDURE StrToFloat(str: STRING; VAR err: BOOLEAN): REAL;
CONST maxDBL = 1.69E308; maxINT = 7FFFFFFFH;
VAR i, scale: INTEGER; res, m, d: REAL; minus, neg: BOOLEAN;

  PROCEDURE part1 (str: STRING; VAR res, d: REAL; VAR i: INTEGER): BOOLEAN;
  BEGIN
    res := 0.0;
    d := 1.0;
    WHILE digit(str[i]) DO
      res := res * 10.0 + FLT(ORD(str[i]) - ORD("0"));
      INC(i)
    END;
    IF str[i] = "." THEN
      INC(i);
      WHILE digit(str[i]) DO
        d := d / 10.0;
        res := res + FLT(ORD(str[i]) - ORD("0")) * d;
        INC(i)
      END
    END
    RETURN str[i] # 0X
  END part1;

  PROCEDURE part2 (str: STRING; VAR i, scale: INTEGER; VAR minus, err: BOOLEAN; VAR m, res: REAL): BOOLEAN;
  BEGIN
    INC(i);
    m := 10.0;
    minus := FALSE;
    IF str[i] = "+" THEN
      INC(i)
    ELSIF str[i] = "-" THEN
      minus := TRUE;
      INC(i);
      m := 0.1
    END;
    scale := 0;
    err := FALSE;
    WHILE ~err & digit(str[i]) DO
      IF scale > maxINT DIV 10 THEN
        err := TRUE;
        res := 0.0
      ELSE
        scale := scale * 10;
        IF scale > maxINT - (ORD(str[i]) - ORD("0")) THEN
          err := TRUE;
          res := 0.0
        ELSE
          scale := scale + (ORD(str[i]) - ORD("0"));
          INC(i)
        END
      END
    END
    RETURN ~err
  END part2;

  PROCEDURE part3 (VAR err, minus: BOOLEAN; VAR res, m: REAL; VAR scale: INTEGER);
  VAR i: INTEGER;
  BEGIN
    err := FALSE;
    IF scale = maxINT THEN
      err := TRUE;
      res := 0.0
    END;
    i := 1;
    WHILE ~err & (i <= scale) DO
      IF ~minus & (res > maxDBL / m) THEN
        err := TRUE;
        res := 0.0
      ELSE
        res := res * m;
        INC(i)
      END
    END
  END part3;

BEGIN
  IF CheckReal(str, i, neg) THEN
    IF part1(str, res, d, i) & part2(str, i, scale, minus, err, m, res) THEN
      part3(err, minus, res, m, scale)
    END;
    IF neg THEN
      res := -res
    END
  ELSE
    res := 0.0;
    err := TRUE
  END
  RETURN res
END StrToFloat;

PROCEDURE String*(VAR s: ARRAY OF CHAR);
VAR res, length: INTEGER; str: STRING;
BEGIN
  res := ConsoleLib.gets(sys.ADR(str[0]), LEN(str));
  length := LENGTH(str);
  IF length > 0 THEN
    str[length - 1] := 0X
  END;
  COPY(str, s);
  Done := TRUE
END String;

PROCEDURE Char*(VAR x: CHAR);
VAR str: STRING;
BEGIN
  String(str);
  x := str[0];
  Done := TRUE
END Char;

PROCEDURE Ln*;
VAR str: STRING;
BEGIN
  String(str);
  Done := TRUE
END Ln;

PROCEDURE Real* (VAR x: REAL);
VAR str: STRING; err: BOOLEAN;
BEGIN
  err := FALSE;
  REPEAT
    String(str)
  UNTIL ~Space(str);
  x := StrToFloat(str, err);
  Done := ~err
END Real;


PROCEDURE Int*(VAR x: INTEGER);
VAR str: STRING; err: BOOLEAN;
BEGIN
  err := FALSE;
  REPEAT
    String(str)
  UNTIL ~Space(str);
  x := StrToInt(str, err);
  Done := ~err
END Int;

PROCEDURE Open*;
BEGIN
  Done := TRUE
END Open;

END In.
