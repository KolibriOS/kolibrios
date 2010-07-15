;;===Level_mode================================================================================================================

Level_begin:

    mov  [score],   0
    mov  [action],  0
    mov  [number_of_free_dots], GRID_WIDTH*GRID_HEIGHT-3

      invoke  ini.get_str, cur_dir_path, aScore, aChampion_name, champion_name, 15, champion_name
      invoke  ini.get_int, cur_dir_path, aScore, aHiscore, 0
      
    test eax, eax
     jz  @f
    mov  dword  [hi_score],    eax
  @@:

    mov  esi, start_map
    mov  edi, field_map
    mov  ecx, GRID_WIDTH*GRID_HEIGHT/4
    rep  movsd

Level_body:
    ;;===Level_body========================================================================================================

mcall     26, 9
    mov  [time_before_waiting], eax
    mov  eax, [time_wait_limit]
    mov  [time_to_wait],    eax

  Redraw:
      mcall     12,1
      mcall     0,200*65536+WINDOW_WIDTH,326*65536+WINDOW_HEIGHT,[window_style], ,window_title
    
      call      Draw_decorations
      call      Draw_snake
      call      Draw_eat
      call      Draw_level_strings

      mcall     12,2
    
  Waiting:
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
     jmp Waiting
  @@:

      Message:                                  ; ok, what an event?
        dec  al                                 ; has the window been moved or resized?
         jz  Redraw                             ;
        dec  al                                 ; was a key pressed?
         jz  Key                                ; 
        dec  al                                 ; was a button pressed?
         jz  Button                             ; 


  Key:
      mcall     2                               ; get keycode
    
    cmp  ah,  0x1B                              ; Escape
     je  First_menu
    cmp  ah,  0x20                              ; Space
     je  Pause_mode
    cmp  ah,  0xB0                              ; Left
     je  .left
    cmp  ah,  0xB1                              ; Down
     je  .down
    cmp  ah,  0xB2                              ; Up
     je  .up
    cmp  ah,  0xB3                              ; Right
     je  .right
    
     jmp Waiting                                ; jump to wait for another event
        
        
  .left:
    cmp  [action],  0
     jne @f
    mov  [time_to_wait],    0
  @@:
    mov  [action],  1
    mov  [snake_napravlenie_next],  0
     jmp Waiting
            
  .down:
    cmp  [action],  0
     jne @f
    mov  [time_to_wait],    0
  @@:
    mov  [action],  1
    mov  [snake_napravlenie_next],  1
     jmp Waiting
            
  .up:
    cmp  [action],  0
     jne @f
    mov  [time_to_wait],    0
  @@:
    mov  [action],  1        
    mov  [snake_napravlenie_next],  2
     jmp Waiting
            
  .right:
    cmp  [action],  0
     jne @f
    mov  [time_to_wait],    0
  @@:
    mov  [action],  1           
    mov  [snake_napravlenie_next],  3
     jmp Waiting
            

  Button:                                       ; процедура обрабоки кнопок в программе
      mcall     17                              ; функция 17: получить номер нажатой кнопки

    shr  eax, 8                                 ; сдвигаем регистр eax на 8 бит вправо, чтобы получить номер нажатой кнопки
    cmp  eax, 1
     je  Exit                                   ; если это не кнопка 1 (зарезервирована системой как кнопка закрытия программы), пропускаем 2 следующие строчки кода

     jmp Waiting
        
        
  Game_step:

    cmp  [snake_napravlenie],   LEFT            ; are we moving to left?
     jz  .left
    cmp  [snake_napravlenie],   DOWN            ; ... down?
     jz  .down
    cmp  [snake_napravlenie],   UP              ; ... up?
     jz  .up
     jmp .right                                 ; then right
     
  .left:
    cmp  [snake_napravlenie_next],  RIGHT       ; next step is to right?
     jz  .with_rewerse
     jmp .without_rewerse
  
  .down:
    cmp  [snake_napravlenie_next],  UP          ; next step is to up?
     jz  .with_rewerse
     jmp .without_rewerse
     
  .up:
    cmp  [snake_napravlenie_next],  DOWN        ; next step is to bottom?
     jz  .with_rewerse
     jmp .without_rewerse

  .right:
    cmp  [snake_napravlenie_next],  LEFT        ; next step is to left?
     jz  .with_rewerse
     jmp .without_rewerse


  .with_rewerse:
      call      Set_reverse_napravlenie
      call      Reverse
        
  .without_rewerse:
;    mov  [time_to_wait],   0
    mov  edx, snake_dots-2
    add  edx, [snake_length_x2]
        
    cmp  [snake_napravlenie_next],  LEFT
     je  .to_left
    cmp  [snake_napravlenie_next],  DOWN
     je  .to_down 
    cmp  [snake_napravlenie_next],  UP
     je  .to_up
    cmp  [snake_napravlenie_next],  RIGHT
     je  .to_right    
        
      .to_left:
        mov  [snake_napravlenie],   LEFT
        mov  ax,  [edx]
        dec  al
        cmp  al,  -1
         jne @f
        mov  al,  GRID_WIDTH-1
      @@:
         jmp Snake_move

      .to_down:
        mov  [snake_napravlenie],   DOWN
        mov  ax,  [edx]
        inc  ah
        cmp  ah,  GRID_HEIGHT
         jne @f
        mov  ah,  0
      @@:
         jmp Snake_move
        
      .to_up:
        mov  [snake_napravlenie],   UP
        mov  ax,  [edx]
        dec  ah
        cmp  ah,  -1
         jne @f
        mov  ah,  GRID_HEIGHT-1
      @@:
         jmp Snake_move
        
      .to_right:
        mov  [snake_napravlenie],   RIGHT
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

    call    Draw_menu_esc
    call    Draw_score_string
    call    Draw_score_number                   ; Draw score (number)
    call    Draw_hiscore_string
    call    Draw_hiscore_number
    call    Draw_champion_string
    call    Draw_champion_name

      mcall     4,225*65536+BOTTOM_MIDDLE_STRINGS,[navigation_strings_color],string_pause_space ; Draw 'PAUSE - SPACE' string
        
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

    dec  cx
    add  esi, 2
    sub  edi, 2

    test  cx, cx
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


Set_reverse_napravlenie:
    ;;===Set_reverse_napravlenie===========================================================================================

    mov  eax, snake_dots
    mov  ebx, snake_dots+2

    mov  cl,  [eax]                             ; The last dot x_coord
    mov  dl,  [ebx]                             ; The pre_last dot x_coord

    cmp  cl,  dl
     je  .X_ravny
    
    cmp  cl,  0
     jne .skip2
    
    cmp  dl,  23
     jne .Normal_y_ravny
    mov  [snake_napravlenie_next],  3
    ret
        
  .skip2:
    cmp  cl,  23
     jne .Normal_y_ravny
    cmp  dl,  0
     jne .Normal_y_ravny
    mov  [snake_napravlenie_next],  0
    ret
    
  .Normal_y_ravny:

    cmp  cl,  dl
     jg  .Napravlenie_to_right
    mov  [snake_napravlenie_next],  0
    ret

  .Napravlenie_to_right:
    mov  [snake_napravlenie_next],  3
    ret

  .X_ravny:
    inc  eax
    inc  ebx
    mov  cl,  [eax]
    mov  dl,  [ebx]
    
    cmp  cl,  0
     jne .skip3
    
    cmp  dl,  10
     jne .Normal_x_ravny
    mov  [snake_napravlenie_next],  1
    ret
        
  .skip3:
    cmp  cl,  10
     jne .Normal_x_ravny
    cmp  dl,  0
     jne .Normal_x_ravny
    mov  [snake_napravlenie_next],  2
    ret
    
  .Normal_x_ravny:

    cmp  cl,  dl                                ; !!!
     jg  .Napravlenie_to_down                   ; 0 1 2 ...
    mov  [snake_napravlenie_next],  2           ; 1
    ret                                         ; 2
                                                ; .
  .Napravlenie_to_down:                         ; .
    mov  [snake_napravlenie_next],  1           ; .

    ret

    ;;---Set_reverse_napravlenie-------------------------------------------------------------------------------------------
    
    
Snake_move:
    ;;===Snake_move========================================================================================================
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
    dec  word[number_of_free_dots]
    cmp  word[number_of_free_dots], 0
     je  Game_over
    mov  ax,  word[eat]
    mov  cl,  1
      call      Draw_on_map
      call      Draw_head_prehead
      call      Get_eat
      call      Draw_eat

     jmp Keys_done


  .eat_and_new_head_are_different:

    mov  ecx, snake_dots-4
    add  ecx, [snake_length_x2]

      call      Get_from_map
    test bl,  bl
     jnz Game_over

    mov  cl,  1
      call      Draw_on_map
      
    mov  bx,  word[snake_dots]
    mov  edx, [background_color]
      call      Draw_square

    mov  ax,  word[snake_dots]
    mov  cl,  0
      call      Draw_on_map
      call      Sdvig
      
      call      Draw_head_prehead


  Keys_done:

    cmp  [score],   0
     je  @f
    dec  [score]
      call      Draw_score_number
  @@:
      mcall     26, 9
    mov  [time_before_waiting], eax
    mov  eax, [time_wait_limit]
    mov  [time_to_wait],    eax
     jmp Waiting

    ;;---Snake_move--------------------------------------------------------------------------------------------------------

;;---Some_functions------------------------------------------------------------------------------------------------------------