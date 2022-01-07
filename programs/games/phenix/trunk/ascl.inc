;lang equ ru ; ru en fr ge fi

;
;   Assembler
;     SMALL
;       CODE
;         Libary
;
;  Ver 0.17 (for FASM v1.54 or higer)
;

macro movt op1,op2
{
   if op2 eq
      ;display 'not set'
   else
      if op1 eq op2
         display '+'
         ;display 'skip mov ' # `op1 # ',' # `op2 , 10,13
      else
         mov op1,op2
      end if
   end if
}

macro clear_buffer buffer, buf_size, value
{
   movt edi,buffer    ; edi = buffer adress
   movt ecx,buf_size  ; ecx = buffer size
   movt al,value
   call clear_buffer_proc

if ~ defined clear_buffer_used
clear_buffer_used equ 1
   jmp @f
clear_buffer_proc:
   mov ah,al
   push ax
   shl eax,16
   pop ax            ; eax = al:al:al:al
   push ecx
   shr ecx,2         ; ecx = ecx/4 when we use stosd
   cld               ; set direction flag
   rep stosd
   pop ecx
   and ecx,011b      ; last two bits
   rep stosb
   ret
@@:
end if
}

; draw_button - function for draw button with label placed at center

macro draw_button button_id,x,y,xs,ys,text,button_color,text_color
{
local end_of_data,label
   jmp end_of_data
label db  text               ;arg label
end_of_data:
   words2reg ebx,x,xs        ; ebx = x * 65536 + xsize
   words2reg ecx,y,ys        ; ecx = y * 65536 + ysize
   mov edx,button_id
   mov esi,button_color
   push dword end_of_data-label
   push dword text_color
   push dword label
   call draw_button_proc
if ~ defined draw_button_used
draw_button_used equ 1
   jmp @f
draw_button_proc:
   mov eax,8     ; eax = 8 add button function
   mcall

   mov esi,[esp+12] ; esi = lenght of label
   mov eax,esi
   mov edx,6     ; 6 - width of one char
   mul edx       ; eax = size of label in pixels (chars number * 6)

   sub bx,ax
   shr bx,1
   mov edi,ebx
   shr edi,16
   add bx,di

   sub cx,7
   shr cx,1
   mov edi,ecx
   shr edi,16
   add cx,di

   shl ebx,16
   mov bx,cx     ; ebx = bx * 65536 + edx

   mov ecx,[esp+8] ; text color
   mov edx,[esp+4] ; set address of label
   mov eax,4       ; eax = 4 draw label function
   mcall
   ret 3*4
@@:
end if
}

; scan_event - Scan event without wait
macro scan_event redraw,key,button
{
   mov eax,11
   mcall
   dec eax
   jz  redraw
   dec eax
   jz  key
   dec eax
   jz  button
}

; wait_event - Wait for event
macro wait_event redraw,key,button
{
   mov eax,10
   mcall
   dec eax
   jz  redraw
   dec eax
   jz  key
   dec eax
   jz  button
}

; time_event - Wait for event with timeout
macro time_event xfps,noevent,redraw,key,button
{
   mov eax,23
   mov ebx,xfps
   mcall
   ;cmp eax,0
   ;je  noevent
   dec eax
   js  noevent
   jz  redraw
   dec eax
   jz  key
   dec eax
   jz  button
}

; function for set ascii or scan code keyboard input mode
macro keyboard_set_input_mode mode
{
   mov eax,66      ; eax = 66 - keyboard service function
   mov ebx,1       ; ebx = 1 - sub function for set input mode for keyboard
   mov ecx,mode    ; ecx = mode 0 - ASCII; 1 - scan codes;
   mcall
}

; function for read layout to 128 byte array
macro keyboard_get_layout layout_type, layout_array
{
   mov eax,26           ; eax = 26 - hardware service function
   mov ebx,2            ; ebx = 2 - subfunction for get keyboard layout
   mov ecx,layout_type  ; ecx = layout type: 1 - for normal layout
   mov edx,layout_array ; edx = address to 128 byte array
   mcall
}

; function for get key from keyboard buffer
macro window_get_key output
{
   mov  eax,2  ; eax = 2 - get keyboard pressed key function
   mcall
   if ~output eq
      mov output,eax
   end if
}

; function for get id of button pressed in window
macro window_get_button output
{
   mov  eax,17 ; eax = 17 - get id of pressed button function
   mcall
   if ~output eq
      mov output,eax
   end if
}

macro draw_pixel x,y,color
{
   mov ebx,x
   mov ecx,y
   mov edx,color
   mov eax,1
   mcall
}

macro puttxt x,y,offs,size,color
{
   words2reg ebx,x,y
   mov ecx,color
   mov edx,offs
   mov esi,size
   mov eax,4
   mcall
}

; window_begin_draw - Begin of draw window
macro window_begin_draw
{
   mov eax,12
   mov ebx,1
   mcall
}

; window_end_draw - End draw of window
macro window_end_draw
{
   mov eax,12
   mov ebx,2
   mcall
}

; fs_read_file - read file from file system and save it to array
; example:
; fs_read_file '/sys/clock.bmp',load_area,temp_area

macro fs_read_file file_name,file_load_area,file_temp_area
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
   shr  eax,9    ; div to 512 and add 1 - for get number of blocks
   inc  eax
   mov  dword [fileinfo+8],eax
   mov  eax,58
   mov  ebx,fileinfo
   mcall
}

; random - generate random count (small)
; example:
; random 10000,eax ; generate random value from 0 to 10000 and save it in eax
; note:
; max_value<65536
; only modificate eax register

randomuse = 0
macro random max_value,result
{
local rxproc
randomuse = randomuse + 1
   jmp rxproc

if defined randomuse & randomuse = 1
   randomproc:
      ;int3
      ;push bx
      ;push cx
      ;push dx
      ;push si
      ;push di
      pushad
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
      mov [rnd_result],dx
      popad
      mov eax,dword [rnd_result]
      ret
   ; data block after ret
   rsx1   dw 0x4321
   rsx2   dw 0x1234
   rnd_result dw 0
   dw 0
end if

rxproc:
   movt eax,max_value
   call randomproc
   movt result,eax ;dword [rnd_result]
}

; close - Close application

macro close
{
   mov eax,-1
   mcall
}

; delay - Create delay 1/100 sec
; example:
; delay 100   ;delay 2 sec 1/100*200=2 sec

macro delay arg1
{
   mov eax,5
   mov ebx,arg1
   mcall
}

; window - Draw window
; window Xstart,Ystart,'Text',Color
; example:
; window 10,10,640+8,480+24,window_Skinned

macro window arg1,arg2,arg3,arg4,arg5
{
   words2reg ebx,arg1,arg3
   words2reg ecx,arg2,arg4
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

;Key's scan codes

ErrorCode      equ   0 ; 0x00
key_Esc        equ   1 ; 0x01
key_1          equ   2 ; 0x02 !
key_2          equ   3 ; 0x03 @
key_3          equ   4 ; 0x04 #
key_4          equ   5 ; 0x05 $
key_5          equ   6 ; 0x06 %
key_6          equ   7 ; 0x07 "
key_7          equ   8 ; 0x08 &
key_8          equ   9 ; 0x09 *
key_9          equ  10 ; 0x0A (
key_0          equ  11 ; 0x0B )

key_Backspace  equ  14 ; 0x0E
key_Q          equ  16 ; 0x10
key_W          equ  17 ; 0x11
key_E          equ  18 ; 0x12
key_R          equ  19 ; 0x13
key_T          equ  20 ; 0x14
key_Y          equ  21 ; 0x15
key_U          equ  22 ; 0x16
key_I          equ  23 ; 0x17
key_O          equ  24 ; 0x18
key_P          equ  25 ; 0x19

key_Enter     equ 28   ; 0x1C
key_A         equ 30   ; 0x1E
key_S         equ 31   ; 0x1F
key_D         equ 32   ; 0x20
key_F         equ 33   ; 0x21
key_G         equ 34   ; 0x22
key_H         equ 35   ; 0x23
key_J         equ 36   ; 0x24
key_K         equ 37   ; 0x25
key_L         equ 38   ; 0x26
key_Colon     equ 39   ; 0x27 ':;'
key_DQuotes   equ 0x28 ; 40 '"''
key_Tilda     equ 0x29 ; 41 '~`'
key_LShift    equ 0x2A ; 42
key_Z         equ 0x2C ; 44
key_X         equ 0x2D ; 45
key_C         equ 0x2E ; 46
key_V         equ 0x2F ; 47
key_B         equ 0x30 ; 48
key_N         equ 0x31 ; 49
key_M         equ 0x32 ; 50
key_Comma     equ 0x33 ; 51 ',<'
key_Dot       equ 0x34 ; 52 '.>'
key_Question  equ 0x35 ; 53 '?/'
key_RShift    equ 0x36 ; 54
key_NumStar   equ 0x37 ; 55 '*'
key_LAlt      equ 0x38 ; 56
key_Space     equ 0x39 ; 57
key_CapsLock  equ 0x3A ; 58
key_F1        equ 0x3B ; 59
key_F2        equ 0x3C ; 60
key_F3        equ 0x3D ; 61
key_F4        equ 62
key_F5        equ 63
key_F6        equ 64
key_F7        equ 65
key_F8        equ 66
key_F9        equ 67
key_F10       equ 0x44 ; 68
key_NumLock   equ 0x45 ; 69
key_ScrollLock equ 0x46 ; 70
key_Up        equ 0x48 ; 72
key_Num8      equ 0x48 ; 72
key_PageUp    equ 0x49 ; 73
key_Num9      equ 0x49 ; 73
key_NumMinus  equ 0x4A ; 74
key_Left      equ 0x4B ; 75
key_Num4      equ 0x4B ; 75
key_Num5      equ 0x4C ; 76
key_Right     equ 0x4D ; 77
key_Down      equ 0x50 ; 80
key_Num2      equ 0x50 ; 80
key_F11       equ 0x57 ; 87
key_F12       equ 0x58 ; 88
key_PA1       equ  90 ; 0x5A
key_Lwin      equ  91 ; 0x5B
key_F13       equ  91 ; 0x5B
key_Rwin      equ  92 ; 0x5C
key_F14       equ  92 ; 0x5C
key_Menu      equ  93 ; 0x5D
key_F15       equ  93 ; 0x5D
key_F16       equ  99 ; 0x63
key_F17       equ 100 ; 0x64
key_F18       equ 101 ; 0x65
key_F19       equ 102 ; 0x66
key_F20       equ 103 ; 0x67
key_F21       equ 104 ; 0x68
key_F22       equ 105 ; 0x69
key_F23       equ 106 ; 0x6A
key_F24       equ 107 ; 0x6B
key_EraseEOF  equ 109 ; 0x6D
key_CopyPlay  equ 111 ; 0x6F
key_CrSel     equ 114 ; 0x72
key_Delta     equ 115 ; 0x73
key_ExSel     equ 116 ; 0x74
key_Clear     equ 118 ; 0x76



;Attributes

;Window Attributes
window_Skinned equ 0x04000000
window_Type2   equ 0x02000000
window_Type1   equ 0x00000000
window_Reserve equ 0x01000000

;Label Attributes
ends_by_zero     equ 10000000b shl 24
fill_background  equ 01000000b shl 24
cp866_small      equ 00000000b shl 24
cp866            equ 00010000b shl 24
utf16            equ 00100000b shl 24
utf8             equ 00110000b shl 24
font_size_x1     equ 00000000b shl 24
font_size_x2     equ 00000001b shl 24
font_size_x3     equ 00000010b shl 24
font_size_x4     equ 00000011b shl 24
font_size_x5     equ 00000100b shl 24
font_size_x6     equ 00000101b shl 24
font_size_x7     equ 00000110b shl 24
font_size_x8     equ 00000111b shl 24

;Colors
cl_White  equ 0x00ffffff
cl_Black  equ 0x00000000
cl_Grey   equ 0x00888888
cl_Red    equ 0x00ff0000
cl_Lime   equ 0x0000ff00
cl_Blue   equ 0x000000ff
cl_Yellow equ 0x00ffff00
cl_Cyan   equ 0x0000ffff
cl_Green  equ 0x0000af00
cl_Blue   equ 0x000000ff
cl_Purple equ 0x008080ff
cl_Violet equ 0x008040ff
;cl_Cyan   equ 0x0040e0ff

macro wordstoreg reg,hiword,loword
{
if  hiword eqtype 0 & loword eqtype 0
    mov  reg,dword (hiword)*65536+(loword)
else if hiword eqtype 12 & loword eqtype eax
    mov  reg,dword (hiword)*65536
    add  reg,dword loword
else if hiword eqtype 12 & loword eqtype [123]
    mov  reg,dword (hiword)*65536
    add  reg,dword loword
else
    mov  reg,dword hiword
    shl  reg,16
    add  reg,dword loword
end if
}

macro dword2reg reg,doubleword
{
if doubleword eq
   ; not changes
else
   mov reg,dword doubleword
end if
}

macro words2reg reg,hiword,lowword
{
if hiword eq
   if lowword eq
      ; if operators empty nothing do
   else
      if lowword eqtype 12/4
         and reg,dword 0xffff0000
         add reg,dword lowword
      else
         and reg,dword 0xffff0000
         add reg,dword lowword
      end if
   end if
else
   if lowword eq
      if hiword eqtype 12/4
         and reg,dword 0x0000ffff
         add reg,dword hiword*65536
      else
         shl reg,16
         add reg,dword hiword
         ror reg,16
      end if
   else
      if lowword eqtype 12/4 & hiword eqtype 12/4
         if lowword = 0 & hiword = 0
            xor reg,reg
         else
            ; use brackets for calcualtion without error!!!
            mov reg,dword (hiword)*65536+(lowword)
         end if
      else
         mov reg,dword hiword
         shl reg,16
         add reg,dword lowword
      end if
   end if
end if
}
