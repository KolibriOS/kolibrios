; CODE VIEWER - Compile with FASM for Menuet
   
;B+ System header
use32
org 0x0
 db 'MENUET00'
 dd 38
 dd START
 dd I_END
 dd 0x100000
 dd 0x00000000

include 'lang.inc'
include 'macros.inc'
;E:.
   
;B+ Definitions
type_begin equ 0
type_end_normal equ 2
type_not_end equ 4
type_end_plus equ 6
type_include equ 8
type_file_end equ 11
; +1, if active jump
   
char_end equ 11
new_line equ 10
   
win_width equ (496+6*6)
win_field equ (4+10+6*6)
win_toptext equ 38+4
;B+ Keys
KEY_UP equ (130+48)
KEY_DOWN equ (129+48)
KEY_PGUP equ (136+48)
KEY_PGDOWN equ (135+48)
;E:.
start_data equ (I_END+10+27+16)
;E:.
   
;B+ Execution
START:
  jmp  load_file1
;B+ Main cicle
redr:
  call draw_window
still:
  mov  eax,10
  int  0x40
still_:
  cmp  eax,1
  je   redr
  cmp  eax,2
  jne  no_key
  mov  eax,2
  int  0x40
  jmp  key
no_key:
  cmp  eax,3
  jne  still
  mov  eax,17
  int  0x40
  cmp  ah,1
  jne  .no_close
  mov  eax,-1
  int  0x40
.no_close:
  jmp  button
  jmp  still
;E:.
key:
;B+ Scroll text
  mov  ebx,[top_pos]
  mov  [activ_pos],ebx
  mov  bx,[top_line]
  mov  [activ_line],bx
  mov  bx,[top_depth]
  mov  [activ_depth],bx
  mov  ebx,[top_file]
  mov  [activ_file],ebx
  cmp  ah,KEY_DOWN     ;key_up
  jne  .no_down
  call line_down
  jmp  .save_shift
.no_down:
  cmp  ah,KEY_UP       ;key down
  jne  .no_up
  call line_up
  jmp  .save_shift
.no_up:
  cmp  ah,KEY_PGUP     ;key page up
  jne  .no_pgup
  mov  edi,35
.next_line:
  call line_up
  dec  edi
  jnz  .next_line
  jmp  .save_shift
.no_pgup:
  cmp  ah,KEY_PGDOWN   ;key page down
  jne  .no_pgdown
  mov  edi,35
.next_line1:
  call line_down
  dec  edi
  jnz  .next_line1
  jmp  .save_shift
.no_pgdown:
  jmp  still
.save_shift:
  mov  ebx,[activ_pos]
  mov  [top_pos],ebx
  mov  bx,[activ_line]
  mov  [top_line],bx
  mov  bx,[activ_depth]
  mov  [top_depth],bx
  mov  ebx,[activ_file]
  mov  [top_file],ebx
  ; Show new text
  mov  ebx,1
  mov  eax,12
  int  0x40
  call show_text
  mov  ebx,2
  mov  eax,12
  int  0x40
  jmp  still
;E:.
button:
;B+ Distribute button events
  shr  eax,8
  and  eax,0xff
  cmp  eax,100
  jge  down_buttons
;B+ Left buttons
;B+ Find line place
  sub  eax,2
  mov  ebx,[top_pos]
  mov  [activ_pos],ebx
  mov  bx,[top_depth]
  mov  [activ_depth],bx
  mov  ebx,[top_file]
  mov  [activ_file],ebx
  mov  bx,[top_line]
  mov  [activ_line],bx
  mov  ecx,eax
  or   ecx,ecx
  jz   .line_find
.next_line:
 push ecx
  call line_down
 pop  ecx
  loop .next_line
.line_find:
  mov  ebx,[activ_pos]
;E:.
;B+ Switch, if special line
  mov  cx,[ebx]
  test cx,not 15
  jnz  still
  cmp  cx,type_begin
  je   .is_begin_end
  cmp  cx,type_begin+1
  jne  .not_begin_end
.is_begin_end:
  xor  [ebx],word 1
  mov  ebx,[ebx+4]
  add  ebx,[activ_file]
  xor  [ebx],word 1
  jmp  .paint
.not_begin_end:
  cmp  cx,type_include
  je   .open_file
  ;close file
  cmp  cx,type_include+1
  jne  .not_include
  xor  [ebx],word 1
  jmp  .paint
.open_file:
  ;Open file
  cmp  dword [ebx+4],-1
  je   .load_file
  xor  [ebx],word 1
  jmp  .paint
.paint1:
 pop  ebx
  jmp  .paint
.load_file:
 push ebx
;B+ Prepare file name
  mov  ecx,26
  mov  edi,incl_filename
  mov  al,' '
  cld
rep  stosb
  mov  cl,[skoba_txt]
  mov  ch,[skoba_txt+1]
  add  ebx,12
.next_char:
  cmp  [ebx],cl
  je   .begin
  cmp  byte [ebx],new_line
  je   .paint1
  inc  ebx
  jmp  .next_char
.begin:
  inc  ebx
  mov  esi,ebx
.next_char1:
  cmp  [ebx],ch
  je   .end
  cmp  byte [ebx],new_line
  je   .paint1
  inc  ebx
  jmp  .next_char1
.end:
  sub  ebx,esi
  mov  edi,incl_filename
  mov  ecx,ebx
 push ecx
rep movsb
;  mov  al,0
;  stosb
   
  ;Save in memory
  mov  ecx,[esp]
  mov  esi,incl_filename
  mov  edi,[end_of_file]
rep movsb
 pop  ecx
  mov  [edi],ecx
  add  ecx,2
  add  [end_of_file],ecx
   
  mov  ebx,[esp]
;E:.
  mov  edi,[top_free_file]
  mov  [ebx+4],edi
   
  mov  [activ_pos],ebx
  mov  eax,[activ_file]
  sub  ebx,eax
  mov  [incl_line],ebx
 push eax
  call line_down
 pop  eax
  mov  ebx,[activ_pos]
  sub  ebx,eax
  mov  [incl_next],ebx
 pop  ebx
  xor  [ebx],word 1
 push  eax
  mov  eax,[end_of_file]
  add  eax,16
  mov  [activ_file],eax
  lea  edi,[file_array+4*edi]
  mov  [edi],eax
  mov  [activ_pos],eax
   
  mov  ebx,[incl_line]
  mov  [eax-12],ebx
  mov  bx,[activ_line]
   dec  bx
   mov  word [eax-6],bx
;B+ Save number for new file
  mov  ebx,[esp]
  cmp  ebx,[file_array]
  jne  .no_root
  mov  [eax-8],word 0
  jmp  .parent_ok
.no_root:
  mov  ebx,[esp]
  mov  bl,[ebx-2]
  and  ebx,0xff
  mov  [eax-8],bx
.parent_ok:
 push eax
;E:.
  call add_file
  mov  edi,[block_pointer]
  mov  word [edi+4],char_end
  add  [block_pointer],8
 push edi
  call fresh_file
 pop  edi eax
  mov  [block_pointer],edi
   
;B+ Save newfile line count
  mov  edi,[line_]
  mov  word [eax-4],di
;E:.
 pop  eax
   
  mov  [activ_file],eax
  inc  [top_free_file]
  jmp  .paint
.not_include:
   
.paint:
  mov  ebx,1
  mov  eax,12
  int  0x40
  call show_text
  mov  ebx,2
  mov  eax,12
  int  0x40
;E:.
  jmp  still
;E:.
down_buttons:
;B+ If down buttons
  cmp  eax,101
  je   load_file   ;Button 'Load'
  sub  eax,100
  mov  edx,[str_table+4*eax]
  mov  [str_start],edx
;B+ Clear old info
 push edx
  ;clear text
  mov  ecx,0xaa0000
  call print_str
 pop  edx
  ;clear memory place
  mov  ecx,[edx]
  lea  edi,[edx+8]
  mov  eax,'    '
  cld
rep stosb
;E:.
  mov  ebx,[edx]
  mov  [max_len],ebx
  mov  ebx,[edx+4]
  jmp  load_str
save_str:
;B+ Save in base place
;B+ If file convert to upper case
  mov  eax,it1
  cmp  eax,[str_start]
  jne  .no_file
  mov  ecx,0xaa0000
  mov  edx,str_
  mov  esi,[it1]
  mov  ebx,[it1+4]
  mov  eax,4
  int  0x40
  mov  edx,edi
  mov  eax,str_
  dec  eax
  inc  edx
.next_char:
  inc  eax
  dec  edx
  jz   .no_file
  cmp  byte [eax],'a'
  jl   .next_char
  cmp  byte [eax],'z'
  jg   .next_char
  add  byte [eax],'A'-'a'
  jmp  .next_char
.no_file:
;E:.
  mov  edx,[str_start]
  add  edx,8
  mov  ecx,edi
  add  edi,edx
  mov  byte [edi],char_end
  mov  eax,it1
  cmp  eax,[str_start]
  jne  .no_null
  mov  byte [edi],' ' ;0
.no_null:
  mov  esi,str_
  mov  edi,edx
  cld
rep movsb
  mov  ecx,0xffffff
  jmp  print_str
;E:.
;E:.
;E:.
;B+ String tools
;B+ Data for load string
curs db '_'
str_: times 100 db ' '
           db char_end
max_len dd 10
;E:.
load_str:
;B+ Load text field
  xor  edi,edi
  mov  ecx,0xffaaaa
  mov  esi,1
.next_char:
  mov  edx,curs
  mov  eax,4
  int  0x40
;B+ Get key event
  mov  eax,10
  int  0x40
 push eax
  mov  ecx,0xaa0000
  mov  eax,4
  int  0x40
 pop  eax
  cmp  eax,2
  je   .yes_key
  call save_str
  jmp  still_
.yes_key:
  mov  eax,2
  int  0x40
;E:.
;B+ Test enter
  cmp  ah,13
  jne  .no_ok
  call save_str
  jmp  still
.no_ok:
;E:.
;B+ Test backspace
  cmp  ah,8
  jne  .no_backsp
  or   edi,edi
  jz   .next_char
  mov  byte [str_+edi],' '
  mov  ecx,0xaa0000
  mov  eax,4
  int  0x40
  dec  edi
  sub  ebx,6*65536
  lea  edx,[str_+edi]
  int  0x40
  mov  ecx,0xffaaaa
  jmp  .next_char
 .no_backsp:
;E:.
;B+ Prin 1 char
  mov  [str_+edi],ah
  mov  ecx,0xaa0000
  mov  eax,4
  int  0x40
  mov  ecx,0xffaaaa
  lea  edx,[str_+edi]
  cmp  [max_len],edi
  je   .next_char
  int  0x40
  add  ebx,6*65536
  inc  edi
;E:.
  jmp  .next_char
;E:.
print_str:
;B+ Print select string
  mov  ebx,[str_start]
  lea  edx,[ebx+8]
  mov  esi,[ebx]
  mov  ebx,[ebx+4]
  mov  eax,4
  int  0x40
;B+ Test special strings
 pusha
  mov  eax,[str_start]
  cmp  eax,it2
  jge  .is_ok1
 popa
  ret
.is_ok1:
  cmp  eax,it3
  jle  .is_ok
 popa
  ret
.is_ok:
;E:.
  add  eax,8
.next_char:
  mov  esi,1
  cmp  byte [eax],' '
  jne  .no_space
;B+ Draw special space
 push eax
  mov  edx,space
  mov  eax,4
  int  0x40
 push  ebx
  sub  ebx,1*65536
  mov  edx,dot
  int  0x40
  add  ebx,3*65536
  int  0x40
 pop  ebx
 pop  eax
;E:.
.no_space:
  add  ebx,6*65536
  cmp  byte [eax],char_end
  jne  .no_ret
 popa
  ret
.no_ret:
  inc  eax
  jmp  .next_char
   
space db '_'
dot db '.'
;E:.
;E:.
;B+ Add / remove files
add_file:
;B+ Load and link file
  mov  eax,[activ_file]
 push  eax
  mov  ebx,incl_filename
  mov  ecx,0
  mov  edx,-1
  mov  esi,eax
  mov  eax,6
  int  0x40
   
  mov  ebx,[esp]
  inc  eax
  mov  [ebx-16],eax
  dec  eax
  add  ebx,eax
  add  eax,16+15           +20 ;???
  add  [end_of_file],eax
  mov  byte [ebx],new_line
  mov  word [ebx+1],char_end
  mov  ax,[activ_line]
  mov  word [ebx+3],ax
  mov  eax,[incl_next]
  mov  [ebx+5],eax
  mov  dword [ebx+9],new_line
  mov  byte [ebx+13],new_line
 pop  ebx
  mov  eax,[top_free_file]
  mov  byte [ebx-2],al ; this file num
  mov  byte [ebx-1],new_line
  ret
;E:.
;B+ Include file data
incl_filename db 'KERNEL.ASM'
if_e:
times (26+incl_filename-if_e) db ' '
incl_line dd 0x0
incl_next dd 0x0
;E:.
;E:.
;E:.
   
;B+ Visualization tools
draw_window:
;B+ Redraw window
  mov  ebx,1
  mov  eax,12
  int  0x40
;B+ Draw window
  mov  ebx,((640-win_width)/2)*65536+win_width
  mov  ecx,10*65536+win_toptext+35*10+1+2*16
  mov  edx,[color_depth]
  or   edx,0x03000000
  mov  esi,0x80aaaaff
  mov  edi,0x00009000
  mov  eax,0x0
  int  0x40
;E:.
;B+ Draw caption
  mov  ebx,8*65537
  mov  ecx,0xffffff
  mov  edx,caption
  mov  esi,caption_end-caption
  mov  eax,4
  int  0x40
;E:.
;B+ Draw first line
  mov  ebx,5*65536+win_width-9
  mov  ecx,25*65536+win_toptext-22-4
  mov  edx,0xaa0000
  mov  eax,13
  int  0x40
  mov  ebx,21*65536+29
  mov  ecx,0xffffff
  mov  edx,line1up1
  mov  esi,line1up1_end-line1up1
  mov  eax,4
  int  0x40
  mov  ebx,(win_field+6)*65536+29
  mov  edx,line1up2
  mov  esi,line1up2_end-line1up2
  int  0x40
;E:.
;B+ Main text zone
;B+ Fill text
  ;Clear type lines
  mov  edi,lines_view
  mov  ecx,35
  mov  eax,0
  cld
repe stosd
  call show_text
;E:.
;B+ Define left buttons
;  mov  ebx,5*65536+9
;  mov  ecx,win_toptext*65536+9
;  mov  edx,0
;  mov  esi,0x9000a0
;  mov  eax,8
;  mov  edi,35
;.new_button:
;  int  0x40
;  add  ecx,10*65536
;  inc  edx
;  dec  edi
;  jnz  .new_button
;E:.
;B+ Vertical line
  mov  ebx,(win_field-1)*65537
  mov  ecx,24*65536+win_toptext+35*10
  mov  edx,0xffffff
  mov  eax,38
  int  0x40
;E:.
;E:.
;B+ Down controle zone
  mov  ebx,5*65536+win_width-9
  mov  ecx,(35*10+win_toptext+1)*65536+28
  mov  edx,0xaa0000
  mov  eax,13
  int  0x40
   
  mov  eax,line1down
  mov  ebx,filetxt
  mov  ecx,filetxt_end
  call ins_button_prep
  mov  edx,100
  mov  eax,8
  int  0x40
  mov  eax,line1down
  mov  ebx,loadtxt
  mov  ecx,loadtxt_end
  call ins_button_prep
  inc  edx
  mov  eax,8
  int  0x40
  mov  eax,line1down
  mov  ebx,begintxt
  mov  ecx,begintxt_end
  call ins_button_prep
  inc  edx
  mov  eax,8
  int  0x40
  mov  eax,line1down
  mov  ebx,endtxt
  mov  ecx,endtxt_end
  call ins_button_prep
  inc  edx
  mov  eax,8
  int  0x40
  mov  eax,line2down
  mov  ebx,inctxt
  mov  ecx,inctxt_end
  call ins_button_prep
  add  ecx,14*65536
  inc  edx
  mov  eax,8
  int  0x40
  mov  eax,line2down
  mov  ebx,septxt
  mov  ecx,septxt_end
  call ins_button_prep
  add  ecx,14*65536
  inc  edx
  mov  eax,8
  int  0x40
   
  mov  ebx,22*65536+35*10+win_toptext+4
  mov  ecx,0xffffff
  mov  edx,line1down
  mov  esi,line1down_end-line1down
  mov  eax,4
  int  0x40
  add  ebx,14
  mov  edx,line2down
  mov  esi,line2down_end-line2down
  int  0x40
;E:.
;B+ Down controle strings
  mov  ecx,0xffffff
  mov  eax,it1
  mov  [str_start],eax
  call print_str
  mov  eax,it2
  mov  [str_start],eax
  call print_str
  mov  eax,it3
  mov  [str_start],eax
  call print_str
  mov  eax,it4
  mov  [str_start],eax
  call print_str
  mov  eax,it5
  mov  [str_start],eax
  call print_str
;E:.
  mov  ebx,2
  mov  eax,12
  int  0x40
  ret
;E:.
ins_button_prep:
;B+ Insert button
 push edx
 push eax
  sub  ecx,ebx
  mov  eax,6
  mul  ecx
  add  eax,6
 push ax
  mov  eax,[esp+2]
  sub  ebx,eax
  mov  eax,6
  mul  ebx
  add  eax,18
  xchg eax,ebx
  shl  ebx,16
 pop  bx
  mov  ecx,(35*10+win_toptext+1)*65536+13
  mov  esi,0x00a050
 pop  eax
 pop  edx
  ret
;E:.
show_text:
;B+ Show text
;B+ Show file on top
  mov  ebx,(win_field+45)*65536+win_width-(win_field+45+8)
  mov  ecx,25*65536+win_toptext-22-4
  mov  edx,0xaa0000
  mov  eax,13
  int  0x40
   
  mov  edx,[top_file]
  xor  esi,esi
  mov  si,[edx-18]
  sub  edx,18
  sub  edx,esi
  mov  ebx,(win_field+45)*65536+29
  mov  ecx,0xffaaaa
  mov  eax,4
  int  0x40
;E:.
  mov  ax,[top_line]
  mov  [activ_line],ax
  mov  ebx,[top_pos]
  mov  [activ_pos],ebx
  mov  ax,[top_depth]
  mov  [activ_depth],ax
  mov  eax,[top_file]
  mov  [activ_file],eax
  mov  ecx,35
  mov  [line_],0
.next_line:
  cmp  [activ_depth],0
  jne  .next
  cmp  byte [ebx],char_end
  je   .end_of_file
.next:
 push ecx
  call show_line
  call line_down
  inc  [line_]
 pop  ecx
  loop .next_line
  ret
.end_of_file:
 push ecx
  call clear_line
   
  inc  [activ_pos]
  inc  [line_]
 pop  ecx
  loop .end_of_file
  ret
;E:.
;B+ Button chars
leftchars db '-','+',' ','H'
          db '?', 0 ,'!','H'
          db '&','x',' ', 0
;E:.
show_line:
;B+ Show line
  call clear_line
  mov  ebx,ecx
  shr  ebx,16
  inc  ebx
  or   ebx,(win_field+6)*65536
 push ebx
 push eax
;B+ Draw left button char
  mov  eax,[activ_pos]
  mov  ax,[eax]
  test ax,not 15
  jnz  .no_text
  and  eax,0xff
  lea  edx,[leftchars+eax]
  and  ebx,0xffff
  or   ebx,8*65536
  mov  ecx,0xffffff
  mov  esi,1
  mov  eax,4
  int  0x40
  mov  ebx,[esp+4]
.no_text:
;E:.
;B+ Draw line number
  xor  ecx,ecx
  mov  cx,[activ_line]
  mov  edx,(10+4+4)*65536
  mov  dx,bx
  mov  ebx,5*65536
  mov  esi,0xeeeeee
  mov  eax,47
  int  0x40
;E:.
;B+ Find line length
  xor  esi,esi
  mov  eax,[activ_pos]
  cmp  byte [eax],new_line
  je   .len_ok
  test word [eax],not 15
  jnz  .next_char
  add  eax,12 ;sckip system zone
.next_char:
  cmp  byte [eax],new_line
  je   .len_ok
  inc  esi
  inc  eax
  jmp  .next_char
.len_ok:
  mov  eax,6
  mul  esi
  mov  ecx,eax
  add  ecx,6
 pop  eax
  mov  [eax+2],cx
;E:.
  mov  ecx,[textcolor]
  mov  edx,[activ_pos]
  xor  edi,edi
  xor  ebx,ebx
  mov  bx,word [edx]
  cmp  bl,new_line
  je   .normal_show_line
  test bx,not 15
  jnz  .normal_show_line
  or   edi,ebx
  mov  ecx,[color_type+4*ebx]
  add  edx,12 ;sckip system zone
.normal_show_line:
 pop  ebx
  mov  eax,4
  int  0x40
  test edi,4 ;bad type close block
  jz   .nobad
  add  ebx,65536
  int  0x40
.nobad:
   ret
;E:.
color_type:
;B+ Color data
 dd 0x00dddd,0x00ee00 ;0/1 begin  00ee00
 dd 0x00b5b5,0xffa000 ;2/3 end    00d000
 dd 0xffffff,-1       ;4 text end
 dd 0x00b5b5,0xffa000 ;6/7 auto end
 dd 0x80ccff,0x80aaff ;8/9 include
 dd -1      ,0xffa000 ;11 file end
textcolor dd 0xffffff
;E:.
clear_line:
;B+ Clear text in line
;B+ Find line position
  mov  eax,[line_]
  mov  ecx,10
  mul  cx
  mov  ecx,eax
  add  ecx,win_toptext
;E:.
;B+ Draw/clear button
 push ecx
  shl  ecx,16
  mov  cx,9
  mov  ebx,5*65536+9
  mov  edx,[line_]
  add  edx,2
  mov  esi,0x9000a0
  mov  eax,8
  int  0x40
 pop  ecx
;E:.
;B+ Clear zone for line
  shl  ecx,16
  mov  cx,10
  mov  ebx,(5+11)*65536+win_field-5-12
  xor  edx,edx
  mov  dx,[activ_depth]
   mov  edx,[color_depth+4*edx]
   mov  eax,13
  int  0x40
;E:.
;B+ Clear main text zone
  mov  ebx,(win_field)*65536+(win_width-win_field-8)
  mov  eax,[line_]
  lea  eax,[lines_view+4*eax]
 push eax
  mov  ax,[eax]
  cmp  ax,[activ_depth]
  jne  .draw_all_line
  mov  eax,[esp]
  cmp  bx,[eax+2]
  jle  .draw_all_line
  mov  bx,[eax+2]
.draw_all_line:
  mov  eax,13
  int  0x40
;E:.
;B+ Update line type I
  pop  eax
  mov  bx,[activ_depth]
  mov  [eax],bx
;E:.
  ret
   
activ_depth dw 0x0
color_depth dd 0x404040,0x606060,0x707070,0x7a7a7a
            dd 0x878787,0x909090,0x9a9a9a,0xa5a5a5
            dd 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
lines_view: times 35 dd 0
;E:.
line_up:
;B+ Line up
;B+ Test jumps I
  mov  ebx,[activ_pos]
  cmp  ebx,[activ_file]
  jne  .yes_scroll
  cmp  [activ_depth],0
  jz   .top_of_file
  dec  [activ_depth]
  xor  eax,eax
  mov  ax,[ebx-8]
  mov  eax,[file_array+4*eax]
  mov  [activ_file],eax
  add  eax,[ebx-12]
  mov  [activ_pos],eax
  mov  ax,[ebx-6]
  mov  [activ_line],ax
.top_of_file:
  ret
;E:.
.yes_scroll:
  sub  ebx,2
.normal_line_up1:
  cmp  byte [ebx],new_line
  je   .line_ok
  dec  ebx
  jmp  .normal_line_up1
.line_ok:
;B+ Test for special line
  cmp  dword [ebx],new_line
  jne  .line_ok1
  sub  ebx,9
;E:.
.line_ok1:
  inc  ebx
  test word [ebx],not 15
  jnz  .normal_line_up
  test word [ebx],1
  jz   .normal_line_up
;B+ Test jumps II
  cmp  word [ebx],type_end_normal+1
  je   .to_begin
  cmp  word [ebx],type_end_plus+1
  je   .to_begin
  cmp  word [ebx],type_include+1
  je   .to_incl
  jmp  .normal_line_up
;E:.
.to_begin:
  mov  eax,[ebx+4]
  add  eax,[activ_file]
  mov  [activ_pos],eax
  mov  ax,[ebx+2]
  inc  ax
  sub  [activ_line],ax
  ret
.to_incl:
  inc  [activ_depth]
  mov  eax,[ebx+4]
  mov  eax,[file_array+4*eax]
  mov  [activ_file],eax
  mov  bx,[eax-4]
  mov  [activ_line],bx
  add  eax,[eax-16]
  mov  [activ_pos],eax
  ret
.normal_line_up:
  mov  [activ_pos],ebx
  dec  [activ_line]
  ret
;E:.
line_down:
;B+ Line down
  mov  ebx,[activ_pos]
  cmp  byte [ebx],char_end
  jne  .yes_scroll
  cmp  [activ_depth],0
  jne  .yes_scroll
  ret
.yes_scroll:
  cmp  byte [ebx],new_line
  je   .normal_line_down
  test word [ebx],not 15
  jnz  .normal_line_down
  test word [ebx],1
  jz   .not_activated
;B+ Test jumps
  cmp  word [ebx],type_begin+1
  jne  .no_begin
  mov  eax,[ebx+4]
  add  eax,[activ_file]
  mov  [activ_pos],eax
  mov  ax,[ebx+2]
  add  [activ_line],ax
  call line_down
  ret
.no_begin:
  cmp  word [ebx],type_end_normal+1
  je   .not_activated
  cmp  word [ebx],type_end_plus+1
  je   .not_activated
  ;goto include
  cmp  word [ebx],type_include+1
  jne  .no_incl
  inc  [activ_depth]
  mov  eax,[ebx+4]
  mov  eax,[file_array+4*eax]
  mov  [activ_file],eax
  mov  [activ_pos],eax
  mov  [activ_line],1
  ret
.no_incl:
  ;return from include
  cmp  word [ebx],type_file_end
  jne  .no_end
;CHECK FOR FIRST
  mov  ax,[ebx+2]
  mov  [activ_line],ax
  dec  [activ_depth]
  mov  ecx,[activ_file]
  xor  eax,eax
  mov  ax,[ecx-8]
  mov  eax,[file_array+4*eax]
  mov  [activ_file],eax
  add  eax,[ebx+4]
  mov  [activ_pos],eax
  mov  ebx,eax
  ret
.no_end:
;E:.
.not_activated:
  add  ebx,12 ;sckip system zone
.normal_line_down:
  cmp  byte [ebx],new_line
  je   .line_ok
  cmp  byte [ebx],char_end
  inc  ebx
  jmp  .normal_line_down
.line_ok:
  inc  ebx
  mov  [activ_pos],ebx
  inc  [activ_line]
  ret
;E:.
;B+ File possition var.
line_      dd 0x0 ;Line on screen
   
top_pos    dd start_data
top_line   dw 1
top_depth  dw 0x0
activ_pos  dd start_data
activ_line dw 0x0 ;Abs line in file
;E:.
;E:.
   
;B+ Load file tools
load_file:
;B+ Main
;B+ Init parameters
  mov  [top_pos],start_data
  mov  [activ_pos],start_data
  mov  [top_file],start_data
  mov  [activ_file],start_data
  mov  [file_array],start_data
  mov  [top_free_file],1
  mov  [end_of_file],start_data
  mov  [top_line],1
  mov  esi,file_txt
  mov  edi,incl_filename
  mov  ecx,25
  cld
rep movsb
;E:.
load_file1:
;B+ Load and fresh file
  mov  eax,[end_of_file]
 push eax
  call add_file
  mov  edi,[esp]
  mov  esi,incl_filename
  sub  edi,16+25+2
  mov  ecx,27
  cld
rep movsb
  pop  edi
  mov  word [edi-18],25
  mov  eax,[top_pos]
  mov  [activ_pos],eax
  call fresh_file
   
  call  draw_window
   
  jmp  still
;E:.
;E:.
fresh_file:
;B+ Fresh file
  mov  [line_],0
  mov  [next_],next
.fresh_next:
  inc  [line_]
  ;Test open block
  mov  eax,[begin_txt]
  mov  ebx,[activ_pos]
  cmp  [ebx],eax
  je   block_begin
  ;Test close block
  mov  eax,[end_txt]
  cmp  [ebx],eax
  je   block_end
;B+ Test include
  mov  esi,ebx
  mov  edi,include_txt
  mov  ecx,20
  cld
rep cmpsb
  cmp  byte [edi-1],char_end
  je  include_file
;E:.
next:
;B+ Go to next line
  mov  ebx,[activ_pos]
.next_char:
  cmp  byte [ebx],new_line
  je   yes_next
  cmp  byte [ebx],char_end
  je   file_end
  inc  ebx
  jmp  .next_char
yes_next:
  inc  ebx
  inc  [activ_line]
  mov  [activ_pos],ebx
  jmp  fresh_file.fresh_next
;E:.
file_end:
;B+ Auto set close block
  mov  [next_],.try_fill_next
  sub  ebx,5
  dec  [activ_line]
  mov  [activ_pos],ebx
.try_fill_next:
  add  [activ_pos],5
  inc  [activ_line]
  mov  ebx,[activ_pos]
  mov  eax,[end_txt]
  mov  ecx,[block_pointer]
  cmp  word [ecx-4],char_end
  je   .fill_ok
   inc  [line_]
   call ins_5_bytes
  mov  dword [ebx],eax
  mov  byte [ebx+4],new_line
  mov  byte [ebx+5],char_end
  mov  [activ_pos],ebx
  jmp  block_end
;E:.
.fill_ok:
 ret
;E:.
;B+ Specify line markers
block_begin:
;B+ Mark block begin
;B+ Mark info in stack
  mov  eax,[activ_pos]
  mov  ebx,[block_pointer]
 push eax
  sub  eax,[activ_file]
  mov  [ebx],eax
 pop  eax
  mov  word [ebx+4],0
  mov  cx,[activ_line]
  mov  word [ebx+6],cx
  add  ebx,8
  mov  [block_pointer],ebx
;E:.
  call ins_12_bytes
  ;line / possition not ready
  mov  word [eax],type_begin+1
  mov  dword [eax+8],new_line
  add  [activ_pos],12
  jmp  [next_]
;E:.
block_end:
;B+ Mark block end
;B+ Mark end type I
  mov  eax,[activ_pos]
  call ins_12_bytes
  mov  ecx,[block_pointer]
  cmp  word [ecx-4],char_end
  je   .normal_line
;E:.
  ;Pop stack
  sub  [block_pointer],8
  sub  ecx,8
;B+ Form this (END) line/place to BEGIN
  mov  edx,[ecx]
  add  edx,[activ_file]
  mov  eax,[activ_pos]
  sub  eax,[activ_file]
  mov  [edx+4],eax
  mov  bx,[activ_line]
  sub  bx,[ecx+6]
  mov  [edx+2],bx
;E:.
;B+ From stack line/place to this (END)
  mov  eax,[activ_pos]
  mov  edx,[ecx]
  mov  [eax+4],edx
  mov  [eax+2],bx
;E:.
;B+ Mark end type II
  mov  word [eax],type_end_normal+1
  mov  ecx,[next_]
  cmp  ecx,next
  je   .yes_normal
  mov  word [eax],type_end_plus+1
.yes_normal:
  mov  dword [eax+8],new_line
  add  [activ_pos],12
  jmp  [next_]
.normal_line:
  mov  word [eax],type_not_end
  mov  dword [eax+8],new_line
  add  [activ_pos],12
  jmp  [next_]
;E:.
;E:.
include_file:
;B+ Include and attach file
  mov  eax,[activ_pos]
 push eax
  call ins_12_bytes
 pop  eax
  mov  word [eax],type_include
  mov  word [eax+2],0
  mov  dword [eax+4],-1
  mov  dword [eax+8],new_line
  dec  [line_]
  jmp  [next_]
;E:.
   
next_ dd next
;E:.
;B+ Additional tools
ins_12_bytes:
  mov  ecx,[end_of_file]
  add  [end_of_file],12
  mov  esi,ecx
  add  ecx,12
  mov  edi,ecx
  sub  ecx,11
  sub  ecx,eax
  std
rep movsb
  mov  ecx,[activ_file]
  add  dword [ecx-16],12
  ret
   
ins_5_bytes:
  mov  ecx,[end_of_file]
  add  [end_of_file],5
  mov  esi,ecx
  add  ecx,5
  mov  edi,ecx
  sub  ecx,4
  sub  ecx,ebx
  std
rep movsb
  mov  ecx,[activ_file]
  add  dword [ecx-16],5
  ret
   
ins_1_byte:
  mov  ecx,[end_of_file]
   inc  [end_of_file]
   mov  esi,ecx
  inc  ecx
  mov  edx,ecx
  sub  ecx,ebx
  std
rep movsb
  mov  ecx,[activ_file]
  inc  dword [ecx-16]
  ret
;E:.
;E:.
   
;B+ Data section
   
;B+ View data
caption db 'CODE VIEWER - ver. 0.2'
caption_end:
   
line1up1 db 'LINE'
line1up1_end:
line1up2 db 'FILE:'
line1up2_end:
   
line1down:
filetxt db 'FILE->'
filetxt_end:
times 28 db ' '
loadtxt db 'LOAD'
loadtxt_end:
db '  BLOCK: '
begintxt db 'BEGIN->'
begintxt_end:
times 10 db ' '
db '...'
times 10 db ' '
endtxt db '<-END'
endtxt_end:
line1down_end:
   
line2down:
inctxt db 'INCLUDE KEYWORD->'
inctxt_end:
times 46 db ' '
septxt db 'SEPARATORS->'
septxt_end:
line2down_end:
;E:.
;B+ Parameter strings
str_start dd it1
str_table dd it1,0,it2,it3,it4,it5
;align
it1:
 dd 25
 dd (20+6*7)*65536+35*10+win_toptext+3
file_txt:
 db 'KERNEL.ASM               ',char_end
;align 4
it2:
 dd 4
 dd (20+56*6)*65536+35*10+win_toptext+3
begin_txt:
 db ';B+ ',char_end
;align 4
it3:
 dd 4
 dd (20+71*6)*65536+35*10+win_toptext+3
end_txt:
 db ';E:.',char_end
;align 4
it4:
 dd 43
 dd (20+18*6)*65536+35*10+win_toptext+3+14
include_txt:
 db 'include ',char_end,'                                   '
 ;align 4
it5:
 dd 2
 dd (20+76*6)*65536+35*10+win_toptext+3+14
skoba_txt:
 db '""    ',char_end
;E:.
   
block_pointer dd block_stack+8
block_stack dw 0,0,char_end,0
            times 10*2 dd 0x0
   
; STACK BOX:
; ÚÄÄÄÄÄÄÄÄÄÂÄÄÄÄÂÄÄÄÄÏ
; Óabs placeÓ  0 ÓlineÓ
; ÀÄÄÄÄÄÄÄÄÄÁÄÄÄÄÁÄÄÄÄÙ
; or 0000 'char_end' 00 - bottom
   
top_file   dd start_data
activ_file dd start_data
top_free_file dd 1
file_array dd start_data
           times 50 dd 0x0
   
end_of_file dd start_data
   
; TEXT MARKER:
; ÚÄÄÄÄÂÄÄÄÄÂÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÏ
; ÓtypeÓn.l.Ónew placeÓ    13   Ó
; ÀÄÄÄÄÁÄÄÄÄÁÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÙ
   
;E:.
   
;B+ Program preview
;ÚÄÄÄÄÄ
;ÓCODE VIEWER - ver. 0.2    o _ x
;ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;Ó    Line | File:
;Ó---------+---------------------
;Ó[+]    |
;Ó[-]    |
;Ó[ ]    |
;Ó[#]    |
;ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;Ó [FILE->]___________      [LOAD] BLOCK: [BEGIN->];B+ú   ...  ;E:.[<-END] Ó
;Ó [INCLUDE KEYWORD->]include     [SEPARATORS->]""
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;E:.
   
I_END:

