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

MODULE Args;

IMPORT sys := SYSTEM, KOSAPI;

CONST

  MAX_PARAM = 1024;

VAR

  Params: ARRAY MAX_PARAM, 2 OF INTEGER;
  argc*: INTEGER;

PROCEDURE GetChar(adr: INTEGER): CHAR;
VAR res: CHAR;
BEGIN
  sys.GET(adr, res)
  RETURN res
END GetChar;

PROCEDURE ParamParse;
VAR p, count, name: INTEGER; c: CHAR; cond: INTEGER;

  PROCEDURE ChangeCond(A, B, C: INTEGER; c: CHAR; VAR cond: INTEGER);
  BEGIN
    IF (c <= 20X) & (c # 0X) THEN
      cond := A
    ELSIF c = 22X THEN
      cond := B
    ELSIF c = 0X THEN
      cond := 6
    ELSE
      cond := C
    END
  END ChangeCond;

BEGIN
  p := KOSAPI.GetCommandLine();
  name := KOSAPI.GetName();
  Params[0, 0] := name;
  WHILE GetChar(name) # 0X DO
    INC(name)
  END;
  Params[0, 1] := name - 1;
  cond := 0;
  count := 1;
  WHILE (argc < MAX_PARAM) & (cond # 6) DO
    c := GetChar(p);
    CASE cond OF
    |0: ChangeCond(0, 4, 1, c, cond); IF cond = 1 THEN Params[count, 0] := p END
    |1: ChangeCond(0, 3, 1, c, cond); IF cond IN {0, 6} THEN Params[count, 1] := p - 1; INC(count) END
    |3: ChangeCond(3, 1, 3, c, cond); IF cond = 6 THEN Params[count, 1] := p - 1; INC(count) END
    |4: ChangeCond(5, 0, 5, c, cond); IF cond = 5 THEN Params[count, 0] := p END
    |5: ChangeCond(5, 1, 5, c, cond); IF cond = 6 THEN Params[count, 1] := p - 1; INC(count) END
    ELSE
    END;
    INC(p)
  END;
  argc := count
END ParamParse;

PROCEDURE GetArg*(n: INTEGER; VAR s: ARRAY OF CHAR);
VAR i, j, len: INTEGER; c: CHAR;
BEGIN
  j := 0;
  IF n < argc THEN
    len := LEN(s) - 1;
    i := Params[n, 0];
    WHILE (j < len) & (i <= Params[n, 1]) DO
      c := GetChar(i);
      IF c # 22X THEN
        s[j] := c;
        INC(j)
      END;
      INC(i);
    END;
  END;
  s[j] := 0X
END GetArg;

BEGIN
  ParamParse
END Args.