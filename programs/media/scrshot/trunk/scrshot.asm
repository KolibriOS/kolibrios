;===[includes]===;
include 'lang.inc'
include 'ascl.inc'
include 'macros.inc'
;===[header and etc.]===;
meos_app_start
;===[code:]===;
code
;---------------------------------------
;====== we want keyborad events ========
mov eax,40
mov ebx,00000000000000000000000000000010b
int 0x40

;====== wanna use keyboard scan codes ==
mov eax,66
mov ebx,1
mov ecx,1
int 0x40

;=======GET SCREEN SIZE=================

    mov  eax,14 		; get screen size
    int  0x40
    push eax
    and  eax,0x0000ffff
    inc  eax
    mov  [size_y],eax
    pop  eax
    shr  eax,16
    inc  eax
    mov  [size_x],eax

    mov  eax,[size_x]
    shr  eax,2
    mov  [cmp_ecx],eax

    mov  eax,[size_x]
    xor  edx,edx
    mov  ebx,3
    mul  ebx
    mov  [add_esi],eax

    mov  eax,[size_y]
    shr  eax,2
    mov  [cmp_edx],eax

    mov   eax,[size_y]
    imul  eax,[size_x]
    imul  eax,3
    mov   [i_size],eax

;=======================================

still:
mov eax,10
int 0x40
cmp eax,2
je key
jmp still

key:
mov eax,2
int 0x40
cmp ah,55 ;===print screen to hdd
je print_screen_hdd
cmp ah,84 ;===alt+prnt.screen = print screen to fdd
je print_screen_fdd
cmp ah,70 ;===lets make exit - scroll lock
je close_now
jmp still

close_now:
close
;==================================
;===save file to hdd===============
print_screen_hdd:
mov [savetofdd],0
call change_fname
call save_screen
jmp still
;===save file to fdd===============
print_screen_fdd:
mov [savetofdd],1
call change_fname
call save_screen
jmp still
;===change our file name ==========
change_fname:
cmp [bmp_name+7],'9'
jne addfname
cmp [bmp_name+6],'9'
je leavenow   ;if name is > than 'screen99.bmp' then we do not change name
mov [bmp_name+7],'0'
add [bmp_name+6],0x1
jmp leavenow
addfname:
add [bmp_name+7],0x1
leavenow:
ret
;==================================

;============SAVE SCREEN===========
read_pixel:
pushad

mov esi,eax
mov eax,[size_x]
mul ebx
add eax,esi

xchg eax,ebx
mov eax,35
int 0x40
mov [esp+28],eax

popad
ret

save_screen:

     pusha

; 1) READ SCREEN
     mov  edi,bmp_file_area+0x36 ;0x10036

     mov  eax,[size_y]
     dec  eax
     mov  [temp_y],eax

  ynew:

     xor  eax,eax
     mov  [temp_x],eax

  xnew:

     mov  eax,[temp_x]
     mov  ebx,[temp_y]
     call read_pixel

     mov  [edi],eax
     add  edi,3

     inc  [temp_x]

     mov  eax,[size_x]
     cmp  [temp_x],eax
     jb   xnew

     dec  [temp_y]

     cmp  [temp_y],0
     jge  ynew

; 2) BMP HEADER

     mov  [bmp_file_area],word 'BM'	; bmp signature
     mov  eax,[i_size]
     mov  [bmp_file_area+34],eax	; image size
     mov  ebx,0x36
     mov  [bmp_file_area+10],ebx	; headers size
     add  eax,ebx
     mov  [bmp_file_area+2],eax 	; file size
     mov  [bmp_file_area+14],dword 0x28
     mov  eax,[size_x]
     mov  [bmp_file_area+18],eax	; x size
     mov  eax,[size_y]
     mov  [bmp_file_area+22],eax	; y size
     mov  [bmp_file_area+26],word 1
     mov  [bmp_file_area+28],word 0x18	; bpp = 24 = 0x18

; 3) SAVE FILE
cmp [savetofdd],1
je save_to_fdd
     mov  eax,56
     mov  ebx,bmp_name
     mov  edx,bmp_file_area
     mov  ecx,[i_size]
     add  ecx,0x36
     mov  esi,path
     int  0x40
  popa
  ret

save_to_fdd:
mov eax,33;
mov ebx,bmp_name
mov ecx,bmp_file_area
mov edx,[i_size]
add edx,0x36
mov esi,0
int 0x40
  popa
  ret


;=======================================

;---------------------------------------
;===[DATA]===;
data
;---------------------------------------
bmp_name db 'SCREEN00BMP'
path db 0

i_size	 dd  0x1

m_x	 dd  100
m_y	 dd  100

cmp_ecx  dd  0
add_esi  dd  0
cmp_edx  dd  0

savetofdd db 0

;===[uninitialised data]===;
udata
;---------------------------------------

temp_x dd ?
temp_y dd ?

size_x dd ?
size_y dd ?

bmp_file_area:
rb 0x250000

;the happy end
meos_app_end