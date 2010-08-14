;;===Level_mode================================================================================================================

Level_begin:

      call      Load_level
      call      Get_eat

Level_body:
    ;;===Level_body========================================================================================================

      call      Hide_cursor
mcall     26, 9
    mov  [time_before_waiting], eax
    mov  eax, [time_wait_limit]
    mov  [time_to_wait],    eax

  .redraw:
      mcall     12,1
    mov  ebx, [wp_x]
    shl  ebx, 16
    add  ebx, dword[window_width]
    mov  ecx, [wp_y]
    shl  ecx, 16
    add  ecx, dword[window_height]
      mcall     0, , ,[window_style], ,window_title
    
      call      Draw_decorations
      call      Draw_stones
      call      Draw_snake
      call      Draw_eat
      call      Draw_level_strings

      mcall     12,2
    
  .still:
      mcall     26, 9
    push eax
    sub  eax, [time_before_waiting]
    pop  [time_before_waiting]
    cmp  [time_to_wait],    eax
     jg  @f
    cmp  [action],  0
     jne Game_step
  @@:
    sub  [time_to_wait],    eax
      mcall     23, [time_to_wait]              ; 

    test al,  al
     jnz  @f
    cmp  [action],  0
     jne Game_step
      mcall     26, 9
    mov  [time_before_waiting], eax
    mov  eax, [time_wait_limit]
    mov  [time_to_wait],    eax
     jmp .still
  @@:

      .message:                                 ; ok, what an event?
        dec  al                                 ; has the window been moved or resized?
         jz  .redraw                            ;
        dec  al                                 ; was a key pressed?
         jz  .key                               ; 
        dec  al                                 ; was a button pressed?
         jz  .button                            ; 


  .button:                                      ; процедура обрабоки кнопок в программе
      mcall     17                              ; функция 17: получить номер нажатой кнопки

    shr  eax, 8                                 ; сдвигаем регистр eax на 8 бит вправо, чтобы получить номер нажатой кнопки
    cmp  eax, 1
     je  Save_do_smth_else_and_exit             ; если это не кнопка 1 (зарезервирована системой как кнопка закрытия программы), пропускаем 2 следующие строчки кода

     jmp .still


  .key:
      mcall     2                               ; get keycode
    
    cmp  ah,  0x1B                              ; Escape
     je  First_menu
    cmp  ah,  0x20                              ; Space
     je  Pause_mode
    cmp  ah,  0xB0                              ; Left
     je  .key.left
    cmp  ah,  0xB1                              ; Down
     je  .key.down
    cmp  ah,  0xB2                              ; Up
     je  .key.up
    cmp  ah,  0xB3                              ; Right
     je  .key.right
    
     jmp .still                                 ; jump to wait for another event


  .key.left:
    bts  [action],  0
     jc  @f
    mov  [time_to_wait],    0
  @@:
    mov  [snake_direction_next],    LEFT
     jmp .still
            
  .key.down:
    bts  [action],  0
     jc  @f
    mov  [time_to_wait],    0
  @@:
    mov  [snake_direction_next],    DOWN
     jmp .still
            
  .key.up:
    bts  [action],  0
     jc  @f
    mov  [time_to_wait],    0
  @@:
    mov  [snake_direction_next],    UP
     jmp .still
            
  .key.right:
    bts  [action],  0
     jc  @f
    mov  [time_to_wait],    0
  @@:
    mov  [snake_direction_next],    RIGHT
     jmp .still


  Game_step:

    cmp  [snake_direction], LEFT                ; are we moving to left?
     jz  .left
    cmp  [snake_direction], DOWN                ; ... down?
     jz  .down
    cmp  [snake_direction], UP                  ; ... up?
     jz  .up
     jmp .right                                 ; then right
     
  .left:
    cmp  [snake_direction_next],    RIGHT       ; next step is to right?
     jz  .with_reverse
     jmp .without_reverse
  
  .down:
    cmp  [snake_direction_next],    UP          ; next step is to up?
     jz  .with_reverse
     jmp .without_reverse
     
  .up:
    cmp  [snake_direction_next],    DOWN        ; next step is to bottom?
     jz  .with_reverse
     jmp .without_reverse

  .right:
    cmp  [snake_direction_next],    LEFT        ; next step is to left?
     jz  .with_reverse
     jmp .without_reverse


  .with_reverse:
      call      Set_reverse_direction
      call      Reverse
        
  .without_reverse:
    mov  edx, snake_dots-2
    add  edx, [snake_length_x2]
        
    cmp  [snake_direction_next],    LEFT
     je  .to_left
    cmp  [snake_direction_next],    DOWN
     je  .to_down 
    cmp  [snake_direction_next],    UP
     je  .to_up
    cmp  [snake_direction_next],    RIGHT
     je  .to_right    
        
      .to_left:
        mov  [snake_direction], LEFT
        mov  ax,  [edx]
        dec  al
        cmp  al,  -1
         jne @f
        mov  al,  GRID_WIDTH
        dec  al
      @@:
         jmp Snake_move

      .to_down:
        mov  [snake_direction], DOWN
        mov  ax,  [edx]
        inc  ah
        cmp  ah,  GRID_HEIGHT
         jne @f
        mov  ah,  0
      @@:
         jmp Snake_move
        
      .to_up:
        mov  [snake_direction], UP
        mov  ax,  [edx]
        dec  ah
        cmp  ah,  -1
         jne @f
        mov  ah,  GRID_HEIGHT
        dec  ah
      @@:
         jmp Snake_move
        
      .to_right:
        mov  [snake_direction], RIGHT
        mov  ax,  [edx]
        inc  al
        cmp  al,  GRID_WIDTH
         jne @f
        mov  al,  0
      @@:
         jmp Snake_move

    ;;---Level_body--------------------------------------------------------------------------------------------------------

;;---Level_mode----------------------------------------------------------------------------------------------------------------


;;===Some_functions============================================================================================================

Draw_snake:
    ;;===Draw_snake========================================================================================================
    
      call      Draw_head_prehead
    mov  edx, [snake_color]
    mov  esi, snake_dots-6
    add  esi, [snake_length_x2]

  @@:
    mov  bx,  [esi]
    sub  esi, 2
      call      Draw_square
    cmp  esi, snake_dots-2
     jne @b

    ret
        
    ;;---Draw_snake--------------------------------------------------------------------------------------------------------


Draw_head_prehead:
    ;;===Draw_head_prehead=================================================================================================

    mov  edx, [snake_head_color]
    mov  esi, snake_dots-2
    add  esi, [snake_length_x2]
    mov  bx,  [esi]
      call      Draw_square
    sub  esi, 2
    mov  bx,  [esi]
    mov  edx, [snake_color]
      call      Draw_square

    ret

    ;;---Draw_head_prehead-------------------------------------------------------------------------------------------------


Draw_level_strings:
    ;;===Draw_level_strings================================================================================================

    mov  ebx, [window_width]
    shr  ebx, 1
    sub  ebx, string_pause_space.size*3+6
    shl  ebx, 16
    add  ebx, [top_strings]
      mcall     4, ,[navigation_strings_color],string_pause_space ; Draw 'PAUSE - SPACE' string

;    call    Draw_menu_esc
    call    Draw_score_string
    call    Draw_score_number                   ; Draw score (number)
    call    Draw_champion_string
    call    Draw_champion_name
    call    Draw_hiscore_string
    call    Draw_hiscore_number

    cmp  [play_mode],   LEVELS_MODE
     jne @f

    mov  ebx, [window_width]
    shr  ebx, 3
    sub  ebx, 5
    shl  ebx, 16
    add  ebx, [bottom_bottom_strings]
      mcall     4, ,[level_string_color],string_level

    mov  edx, [window_width]
    shr  edx, 3
    sub  edx, 5+1
    add  edx, string_level.size*6
    shl  edx, 16
    add  edx, [bottom_bottom_strings]
      mcall     47,0x00020000,[cur_level_number], ,[level_number_color],[background_color]

  @@:

    ret

    ;;---Draw_level_strings------------------------------------------------------------------------------------------------
    
    
Reverse:
    ;;===Reverse===========================================================================================================

    mov  ecx, [snake_length_x2]
    shr  ecx, 2
    mov  esi, snake_dots
    mov  edi, snake_dots-2
    add  edi, [snake_length_x2]

  @@:
    mov  ax,  [edi]
    xchg ax,  [esi]
    mov  [edi], ax

    add  esi, 2
    sub  edi, 2
    dec  cx
     jnz @b

    ret

    ;;---Reverse-----------------------------------------------------------------------------------------------------------
        
        
Draw_eat:
    ;;===Draw_eat==========================================================================================================
        
    mov  bx,  word[eat]
    mov  edx, [eat_color]
        
    call    Draw_square
        
    ret
        
    ;;---Draw_eat----------------------------------------------------------------------------------------------------------


Get_eat:
    ;;===Get_eat===========================================================================================================
    ;;  in  :
    ;;
    ;;  out :
    ;;          ax  =   coord's of the eat square (al=x, ah=y)
    ;;
    
      mcall     26,9
;    xor  eax, esp
    shl  eax, 1
    xor  edx, edx
    div  word[number_of_free_dots]
    mov  ebx, field_map

  .loop:
    cmp  byte[ebx], 0
     jne @f
    test dx,  dx
     jz  .place_found
    dec  dx
  @@:
    inc  ebx
     jmp .loop
     
  .place_found:
    sub  ebx, field_map
    mov  eax, ebx
    mov  bl,  GRID_WIDTH
    div  bl
    xchg al,  ah
    
    mov  word[eat], ax

    ret

    ;;---Get_eat-----------------------------------------------------------------------------------------------------------


Sdvig:
    ;;===Sdvig=============================================================================================================

    mov  esi, snake_dots+2
    mov  edi, snake_dots
    mov  ecx, [snake_length_x2]
    shr  ecx, 1
    
    cld
    rep  movsw

    ret

    ;;---Sdvig-------------------------------------------------------------------------------------------------------------


Set_reverse_direction:
    ;;===Set_reverse_direction==================================================================================================

    mov  eax, snake_dots
    mov  ebx, snake_dots+2

    mov  cl,  [eax]                             ; The last dot x_coord
    mov  ch,  [ebx]                             ; The pre_last dot x_coord

    cmp  cl,  ch
     je  .X_ravny
    
    cmp  cl,  0
     jne .skip2
    
    mov  dl,  GRID_WIDTH
    dec  dl
    cmp  ch,  dl
     jne .Normal_y_ravny
    mov  [snake_direction_next],    RIGHT
    ret
        
  .skip2:
    mov  dl,  GRID_WIDTH
    dec  dl
    cmp  cl,  dl
     jne .Normal_y_ravny
    cmp  ch,  0
     jne .Normal_y_ravny
    mov  [snake_direction_next],    LEFT
    ret
    
  .Normal_y_ravny:

    cmp  cl,  ch
     jg  .Napravlenie_to_right
    mov  [snake_direction_next],    LEFT
    ret

  .Napravlenie_to_right:
    mov  [snake_direction_next],    RIGHT
    ret

  .X_ravny:
    inc  eax
    inc  ebx
    mov  cl,  [eax]
    mov  ch,  [ebx]
    
    cmp  cl,  0
     jne .skip3
    
    mov  dl,  GRID_HEIGHT
    dec  dl
    cmp  ch,  dl
     jne .Normal_x_ravny
    mov  [snake_direction_next],    DOWN
    ret
        
  .skip3:
    mov  dl,  GRID_HEIGHT
    dec  dl
    cmp  ch,  dl
     jne .Normal_x_ravny
    cmp  ch,  0
     jne .Normal_x_ravny
    mov  [snake_direction_next],    UP
    ret
    
  .Normal_x_ravny:

    cmp  cl,  ch                                ; !!!
     jg  .Napravlenie_to_down                   ; 0 1 2 ...
    mov  [snake_direction_next],    UP          ; 1
    ret                                         ; 2
                                                ; .
  .Napravlenie_to_down:                         ; .
    mov  [snake_direction_next],    DOWN        ; .

    ret

    ;;---Set_reverse_direction--------------------------------------------------------------------------------------------------
    
    
Snake_move:
    ;;===Snake_move=============================================================================================================
    ;;  in  :
    ;;           ax =   coord's of new head
    ;;          edx =   snake_dots+[snake_length_x2]-2 (snake head)
    ;;

    add  edx, 2
    mov  [edx], ax
    cmp  ax,  word[eat]
     jne .eat_and_new_head_are_different

    add  [snake_length_x2], 2
    add  [score],   SCORE_EAT
    dec  [number_of_free_dots]
    cmp  [number_of_free_dots], 0
     je  Game_over
    mov  ax,  word[eat]
    mov  cl,  1
      call      Draw_on_map
      call      Draw_head_prehead
      call      Get_eat
      call      Draw_eat

     jmp Keys_done


  .eat_and_new_head_are_different:

    push ax
    
    mov  ax,  word[snake_dots]
    mov  cl,  0
      call      Draw_on_map

    pop ax

      call      Get_from_map
    test bl,  bl
     jnz Game_over

    mov  cl,  1
      call      Draw_on_map
      
    mov  bx,  word[snake_dots]
    mov  edx, [background_color]
      call      Draw_square
    
      call      Sdvig
      
      call      Draw_head_prehead


  Keys_done:

    cmp  [score],   0
     je  @f
    dec  [score]
      call      Draw_score_number
  @@:

    cmp  [play_mode],   LEVELS_MODE
     jne @f
    cmp  [snake_length_x2], (EAT_TO_END_LEVEL+3)*2
     je  Do_smth_between_levels
  @@:

      mcall     26, 9
    mov  [time_before_waiting], eax
    mov  eax, [time_wait_limit]
    mov  [time_to_wait],    eax
     jmp Level_body.still

    ;;---Snake_move------------------------------------------------------------------------------------------------------------


Do_smth_between_levels:
    ;;===Do_smth_between_levels================================================================================================

    inc  [cur_level_number]
    cmp  [cur_level_number],    LAST_LEVEL_NUMBER+1
     jne @f
    mov  [cur_level_number],    LEVELS_MODE_FIRST_LEVEL
  @@:

      call      Draw_splash

  @@:
      mcall     2
    cmp  eax, 1
     jne @b

     jmp Level_begin

    ;;---Do_smth_between_levels------------------------------------------------------------------------------------------------


Draw_splash:
    ;;===Draw_splash===========================================================================================================

    mov  al,  0
    mov  cl,  GRID_WIDTH-1
    mov  edx, [splash_background_color]

  .draw:
    mov  bh,  GRID_HEIGHT-1
    mov  bl,  al
  @@:
      call      Draw_square
    sub  bh,  2
    cmp  bh,  0
     jg  @b
    
    inc  al
    
    mov  bh,  GRID_HEIGHT-2
    mov  bl,  cl
  @@:
      call      Draw_square
    sub  bh,  2
    cmp  bh,  0
     jnl @b

    dec  cl
    cmp  cl,  0
     jl  @f

    push eax ebx
      mcall     5,PAUSE_WHILE_DRAWING_SPLASH
    pop  ebx eax

     jmp .draw
  @@:

    

    mov  ax,  2*0x100+24
    mov  cx,  1*0x100+5
    mov  edx, [splash_level_string_color]
    mov  esi, picture_level
      call      Draw_picture

    mov  eax, [cur_level_number]
    mov  dl,  10
    div  dl
    push ax
    
    mov  esi, digits_font
  @@:
    test al,  al
     jz  @f
    add  esi, 20
    dec  al
     jmp @b
  @@:
    
    mov  ax,  9*0x100+4
    mov  cx,  8*0x100+5
    mov  edx, [splash_level_number_color]
      call      Draw_picture
    
    pop  ax
    mov  esi, digits_font
  @@:
    test ah,  ah
     jz  @f
    add  esi, 20
    dec  ah
     jmp @b
  @@:

    mov  ax,  15*0x100+4
    mov  cx,  8*0x100+5
    mov  edx, [splash_level_number_color]
      call      Draw_picture

      mcall     5,PAUSE_BETWEEN_LEVELS

    ret

    ;;---Draw_splash-----------------------------------------------------------------------------------------------------------


;;---Some_functions------------------------------------------------------------------------------------------------------------