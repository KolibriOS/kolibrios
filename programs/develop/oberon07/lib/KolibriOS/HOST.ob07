(*
    BSD 2-Clause License

    Copyright (c) 2018-2022, Anton Krotov
    All rights reserved.
*)

MODULE HOST;

IMPORT SYSTEM, K := KOSAPI, API;


CONST

    slash* = "/";
    eol* = 0DX + 0AX;

    bit_depth* = API.BIT_DEPTH;
    maxint* = ROR(-2, 1);
    minint* = ROR(1, 1);

    MAX_PARAM = 1024;


TYPE

    DAYS = ARRAY 12, 31, 2 OF INTEGER;

    FNAME = ARRAY 520 OF CHAR;

    FS = POINTER TO rFS;

    rFS = RECORD
        subfunc, pos, hpos, bytes, buffer: INTEGER;
        name: FNAME
    END;

    FD = POINTER TO rFD;

    rFD = RECORD
        attr: INTEGER;
        ntyp: CHAR;
        reserved: ARRAY 3 OF CHAR;
        time_create, date_create,
        time_access, date_access,
        time_modif,  date_modif,
        size, hsize: INTEGER;
        name: FNAME
    END;


VAR


    Console: BOOLEAN;

    days: DAYS;

    Params: ARRAY MAX_PARAM, 2 OF INTEGER;
    argc*: INTEGER;

    maxreal*, inf*: REAL;


PROCEDURE [stdcall, "Console.obj", "con_init"] con_init (wnd_width, wnd_height, scr_width, scr_height, title: INTEGER);

PROCEDURE [stdcall, "Console.obj", "con_exit"] con_exit (bCloseWindow: BOOLEAN);

PROCEDURE [stdcall, "Console.obj", "con_write_string"] con_write_string (string, length: INTEGER);


PROCEDURE ExitProcess* (p1: INTEGER);
BEGIN
    IF Console THEN
        con_exit(FALSE)
    END;
    K.sysfunc1(-1)
END ExitProcess;


PROCEDURE OutChar* (c: CHAR);
BEGIN
    IF Console THEN
        con_write_string(SYSTEM.ADR(c), 1)
    ELSE
        K.sysfunc3(63, 1, ORD(c))
    END
END OutChar;


PROCEDURE GetFileInfo (FName: ARRAY OF CHAR; VAR Info: rFD): BOOLEAN;
VAR
    res2: INTEGER;
    fs:   rFS;

BEGIN
    fs.subfunc := 5;
    fs.pos := 0;
    fs.hpos := 0;
    fs.bytes := 0;
    fs.buffer := SYSTEM.ADR(Info);
    COPY(FName, fs.name)
    RETURN K.sysfunc22(70, SYSTEM.ADR(fs), res2) = 0
END GetFileInfo;


PROCEDURE Exists (FName: ARRAY OF CHAR): BOOLEAN;
VAR
    fd: rFD;

BEGIN
    RETURN GetFileInfo(FName, fd) & ~(4 IN BITS(fd.attr))
END Exists;


PROCEDURE Close (VAR F: FS);
BEGIN
    IF F # NIL THEN
        DISPOSE(F)
    END
END Close;


PROCEDURE Open (FName: ARRAY OF CHAR): FS;
VAR
    F: FS;

BEGIN
    IF Exists(FName) THEN
        NEW(F);
        IF F # NIL THEN
            F.subfunc := 0;
            F.pos := 0;
            F.hpos := 0;
            F.bytes := 0;
            F.buffer := 0;
            COPY(FName, F.name)
        END
    ELSE
        F := NIL
    END

    RETURN F
END Open;


PROCEDURE Read (F: FS; Buffer, Count: INTEGER): INTEGER;
VAR
    res, res2: INTEGER;

BEGIN
    IF F # NIL THEN
        F.subfunc := 0;
        F.bytes := Count;
        F.buffer := Buffer;
        res := K.sysfunc22(70, SYSTEM.ADR(F^), res2);
        IF res2 > 0 THEN
            F.pos := F.pos + res2
        END
    ELSE
        res2 := 0
    END

    RETURN res2
END Read;


PROCEDURE Write (F: FS; Buffer, Count: INTEGER): INTEGER;
VAR
    res, res2: INTEGER;

BEGIN
    IF F # NIL THEN
        F.subfunc := 3;
        F.bytes := Count;
        F.buffer := Buffer;
        res := K.sysfunc22(70, SYSTEM.ADR(F^), res2);
        IF res2 > 0 THEN
            F.pos := F.pos + res2
        END
    ELSE
        res2 := 0
    END

    RETURN res2
END Write;


PROCEDURE Create (FName: ARRAY OF CHAR): FS;
VAR
    F:    FS;
    res2: INTEGER;

BEGIN
    NEW(F);
    IF F # NIL THEN
        F.subfunc := 2;
        F.pos := 0;
        F.hpos := 0;
        F.bytes := 0;
        F.buffer := 0;
        COPY(FName, F.name);
        IF K.sysfunc22(70, SYSTEM.ADR(F^), res2) # 0 THEN
            DISPOSE(F)
        END
    END

    RETURN F
END Create;


PROCEDURE FileRead* (F: INTEGER; VAR Buffer: ARRAY OF CHAR; bytes: INTEGER): INTEGER;
VAR
    n: INTEGER;
    fs: FS;

BEGIN
    SYSTEM.GET(SYSTEM.ADR(F), fs);
    n := Read(fs, SYSTEM.ADR(Buffer[0]), bytes);
    IF n = 0 THEN
        n := -1
    END

    RETURN n
END FileRead;


PROCEDURE FileWrite* (F: INTEGER; Buffer: ARRAY OF BYTE; bytes: INTEGER): INTEGER;
VAR
    n: INTEGER;
    fs: FS;

BEGIN
    SYSTEM.GET(SYSTEM.ADR(F), fs);
    n := Write(fs, SYSTEM.ADR(Buffer[0]), bytes);
    IF n = 0 THEN
        n := -1
    END

    RETURN n
END FileWrite;


PROCEDURE FileCreate* (FName: ARRAY OF CHAR): INTEGER;
VAR
    fs: FS;
    res: INTEGER;

BEGIN
    fs := Create(FName);
    SYSTEM.GET(SYSTEM.ADR(fs), res)
    RETURN res
END FileCreate;


PROCEDURE FileClose* (F: INTEGER);
VAR
    fs: FS;

BEGIN
    SYSTEM.GET(SYSTEM.ADR(F), fs);
    Close(fs)
END FileClose;


PROCEDURE FileOpen* (FName: ARRAY OF CHAR): INTEGER;
VAR
    fs: FS;
    res: INTEGER;

BEGIN
    fs := Open(FName);
    SYSTEM.GET(SYSTEM.ADR(fs), res)
    RETURN res
END FileOpen;


PROCEDURE chmod* (FName: ARRAY OF CHAR);
END chmod;


PROCEDURE GetTickCount* (): INTEGER;
    RETURN K.sysfunc2(26, 9)
END GetTickCount;


PROCEDURE AppAdr (): INTEGER;
VAR
    buf: ARRAY 1024 OF CHAR;
    a: INTEGER;

BEGIN
    a := K.sysfunc3(9, SYSTEM.ADR(buf), -1);
    SYSTEM.GET(SYSTEM.ADR(buf) + 22, a)
    RETURN a
END AppAdr;


PROCEDURE GetCommandLine (): INTEGER;
VAR
    param: INTEGER;

BEGIN
    SYSTEM.GET(28 + AppAdr(), param)
    RETURN param
END GetCommandLine;


PROCEDURE GetName (): INTEGER;
VAR
    name: INTEGER;

BEGIN
    SYSTEM.GET(32 + AppAdr(), name)
    RETURN name
END GetName;


PROCEDURE GetChar (adr: INTEGER): CHAR;
VAR
    res: CHAR;

BEGIN
    SYSTEM.GET(adr, res)
    RETURN res
END GetChar;


PROCEDURE ParamParse;
VAR
    p, count, name, cond: INTEGER;
    c: CHAR;


    PROCEDURE ChangeCond (A, B, C: INTEGER; c: CHAR; VAR cond: INTEGER);
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
    p := GetCommandLine();
    name := GetName();
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
        |6:
        END;
        INC(p)
    END;
    argc := count
END ParamParse;


PROCEDURE GetArg* (n: INTEGER; VAR s: ARRAY OF CHAR);
VAR
    i, j, len: INTEGER;
    c: CHAR;

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
            INC(i)
        END
    END;
    s[j] := 0X
END GetArg;


PROCEDURE GetCurrentDirectory* (VAR path: ARRAY OF CHAR);
VAR
    n: INTEGER;

BEGIN
    n := K.sysfunc4(30, 2, SYSTEM.ADR(path[0]), LEN(path) - 2);
    path[n - 1] := slash;
    path[n] := 0X
END GetCurrentDirectory;


PROCEDURE isRelative* (path: ARRAY OF CHAR): BOOLEAN;
    RETURN path[0] # slash
END isRelative;


PROCEDURE UnixTime* (): INTEGER;
VAR
    date, time, year, month, day, hour, min, sec: INTEGER;

BEGIN
    date  := K.sysfunc1(29);
    time  := K.sysfunc1(3);

    year  := date MOD 16;
    date  := date DIV 16;
    year  := (date MOD 16) * 10 + year;
    date  := date DIV 16;

    month := date MOD 16;
    date  := date DIV 16;
    month := (date MOD 16) * 10 + month;
    date  := date DIV 16;

    day   := date MOD 16;
    date  := date DIV 16;
    day   := (date MOD 16) * 10 + day;
    date  := date DIV 16;

    hour  := time MOD 16;
    time  := time DIV 16;
    hour  := (time MOD 16) * 10 + hour;
    time  := time DIV 16;

    min   := time MOD 16;
    time  := time DIV 16;
    min   := (time MOD 16) * 10 + min;
    time  := time DIV 16;

    sec   := time MOD 16;
    time  := time DIV 16;
    sec   := (time MOD 16) * 10 + sec;
    time  := time DIV 16;

    INC(year, 2000)

    RETURN ((year - 1970) * 365 + days[month - 1, day - 1, ORD(year DIV 4 = 0)] + (year - 1969) DIV 4) * 86400 + hour * 3600 + min * 60 + sec
END UnixTime;


PROCEDURE splitf* (x: REAL; VAR a, b: INTEGER): INTEGER;
BEGIN
    SYSTEM.GET32(SYSTEM.ADR(x), a);
    SYSTEM.GET32(SYSTEM.ADR(x) + 4, b)
    RETURN a
END splitf;


PROCEDURE d2s* (x: REAL): INTEGER;
VAR
    h, l, s, e: INTEGER;

BEGIN
    e := splitf(x, l, h);

    s := ASR(h, 31) MOD 2;
    e := (h DIV 100000H) MOD 2048;
    IF e <= 896 THEN
        h := (h MOD 100000H) * 8 + (l DIV 20000000H) MOD 8 + 800000H;
        REPEAT
            h := h DIV 2;
            INC(e)
        UNTIL e = 897;
        e := 896;
        l := (h MOD 8) * 20000000H;
        h := h DIV 8
    ELSIF (1151 <= e) & (e < 2047) THEN
        e := 1151;
        h := 0;
        l := 0
    ELSIF e = 2047 THEN
        e := 1151;
        IF (h MOD 100000H # 0) OR (BITS(l) * {0..31} # {}) THEN
            h := 80000H;
            l := 0
        END
    END;
    DEC(e, 896)

    RETURN LSL(s, 31) + LSL(e, 23) + (h MOD 100000H) * 8 + (l DIV 20000000H) MOD 8
END d2s;


PROCEDURE init (VAR days: DAYS);
VAR
    i, j, n0, n1: INTEGER;

BEGIN

    FOR i := 0 TO 11 DO
        FOR j := 0 TO 30 DO
            days[i, j, 0] := 0;
            days[i, j, 1] := 0;
        END
    END;

    days[ 1, 28, 0] := -1;

    FOR i := 0 TO 1 DO
        days[ 1, 29, i] := -1;
        days[ 1, 30, i] := -1;
        days[ 3, 30, i] := -1;
        days[ 5, 30, i] := -1;
        days[ 8, 30, i] := -1;
        days[10, 30, i] := -1;
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
    END;

    inf := SYSTEM.INF();
    maxreal := 1.9;
    PACK(maxreal, 1023);
    Console := TRUE;
    IF Console THEN
        con_init(-1, -1, -1, -1, SYSTEM.SADR("Oberon-07 for KolibriOS"))
    END;
    ParamParse
END init;


BEGIN
    init(days)
END HOST.