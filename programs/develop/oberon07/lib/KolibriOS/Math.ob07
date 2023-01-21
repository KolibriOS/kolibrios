(*
    BSD 2-Clause License

    Copyright (c) 2013-2014, 2018-2022 Anton Krotov
    All rights reserved.
*)

MODULE Math;

IMPORT SYSTEM;


CONST

    pi* = 3.141592653589793;
    e*  = 2.718281828459045;


PROCEDURE IsNan* (x: REAL): BOOLEAN;
VAR
    h, l: SET;

BEGIN
    SYSTEM.GET(SYSTEM.ADR(x), l);
    SYSTEM.GET(SYSTEM.ADR(x) + 4, h)
    RETURN (h * {20..30} = {20..30}) & ((h * {0..19} # {}) OR (l * {0..31} # {}))
END IsNan;


PROCEDURE IsInf* (x: REAL): BOOLEAN;
    RETURN ABS(x) = SYSTEM.INF()
END IsInf;


PROCEDURE Max (a, b: REAL): REAL;
VAR
    res: REAL;

BEGIN
    IF a > b THEN
        res := a
    ELSE
        res := b
    END
    RETURN res
END Max;


PROCEDURE Min (a, b: REAL): REAL;
VAR
    res: REAL;

BEGIN
    IF a < b THEN
        res := a
    ELSE
        res := b
    END
    RETURN res
END Min;


PROCEDURE SameValue (a, b: REAL): BOOLEAN;
VAR
    eps: REAL;
    res: BOOLEAN;

BEGIN
    eps := Max(Min(ABS(a), ABS(b)) * 1.0E-12, 1.0E-12);
    IF a > b THEN
        res := (a - b) <= eps
    ELSE
        res := (b - a) <= eps
    END
    RETURN res
END SameValue;


PROCEDURE IsZero (x: REAL): BOOLEAN;
    RETURN ABS(x) <= 1.0E-12
END IsZero;


PROCEDURE [stdcall] sqrt* (x: REAL): REAL;
BEGIN
    SYSTEM.CODE(
    0DDH, 045H, 008H,              (*  fld     qword [ebp + 08h]  *)
    0D9H, 0FAH,                    (*  fsqrt                      *)
    0C9H,                          (*  leave                      *)
    0C2H, 008H, 000H               (*  ret     08h                *)
    )
    RETURN 0.0
END sqrt;


PROCEDURE [stdcall] sin* (x: REAL): REAL;
BEGIN
    SYSTEM.CODE(
    0DDH, 045H, 008H,              (*  fld     qword [ebp + 08h]  *)
    0D9H, 0FEH,                    (*  fsin                       *)
    0C9H,                          (*  leave                      *)
    0C2H, 008H, 000H               (*  ret     08h                *)
    )
    RETURN 0.0
END sin;


PROCEDURE [stdcall] cos* (x: REAL): REAL;
BEGIN
    SYSTEM.CODE(
    0DDH, 045H, 008H,              (*  fld     qword [ebp + 08h]  *)
    0D9H, 0FFH,                    (*  fcos                       *)
    0C9H,                          (*  leave                      *)
    0C2H, 008H, 000H               (*  ret     08h                *)
    )
    RETURN 0.0
END cos;


PROCEDURE [stdcall] tan* (x: REAL): REAL;
BEGIN
    SYSTEM.CODE(
    0DDH, 045H, 008H,              (*  fld     qword [ebp + 08h]  *)
    0D9H, 0FBH,                    (*  fsincos                    *)
    0DEH, 0F9H,                    (*  fdivp st1, st              *)
    0C9H,                          (*  leave                      *)
    0C2H, 008H, 000H               (*  ret     08h                *)
    )
    RETURN 0.0
END tan;


PROCEDURE [stdcall] arctan2* (y, x: REAL): REAL;
BEGIN
    SYSTEM.CODE(
    0DDH, 045H, 008H,              (*  fld     qword [ebp + 08h]  *)
    0DDH, 045H, 010H,              (*  fld     qword [ebp + 10h]  *)
    0D9H, 0F3H,                    (*  fpatan                     *)
    0C9H,                          (*  leave                      *)
    0C2H, 010H, 000H               (*  ret     10h                *)
    )
    RETURN 0.0
END arctan2;


PROCEDURE [stdcall] ln* (x: REAL): REAL;
BEGIN
    SYSTEM.CODE(
    0D9H, 0EDH,                    (*  fldln2                     *)
    0DDH, 045H, 008H,              (*  fld     qword [ebp + 08h]  *)
    0D9H, 0F1H,                    (*  fyl2x                      *)
    0C9H,                          (*  leave                      *)
    0C2H, 008H, 000H               (*  ret     08h                *)
    )
    RETURN 0.0
END ln;


PROCEDURE [stdcall] log* (base, x: REAL): REAL;
BEGIN
    SYSTEM.CODE(
    0D9H, 0E8H,                    (*  fld1                       *)
    0DDH, 045H, 010H,              (*  fld     qword [ebp + 10h]  *)
    0D9H, 0F1H,                    (*  fyl2x                      *)
    0D9H, 0E8H,                    (*  fld1                       *)
    0DDH, 045H, 008H,              (*  fld     qword [ebp + 08h]  *)
    0D9H, 0F1H,                    (*  fyl2x                      *)
    0DEH, 0F9H,                    (*  fdivp st1, st              *)
    0C9H,                          (*  leave                      *)
    0C2H, 010H, 000H               (*  ret     10h                *)
    )
    RETURN 0.0
END log;


PROCEDURE [stdcall] exp* (x: REAL): REAL;
BEGIN
    SYSTEM.CODE(
    0DDH, 045H, 008H,           (*  fld     qword [ebp + 08h]  *)
    0D9H, 0EAH,                 (*  fldl2e                     *)
    0DEH, 0C9H, 0D9H, 0C0H,
    0D9H, 0FCH, 0DCH, 0E9H,
    0D9H, 0C9H, 0D9H, 0F0H,
    0D9H, 0E8H, 0DEH, 0C1H,
    0D9H, 0FDH, 0DDH, 0D9H,
    0C9H,                       (*  leave                      *)
    0C2H, 008H, 000H            (*  ret     08h                *)
    )
    RETURN 0.0
END exp;


PROCEDURE [stdcall] round* (x: REAL): REAL;
BEGIN
    SYSTEM.CODE(
    0DDH, 045H, 008H,           (*  fld     qword [ebp + 08h]  *)
    0D9H, 07DH, 0F4H, 0D9H,
    07DH, 0F6H, 066H, 081H,
    04DH, 0F6H, 000H, 003H,
    0D9H, 06DH, 0F6H, 0D9H,
    0FCH, 0D9H, 06DH, 0F4H,
    0C9H,                       (*  leave                     *)
    0C2H, 008H, 000H            (*  ret     08h               *)
    )
    RETURN 0.0
END round;


PROCEDURE [stdcall] frac* (x: REAL): REAL;
BEGIN
    SYSTEM.CODE(
    050H,
    0DDH, 045H, 008H,           (*  fld     qword [ebp + 08h]  *)
    0D9H, 0C0H, 0D9H, 03CH,
    024H, 0D9H, 07CH, 024H,
    002H, 066H, 081H, 04CH,
    024H, 002H, 000H, 00FH,
    0D9H, 06CH, 024H, 002H,
    0D9H, 0FCH, 0D9H, 02CH,
    024H, 0DEH, 0E9H,
    0C9H,                       (*  leave                     *)
    0C2H, 008H, 000H            (*  ret     08h               *)
    )
    RETURN 0.0
END frac;


PROCEDURE sqri* (x: INTEGER): INTEGER;
    RETURN x * x
END sqri;


PROCEDURE sqrr* (x: REAL): REAL;
    RETURN x * x
END sqrr;


PROCEDURE arcsin* (x: REAL): REAL;
    RETURN arctan2(x, sqrt(1.0 - x * x))
END arcsin;


PROCEDURE arccos* (x: REAL): REAL;
    RETURN arctan2(sqrt(1.0 - x * x), x)
END arccos;


PROCEDURE arctan* (x: REAL): REAL;
    RETURN arctan2(x, 1.0)
END arctan;


PROCEDURE sinh* (x: REAL): REAL;
BEGIN
    x := exp(x)
    RETURN (x - 1.0 / x) * 0.5
END sinh;


PROCEDURE cosh* (x: REAL): REAL;
BEGIN
    x := exp(x)
    RETURN (x + 1.0 / x) * 0.5
END cosh;


PROCEDURE tanh* (x: REAL): REAL;
BEGIN
    IF x > 15.0 THEN
        x := 1.0
    ELSIF x < -15.0 THEN
        x := -1.0
    ELSE
        x := 1.0 - 2.0 / (exp(2.0 * x) + 1.0)
    END

    RETURN x
END tanh;


PROCEDURE arsinh* (x: REAL): REAL;
    RETURN ln(x + sqrt(x * x + 1.0))
END arsinh;


PROCEDURE arcosh* (x: REAL): REAL;
    RETURN ln(x + sqrt(x * x - 1.0))
END arcosh;


PROCEDURE artanh* (x: REAL): REAL;
VAR
    res: REAL;

BEGIN
    IF SameValue(x, 1.0) THEN
        res := SYSTEM.INF()
    ELSIF SameValue(x, -1.0) THEN
        res := -SYSTEM.INF()
    ELSE
        res := 0.5 * ln((1.0 + x) / (1.0 - x))
    END
    RETURN res
END artanh;


PROCEDURE floor* (x: REAL): REAL;
VAR
    f: REAL;

BEGIN
    f := frac(x);
    x := x - f;
    IF f < 0.0 THEN
        x := x - 1.0
    END
    RETURN x
END floor;


PROCEDURE ceil* (x: REAL): REAL;
VAR
    f: REAL;

BEGIN
    f := frac(x);
    x := x - f;
    IF f > 0.0 THEN
        x := x + 1.0
    END
    RETURN x
END ceil;


PROCEDURE power* (base, exponent: REAL): REAL;
VAR
    res: REAL;

BEGIN
    IF exponent = 0.0 THEN
        res := 1.0
    ELSIF (base = 0.0) & (exponent > 0.0) THEN
        res := 0.0
    ELSE
        res := exp(exponent * ln(base))
    END
    RETURN res
END power;


PROCEDURE ipower* (base: REAL; exponent: INTEGER): REAL;
VAR
    i: INTEGER;
    a: REAL;

BEGIN
    a := 1.0;

    IF base # 0.0 THEN
        IF exponent # 0 THEN
            IF exponent < 0 THEN
                base := 1.0 / base
            END;
            i := ABS(exponent);
            WHILE i > 0 DO
                WHILE ~ODD(i) DO
                    i := LSR(i, 1);
                    base := sqrr(base)
                END;
                DEC(i);
                a := a * base
            END
        ELSE
            a := 1.0
        END
    ELSE
        ASSERT(exponent > 0);
        a := 0.0
    END

    RETURN a
END ipower;


PROCEDURE sgn* (x: REAL): INTEGER;
VAR
    res: INTEGER;

BEGIN
    IF x > 0.0 THEN
        res := 1
    ELSIF x < 0.0 THEN
        res := -1
    ELSE
        res := 0
    END

    RETURN res
END sgn;


PROCEDURE fact* (n: INTEGER): REAL;
VAR
    res: REAL;

BEGIN
    res := 1.0;
    WHILE n > 1 DO
        res := res * FLT(n);
        DEC(n)
    END

    RETURN res
END fact;


PROCEDURE DegToRad* (x: REAL): REAL;
    RETURN x * (pi / 180.0)
END DegToRad;


PROCEDURE RadToDeg* (x: REAL): REAL;
    RETURN x * (180.0 / pi)
END RadToDeg;


(* Return hypotenuse of triangle *)
PROCEDURE hypot* (x, y: REAL): REAL;
VAR
    a: REAL;

BEGIN
    x := ABS(x);
    y := ABS(y);
    IF x > y THEN
        a := x * sqrt(1.0 + sqrr(y / x))
    ELSE
        IF x > 0.0 THEN
            a := y * sqrt(1.0 + sqrr(x / y))
        ELSE
            a := y
        END
    END

    RETURN a
END hypot;


END Math.