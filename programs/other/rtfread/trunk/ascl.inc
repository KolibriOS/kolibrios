lang equ ru ; ru en fr ge fi

;
;   Assembler
;     SMALL
;       CODE
;         Libary
;
;  Ver 0.14 By Pavlushin Evgeni (RUSSIA)
;  www.waptap@mail.ru

;Please compile aplications on FASM ver1.54 or higer!!!

;InfoList
;0.01 scank,putpix,puttxt
;0.02 label,random,colors
;0.03 window,startwd,endwd,attributes
;0.04 close,delay,scevent ~30.04.2004
;0.05 small random, ~04.05.2004
;0.06 wtevent ~09.05.2004
;0.07 timeevent ~23.05.2004
;0.08 txtput ~14.06.2004
;0.09 opendialog,savedialog ~20.06.2004
;0.10 wordstoreg by halyavin, add at ~30.08.2004
; random bug deleted eax is use.
;0.11 loadfile from me +puttxt bug del ~07.09.2004
;0.12 open/save dialog ~13.09.2004
;0.13 dialogs bugs deleted
;0.14 drawlbut ~03.10.2004

; LOADFILE
; (SYNTAX)  LOADFILE 'full_path_to_file',file_load_area,file_temp_area
; (SAMPLE)  LOADFILE '/rd/1/clock.bmp',load_area,temp_area

macro loadfile file_name,file_load_area,file_temp_area
{
local open,fileinfo,string
    jmp open
fileinfo:
    dd 0
    dd 0
    dd 1
    dd file_load_area
    dd file_temp_area
string:
    db file_name,0
open:
    mov  dword [fileinfo+8],1 ; how many blocks to read (1)
    mov  eax,58
    mov  ebx,fileinfo
    mcall
    mov  eax,[file_load_area+2]
    shr  eax,9 ; поделим на 512 и прибавим 1 - получим число блоков
    inc  eax
    mov  dword [fileinfo+8],eax
    mov  eax,58
    mov  ebx,fileinfo
    mcall
}


macro wordstoreg reg,hiword,loword
{
if  hiword eqtype 0 & loword eqtype 0
    mov  reg,hiword*65536+loword
else if hiword eqtype 12 & loword eqtype eax
    mov  reg,hiword*65536
    add  reg,loword
else if hiword eqtype 12 & loword eqtype [123]
    mov  reg,hiword*65536
    add  reg,loword
else
    mov  reg,hiword
    shl  reg,16
    add  reg,loword
end if
}


; DRAW BUTTON with label

macro drawlbut x,y,xs,ys,text,id,bcolor,tcolor
{
local asd,lab
    jmp asd
lab db  text                 ;arg label
asd:
    wordstoreg ebx,x,xs
    wordstoreg ecx,y,ys
    mov edx,id
    mov esi,bcolor
    mov eax,8
    mcall

    mov eax,asd-lab          ;calc size
    mov ebx,6
    mul ebx
    mov esi,eax

    mov eax,xs
    sub eax,esi
    shr eax,1
    add eax,x

    mov edx,ys
    sub edx,7
    shr edx,1
    add edx,y

    mov ebx,eax
    shl ebx,16
    add ebx,edx

    mov ecx,tcolor             ;arg4 color
    mov edx,lab
    mov esi,asd-lab          ;calc size
    mov eax,4
    mcall
}


macro opendialog redproc,openoff,erroff,path
{
local new_d, get_loops, dlg_pid_get, DLGPID, num_of_proc
local run_fileinfo, param
local getmesloop, loox, mred, mkey, mbutton, mgetmes
local dlg_is_work, ready, procinfo
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
    mov eax,9
    mov ebx,procinfo
    mov ecx,-1
    mcall

; convert eax bin to param dec
    mov eax,dword [procinfo+30]  ;offset of myPID
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

; wirite 1 byte space to param
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

; change wanted events list 7-bit IPC event
    mov eax,40
    mov ebx,01000111b
    mcall

;
; STEP 3 run SYSTEM XTREE with parameters
;

    mov eax,70
    mov ebx,run_fileinfo
    mcall

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
    mov ebx,procinfo
    mov ecx,-1
    mov eax,9
    mcall
    mov ebp,eax

loox:
    mov eax,9
    mov ebx,procinfo
    mov ecx,ebp
    mcall
    mov eax,[DLGPID]
    cmp [procinfo+30],eax    ;IF Dialog find
    je  dlg_is_work          ;jmp to dlg_is_work
    dec ebp
    jnz loox

    jmp erroff

dlg_is_work:
    cmp [procinfo+50],word 9 ;If slot state 9 - dialog is terminated
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
    mov  eax,2
    mcall                   ; read (eax=2)
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
 db '/RD/1/SYSXTREE',0

procinfo:
times 1024 db 0
}


macro savedialog redproc,openoff,erroff,path
{
local new_d, get_loops, dlg_pid_get, DLGPID, num_of_proc
local run_fileinfo, run_filepath, param
local getmesloop, loox, mred, mkey, mbutton, mgetmes
local dlg_is_work, ready, procinfo
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
    mov eax,9
    mov ebx,procinfo
    mov ecx,-1
    mcall

; convert eax bin to param dec
    mov eax,dword [procinfo+30]  ;offset of myPID
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

; wirite 1 byte space to param
    mov [param+4],byte 32    ;Space for next parametr
; and 1 byte type of dialog to param
    mov [param+5],byte 'S'   ;Get Open dialog (Use 'S' for Save dialog)

;
; STEP2 prepare IPC area for get messages
;

; prepare IPC area
    mov [path],dword 0
    mov [path+4],dword 8

; define IPC memory
    mov eax,60
    mov ebx,1        ; define IPC
    mov ecx,path ; offset of area
    mov edx,1024+16      ; size
    mcall

; change wanted events list 7-bit IPC event
    mov eax,40
    mov ebx,01000111b
    mcall

;
; STEP 3 run SYSTEM XTREE with parameters
;

    mov eax,70
    mov ebx,run_fileinfo
    mcall

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
    mov ebx,procinfo
    mov ecx,-1
    mov eax,9
    mcall
    mov ebp,eax

loox:
    mov eax,9
    mov ebx,procinfo
    mov ecx,ebp
    mcall
    mov eax,[DLGPID]
    cmp [procinfo+30],eax    ;IF Dialog find
    je  dlg_is_work          ;jmp to dlg_is_work
    dec ebp
    jnz loox

    jmp erroff

dlg_is_work:
    cmp [procinfo+50],word 9 ;If slot state 9 - dialog is terminated
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
    mov  eax,2
    mcall                   ; read (eax=2)
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
   dd 0  ; My dec PID
   dd 0,0  ; Type of dialog

run_fileinfo:
 dd 7
 dd 0
 dd param
 dd 0
 dd 0
;run_filepath:
 db '/RD/1/SYSXTREE',0

procinfo:
times 1024 db 0
}





; RANDOM - generate random count (small)
; (SYNTAX)  RANDOM MaxCount,OutArgument
; (SAMPLE)  RANDOM 10000,eax
; ( NOTE )  Maxint<65536 ; use random 65536,eax for more combinations

randomuse = 0

macro random arg1,arg2
{
local rxproc
randomuse = randomuse + 1

      jmp rxproc

if defined randomuse & randomuse = 1
randomproc:
      jmp rnj
rsx1 dw 0x4321
rsx2 dw 0x1234
rnj:
;    mov eax,arg1
    push bx
    push cx
    push dx
    push si
    push di
    mov cx,ax
    mov ax,word ptr rsx1
    mov bx,word ptr rsx2
    mov si,ax
    mov di,bx
    mov dl,ah
    mov ah,al
    mov al,bh
    mov bh,bl
    xor bl,bl
    rcr dl,1
    rcr ax,1
    rcr bx,1
    add bx,di
    adc ax,si
    add bx,0x62e9
    adc ax,0x3619
    mov word ptr rsx1,bx
    mov word ptr rsx2,ax
    xor dx,dx
    cmp ax,0
    je nodiv
    cmp cx,0
    je nodiv
    div cx
nodiv:
    mov ax,dx
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    and eax,0000ffffh
;    mov arg2,0
;    mov arg2,eax
    ret
end if

rxproc:
    mov eax,arg1
    call randomproc
    mov arg2,eax
}

macro scank
{
    mov eax,10
    mcall
}

macro putpix x,y,color
{
    mov ebx,x
    mov ecx,y
    mov edx,color
    mov eax,1
    mcall
}

macro puttxt x,y,offs,size,color
{
;    mov ebx,x
;    shl ebx,16
;    add ebx,y
    wordstoreg ebx,x,y
    mov ecx,color
    mov edx,offs
    mov esi,size
    mov eax,4
    mcall
}

macro outcount data, x, y, color, numtype
{
    mov ecx,data
    mov ebx,numtype
    mov bl,0
;    mov edx,x*65536+y
    wordstoreg edx,x,y
    mov esi,color
    mov eax,47
    mcall
}

; SCEVENT - Scan event

macro scevent red,key,but
{
    mov eax,11
    mcall
    dec eax
    jz  red
    dec eax
    jz  key
    dec eax
    jz  but
}

; WTEVENT - Wait event

macro wtevent red,key,but
{
    mov eax,10
    mcall
    dec eax
    jz  red
    dec eax
    jz  key
    dec eax
    jz  but
}

; TIMEEVENT - Wite for event with timeout

macro timeevent xfps,noevent,red,key,but
{
    mov eax,23
    mov ebx,xfps
    mcall
    cmp eax,0
    je  noevent
    dec eax
    jz  red
    dec eax
    jz  key
    dec eax
    jz  but
}


; CLOSE - Close program

macro close
{
    mov eax,-1
    mcall
}

; DELAY - Create delay 1/100 sec
; (SYNTAX)  Delay time
; (SAMPLE)  Delay 100   ;delay 2 sec 1/100*200=2 sec

macro delay arg1
{
    mov eax,5
    mov ebx,arg1
    mcall
}

; WINDOW - Draw window
; (SYNTAX)  WINDOW Xstart,Ystart,'Text',Color
; (SAMPLE)  WINDOW 10,10,640+8,480+24,window_Skinned

macro window arg1,arg2,arg3,arg4,arg5
{
;    mov ebx,arg1*65536+arg3
;    mov ecx,arg2*65536+arg4
    wordstoreg ebx,arg1,arg3
    wordstoreg ecx,arg2,arg4
    mov edx,arg5
    mov eax,0
    mcall
}

macro colorwindow arg1,arg2,arg3,arg4,arg5,arg6,arg7
{
    mov ebx,arg1*65536+arg3
    mov ecx,arg2*65536+arg4
    mov edx,arg5
    mov esi,arg6
    mov edi,arg7
    mov eax,0
    mcall
}


; STARTWD - Start of window draw

macro startwd
{
    mov eax,12
    mov ebx,1
    mcall
}

; ENDWD - End window draw

macro endwd
{
    mov eax,12
    mov ebx,2
    mcall
}

; LABEL - Put text to frame
; (SYNTAX)  LABEL Xstart,Ystart,'Text',Color
; (SAMPLE)  LABEL 10,12,'Hello World!',cl_Green+font_Big


;Key's
key_Up     equ 178
key_Down   equ 177
key_Right  equ 179
key_Left   equ 176
key_Esc    equ 27
key_Space  equ 32
key_Enter  equ 13
key_Bspace equ 8
key_F1     equ 50
key_F2     equ 51
key_F3     equ 52
key_F4     equ 53
key_F5     equ 54
key_F6     equ 55
key_F7     equ 56
key_F8     equ 57
key_F9     equ 48
key_F10    equ 49
key_F11    equ 68
key_F12    equ 255
key_Home   equ 180
key_End    equ 181
key_PgUp   equ 184
key_PgDown equ 183

;Attributes

;Window Attributes
window_Skinned equ 0x03000000
window_Type2   equ 0x02000000
window_Type1   equ 0x00000000
window_Reserve equ 0x01000000

;Font Attributes
font_Big  equ 0x10000000

;Colors
cl_White  equ 0x00ffffff
cl_Black  equ 0x00000000
cl_Grey   equ 0x00888888
cl_Red    equ 0x00ff0000
cl_Lime   equ 0x0000ff00
cl_Green  equ 0x0000af00
cl_Blue   equ 0x000000ff
cl_Purple equ 0x008080ff
cl_Violet equ 0x008040ff
cl_Cyan   equ 0x0040e0ff
