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
    mcall 58,Finfo
    mov  [filesize],ebx
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
    mov  edx,[Finfo.block]
    imul edx,BSIZE
    sub  esi,BUFPTR
    add  esi,edx
    cmp  ebx,1
    je   .cur
    jb   .begin
    mov  esi,[filesize]
    not  eax
  .cur:
    add  eax,esi
  .begin:
    mov  ecx,BSIZE
    xor  edx,edx
    div  ecx
    lea  esi,[BUFPTR+edx]
    mov  [Finfo.block],eax
if SYS eq win
    mul  ecx
    invoke SetFilePointer,[hnd],eax,NULL,FILE_BEGIN
    invoke ReadFile,[hnd],area,INBUF, os_work, NULL
;    add  [byte_count],area
else
;    pregs
;    wait
    mov  [Finfo.count],BUFSIZE*2
    mcall 58,Finfo
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
    mcall 51,1,thread,child_stack;MEMORY
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
 .ex:
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
    mcall 4,<5,7>,0x10f0f0f0,filename,255
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
    mov  ecx,255
    dec  esi
    mov  edi,filename
    xor  al,al
    rep  stosb
    mov  ecx,255
    mov  edi,filename
    rep  movsb
;    test [Flags],LIST_MODE
;    jz   again
    xor  eax,eax
    mov  edi,filename
    mov  ecx,255
    repne scasb
    cmp  byte[edi-2],'/'
    je   .slash
    mov  byte[edi-1],'/'
    inc  edi
  .slash:
;    Dump filename,255,os_work
;    ud2
    mov  ecx,edi
;    dec  ecx
    mov  edx,ecx
    mov  dword[Finfo],13
;    mov  dword[Finfo.count],1
    mov  edi,filename+5
    sub  ecx,edi
  .lp:
    mov  al,'/'
    repne scasb
;    jne  .ex
;    dpd  ecx
    and  byte[edi-1],0
    mcall 58,Finfo
    mov  byte[edi-1],'/'
    test ebx,32
    jz   .lp
    test [Flags],LIST_MODE
    jne  .listm
    and  byte[edi-1],0
;    Dump filename,255,os_work
    jmp  .agg
;    ud2
  .listm:
    lea  esi,[edi-1+ecx]
    lea  edi,[esi+1]
    std
    rep  movsb
    and  byte[edi],0
    cld
;    dpd  filename

    inc  edi
    dpd  edi
    cmp  edx,edi
    jne  .slash2
;    cmp  byte[edi],'/'
;    je   .slash2
    mov  byte[edi],'/'
    inc  edx
  .slash2:
    sub  edx,edi
    mov  [lpath],edi
    mov  [lpath_len],edx
    dpd  edx
  .agg:
    mov  dword[Finfo],0
;    ud2
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
        db      '/RD/1/TINYPAD',0
editor_par db '*'
par_fsize rb 11
end if