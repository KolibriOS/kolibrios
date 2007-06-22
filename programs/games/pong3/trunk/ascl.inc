lang equ ru ; ru en fr ge fi

;
;   Assembler
;     SMALL
;       CODE
;         Libary
;
;  Ver 0.09 By Pavlushin Evgeni (RUSSIA)
;  www.waptap@mail.ru

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

macro opendialog redproc,openoff,erroff,path,testtime
{
local still, dlg_file, clip_file, redwait
local button, key, red, zzz, still, zzz2, incf, file_is_open, zzz3

    mov ecx,200
zzz3:
    mov byte [path+ecx-1],0
    loop zzz3

    dec byte [clip_file+7]
incf:
    inc byte [clip_file+7]
    mov eax,6
    mov ebx,clip_file
    mov ecx,0
    mov edx,-1
    mov esi,path
    mcall
    cmp byte [clip_file+7],'Z'
    je  erroff
    cmp eax,0xffffffff
    jne incf

    mov ecx,200
zzz2:
    mov byte [path+ecx],0
    loop zzz2

    mov ebx,clip_file
    mov ecx,path
    mov edx,100
    mov esi,0
    mov eax,33
    mcall

    mov ebx,dlg_file
    mov ecx,clip_file
    mov eax,19
    mcall

still:
    scevent red,key,button
    cmp byte [path],0
    jne file_is_open
    cmp dword [redwait],testtime ;20
    jnae zzz
    mov eax,6
    mov ebx,clip_file
    mov ecx,0
    mov edx,-1
    mov esi,path
    mcall
    cmp eax,0xffffffff
    je  erroff ;errexit
    mov dword [redwait],0
zzz:
    delay 5
    inc dword [redwait]
    jmp still

file_is_open:
    mov eax,32
    mov ebx,clip_file
    mcall
    jmp openoff

key:                          ; key
    mcall                   ; read (eax=2)
    jmp  still
button:                       ; button
    mov  eax,17                 ; get id
    mcall
    cmp  ah,1                   ; button id=1 ?
    jne  still
    mov  eax,-1                 ; close this program
    mcall

red:
    call redproc
    jmp  still

dlg_file  db 'SYSXTREE   '
clip_file db 'OPENCLPACLP'
          db 'O'  ;Dialog type O-open S-save
          db 0    ;Marker
redwait dd 0
}

macro savedialog redproc,openoff,erroff,path,testtime
{
local still, dlg_file, clip_file, redwait, zzz3
local button, key, red, zzz, still, zzz2, incf, file_is_open

    mov ecx,200
zzz3:
    mov byte [path+ecx-1],0
    loop zzz3

    dec byte [clip_file+7]
incf:
    inc byte [clip_file+7]
    mov eax,6
    mov ebx,clip_file
    mov ecx,0
    mov edx,-1
    mov esi,path
    mcall
    cmp byte [clip_file+7],'Z'
    je  erroff
    cmp eax,0xffffffff
    jne incf

    mov ecx,200
zzz2:
    mov byte [path+ecx],0
    loop zzz2

    mov ebx,clip_file
    mov ecx,path
    mov edx,100
    mov esi,0
    mov eax,33
    mcall

    mov ebx,dlg_file
    mov ecx,clip_file
    mov eax,19
    mcall

still:
    scevent red,key,button
    cmp byte [path],0
    jne file_is_open
    cmp dword [redwait],testtime ;20
    jnae zzz
    mov eax,6
    mov ebx,clip_file
    mov ecx,0
    mov edx,-1
    mov esi,path
    mcall
    cmp eax,0xffffffff
    je  erroff ;errexit
    mov dword [redwait],0
zzz:
    delay 5
    inc dword [redwait]
    jmp still

file_is_open:
    mov eax,32
    mov ebx,clip_file
    mcall
    jmp openoff

key:                          ; key
    mcall                   ; read (eax=2)
    jmp  still
button:                       ; button
    mov  eax,17                 ; get id
    mcall
    cmp  ah,1                   ; button id=1 ?
    jne  still
    mov  eax,-1                 ; close this program
    mcall

red:
    call redproc
    jmp  still

dlg_file  db 'SYSXTREE   '
clip_file db 'SAVECLPACLP'
          db 'S'  ;Dialog type O-open S-save
          db 0    ;Marker
redwait dd 0
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
;    mov arg2,0
;    mov arg2,eax
    ret
end if

rxproc:
    mov eax,arg1
    call randomproc
    mov arg2,0
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
    mov ebx,x
    shl ebx,16
    add ebx,y
    mov ecx,color
    mov edx,offs
    mov esi,size
    mov eax,4
    mcall
}

;_ equ '____'

;macro libtest x,y,color
;{
;if x = as dword
;    mov ebx,x
;end if
;    mov ecx,y
;    mov edx,color
;    mov eax,1
;    mcall
;}


macro outcount data, x, y, color, numtype
{
    mov ecx,data
    mov ebx,numtype
    mov bl,0
    mov edx,x*65536+y
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
    mov ebx,arg1*65536+arg3
    mov ecx,arg2*65536+arg4
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

macro label arg1,arg2,arg3,arg4
{
local asd,lab
    jmp asd
lab db  arg3                 ;arg label
asd:
    mov ebx,arg1             ;arg1=y arg2=x
    shl ebx,16
    add ebx,arg2
    mov ecx,arg4             ;arg4 color
    mov edx,lab
    mov esi,asd-lab          ;calc size
    mov eax,4
    mcall
}

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
window_Skinned equ 0x04000000
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
