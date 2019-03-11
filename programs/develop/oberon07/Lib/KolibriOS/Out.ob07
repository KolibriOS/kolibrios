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

MODULE Out;

IMPORT ConsoleLib, sys := SYSTEM;

CONST

  d = 1.0 - 5.0E-12;

VAR

  Realp: PROCEDURE (x: REAL; width: INTEGER);

PROCEDURE Char*(c: CHAR);
BEGIN
  ConsoleLib.write_string(sys.ADR(c), 1)
END Char;

PROCEDURE String*(s: ARRAY OF CHAR);
BEGIN
  ConsoleLib.write_string(sys.ADR(s[0]), LENGTH(s))
END String;

PROCEDURE WriteInt(x, n: INTEGER);
VAR i: INTEGER; a: ARRAY 16 OF CHAR; neg: BOOLEAN;
BEGIN
  i := 0;
  IF n < 1 THEN
    n := 1
  END;
  IF x < 0 THEN
    x := -x;
    DEC(n);
    neg := TRUE
  END;
  REPEAT
    a[i] := CHR(x MOD 10 + ORD("0"));
    x := x DIV 10;
    INC(i)
  UNTIL x = 0;
  WHILE n > i DO
    Char(" ");
    DEC(n)
  END;
  IF neg THEN
    Char("-")
  END;
  REPEAT
    DEC(i);
    Char(a[i])
  UNTIL i = 0
END WriteInt;

PROCEDURE IsNan(AValue: REAL): BOOLEAN;
VAR h, l: SET;
BEGIN
  sys.GET(sys.ADR(AValue), l);
  sys.GET(sys.ADR(AValue) + 4, h)
  RETURN (h * {20..30} = {20..30}) & ((h * {0..19} # {}) OR (l * {0..31} # {}))
END IsNan;

PROCEDURE IsInf(x: REAL): BOOLEAN;
  RETURN ABS(x) = sys.INF()
END IsInf;

PROCEDURE Int*(x, width: INTEGER);
VAR i: INTEGER;
BEGIN
  IF x # 80000000H THEN
    WriteInt(x, width)
  ELSE
    FOR i := 12 TO width DO
      Char(20X)
    END;
    String("-2147483648")
  END
END Int;

PROCEDURE OutInf(x: REAL; width: INTEGER);
VAR s: ARRAY 5 OF CHAR; i: INTEGER;
BEGIN
  IF IsNan(x) THEN
    s := "Nan";
    INC(width)
  ELSIF IsInf(x) & (x > 0.0) THEN
    s := "+Inf"
  ELSIF IsInf(x) & (x < 0.0) THEN
    s := "-Inf"
  END;
  FOR i := 1 TO width - 4 DO
    Char(" ")
  END;
  String(s)
END OutInf;

PROCEDURE Ln*;
BEGIN
  Char(0DX);
  Char(0AX)
END Ln;

PROCEDURE _FixReal(x: REAL; width, p: INTEGER);
VAR e, len, i: INTEGER; y: REAL; minus: BOOLEAN;
BEGIN
  IF IsNan(x) OR IsInf(x) THEN
    OutInf(x, width)
  ELSIF p < 0 THEN
    Realp(x, width)
  ELSE
    len := 0;
    minus := FALSE;
    IF x < 0.0 THEN
      minus := TRUE;
      INC(len);
      x := ABS(x)
    END;
    e := 0;
    WHILE x >= 10.0 DO
      x := x / 10.0;
      INC(e)
    END;
    IF e >= 0 THEN
      len := len + e + p + 1;
      IF x > 9.0 + d THEN
        INC(len)
      END;
      IF p > 0 THEN
        INC(len)
      END
    ELSE
      len := len + p + 2
    END;
    FOR i := 1 TO width - len DO
      Char(" ")
    END;
    IF minus THEN
      Char("-")
    END;
    y := x;
    WHILE (y < 1.0) & (y # 0.0) DO
      y := y * 10.0;
      DEC(e)
    END;
    IF e < 0 THEN
      IF x - FLT(FLOOR(x)) > d THEN
        Char("1");
        x := 0.0
      ELSE
        Char("0");
        x := x * 10.0
      END
    ELSE
      WHILE e >= 0 DO
        IF x - FLT(FLOOR(x)) > d THEN
          IF x > 9.0 THEN
            String("10")
          ELSE
            Char(CHR(FLOOR(x) + ORD("0") + 1))
          END;
          x := 0.0
        ELSE
          Char(CHR(FLOOR(x) + ORD("0")));
          x := (x - FLT(FLOOR(x))) * 10.0
        END;
        DEC(e)
      END
    END;
    IF p > 0 THEN
      Char(".")
    END;
    WHILE p > 0 DO
      IF x - FLT(FLOOR(x)) > d THEN
        Char(CHR(FLOOR(x) + ORD("0") + 1));
        x := 0.0
      ELSE
        Char(CHR(FLOOR(x) + ORD("0")));
        x := (x - FLT(FLOOR(x))) * 10.0
      END;
      DEC(p)
    END
  END
END _FixReal;

PROCEDURE Real*(x: REAL; width: INTEGER);
VAR e, n, i: INTEGER; minus: BOOLEAN;
BEGIN
  IF IsNan(x) OR IsInf(x) THEN
    OutInf(x, width)
  ELSE
    e := 0;
    n := 0;
    IF width > 23 THEN
      n := width - 23;
      width := 23
    ELSIF width < 9 THEN
      width := 9
    END;
    width := width - 5;
    IF x < 0.0 THEN
      x := -x;
      minus := TRUE
    ELSE
      minus := FALSE
    END;
    WHILE x >= 10.0 DO
      x := x / 10.0;
      INC(e)
    END;
    WHILE (x < 1.0) & (x # 0.0) DO
      x := x * 10.0;
      DEC(e)
    END;
    IF x > 9.0 + d THEN
      x := 1.0;
      INC(e)
    END;
    FOR i := 1 TO n DO
      Char(" ")
    END;
    IF minus THEN
      x := -x
    END;
    Realp := Real;
    _FixReal(x, width, width - 3);
    Char("E");
    IF e >= 0 THEN
      Char("+")
    ELSE
      Char("-");
      e := ABS(e)
    END;
    IF e < 100 THEN
      Char("0")
    END;
    IF e < 10 THEN
      Char("0")
    END;
    Int(e, 0)
  END
END Real;

PROCEDURE FixReal*(x: REAL; width, p: INTEGER);
BEGIN
  Realp := Real;
  _FixReal(x, width, p)
END FixReal;

PROCEDURE Open*;
END Open;

END Out.