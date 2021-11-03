;
;   Phoenix Game. Based on ASCL Library
;

;**********************************************************************
   use32
   org    0x0
   db     'MENUET01'              ; 8 byte id
   dd     0x01                    ; header version
   dd     START                   ; start of code
   dd     IM_END                  ; size of image
   dd     I_END                   ; memory for app
   dd     I_END                   ; esp stack position
   dd     0x0 , 0x0               ; I_Param , I_Icon
;**********************************************************************

include 'lang.inc'    ; for SVN 'lang.inc'
include '../../../macros.inc'  ; for SVN '..\..\..\macros.inc;'
include 'ascl.inc'         ; main
include 'ascgl.inc'        ; graphics
include 'ascpoal.inc'      ; processing object array
include 'ascgml.inc'       ; game library for collision detection

; debug flags
keyboard_debug=0     ; Set to 1 to view all keyboard keys state in game
light_space=0        ; May used for check canvas position in window
debug_scenario=0     ; May used for fast test of gameplay
stack_leak_debug=0   ; Show stack size on screen
debug_mode=1         ; Support keys:
; A - set player hp = 10000. Make player ship undestructible
; E - player hp / 2
; W - view final screen after game begin

screen_w equ 640
screen_h equ 440
;screen_w equ 1014
;screen_h equ 739

; Define special colors
cl_DarkRed equ 0x990000
cl_Orange  equ 0xbbbb00

; Define objects type id
EmptyObject  equ 0
; This numbers not possible for modify because draw_objects:
BlueWarShip  equ 1
GreenWarShip equ 2
Asteroid     equ 3
Box          equ 4

Laser        equ 1
Plasma       equ 2
StaticPlasma equ 3

;**************************************************************
;                      Start of programm
;**************************************************************

START:                          ; start of execution

   max_particles = 400
   particle_size = 20

   mov [particles],dword max_particles
   mov [particles+4],dword particle_size

   max_objects = 100
   object_size = 20

   mov [objects],dword max_objects
   mov [objects+4],dword object_size

   keyboard_set_input_mode 1  ; 0 - ASCII; 1 - scancodes;

   ;giftoimg gif_file_area2,canvas
   giftoimg gif_file_area,objects_image

   ; load sprites
   mov cl,9      ; cl - counter
   xor esi,esi   ; esi = 0
   lea edi,[ship_img]  ; address of first sprite
load_sprite:
   pushad
   getimg objects_image,esi,0,32,32,edi
   popad
   add edi,8+32*32*3   ; incrase offest of image data
   add esi,32          ; incrase x position
   dec cl
   jnz load_sprite

set_main_menu:
   mov ebx,-1       ; at start event_mode = -1
   lea ecx,[draw_main_window]
   lea edx,[menu_keyboard]
set_draw_proc:
   ; assembler extract command ; for use only bl
   mov [event_mode],ebx
   mov [draw_proc],ecx
   mov [keyboard_proc],edx
   ; at first draw window
main_menu:
redraw_event:     ; redraw event appears if user move, show/hide window
   call draw_window
no_event:
   call [draw_proc]  ; no event no requre draw_window for no flickering
if stack_leak_debug = 1
   ; stack memory leak debug
   lea ebp,[I_END]
   sub ebp,esp
   outcount ebp,10,30,cl_Green,5*65536
end if
menu_loop:
   ;wait_event redraw_event,keyboard_event,button_event
   mov eax,10   ; wait event
   mov ebx,[event_mode]
   cmp ebx,-1   ; ebx delay for mcall 23
   je wait_ev
   mov eax,23   ; time event
wait_ev:
   mcall
   dec eax
   js  no_event
   jz  redraw_event
   dec eax
   jz  keyboard_event
   dec eax
   jz  button_event
   jmp menu_loop  ; if unknown event, skip it

keyboard_event:   ; keyboard event if user press button
   call [keyboard_proc]
   jmp  menu_loop
button_event:
   window_get_button
   cmp ah,1       ; if button id=1 (Red X of window) then close app
   je  close_app
   cmp ah,2       ; if button id=2 then close app
   je  close_app
   cmp ah,3       ; if button id=3 then start game
   je  start_game
   cmp ah,4       ; if button id=3 then show help menu
   je  set_help_window  ; jmp with redraw window
   cmp ah,5       ; if button id=5 then back to main menu
   je  set_main_menu    ; jmp with redraw window
   cmp ah,6
   je  next_level  ; run next level
   cmp ah,7
   je  restart_lev ; restart level from gameover
   cmp ah,9
   je  set_game_window  ; back to game from menu
   jmp menu_loop

close_app:
   mov  eax,-1          ; close this program
   mcall

menu_keyboard:
   ; nothing do
   window_get_key
   ret

;***********************
;   Main screen menu
;***********************

draw_main_window:
   draw_label 160,160,'Phoenix',cl_Red+font_size_x8
   draw_button 3,300,320,60,14,'START',cl_DarkRed,cl_Yellow
   draw_button 4,300,340,60,14,'HELP',cl_DarkRed,cl_Yellow
   draw_button 2,300,360,60,14,'EXIT',cl_DarkRed,cl_Yellow
   ret

;**********************
;  End level process
;**********************

set_end_level_window_from_call:
   add esp,4    ; remove call address from stack
set_end_level_window:
   lea edx,[menu_keyboard]
   mov ebx,-1
   lea ecx,[draw_end_level_window]
   jmp set_draw_proc

draw_end_level_window:
   draw_frect 170,130,300,190,cl_Grey
   draw_label 280,150,'LEVEL COMPLETE',cl_Black
   draw_button 2,180,300,80,14,'EXIT',cl_DarkRed,cl_Black
   draw_button 5,280,300,80,14,'<MENU>',cl_DarkRed,cl_Black
   draw_button 6,380,300,80,14,'NEXT >',cl_DarkRed,cl_Black

   draw_image 180,170,bwarship_img
   draw_number [bships_destroyed],240,185,cl_Blue,3*65536

   draw_image 180,210,gwarship_img
   draw_number [gships_destroyed],240,225,cl_Green,3*65536

   draw_image 180,250,box_img
   draw_number [boxes_taken],240,265,cl_Orange,3*65536
   ret

;**********************
;  Game Over process
;**********************

restart_lev:
   mov [next_wave_timer],0  ; Reset new wave timer
prevpart:
   call get_wave_info_offset
   xor eax,eax
   mov ax,[ebp]
   dec eax                  ; If eax = 0 then start of level finded
   jz  reset_all_varibles_and_restart_level
   dec [level_wave]
   jmp prevpart

set_game_over_window_from_call:
   add esp,4
set_game_over_window:
   lea edx,[menu_keyboard]
   mov ebx,-1
   lea ecx,[draw_game_over_window]
   jmp set_draw_proc

draw_game_over_window:
   draw_frect 170,160,300,120,cl_Grey
   draw_label 292,200,'GAME OVER',cl_Black
   draw_button 2,180,260,80,14,'EXIT',cl_DarkRed,cl_Black
   draw_button 5,280,260,80,14,'MENU',cl_DarkRed,cl_Black
   draw_button 7,380,260,80,14,'RESTART',cl_DarkRed,cl_Black
   ret

;***********************
;    Main game loop
;***********************

next_level:
   mov [next_wave_timer],0 ; Reset next wave timer
   inc [level_wave]        ; Next wave
start_game:
   ; Set canvas size before logo is showed
   image_set_size canvas,screen_w,screen_h

; Clear all, and prepeare varibles before start
reset_all_varibles_and_restart_level:

   ; Clear objects arrays
   mov eax,0
   cld
   mov edi,particles+8
   mov ecx,max_particles*particle_size/4
   rep stosd
   mov edi,objects+8
   mov ecx,max_objects*object_size/4
   rep stosd

   ; Reset keyboard state array
   clear_buffer keymap, 128, 0

   ; Reset player ship state
   mov [player_hp],100
   mov [shipx],screen_w/2-16 ; Player ship x start position
   mov [shipy],screen_h-40   ; Player ship y start position
   mov [laser_shoots],1000   ; Laser projectiles at start
   mov [plasma_shoots],500   ; Plasma projectiles at start

   ; Reset counters and gun charge values
   xor eax,eax
   mov [boxes_taken],eax
   mov [gships_destroyed],eax
   mov [bships_destroyed],eax
   mov [laser_charge],eax
   mov [plasma_charge],eax

set_game_window:
   lea edx,[ingame_keyboard]
   lea ecx,[draw_game_window]
   mov ebx,2          ; time_event ebx = 2 ms
   jmp set_draw_proc

draw_game_window:
   draw_image 5,24,canvas

   xor eax,eax               ; Use eax as zero for compare
   cmp [pause_on],eax        ; If pause = 0 nothing do
   je no_pause
   ret
no_pause:

   cmp [player_hp],eax       ; If player_hp < 0 game over
   jl  set_game_over_window_from_call

   cmp [laser_charge],eax
   je  no_dec_laser_charge
   dec dword [laser_charge]  ; If laser_charge > 0 decrase it
no_dec_laser_charge:
   cmp [plasma_charge],eax
   je  no_dec_plasma_charge
   dec dword [plasma_charge] ; If plasma_charge > 0 decrase it
no_dec_plasma_charge:

   ; new wave test
   inc dword [next_wave_timer]
   call get_wave_info_offset
   mov ax,[ebp]        ; [ebp] = time to activate string
   cwde                ; extend ax to eax
   cmp [next_wave_timer],eax
   jne no_next_wave

add_new_wave_of_objects:
   mov cl,[ebp+4+0]    ; cl = count of ships
   mov ch,GreenWarShip ; ch = type of object
   mov ebx,0x09040302  ; ebx = [xmoving]:[xaccel]:[ymoving]:[yaccel]
   call add_objects    ; add objects

   mov cl,[ebp+4+1]    ; cl = count of ships
   mov ch,BlueWarShip  ; ch = type of object
   mov ebx,0x03010306  ; ebx = random ranges : x = -1..1 y = -6..-4
   call add_objects    ; add objects

   mov cl,[ebp+4+2]    ; cl = count of asteroids
   mov ch,Asteroid     ; ch = type of object
   mov ebx,0x05020502  ; ebx = [xmoving]:[xaccel]:[ymoving]:[yaccel]
   call add_objects    ; add objects

   mov cl,[ebp+4+3]    ; cl = count of boxes
   mov ch,Box          ; ch = type of object
   mov ebx,0x05020401  ; ebx = [xmoving]:[xaccel]:[ymoving]:[yaccel]
   call add_objects    ; add objects

   mov [next_wave_timer],0 ; Reset next wave timer
   inc [level_wave]        ; Next wave
no_next_wave:

   ; Calculate all active objects on screen
   xor eax,eax           ; Use eax as zero
   mov [objects_num],eax
   array_processing objects,endtest ; ar_proc not modify eax, ebx
   xor eax,eax           ; Use eax as zero
   cmp [objects_num],eax ; If [objects_num] != 0, level is not complete
   jnz level_not_complete

   call get_wave_info_offset
   mov ax,[ebp+2]  ; ax = maxyrange
   cwde            ; extend ax to eax
   xor ebx,ebx     ; Use ebx as zero for compare
   cmp eax,ebx     ; If [ebp+2] of level = 0, this is end of level
   je  set_end_level_window_from_call  ; Show player score
   dec ebx         ; Use ebx as -1 for compare
   cmp eax,ebx
   je  set_final_screen_window_from_call ; Show final animation
level_not_complete:

no_end_lev:

; Key state processing
   cmp byte [keymap+key_Right],0
   je  no_key_right
   add dword [shipx],6
no_key_right:
   cmp byte [keymap+key_Left],0
   je  no_key_left
   sub dword [shipx],6
no_key_left:
   cmp byte [keymap+key_Space],0
   je  no_key_lshoot
   call try_to_make_laser_shoot
   ;call try_to_make_plasma_shoot
no_key_lshoot:
   cmp byte [keymap+key_Up],0
   je  no_key_pshoot
   ;call try_to_make_plasma_nuke
   call try_to_make_plasma_shoot
no_key_pshoot:
   cmp byte [keymap+key_Down],0
   je  no_key_pnuke
   call try_to_make_plasma_nuke
   ;call try_to_make_laser_shoot
no_key_pnuke:

; Ship position correction (clamp macro)
   cmp [shipx],5
   jnl @f
   mov [shipx],5
@@:
   cmp [shipx],screen_w-32-5
   jng @f
   mov [shipx],screen_w-32-5
@@:

   mov al,7
if light_space = 1
   mov al,255
end if
   clear_buffer canvas+8,canvas_end-canvas-8,al

   compmas objects,particles,objects_and_particles_hit_handling
   ; move objects and particles
   array_processing objects,move_objects
   array_processing particles,move_particles
   ; remove particles out of screen
   array_processing particles,remove_outofscr_particles
   ; objects and particles collision test
   array_processing objects,player_and_objects_collision_handling
   array_processing particles,player_and_particles_collision_handling
   ; draw objects and particles
   array_processing objects,draw_objects
   array_processing particles,draw_particles
   ; draw player ship
   image_draw_acimage canvas,ship_img,[shipx],[shipy],cl_Black

   ; Draw info indicators
   draw_frect 150,5,64,5,cl_Black
   mov eax,[plasma_charge]
   sub eax,256
   neg eax
   shr eax,2
   draw_frect 150,5,eax,5,cl_Cyan

   draw_frect 150,12,64,5,cl_Black
   mov eax,[laser_charge]
   sub eax,8
   neg eax
   shl eax,3
   draw_frect 150,12,eax,5,cl_Yellow

   draw_frect 220,2,6*5+2 ,9,cl_Grey
   draw_number [plasma_shoots],221,3,cl_Cyan,5*65536
   draw_frect 220,11,6*5+2 ,9,cl_Grey
   draw_number [laser_shoots],221,12,cl_Yellow,5*65536

   draw_frect 280,6,6*5+2 ,9,cl_Grey
   draw_number [gships_destroyed],281,7,cl_Green,5*65536
   draw_frect 320,6,6*5+2 ,9,cl_Grey
   draw_number [bships_destroyed],321,7,cl_Blue,5*65536
   draw_frect 360,6,6*5+2 ,9,cl_Grey
   draw_number [boxes_taken],361,7,0xffaa00,5*65536

   ; number of objects in scene
   draw_frect 400,2,6*5+2 ,9,cl_Grey
   draw_number [objects_num],401,2,cl_Lime,5*65536

   draw_frect 400,11,6*5+2 ,9,cl_Grey
   draw_number [player_hp],401,12,cl_Red,5*65536

   draw_frect 450,11,6*5+2 ,9,cl_Grey
   draw_number [score],451,12,cl_Yellow,5*65536 ;+hide_zeros

   ; print keyboard keys state as string for debug
if keyboard_debug = 1
   mov ebx,10*65536+40
   mov edx,keymap
   mov esi,128
   mov ecx,cl_White
   mov eax,4
   mcall
end if
   ret

; Proc for calculate active objects on screen
; eax - empty object type = 0
endtest:
   xor eax,eax
   cmp dword [edi+8],eax  ; object is empty ?
   je  is_free
   inc [objects_num]
is_free:
   ret

; Proc for get offest to current level wave information
get_wave_info_offset:
   mov ebp,[level_wave]
   shl ebp,3           ; ebp = ebp*8; 8 - lenght of one string in levels array
   add ebp,levels      ; ebp = offset to string in levels array
   ret

; time_bwns - time before next wave start
; yrmax - y random position maximum value
macro objects_wave time_bnws, yrmax, gships, bships, asteroids, boxes{
   dw time_bnws, yrmax
   db gships, bships, asteroids, boxes
}

level_wave dd 0
; this array may optimized
levels:
; for game not ended at start, each level must have objects set at start (1)
   ; test pattern
if debug_scenario = 1
   ; two levels for debug game
   objects_wave   1,   10,  1,  2,  4,  8   ; objset at start
   objects_wave   0,    0,  0,  0,  0,  0
   objects_wave   1,   10,  3,  3,  3,  3   ; objset at start
   objects_wave   0,   -1,  0,  0,  0,  0
else
   ; level 1
   objects_wave   1, 4000,  3, 10, 10,  0    ; objset at start
   objects_wave 800, 2000,  5,  3,  5,  3
   objects_wave 400, 2000,  3,  7,  5,  3
   objects_wave 400,  800,  3,  5,  3,  0
   objects_wave   0,    0,  0,  0,  0,  0    ; end of level
   ; level 2
   objects_wave   1, 4000, 10, 40,  0,  8    ; objset at start
   objects_wave 400, 4000, 10, 10, 20,  6
   objects_wave 400, 2000,  0, 20, 10,  2
   objects_wave 400,  400, 10, 10, 20,  0
   objects_wave   0,    0,  0,  0,  0,  0    ; end of game
   ; level 3
   objects_wave   1,  800,  0,  0,  5,  5   ; objset at start
   objects_wave 500, 2000,  4, 20, 30,  0
   objects_wave 500, 2000,  4, 20,  0,  8
   objects_wave 500, 2000, 10,  0,  0,  4
   objects_wave 500, 4000,  0, 30,  0,  0
   objects_wave 400,  400,  3,  5, 15,  0
   objects_wave 400,  400,  0,  0, 10,  0
   objects_wave   0,   -1,  0,  0,  0,  0    ; end of level
end if

;***********************************
;         In game keyboard
;***********************************

ingame_keyboard:
   window_get_key  ; read key (eax=2)
   cmp al,0
   jne this_is_hotkey
   ; ah - contain scan code, al = 0
   shl eax,16
   shr eax,24      ; equal shr eax,8 + and eax,0x0FF
   ; eax - contain scan code

   cmp al,key_P+128
   jne not_P_key_up_scan_code
   not [pause_on]
not_P_key_up_scan_code:

if debug_mode = 1
   cmp al,key_E    ; player hp = player hp / 2
   jne no_hp_test
   shr [player_hp],1
no_hp_test:
   cmp al,key_A    ; player hp = 10000
   jne no_hp_up
   mov [player_hp],10000
no_hp_up:
   cmp al,key_W    ; Run final screen
   je  set_final_screen_window_from_call
end if

   ; Keyboard array update, needs sub state
   cmp al,01111111b
   ja  key_released
   mov byte [keymap+eax],'1'  ; If scan code of key down
   jmp key_pressed
key_released:
   and al,01111111b
   mov byte [keymap+eax],0    ; If scan code of key up
key_pressed:
this_is_hotkey:
   ret

;**********************
; Final screen process
;**********************

set_final_screen_window_from_call:
   add esp,4    ; Remove call address from stack
set_final_screen_window:
   lea edx,[menu_keyboard]
   mov ebx,1
   lea ecx,[draw_final_screen_window]
   jmp set_draw_proc

you_won_text: db 'YOU WON!',0

draw_final_screen_window:
   draw_image 5,24,canvas
   logo_font_size equ 5
   logo_x = 5+(screen_w/2)-(6*logo_font_size*8/2)
   logo_y = screen_h/16*5
   draw_label logo_x,logo_y,you_won_text,cl_White+((logo_font_size-1) shl 24)
   ;image_draw_label canvas,200,8,'YOU WON!',cl_White
   draw_button 5,(screen_w/2)-40+5,300,80,14,'BACK TO MENU',cl_DarkRed,cl_Black

   clear_buffer canvas+8,canvas_end-canvas-8,7

   image_draw_acimage canvas,ship_img,(screen_w/2)-16,220,cl_Black
   array_processing particles,draw_particles
   array_processing particles,move_particles    ; move particles
   array_processing particles,remove_outofscr_particles     ; del_outscreen_particles

try_to_make_firework:
   inc [next_wave_timer]
   cmp [next_wave_timer],30
   jna no_firework
   mov [next_wave_timer],0   ; reset firework timer before make firework
   random screen_w-60,eax
   mov ebx,eax
   random screen_h-60,eax
   mov edx,eax
   mov ecx,8                 ; how much particles make in one fire explode
next_star:
   array_find particles,find_empty_object
   jc  close_app
   mov [edi],ebx        ; random x position
   mov [edi+4],edx      ; random y position
   mov [edi+8],dword 3  ; type of partice = 3. final screen particle
rerand:
   random 5,eax
   sub eax,2
   jz  rerand         ; eax = -2...2 exclude 0
   mov [edi+12],eax   ; x velocity
rerand2:
   random 7,eax
   sub eax,3
   jz  rerand2        ; eax = -3...3 exclude 0
   mov [edi+16],eax   ; y velocity
   dec ecx
   jnz next_star
no_firework:
   ret

;***********************
;       Help menu
;***********************

set_help_window:
   lea edx,[menu_keyboard]
   mov ebx,-1
   lea ecx,[draw_help_window]
   jmp set_draw_proc

draw_help_window:
   ; draw background and gray rectangle for label
   ;draw_frect canvas size cl_Black
   draw_frect 40,50,580,380,cl_Grey

   ; draw labels
   mov ebp,4*7+3  ; Set value to labels counter
   mov ebx,180*65536+90
   mov edx,helptext
   mov esi,50
   mov ecx,cl_White+(10000000b shl 24)
draw_next_string:
   mov eax,4
   mcall        ; Draw label
@@:
   mov al,[edx] ; Loop for find next zero
   inc edx
   cmp al,0
   jne @b

   add ebx,10   ; Incrase y position of label
   dec ebp      ; Decrase labels counter
   jnz draw_next_string

   ; draw images of space objects
   mov eax,90
   mov ecx,7
@@:
   mov esi,[(img_offset-4)+ecx*4]
   pushad
   draw_image 90,eax,esi
   popad
   add eax,40
   dec ecx
   jnz @b

   draw_button 5,500,400,80,14,'< BACK',cl_DarkRed,cl_Black
   ret

; Offset to images showed in help screen in reverse sequence
img_offset:
dd box_img,asteroid_img,gwarship_img,bwarship_img
dd plasma1_img,laser_img,ship_img

helptext:
   db 'Phoenix - player ship',0
   db 'Controls:',0
   db 'Left or Right Arrows for move ship, P - button for pause',0
   db 0
   db 'Laser gun',0
   db 'Recharge fast, speed fast. Projectile speed is medium',0
   db 'Press Space button for shoot',0
   db 0
   db 'Plasma gun',0
   db 'Recharge slow, reload fast. Projectile speed is fast',0
   db 'Press Up button for shoot or Down button for make Nuke',0
   db 0
   db 'Blue warship',0
   db 'Moving speed is fast',0
   db 'Armed with plasma bombs',0
   db 0
   db 'Green warship',0
   db 'Moving speed is medium',0
   db 'Armed with laser gun',0
   db 0
   db 'Asteroid',0
   db 'Is not destructable dangeros object!',0
   db 'Collision with asteroid damage ship to much',0
   db 0
   db 'Repear Box',0
   db 'Shield pack. Shield +5, Score +30',0
   db 'Take on board for shield level up!',0
   db 0,0,0
   db 'Developed by Pavlushin Evgeni 2004',0

; ****************************************
;          GLOBAL DRAW WINDOW
; ****************************************

draw_window:
   window_begin_draw
   mcall 0, <40, screen_w+9>, <40, screen_h+24+4>, 0x14000000,, wtitle
   window_end_draw
   ret

; ****************************************
;           GAME PROCEDURE AREA
; ****************************************

; Procedure for add ships to scene
; cl - number of ships which need to add 0..255
; ebp - offset to level string
add_objects:
   ; unpack values from ebx
   xor eax,eax
   mov al,ch
   mov [shiptype],eax
   mov al,bl
   mov [yaccel],eax
   mov al,bh
   mov [ymoving],eax
   shr ebx,16
   mov al,bl
   mov [xaccel],eax
   mov al,bh
   mov [xmoving],eax
next_ship:
   cmp cl,0
   je  no_ships    ; if ships quantity = 0, exit from proc
   push ecx
   push ebp
   ; find empty slot in space objects array
   array_find objects,find_empty_object
   jc  close_app
   ; edi = offset to empty place in array
   mov eax,[shiptype]
   mov dword [edi+8],eax   ; store ship type
   ; Randomize x position
   random screen_w-32,eax
   mov [edi],eax
   ; Randomize y position
   pop ebp
   mov ax,[ebp+2]  ; get max range
   cwde            ; extend ax to eax
   random eax,eax
   neg eax
   mov [edi+4],eax
   ; Randomize x moving
   random [xmoving],eax
   sub eax,[xaccel]
   mov [edi+12],eax
   ; Randomize y moving
   random [ymoving],eax
   add eax,[yaccel]
   mov [edi+16],eax
   pop ecx
   dec cl
   jnz next_ship
no_ships:
   ret

; search empty slot in object array
find_empty_object:
   cmp [edi+8],dword 0  ; if object type == 0 then it empty
   je  is_finded        ; empty object is finded set CF = 0
; find_next
   stc     ; CF = 1
   ret
is_finded:
   clc     ; CF = 0
   ret

; Try to draw particle from particle array
draw_particles:
   mov ecx,[edi+8]         ; ecx - type of particle
   cmp ecx,0               ; if type == 0 then do not draw object
   je  return
   mov eax,laser_img
   cmp ecx,Laser           ; this is laser particle
   je  draw_space_object
   mov eax,plasma2_img
   cmp ecx,StaticPlasma    ; particle type for final screen animation
   je  draw_space_object
   random 3,ebx            ; if else this is Plasma particle
   mov eax,plasma1_img
   dec ebx
   jz draw_space_object
   mov eax,plasma2_img
   dec ebx
   jz draw_space_object
   mov eax,plasma3_img
   jmp draw_space_object

; Draw space objects from array
draw_objects:
   mov ecx,[edi+8]         ; ecx = [edi+8] - type of ship
   cmp ecx,0               ; if type of ship == 0 then not draw it
   je  return
   cmp ecx,(ot_end-object_type)/4+1
   jae return              ; if type out of range ignore it
   mov edx,[edi+4]         ; edx = [edi+4] - y position of ship
   cmp edx,screen_h-40
   jg  return
   cmp edx,0               ; do not draw object if it y position is out of screen
   jl  return
   mov eax,[(object_type-4)+ecx*4]   ; -4 when types starts from 1 instead 0
draw_space_object:
   image_draw_acimage canvas,eax,dword [edi],dword [edi+4],cl_Black
return:
   ret

object_type: dd bwarship_img,gwarship_img,asteroid_img,box_img
ot_end:

; Update (move) particles (laser,plasma)
move_particles:
   xor eax,eax
   cmp [edi+8],eax      ; Is object not empty ?
   je this_is_empty_particle
move_particle:
   mov eax,[edi+12]
   add [edi],eax        ; objectx + [edi+12]
   mov eax,[edi+16]
   add [edi+4],eax      ; objecty + [edi+16]
this_is_empty_particle:
   ret

;  update (move) space objects (ships,asteroids,boxes)
move_objects:
   xor eax,eax
   cmp [edi+8],eax
   je  object_is_empty
   ;call move_particle
   mov eax,[edi+12]
   add [edi],eax
   mov eax,[edi+16]
   add [edi+4],eax

   ; Do not allow object to go out of screen from right side
   mov eax,screen_w-32     ; eax = right side of screen
   cmp dword [edi],eax
   jng right_side_ok
   mov dword [edi],eax
   neg dword [edi+12]
   jmp left_side_ok
right_side_ok:
   ; Do not allow object to go out of screen from left side
   xor eax,eax             ; eax = 0 - left side of screen
   cmp dword [edi],eax
   jnl left_side_ok
   mov dword [edi],eax
   neg dword [edi+12]
left_side_ok:
   ; If object out of screen remove it
   cmp dword [edi+4],screen_h;-40
   jng y_ok
   mov dword [edi+8],0     ; Delete object
   ret
y_ok:
   cmp dword [edi+8],GreenWarShip  ; Object is green enemy ship?
   jne no_grs
   mov eax,dword [edi+4]   ; eax = y position of enemy ship
   ; alternative way is use random for shoot
   cmp eax,100
   jna no_grs
   cmp eax,103
   jna grs
   cmp eax,200
   jna no_grs
   cmp eax,203
   jna grs
   cmp eax,300
   jna no_grs
   cmp eax,303
   ja  no_grs
grs:
   ; invert y moving direction and make shoot
   neg dword [edi+12]
   mov [temp],edi
   array_find particles,find_empty_object
   jc  close_app
   mov esi,[temp]      ; edi contains address to free element
   mov [edi+8],dword 1
   mov [edi+12],dword 0
   mov [edi+16],dword 10
   jmp set_particle_position
no_grs:
   cmp dword [edi+8],BlueWarShip  ; object is blue enemy ship ?
   jne no_bls
   mov ecx,dword [edi+4]
   cmp ecx,50
   jna no_bls
   cmp ecx,64
   jna bls
   cmp ecx,100
   jna no_bls
   cmp ecx,114
   jna bls
   cmp ecx,150
   jna no_bls
   cmp ecx,164
   ja  no_bls
bls:
   ; drop plasma mine
   mov [temp],edi
   array_find particles,find_empty_object
   jc  close_app
   mov esi,[temp]
   mov [edi+8],dword 2
   mov [edi+12],dword 0
   mov [edi+16],dword 5
set_particle_position:
   mov eax,[esi]
   mov [edi],eax       ; Particle x = Ship x
   mov eax,[esi+4]
   mov [edi+4],eax     ; Partcle y = Ship y
no_bls:
object_is_empty:
   ret

; Remove particles that have gone out off screen
remove_outofscr_particles:
   cmp dword [edi+4],40  ; test y position
   jl  del
   cmp dword [edi+4],screen_h-40
   jg  del
   cmp dword [edi],0     ; x test used for plasma shoots
   jl  del
   cmp dword [edi],screen_w-32
   jg  del
   ret        ; do not delete
del:
   xor eax,eax
   mov [edi+8],eax ; [edi+8] = 0
not_del:
   ret

objects_and_particles_hit_handling:
   xor eax,eax
   cmp [esi+8],eax
   je  no_hit
   cmp [edi+8],eax    ; If object is empty skip crush test
   je  no_hit
   cmp [esi+16],eax
   jg  no_hit

   mov eax,[esi]
   shl eax,16
   mov ax,word [esi+4]
   mov ebx,32*65536+32
   mov ecx,[edi]
   shl ecx,16
   mov cx,word [edi+4]
   mov edx,32*65536+32

   game_collision_2d eax,ebx,ecx,edx
   jnc no_hit

   cmp dword [edi+8],GreenWarShip
   jne not_grship
   inc [gships_destroyed]
   add [score],30
   jmp remove_object_and_particle
not_grship:
   cmp dword [edi+8],BlueWarShip
   jne not_blship
   inc [bships_destroyed]
   add [score],20
   jmp remove_object_and_particle
not_blship:
   cmp dword [edi+8],Asteroid
   jne not_asteroid
   cmp dword [edi+16],1 ; Asteroid have minimal speed?
   je  remove_only_particle
   dec dword [edi+16]   ; Decrase speed of asteroid
   jmp remove_only_particle
not_asteroid:
remove_object_and_particle:  ; When hit to ship or box
   mov [edi+8],dword 0
remove_only_particle:        ; When hit to asteroid
   mov [esi+8],dword 0
no_hit:
   ret

player_and_objects_collision_handling:
   cmp [edi+8],dword 0
   je  no_obj_cr

   mov eax,[shipx]
   shl eax,16
   mov ax,word [shipy]
   mov ebx,32*65536+32
   mov ecx,[edi]
   shl ecx,16
   mov cx,word [edi+4]
   mov edx,32*65536+32

   game_collision_2d eax,ebx,ecx,edx
   jnc no_obj_cr
   cmp dword [edi+8],Box  ; if box
   jne no_fbox
   add [player_hp],5
   add [score],50
   mov [edi+8],dword 0  ; delete object
   inc [boxes_taken]
   ret
no_fbox:
   sub [player_hp],16
   mov [edi+8],dword 0  ; delete object
no_obj_cr:
   ret

player_and_particles_collision_handling:
   xor eax,eax       ; use eax as zero
   cmp [edi+8],eax   ; empty object?
   je  no_gobj_cr
   cmp [edi+16],eax  ; is player ?
   jl  no_gobj_cr

   mov eax,[shipx]
   shl eax,16
   mov ax,word [shipy]
   mov ebx,32*65536+32
   mov ecx,[edi]
   shl ecx,16
   mov cx,word [edi+4]
   mov edx,32*65536+32

   game_collision_2d eax,ebx,ecx,edx
   jnc no_gobj_cr
   sub [player_hp],4
   mov [edi+8],dword 0  ; delete object
no_gobj_cr:
   ret

;**************************
; Player ship shoot procs
;**************************

; Try to make laser shoot
try_to_make_laser_shoot:
   cmp [laser_charge],dword 0
   jne no_laser_shoot     ; laser_shoots is heat, need time for recharge
   cmp [laser_shoots],dword 0
   je  no_laser_shoot     ; Don't shoot when so many laser particles on screen
   array_find particles,find_empty_object  ; edi = offset to emppty object
   jc  close_app          ; ?
   mov eax,[shipx]
   mov [edi],eax           ; 0  = x position of shoot
   mov eax,[shipy]
   mov [edi+4],eax         ; 4  = y position of shoot
   mov [edi+8],dword 1     ; 8  = 1 - laser type
   mov [edi+12],dword 0    ; 12 = 0   - x speed
   mov [edi+16],dword -12  ; 16 = -12 - y speed
   mov [laser_charge],dword 8  ; Reset shoot timer
   dec [laser_shoots]      ; Decrase number of laser projectiles
no_laser_shoot:
   ret

; Try to make plasma shoot
try_to_make_plasma_shoot:
   cmp [plasma_charge],dword 256-16
   jae no_plasma_shoot
   cmp [plasma_shoots],0
   je  no_plasma_shoot
   array_find particles,find_empty_object
   ; edi = off to free element
   jc  close_app     ;?
   mov eax,[shipx]
   mov [edi],eax
   mov eax,[shipy]
   mov [edi+4],eax
   mov [edi+8],dword 2   ; 8 = 2 - plasma
   mov [edi+12],dword 0    ; 12 = 0   - x speed
   mov [edi+16],dword -8   ; 16 = -8 - y speed
   dec [plasma_shoots]     ; Decrase number of plasma projectiles
   add [plasma_charge],dword 8
   cmp [plasma_charge],dword 256
   jna no_plasma_shoot
   mov [plasma_charge],256
no_plasma_shoot:
   ret

; Try to make plasma nuke
try_to_make_plasma_nuke:
   xor eax,eax     ; Use eax as zero
   cmp [plasma_charge],eax
   jne no_plasma_nuke
   cmp [plasma_shoots],eax
   je  no_plasma_nuke
   mov eax,[shipy]
   mov [temp3],eax
   mov [temp2],dword 5
loopx2:
   mov [temp],dword 10
loopx:
   array_find particles,find_empty_object ; edi = offset to empty element
   jc  close_app
   random 25,eax
   mov ebp,eax
   sub ebp,12
   add ebp,[shipx]
   mov [edi],ebp    ; [edi] = random(0..25)-12+shipx
   shr eax,3
   random eax,eax
   neg eax
   add eax,[temp3]
   mov [edi+4],eax
   mov [edi+8],dword 2  ; 8 = 2 - plasma
   random 5,eax
   sub eax,2
   mov [edi+12],eax
   random 7,eax
   sub eax,8
   mov [edi+16],eax
   dec [temp]
   jnz loopx
   sub [temp3],30     ; shipy - 30
   dec [temp2]
   jnz loopx2
   mov [plasma_charge],dword 256  ; Wait for cannon
   sub [plasma_shoots],50 ; -50 plasma bullets after nuke
no_plasma_nuke:
   ret

; DATA AREA
IM_END:
wtitle db 'Phoenix for KOS', 0

score    dd 0    ; player score

; Pause state, if != 0 then game on pause
pause_on dd 0

; Frames countdown timer until start of next wave
; If = 0 then activate to next wave
; next_wave_timer
next_wave_timer dd 0

;gif_file_area ~21500
;gif_file_area2:
;file 'phoenix.gif'
gif_file_area:
file 'objects.gif' ; Include gif file to code

IncludeUGlobals

; Window drawing function delegate (pointer)
draw_proc rd 1
; Keyboard processing function delegate (pointer)
keyboard_proc rd 1

; Counter of objects on screen
objects_num rd 1

player_hp rd 1   ; Health points of player ship
shipx rd 1       ; Player ship x position
shipy rd 1       ; Player ship y position
; guns
laser_shoots  rd 1 ; laser bullets quantity
plasma_shoots rd 1 ; plasma bullets quantity
; Counters of player statistics
gships_destroyed rd 1 ; Number of green ships destroyed by player
bships_destroyed rd 1 ; Number of blue ships destroyed by player
boxes_taken rd 1      ; Number of repair boxes taken by player
; Gun recharge counters
; 0 = fully charged
; 256 = fully uncharged
laser_charge  rd 1 ; Laser gun recharge counter
plasma_charge rd 1 ; Plasma gun recharge counter

; Tempory varibles
temp  rd 1
temp2 rd 1
temp3 rd 1

event_mode rd 1   ; if -1 wait, 0 scan, n - n delay between events

; Tempory varibles for add_objects proc
shiptype rd 1
xmoving  rd 1
ymoving  rd 1
xaccel   rd 1
yaccel   rd 1

; Memory for contain not splitted image
objects_image: rb 8+288*32*3  ;8+256*64*3
; Images sequence extracted from objects_image
ship_img:      rb 8+32*32*3   ; Player red space ship
laser_img:     rb 8+32*32*3   ; Double laser beams
bwarship_img:  rb 8+32*32*3   ; Blue enemy ship
gwarship_img:  rb 8+32*32*3   ; Green enemy ship
asteroid_img:  rb 8+32*32*3   ; Space asteroid
plasma1_img:   rb 8+32*32*3   ; Plasma big flash
plasma2_img:   rb 8+32*32*3   ; Plasma medium flash
plasma3_img:   rb 8+32*32*3   ; Plasma small flash
box_img:       rb 8+32*32*3   ; Repear kit box

; array for storing state of keyboard keys
keymap:        rb 128

particles:
rd max_particles   ; dword = maximum number of particle objects
rd particle_size   ; dword = size of each particle object in bytes
rb max_particles*particle_size

objects:
rd max_objects     ; dword = maximum number of particles
rd object_size     ; dword = size of each object in bytes
rb max_objects*object_size

canvas:
rb 8+(screen_w*screen_h*3)
canvas_end:

; application stack size
align 16
stack_max:
  rb 2048
I_END: