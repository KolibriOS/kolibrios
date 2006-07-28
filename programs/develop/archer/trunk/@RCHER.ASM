; @RCHER - DEflate unpacker v1.0
;
; Written in pure assembler by Ivushkin Andrey aka Willow
;
; Created: May 6, 2005
;
; Last changed: July 14, 2005
;
; Compile with FASM


SYS equ meos
PARAM_PTR equ png_image
DUMPFILE equ '/hd/1/out.txt'

SHOW_RBLOCK equ 0;1
SHOW_PNG_SEC equ 0;1
SHOW_METH equ 0;1
FILE_NUM equ 0
MEMORY equ 0x800000

BITS equ 16
BUFSIZE equ 32
BUFPTR equ area

PASSW_LEN equ 32
NO_STOPS equ 1
SHOW_CHARS equ 0
BSIZE equ 512
INBUF equ BUFSIZE*BSIZE*2
OUTBUF equ 4*1024*1024
IGNORE_DIRS equ 0

MOVE_SLINE_LEV equ 8

DECRYPT_MODE equ 100b
PNG_MODE equ 10b
TAR_MODE equ 1000b
STAY_MODE equ 10000b
IPC_MODE equ 100000b
RAW_MODE equ 1000000b
THREAD_YES equ 10000000b

if SYS eq win
format PE console
entry start
;match =win,SYS
;{
;    include 'D:\Ivushkin\projects\fasmw\INCLUDE\win32a.inc'
;}
else
use32

  org    0x0

  db     'MENUET01'
  dd     0x01
  dd     start
  dd     I_END
  dd     MEMORY
  dd     MEMORY-2048
if  PARAM_PTR eq param
  dd     0
else
  dd     PARAM_PTR
end if
  dd     0x0
include "lang.inc"

if  PARAM_PTR eq param
  param db 'RQ'
  db '000037'
  db '/hd/1/zip/png.zip',0
end if
;match =meos,SYS
;{
    include "macros.inc"
;    purge mov
    include "debug.inc"
;}
end if

language equ en

if SYS eq win
section '.text' code readable executable writeable
end if
include "arcmacro.inc"
include "parser.inc"
include "deflate.inc"
include "sys.inc"

FileNum dd FILE_NUM
;FNAME equ '/hd/1/zip/pngsuite/s01n3p01.png'

start:
if SYS eq win
    invoke GetStdHandle, STD_OUTPUT_HANDLE
    mov  [cons],eax
    invoke GetStdHandle, STD_INPUT_HANDLE
    mov  [cons_in],eax
    Msg  0
    Msg  1
else
    Msg  0
    Msg  1
    mcall 40,10000101b
;    jmp again
    CmdLine
 red:
    call draw_window
    mcall 12,2

 still:
    mcall 10
    cmp  eax,1
    je   red
    cmp  eax,2
    jne  .nokey
    mcall 2
    cmp  ah,9
    jz   quit
    jmp  still
  .nokey:
    cmp  eax,3
    jne  still
    mcall 17
    cmp  ah,1
    jne  still
    mcall 23,30
    cmp  eax,3
    jne  .noquit
    call KillViewer
    mcall -1
  .noquit:
    mcall 17
    QueryFile
    and  [FileNum],0
    test eax,eax
    jnz  still
end if
again:
    Newline
    xor  eax,eax
;    and  [Flags],STAY_MODE
    and  [CRC32],eax
    and  [IDATsize],eax
    mov  [Adler32],1
    call OpenFile
    test ebx,ebx
    jnz   .sizeok2
  .sizebad2:
    Msg  14
    jmp  quit
  .sizeok2:
    mov  [filesize],ebx
    test [Flags],RAW_MODE
    jz   .norawm
    mov  eax,[arc_base]
    xor  ebx,ebx
    call FileSeek
    jmp  .sizeok
  .norawm:
    call SfxParse

    call ZipParse
    cmp  edx,52
    je   .notzip
    mov  ecx,[FileNum]
    call ZipFindN
    cmp  edx,4
    je   quit
    test [Flags],DECRYPT_MODE
    jz   .nopass
    call ZipDecrypt
    push esi
    mov  ecx,31
  ._:
    mov  al,[esi]
    push eax
    call decrypt_byte
    xor  al,byte[esp]
    add  esp,4
    call UKeys
    mov  [esi],al
    inc  esi
    loop ._
    pop  esi
    ud2
  .nopass:
    cmp  [unp_size],0
    jne  .sizeok
  .sizebadq:
    Msg  13
    jmp  quit
  .notzip:
    call GzipParse
    test edx,edx
    jne   .notgzip
    jmp  .sizeok
  .notgzip:
    call PngParse
    test edx,edx
    jz  .sizeok1
    cmp  edx,21
    je   .sizebad
    test [Flags],RAW_MODE
    jne  .sizeok
    Msg  40
    jmp  exit
  .sizebad:
    Msg  17
    jmp  exit
  .sizeok1:
  if ~ SYS eq win
    call KillViewer
  end if
    Msg  39
    mov  edi,[outp]
    call Deflate.blkbegin
    jmp  .defl_end
  .sizeok:
    call Deflate                ; <===========
  .defl_end:
    test [bits],7
    jnz  .esi_ok
    dec  esi
  .esi_ok:
    push dword[esi] ; <- Adler !!!
    mov  esi,output
    sub  edi,esi
    mov  ecx,edi
    mov  [outfile.size],ecx
    test [Flags],RAW_MODE
    jnz  .skipAdler
    test [Flags],PNG_MODE
    jnz  .skipCRC
    push esi ecx
    call UCRC
    Msg  11
    mov  eax,[CRC32]
    mov  edx,36
    cmp  eax,[CRC_check]
    je   .crcok
    dec  edx
  .crcok:
    Msg  edx
  if  SYS eq win
  else
    dph  eax
  end if
    pop  ecx esi
    jmp  .skipAdler
  .skipCRC:
    call UAdler
    Msg  10
    mov  eax,[Adler32]
    bswap eax   ; <- calculated Adler !!!
    mov  edx,36
    cmp  eax,[esp]
    je   .adlok
    dec  edx
  .adlok:
    Msg  edx
  if  SYS eq win
  else
    dph  eax
  end if
  .skipAdler:
    pop  eax
    Newline
  .nomoreIDAT:
    mov  ebx,output
if SYS eq win
    Msg 37
    invoke CloseHandle, [hnd]

    invoke CreateFile,dumpfile,GENERIC_WRITE, FILE_SHARE_WRITE, NULL, \
    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL
    mov  [hnd],eax
    invoke WriteFile, eax,ebx,[outfile.size],cparam1,NULL
    invoke CloseHandle, [hnd]
else
    test [Flags],PNG_MODE
    jnz  .nosave
    test [Flags],TAR_MODE
    jnz  .nomsg
    Msg 37
  .nomsg:
    mov  [outfile.out],ebx
    mcall 58,outfile
    test [Flags],TAR_MODE
    jnz  .nosave
    call StartPad
  .nosave:
end if
    test  [Flags],PNG_MODE
    jz   NoPng
      xor  eax,eax
      mov  edi,filters
      mov  ecx,6
      rep  stosd
      mov  edi,png_image
      mov  esi,output
;//
      mov  [outp],edi
;//
      call PngFilter
      call FiltStats
      mov  [outfile.size],edi
      mov  ebx,[outp];png_image
if  SYS eq win
exit:
    Msg  12
    invoke CreateFile,outfile,GENERIC_WRITE, FILE_SHARE_WRITE, NULL, \
    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL
    mov  [hnd],eax
    invoke WriteFile, eax,ebx,[outfile.size],cparam1,NULL
    invoke CloseHandle, [hnd]
	call  RunViewer
   	and  [arc_base],0
    and  [Flags],STAY_MODE
	
quit:	
    QueryFile
    test eax,eax
    jnz  again
    invoke ExitProcess,0
else
exit:
    mov  [outfile.out],ebx
    test [Flags],TAR_MODE
    jz   .notar
    Msg  37
    mcall 58,outfile
    call StartPad
 .notar:
    Msg  12
   	call  RunViewer
   	and  [arc_base],0
    and  [Flags],STAY_MODE
quit:
    test [Flags],STAY_MODE
    jnz  red
    mcall -1
end if

NoPng:
    test  [Flags],TAR_MODE
  if SYS eq win
    jz  quit
  else
    jz    still
  end if
    mov   ecx,dumpf_len
    mov   esi,dumpfile
    mov   edi,filename
    rep   movsb
    call  OpenFile
    test  ebx,ebx
    jz    again.sizebadq
    call  TarParse
    mov   ecx,[FileNum]
    call  TarFindN
    cmp   [outfile.size],0
    jz    again.sizebadq
    mov   ebx,esi
    jmp   exit

if ~ SYS eq win
draw_window:
    mcall 12,1
    mov  ebx,15
    mov  ecx,ebx
    mov  edx,0x0e26830
    mov  edi,edx
    mcall 0,,,,0x1f2a870
    mcall 8,,,1+1 shl 30
    mcall 4,<6,5>,0x10d0d0d0,msgs+5,1
    mcall ,<5,5>,0x10202020
    ret
end if

include 'data.inc'

if SYS eq win
section '.idata' import data readable writeable

  library kernel,'KERNEL32.DLL',\
          comdlg,'COMDLG32.DLL',\
          user,'USER32.DLL'

  import kernel,\
     CreateFile,'CreateFileA',\
     GetFileSize,'GetFileSize',\
     ReadFile,'ReadFile',\
     WriteFile,'WriteFile',\
     SetFilePointer,'SetFilePointer',\
     CloseHandle,'CloseHandle',\
   	 GetStdHandle,'GetStdHandle',\
   	 WriteConsole,'WriteConsoleA',\
   	 ReadConsole,'ReadConsoleA',\
   	 CreateProcess,'CreateProcessA',\
   	 WritePrivateProfileString,'WritePrivateProfileStringA',\	
   	 ExitProcess,'ExitProcess'

  import comdlg,\
     GetOpenFileName,'GetOpenFileNameA'

  import user,\
   	 wsprintf,'wsprintfA',\
   	 SendMessage,'SendMessageA',\
   	 FindWindowEx,'FindWindowExA',\
  	  WaitForInputIdle,'WaitForInputIdle'
	

section '.reloc' fixups data readable discardable
end if

if ~ SYS in <meos,win>
    error "Please specify SYS: meos OR win"
end if


