;
;   Hunter. Version 1.1.
;   The game application for MeOS game compo 05.03.2005 - 12.03.2005
;
;   Author:       Trans
;   Date:         07.03.2005 - 08.03.2005
;   Modification: 08.05.2007
;   Compiler:     FASM
;   Target:       MenuetOS game
;

include '..\..\..\macros.inc'

use32

               org    0x0

               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     end_memory	;0x180000 ; memory for app
               dd     start_stack             ; esp
               dd     0x0 , 0x0               ; I_Param , I_Icon

START:                          ; start of execution
     mov eax,40
     mov ebx,00100111b
     mcall

	mov eax,26
	mov ebx,9
	mcall
	mov [second_count],eax

	call init_object

     call draw_window

still:

;    mov eax,23                 ; wait here for event
;    mov ebx,10
;    mcall

	mov eax,05
	mov ebx,5
	mcall
	mov eax,11
	mcall

	push eax
	mov eax,26
	mov ebx,9
	mcall
	mov ebx,eax
	sub eax,[second_count]
	cmp ax,100
	jb still_continue_00
	inc dword [time_count]
	mov [second_count],ebx
	xor edx,edx
	mov eax,ebx
	mov ebx,30
	div ebx
	cmp dl,0
	jnz still_loc_01
	call change_objects_speed
still_loc_01:
	xor eax,eax
	mov esi,forward_list
	mov ecx,4
still_loc_00:
	cmp eax,dword [esi]
	jnz still_continue_00
	add esi,4
	loop still_loc_00
;	call set_current_objects_list
	call init_object
still_continue_00:
	pop eax
	
	call movie_objects
	
    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,2                  ; key in buffer ?
    je   key
    cmp  eax,3                  ; button in buffer ?
    je   button
    cmp  eax,6                  ; mouse ?
    je   mouse

	call draw_frame

    jmp  still

  red:                          ; redraw
    call draw_window
    jmp  still

  key:                          ; key
    mov  eax,2                  ; just read it and ignore
    mcall
	cmp al,0
	jnz still
	cmp byte [menu_flag],0
	jnz still
	cmp ah,' '
	jnz key_loc_00
	inc byte [pause_flag]
	call draw_frame
	call pause_game
key_loc_00:
    jmp  still

  button:                       ; button
    mov  eax,17                 ; get id
    mcall

    cmp  ah,1                   ; button id=1 ?
    jne  noclose

	inc byte [game_over_flag]
	call game_over_draw

    mov  eax,-1                 ; close this program
    mcall
  noclose:

    jmp  still


mouse:                          ; mouse
    mov eax,37                 ; get coordinate
    xor ebx,ebx
    inc ebx
    mcall

    cmp ax,20
    jae mouse_loc_00
    mov ax,20
mouse_loc_00:
    sub ax,20
    shr ax,1
    ror eax,16
    shr ax,1
    push ax
    ror eax,16
    mov dword [mouse_coord],eax
    pop ax
    cmp ax,110
    ja mouse_loc_03
    mov byte [gun_angle],0
    jmp mouse_loc_05
mouse_loc_03:
    cmp ax,210
    ja mouse_loc_04
    mov byte [gun_angle],1
    jmp mouse_loc_05
mouse_loc_04:
    mov byte [gun_angle],2
mouse_loc_05:    

    mov eax,37
    xor ebx,ebx
    mov bl,2
    mcall

    cmp ax,1
    jnz not_left_button
	call left_button_down
not_left_button:
    call draw_frame
    jmp  still




;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:


    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall

                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,100*65536+640         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+420         ; [y start] *65536 + [y size]
    mov  edx,0x13ffffff            ; color of work area RRGGBB,8->color gl
    mov  edi,title                 ; WINDOW LABEL
    mcall

	call draw_frame
	
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall

    ret

;-------------Subprograms------------------------

;
; In:
; Out:
draw_frame:
	cmp byte [menu_flag],0
	jz df_loc_00
	call draw_menu
	retn
df_loc_00:
	call draw_backside
	mov esi,buf_oblako
	mov eax,0*65536+0
	call draw_pict
	mov eax,190*65536+23
	call draw_pict_scale_div_2
	mov esi,buf_sun
	mov eax,256*65536+0
	call draw_pict
	mov esi,buf_rock
	mov eax,47*65536+83
	call draw_pict_scale_div_2
	mov eax,0*65536+70
	call draw_pict

	call draw_objects

	mov esi,buf_kust02
	mov eax,256*65536+127
	call draw_pict
	mov esi,buf_kust01
	mov eax,0*65536+127
	call draw_pict
	mov esi,buf_time
	mov eax,5*65536+180
	call draw_pict
	mov esi,buf_shots
	mov eax,200*65536+180
	call draw_pict
	mov esi,buf_lives
	mov eax,5*65536+5
	call draw_pict
	mov eax,56*65536+3
	call draw_lives
	call draw_promakh
;	call draw_hole	
	cmp byte [gun_angle],0
        jnz df_loc_01
	mov esi,buf_gun02
	mov eax,120*65536+151
	call draw_pict
	cmp byte [fire_flag],0
	jz df_loc_03
	mov esi,buf_fire02
	mov eax,108*65536+131
	call draw_pict
	dec byte [fire_flag]
	jmp df_loc_03
df_loc_01:
	cmp byte [gun_angle],2
	jnz df_loc_02
	mov esi,buf_gun01
	mov eax,140*65536+151
	call draw_pict
	cmp byte [fire_flag],0
	jz df_loc_03
	mov esi,buf_fire01
	mov eax,167*65536+128
	call draw_pict
	dec byte [fire_flag]
	jmp df_loc_03
df_loc_02:
	mov esi,buf_gun00
	mov eax,140*65536+144
	call draw_pict
	cmp byte [fire_flag],0
	jz df_loc_03
	mov esi,buf_fire00
	mov eax,135*65536+124
	call draw_pict
	dec byte [fire_flag]
df_loc_03:
;	mov esi,buf_star
;	mov eax,300*65536+180
;	call draw_pict
	mov esi,buf_score
	mov eax,120*65536+5
	call draw_pict
	mov eax,260*65536+180
	mov ebx,[shots_count]
	call number_print
	mov eax,175*65536+3
	mov ebx,[score_count]
	call number_print
	mov eax,60*65536+180
	mov ebx,[time_count]
	call number_print
	call draw_mushka
	cmp byte [pause_flag],0
	jz df_loc_04
	mov esi,buf_pause
	mov eax,90*65536+80
	call draw_pict
df_loc_04:
	cmp byte [game_over_flag],0
	jz df_loc_05
	mov esi,buf_gameover
	mov eax,[game_over_coord]
	call draw_pict
df_loc_05:

	call buffer_scale_on_2
;	call smooth_filter
	mov eax,07
	mov ebx,buffer02
	mov ecx,640*65536+400
	mov edx,0*65536+20
	mcall
	retn


;
; In:
; Out:
draw_menu:
	call clear_buffer
	mov esi,buf_start
	mov eax,60*65536+50
	call draw_pict
	mov esi,buf_exit
	mov eax,86*65536+120
	call draw_pict
	call draw_mushka
	call buffer_scale_on_2
	mov eax,07
	mov ebx,buffer02
	mov ecx,640*65536+400
	mov edx,0*65536+20
	mcall
	retn



;
; In:
; Out:
pause_game:
	mov eax,40
	mov ebx,00000111b
	mcall

pause_game_00:
    mov eax,10                 ; wait here for event
    mcall

    cmp  eax,1                  ; redraw request ?
    je   pg_red
    cmp  eax,2                  ; key in buffer ?
    je   pg_key
    cmp  eax,3                  ; button in buffer ?
    je   pg_button
    jmp  pause_game_00

pg_red:                          ; redraw
    call draw_window
    jmp  pause_game_00

pg_key:                          ; key
    mov  eax,2                  ; just read it and ignore
    mcall
	cmp al,0
	jnz pause_game_00
	cmp ah,' '
	jnz pgk_loc_00
	dec byte [pause_flag]
	call draw_frame
	mov eax,40
	mov ebx,00100111b
	mcall
	retn
pgk_loc_00:
    jmp  pause_game_00

pg_button:                       ; button
    mov  eax,17                 ; get id
    mcall
    cmp  ah,1                   ; button id=1 ?
    jne  pg_noclose
    mov  eax,-1                 ; close this program
    mcall
pg_noclose:
    jmp  pause_game
	retn

;
; In:
; Out:
left_button_down:
	cmp byte [menu_flag],0
	jz lbd_loc_00
	call check_menu
	retn
lbd_loc_00:
	inc dword [shots_count]
	inc byte [fire_flag]
;	mov eax,[mouse_coord]
;	mov [current_hole],eax
	call check_kill_object
	cmp byte [promakh_count],5
	jnz lbd_loc_01
	mov byte [promakh_count],0
	dec byte [lives_count]
	jnz lbd_loc_01
	call draw_frame
	mov byte [game_over_flag],1
	call game_over_draw
	dec byte [game_over_flag]
	inc byte [menu_flag]
lbd_loc_01:
	retn

;
; In:
; Out:
check_menu:
	push eax
	mov eax,[mouse_coord]
	cmp ax,50
	jb cm_loc_00
	cmp ax,90
	ja cm_loc_00
	shr eax,16
	cmp ax,60
	jb cm_loc_00
	cmp ax,260
	ja cm_loc_00
	dec byte [menu_flag]
	xor eax,eax
	mov [score_count],eax
	mov [shots_count],eax
	mov [time_count],eax
	mov [current_hole],eax
	mov [current_hole+4],eax
	mov [gun_angle],al
	mov [stars_count],al
	mov [promakh_count],al
	mov byte [lives_count],3
;	call set_current_objects_list
	call init_object
	pop eax
	retn
cm_loc_00:
	mov eax,[mouse_coord]
	cmp ax,120
	jb cm_loc_01
	cmp ax,160
	ja cm_loc_01
	shr eax,16
	cmp ax,86
	jb cm_loc_01
	cmp ax,234
	ja cm_loc_01
	mov eax,-1
	mcall
cm_loc_01:
	pop eax
	retn


;
;
;
game_over_draw:
	pusha
	mov dword [game_over_coord],42*65536+0
	mov ecx,15
god_loc_00:
	push ecx
	call draw_frame
	add word [game_over_coord],5
	mov eax,05
	mov ebx,3
	mcall
	pop ecx
	loop god_loc_00
;	mov eax,7
	mov edx,0*65536+20
	mov ebx,buffer02
	mov ecx,3
god_loc_01:
	push ecx
	mov eax,7
	mov ecx,640*65536+400
	mcall
	push ebx
	mov eax,05
	mov ebx,5
	mcall
	pop ebx
	sub ebx,640*3*5
	pop ecx
	loop god_loc_01
	mov ecx,4
god_loc_02:
	push ecx
	mov eax,7
	mov ecx,640*65536+400
	mcall
	push ebx
	mov eax,5
	mov ebx,5
	mcall
	pop ebx
	add ebx,640*3*5
	pop ecx
	loop god_loc_02
	mov eax,5
	mov ebx,200
	mcall
	popa
	retn

include 'picture.inc'
include 'math.inc'
include 'object.inc'
;----------------Data----------------------------

mouse_coord	dd 144*65536+84
pause_flag	db 0	; 0/1 - active/pause
menu_flag	db 1	; 0/1 - game/menu
lives_count	db 3
stars_count	db 0	; 3 Stars = 1 Heart
shots_count	dd 0
time_count	dd 0
score_count	dd 0
second_count	dd 0
game_over_coord	dd 0
gun_angle	db 0	; 0/1/2 - left/vertical/right
game_over_flag	db 0	; 0/1 - continuing/end game
promakh_count	db 0
fire_flag	db 0	; 0/1 - no fire/fire from gun
current_hole	dd 0,0,0,0,0,0
forward_list	db 0,0,0,0,0,0,0,0
back_list	db 0,0,0,0,0,0,0,0
present_objects_list:
	dd buf_glass,   buf_net,     buf_glaz, buf_plane00
	dd buf_heart02, buf_plane01, buf_net,  buf_glaz

title   db 'HUNTER 1.2  Trans, 2005',0

buf_start:
include 'inc\start.inc'
buf_exit:
include 'inc\exit.inc'
buf_gameover:
include 'inc\gameover.inc'
buf_pause:
include 'inc\pause.inc'
buf_gun00:
include 'inc\gun00.inc'
buf_gun01:
include 'inc\gun01.inc'
buf_gun02:
include 'inc\gun02.inc'
buf_heart:
include 'inc\heart.inc'
buf_star:
include 'inc\star.inc'
buf_lives:
include 'inc\lives.inc'
buf_score:
include 'inc\score.inc'
buf_shots:
include 'inc\shots.inc'
buf_time:
include 'inc\time.inc'
buf_numbers:
;include 'inc\numbers.inc'
buf_num0:
include 'inc\num0.inc'
buf_num1:
include 'inc\num1.inc'
buf_num2:
include 'inc\num2.inc'
buf_num3:
include 'inc\num3.inc'
buf_num4:
include 'inc\num4.inc'
buf_num5:
include 'inc\num5.inc'
buf_num6:
include 'inc\num6.inc'
buf_num7:
include 'inc\num7.inc'
buf_num8:
include 'inc\num8.inc'
buf_num9:
include 'inc\num9.inc'
buf_plus:
include 'inc\plus.inc'
buf_minus:
include 'inc\minus.inc'
buf_sun:
include 'inc\sun.inc'
buf_oblako:
include 'inc\oblako.inc'
buf_mushka:
include 'inc\mushka.inc'
buf_kust01:
include 'inc\kust01.inc'
buf_kust02:
include 'inc\kust02.inc'
buf_glass:
include 'inc\glass.inc'
buf_glaz:
include 'inc\glaz.inc'
buf_net:
include 'inc\net.inc'
buf_plane00:
include 'inc\plane00.inc'
buf_plane01:
include 'inc\plane01.inc'
buf_heart02:
include 'inc\heart02.inc'
buf_rock:
include 'inc\rock.inc'
buf_fire00:
include 'inc\fire00.inc'
buf_fire01:
include 'inc\fire01.inc'
buf_fire02:
include 'inc\fire02.inc'
buf_hole:
include 'inc\hole.inc'

I_END:

end_stack:
	org $+1000h
start_stack:

	org $+64	; For stack protect

movieng_objects:
; For one object - 16 bytes structure:
;	dd ; +00 Pointer to object  OR  0 - if empty structure
;	dd ; +04 Current object coordinate X*65536+Y
;	db ; +08 Speed - Maximum 16 (???)
;	db ; +09 Amplitude
;	db ; +0A Direction 0/1 - right/left
;	db ; +0B Object Cost
;	dd ; +0C Reserve
;
   org $+16*16

buffer00:
   org $+320*201*3
buffer01:
   org $+100*201*3
buffer02:
   org $+640*401*3
   org $+640*40*3
end_memory: