; @RCHER system-dependent and other stuff
; Written in pure assembler by Ivushkin Andrey aka Willow

OpenFile:
    mov  [outp],output
    mov  esi,area
    and  [Finfo.block],0
if SYS eq win
    invoke CreateFile, filename, GENERIC_READ, FILE_SHARE_READ, NULL, \
    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL
    inc  eax
    test eax,eax
    jz   .ex
    dec  eax
    mov  [hnd],eax
    invoke GetFileSize,eax,NULL
  .ex:
    mov  ebx,eax
    mov  [filesize],eax
else
    and  dword [attrinfo+32], 0
    mcall 70,attrinfo
    mov  eax,[attrinfo+32]
    mov  [filesize],ebx
    mcall 70,Finfo
end if
    ret

ResetFile:
    and  [file_count],0
    mov  eax,[arc_base]
    xor  ebx,ebx
    mov  esi,area
    call FileSeek
    ret


FileSeek:
; eax - offset
; esi - current pointer (in memory!)
; ebx - translation method: 0 - beginning, 1 - current, 2 - end
    push eax ecx edx
    sub  esi,BUFPTR
    add  esi,[Finfo.block]
    cmp  ebx,1
    je   .cur
    jb   .begin
    mov  esi,[filesize]
    not  eax
  .cur:
    add  eax,esi
  .begin:
    mov  esi,BUFPTR
    mov  [Finfo.block],eax
if SYS eq win
    invoke SetFilePointer,[hnd],eax,NULL,FILE_BEGIN
    invoke ReadFile,[hnd],area,INBUF, os_work, NULL
;    add  [byte_count],area
else
;    pregs
;    wait
    mov  [Finfo.count],BUFSIZE*1024
    mcall 70,Finfo
;    mov  [byte_count],area+INBUF
end if
    pop  edx ecx eax
    ret

macro QueryFile
{
local ex
if SYS eq win
  	invoke GetOpenFileName,ofstruc
else
   opendialog draw_window,QFok,QFcanc,filename
 QFok:
   xor eax,eax
   jmp ex
   ret
 QFcanc:
   mov eax,1
end if
 ex:
}

RunViewer:
    test   [Flags],PNG_MODE
    jz   .ex
 if SYS eq win
   	mov   eax,[PNG_info.Width]
   	call  int2str
   	invoke WritePrivateProfileString,ini_sec,ini_rwidth,os_work,ini_file
   	mov   eax,[PNG_info.Height]
   	call  int2str
   	invoke WritePrivateProfileString,ini_sec,ini_rheight,os_work,ini_file
   	invoke CreateProcess,NULL,iview_cmd,NULL,NULL,TRUE,\
    		NORMAL_PRIORITY_CLASS,NULL,NULL,suinfo,pinfo
   	invoke WaitForInputIdle,dword[pinfo],0xFFFFFFFF
   	invoke FindWindowEx, NULL,NULL,NULL,rawwnd_txt
   	invoke FindWindowEx, eax,NULL,NULL,ok_txt
   	invoke SendMessage,eax,BM_CLICK,NULL,NULL
 else
    test [Flags],THREAD_YES
    jnz  .ex
    mcall 51,1,thread,child_stack_top;MEMORY
    mov  [child],eax
 end if
  .ex:	
	   ret

if SYS eq win
int2str:
; in: eax - number
; out: formatted string -> os_work
   	invoke wsprintf,os_work,fmt_str,eax
   	add   esp,12
    ret

rawwnd_txt db 'Set RAW open parameters',0
ok_txt  db 'OK',0
ini_sec db 'RAW',0
ini_rwidth db 'RWidth',0
ini_rheight db 'RHeight',0
ini_file db "C:\Program Files\IrfanView\i_view32.ini",0
fmt_str db "%d",0	
iview_cmd db '"C:\Program Files\IrfanView\i_view32.exe"'
          db ' "D:\Ivushkin\projects\zip\output.raw"',0
filt_str:
	db 'Pictures (*.png)',0,'*.png',0
	db 'Archives (*.zip;*.*gz)',0,'*.zip;*.*gz',0
	db 'All files (*.*)',0,'*.*',0,0
	
suinfo STARTUPINFO
pinfo  PROCESS_INFORMATION
cpstruc:
ofstruc:
	dd  ofstruc_end-ofstruc
	dd  NULL
	dd  NULL
	dd  filt_str
	dd  NULL
	dd  NULL
	dd  0
	dd  filename
	dd  256
	dd  NULL
	dd  0
	dd  NULL
	dd  NULL
	dd  NULL
	dw  NULL
	dw  NULL
	dd  NULL
	dd  NULL
	dd  NULL
	dd  NULL
	dd  NULL
	dd  NULL
	dd  NULL
ofstruc_end:	 		

 else
    mov  ebx,-1
    mov  ecx,ebx
    mov  edx,[PNG_info.Width]
    add  edx,10
    mov  esi,[PNG_info.Width]
    add  esi,30
    mcall 67
    mcall 7,outfile,[outfile.size],10 shl 16+15
 .ext:
    ret

MIN_WIDTH equ 300
thread:
 .red:
    mcall 12,1
    mov  ebx,[PNG_info.Width]
    cmp  ebx,MIN_WIDTH
    jae  .more
    mov  ebx,MIN_WIDTH
  .more:
    add  ebx,20
    mov  ecx,[PNG_info.Height]
    add  ecx,30
    mcall 0,,,0x3808080
    ;mcall 4,<5,7>,0x10f0f0f0,filename,255
    mcall 71,1,filename,255 ;by Leency
    mov  ecx,[PNG_info.Width]
    shl  ecx,16
    add  ecx,[PNG_info.Height]
    mcall 7,[png_],,10 shl 16+25
    mcall 12,2
  .still:
    mcall 10
    cmp  eax,1
    je   .red
    cmp  eax,2
    jne  .nokey
    mcall 2
    cmp  ah,27 ; Esc - close
    je   .close
    jmp  .still
  .nokey:
    cmp  eax,3
    jne  .still
  .close:
    and  [child],0
    mcall 64,1,MEMINIT
    mcall -1

KillViewer:
    pusha
    mov  edi,[child]
    test edi,edi
    jz   .noth
    mcall 9,os_work,-1
    mov  ecx,eax
  .fchild:
    push ecx
    mcall 9,os_work
    cmp  edi,[ebx+30]
    jne  .lp
    mov  ecx,[esp]
    mcall 18,2
    pop  ecx
    jmp  .noth
  .lp:
    pop  ecx
    loop .fchild
  .noth:
    popa
    ret

macro CmdLine
{
    mov  esi,PARAM_PTR
  .parse:
    lodsb

    test al,al
    jnz  .noend
    or  [Flags],STAY_MODE
    jmp  red
  .noend:
    cmp  al,' '
    je   .stay
    cmp  al,'/'
    jne  .yespar
    dec  esi
    mov  ecx,255
    mov  edi,filename
    rep  movsb
    jmp  again;cmdl
  .yespar:
    cmp  al,'N'
    jne  .nonum
    call get_6ASCII_num
  .fnum:
    mov  [FileNum],eax
    jmp  .parse
  .nonum:
    cmp  al,'n'
    jne  .nonum2
    lodsd
    jmp  .fnum
  .nonum2:
    cmp  al,'s'
    jne  .nostay
  .stay:
    or   [Flags],STAY_MODE
    jmp  .parse
  .nostay:
    cmp  al,'i'
    jne  .noclPID
    lodsd
    mov  [clientPID],eax
    or   [Flags],IPC_MODE
    jmp  .parse
  .noclPID:
    cmp  al,'R'
    jne  .noraw
    or   [Flags],RAW_MODE
    jmp  .parse
  .noraw:
    cmp  al,'q'
    jne  .noofs
    lodsd
  .fofs:
    mov  [arc_base],eax
    jmp  .parse
  .noofs:
    cmp  al,'Q'
    jne  .noofs2
    call get_6ASCII_num
    jmp  .fofs
  .noofs2:
    cmp  al,'L'
    jne  .nolist
    or   [Flags],LIST_MODE
  .nolist:
    jmp  .parse

get_6ASCII_num:
; in: esi - str ptr, out: eax - num
    xor  edx,edx
    mov  ebx,10
    mov  ecx,6
  .lp:
    xor  eax,eax
    lodsb
    sub  al,'0'
    imul edx,ebx
    add  edx,eax
    loop .lp
    mov  eax,edx
    ret
}

StartPad:
    pusha
    mov   esi,[outfile.size]
;    dpd  esi
; convert number in esi to decimal representation
        mov     ecx, 10
        push    -'0'
        mov     eax, esi
@@:
        xor	edx, edx
        div     ecx
        push    edx
        test    eax, eax
        jnz     @b
        mov     edi, par_fsize
@@:
        pop     eax
        add     al, '0'
        stosb
        jnz     @b
    mcall 70,fileinfo
    mov  ecx,eax
    mcall 5,20
    mcall 60,2,,[outfile.out];output
    mcall 64,1,MEMINIT
    popa
    ret

fileinfo:
        dd      7
        dd      0
        dd      editor_par
        dd      0
        dd      0
        db      '/sys/TINYPAD',0
editor_par db '*'
par_fsize rb 11
end if