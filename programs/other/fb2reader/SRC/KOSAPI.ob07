(*
    BSD 2-Clause License

    Copyright (c) 2018-2019, 2022 Anton Krotov
    All rights reserved.
*)

MODULE KOSAPI;

IMPORT SYSTEM;


TYPE

    STRING = ARRAY 1024 OF CHAR;


VAR

    DLL_INIT: PROCEDURE [stdcall] (entry: INTEGER);


PROCEDURE [stdcall-] sysfunc1* (arg1: INTEGER): INTEGER;
BEGIN
    SYSTEM.CODE(
    08BH, 045H, 008H,   (*  mov     eax, dword [ebp + 8]   *)
    0CDH, 040H,         (*  int     64                     *)
    0C9H,               (*  leave                          *)
    0C2H, 004H, 000H    (*  ret     4                      *)
    )
    RETURN 0
END sysfunc1;


PROCEDURE [stdcall-] sysfunc2* (arg1, arg2: INTEGER): INTEGER;
BEGIN
    SYSTEM.CODE(
    053H,               (*  push    ebx                    *)
    08BH, 045H, 008H,   (*  mov     eax, dword [ebp +  8]  *)
    08BH, 05DH, 00CH,   (*  mov     ebx, dword [ebp + 12]  *)
    0CDH, 040H,         (*  int     64                     *)
    05BH,               (*  pop     ebx                    *)
    0C9H,               (*  leave                          *)
    0C2H, 008H, 000H    (*  ret     8                      *)
    )
    RETURN 0
END sysfunc2;


PROCEDURE [stdcall-] sysfunc3* (arg1, arg2, arg3: INTEGER): INTEGER;
BEGIN
    SYSTEM.CODE(
    053H,               (*  push    ebx                    *)
    08BH, 045H, 008H,   (*  mov     eax, dword [ebp +  8]  *)
    08BH, 05DH, 00CH,   (*  mov     ebx, dword [ebp + 12]  *)
    08BH, 04DH, 010H,   (*  mov     ecx, dword [ebp + 16]  *)
    0CDH, 040H,         (*  int     64                     *)
    05BH,               (*  pop     ebx                    *)
    0C9H,               (*  leave                          *)
    0C2H, 00CH, 000H    (*  ret     12                     *)
    )
    RETURN 0
END sysfunc3;


PROCEDURE [stdcall-] sysfunc4* (arg1, arg2, arg3, arg4: INTEGER): INTEGER;
BEGIN
    SYSTEM.CODE(
    053H,               (*  push    ebx                    *)
    08BH, 045H, 008H,   (*  mov     eax, dword [ebp +  8]  *)
    08BH, 05DH, 00CH,   (*  mov     ebx, dword [ebp + 12]  *)
    08BH, 04DH, 010H,   (*  mov     ecx, dword [ebp + 16]  *)
    08BH, 055H, 014H,   (*  mov     edx, dword [ebp + 20]  *)
    0CDH, 040H,         (*  int     64                     *)
    05BH,               (*  pop     ebx                    *)
    0C9H,               (*  leave                          *)
    0C2H, 010H, 000H    (*  ret     16                     *)
    )
    RETURN 0
END sysfunc4;


PROCEDURE [stdcall-] sysfunc5* (arg1, arg2, arg3, arg4, arg5: INTEGER): INTEGER;
BEGIN
    SYSTEM.CODE(
    053H,               (*  push    ebx                    *)
    056H,               (*  push    esi                    *)
    08BH, 045H, 008H,   (*  mov     eax, dword [ebp +  8]  *)
    08BH, 05DH, 00CH,   (*  mov     ebx, dword [ebp + 12]  *)
    08BH, 04DH, 010H,   (*  mov     ecx, dword [ebp + 16]  *)
    08BH, 055H, 014H,   (*  mov     edx, dword [ebp + 20]  *)
    08BH, 075H, 018H,   (*  mov     esi, dword [ebp + 24]  *)
    0CDH, 040H,         (*  int     64                     *)
    05EH,               (*  pop     esi                    *)
    05BH,               (*  pop     ebx                    *)
    0C9H,               (*  leave                          *)
    0C2H, 014H, 000H    (*  ret     20                     *)
    )
    RETURN 0
END sysfunc5;


PROCEDURE [stdcall-] sysfunc6* (arg1, arg2, arg3, arg4, arg5, arg6: INTEGER): INTEGER;
BEGIN
    SYSTEM.CODE(
    053H,               (*  push    ebx                    *)
    056H,               (*  push    esi                    *)
    057H,               (*  push    edi                    *)
    08BH, 045H, 008H,   (*  mov     eax, dword [ebp +  8]  *)
    08BH, 05DH, 00CH,   (*  mov     ebx, dword [ebp + 12]  *)
    08BH, 04DH, 010H,   (*  mov     ecx, dword [ebp + 16]  *)
    08BH, 055H, 014H,   (*  mov     edx, dword [ebp + 20]  *)
    08BH, 075H, 018H,   (*  mov     esi, dword [ebp + 24]  *)
    08BH, 07DH, 01CH,   (*  mov     edi, dword [ebp + 28]  *)
    0CDH, 040H,         (*  int     64                     *)
    05FH,               (*  pop     edi                    *)
    05EH,               (*  pop     esi                    *)
    05BH,               (*  pop     ebx                    *)
    0C9H,               (*  leave                          *)
    0C2H, 018H, 000H    (*  ret     24                     *)
    )
    RETURN 0
END sysfunc6;


PROCEDURE [stdcall-] sysfunc7* (arg1, arg2, arg3, arg4, arg5, arg6, arg7: INTEGER): INTEGER;
BEGIN
    SYSTEM.CODE(
    053H,               (*  push    ebx                    *)
    056H,               (*  push    esi                    *)
    057H,               (*  push    edi                    *)
    055H,               (*  push    ebp                    *)
    08BH, 045H, 008H,   (*  mov     eax, dword [ebp +  8]  *)
    08BH, 05DH, 00CH,   (*  mov     ebx, dword [ebp + 12]  *)
    08BH, 04DH, 010H,   (*  mov     ecx, dword [ebp + 16]  *)
    08BH, 055H, 014H,   (*  mov     edx, dword [ebp + 20]  *)
    08BH, 075H, 018H,   (*  mov     esi, dword [ebp + 24]  *)
    08BH, 07DH, 01CH,   (*  mov     edi, dword [ebp + 28]  *)
    08BH, 06DH, 020H,   (*  mov     ebp, dword [ebp + 32]  *)
    0CDH, 040H,         (*  int     64                     *)
    05DH,               (*  pop     ebp                    *)
    05FH,               (*  pop     edi                    *)
    05EH,               (*  pop     esi                    *)
    05BH,               (*  pop     ebx                    *)
    0C9H,               (*  leave                          *)
    0C2H, 01CH, 000H    (*  ret     28                     *)
    )
    RETURN 0
END sysfunc7;


PROCEDURE [stdcall-] sysfunc22* (arg1, arg2: INTEGER; VAR res2: INTEGER): INTEGER;
BEGIN
    SYSTEM.CODE(
    053H,               (*  push    ebx                    *)
    08BH, 045H, 008H,   (*  mov     eax, dword [ebp +  8]  *)
    08BH, 05DH, 00CH,   (*  mov     ebx, dword [ebp + 12]  *)
    0CDH, 040H,         (*  int     64                     *)
    08BH, 04DH, 010H,   (*  mov     ecx, dword [ebp + 16]  *)
    089H, 019H,         (*  mov     dword [ecx], ebx       *)
    05BH,               (*  pop     ebx                    *)
    0C9H,               (*  leave                          *)
    0C2H, 00CH, 000H    (*  ret     12                     *)
    )
    RETURN 0
END sysfunc22;


PROCEDURE mem_commit (adr, size: INTEGER);
VAR
    tmp: INTEGER;
BEGIN
    FOR tmp := adr TO adr + size - 1 BY 4096 DO
        SYSTEM.PUT(tmp, 0)
    END
END mem_commit;


PROCEDURE [stdcall] malloc* (size: INTEGER): INTEGER;
VAR
    ptr: INTEGER;
BEGIN
    SYSTEM.CODE(060H); (* pusha *)
    IF sysfunc2(18, 16) > ASR(size, 10) THEN
        ptr := sysfunc3(68, 12, size);
        IF ptr # 0 THEN
            mem_commit(ptr, size)
        END
    ELSE
        ptr := 0
    END;
    SYSTEM.CODE(061H)  (* popa  *)
    RETURN ptr
END malloc;


PROCEDURE [stdcall] free* (ptr: INTEGER): INTEGER;
BEGIN
    SYSTEM.CODE(060H); (* pusha *)
    IF ptr # 0 THEN
        ptr := sysfunc3(68, 13, ptr)
    END;
    SYSTEM.CODE(061H)  (* popa  *)
    RETURN 0
END free;


PROCEDURE [stdcall] realloc* (ptr, size: INTEGER): INTEGER;
BEGIN
    SYSTEM.CODE(060H); (* pusha *)
    ptr := sysfunc4(68, 20, size, ptr);
    SYSTEM.CODE(061H)  (* popa  *)
    RETURN ptr
END realloc;


PROCEDURE AppAdr (): INTEGER;
VAR
    buf: ARRAY 1024 OF CHAR;
    a: INTEGER;
BEGIN
    a := sysfunc3(9, SYSTEM.ADR(buf), -1);
    SYSTEM.GET(SYSTEM.ADR(buf) + 22, a)
    RETURN a
END AppAdr;


PROCEDURE GetCommandLine* (): INTEGER;
VAR
    param: INTEGER;
BEGIN
    SYSTEM.GET(28 + AppAdr(), param)
    RETURN param
END GetCommandLine;


PROCEDURE GetName* (): INTEGER;
VAR
    name: INTEGER;
BEGIN
    SYSTEM.GET(32 + AppAdr(), name)
    RETURN name
END GetName;


PROCEDURE [stdcall] dll_init2 (arg1, arg2, arg3, arg4, arg5: INTEGER);
BEGIN
    SYSTEM.CODE(
    060H,               (*  pusha                          *)
    08BH, 045H, 008H,   (*  mov     eax, dword [ebp +  8]  *)
    08BH, 05DH, 00CH,   (*  mov     ebx, dword [ebp + 12]  *)
    08BH, 04DH, 010H,   (*  mov     ecx, dword [ebp + 16]  *)
    08BH, 055H, 014H,   (*  mov     edx, dword [ebp + 20]  *)
    08BH, 075H, 018H,   (*  mov     esi, dword [ebp + 24]  *)
    0FFH, 0D6H,         (*  call    esi                    *)
    061H,               (*  popa                           *)
    0C9H,               (*  leave                          *)
    0C2H, 014H, 000H    (*  ret     20                     *)
    )
END dll_init2;


PROCEDURE GetProcAdr* (name: ARRAY OF CHAR; lib: INTEGER): INTEGER;
VAR
    cur, procname, adr: INTEGER;

    PROCEDURE streq (str1, str2: INTEGER): BOOLEAN;
    VAR
        c1, c2: CHAR;
    BEGIN
        REPEAT
            SYSTEM.GET(str1, c1);
            SYSTEM.GET(str2, c2);
            INC(str1);
            INC(str2)
        UNTIL (c1 # c2) OR (c1 = 0X)

        RETURN c1 = c2
    END streq;

BEGIN
    adr := 0;
    IF (lib # 0) & (name # "") THEN
        cur := lib;
        REPEAT
            SYSTEM.GET(cur, procname);
            INC(cur, 8)
        UNTIL (procname = 0) OR streq(procname, SYSTEM.ADR(name[0]));
        IF procname # 0 THEN
            SYSTEM.GET(cur - 4, adr)
        END
    END

    RETURN adr
END GetProcAdr;


PROCEDURE init (dll: INTEGER);
VAR
    lib_init: INTEGER;
BEGIN
    lib_init := GetProcAdr("lib_init", dll);
    IF lib_init # 0 THEN
        DLL_INIT(lib_init)
    END;
    lib_init := GetProcAdr("START", dll);
    IF lib_init # 0 THEN
        DLL_INIT(lib_init)
    END
END init;


PROCEDURE OutChar* (c: CHAR);
BEGIN
    sysfunc3(63, 1, ORD(c))
END OutChar;


PROCEDURE OutLn*;
BEGIN
    OutChar(0DX);
    OutChar(0AX)
END OutLn;


PROCEDURE OutString (s: ARRAY OF CHAR);
VAR
    i: INTEGER;
BEGIN
    i := 0;
    WHILE (i < LEN(s)) & (s[i] # 0X) DO
        OutChar(s[i]);
        INC(i)
    END
END OutString;


PROCEDURE imp_error (lib, proc: STRING);
BEGIN
    OutString("import error: ");
    IF proc = "" THEN
        OutString("can't load '")
    ELSE
        OutString("not found '"); OutString(proc); OutString("' in '")
    END;
    OutString(lib);
    OutString("'" + 0DX + 0AX)
END imp_error;


PROCEDURE GetStr (adr, i: INTEGER; VAR str: STRING);
VAR
    c: CHAR;
BEGIN
    REPEAT
        SYSTEM.GET(adr, c); INC(adr);
        str[i] := c; INC(i)
    UNTIL c = 0X
END GetStr;


PROCEDURE [stdcall-] dll_Load* (import_table: INTEGER): INTEGER;
CONST
	path = "/sys/lib/";
VAR
    imp, lib, exp, proc, pathLen: INTEGER;
    procname, libname: STRING;
BEGIN
    SYSTEM.CODE(060H); (* pusha *)
    libname := path;
    pathLen := LENGTH(libname);

    SYSTEM.GET(import_table, imp);
    WHILE imp # 0 DO
        SYSTEM.GET(import_table + 4, lib);
        GetStr(lib, pathLen, libname);
        exp := sysfunc3(68, 19, SYSTEM.ADR(libname[0]));
        IF exp = 0 THEN
            imp_error(libname, "")
        ELSE
            REPEAT
                SYSTEM.GET(imp, proc);
                IF proc # 0 THEN
                    GetStr(proc, 0, procname);
                    proc := GetProcAdr(procname, exp);
                    IF proc # 0 THEN
                        SYSTEM.PUT(imp, proc)
                    ELSE
                    	proc := 1;
                        imp_error(libname, procname)
                    END;
                    INC(imp, 4)
                END
            UNTIL proc = 0;
            init(exp)
        END;
        INC(import_table, 8);
        SYSTEM.GET(import_table, imp);
    END;

    SYSTEM.CODE(061H) (* popa *)
    RETURN 0
END dll_Load;


PROCEDURE [stdcall] dll_Init (entry: INTEGER);
BEGIN
    SYSTEM.CODE(060H); (* pusha *)
    IF entry # 0 THEN
        dll_init2(SYSTEM.ADR(malloc), SYSTEM.ADR(free), SYSTEM.ADR(realloc), SYSTEM.ADR(dll_Load), entry)
    END;
    SYSTEM.CODE(061H); (* popa  *)
END dll_Init;


PROCEDURE LoadLib* (name: ARRAY OF CHAR): INTEGER;
VAR
    Lib: INTEGER;
BEGIN
    DLL_INIT := dll_Init;
    Lib := sysfunc3(68, 19, SYSTEM.ADR(name[0]));
    IF Lib # 0 THEN
        init(Lib)
    END
    RETURN Lib
END LoadLib;


PROCEDURE _init* (import_table: INTEGER);
BEGIN
    DLL_INIT := dll_Init;
    dll_Load(import_table)
END _init;


END KOSAPI.