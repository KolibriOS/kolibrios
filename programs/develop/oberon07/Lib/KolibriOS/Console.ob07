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

MODULE Console;

IMPORT ConsoleLib, In, Out;


CONST

    Black* = 0;      Blue* = 1;           Green* = 2;        Cyan* = 3;
    Red* = 4;        Magenta* = 5;        Brown* = 6;        LightGray* = 7;
    DarkGray* = 8;   LightBlue* = 9;      LightGreen* = 10;  LightCyan* = 11;
    LightRed* = 12;  LightMagenta* = 13;  Yellow* = 14;      White* = 15;


PROCEDURE SetCursor* (X, Y: INTEGER);
BEGIN
    ConsoleLib.set_cursor_pos(X, Y)
END SetCursor;


PROCEDURE GetCursor* (VAR X, Y: INTEGER);
BEGIN
    ConsoleLib.get_cursor_pos(X, Y)
END GetCursor;


PROCEDURE Cls*;
BEGIN
    ConsoleLib.cls
END Cls;


PROCEDURE SetColor* (FColor, BColor: INTEGER);
VAR
    res: INTEGER;

BEGIN
    IF (FColor IN {0..15}) & (BColor IN {0..15}) THEN
        res := ConsoleLib.set_flags(LSL(BColor, 4) + FColor)
    END
END SetColor;


PROCEDURE GetCursorX* (): INTEGER;
VAR
    x, y: INTEGER;

BEGIN
    ConsoleLib.get_cursor_pos(x, y)
    RETURN x
END GetCursorX;


PROCEDURE GetCursorY* (): INTEGER;
VAR
    x, y: INTEGER;

BEGIN
    ConsoleLib.get_cursor_pos(x, y)
    RETURN y
END GetCursorY;


PROCEDURE open*;
BEGIN
    ConsoleLib.open(-1, -1, -1, -1, "");
    In.Open;
    Out.Open
END open;


PROCEDURE exit* (bCloseWindow: BOOLEAN);
BEGIN
    ConsoleLib.exit(bCloseWindow)
END exit;


END Console.
