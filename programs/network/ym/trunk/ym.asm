; Yahoo Messanger for MenuetOS
; Compile with FASM for Menuet
   
;B+ System header
use32
 org	0x0
 db	'MENUET01'    ; header
 dd	0x01	      ; header version
 dd	START	      ; entry point
 dd	I_END	      ; image size
 dd	I_END+0x10000 ; required memory
 dd	I_END+0x10000 ; esp
 dd	0x0 , 0x0     ; I_Param , I_Path

;E:.
include 'lang.inc'   
include '..\..\..\macros.inc'
;B+ Definitions
v_sp equ 330
h_sp equ 400
fr_sp equ 120
   
line_wid equ 45
fr_max_lines equ 17								
   
;memory
sys_colors  equ I_END
text_zone   equ sys_colors+4*10
;friend_zone equ text_zone+45*25                        ;uncom
	    ;friend_zone+32*fr_max_lines
;E:.
   
START:
;B+ Main execution

  mov  ebx,3
  mov  ecx,sys_colors
  mov  edx,10*4
  mov  eax,48
  mcall
   
  call clear_text
   
red:
  call draw_window
still:
  mov  ebx,50
  mov  eax,23
  mcall
   
  cmp  eax,1
  je   red
  cmp  eax,2
  je   key
  cmp  eax,3
  je   button
   
  call check_message
   
  jmp  still
   
key:
  mov  eax,2
  mcall
  cmp  [is_connect],0
  je   still
  call send_key_string
  jmp  still
   
button:
  mov  eax,17
  mcall
   
  cmp  ah,1
  jne  noclose
  or  eax,-1
  mcall
  jmp  $
noclose:
   
;B+ Check friend
  cmp  ah,2
  jb   .no_friend
  cmp  ah,100
  ja   .no_friend
   
  ;pressed number
  sub  ah,2
  shr  ax,8
   
  ;find real name
  mov  [friend_p],friend_zone
  mov  edi,0
.next:
 push [friend_p]
  call test_friend
  jc   .group
  inc  edi
.group:
  cmp  di,ax
 pop  ebx
  jbe  .next
  inc  ebx
  ;exact place
  mov  ecx,[friend_p]
  sub  ecx,ebx
  dec  ecx
   
  ;Insert in send
  cmp  al,10
  jb   .good
  add  al,'A'-'0'-10
.good:
  add  al,'0'
  mov  [view_text+1],al
   
  ;Clear old a. friend
 pusha
  mov  ebx,(h_sp-140) shl 16 + 132
  mov  ecx,(v_sp-53) shl 16 + 10
  mov  edx,[sys_colors+4*5]
  mov  eax,13
  mcall
 popa
   
  ;show item
  mov  [f_name_b],ebx
  mov  [f_name_l],ecx
  call show_a_friend
  jmp  still
   
.no_friend:
;E:.
   
;B+ Check User / Password
  cmp  ah,103
  je   input_username
  cmp  ah,104
  je   input_password
;E:.
   
;B+ Connect / Dis...
  cmp  ah,101
  je   yahoo_c
  cmp  ah,102
  je   yahoo_d
;E:.
   
  jmp  still
;E:.
   
draw_window:
;B+ Draw window
  
  mov  ebx,1
  mov  eax,12
  mcall
 
  xor  eax,eax		     ;DRAW WINDOW
  mov  ebx,150*65536+h_sp
  mov  ecx,100*65536+v_sp
  mov  edx,[sys_colors+4*5]
  or   edx,0x13000000
  mov  edi,title
  mcall

;B+ Friend panel
  mov  ebx,(h_sp-fr_sp) shl 16 + 3
  mov  ecx,20 shl 16 + v_sp-31 -56
  mov  edx,[sys_colors+4*9]
  mov  eax,13
  mcall
  call show_friends
;E:.
   
;B+ Input panel
  mov  ebx,5 shl 16 + h_sp-9
  mov  ecx,(v_sp-31 -33-3) shl 16 + 3
  mov  edx,[sys_colors+4*9]
  mov  eax,13
  mcall
  mov  ebx,(h_sp-(fr_sp-12)*8/6) shl 16 + 4
  mov  ecx,(v_sp-31-33) shl 16 + 30
  mcall
  mov  ebx,(h_sp-8) shl 16 + 4
  mcall
  call show_a_friend
  call show_string
;E:.
   
;B+ Login panel
  mov  ebx,5 shl 16 + h_sp-9
  mov  ecx,(v_sp-35) shl 16 + 31
  mov  edx,[sys_colors+4*9]
  mov  eax,13
  mcall
  mov  ebx,(5+2+8+(user_txt_end-user_txt)*6) shl 16 + 6*15+7
  mov  ecx,(v_sp-32) shl 16 + 12
  mov  edx,[sys_colors+4*5]
  mcall
  mov  ebx,(171+2+8+(psw_txt_end-psw_txt)*6) shl 16 + 6*23+7
  mov  ecx,(v_sp-32) shl 16 + 12
  mcall
   
  ;connect button
  mov  ebx,(h_sp-128) shl 16 + (con_txt_end-con_txt)*6 + 7
  mov  ecx,(v_sp-18) shl 16 + 12
  mov  edx,101
  mov  esi,[sys_colors+4*6]
  mov  eax,8
  mcall
  ;disconnect button
  shl  ebx,16
  add  ebx,(h_sp-128+3) shl 16 + (dis_txt_end-dis_txt)*6 + 7
  mov  edx,102
  mcall
  ;user button
  mov  ebx,8 shl 16 + (user_txt_end-user_txt)*6 + 5
  mov  ecx,(v_sp-18-15) shl 16 + 12
  mov  edx,103
  mcall
  ;password button
  mov  ebx,174 shl 16 + (psw_txt_end-psw_txt)*6 + 5
  mov  edx,104
  mcall
   
  ;login text
  mov  ebx,11 shl 16 + v_sp-15
  mov  ecx,[sys_colors+4*7]
  mov  edx,login_txt
  mov  esi,login_txt_end-login_txt
  mov  eax,4
  mcall
  ;user text
  mov  ebx,11 shl 16 + v_sp-15-15
  mov  edx,user_txt
  mov  esi,user_txt_end-user_txt
  mcall
  ;password text
  mov  ebx,(174+5) shl 16 + v_sp-15-15
  mov  edx,psw_txt
  mov  esi,psw_txt_end-psw_txt
  mcall
  ;connect text
  mov  ebx,(h_sp-128+5) shl 16 + v_sp-15
  mov  edx,con_txt
  mov  esi,con_txt_end-con_txt
  mcall
  ;disconnect text
  add  ebx,((con_txt_end-con_txt)*6+8 + 3) shl 16
  mov  edx,dis_txt
  mov  esi,dis_txt_end-dis_txt
  mcall
   
  call show_username
  call show_password
;E:.
   
  call show_text
   
  mov  ebx,2
  mov  eax,12
  mcall
  ret
;E:.
   
show_friends:
;B+ Show friend list
  cmp  [last_friend_place],friend_zone
  jne  .yes_show
  ret
.yes_show:
   
  ;show button
  mov  ebx,(h_sp-fr_sp+5) shl 16 + 10
  mov  ecx,(20+3) shl 16 + 10
  mov  edx,2
  mov  esi,[sys_colors+4*6]
  mov  eax,8
  mov  edi,0
   
  mov  [friend_p],friend_zone
.next_button:
  call test_friend
  jc   .no_b
  mcall
  inc  edx
.no_b:
  inc  edi
  add  ecx,15 shl 16
  cmp  edi,[last_friend_line]
  jne  .next_button
   
  ;show numbers
  mov  [digit],'0'-1
  mov  ebx,(h_sp-fr_sp+8) shl 16 + (20+3)+2
  ;mov  ecx,[sys_colors+4*7]
  mov  edx,digit
  mov  esi,1
  mov  eax,4
  mov  edi,0
   
  mov  [friend_p],friend_zone
 push edx
.next_digit:
  mov  edx,[friend_p]
  call test_friend
  cmp  [edx],byte 1
  je   .no_item
  inc  [digit]
  cmp  [digit],'9'+1
  jne  .good
  mov  [digit],'A'
.good:
  ;add  ebx,1 shl 16
  cmp  [edx],byte 2
  mov  edx,[esp]
  mov  ecx,[sys_colors+4*6]
  call hi_light
  jne  .no_online
  mov  ecx,[sys_colors+4*7]
  ;mcall
  ;or    ecx,0x10000000
.no_online:
  ;sub  ebx,1 shl 16
  mcall
  ;and  ecx,not 0x10000000
.no_item:
  add  ebx,15
  inc  edi
  cmp  edi,[last_friend_line]
  jne  .next_digit
  add  esp,4
   
  ;show names
  mov  ebx,(h_sp-fr_sp+8 + 10) shl 16 + (20+3)+2
  mov  ecx,[sys_colors+4*8]
  mov  eax,4
  mov  edi,0
   
  mov  [friend_p],friend_zone
  mov  esi,4
.next_name:
  mov  edx,[friend_p]
  call test_friend
  mov  esi,[friend_p]
  inc  edx
  sub  esi,edx
   
  and  ebx,0xffff
  or   ebx,(h_sp-fr_sp+8 + 10) shl 16
  cmp  [edx-1],byte 1
  jne  .no_group
  sub  ebx,12 shl 16
.no_group:
  mcall
  add  ebx,15
  inc  edi
  cmp  edi,[last_friend_line]
  jne  .next_name
   
  ret
.p db 16 ;>
   
digit db '0'
;last_friend_line dd 0x0                                ;uncom
   
test_friend:
 push eax
  mov  eax,[friend_p]
  clc
  cmp  [eax],byte 1
  jne  .no_hide
  stc
.no_hide:
 pushf
.next:
  inc  [friend_p]
  mov  eax,[friend_p]
  cmp  [eax],byte 0
  jne  .next
  inc  [friend_p]
 popf
 pop  eax
  ret
   
friend_p dd 0x0
   
hi_light:
 pushf
  add  ecx,0x400000
  test ecx,0xb00000
  jnz  .no_red_plus
  sub  ecx,0x400000
.no_red_plus:
  add  ecx,0x004000
  test ecx,0x00b000
  jnz  .no_green_plus
  sub  ecx,0x008000
.no_green_plus:
  add  ecx,0x000040 ;80
  test ecx,0x0000b0 ;80
  jnz  .no_blue_plus
  sub  ecx,0x000040 ;100
.no_blue_plus:
 popf
  ret
;E:.
   
;B+ Message text op.
clear_text:
  mov  edi,text_zone
  mov  ecx,45*26
  mov  al,0
  cld
rep stosb
  ret
   
show_text:
  mov  ebx,7 shl 16 + (20+3) ;+ 2
  mov  ecx,[sys_colors+4*8]
  mov  edx,text_zone+45
  mov  esi,45
  mov  eax,4
  mov  edi,0
.next_line:
  cmp  [edx-1],byte 0
  jne  .shift
  mcall
.next:
  add  ebx,10
  add  edx,45
  inc  edi
  cmp  edi,24
  jne  .next_line
  ret
.shift:
  add  ebx,3 shl 16
  mcall
  sub  ebx,3 shl 16
  jmp  .next
   
scroll_text:
 pusha
  ;move text
  mov  edi,text_zone
  mov  esi,edi
  add  esi,line_wid
  mov  ecx,line_wid*24
  cld
rep movsb
  ;clear last line
  mov  ecx,line_wid
  mov  al,0
rep stosb
  ;clear zone
  mov  ebx,7 shl 16 + line_wid*6+2
  mov  ecx,(25-2) shl 16 + 24*10-2 +2
  mov  edx,[sys_colors+4*5]
  mov  eax,13
  mcall
  ;show text
  call show_text
 popa
  ret
   
show_message:
 ;ebx - begin
 ;ecx - length
   
  mov  eax,[.m_p]
  add  eax,ecx
.test:
  cmp  eax,text_zone+line_wid*25-1
  jb   .good1
  call scroll_text
  sub  eax,line_wid
  sub  [.m_p],line_wid
  jmp  .test
.good1:
  cmp  [.m_p],text_zone+line_wid
  jae  .good2
  add  ebx,line_wid
  add  [.m_p],line_wid
  sub  ecx,line_wid
  jmp  .good1
.good2:
  ;
 push ecx
  mov  esi,ebx
  mov  edi,[.m_p]
  cld
rep movsb
 pop  ecx
   
  ;find v place
  mov  eax,[.m_p]
  sub  eax,text_zone+line_wid
  mov  ebx,line_wid
  xor  edx,edx
  div  ebx
  xor  edx,edx
  mov  ebx,10
  mul  ebx
  mov  ebx,eax
  ;show line
  add  ebx,7 shl 16 + 23 ;+2
  mov  ecx,[sys_colors+4*8]
  mov  edx,[.m_p]
  mov  esi,line_wid
  mov  eax,4
  mcall
  add  ebx,3 shl 16
.next_line:
  add  ebx,10
  add  edx,line_wid
  cmp  [edx-1],byte 0
  je   .good3
  mcall
  jmp  .next_line
.good3:
  mov  [.m_p],edx
  ret
   
.m_p dd text_zone+45
;E:.
   
;B+ Show current people
show_a_friend:
  mov  ebx,(h_sp-137) shl 16 + v_sp-52
  mov  ecx,[sys_colors+4*8]
  or   ecx,0x10000000
  mov  edx,[f_name_b]
  mov  esi,[f_name_l]
  mov  eax,4
  mcall
  ret
   
f_name_b dd fnb
f_name_l dd 10
   
fnb:
 db 'yahoo_help'
;E:.
   
;B+ Input strings
send_key_string:
;B+ Test active keys
  cmp  ah,13
  je   send_text
  cmp  ah,27
  je   clear_input_text
  cmp  ah,8
  je   .backs_text
;E:.
   
  mov  [.this_c],ah
  cmp  [.c_pl],123
  jne  .show
  ret
.show:
   
  ;save char
  mov  ebx,[.c_pl]
  mov  [in_text+ebx],ah
  inc  [.c_pl]
   
  ;show char
  mov  ebx,[.xy]
  mov  ecx,[sys_colors+4*8]
  mov  edx,.this_c
  mov  esi,1
  mov  eax,4
  mcall
  ;
  cmp  [.c_pl],41
  je   .new_line
  cmp  [.c_pl],82
  je   .new_line
  add  [.xy],6 shl 16
  call show_cursor
  ret
  ;;;
.new_line:
  and  [.xy],0x0000ffff
  add  [.xy],9 shl 16 + 9
  call show_cursor
  ret
   
.this_c db ' '
.c_pl dd 0x0
.xy dd 7 shl 16 + v_sp-62
   
;B+ Special keys - action
.backs_text:
  ;
  cmp  [.c_pl],0
  jne  .yes_back
  ret
.yes_back:
  cmp  [.c_pl],41
  je   .back_line
  add  [.xy],2 shl 16
  cmp  [.c_pl],82
  je   .back_line
  sub  [.xy],2 shl 16
.next:
  ;
  sub  [.xy],6 shl 16
  dec  [.c_pl]
  mov  eax,[.c_pl]
  mov  bl,[in_text+eax]
  mov  [.this_c],bl
  mov  ebx,[.xy]
  mov  ecx,[sys_colors+4*5]
  mov  edx,.this_c
  mov  esi,1
  mov  eax,4
  mcall
  mov  ebx,[.c_pl]
  mov  [in_text+ebx],byte 0
  jmp  show_cursor
  ;
.back_line:
  ;and  [.xy],0x0000ffff
  sub  [.xy],9
  add  [.xy],(253-9) shl 16
  jmp  .next
   
send_text:
  ;show text to message board
  mov  ebx,view_text
  mov  ecx,[send_key_string.c_pl]
  add  ecx,3
  call show_message
   
  ;send message to internet
  ;call internet_send
   
clear_input_text:
  ;prepare new message
  ;; clear memory
  mov  edi,in_text
  mov  ecx,255/4
  xor  eax,eax
  cld
rep stosd
  ;; clear zone
  mov  ebx,5 shl 16 + h_sp-140-9
  mov  ecx,(v_sp-31 -33) shl 16 + 29
  mov  edx,[sys_colors+4*5]
  mov  eax,13
  mcall
  ;; move cursor
  mov  ebx,7 shl 16 + v_sp-62
  mov  [send_key_string.xy],ebx
  mov  [show_cursor.old_xy],ebx
  ;; clear place
  xor  ebx,ebx
  mov  [send_key_string.c_pl],ebx
   
;  call show_cursor
;  ret
;E:.
   
show_cursor:
  ;login text
;  mov  ebx,4 shl 16 + v_sp-64
  mov  ebx,[.old_xy]
  sub  ebx,3 shl 16 + 2
  mov  ecx,[sys_colors+4*5]
  mov  edx,curs
  mov  esi,1
  mov  eax,4
  mcall
  add  ebx,4
  mcall
  mov  ebx,[send_key_string.xy]
  mov  [.old_xy],ebx
  sub  ebx,3 shl 16 + 2
  mov  ecx,0xffffff;[sys_colors+4*8]
  mcall
  add  ebx,4
  mcall
  ret
   
.old_xy dd 7 shl 16 + v_sp-62
curs db '|'
   
show_string:
  mov  ebx,7 shl 16 + v_sp-62
  mov  ecx,[sys_colors+4*8]
  mov  edx,in_text
  mov  esi,41
  mov  eax,4
  mcall
  add  ebx,2 shl 16 + 9
  add  edx,41
  mcall
  add  ebx,9
  add  edx,41
  mcall
  call show_cursor
  ret
   
view_text db 16,'?',16
in_text: times 255 db 0
;E:.
   
;B+ Friends...
add_friend:
 ;ebx - begin
 ; [ebx]=1 - Group name
 ; [ebx]=2 - Active user
 ; [ebx]=other - Non active user
 ;ecx - length
  cmp  [last_friend_line],fr_max_lines-1
  je   .no_more
  test ecx,not 31
  jnz  .no_more ; very long id name
  inc  [last_friend_line]
  mov  esi,ebx
  mov  edi,[last_friend_place]
  inc  ecx
  add  [last_friend_place],ecx
  dec  ecx
  cld
rep movsb
  mov  al,0
  stosb
  stosb
.no_more:
  ret
   
last_friend_place dd fr_e				;del
;last_friend_place dd friend_zone                       ;uncom
   
find_friend:
 push ebx ecx
  mov  edx,friend_zone
  mov  esi,0
  mov  edi,[last_friend_line]
;  inc  edi                                             ;? uncom ?
.next_name:
  cmp  [edx],byte 1
  je   .no_find ;Group                                                          
  inc  edx
  dec  ecx
.next:
  mov  al,[edx]
  mov  ah,[ebx]
  cmp  ah,al
  jne  .no_find
  inc  edx
  inc  ebx
  dec  ecx
  jne  .next
  cmp  [edx],byte 0
  jne  .no_find
  ;find
  mov  eax,esi
  cmp  esi,9
  ja   .letter
  add  al,'0'
  ret
.letter:
  add  al,'A'-10
  ret
.no_find:
  cmp  [edx],byte 0
  je   .go_next
  inc  edx
  jmp  .no_find
.go_next:
  dec  edi
  je   .noting
  mov  ebx,[esp+4]
  mov  ecx,[esp]
  inc  esi
  jmp  .next_name
.noting:
  mov  al,'!'
 pop  ecx ebx
  ret
   
;E:.
   
;B+ Connect / Disconnect
yahoo_c:
  call connect
  cmp  eax,0
  jne  still ;not connected
  mov  [is_connect],0x1
  jmp  still
   
yahoo_d:
  cmp  [is_connect],0x0
  je   .noting
   
  call disconnect
  ;
  ;stop connection
  mov  [is_connect],0x0
  ;
  ;clear text
  mov  ah,27
  call send_key_string
  ;
  ;clear friends
;  mov  [last_friend_line],0x0                          ;uncom
;  mov  [last_friend_place],friend_zone                 ;uncom
  ;
  ;set dafaut friend
  mov  [f_name_b],fnb
  mov  [f_name_l],10
  mov  [view_text+1],'?'
   
  call draw_window
   
.noting:
  jmp  still
   
is_connect dd 0x0
;E:.
   
;B+ Load username / password
input_username:
  mov  edi,username
  mov  [edi],byte '_'
  inc  edi
  mov  ecx,16-1
  cld
rep stosb
  mov  [.unp],username
   
.next:
  call show_username
   
  ;get enen
  mov  eax,10
  mcall
   
  cmp  eax,1
  je   .end
  cmp  eax,3
  je   .end
   
  ;key
  mov  eax,2
  mcall
   
  cmp  ah,13
  je   .end
  cmp  ah,8
  jne  .no_back
  cmp  [.unp],username
  je   .next
  dec  [.unp]
  mov  ebx,[.unp]
  mov  [ebx],byte '_'
  mov  [ebx+1],byte 0
  jmp  .next
.no_back:
   
  cmp  [.unp],username+16
  je   .next
   
  cmp  ah,'0'
  jb   .bad
   
  mov  ebx,[.unp]
  mov  [ebx],ah
  mov  [ebx+1],byte '_'
  inc  [.unp]
   
.bad:
  jmp  .next
.end:
  ;del cursor
  mov  ebx,[.unp]
  mov  [ebx],byte 0
  call show_username
  ;clear password
  mov  [password],byte 0
  ;hide password
  mov  ebx,(2+41*6) shl 16 + v_sp-15-15
  mov  ecx,[sys_colors+4*5]
  mov  edx,f_password
  mov  esi,4
  mov  eax,4								       
  mcall
  jmp  still
.unp dd username
   
show_username:
  ;hide
  mov  ebx,(4+12*6-1) shl 16 + 16*6+1
  mov  ecx,(v_sp-15-15) shl 16 + 9
  mov  edx,[sys_colors+4*5]
  mov  eax,13
  mcall
  ;show
  mov  ebx,(4+12*6) shl 16 + v_sp-15-15
  mov  ecx,[sys_colors+4*8]
  mov  edx,username
  mov  esi,16
  mov  eax,4									
  mcall
  ret
   
username: times (16+1) db 0
   
   
   
input_password:
  ;clear
  mov  edi,password
  mov  ecx,24
  mov  al,0
  cld
rep stosb
  mov  [.unp],password
  ;hide password
  mov  ebx,(2+41*6) shl 16 + v_sp-15-15
  mov  ecx,[sys_colors+4*5]
  mov  edx,f_password
  mov  esi,4
  mov  eax,4									
  mcall
   
.next:
  ;get enen
  mov  eax,10
  mcall
   
  cmp  eax,1
  je   still
  cmp  eax,3
  je   still
   
  ;key
  mov  eax,2
  mcall
   
  cmp  [.unp],password+24
  je   .no_next
  cmp  ah,13
  jne  .no_still
.no_next:
  call show_password
  jmp  still
.no_still:									
   
  mov  ebx,[.unp]
  mov  [ebx],ah
  inc  [.unp]
  jmp  .next
   
.unp dd password
   
show_password:
  cmp  [password],byte 0
  je   .end
  mov  ebx,(2+41*6) shl 16 + v_sp-15-15
  mov  ecx,[sys_colors+4*8]
  mov  edx,f_password
  mov  esi,4
  mov  eax,4									
  mcall
.end:
  ret										
   
f_password db '####'
   
password: times (24+1) db 0
;E:.
   
   
   
;B+ INTERNET
   
;Functions:
 ;call add_friend
 ; ebx+1 - pointer to name
 ;   [ebx]=1 - Group name
 ;   [ebx]=2 - Active user
 ;   [ebx]=other - Non active user
 ; ecx - length
 ;
 ;call show_message
 ; ebx - begin of string
 ; ecx - length
 ; -----
 ; NOTE Use format:
 ;  (<char>) <message>
 ;  where:
 ;   <char> - friend user char
 ;   <message> - message from friend
 ;
 ;call find_friend
 ; ebx - begin of name
 ; ecx - length
 ; ret:
 ; al - friend user char
 ; -----
 ; NOTE currenly don't show message if al='!'
										
;Variables
 ;usernave (zero terminated)
 ;password (zero terminated)
 ;f_name_b - current friend user (to send)
 ;f_name_l - ^ length
   
;Memory
 ; (friend_zone+32*fr_max_lines) < addr: [addr] - free
   
   
connect:
  ;conect to yahoo
  ;return 0 if OK
  ;return <>0 if some other event (sys.func.23)
  mov  eax,0
  ret
   
disconnect:									
  ;disconnect
  ret
   
check_message:
  ;test receive messages
  ret
   
;E:.
   
   
   
;B+ Test data                                           ;del
friend_zone:						;del
 db 1,'First:',0					;del
 db 2,'hahaha',0					;del
 db 3,'second',0					;del
 db 3,'menuetos',0					;del
 db 1,'Treti:',0					;del
 db 2,'fourth',0					;del
fr_e db 0						;del
							;del
times 200 db 0						;del
							;del
last_friend_line dd 0x6 				;del

title db 'Messenger (Yahoo Compatible)',0
   
;User / Password
login_txt db 'STATUS:            SESSION: ___.___.___.___'
		     ;VISIBLE
		     ;HIDDEN
login_txt_end:
user_txt db 'USER ID ->'
user_txt_end:
psw_txt db 'PASSWORD ->'
psw_txt_end:
con_txt db 'CONNECT'
con_txt_end:
dis_txt db 'DISCONNECT'
dis_txt_end:
   
;E:.
I_END:
   