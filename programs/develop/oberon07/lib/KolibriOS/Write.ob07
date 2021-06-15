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

MODULE Write;

IMPORT File, sys := SYSTEM;

PROCEDURE Char*(F: File.FS; x: CHAR): BOOLEAN;
  RETURN File.Write(F, sys.ADR(x), sys.SIZE(CHAR)) = sys.SIZE(CHAR)
END Char;

PROCEDURE Int*(F: File.FS; x: INTEGER): BOOLEAN;
  RETURN File.Write(F, sys.ADR(x), sys.SIZE(INTEGER)) = sys.SIZE(INTEGER)
END Int;

PROCEDURE Real*(F: File.FS; x: REAL): BOOLEAN;
  RETURN File.Write(F, sys.ADR(x), sys.SIZE(REAL)) = sys.SIZE(REAL)
END Real;

PROCEDURE Boolean*(F: File.FS; x: BOOLEAN): BOOLEAN;
  RETURN File.Write(F, sys.ADR(x), sys.SIZE(BOOLEAN)) = sys.SIZE(BOOLEAN)
END Boolean;

PROCEDURE Set*(F: File.FS; x: SET): BOOLEAN;
  RETURN File.Write(F, sys.ADR(x), sys.SIZE(SET)) = sys.SIZE(SET)
END Set;

PROCEDURE WChar*(F: File.FS; x: WCHAR): BOOLEAN;
  RETURN File.Write(F, sys.ADR(x), sys.SIZE(WCHAR)) = sys.SIZE(WCHAR)
END WChar;

END Write.