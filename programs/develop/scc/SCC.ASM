;
;   Port to KolibliOS
;   (jacekm.pl@gmail.com)
;

;B+ System header
use32
org    0x0
  db	 'MENUET01'
  dd	 0x01
  dd	 START
  dd	 I_END
  dd	 0x500000
  dd	 0x7fff0
  dd	 0x0,0x0
;E:.

;B+ Include C files
  ; Compiler tools
  include "INTRINS.ASM"

  ; C Library ("stdio.asm")
  include "LIBSTD.ASM"

  ; MenuetOS implement
  include "OSFUNC.ASM"

_fopen	equ _OS_fopen
_fputc	equ _OS_fputc
_fgetc	equ _OS_fgetc
_fclose equ _OS_fclos

  ;Main program (compiled from C)
  include "GETARG.ASM"
  include "CC1.ASM"
  include "CC2.ASM"
  include "CC3.ASM"
  include "CC4.ASM"
;E:.

START:
;B+ Main cycle
  call draw_window
still:
  mov  eax,10
  int  0x40
still1:

  cmp  eax,1
  je   red
  cmp  eax,2
  je   key
  cmp  eax,3
  je   button
  jmp  still

red:
  call draw_window
  jmp  still

key:
  mov  eax,2
  int  0x40
  jmp  still

button:
  mov  eax,17
  int  0x40

  cmp  ah,1
  jne  .noclose
  mov  eax,-1
  int  0x40
.noclose:

  cmp  ah,2
  jne  .no_run
;  call begin_osfunc
;B+ Copy file path
  mov  esi,inp_path
  mov  edi,filepath
  mov  ecx,[p_len]
 push ecx
  cld
rep movsb
 pop  ecx
  add  ecx,filepath
  mov  [p_filename],ecx
;E:.
;B+ Clear word area
  mov  ebx,10 shl 16 + 280
  mov  ecx,60 shl 16 + 250
  mov  edx,0x303030
  mov  eax,13
  int  0x40
;E:.
;B+ Init parameters
  mov  ecx,_input
  sub  ecx,_nogo
  mov  edi,_nogo
  xor  eax,eax
  cld
rep stosb
  mov  [_input],-1
  mov  [_input2],-1
  mov  [_usexpr],1
  mov  [_ccode],1
  mov  [_quote],34
  mov  [test_outch.y_coord],60
;E:.
  call init_osfunc
 push dword 4
 push args
;B+ Save exit ESP
  mov  [exit_esp],esp
;E:.
  call _main
  add  esp,8
;  call end_osfunc
.no_run:

  cmp  ah,3
  jne  .no_path
  mov  eax,inp_path
  mov  bx,27
  call read_string
  mov  edx,[r_pos]
  mov  [p_len],edx
  ;
  cmp  ah,10
  je   still
  jmp  still1
.no_path:

  cmp  ah,4
  jne  .no_file
  mov  eax,inp_file
  mov  bx,42
  call read_string
  mov  edx,[r_pos]
  mov  [f_len],edx
  ;
  cmp  ah,10
  je   still
  jmp  still1
.no_file:

  jmp  still
;E:.

read_string:
;B+ Read string
  mov  [c_place],eax
  mov  [r_pos],0
  mov  [rel_x],60
  mov  [rel_y],bx

  mov  ebx,60
  shl  ebx,16
  mov  bx,6*26+1
  movzx ecx,[rel_y]
  shl  ecx,16
  mov  cx,10
  mov  edx,0x000000
  mov  eax,13
  int  0x40

  add  ecx,11 shl 16
  mov  cx,2
  mov  edx,0x999999
  int  0x40

.next:
  mov  ebx,[rel_x]
  shl  ebx,16
  mov  bx,7
  movzx ecx,[rel_y]
  shl  ecx,16
  mov  cx,10
  mov  edx,0x000000
  mov  eax,13
  int  0x40

  mov  ebx,[c_place]
  mov  byte [ebx],0

  mov  eax,10
  int  0x40
  cmp  eax,2
  je   .key
  jmp  .to_ret
.key:

  mov  eax,2
  int  0x40
  cmp  ah,13
  jne  .no_enter
  jmp  .to_ret
.no_enter:

  cmp  ah,8
  jne  .no_back
  cmp  [r_pos],0
  je   .next
  dec  [r_pos]
  dec  [c_place]
  sub  [rel_x],6
  jmp  .next
.no_back:

  cmp  [r_pos],25
  je   .next

  cmp  ah,'a'
  jb   .no_sm_l
  cmp  ah,'z'
  ja   .no_sm_l
  add  ah,'A'-'a'
.no_sm_l:

  mov  byte [ebx],ah
  mov  [char],ah
  inc  [c_place]
  inc  [r_pos]

  mov  ebx,[rel_x]
  inc  ebx
  shl  ebx,16
  mov  bx,[rel_y]
  inc  bx
  mov  ecx,0x00ffff
  mov  edx,char
  mov  esi,1
  mov  eax,4
  int  0x40

  add  [rel_x],6
  jmp  .next

.to_ret:
 push eax
  mov  ebx,60
  shl  ebx,16
  mov  bx,6*26+1
  movzx ecx,[rel_y]
  add  ecx,11
  shl  ecx,16
  mov  cx,2
  mov  edx,0x000000
  mov  eax,13
  int  0x40
 pop  eax
  ret

rel_x dd 60
rel_y dw 0x0
c_place dd 0x0
r_pos dd 0x0
char db '?'
;E:.

draw_window:
;B+ Draw window
  mov  eax,12
  mov  ebx,1
  int  0x40

  mov  ebx,100*65536+320
  mov  ecx,100*65536+320
  mov  edx,0x035555DD
  mov  esi,0x805080d0
  mov  edi,0x005080d0
  mov  eax,0
  int  0x40

  mov  ebx,8*65536+8
  mov  ecx,0x10ddeeff
  mov  edx,caption
  mov  esi,caption_end-caption
  mov  eax,4
  int  0x40

  ;mov  ebx,(300-19)*65536+12
  ;mov  ecx,5*65536+12
  ;mov  edx,1
  ;mov  esi,0x6688dd
  ;mov  eax,8
  ;int  0x40

;B+ Hot buttons
  mov  ebx,220 shl 16 + 70
  mov  ecx,25 shl 16 + 29
  mov  edx,2 ; compile
  mov  esi,0x6688dd
  mov  eax,8
  int  0x40

  mov  ebx,10 shl 16 + 47
  mov  ecx,26 shl 16 + 12
  mov  edx,3 ; path
  mov  esi,0x6688dd
  mov  eax,8
  int  0x40

  mov  ebx,10 shl 16 + 47
  mov  ecx,41 shl 16 + 12
  mov  edx,4 ; file name
  mov  esi,0x6688dd
  mov  eax,8
  int  0x40
;E:.

;B+ Text in buttons
  mov  ebx,231 shl 16 + 36
  mov  ecx,0x10ffffff
  mov  edx,txt_compile
  mov  esi,txt_compile_end-txt_compile
  mov  eax,4
  int  0x40

  mov  ebx,14 shl 16 + 28
  mov  ecx,0xffffff
  mov  edx,txt_path
  mov  esi,txt_path_end-txt_path
  int  0x40

  mov  ebx,14 shl 16 + 43
  mov  edx,txt_file
  mov  esi,txt_file_end-txt_file
  int  0x40
;E:.

;B+ Draw path / file name
  mov  ebx,61 shl 16 + 28
  mov  ecx,0x00ffff
  mov  edx,inp_path
  mov  esi,[p_len]
  mov  eax,4
  int  0x40

  mov  ebx,61 shl 16 + 43
  mov  ecx,0x00ffff
  mov  edx,inp_file
  mov  esi,[f_len]
  mov  eax,4
  int  0x40
;E:.

  mov  eax,12
  mov  ebx,2
  int  0x40

  ret
;E:.

;B+ Data area

;B+ Interface
inp_path: times 100 db '/SYS/',0
p_len dd 5
inp_file: times 100 db 'EXAMPLE',0
f_len dd 7

txt_compile db 'Compile'
txt_compile_end:
txt_path    db 'Path ->'
txt_path_end:
txt_file    db 'File ->'
txt_file_end:
;E:.

;B+ Main arguments
args dd arg_str0,inp_file,arg_str2,arg_str3,arg_str4

arg_str0 db 'CC',0
arg_str1 db 'prog',0
arg_str2 db '-m',0
arg_str3 db '-a',0
arg_str4 db '-p',0
;E:.

;B+ System parameters
exit_esp dd 0x0
;E:.

caption db  'Small C compiler for KolibriOS v 0.5.3'
caption_end:

times 8-($ mod 8) db 0
;E:.

I_END:
