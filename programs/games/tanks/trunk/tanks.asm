;************************************************************
;*GAME TANKS CRETED BY ANDREW_PROGRAMMER AKA ANDREY IGNATYEV*
;*********************/8/2005********************************
;Leency aka Lipatov Kirill 19.07.2011: update and some fixes
;                      v1.02

; bug: window can't end redraw untill new game wasn't start
; if press close button it goes to infinite loop

use32
org 0x0
include 'lang.inc'
include '../../../macros.inc'
  db  'MENUET01'
  dd  0x1
  dd  START
  dd  I_END
  dd  0x4000+640*400*3+50*20*20*3+512+640+1+24*20*20*3+1
  dd  0x4000
  dd  0x0
  dd  0x0
START:
   mov eax,40
   mov ebx,111b
   mcall
;----------------------------------------------------------
;---------load all sprites from arrays to memory-----------
;----------------------------------------------------------
   and [number_sprite],0
   xor eax,eax
nextfile:
   mov ebx,[spisok+4*eax]
   mov ecx,50
   mov esi,ebx
   add esi,2
   mov edi,0x4000+(640*400*3)+(50*20*20*3)+10
   rep movsb ;copy palitra to memory
   mov esi,ebx
   add esi,52
   xor ecx,ecx
   mov cl,byte [ebx]
   mov edi,0x4000+(640*400*3)+(50*20*20*3)+512
   push eax
   call unpakin
   pop eax
   mov ecx,20*20
   mov esi,0x4000+(640*400*3)+(50*20*20*3)+512
   mov edi,[number_sprite]
   imul edi,3*20*20
   add edi,0x4000+(640*400*3)
   unp:
      xor ebx,ebx
      mov bl,byte[esi]
      lea ebx,[ebx+ebx*2]
      add ebx,0x4000+(640*400*3)+(50*20*20*3)+10
      mov edx,[ebx]
      mov [edi],edx
      add esi,1
      add edi,3
      dec ecx
   jnz unp
   inc [number_sprite]
   inc eax
   cmp [number_sprite],26;total number of pictures equal 26
   jne nextfile
;----------------------------------------------------------
;------rotate sprites of tanks to pi/2---------------------
;----------------------------------------------------------
   mov [sp_pos],0x4000+(640*400*3)+18*20*20*3
   and [num_blocks],0
next_block_rotate:
   mov [number_sprite],0
next_sprite_rotate:
   mov [x],19
   and [y],0
rotate_90:
   mov ebx,[number_sprite]
   mov esi,[y]
   imul esi,60
   imul ebx,1200
   add esi,ebx	;esi=number_sprite*1200+y*60
   add esi,dword [sp_pos]
   mov edi,0x4000+(640*400*3)+(50*20*20*3)
   mov ecx,15
   rep movsd
   mov edi,[number_sprite]
   imul edi,1200
   add edi,dword [sp_pos]
   add edi,8*20*20*3
   mov ebx,[x]
   lea ebx,[ebx+2*ebx]
   add edi,ebx
   mov esi,0x4000+(640*400*3)+(50*20*20*3)
   mov ecx,20*3
vertical:
   xor eax,eax
   mov al,byte[esi+2]
   mov [edi+2],al
   xor eax,eax
   mov ax,word[esi]
   mov [edi],ax
   add edi,60
   add esi,3
   sub ecx,3
   jnz vertical
   dec [x]
   inc [y]
   cmp [x],-1
   jne rotate_90
   inc [number_sprite]
   cmp [number_sprite],9
   jne next_sprite_rotate
   add [sp_pos],8*20*20*3
   inc [num_blocks]
   cmp [num_blocks],3
   jne next_block_rotate
;---------------------------------------------------------
restart_level:
   call drawwin
;Start game(demo scen)
   mov [y],18
@1:
 mov [x],30
 @2:
    mov esi,[x]
    mov edi,[y]
    imul esi,20*3
    imul edi,20*(640*3)
    add edi,esi
    add edi,0x4000
    mov esi,0x4000+(640*400*3)+1200
    mov [counter],20
    mov eax,esi
    mov ebx,edi
     @3:
	mov esi,eax
	mov edi,ebx
	mov ecx,15
	rep movsd
	add eax,20*3
	add ebx,640*3
	dec [counter]
     jnz @3
    dec [x]
 jnz @2
dec [y]
jnz @1
;draw script ******TANKS*******
   mov eax,54
@11:
   mov esi,5
   mov edi,80
   xor ebx,ebx
   xor edx,edx
   mov bl,byte[TANKS_script+2*eax]   ;dx
   mov dl,byte[TANKS_script+2*eax+1] ;dy
   imul ebx,20
   imul edx,20
   add esi,ebx
   add edi,edx
   lea esi,[esi+2*esi]
   lea edi,[edi+2*edi]
   imul edi,640
   add edi,esi
   add edi,0x4000
   mov esi,0x4000+(640*400*3)
   mov ebx,esi
   mov edx,edi
   mov [counter],20
   @22:
   mov esi,ebx
   mov edi,edx
   mov ecx,15
   rep movsd
   add ebx,20*3
   add edx,640*3
   dec [counter]
   jnz @22
   dec eax
   cmp eax,-1
   jne @11
   mov eax,7
   mov ebx,0x4000
   mov ecx,640*65536+400
   mov edx,0*65536+20
   mcall
   call menu
;----------------------------
new_level:
   call drawwin
   mov [SpriteInfo+72+8],dword 100  ;Xo
   mov [SpriteInfo+72+36],dword 100 ;Yo
   mov edx,[number_level]
   call load_level
;--------------------------
   and [strike_action],0
   and [bazas],0
   and [y],0
   mov eax,8
next_y:
   and [x],0
 next_x:
   mov esi,[y]
   shl esi,5
   add esi,[x]
   add esi,0x4000+(640*400*3)+(50*20*20*3)+512
   xor ebx,ebx
   mov bl,byte[esi]
   cmp bl,7
   jne no_baza
   mov ecx,[x]
   mov edx,[y]
   inc ecx
   inc [bazas]
   add eax,4
   imul ecx,20
   imul edx,20
   mov [SpriteInfo+9*eax+0],dword 3
   mov [SpriteInfo+9*eax+4],dword 1
   mov [SpriteInfo+9*eax+8],dword ecx
   mov [SpriteInfo+9*eax+12],dword edx
   mov [SpriteInfo+9*eax+16],dword 5
   mov [SpriteInfo+9*eax+28],dword -5
   mov edx,eax
   shr edx,2
   mov [LifesTanks+edx],byte 3;three lifes
   mov [PulyTanks+edx],byte 1 ;can draw animation of "puly"
   no_baza:
   inc [x]
   cmp [x],32
   jne next_x
   inc [y]
   cmp [y],20
   jne next_y
   mov [LifesPlayer],3*4
   and [score],0
;----------------------------------------------------------
;--------------------main loop-----------------------------
;----------------------------------------------------------
maincycle:
	 ;save fon
	 mov eax,23*4
	 SaveFonForSprites:
	 mov ebx,[SpriteInfo+9*eax+4]
	 test ebx,ebx			  ;if ebx<>0 then can save
	 jz NoSaveFon
	 mov esi,[SpriteInfo+9*eax+8]	  ; cordinat x
	 mov edi,[SpriteInfo+9*eax+12]	  ; cordinat y
	 mov edx,eax
	 shr edx,2
	 call SaveFon
	 NoSaveFon:
	 sub eax,4
	 cmp eax,-4
	 jne SaveFonForSprites
	 ;put sprites
	 mov eax,23*4
	 PutSprites:
	 mov ebx,[SpriteInfo+9*eax+4]
	 test ebx,ebx
	 jz NoPutSprite
	 and ebx,10000b
	 test ebx,ebx	; move to transparent shablon ?
	 jnz NoPutSprite
	 mov esi,[SpriteInfo+9*eax+8]	  ; x
	 mov edi,[SpriteInfo+9*eax+12]	  ; y
	 mov edx,[SpriteInfo+9*eax+0]	  ;number sprite in sprites table
	 shl edx,2
	 add edx,[SpriteInfo+9*eax+24]	  ;edx=4*NumSprites+phas
	 call PutSprite
	 NoPutSprite:
	 sub eax,4
	 cmp eax,-4
	 jne PutSprites
	 ;put image
	 call clock
	 mov ebx,[time]
	 sub ebx,[time_frame_old]
	 cmp ebx,5
	 jl NoPutScreen
	 mov ebx,[time]
	 mov [time_frame_old],ebx
	 mov eax,7
	 mov ebx,0x4000
	 mov ecx,640*65536+400
	 mov edx,0*65536+20
	 mcall
	 NoPutScreen:
	 ;put fon
	 mov eax,23*4
	 PutFonForSprites:
	 mov ebx,[SpriteInfo+9*eax+4]
	 test ebx,ebx
	 jz NoPutFon
	 mov esi,[SpriteInfo+9*eax+8]
	 mov edi,[SpriteInfo+9*eax+12]
	 mov edx,eax
	 shr edx,2
	 call PutFon
	 NoPutFon:
	 sub eax,4
	 cmp eax,-4
	 jne PutFonForSprites
	 ;change phas(if need)
	 mov eax,23*4
	 ChangePhasSprites:
	 push eax
	 call clock
	 pop eax
	 mov ebx,[SpriteInfo+9*eax+16] ;time of animation
	 mov ecx,[SpriteInfo+9*eax+20] ;time of last frame
	 mov edx,[time]
	 sub edx,ecx
	 cmp edx,ebx
	 jl no_change_phas
	 mov ebx,[time]
	 mov [SpriteInfo+9*eax+20],ebx ;save new time
	 ;------------------------------
	 mov esi,[SpriteInfo+9*eax+8]
	 mov edi,[SpriteInfo+9*eax+12]
	 mov ecx,[SpriteInfo+9*eax+28]
	 mov edx,[SpriteInfo+9*eax+32]
	 add esi,ecx		       ;x=x+dx
	 add edi,edx		       ;y=y+dy
	 mov [SpriteInfo+9*eax+8],dword esi
	 mov [SpriteInfo+9*eax+12],dword edi
	 ;-------------------------------
	 mov edx,[SpriteInfo+9*eax+24] ;phas
	 add edx,1
	 cmp edx,4
	 jne no_limit_phas
	 xor edx,edx
	 mov [SpriteInfo+9*eax+24],dword edx
	 mov [SpriteInfo+9*eax+4],dword edx
	 jmp no_change_phas
	 no_limit_phas:
	 mov [SpriteInfo+9*eax+24],dword edx
	 no_change_phas:
	 sub eax,4
	 cmp eax,-4
	 jne ChangePhasSprites
	 ;-------------------------------------------------
	 ;----------------keys-----------------------------
	 ;-------------------------------------------------
	 mov eax,23
	 mov ebx,3
	 mcall
	 cmp eax,1
	 jne keypressed
	 call drawwin
	 jmp action
	 keypressed:
	 cmp eax,2
	 jne action
	 and [_dx],0
	 and [_dy],0
	 mov eax,2
	 mcall
	 shr eax,8
	 ;---------
	 cmp eax,32
	 jne key2
	 cmp [strike_action],0
	 jne action
	 mov [SpriteInfo+36+4],dword 1 ;can draw sprite "puly"
	 mov [strike_action],1	       ;sprite is active
	 mov eax,[SpriteInfo+72+8]
	 mov ebx,[SpriteInfo+72+12]
	 mov ecx,[SpriteInfo+72+28]
	 mov edx,[SpriteInfo+72+32]
	 add eax,ecx
	 add ebx,edx
	 mov [SpriteInfo+36+8],dword eax
	 mov [SpriteInfo+36+12],dword ebx
	 jmp action
    key2:cmp eax,176
	 jne key3
	 mov [SpriteInfo+8*9+0],dword 8
	 mov [SpriteInfo+8*9+4],dword 1
	 mov [SpriteInfo+8*9+28],dword -5
	 mov [SpriteInfo+8*9+32],dword 0
	 mov [_dx],0
	 mov [_dy],10
	 cmp [strike_action],0
	 jne action
	 mov [SpriteInfo+36+28],dword -5
	 mov [SpriteInfo+36+32],dword 0
	 jmp action
    key3:cmp eax,179
	 jne key4
	 mov [SpriteInfo+8*9+0],dword 4
	 mov [SpriteInfo+8*9+4],dword 1
	 mov [SpriteInfo+8*9+28],dword 5
	 mov [SpriteInfo+8*9+32],dword 0
	 mov [_dx],15
	 mov [_dy],10
	 cmp [strike_action],0
	 jne action
	 mov [SpriteInfo+36+28],dword 5
	 mov [SpriteInfo+36+32],dword 0
	 jmp action
    key4:cmp eax,178
	 jne key5
	 mov [SpriteInfo+8*9+0],dword 2
	 mov [SpriteInfo+8*9+4],dword 1
	 mov [SpriteInfo+8*9+28],dword 0
	 mov [SpriteInfo+8*9+32],dword -5
	 mov [_dx],10
	 mov [_dy],1
	 cmp [strike_action],0
	 jne action
	 mov [SpriteInfo+36+28],dword 0
	 mov [SpriteInfo+36+32],dword -5
	 jmp action
    key5:cmp eax,177
	 jne key6
	 mov [SpriteInfo+8*9+0],dword 6
	 mov [SpriteInfo+8*9+4],dword 1
	 mov [SpriteInfo+8*9+28],dword 0
	 mov [SpriteInfo+8*9+32],dword 5
	 mov [_dy],15
	 mov [_dx],10
	 cmp [strike_action],0
	 jne action
	 mov [SpriteInfo+36+28],dword 0
	 mov [SpriteInfo+36+32],dword 5
	 jmp action
    key6:cmp eax,27
	 jne action
	 mov eax,-1
	 mcall
action:
	 mov [SpriteInfo+72+4],dword 1
	 mov [SpriteInfo+72+16],dword 5 ;usal speed of tank
	 mov esi,[SpriteInfo+8*9+8]   ;cordinat x of sprite
	 mov edi,[SpriteInfo+8*9+12]  ;cordinat y of sprite
	 mov ecx,[SpriteInfo+72+28]
	 mov edx,[SpriteInfo+72+32]
	 add ecx,[_dx]
	 add edx,[_dy]
	 add esi,ecx		      ;x=x+_dx+dx
	 add edi,edx		      ;y=y+_dy+dy
	 call map_x_y
	 cmp ecx,6
	 jne no_woter
	 mov [SpriteInfo+72+16],dword 7 ;in water tank slow move
	 no_woter:
	 cmp ecx,4
	 jne no_palma
	 mov [SpriteInfo+72+4],dword 10000b;if bit 4 than transparent sprite
	 no_palma:
	 cmp ecx,8
	 jne no_derevo
	 mov [SpriteInfo+72+4],dword 10000b
	 no_derevo:
	 test edx,edx
	 jnz ani
	 mov [SpriteInfo+8*9+28],dword 0
	 mov [SpriteInfo+8*9+32],dword 0
	 ;-------------------------------------------------
	 ani:
	 mov eax,[strike_action]
	 test eax,eax
	 jz no_anim_strike
	 mov esi,[SpriteInfo+36+8]
	 mov edi,[SpriteInfo+36+12]
	 mov ecx,[SpriteInfo+36+28]
	 mov edx,[SpriteInfo+36+32]
	 add esi,ecx
	 add edi,edx
	 call map_x_y
	 cmp cl,0
	 jne no_stena
	 mov [ebx],byte 2 ;trava
	 jmp bum
	 no_stena:
	 cmp cl,1
	 jne no_brony
	 jmp bum
	 no_brony:
	 cmp cl,7
	 jne anim_action_puly
	 sub [bazas],1
	 mov [ebx],byte 2 ;trava
	 bum:
	 mov esi,[SpriteInfo+36+8]
	 mov edi,[SpriteInfo+36+12]
	 mov [SpriteInfo+4],dword 1
	 mov [SpriteInfo+8],dword esi
	 mov [SpriteInfo+12],dword edi
	 mov [SpriteInfo+36+4],dword 0
	 mov [end_bum],1
	 jmp no_anim_strike
	 anim_action_puly:
	 mov [SpriteInfo+36+4],dword 1
	 no_anim_strike:
	 ;-------------------------------------------------
	 ;проверяем попала ли пуля в противников
	 mov esi,[SpriteInfo+4*9+8]
	 mov edi,[SpriteInfo+4*9+12]
	 mov [x],esi ;координата x пули
	 mov [y],edi ;координата y пули
	 mov eax,3*4
	 bum_tank:
	 mov ebx,eax
	 shr ebx,2
	 xor ecx,ecx
	 mov cl,byte [LifesTanks+ebx]
	 cmp ecx,0
	 jz no_strike
	 mov esi,[SpriteInfo+9*eax+8] ;x
	 mov edi,[SpriteInfo+9*eax+12] ;y
	 sub esi,[x]
	 sub edi,[y]
	 cmp esi,5
	 ja no_strike
	 cmp edi,5
	 ja no_strike
	 add [score],5
	 cmp [score],20
	 jl no_bonus
	 add [LifesPlayer],4
	 and [score],0
	 no_bonus:
	 dec ecx;,1
	 mov [LifesTanks+ebx],cl ;life=life-1
	 mov esi,[x]
	 mov edi,[y]
	 mov [SpriteInfo+4],dword 1
	 mov [SpriteInfo+8],dword esi
	 mov [SpriteInfo+12],dword edi
	 mov [SpriteInfo+36+4],dword 0
	 mov [end_bum],1
	 no_strike:
	 add eax,4
	 cmp eax,10*4
	 jne bum_tank
	 ;-------------------------------------------------
	 xor eax,eax
	 mov al,[end_bum]
	 test eax,eax
	 jz no_end_strike
	 mov eax,[SpriteInfo+24]
	 cmp eax,3
	 jne no_end_strike
	 and [strike_action],0
	 and [end_bum],0
	 mov edx,[number_level]
	 mov ecx,15
	 call load_level
	 no_end_strike:
	 ;----------------------------
	 ;проверяем,попали ли противники в игрока
	 mov eax,3*4
	 mov esi,[SpriteInfo+72+8]
	 mov edi,[SpriteInfo+72+12]
	 mov [x],esi
	 mov [y],edi
	 strike_to_player:
	 mov ebx,eax
	 add ebx,28
	 mov esi,[SpriteInfo+9*ebx+8]
	 mov edi,[SpriteInfo+9*ebx+12]
	 sub esi,[x]
	 sub edi,[y]
	 cmp esi,5
	 ja no_strike_to_player
	 cmp edi,5
	 ja no_strike_to_player
	 mov esi,[x]
	 mov edi,[y]
	 mov [SpriteInfo+9*ebx+4],dword 0
	 add ebx,28
	 mov [SpriteInfo+9*ebx+4],dword 1
	 mov [SpriteInfo+9*ebx+8],dword esi
	 mov [SpriteInfo+9*ebx+12],dword edi
	 sub [LifesPlayer],1
	 cmp [LifesPlayer],0
	 jne no_game_over
	 call end_game
	 jmp restart_level
	 no_game_over:
	 no_strike_to_player:
	 add eax,4
	 cmp eax,10*4
	 jne strike_to_player
	 ;----------------------------
	 xor eax,eax
	 mov al,[bazas]
	 test eax,eax
	 jnz no_end_level
	 call you_won
	 inc [number_level]
	 cmp [number_level],25
	 jne no_end_game
	 mov eax,-1
	 mcall
	 no_end_game:
	 jmp new_level
	 no_end_level:
	 ;-------------------------------------------------
	 ;------------ logic of tanks----------------------
	 ;-------------------------------------------------
	 mov eax,12
	 next_bad_tank:
	 mov ebx,eax
	 shr ebx,2
	 xor ecx,ecx
	 mov cl,byte[LifesTanks+ebx]
	 test ecx,ecx
	 jz no_action_bad_tank
	 mov ebx,[SpriteInfo+9*eax+4]
	 test ebx,ebx
	 jnz no_action_bad_tank
	 ;---------------------
	 mov esi,[SpriteInfo+9*eax+8]
	 mov edi,[SpriteInfo+9*eax+12]
	 mov [x],esi
	 mov [y],edi
	 mov ecx,[SpriteInfo+9*eax+28]
	 mov edx,[SpriteInfo+9*eax+32]
	 ;смотрим что впереди танка-противника
	 cmp ecx,0
	 ja more_null_x
	 lea ecx,[ecx+2*ecx]
	 jmp test_y
	 more_null_x:
	 shl ecx,2
	 add ecx,15
	 test_y:
	 cmp edx,0
	 ja more_null_y
	 lea edx,[edx+2*edx]
	 jmp add_numbers
	 more_null_y:
	 shl edx,2
	 add edx,15
	 add_numbers:
	 ;--------------------
	 add esi,ecx
	 add edi,edx
	 call map_x_y
	 cmp ecx,4
	 jne no_palma_p
	 mov [SpriteInfo+9*eax+4],dword 10000b
	 no_palma_p:
	 cmp ecx,8
	 jne no_derevo_p
	 mov [SpriteInfo+9*eax+4],dword 10000b
	 no_derevo_p:
	 ;если на пути танка препятствие,то надо изменить
	 ;направление движения
	 test edx,edx
	 jnz lab1
	 ;проверяем - не попал ли танк в тупик(3 напрвления заняты)
	 and [tupik],0
	 mov esi,[x]
	 mov edi,[y]
	 add esi,(20+15)
	 add edi,10
	 call map_x_y
	 test edx,edx
	 jnz direct1
	 inc [tupik]
	 direct1:
	 mov esi,[x]
	 mov edi,[y]
	 add esi,10
	 add edi,(20+15)
	 call map_x_y
	 test edx,edx
	 jnz direct2
	 inc [tupik]
	 direct2:
	 mov esi,[x]
	 mov edi,[y]
	 add esi,-15
	 add edi,10
	 call map_x_y
	 test edx,edx
	 jnz direct3
	 inc [tupik]
	 direct3:
	 mov esi,[x]
	 mov edi,[y]
	 add esi,10
	 add edi,-15
	 call map_x_y
	 test edx,edx
	 jnz direct4
	 inc [tupik]
	 direct4:
	 cmp [tupik],3
	 je no_move0
	 ;------------------
	 ;strategy1
	 mov esi,[x]
	 mov edi,[y]
	 add esi,(20+15)
	 add edi,10
	 call map_x_y
	 test edx,edx
	 jnz no_strategy1
	 mov esi,[x]
	 mov edi,[y]
	 add esi,10
	 add edi,-15
	 call map_x_y
	 jnz no_strategy1
	 jmp no_move3 ;going to left
	 no_strategy1:
	 ;------------------
	 mov esi,[x]
	 mov edi,[y]
	 add esi,-15
	 add edi,10
	 call map_x_y
	 test edx,edx
	 jnz no_strategy2
	 mov esi,[x]
	 mov edi,[y]
	 add esi,10
	 add edi,-15
	 call map_x_y
	 test edx,edx
	 jnz no_strategy2
	 jmp no_move2
	 no_strategy2:
	 ;------------------
	 mov esi,[x]
	 mov edi,[y]
	 add edi,(20+15)
	 add esi,10
	 call map_x_y
	 test edx,edx
	 jnz no_strategy3
	 mov esi,[x]
	 mov edi,[y]
	 add esi,-15
	 add edi,10
	 call map_x_y
	 test edx,edx
	 jnz no_strategy3
	 jmp no_move1
	 no_strategy3:
	 ;-------------------------------------------------
	 no_move0:
	 mov esi,[x]  ; x
	 mov edi,[y] ; y
	 add edi,-15		     ; (y-20) up
	 add esi,10
	 call map_x_y
	 test edx,edx
	 jz no_move1
	 ;вверху свободно - можно двигаться   up
	 mov [SpriteInfo+9*eax+0],dword 3
	 mov [SpriteInfo+9*eax+28],dword 0
	 mov [SpriteInfo+9*eax+32],dword -5
	 jmp lab1
	 no_move1:
	 mov esi,[x]
	 mov edi,[y]
	 add esi,(20+15)		 ;x+20
	 add edi,10
	 call map_x_y
	 test edx,edx
	 jz no_move2
	 ;                                 right
	 mov [SpriteInfo+9*eax+0],dword 5
	 mov [SpriteInfo+9*eax+28],dword 5
	 mov [SpriteInfo+9*eax+32],dword 0
	 jmp lab1
	 no_move2:
	 mov esi,[x]
	 mov edi,[y]
	 add esi,10		      ;x-20
	 add edi,(20+15)
	 call map_x_y
	 test edx,edx
	 jz no_move3
	 ;                                  down
	 mov [SpriteInfo+9*eax+0],dword 7
	 mov [SpriteInfo+9*eax+28],dword 0
	 mov [SpriteInfo+9*eax+32],dword 5
	 jmp lab1
	 no_move3:
	 mov esi,[x]
	 mov edi,[y]
	 add edi,10		    ;y+20
	 add esi,-15
	 call map_x_y
	 test edx,edx
	 jz no_move4
	 ;внизу свободно-можно двигаться    left
	 mov [SpriteInfo+9*eax+0],dword 9
	 mov [SpriteInfo+9*eax+28],dword -5
	 mov [SpriteInfo+9*eax+32],dword 0
	 jmp lab1
	 no_move4:
	 mov [SpriteInfo+9*eax+28],dword 0
	 mov [SpriteInfo+9*eax+32],dword 0
	 lab1:
	 mov edx,dword[SpriteInfo+9*eax+4]
	 test edx,edx
	 jnz no_action_bad_tank
	 mov [SpriteInfo+9*eax+4],dword 1
	 no_action_bad_tank:
	 add eax,4
	 cmp eax,10*4
	 jne next_bad_tank
	 ;-------------------------------------------------
	 mov eax,12
	 next_puly:
	 mov edx,eax
	 shr edx,2
	 xor ecx,ecx
	 mov cl,byte[PulyTanks+edx]
	 test ecx,ecx		      ;can change course of puly ?
	 jz no_change_course
	 xor ecx,ecx
	 mov cl,byte[LifesTanks+edx]
	 test ecx,ecx		      ;is tank not destroed ?
	 jz no_draw_s
	 mov ecx,[SpriteInfo+9*eax+24] ;tank's phas of animation
	 test ecx,ecx		       ; phas=0 ?
	 jnz no_change_course
	 mov esi,[SpriteInfo+9*eax+8]
	 mov edi,[SpriteInfo+9*eax+12]
	 mov ecx,[SpriteInfo+9*eax+28]
	 mov edx,[SpriteInfo+9*eax+32]
	 mov ebx,eax
	 add ebx,28
	 mov [SpriteInfo+9*ebx+0],dword 1
	 mov [SpriteInfo+9*ebx+4],dword 1
	 mov [SpriteInfo+9*ebx+28],dword ecx
	 mov [SpriteInfo+9*ebx+32],dword edx
	 mov [SpriteInfo+9*ebx+16],dword 3
	 mov [SpriteInfo+9*ebx+8],dword esi
	 mov [SpriteInfo+9*ebx+12],dword edi
	 mov ebx,eax
	 shr ebx,2
	 mov [PulyTanks+ebx],byte 0
	 jmp no_draw_s
	 ;-------------------
	 no_change_course:
	 mov ebx,eax
	 add ebx,28
	 mov ecx,[SpriteInfo+9*ebx+24]
	 test ecx,ecx
	 jnz no_draw_s
	 mov esi,[SpriteInfo+9*ebx+8]
	 mov edi,[SpriteInfo+9*ebx+12]
	 mov ecx,[SpriteInfo+9*ebx+28]
	 mov edx,[SpriteInfo+9*ebx+32]
	 call map_x_y
	 cmp ecx,1
	 je bumm
	 cmp ecx,0
	 je strike_to_wall
	 jmp no_bumm
	 strike_to_wall:
	 mov [ebx],byte 2
	 bumm:
	 ;-------------------
	 mov ebx,eax
	 add ebx,28
	 mov [SpriteInfo+9*ebx+28],dword 0
	 mov [SpriteInfo+9*ebx+32],dword 0
	 mov esi,[SpriteInfo+9*ebx+8]
	 mov edi,[SpriteInfo+9*ebx+12]
	 add ebx,28
	 mov [SpriteInfo+9*ebx+4],dword 1
	 mov [SpriteInfo+9*ebx+8],dword esi
	 mov [SpriteInfo+9*ebx+12],dword edi
	 mov [SpriteInfo+9*ebx+16],dword 3
	 mov [SpriteInfo+9*ebx+28],dword 0
	 mov [SpriteInfo+9*ebx+32],dword 0
	 mov ebx,eax
	 shr ebx,2
	 mov [PulyTanks+ebx],byte 1
	 mov edx,[number_level]
	 mov ecx,15
	 push eax
	 call load_level
	 pop eax
	 jnp no_draw_s
	 ;-------------------
	 no_bumm:
	 mov ebx,eax
	 add ebx,28
	 mov [SpriteInfo+9*ebx+4],dword 1 ; can draw sprite of puly
	 no_draw_s:
	 add eax,4
	 cmp eax,10*4
	 jne next_puly
	 ;-------------------------------------------------
	 ;-------------end of tanks logic------------------
	 ;-------------------------------------------------
	 mcall 13, 190*65536+192, 5*65536+7, 0

	 mcall 4,190*65536+5,0x1ded00,Level,5 
	 mcall 4,260*65536+5,0x3558ff,Lifes,5
	 mcall 4,330*65536+5,0xf93500,Score,5
	 
	 mov eax,47
	 mov ebx,3*65536
	 mov ecx,[number_level]
	 inc ecx
	 mov edx,225*65536+5
	 mov esi,0x1ded00
	 mcall
	 mov eax,47
	 mov ebx,3*65536
	 xor ecx,ecx
	 mov cl,[LifesPlayer]
	 shr ecx,2
	 mov edx,295*65536+5
	 mov esi,0x3558ff
	 mcall
	 mov eax,47
	 mov ebx,3*65536
	 xor ecx,ecx
	 mov cl,byte[score]
	 mov edx,365*65536+5
	 mov esi,0xf93500
	 mcall
	 jmp maincycle
;----------------------------------------------------------
;-----------------end of main cycle------------------------
;----------------------------------------------------------
;draw sprite in video memory
PutSprite:
	push eax
	mov ebx,esi
	mov eax,edi
	lea ebx,[ebx+2*ebx]
	lea eax,[eax+2*eax]
	imul eax,640
	add eax,0x4000
	add eax,ebx
	mov [counter],20
	mov esi,edx
	imul esi,1200
	add esi,0x4000+(640*400*3)+10*1200
	mov ebx,esi
	mov edi,eax
   draw:
	mov esi,ebx
	mov edi,eax
	mov ecx,20
	;--------------------
	rep_movsb:
	xor edx,edx
	mov edx,[esi]
	and edx,0xffffff
	test edx,edx ;0 is transparent color
	jz transparent_color
	mov [edi],dx
	shr edx,16
	mov [edi+2],dl
	transparent_color:
	add esi,3
	add edi,3
	sub ecx,1
	jnz rep_movsb
	;--------------------
	add eax,640*3
	add ebx,20*3
	dec [counter]
	jnz draw
	pop eax
      ret
;----------------------------------------------------------
SaveFon:
	push eax
	lea esi,[esi+2*esi]
	lea edi,[edi+2*edi]
	imul edi,640
	add esi,edi
	add esi,0x4000
	mov edi,edx
	imul edi,1200
	add edi,0x4000+(640*400*3)+(50*20*20*3)+512+641
	mov [counter],20
	mov eax,esi
	mov ebx,edi
save_to:
	mov esi,eax
	mov edi,ebx
	mov ecx,15
	rep movsd
	add eax,640*3
	add ebx,20*3
	dec [counter]
	jnz save_to
	pop eax
	ret
;----------------------------------------------------------
PutFon:
	push eax
	lea esi,[esi+2*esi]
	lea edi,[edi+2*edi]
	imul edi,640
	add edi,esi
	add edi,0x4000
	mov esi,edx
	imul esi,1200
	add esi,0x4000+(640*400*3)+(50*20*20*3)+512+641
	mov [counter],20
	mov eax,esi
	mov ebx,edi
 put_to:
	mov esi,eax
	mov edi,ebx
	mov ecx,15
	rep movsd
	add eax,20*3
	add ebx,640*3
	dec [counter]
	jnz put_to
	pop eax
       ret
;----------------------------------------------------------
;get time in 1/100 sec
clock:	mov eax,26
	mov ebx,9
	mcall
	mov [time],eax
	ret
;----------------------------------------------------------
;-----------------load level to memory---------------------
;----------------------------------------------------------
load_level:
	   cmp ecx,15
	   je no_load_level
	   mov eax,edx
	   mov ebx,[levels+4*eax]
	   mov esi,ebx
	   add esi,2
	   xor ecx,ecx
	   xor eax,eax
	   mov cl,byte[ebx]
	   mov al,byte[ebx+1]
	   add ecx,eax
	   mov edi,0x4000+(640*400*3)+(50*20*20*3)+512
	   call unpakin
	   no_load_level:
	   and [y],0
       c_y:
	   and [x],0
       c_x:
	   mov eax,[x]
	   mov ebx,[y]
	   shl ebx,5
	   add eax,ebx
	   add eax,0x4000+(640*400*3)+(50*20*20*3)+512
	   mov ecx,eax
	   xor eax,eax
	   mov al,byte [ecx]
	   imul eax,1200
	   add eax,0x4000+(640*400*3)
	   mov ebx,[x]
	   imul ebx,20*3
	   mov ecx,[y]
	   imul ecx,20*3*640
	   add ebx,ecx
	   add ebx,0x4000
	   mov esi,eax
	   mov edi,ebx
	   ;----------------------------
	   mov edx,20
  next_line:
	   mov esi,eax
	   mov edi,ebx
	   mov ecx,15
	   rep movsd
	   add eax,20*3
	   add ebx,(640*3)
	   dec edx
	   jnz next_line
	   ;----------------------------
	   inc [x]
	   cmp [x],32
	   jne c_x
	   inc [y]
	   cmp [y],20
	   jne c_y
	   ret
;----------------------------------------------------------
map_x_y:
	  push eax
	  mov ecx,20
	  mov eax,esi
	  cdq
	  idiv ecx
	  mov esi,eax
	  mov eax,edi
	  cdq
	  idiv ecx
	  mov edi,eax
	  ;------------------
	  mov ebx,edi
	  shl ebx,5
	  add ebx,esi
	  add ebx,0x4000+(640*400*3)+(50*20*20*3)+512
	  xor ecx,ecx
	  mov cl,byte[ebx]
	  cmp cl,0
	  je false_draw
	  cmp cl,1
	  je false_draw
	  cmp cl,3
	  je false_draw
	  cmp cl,7
	  je false_draw
	  mov edx,1
	  jmp lab2
	  false_draw:
	  xor edx,edx
	  lab2:
	  pop eax
	  ret
;----------------------------------------------------------
you_won:
	 mov eax,13
	 mov ebx,1*65536+640
	 mov ecx,20*65536+400
	 mov edx,0xc6e9
	 mcall
	 mov eax,4
	 mov ebx,220*65536+190
	 mov ecx,0xffffff
	 mov edx,won1
	 mov esi,29
	 mcall
	 mov eax,4
	 mov ebx,220*65536+200
	 mov ecx,0xffffff
	 mov edx,won2
	 mov esi,29
	 mcall
	 mov eax,4
	 mov ebx,220*65536+210
	 mov ecx,0xffffff
	 mov edx,won3
	 mov esi,29
	 mcall
	 mov eax,5
	 mov ebx,100
	 mcall
	 ret
;----------------------------------------------------------
end_game:
	mov eax,13
	mov ebx,0*65536+640
	mov ecx,20*65536+400
	mov edx,0
	mcall
	mov eax,4
	mov ebx,280*65536+200
	mov ecx,0xffffff
	mov edx,game_over
	mov esi,9
	mcall
	mov eax,5
	mov ebx,150
	mcall
	ret
;----------------------------------------------------------
drawwin:
	mcall	12,1
	;рисуем окно задавая все необходимые цвета
	mcall	0,100*65536+649,50*65536+446,(0x74000000+0xffffff),,name
	mcall	12,2
	mcall	13, 0*65536+640,  0*65536+20, 0
	ret
;----------------------------------------------------------
menu:
   cycle_menu:

	mcall 13,238*65536+141,229*65536+20,0xed16
	mcall 4,255*65536+235,0xff0200,start_menu,11
	mcall 47,3*65536,[number_level],345*65536+235, 0xff0200
	
	mcall 4,186*65536+5,0x888888,description,49
	
	still:
	mov eax,10
	mcall
	cmp eax,2
	jne still
	mov eax,2
	mcall
	shr eax,8
	cmp eax,32
	je start_game
	cmp eax,176
	jne no_left
	dec [number_level]
	and [number_level],11111b
	jmp cycle_menu
	no_left:
	cmp eax,179
	jne no_right
	inc [number_level]
	and [number_level],11111b
	jmp cycle_menu
	no_right:
	cmp eax,27
	jne cycle_menu
	mov eax,-1
	mcall
	start_game:
	ret
;----------------------------------------------------------
;--------------unpak pix engin-----------------------------
;----------------------------------------------------------
      unpakin:
 NextLitlColor:
	   xor edx,edx
	   mov dl,byte[esi]
	   xor eax,eax
	   xor ebx,ebx
	   mov al,dl
	   mov bl,al
	   shr al,4
	   and al,0xf
	   mov [LitlCounter],al
	   and bl,0xf
	   mov [LitlColor],bl
	   xor eax,eax
       ;----------------------
	   mov al,[LitlColor]
       beg:mov [edi],al
	   add edi,1
	   add [LitlCounter],-1
	   cmp [LitlCounter],-1
	   jne beg
	   inc esi
	   dec ecx
	   jnz NextLitlColor
	   ret
LitlCounter db 0
LitlColor   db 0
;----------------------------------------------------------
time		dd 0
time_frame_old	dd 0
number_sprite	dd 0
number_level	dd 0
counter 	db 0
sp_pos		dd 0
num_blocks	dd 0
;-------------------
x		dd 0
y		dd 0
_dx		dd 0
_dy		dd 0
strike_action	dd 0
end_bum 	db 0
bazas		db 0
name		db 'Tanks v1.02' ,0
description db 'SPACE - New Game        Left/Right - Change level' ,0
won1		db '*****************************'
won2		db '*    YOU WON LEVEL   !!!    *'
won3		db '*****************************'
game_over	db 'GAME OVER'
Lifes		db 'LIVES'
Level		db 'LEVEL'
Score		db 'SCORE'
start_menu	db 'START LEVEL'
LifesTanks	rb 16
PulyTanks	rb 16
LifesPlayer	db 0
tupik		db 0
score		db 0
SpriteInfo:
	    dd 0,0,0,0,4,0,0,0,0
	    dd 1,0,100,80,3,0,0,0,0
	    dd 2,1,100,100,5,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
	    dd 0,0,0,0,0,0,0,0,0
TANKS_script:
	    db 1,1,2,1,3,1,4,1,5,1,3,2,3,3,3,4,3,5 ;T
	    db 7,3,7,4,7,5,8,2,8,4,9,1,9,4,10,2,10,4,11,3,11,4,11,5 ;A
	    db 13,1,13,2,13,3,13,4,13,5,14,1,15,2,16,3,17,1,17,2,17,3,17,4,17,5 ;N
	    db 19,1,19,2,19,3,19,4,19,5,20,3,21,2,22,1,22,3,23,4,23,5 ;K
	    db 25,4,26,2,26,5,27,1,27,3,27,5,27,1,28,4,28,1,29,2 ;S

;----------------------------------------------------------
;-----------------------data-------------------------------
;----------------------------------------------------------
water:
db 247,0,0,0,0,0,255,0
db 0,128,0,255,128,0,128,255
db 0,255,0,0,255,255,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,7,0,2,1,19,2
db 1,2,1,2,17,2,1,18
db 35,49,19,2,49,2,17,2
db 17,3,6,5,19,2,19,17
db 18,35,18,1,2,51,5,1
db 2,5,3,6,18,1,19,5
db 3,17,2,19,6,5,19,6
db 1,3,33,19,6,3,5,3
db 2,17,51,2,1,19,18,3
db 6,67,18,1,2,3,5,35
db 1,2,17,19,5,6,5,6
db 3,1,18,51,2,3,18,3
db 6,3,5,6,19,5,19,1
db 19,33,18,1,18,3,6,3
db 6,67,1,2,1,2,65,2
db 1,19,5,3,21,3,17,34
db 1,66,1,18,67,2,17,2
db 33,2,1,18,1,2,1,34
db 1,2,1,18,1,18,17,2
db 33,34,17,50,17,2,17,66
db 1,2,1,19,5,3,2,17
db 2,17,50,17,34,19,6,3
db 6,19,1,2,65,19,6,3
db 17,3,5,19,5,35,18,17
db 3,6,3,5,19,18,3,6
db 5,19,5,19,18,17,3,21
db 3,21,2,1,35,21,19,17
db 2,1,2,3,6,3,6,5
db 3,1,18,1,35,2,17,50
db 1,3,5,6,35,1,2,1
db 2,1,2,17,2,17,2,17
db 2,51,16
voda1:
db 137,0,0,0,0,255,255,0
db 255,128,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,3,0,2,17,18,17
db 18,65,18,17,2,33,2,97
db 2,1,2,33,18,17,18,49
db 18,65,2,65,18,17,2,17
db 2,33,18,33,2,113,18,17
db 18,17,2,49,18,17,18,145
db 34,33,18,49,18,17,18,145
db 2,65,2,65,18,17,18,17
db 2,17,34,49,18,97,2,113
db 2,49,18,1,2,49,2,1
db 18,17,2,1,34,65,2,97
db 2,113,2,65,18,17,2,49
db 18,1,2,33,18,1,2,81
db 18,193,18,81,18,17,18,129
db 18,145,18,1,2,81,34,17
db 18,65,34,1,18,145,2,129
db 18,49,18,33,2,33,34,81
db 18,97,2,1,16
trava:
db 214,0,0,0,0,0,255,0
db 0,128,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,3,0,2,33,2,1
db 2,1,2,17,2,1,34,17
db 18,1,18,1,2,49,2,17
db 2,33,2,1,2,33,2,17
db 34,17,18,1,2,1,18,33
db 2,17,34,17,18,33,18,17
db 50,97,2,33,2,33,18,1
db 18,17,50,1,2,1,50,1
db 18,65,2,33,18,1,18,17
db 18,17,82,33,2,1,2,1
db 2,33,2,49,18,1,18,1
db 2,1,2,33,2,17,2,1
db 2,65,2,1,18,1,2,17
db 2,17,34,1,66,1,18,17
db 2,17,2,17,2,33,2,1
db 18,1,2,1,34,1,2,1
db 18,1,18,17,2,33,34,17
db 50,17,2,17,66,1,2,17
db 2,17,2,17,2,17,50,17
db 50,1,34,33,2,81,2,49
db 2,33,34,1,18,33,18,1
db 66,33,2,1,50,17,2,1
db 2,1,2,1,2,49,34,33
db 2,1,18,1,18,33,18,1
db 2,17,2,17,50,17,2,1
db 2,1,2,1,2,1,2,1
db 2,17,2,17,2,17,18,17
db 2,16
tan22:
db 116,0,0,0,0,128,0,255
db 192,192,192,64,128,255,128,128
db 128,0,255,0,0,128,0,0
db 255,255,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,8,0,112,51,8,224
db 0,19,144,8,96,19,144,8
db 64,8,0,19,144,8,32,8
db 0,8,0,19,32,8,32,24
db 2,20,18,8,39,19,39,8
db 18,20,82,55,19,55,82,20
db 18,7,6,23,19,23,6,7
db 18,20,82,6,23,51,23,6
db 146,23,83,23,82,20,18,23
db 19,22,19,23,18,20,82,23
db 19,22,19,23,82,20,18,23
db 83,23,18,20,82,23,83,23
db 146,39,51,39,82,20,18,151
db 18,20,66,8,151,8,50,8
db 48,21,87,21,8,224,224,240
tan21:
db 116,0,0,0,0,128,0,255
db 192,192,192,64,128,255,128,128
db 128,0,255,0,0,128,0,0
db 255,255,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,8,0,112,51,8,224
db 0,19,144,8,96,19,144,8
db 64,8,0,19,144,8,32,8
db 0,8,0,19,32,8,32,24
db 66,8,39,19,39,8,82,20
db 18,55,19,55,18,20,82,7
db 6,23,19,23,6,7,82,20
db 18,6,23,51,23,6,18,20
db 82,23,83,23,146,23,19,22
db 19,23,82,20,18,23,19,22
db 19,23,18,20,82,23,83,23
db 82,20,18,23,83,23,18,20
db 82,39,51,39,146,151,82,20
db 2,8,151,8,2,20,2,8
db 48,21,87,21,8,224,224,240
tan12:
db 116,0,0,0,0,128,0,255
db 192,192,192,64,128,255,128,128
db 128,0,255,0,0,128,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,7,0,112,51,7,224
db 0,19,144,7,96,19,144,7
db 64,7,0,19,144,7,32,7
db 0,7,0,19,32,7,0,55
db 2,20,18,7,33,19,33,7
db 18,20,82,49,19,49,82,20
db 18,1,6,17,19,17,6,1
db 18,20,82,6,17,51,17,6
db 146,17,83,17,82,20,18,17
db 19,22,19,17,18,20,82,17
db 19,22,19,17,82,20,18,17
db 83,17,18,20,82,17,83,17
db 146,33,51,33,82,20,18,145
db 18,20,66,7,145,7,50,7
db 48,21,81,21,7,224,224,240
tan11:
db 116,0,0,0,0,128,0,255
db 192,192,192,64,128,255,128,128
db 128,0,255,0,0,128,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,7,0,112,51,7,224
db 0,19,144,7,96,19,144,7
db 64,7,0,19,144,7,32,7
db 0,7,0,19,32,7,32,23
db 66,7,33,19,33,7,82,20
db 18,49,19,49,18,20,82,1
db 6,17,19,17,6,1,82,20
db 18,6,17,51,17,6,18,20
db 82,17,83,17,146,17,19,22
db 19,17,82,20,18,17,19,22
db 19,17,18,20,82,17,83,17
db 82,20,18,17,83,17,18,20
db 82,33,51,33,146,145,82,20
db 2,7,145,7,2,20,2,7
db 48,21,81,21,7,224,224,240
sten3:
db 104,0,0,0,0,128,128,128
db 192,192,192,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,3,0,226,66,65,2
db 81,2,65,2,81,2,81,2
db 65,2,81,2,81,2,65,2
db 1,226,66,33,2,81,2,81
db 2,65,2,81,2,81,2,65
db 2,81,2,81,2,17,226,66
db 1,2,81,2,81,2,65,2
db 81,2,81,2,65,2,81,2
db 81,2,49,226,66,65,2,97
db 2,65,2,65,2,97,2,65
db 2,65,2,97,2,65,226,82
db 33,2,81,2,81,2,65,2
db 81,2,81,2,65,2,81,2
db 81,2,17,16
sten1:
db 111,0,0,0,0,128,128,128
db 192,192,192,0,0,255,255,255
db 255,64,128,255,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,6,0,227,67,53,3
db 133,3,69,34,5,3,5,98
db 5,3,5,98,5,3,5,98
db 5,3,5,50,53,3,133,3
db 69,227,67,133,3,133,3,114
db 5,3,5,98,5,3,114,5
db 3,5,98,5,3,133,3,133
db 227,83,69,3,133,3,53,50
db 5,3,5,98,5,3,5,98
db 5,3,5,98,5,3,5,34
db 69,3,133,3,53,227,67,101
db 3,133,3,21,82,5,3,5
db 98,5,3,5,98,5,3,5
db 98,5,3,5,2,101,3,133
db 3,21,16
puly1:
db 48,0,0,0,0,0,255,255
db 128,0,255,0,0,255,64,128
db 255,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,5,0,224,224,224,224
db 224,224,224,224,96,49,224,17
db 20,17,192,17,4,18,4,17
db 176,1,4,50,4,1,176,17
db 4,18,4,17,192,17,20,17
db 224,49,224,224,224,224,224,224
db 224,224,224,240
pesok:
db 120,0,0,0,0,0,255,255
db 0,64,128,0,128,128,64,128
db 128,64,128,255,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,6,0,225,81,2,33
db 2,65,2,3,33,2,1,5
db 49,5,49,2,129,3,225,97
db 3,17,2,33,2,1,5,33
db 2,1,5,129,5,65,2,65
db 2,225,97,2,33,3,17,2
db 49,3,145,2,177,3,49,5
db 81,5,1,2,33,2,65,3
db 17,2,17,3,145,2,209,3
db 81,2,49,5,33,2,177,3
db 145,2,17,2,5,113,2,225
db 1,2,33,3,17,3,49,3
db 17,3,161,2,1,5,97,2
db 177,2,49,2,33,5,33,2
db 193,2,49,3,33,2,81,3
db 1,5,1,16
palma:
db 209,0,0,0,0,0,255,0
db 0,128,0,0,255,255,64,128
db 128,64,128,255,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,6,0,2,33,2,1
db 2,1,2,17,2,1,34,17
db 18,1,18,1,51,1,2,17
db 2,1,35,1,2,33,115,2
db 51,21,19,1,2,35,69,35
db 53,35,18,3,21,67,5,3
db 21,19,1,18,19,2,5,3
db 50,1,3,5,3,5,67,17
db 3,1,19,17,67,5,99,34
db 3,2,35,5,3,5,3,5
db 3,5,3,5,35,1,18,1
db 2,19,5,35,5,3,5,35
db 21,19,33,19,5,35,5,35
db 5,51,5,3,34,35,17,51
db 20,3,5,35,5,3,2,1
db 35,18,3,5,3,36,5,19
db 1,35,1,18,3,17,2,3
db 2,52,35,50,1,2,17,2
db 17,2,1,52,3,34,17,50
db 1,34,33,36,49,2,49,2
db 33,34,1,36,17,18,1,66
db 33,2,1,2,1,36,1,2
db 1,2,1,2,1,2,49,66
db 36,18,1,18,33,18,1,2
db 17,84,2,17,2,1,2,1
db 2,1,2,1,2,1,84,2
db 17,18,17,2,16
kamni:
db 210,0,0,0,0,0,255,0
db 0,128,0,64,128,255,64,128
db 128,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,5,0,2,33,2,1
db 2,35,1,2,1,34,17,18
db 1,18,1,2,17,51,1,2
db 33,2,1,2,33,2,1,67
db 4,3,2,1,2,1,18,33
db 2,17,2,83,4,17,18,17
db 50,17,131,2,33,18,1,18
db 17,83,20,3,18,1,18,65
db 2,67,4,19,4,3,18,17
db 82,99,4,35,2,49,18,1
db 18,1,2,1,2,17,35,4
db 2,1,2,65,2,1,18,1
db 2,17,2,17,34,1,66,1
db 18,17,2,17,2,17,2,33
db 2,1,18,1,2,1,2,19
db 1,2,1,18,1,18,19,2
db 33,18,35,4,3,18,17,2
db 1,67,2,1,2,99,1,2
db 17,2,35,4,3,34,35,4
db 3,4,3,1,2,17,83,1
db 67,20,19,4,3,2,17,51
db 4,115,4,35,2,1,83,4
db 67,34,33,2,35,1,3,4
db 19,1,18,1,2,17,2,17
db 50,17,2,1,2,1,2,1
db 2,1,2,1,2,17,2,17
db 2,17,18,17,2,16
drevo:
db 240,0,0,0,0,0,255,0
db 0,128,0,255,0,128,0,64
db 128,64,128,128,128,0,128,255
db 128,128,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,8,0,2,33,2,1
db 2,19,1,35,34,17,18,1
db 18,1,2,17,3,38,7,22
db 3,1,2,1,2,33,2,1
db 19,7,6,23,6,23,3,6
db 18,33,2,17,2,3,7,19
db 6,3,54,7,19,50,33,3
db 22,7,6,3,6,7,22,3
db 23,2,1,18,17,22,7,6
db 7,3,7,3,7,22,7,6
db 35,17,35,54,55,6,23,22
db 19,18,19,7,3,7,6,7
db 22,7,3,22,3,22,7,3
db 18,19,23,3,182,3,33,3
db 22,3,7,6,7,6,7,22
db 7,6,7,22,3,34,7,22
db 7,22,3,39,38,7,6,7
db 19,2,1,3,7,6,23,38
db 7,38,39,6,7,3,1,2
db 19,54,3,38,3,70,3,2
db 1,2,1,3,7,6,7,19
db 4,5,4,19,6,7,6,3
db 1,50,1,34,3,1,36,17
db 3,22,2,49,2,33,34,21
db 4,33,18,1,66,33,2,1
db 52,17,2,1,2,1,2,1
db 2,49,18,20,5,4,5,4
db 18,1,18,33,18,1,2,1
db 132,1,2,1,2,1,2,1
db 2,1,36,5,20,5,4,21
db 36,17,2,16
bum41:
db 141,0,0,0,0,128,255,255
db 0,255,255,0,0,255,128,0
db 255,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,5,0,80,21,50,21
db 16,5,96,21,130,69,32,50
db 33,4,1,66,0,21,16,5
db 34,1,2,17,18,1,66,53
db 34,17,2,17,18,1,18,4
db 18,37,50,33,2,33,2,17
db 50,21,2,4,18,65,50,1
db 50,5,50,33,2,49,2,17
db 50,21,2,81,18,33,2,17
db 130,113,2,17,130,33,18,1
db 194,65,2,33,66,4,2,5
db 65,2,97,34,1,18,5,18
db 1,34,1,18,1,2,17,18
db 1,34,21,2,4,17,34,49
db 82,37,18,1,18,49,34,4
db 50,37,50,1,18,49,66,53
db 0,66,1,114,37,64,18,17
db 2,4,50,69,80,5,82,53
db 64
bum31:
db 114,0,0,0,0,128,255,255
db 0,255,255,0,0,255,128,0
db 255,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,5,0,80,53,18,21
db 144,53,82,85,48,5,18,3
db 18,4,66,5,0,21,16,21
db 146,4,18,85,3,18,3,2
db 49,82,53,98,3,66,1,18
db 53,178,3,1,4,18,21,2
db 3,114,3,50,1,18,21,34
db 4,50,3,130,21,210,3,18
db 3,98,3,194,21,130,3,114
db 21,18,4,18,3,146,3,2
db 21,2,3,114,3,34,1,18
db 37,34,3,2,4,98,4,2
db 3,2,53,226,101,50,4,98
db 69,0,37,146,53,80,37,2
db 3,18,101,80,165,64
bum21:
db 134,0,0,0,0,128,255,255
db 0,255,255,0,0,255,128,0
db 255,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,5,0,80,117,144,69
db 50,101,48,37,34,4,50,21
db 0,21,16,37,130,4,2,133
db 18,4,49,66,101,66,19,2
db 3,18,1,18,69,18,4,18
db 3,2,3,18,19,1,4,2
db 69,34,35,2,83,1,18,53
db 2,4,2,3,18,3,2,3
db 2,3,2,3,18,4,53,34
db 99,2,19,34,53,34,3,2
db 19,2,51,18,4,2,53,2
db 4,1,51,2,3,18,19,34
db 53,2,4,18,3,2,83,2
db 4,18,69,66,19,2,3,34
db 1,2,101,18,4,98,4,2
db 133,82,4,34,165,18,4,66
db 101,0,85,50,101,80,213,80
db 165,64
bum11:
db 79,0,0,0,0,128,255,255
db 0,255,255,0,0,255,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,4,0,80,100,176,196
db 80,196,0,20,16,100,50,100
db 0,84,2,49,18,1,164,34
db 51,18,1,148,2,115,1,2
db 116,2,147,1,100,1,2,147
db 18,84,1,2,147,2,1,84
db 18,147,2,1,100,1,147,1
db 2,100,18,115,2,1,132,34
db 51,34,1,148,130,196,50,116
db 0,228,36,32,228,20,80,196
db 96,164,64
baza1:
db 192,0,0,0,0,0,255,0
db 0,128,0,255,0,128,255,128
db 128,255,0,0,255,128,0,255
db 255,0,128,0,255,0,0,255
db 0,255,255,0,0,0,0,0
db 0,0,0,0,0,0,0,0
db 0,0,11,0,2,33,2,1
db 2,1,2,17,2,1,34,17
db 18,1,18,1,2,49,2,17
db 2,33,2,1,2,17,232,9
db 4,17,2,8,201,8,3,9
db 4,2,1,8,41,150,8,19
db 9,2,1,57,6,101,7,6
db 8,3,10,3,9,2,8,41
db 6,5,84,7,6,8,35,9
db 2,8,41,6,5,84,7,6
db 8,35,9,2,8,41,6,5
db 84,7,6,8,3,21,9,1
db 8,41,6,5,84,7,6,8
db 3,21,9,2,8,41,6,5
db 84,7,6,8,35,9,2,8
db 41,6,5,103,6,8,3,10
db 3,9,1,8,57,134,8,35
db 9,1,8,201,8,3,10,3
db 9,2,232,35,9,2,9,131
db 10,19,10,3,9,4,3,9
db 1,2,25,19,22,131,9,4
db 9,33,9,35,22,131,25,1
db 18,1,233,9,2,1,2,1
db 2,1,2,17,2,17,2,17
db 18,17,2,16
;-----------------------------------
;-----here begin levels-------------
;-----------------------------------
lev01:
db 171,0
db 225,225,33,150,7,2,1,7
db 41,192,17,150,18,1,57,144
db 1,16,17,182,1,57,112,33
db 16,17,38,225,65,96,17,6
db 1,6,1,7,5,80,1,7
db 117,1,96,17,38,1,18,80
db 1,69,24,21,1,96,17,38
db 1,112,1,34,40,37,1,96
db 17,38,128,1,19,224,17,38
db 1,112,1,0,3,96,1,96
db 17,38,1,224,32,1,96,17
db 38,1,224,32,65,0,49,38
db 129,0,145,6,7,6,1,32
db 17,86,48,1,148,70,1,32
db 17,86,48,1,148,22,1,22
db 1,32,17,6,1,54,4,3
db 16,1,116,16,70,1,32,17
db 70,35,16,1,20,17,36,32
db 70,1,8,7,8,17,70,66
db 1,100,112,1,40,17,70,2
db 7,34,1,116,96,1,40,225
db 225,33,16
lev02:
db 255,3
db 225,225,33,5,49,3,225,161
db 5,38,5,3,64,89,0,40
db 54,50,33,5,38,5,3,32
db 35,34,3,16,8,67,6,50
db 33,5,38,5,3,34,16,3
db 36,3,2,35,36,3,66,33
db 5,22,21,3,34,3,0,3
db 36,3,2,3,7,36,5,3
db 2,1,35,33,5,22,21,3
db 7,2,0,2,19,5,20,3
db 2,51,21,3,66,33,5,22
db 21,35,0,2,3,7,5,20
db 18,3,24,3,22,67,2,33
db 5,17,25,48,2,51,2,35
db 24,3,22,9,16,19,2,33
db 5,2,1,25,49,2,3,56
db 3,54,67,0,3,16,33,5
db 2,49,22,1,6,3,22,1
db 8,3,17,88,3,48,33,5
db 18,4,54,1,6,3,22,1
db 8,3,7,1,99,57,33,5
db 18,4,22,1,5,1,54,1
db 8,3,70,51,57,33,5,18
db 4,0,17,5,113,51,0,3
db 38,9,32,33,5,17,4,16
db 1,32,1,37,49,32,3,54
db 19,0,33,5,0,20,16,1
db 2,1,0,20,1,24,34,0
db 33,7,38,3,16,33,5,0
db 33,0,1,2,113,18,32,83
db 16,33,5,16,7,16,1,146
db 33,128,33,114,65,98,225,225
db 193,16
lev03:
db 118,0
db 225,225,33,85,50,35,162,86
db 17,5,7,53,166,34,7,8
db 18,86,17,85,34,19,6,19
db 50,72,2,86,193,6,225,1
db 6,33,160,230,54,17,18,128
db 102,65,102,17,7,2,128,102
db 1,166,17,160,230,54,225,113
db 6,145,214,224,0,17,214,96
db 81,32,17,214,96,1,7,38
db 17,16,17,214,224,0,113,6
db 225,145,82,198,1,144,17,82
db 1,182,1,48,1,54,0,17
db 18,38,2,7,2,166,1,48
db 1,54,0,17,114,230,6,1
db 7,48,225,225,33,16
lev04:
db 146,0
db 225,225,33,89,1,176,89,1
db 57,17,89,1,176,25,7,41
db 1,57,17,89,1,9,65,57
db 64,34,1,25,1,9,17,89
db 1,25,7,5,9,1,57,48
db 35,33,9,1,9,17,89,1
db 73,1,233,1,9,17,89,225
db 113,9,17,233,233,17,233,233
db 17,217,129,105,17,217,1,100
db 1,105,17,41,81,64,1,7
db 73,4,1,105,17,41,1,7
db 41,1,64,1,89,4,1,105
db 17,41,1,36,25,64,1,25
db 1,41,4,145,41,1,52,1
db 64,41,1,7,34,1,98,17
db 41,81,64,129,18,7,50,17
db 233,121,3,20,50,17,233,121
db 35,17,25,17,233,233,225,225
db 33,16
lev05:
db 229,0
db 225,225,33,224,224,17,0,134
db 50,72,34,24,2,40,2,0
db 17,0,134,34,83,2,56,2
db 40,2,0,17,0,134,18,19
db 56,3,2,24,2,8,2,56
db 0,17,0,22,8,86,19,6
db 72,3,40,2,8,2,56,0
db 17,0,22,8,50,22,18,6
db 40,35,2,24,2,8,2,56
db 0,17,0,22,72,22,18,54
db 3,6,56,2,8,2,32,8
db 0,17,0,40,86,18,40,6
db 3,6,88,2,7,8,0,8
db 0,17,0,40,6,7,70,2
db 40,6,3,54,8,54,8,0
db 8,0,17,0,56,38,8,22
db 40,7,86,8,54,8,0,8
db 0,17,0,2,51,56,6,40
db 2,166,8,0,8,0,17,0
db 50,3,34,8,6,40,2,6
db 66,70,16,8,0,17,0,18
db 7,2,72,22,7,8,0,6
db 66,70,0,24,0,17,0,6
db 51,120,0,6,66,8,54,0
db 24,0,17,0,118,3,64,6
db 104,22,7,0,24,0,17,0
db 118,83,6,136,16,24,0,17
db 0,230,22,19,136,0,17,224
db 224,225,225,33,16
lev06:
db 149,0
db 225,225,33,176,1,18,19,192
db 17,0,7,18,112,1,2,7
db 2,16,3,160,17,0,34,112
db 1,18,51,160,17,0,1,144
db 97,160,17,0,1,224,192,17
db 0,1,224,192,17,0,145,48
db 230,17,48,7,66,1,48,209
db 6,17,48,82,1,48,1,68
db 96,22,17,144,1,48,1,4
db 7,18,3,96,22,17,144,1
db 48,1,4,34,3,48,7,41
db 6,17,144,1,48,1,20,96
db 57,6,17,16,129,48,1,20
db 96,4,41,6,17,16,1,40
db 50,64,1,128,4,32,6,17
db 16,1,7,8,66,64,145,4
db 32,6,17,16,177,144,1,70
db 17,224,128,1,70,17,224,144
db 70,225,225,33,16
lev07:
db 159,0
db 225,225,33,0,7,229,197,17
db 64,225,113,21,17,64,1,37
db 134,133,1,21,17,32,33,37
db 6,37,33,7,6,133,1,21
db 17,37,1,37,1,5,134,133
db 1,21,17,133,209,5,49,21
db 17,5,1,229,197,17,5,1
db 133,1,229,37,17,5,1,85
db 1,229,85,17,5,65,21,1
db 21,1,101,113,38,17,5,1
db 32,37,1,149,1,7,8,53
db 1,38,17,5,1,8,16,37
db 1,21,1,7,85,1,3,2
db 53,1,38,17,5,1,8,16
db 21,193,3,69,1,38,17,5
db 1,8,1,0,229,3,69,1
db 38,17,5,1,7,232,117,1
db 38,17,5,1,232,8,129,38
db 17,5,225,33,7,98,38,17
db 229,69,150,225,225,33,16
lev08:
db 209,0
db 225,225,33,36,7,228,84,1
db 48,17,164,195,4,1,48,17
db 164,3,166,3,4,1,0,7
db 2,0,17,164,3,166,3,4
db 1,0,18,0,17,165,3,166
db 3,4,1,48,17,165,3,166
db 3,4,1,48,17,165,3,166
db 3,4,1,48,17,54,3,82
db 3,6,1,4,99,6,3,4
db 1,48,17,54,3,82,3,6
db 1,4,3,7,21,22,3,6
db 3,4,1,0,7,2,0,17
db 6,7,22,3,82,3,6,1
db 4,3,37,22,3,6,3,4
db 1,0,18,0,17,54,3,82
db 3,6,1,4,3,70,3,6
db 3,4,1,48,17,54,3,82
db 3,6,1,4,3,6,67,6
db 3,4,1,48,17,54,115,6
db 1,4,3,102,3,4,1,48
db 17,198,1,4,3,102,3,4
db 1,48,17,6,161,2,1,4
db 3,102,3,4,1,48,17,6
db 7,130,1,2,1,4,131,4
db 1,48,17,38,114,1,2,193
db 48,17,38,226,178,225,225,33
db 16
lev09:
db 162,0
db 225,225,33,229,5,214,17,21
db 33,7,149,1,198,17,21,208
db 1,32,150,17,226,2,1,18
db 80,70,17,2,83,130,1,18
db 0,7,34,0,70,17,2,3
db 144,1,0,18,1,18,0,50
db 0,70,17,2,3,144,1,0
db 18,1,18,0,50,0,70,17
db 2,3,32,7,50,16,1,0
db 66,0,50,0,70,17,2,3
db 32,66,16,1,0,66,0,7
db 130,17,2,3,144,1,48,18
db 64,82,17,18,144,1,0,7
db 50,0,2,32,82,17,18,144
db 1,0,66,0,1,64,50,17
db 178,1,0,66,0,2,80,34
db 17,178,1,96,6,80,34,145
db 0,34,113,150,17,7,82,1
db 0,34,230,38,17,98,16,226
db 82,17,98,16,226,82,225,225
db 33,16
lev10:
db 205,0
db 225,225,33,224,224,17,0,2
db 83,64,233,9,0,17,0,2
db 3,34,0,7,64,233,9,0
db 17,0,2,3,34,96,73,7
db 153,0,17,224,32,1,41,50
db 41,0,17,2,145,16,18,32
db 1,41,50,41,0,17,2,1
db 134,16,18,32,1,41,50,41
db 0,17,2,1,6,41,70,16
db 1,2,32,1,7,25,50,41
db 0,17,2,1,6,89,22,16
db 18,32,49,50,41,0,17,2
db 1,6,89,22,16,1,2,32
db 1,41,50,41,0,17,2,1
db 54,41,22,16,1,2,32,1
db 41,50,41,0,17,2,1,54
db 41,22,16,1,2,32,1,41
db 50,41,0,17,2,1,54,7
db 25,22,16,18,32,1,41,2
db 7,16,41,0,17,2,1,54
db 41,22,16,1,2,224,17,2
db 1,54,41,22,16,1,2,224
db 17,2,1,134,16,18,100,6
db 98,17,2,1,160,1,2,100
db 6,1,82,17,226,118,1,7
db 66,225,225,33,16
lev11:
db 228,0
db 225,225,33,230,230,17,6,104

db 1,104,6,1,7,152,6,17

db 6,104,1,104,6,1,24,1

db 120,6,17,6,104,1,8,102

db 49,8,64,24,6,17,6,104

db 1,8,6,72,38,40,0,88

db 6,17,6,8,97,8,6,33

db 56,6,40,64,24,6,17,6

db 8,1,104,6,1,64,1,6

db 104,0,24,6,17,6,8,1

db 104,6,1,0,40,0,1,6

db 104,0,24,6,17,6,8,1

db 7,88,6,1,0,40,0,1

db 6,40,16,24,0,24,6,17

db 6,8,33,72,6,1,0,8

db 7,8,0,1,6,40,0,40

db 0,24,6,17,6,136,6,1

db 0,40,0,1,6,40,0,40

db 0,24,6,17,166,1,0,40

db 0,1,6,40,64,7,8,6

db 17,6,8,49,6,56,1,64

db 1,6,152,6,17,6,8,7

db 24,1,6,56,97,182,17,6

db 56,1,6,168,6,24,80,40

db 17,6,56,1,198,24,0,56

db 0,40,17,6,72,6,216,0

db 56,0,40,17,102,120,1,7

db 56,0,1,7,8,1,0,40

db 225,225,33,16
lev12:
db 157,0
db 225,225,33,66,1,217,36,98

db 17,66,1,217,36,50,3,18

db 17,66,1,41,161,36,33,50

db 17,66,1,41,1,196,1,2

db 1,50,17,66,1,41,1,196

db 1,2,1,50,17,2,65,41

db 1,148,49,2,1,50,17,2

db 1,7,24,25,33,4,145,7

db 34,1,50,17,2,1,40,25

db 1,7,20,1,7,178,1,50

db 17,2,97,32,1,194,1,50

db 17,162,1,194,1,50,17,162

db 1,2,3,18,145,50,209,2

db 3,18,1,7,178,17,7,36

db 19,98,3,18,1,98,3,66

db 17,68,3,4,130,1,194,17

db 36,35,4,130,1,194,17,114

db 129,34,3,82,3,18,17,18

db 3,66,1,7,226,66,17,226

db 226,225,225,33,16
lev13:
db 233,0
db 225,225,33,224,224,17,0,53

db 225,49,16,1,21,0,17,0

db 53,1,230,22,1,16,1,21

db 0,17,0,53,1,230,22,1

db 16,1,21,0,17,0,53,1

db 38,177,22,1,16,1,21,0

db 17,0,5,1,21,1,38,1

db 7,133,1,22,1,16,1,21

db 0,17,0,5,1,21,1,38

db 1,6,1,7,101,1,22,1

db 16,1,21,0,17,0,5,1

db 21,1,70,1,117,1,22,1

db 16,1,21,0,17,0,5,1

db 21,1,38,1,6,1,5,7

db 85,1,22,1,16,1,21,0

db 17,0,5,1,21,1,38,1

db 38,81,5,1,38,16,1,21

db 0,17,0,5,1,21,1,38

db 1,70,7,53,1,22,1,16

db 1,7,5,0,17,0,5,1

db 21,1,38,177,22,1,16,1

db 21,0,17,0,5,1,21,1

db 230,22,1,16,1,21,0,17

db 0,5,1,21,1,7,230,6

db 1,16,1,21,0,17,0,5

db 1,21,225,49,16,1,21,0

db 17,0,5,1,7,5,224,80

db 1,21,0,17,0,5,225,145

db 21,0,17,224,224,225,225,33

db 16
lev14:
db 152,0
db 225,225,33,7,102,144,57,112

db 17,118,48,7,40,16,7,41

db 112,17,118,0,104,208,17,22

db 49,232,128,17,22,1,25,1

db 8,196,8,128,17,22,1,25

db 1,8,196,8,128,17,38,0

db 9,1,8,20,7,148,8,128

db 17,6,32,9,1,8,196,72

db 64,17,6,0,41,1,8,196

db 72,64,17,6,0,41,1,8

db 196,8,7,40,64,17,6,32

db 9,1,232,128,17,70,1,0

db 81,120,128,17,64,1,80,1

db 224,16,17,64,1,89,1,16

db 86,32,70,0,17,80,89,1

db 16,86,32,70,0,17,86,1

db 7,57,1,32,70,32,70,0

db 17,7,70,97,118,32,70,0

db 17,230,86,128,225,225,33,16

lev15:
db 242,0
db 225,225,33,182,224,32,17,22

db 69,70,0,54,0,54,0,54

db 0,20,17,22,69,32,22,0

db 54,0,54,0,54,0,20,17

db 22,69,0,5,4,22,0,1

db 38,0,54,0,54,0,20,17

db 22,69,0,3,4,22,0,1

db 38,0,54,0,33,6,0,20

db 17,22,69,0,3,4,22,0

db 17,7,6,0,54,0,1,7

db 22,0,20,17,22,69,0,3

db 4,22,0,1,38,0,54,0

db 1,38,0,20,17,22,80,3

db 4,22,0,1,38,0,54,0

db 1,38,0,20,17,22,0,83

db 4,22,0,54,0,54,0,49

db 0,20,17,22,0,3,84,22

db 0,54,0,56,0,54,0,20

db 17,22,0,3,84,22,0,54

db 0,56,0,54,0,20,17,22

db 0,3,7,68,22,0,54,0

db 8,7,24,0,54,0,25,17

db 22,0,83,4,22,0,54,0

db 56,0,54,0,7,9,17,22

db 112,22,0,54,0,54,0,54

db 0,25,17,182,0,54,0,54

db 0,54,0,20,17,192,54,0

db 54,0,54,0,20,225,1,7

db 22,0,54,0,54,0,20,17

db 7,2,116,224,32,20,225,225

db 33,16

;-----------------------------------
spisok:
  dd sten1
  dd sten3
  dd trava
  dd kamni
  dd palma
  dd water
  dd voda1
  dd baza1
  dd drevo
  dd pesok
  dd bum11; here begin animate sprites
  dd bum21
  dd bum31
  dd bum41
  dd puly1
  dd puly1
  dd puly1
  dd puly1
  dd tan11
  dd tan12
  dd tan11
  dd tan12
  dd tan21
  dd tan22
  dd tan21
  dd tan22
;-----------------------------------
levels:
  dd lev01
  dd lev02
  dd lev03
  dd lev04
  dd lev05
  dd lev06
  dd lev07
  dd lev08
  dd lev09
  dd lev10
  dd lev11
  dd lev12
  dd lev13
  dd lev14
  dd lev15
  dd lev02
  dd lev02
  dd lev02
  dd lev02
  dd lev02
  dd lev02
  dd lev02
  dd lev02
  dd lev02
  dd lev02
  dd lev02
  dd lev02
  dd lev02
  dd lev02
I_END: