; @RCHER macros and messages
; Written in pure assembler by Ivushkin Andrey aka Willow

macro stop
{
  if SYS eq win & NO_STOPS eq 0
    int3
  end if
}

macro wait
{
if ~ SYS eq win
local .nodrw,.ex
   pusha
   mcall 10
   cmp   eax,1
   jne   .nodrw
   call  draw_window
 .nodrw:
   mcall 17
 .ex:
   popa
end if
}

macro rbits dest,cnt
{
    if ~ cnt eq
      mov  ecx,cnt
    end if
    if   dest eq 0
      xor  eax,eax
    else
      mov  eax,dest
    end if
    call read_bits
}


macro rep_err cond_ok, result, msg
{
  local .m
    j#cond_ok .m
   if ~ msg eq
    Msg msg
   end if
    mov  edx,result
    jmp  .err
  .m:
}

macro memcpy_esi dest, count,_add
{
    push esi
  if ~ _add eq
    add  esi,_add
  end if
    mov  edi,dest
    mov  ecx,count
    rep  movsb
    pop  esi
}

DebugPrint:
; in: edx-asciiz string ptr
    pusha
if SYS eq win
    mov  ecx,255
    xor  eax,eax
    mov  edi,edx
    repne scasb
    sub  edi,edx
    dec  edi
    invoke WriteConsole,[cons],edx,edi,cparam1,NULL
else
    call debug_outstr
end if
    popa
    ret

macro DebugPrintDec
{
if SYS eq win
    call int2str
    mov  edx,os_work
    call DebugPrint
else
    call debug_outdec
end if
}

macro Newline
{
if SYS eq win
    pusha
    invoke WriteConsole,[cons],cr_lf,2,cparam1,NULL
    popa
else
    newline
end if
}

Msgcall:
; edx - msg number
  pusha
  movzx edx,word[strs+edx*2]
  add  edx,msgs
  call DebugPrint
  popa
  ret

macro Msg num
{
 if ~ num eq edx
   mov  edx,num
 end if
   call Msgcall
}

MSG_COUNT=0
macro str_table [str]
{
 forward
   local label
   dw label-msgs
 common
   msgs:
 forward
   label db str,0
   MSG_COUNT=MSG_COUNT+1
}



strs:
if  language eq ru_RU
str_table \
   <'***  @RCHER - Deflate Unpacker  ***',13,10>,\  ;0
   <'*** Copyright Wiland Inc., 2005 ***',13,10>,\  ;1
   '������ � ��娢�: ',\ ;2
   <'End of Directory ���⨣���',13,10>,\  ;3
   '��� 䠩�� � ⠪�� ����஬',\  ;4
   <'FindByNumber ��',13,10>,\  ;5
   <'������ ID ����� ���䬠��',13,10>,\  ;6
   <'�訡�� � ����� ���䬠��!',13,10>,\  ;7
   <13,10,'����� ⠡���� CRC32',13,10>,\  ;8
   '.',\  ;9
   'Adler32',\  ;10
   'CRC32',\ ;11
   <'�����襭�.',13,10>,\  ;12
   <'���� ����� �㫥��� �����, �ᯠ����� �⬥����.',13,10>,\  ;13
   <'��࠭��� ����� 䠩��-���筨��. �訡�� � �����? �⬥��.',13,10>,\  ;14
   <'�� GZIP ��娢',13,10>,\  ;15
   <'���誮� ����让 ࠧ��� ��室���� 䠩��',13,10>,\  ;16
   <"�訡�� �� �ᯠ�����.",13,10>,\  ; 17
   <'�� ��㭮� PNG',13,10>,\ ; 18
   <13,10,'*�'>,\  ;19
   <13,10,'*�'>,\  ;20
   '���������� ����� ������ IDAT',\  ;21
   '��䨫�����:',\  ;22
   '�������: None ',\  ;23
   ' Sub ',\  ;24
   ' Up ',\  ;25
   ' Avg ',\  ;26
   ' Paeth ',\  ;27
   ' �訡��: ',\  ;28
   <'��������� PNG',13,10>,\  ;29
   <'���� ��� ᦠ��',13,10>,\  ;30
   <'��� 梥� �� �����ন������',13,10>,\  ;31
   <'������ ��஫� ��� ����஢������ 䠩��:',13,10>,\  ;32
   <'���� 䠩��...',13,10>,\  ;33
   '�� ����� SFX',\  ;34
   ' ����ୠ! ',\  ;35
   ' OK: ',\  ;36
   <'������ OUT.TXT',13,10>,\  ;37
   ' ',\  ;38
   <'�����⮢�� ����ࠦ����...',13,10>,\  ;39
   <'������ "R" ��� ��ࠡ�⪨ ����� ������. �⬥��.',13,10>,\  ;40
   <'�� 墠⠥� �����! �⬥��.',13,10>,\  ; 41
   ' ',\  ;
   ' ',\  ;
   ' ',\  ;
   ' ',\  ;
   ' ',\  ;
   ' ',\  ;
   ' ',\  ;
   ' ',\  ;
   ' <- ��������� ��⮤ ᦠ��',\  ;50
   ' <- ����஢��',\  ;51
   <'�� ZIP ��娢',13,10> ; 52
else
str_table \
   <'***  @RCHER - Deflate Unpacker  ***',13,10>,\  ;0
   <'*** Copyright Wiland Inc., 2005 ***',13,10>,\  ;1
   'Files in archive: ',\ ;2
   <'End of Directory reached',13,10>,\  ;3
   'No file has this number',\  ;4
   <'FindByNumber succeded',13,10>,\  ;5
   <'Invalid Huffman block ID',13,10>,\  ;6
   <'Error while getting Huffman codes!',13,10>,\  ;7
   <13,10,'Rebuilding CRC32 table',13,10>,\  ;8
   '.',\  ;9
   'Adler32',\  ;10
   'CRC32',\ ;11
   <'Job finished.',13,10>,\  ;12
   <'File of zero length, unpacking aborted.',13,10>,\  ;13
   <'Source file has strange length, may be missing? Abort.',13,10>,\  ;14
   <'Not a GZIP archive',13,10>,\  ;15
   <'Destination file is too large for now',13,10>,\  ;16
   <"Can't unpack content.",13,10>,\  ; 17
   <'Not a PNG image',13,10>,\ ; 18
   <13,10,'*d'>,\  ;19
   <13,10,'*s'>,\  ;20
   'Unexpected end of IDAT chunks',\  ;21
   'Unfiltering:',\  ;22
   'Filters: None ',\  ;23
   ' Sub ',\  ;24
   ' Up ',\  ;25
   ' Avg ',\  ;26
   ' Paeth ',\  ;27
   ' Invalid ',\  ;28
   <'Interlaced PNG',13,10>,\  ;29
   <'Stored block',13,10>,\  ;30
   <'Color type not supported',13,10>,\  ;31
   <'Enter password for encrypted file:',13,10>,\  ;32
   <'Searching for file...',13,10>,\  ;33
   'Not a SFX module',\  ;34
   ' error! ',\  ;35
   ' OK: ',\  ;36
   <'Creating OUT.TXT',13,10>,\  ;37
   ' ',\  ;38
   <'Preparing bitmap...',13,10>,\  ;39
   <'Specify "R" to force raw data. Abort.',13,10>,\  ;40
   <'Not enough memory! Abort.',13,10>,\  ;
   ' ',\  ;
   ' ',\  ;
   ' ',\  ;
   ' ',\  ;
   ' ',\  ;
   ' ',\  ;
   ' ',\  ;
   ' ',\  ;
   ' <- unknown compression method',\  ;50
   ' <- encrypted',\  ;51
   <'Not a ZIP archive',13,10> ; 52
end if

macro opendialog redproc,openoff,erroff,path
{
local new_d, get_loops, dlg_pid_get, DLGPID, num_of_proc
local run_fileinfo, param
local getmesloop, loox, mred, mkey, mbutton, mgetmes
local dlg_is_work, ready
;
; STEP 1 Run SYSXTREE with parametrs MYPID 4 bytes in dec,
; 1 byte space, 1 byte type of dialog (O - Open ,S - Save)
;

    cld
;;    mov esi,path
    mov edi,path
    xor eax,eax
    mov ecx,(1024+16)/4
    rep stosd

;mov [get_loops],0
mov [dlg_pid_get],0

; Get my PID in dec format 4 bytes
    mcall 9,os_work,-1

; convert eax bin to param dec
    mov eax,dword [os_work+30]  ;offset of myPID
    mov edi,param+4-1            ;offset to 4 bytes
    mov ecx,4
    mov ebx,10
    cld
new_d:
    xor edx,edx
    div ebx
    add dl,'0'
    mov [edi],dl
    dec edi
    loop new_d

; write 1 byte space to param
    mov [param+4],byte 32    ;Space for next parametr
; and 1 byte type of dialog to param
    mov [param+5],byte 'O'   ;Get Open dialog (Use 'S' for Save dialog)

;
; STEP2 prepare IPC area for get messages
;

; prepare IPC area
    mov [path],dword 0
    mov [path+4],dword 8

; define IPC memory
    mov eax,60
    mov ebx,1        ; define IPC
    mov ecx,path     ; offset of area
    mov edx,1024+16  ; size
    mcall

    mcall 40,1000111b
;
; STEP 3 run SYSTEM XTREE with parameters
;

    mcall 70,run_fileinfo

    call redproc

    mov [get_loops],0
getmesloop:
    mov eax,23
    mov ebx,50     ;0.5 sec
    mcall
        dec     eax
        jz      mred
        dec     eax
        jz      mkey
        dec     eax
        jz      mbutton
        cmp     al, 7-3
        jz      mgetmes

; Get number of procces
    mcall 9,os_work,-1
    mov ebp,eax

loox:
    mcall 9,os_work,ebp
    mov eax,[DLGPID]
    cmp dword[os_work+30],eax    ;IF Dialog find
    je  dlg_is_work          ;jmp to dlg_is_work
    dec ebp
    jnz loox

    jmp erroff

dlg_is_work:
    cmp word[os_work+50],word 9 ;If slot state 9 - dialog is terminated
    je  erroff                 ;TESTODP2 terminated too

    cmp [dlg_pid_get],dword 1
    je  getmesloop
    inc [get_loops]
    cmp [get_loops],4  ;2 sec if DLG_PID not get, TESTOP2  terminated
    jae erroff
    jmp getmesloop

mred:
    call redproc
    jmp  getmesloop
mkey:
    mcall 2  ; read (eax=2)
    jmp  getmesloop
mbutton:
    mov  eax,17                 ; get id
    mcall
    cmp  ah,1                   ; button id=1 ?
    jne  getmesloop
    mov  eax,-1                 ; close this program
    mcall
mgetmes:
; If dlg_pid_get then second message get jmp to still
    cmp  [dlg_pid_get],dword 1
    je   ready

; First message is number of PID SYSXTREE dialog

; convert PID dec to PID bin
    movzx eax,byte [path+16]
    sub eax,48
    imul eax,10
    movzx ebx,byte [path+16+1]
    add eax,ebx
    sub eax,48
    imul eax,10
    movzx ebx,byte [path+16+2]
    add eax,ebx
    sub eax,48
    imul eax,10
    movzx ebx,byte [path+16+3]
    add eax,ebx
    sub eax,48
    mov [DLGPID],eax

; Claear and prepare IPC area for next message
    mov [path],dword 0
    mov [path+4],dword 8
    mov [path+8],dword 0
    mov [path+12],dword 0
    mov [path+16],dword 0

; Set dlg_pid_get for get next message
    mov [dlg_pid_get],dword 1
    call redproc   ;show DLG_PID
    jmp  getmesloop

ready:
;
; The second message get
; Second message is 1024 bytes path to SAVE/OPEN file
; shl path string on 16 bytes
;
    cld
    mov esi,path+16
    mov edi,path
    mov ecx,1024/4
    rep movsd
    mov [edi],byte 0

    jmp openoff


; DATA AREA
get_loops   dd 0
dlg_pid_get dd 0
DLGPID      dd 0

param:
   dd 0    ; My dec PID
   dd 0,0  ; Type of dialog

run_fileinfo:
 dd 7
 dd 0
 dd param
 dd 0
 dd 0
;run_filepath
 db '/sys/SYSXTREE',0

}

