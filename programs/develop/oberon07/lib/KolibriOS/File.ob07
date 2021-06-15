(*
    Copyright 2016, 2018, 2021 Anton Krotov

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

MODULE File;

IMPORT sys := SYSTEM, KOSAPI;


CONST

    SEEK_BEG* = 0; SEEK_CUR* = 1; SEEK_END* = 2;


TYPE

    FNAME* = ARRAY 520 OF CHAR;

    FS* = POINTER TO rFS;

    rFS* = RECORD
        subfunc*, pos*, hpos*, bytes*, buffer*: INTEGER;
        name*: FNAME
    END;

    FD* = POINTER TO rFD;

    rFD* = RECORD
        attr*: INTEGER;
        ntyp*: CHAR;
        reserved: ARRAY 3 OF CHAR;
        time_create*, date_create*,
        time_access*, date_access*,
        time_modif*,  date_modif*,
        size*, hsize*: INTEGER;
        name*: FNAME
    END;


PROCEDURE [stdcall] f_68_27 (file_name: INTEGER; VAR size: INTEGER): INTEGER;
BEGIN
    sys.CODE(
    053H,               (*  push    ebx                    *)
    06AH, 044H,         (*  push    68                     *)
    058H,               (*  pop     eax                    *)
    06AH, 01BH,         (*  push    27                     *)
    05BH,               (*  pop     ebx                    *)
    08BH, 04DH, 008H,   (*  mov     ecx, dword [ebp +  8]  *)
    0CDH, 040H,         (*  int     64                     *)
    08BH, 04DH, 00CH,   (*  mov     ecx, dword [ebp + 12]  *)
    089H, 011H,         (*  mov     dword [ecx], edx       *)
    05BH,               (*  pop     ebx                    *)
    0C9H,               (*  leave                          *)
    0C2H, 008H, 000H    (*  ret     8                      *)
    )
    RETURN 0
END f_68_27;


PROCEDURE Load* (FName: ARRAY OF CHAR; VAR size: INTEGER): INTEGER;
    RETURN f_68_27(sys.ADR(FName[0]), size)
END Load;


PROCEDURE GetFileInfo* (FName: ARRAY OF CHAR; VAR Info: rFD): BOOLEAN;
VAR
    res2: INTEGER; fs: rFS;

BEGIN
    fs.subfunc := 5;
    fs.pos     := 0;
    fs.hpos    := 0;
    fs.bytes   := 0;
    fs.buffer  := sys.ADR(Info);
    COPY(FName, fs.name)

    RETURN KOSAPI.sysfunc22(70, sys.ADR(fs), res2) = 0
END GetFileInfo;


PROCEDURE FileSize* (FName: ARRAY OF CHAR): INTEGER;
VAR
    Info: rFD;
    res: INTEGER;
BEGIN
    IF GetFileInfo(FName, Info) THEN
        res := Info.size
    ELSE
        res := -1
    END
    RETURN res
END FileSize;


PROCEDURE Exists* (FName: ARRAY OF CHAR): BOOLEAN;
VAR
    fd: rFD;
BEGIN
    RETURN GetFileInfo(FName, fd) & ~(4 IN BITS(fd.attr))
END Exists;


PROCEDURE Close* (VAR F: FS);
BEGIN
    IF F # NIL THEN
        DISPOSE(F)
    END
END Close;


PROCEDURE Open* (FName: ARRAY OF CHAR): FS;
VAR
    F: FS;

BEGIN

    IF Exists(FName) THEN
        NEW(F);
        IF F # NIL THEN
            F.subfunc := 0;
            F.pos     := 0;
            F.hpos    := 0;
            F.bytes   := 0;
            F.buffer  := 0;
            COPY(FName, F.name)
        END
    ELSE
        F := NIL
    END

    RETURN F
END Open;


PROCEDURE Delete* (FName: ARRAY OF CHAR): BOOLEAN;
VAR
    F: FS;
    res, res2: INTEGER;

BEGIN

    IF Exists(FName) THEN
        NEW(F);
        IF F # NIL THEN
            F.subfunc := 8;
            F.pos     := 0;
            F.hpos    := 0;
            F.bytes   := 0;
            F.buffer  := 0;
            COPY(FName, F.name);
            res := KOSAPI.sysfunc22(70, sys.ADR(F^), res2);
            DISPOSE(F)
        ELSE
            res := -1
        END
    ELSE
        res := -1
    END

    RETURN res = 0
END Delete;


PROCEDURE Seek* (F: FS; Offset, Origin: INTEGER): INTEGER;
VAR
    res: INTEGER;
    fd: rFD;

BEGIN

    IF (F # NIL) & GetFileInfo(F.name, fd) & (BITS(fd.attr) * {4} = {}) THEN
        CASE Origin OF
        |SEEK_BEG: F.pos := Offset
        |SEEK_CUR: F.pos := F.pos + Offset
        |SEEK_END: F.pos := fd.size + Offset
        ELSE
        END;
        res := F.pos
    ELSE
        res := -1
    END

    RETURN res
END Seek;


PROCEDURE Read* (F: FS; Buffer, Count: INTEGER): INTEGER;
VAR
    res, res2: INTEGER;

BEGIN

    IF F # NIL THEN
        F.subfunc := 0;
        F.bytes   := Count;
        F.buffer  := Buffer;
        res := KOSAPI.sysfunc22(70, sys.ADR(F^), res2);
        IF res2 > 0 THEN
            F.pos := F.pos + res2
        END
    ELSE
        res2 := 0
    END

    RETURN res2
END Read;


PROCEDURE Write* (F: FS; Buffer, Count: INTEGER): INTEGER;
VAR
    res, res2: INTEGER;

BEGIN

    IF F # NIL THEN
        F.subfunc := 3;
        F.bytes   := Count;
        F.buffer  := Buffer;
        res := KOSAPI.sysfunc22(70, sys.ADR(F^), res2);
        IF res2 > 0 THEN
            F.pos := F.pos + res2
        END
    ELSE
        res2 := 0
    END

    RETURN res2
END Write;


PROCEDURE Create* (FName: ARRAY OF CHAR): FS;
VAR
    F: FS;
    res2: INTEGER;

BEGIN
    NEW(F);

    IF F # NIL THEN
        F.subfunc := 2;
        F.pos     := 0;
        F.hpos    := 0;
        F.bytes   := 0;
        F.buffer  := 0;
        COPY(FName, F.name);
        IF KOSAPI.sysfunc22(70, sys.ADR(F^), res2) # 0 THEN
            DISPOSE(F)
        END
    END

    RETURN F
END Create;


PROCEDURE DirExists* (FName: ARRAY OF CHAR): BOOLEAN;
VAR
    fd: rFD;
BEGIN
    RETURN GetFileInfo(FName, fd) & (4 IN BITS(fd.attr))
END DirExists;


PROCEDURE CreateDir* (DirName: ARRAY OF CHAR): BOOLEAN;
VAR
    F: FS;
    res, res2: INTEGER;

BEGIN
    NEW(F);

    IF F # NIL THEN
        F.subfunc := 9;
        F.pos     := 0;
        F.hpos    := 0;
        F.bytes   := 0;
        F.buffer  := 0;
        COPY(DirName, F.name);
        res := KOSAPI.sysfunc22(70, sys.ADR(F^), res2);
        DISPOSE(F)
    ELSE
        res := -1
    END

    RETURN res = 0
END CreateDir;


PROCEDURE DeleteDir* (DirName: ARRAY OF CHAR): BOOLEAN;
VAR
    F: FS;
    res, res2: INTEGER;

BEGIN

    IF DirExists(DirName) THEN
        NEW(F);
        IF F # NIL THEN
            F.subfunc := 8;
            F.pos := 0;
            F.hpos := 0;
            F.bytes := 0;
            F.buffer := 0;
            COPY(DirName, F.name);
            res := KOSAPI.sysfunc22(70, sys.ADR(F^), res2);
            DISPOSE(F)
        ELSE
            res := -1
        END
    ELSE
        res := -1
    END

    RETURN res = 0
END DeleteDir;


END File.