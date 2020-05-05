; ------------------------------------------------------------- ;
; KWINE is a fork of program PELoad written by 0CodErr
; author - rgimad
; ------------------------------------------------------------- ;
; standard device (Winbase.h)
%define STD_INPUT_HANDLE     -10
%define STD_OUTPUT_HANDLE    -11
%define STD_ERROR_HANDLE     -12

; starting point for file pointer move (Winbase.h)
%define FILE_BEGIN            0 ; zero or beginning of file
%define FILE_CURRENT          1 ; current value of file pointer
%define FILE_END              2 ; current end-of-file position

; file system operation codes (kernel/trunk/fs/fs_lfn.inc)
%define F70_READ_F            0 ; read file
%define F70_READ_D            1 ; read folder
%define F70_CREATE_F          2 ; create/rewrite file
%define F70_WRITE_F           3 ; write/append to file
%define F70_SETSIZE_F         4 ; set end of file
%define F70_GETATTR_FD        5 ; get file/directory attributes structure
%define F70_SETATTR_FD        6 ; set file/directory attributes structure
%define F70_START_F           7 ; start application
%define F70_DELETE_FD         8 ; delete file
%define F70_CREATE_D          9 ; create directory

; action to take on file that exists or does not exist (Winbase.h)
%define CREATE_NEW            1 ; creates a new file, only if it does not already exist
%define CREATE_ALWAYS         2 ; creates new file, always
%define OPEN_EXISTING         3 ; opens file, only if it exists
%define OPEN_ALWAYS           4 ; opens file, always
%define TRUNCATE_EXISTING     5 ; opens file and truncates it so that its size is zero bytes, only if it exists

%define INVALID_HANDLE_VALUE -1
%define INVALID_FILE_SIZE    -1

GLOBAL EXPORTS
section '.exprt' align 16
;**********************************************************************************
EXPORTS: ;/////////////////////////////////////////////////////////////////////////
;**********************************************************************************
dd sz_ExitProcess,              ExitProcess
dd sz_GetStdHandle,             GetStdHandle
dd sz_SetConsoleMode,           SetConsoleMode
dd sz_WriteFile,                WriteFile
dd sz_ReadFile,                 ReadFile
dd sz_GetCommandLineA,          GetCommandLineA
dd sz_GlobalAlloc,              GlobalAlloc
dd sz_GlobalFree,               GlobalFree
dd sz_GlobalReAlloc,            GlobalReAlloc
dd sz_Sleep,                    Sleep
dd sz_FlushConsoleInputBuffer,  FlushConsoleInputBuffer
dd sz_CloseHandle,              CloseHandle
dd sz_GetFileSize,              GetFileSize
dd sz_CreateFileA,              CreateFileA
dd sz_SetFilePointer,           SetFilePointer
dd sz_VirtualAlloc,             VirtualAlloc
dd sz_VirtualFree,              VirtualFree
dd sz_SetConsoleCursorPosition, SetConsoleCursorPosition
dd sz_DeleteFileA,              DeleteFileA
dd sz_FindClose,                FindClose
dd sz_FindFirstFileA,           FindFirstFileA
dd sz_GetLocalTime,             GetLocalTime
dd sz_GetLastError,             GetLastError
dd sz_GetProcessHeap,           GetProcessHeap
dd sz_HeapAlloc,                HeapAlloc
dd sz_HeapFree,                 HeapFree
dd sz_HeapReAlloc,              HeapReAlloc
dd 0
sz_ExitProcess              db "ExitProcess",0
sz_GetStdHandle             db "GetStdHandle",0
sz_SetConsoleMode           db "SetConsoleMode",0
sz_WriteFile                db "WriteFile",0
sz_ReadFile                 db "ReadFile",0
sz_GetCommandLineA          db "GetCommandLineA",0
sz_GlobalAlloc              db "GlobalAlloc",0
sz_GlobalFree               db "GlobalFree",0
sz_GlobalReAlloc            db "GlobalReAlloc",0
sz_Sleep                    db "Sleep",0
sz_FlushConsoleInputBuffer  db "FlushConsoleInputBuffer",0
sz_CloseHandle              db "CloseHandle",0
sz_GetFileSize              db "GetFileSize",0
sz_CreateFileA              db "CreateFileA",0
sz_SetFilePointer           db "SetFilePointer",0
sz_VirtualAlloc             db "VirtualAlloc",0
sz_VirtualFree              db "VirtualFree",0
sz_SetConsoleCursorPosition db "SetConsoleCursorPosition",0
sz_DeleteFileA              db "DeleteFileA",0
sz_FindClose                db "FindClose",0
sz_FindFirstFileA           db "FindFirstFileA",0
sz_GetLocalTime             db "GetLocalTime",0
sz_GetLastError             db "GetLastError",0
sz_GetProcessHeap           db "GetProcessHeap",0
sz_HeapAlloc                db "HeapAlloc",0
sz_HeapFree                 db "HeapFree",0
sz_HeapReAlloc              db "HeapReAlloc",0








section '.code' align 16
align 16
;**********************************************************************************
ExitProcess: ;/////////////////////////////////////////////////////////////////////
;**********************************************************************************
        xor    eax, eax
        dec    eax
        int    64
        ; ret not need
align 16			
;**********************************************************************************
GetStdHandle: ;////////////////////////////////////////////////////////////////////
;**********************************************************************************
        push   ebx
        push   esi
        push   edi
; if already loaded then do nothing
        cmp    [console], dword 0
        jne    .do_nothing

        push   sz_console
        call   load.library
        mov    [console], eax
        mov    ecx, eax
        mov    ebx, getprocaddress
        push   ecx
        push   sz_con_init
        call   ebx
        mov    [con_init], eax
        push   ecx
        push   sz_con_write_asciiz
        call   ebx
        mov    [con_write_asciiz], eax
        push   ecx
        push   sz_con_exit
        call   ebx
        mov    [con_exit], eax
        push   ecx
        push   sz_con_gets
        call   ebx
        mov    [con_gets], eax
        push   ecx
        push   sz_con_write_string
        call   ebx
        mov    [con_write_string], eax
        push   ecx
        push   sz_con_set_flags
        call   ebx
        mov    [con_set_flags], eax	
        push   ecx
        push   sz_con_set_cursor_pos
        call   ebx
        mov    [con_set_cursor_pos], eax
        push   ecx
        push   sz_con_printf
        call   ebx
        mov    [con_printf], eax
				
				mov    eax, [28]
				cmp    [eax], byte 34 ; quote
				jne    .no_quote
				inc    eax
.no_quote:				
        push   eax
        push   -1
        push   -1
        push   -1
        push   -1
        call   [con_init]
.do_nothing:

        mov    eax, con_handle ; return pointer to console descriptor

        pop    edi
        pop    esi
        pop    ebx
        ret    4
align 16				
;**********************************************************************************
WriteFile: ;///////////////////////////////////////////////////////////////////////
;**********************************************************************************
%define hFile                  [ebp +  8] ; handle to the file
%define lpBuffer               [ebp + 12] ; pointer to buffer containing data
%define nNumberOfBytesToWrite  [ebp + 16] ; number of bytes to be written
%define lpNumberOfBytesWritten [ebp + 20] ; pointer to variable that receives number of bytes written
%define lpOverlapped           [ebp + 24] ; pointer to OVERLAPPED structure
				push   ebp
				mov    ebp, esp
        push   ebx
        push   esi
        push   edi
;---------
        mov    eax, hFile
        cmp    [eax + 8], dword "CON"
        je     .con
        lea    edx, [eax + 8]
        push   edx ;filepath
        dec    esp
        mov    [esp], byte 0
        push   dword lpBuffer ;buffer
        push   dword nNumberOfBytesToWrite;count
        push   dword 0
        push   dword [eax + 4];position ;  in InternalFileInfo in libio
        push   dword F70_WRITE_F
        mov    ebx, esp
        mov    eax, 70
        int    64
        add    esp, 25 ; restore stack
        mov    edx, lpNumberOfBytesWritten
        mov    [edx], ebx
				mov    edx, hFile
				add    [edx + 4], ebx
        jmp    .exit
.con:

        ; push    dword lpBuffer
        ; call    [con_printf]
        ; add     esp, 4

        push   dword nNumberOfBytesToWrite
        push   dword lpBuffer
        call   [con_write_string]

        ; push   dword lpBuffer
        ; call   [con_write_asciiz]

.exit:
;---------
        pop    edi
        pop    esi
        pop    ebx
				pop    ebp
        ret    20
%undef hFile
%undef lpBuffer
%undef nNumberOfBytesToWrite
%undef lpNumberOfBytesWritten
%undef lpOverlapped
align 16
;**********************************************************************************
ReadFile: ;////////////////////////////////////////////////////////////////////////
;**********************************************************************************
%define hFile                  [ebp +  8] ; handle to the file
%define lpBuffer               [ebp + 12] ; pointer to buffer that receives data
%define nNumberOfBytesToRead   [ebp + 16] ; maximum number of bytes to read
%define lpNumberOfBytesRead    [ebp + 20] ; pointer to variable that receives number of bytes read
%define lpOverlapped           [ebp + 24] ; pointer to OVERLAPPED structure
				push   ebp
				mov    ebp, esp
        push   ebx
        push   esi
        push   edi
				
        ; push   dword 0
				; call   GetStdHandle				
				
;---------
        mov    eax, hFile
        cmp    [eax + 8], dword "CON"
        je     .con
				

				
				; lea    eax, [eax + 8]
        ; push   eax
        ; call   [con_write_asciiz]		
				
        lea    edx, [eax + 8]
        push   edx ;filepath
        dec    esp
        mov    [esp], byte 0
        push   dword lpBuffer ;buffer
        push   dword nNumberOfBytesToRead;count
        push   dword 0
        push   dword [eax + 4];position ;  in InternalFileInfo in libio
        push   dword F70_READ_F
        mov    ebx, esp
        mov    eax, 70
        int    64
        add    esp, 25 ; restore stack
        mov    edx, lpNumberOfBytesRead
        mov    [edx], ebx
				mov    edx, hFile
				add    [edx + 4], ebx
        jmp    .exit
.con:
        push   dword nNumberOfBytesToRead
        push   dword lpBuffer
        call   [con_gets]
.exit:
;---------
        pop    edi
        pop    esi
        pop    ebx
        pop    ebp				
        ret    20
%undef hFile
%undef lpBuffer
%undef nNumberOfBytesToRead
%undef lpNumberOfBytesRead
%undef lpOverlapped
align 16
;**********************************************************************************
SetConsoleMode: ;//////////////////////////////////////////////////////////////////
;**********************************************************************************
; ignore input parameters
        xor    eax, eax
				dec    eax
        ret    8
align 16				
;**********************************************************************************
GetCommandLineA: ;/////////////////////////////////////////////////////////////////
;**********************************************************************************
        push   edi
				mov    edi, [28]
				xor    al, al
				xor    ecx, ecx
				dec    ecx
				repne scasb
				mov    eax, edi
				pop    edi
        ret
align 16				
;**********************************************************************************
GlobalAlloc: ;/////////////////////////////////////////////////////////////////////
;**********************************************************************************
%define uFlags                [esp +  4 +1*4] ; memory allocation attributes
%define dwBytes               [esp +  8 +1*4] ; number of bytes to allocate
        push   ebx
; uFlags ignored
        mov    eax, 68
        mov    ebx, 12
        mov    ecx, dwBytes
        int    64

        pop    ebx
        ret    8
%undef uFlags
%undef dwBytes
align 16
;**********************************************************************************
GlobalFree: ;//////////////////////////////////////////////////////////////////////
;**********************************************************************************
%define hMem                  [esp +  4 +1*4] ; handle to global memory object
        push   ebx

        mov    eax, 68
        mov    ebx, 13
        mov    ecx, hMem
        int    64

        pop    ebx
        ret    4
%undef hMem
align 16
;**********************************************************************************
GlobalReAlloc: ;///////////////////////////////////////////////////////////////////
;**********************************************************************************
%define hMem                  [esp +  4 +1*4] ; handle to global memory object
%define dwBytes               [esp +  8 +1*4] ; new size of memory block in bytes
%define uFlags                [esp + 12 +1*4] ; reallocation options
        push   ebx
; uFlags ignored
        mov    eax, 68
        mov    ebx, 20
        mov    ecx, dwBytes
        mov    edx, hMem
        int    64

        pop    ebx
        ret    12
%undef hMem
%undef dwBytes
%undef uFlags
align 16
;**********************************************************************************
Sleep: ;///////////////////////////////////////////////////////////////////////////
;**********************************************************************************
%define dwMilliseconds        [esp +  4 +1*4] ; time interval
        push   ebx

        mov    eax, dwMilliseconds
        mov    ebx, 10
				cmp    eax, ebx
				jae    .ae
				add    eax, 10 ; avoid zero result if dwMilliseconds < 10
.ae:				
        xor    edx, edx
        div    ebx
        mov    ebx, eax

        mov    eax, 5
        int    64

        pop    ebx
        ret    4
%undef dwMilliseconds
align 16
;**********************************************************************************
FlushConsoleInputBuffer: ;/////////////////////////////////////////////////////////
;**********************************************************************************
; not implemented correctly
        xor    eax, eax
        dec    eax
        ret    4
align 16				
;**********************************************************************************
CloseHandle: ;/////////////////////////////////////////////////////////////////////
;**********************************************************************************
%define hObject               [esp +  4 +1*4]
        push   ebx
        mov    eax, 68
        mov    ebx, 13
        mov    ecx, hObject
        int    64
				pop    ebx
        ret    4
%undef hObject
align 16
;**********************************************************************************
GetFileSize: ;/////////////////////////////////////////////////////////////////////
;**********************************************************************************
%define hFile                 [esp +  4 +3*4]
%define lpFileSizeHigh        [esp +  8 +3*4]
        push   ebx
        push   esi
        push   edi
; lpFileSizeHigh ignored
        mov    [esp - (25 + 40) + 0], dword F70_GETATTR_FD
        mov    [esp - (25 + 40) + 8], dword 0
        mov    [esp - (25 + 40) + 20], byte 0
        lea    eax, [esp - 40]
        mov    [esp - (25 + 40) + 16], eax
        lea    ebx, [esp - (25 + 40)]
        mov    eax, hFile
        lea    eax, [eax + 8] ; as in InternalFileInfo in libio
        mov    [esp - (25 + 40) + 21], eax
        mov    eax, 70
        int    64
        test   eax, eax
        jz     .no_error
        mov    eax, INVALID_FILE_SIZE
        jmp    .exit
.no_error:
        mov    eax, [esp - (25 + 40) + 25 + 32] ; file.size
.exit:
        pop    edi
        pop    esi
        pop    ebx
        ret    8
%undef hFile
%undef lpFileSizeHigh
align 16
;**********************************************************************************
CreateFileA: ;//////////////////////////////////////////////////////////////////////
;**********************************************************************************
%define lpFileName            [esp +  4 +3*4]
%define dwDesiredAccess       [esp +  8 +3*4]
%define dwShareMode           [esp + 12 +3*4]
%define lpSecurityAttributes  [esp + 16 +3*4]
%define dwCreationDisposition [esp + 20 +3*4]
%define dwFlagsAndAttributes  [esp + 24 +3*4]
%define hTemplateFile         [esp + 28 +3*4]
        push   ebx
        push   esi
        push   edi
;---------

        ; push   dword 0
				; call   GetStdHandle
				
        ; push   dword lpFileName
        ; call   [con_write_asciiz]		



        mov    eax, 68
        mov    ebx, 12
        mov    ecx, 4096
        int    64

        mov    edx, eax

        lea     edi, [eax + 8] ; as in InternalFileInfo in libio
        mov     esi, lpFileName
.copy_name:
        lodsb
        stosb
        test   al, al
        jnz    .copy_name
        
				mov    eax, dwCreationDisposition
        cmp    eax, CREATE_ALWAYS
				je     .create_always
        cmp    eax, OPEN_EXISTING
				je     .open_existing
				mov    eax, INVALID_HANDLE_VALUE
				jmp    .exit
.open_existing:
        lea    eax, [edx + 8]
        push   eax ;filepath
        dec    esp
        mov    [esp], byte 0
        push   dword 0 ; buffer
        push   dword 0 ; count
        push   dword 0
        push   dword 0
        push   dword F70_READ_F
        mov    ebx, esp
        mov    eax, 70
        int    64
        add    esp, 25 ; restore stack
				test   eax, eax
				jz     .no_error
				mov    eax, INVALID_HANDLE_VALUE
				jmp    .exit
.no_error:				
        mov    eax, edx ; return pointer to file descriptor
        jmp    .exit
.create_always:
        lea    eax, [edx + 8]
        push   eax ;filepath
        dec    esp
        mov    [esp], byte 0
        push   dword 0 ; buffer
        push   dword 0 ; count
        push   dword 0
        push   dword 0
        push   dword F70_CREATE_F
        mov    ebx, esp
        mov    eax, 70
        int    64
        add    esp, 25 ; restore stack
        mov    eax, edx ; return pointer to file descriptor
				jmp    .exit
				
.exit:				
;---------
        pop    edi
        pop    esi
        pop    ebx
        ret    28
%undef lpFileName
%undef dwDesiredAccess
%undef dwShareMode
%undef lpSecurityAttributes
%undef dwCreationDisposition
%undef dwFlagsAndAttributes
%undef hTemplateFile
align 16
;**********************************************************************************
SetFilePointer: ;//////////////////////////////////////////////////////////////////
;**********************************************************************************
%define hFile                 [esp +  4 +3*4]
%define lDistanceToMove       [esp +  8 +3*4]
%define lpDistanceToMoveHigh  [esp + 12 +3*4]
%define dwMoveMethod          [esp + 16 +3*4]
        push   ebx
        push   esi
        push   edi
;---------
        mov    eax, hFile
				cmp    dwMoveMethod, dword FILE_BEGIN
				je     .FILE_BEGIN
				cmp    dwMoveMethod, dword FILE_CURRENT
				je     .FILE_CURRENT
				jmp    .FILE_END
.FILE_BEGIN:
				mov    edx, lDistanceToMove
				mov    [eax + 4], edx
				jmp    .exit
.FILE_CURRENT:
				mov    edx, lDistanceToMove
				add    [eax + 4], edx
				jmp    .exit
.FILE_END:
				push   dword 0
				push   eax
				call   GetFileSize
				mov    edx, eax
				
				mov    eax, hFile
				mov    [eax + 4], edx
				mov    edx, lDistanceToMove
				add    [eax + 4], edx
.exit:
        mov    eax, [eax + 4]
;---------
        pop    edi
        pop    esi
        pop    ebx
        ret    16
%undef hFile
%undef lDistanceToMove
%undef lpDistanceToMoveHigh
%undef dwMoveMethod
align 16				
;**********************************************************************************
VirtualAlloc: ;////////////////////////////////////////////////////////////////////
;**********************************************************************************
%define lpAddress             [esp +  4 +1*4]
%define dwSize                [esp +  8 +1*4]
%define flAllocationType      [esp + 12 +1*4]
%define flProtect             [esp + 16 +1*4]
        push   ebx

        mov    eax, 68
        mov    ebx, 12
        mov    ecx, dwSize
        int    64

        pop    ebx
        ret    16
%undef lpAddress
%undef dwSize
%undef flAllocationType
%undef flProtect
align 16
;**********************************************************************************
VirtualFree: ;/////////////////////////////////////////////////////////////////////
;**********************************************************************************
%define lpAddress             [esp +  4 +1*4]
%define dwSize                [esp +  8 +1*4]
%define dwFreeType            [esp + 12 +1*4]
        push   ebx

        mov    eax, 68
        mov    ebx, 13
        mov    ecx, lpAddress
        int    64

        pop    ebx
        ret    12
%undef lpAddress
%undef dwSize
%undef dwFreeType
align 16
;**********************************************************************************
SetConsoleCursorPosition: ;////////////////////////////////////////////////////////
;**********************************************************************************
%define hConsoleOutput        [esp +  4 +3*4]
%define dwCursorPosition      [esp +  8 +3*4]
        push   ebx
        push   esi
        push   edi
        
				mov    edx, dwCursorPosition
				shld   eax, edx, 16
				shr    edx, 16
				
				push   eax
				push   edx
        call   [con_set_cursor_pos]

        pop    edi
        pop    esi
        pop    ebx
        ret    8
%undef hConsoleOutput
%undef dwCursorPosition
align 16
;**********************************************************************************
DeleteFileA: ;/////////////////////////////////////////////////////////////////////
;**********************************************************************************
%define lpFileName            [esp +  4 +1*4] ; name of file 
        push   ebx
				
        mov    [esp - (25 + 40) + 0], dword F70_DELETE_FD
        mov    [esp - (25 + 40) + 8], dword 0
        mov    [esp - (25 + 40) + 20], byte 0
        lea    eax, [esp - 40]
        mov    [esp - (25 + 40) + 16], eax
        lea    ebx, [esp - (25 + 40)]
        mov    eax, lpFileName
        mov    [esp - (25 + 40) + 21], eax
        mov    eax, 70
        int    64
				
				pop    ebx
        ret    4
%undef lpFileName
align 16				
;**********************************************************************************
FindClose: ;///////////////////////////////////////////////////////////////////////
;**********************************************************************************
%define hFindFile             [esp +  4 +1*4] ; file search handle

        ret    4
%undef hFindFile
align 16
;**********************************************************************************
FindFirstFileA: ;//////////////////////////////////////////////////////////////////
;**********************************************************************************
%define lpFileName            [esp +  4 +3*4] ; name of file 
%define lpFindFileData        [esp +  8 +3*4] ; pointer to WIN32_FIND_DATA structure
        push   ebx
        push   esi
        push   edi

        pop    edi
        pop    esi
        pop    ebx
        ret    8
%undef lpFileName
%undef lpFindFileData
align 16				
;**********************************************************************************
GetLocalTime: ;///////////////////////////////////////////////////////////////////////
;**********************************************************************************
%define lpSystemTime             [esp +  4] ; pointer to SYSTEMTIME structure 
; yet not implemented
        ; mov    eax, lpSystemTime
				; mov    [eax + 0],  dword 12345678H
				; mov    [eax + 4],  dword 12345678H
				; mov    [eax + 8],  dword 12345678H
				; mov    [eax + 12], dword 12345678H				
        ; MSDN: This function does not return a value.
        ret    4
%undef lpSystemTime
align 16				
;**********************************************************************************
GetLastError: ;////////////////////////////////////////////////////////////////////
;**********************************************************************************
        xor    eax, eax
        ret
align 16				
;**********************************************************************************
GetProcessHeap: ;////////////////////////////////////////////////////////////////////
;**********************************************************************************
        xor    eax, eax
				dec    eax
        ret
align 16				
;**********************************************************************************
HeapAlloc: ;///////////////////////////////////////////////////////////////////////
;**********************************************************************************
%define hHeap                 [esp +  4 +1*4]
%define dwFlags               [esp +  8 +1*4]
%define dwBytes               [esp + 12 +1*4]
        push   ebx

        mov    eax, 68
        mov    ebx, 12
        mov    ecx, dwBytes
        int    64

        pop    ebx
        ret    12
%undef hHeap				
%undef dwFlags
%undef dwBytes
align 16
;**********************************************************************************
HeapFree: ;////////////////////////////////////////////////////////////////////////
;**********************************************************************************
%define hHeap                 [esp +  4 +1*4]
%define dwFlags               [esp +  8 +1*4]
%define lpMem                 [esp + 12 +1*4]
        push   ebx

        mov    eax, 68
        mov    ebx, 13
        mov    ecx, lpMem
        int    64

        pop    ebx
        ret    12
%undef hHeap				
%undef dwFlags
%undef lpMem
align 16
;**********************************************************************************
HeapReAlloc: ;/////////////////////////////////////////////////////////////////////
;**********************************************************************************
%define hHeap                 [esp +  4 +1*4]
%define dwFlags               [esp +  8 +1*4]
%define lpMem                 [esp + 12 +1*4]
%define dwBytes               [esp + 16 +1*4]
        push   ebx

        mov    eax, 68
        mov    ebx, 20
        mov    ecx, dwBytes
        mov    edx, lpMem
        int    64

        pop    ebx
        ret    16
%undef hHeap
%undef dwFlags
%undef lpMem
%undef dwBytes






; ------------------------------------------------------------- ;
load.library:
        mov    eax, 68
        mov    ebx, 19
        mov    ecx, [esp + 4]
        int    64
        ret    4
; ------------------------------------------------------------- ;
getprocaddress:
        mov    edx, [esp + 8]
        xor    eax, eax
        test   edx, edx
        jz     .end
.next:
        xor    eax, eax
        cmp    [edx], dword 0
        jz     .end
        mov    esi, [edx]
        mov    edi, [esp + 4]
.next_:
        lodsb
        scasb
        jne    .fail
        or     al, al
        jnz    .next_
        jmp    .ok
.fail:
        add    edx, 8
        jmp    .next
.ok:
        mov    eax, [edx + 4]
.end:
        ret    8
; ------------------------------------------------------------- ;





section '.data' align 16

con_init              dd 0
con_write_asciiz      dd 0
con_exit              dd 0
con_gets              dd 0
con_write_string      dd 0
con_set_flags         dd 0
con_set_cursor_pos    dd 0
con_printf            dd 0
console               dd 0
sz_con_init           db "con_init",0
sz_con_write_asciiz   db "con_write_asciiz",0
sz_con_exit           db "con_exit",0
sz_con_gets           db "con_gets",0
sz_con_write_string   db "con_write_string",0
sz_console            db "/sys/lib/console.obj",0
sz_con_set_flags      db "con_set_flags",0
sz_con_set_cursor_pos db "con_set_cursor_pos",0
sz_con_printf         db "con_printf",0

con_handle:
dd 0
dd 0
dd "CON"


