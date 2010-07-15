;;===Game_over_mode============================================================================================================

Game_over:

    mov  ebx, [score]
    cmp  ebx, [hi_score]
     jng Game_over_Redraw
     
    mov  [is_new_record], 1

      mcall     40,100111b                      ; set events: standart + mouse

Game_over_Redraw:
      mcall     12,1
      mcall     0,200*65536+WINDOW_WIDTH,326*65536+WINDOW_HEIGHT,[window_style], ,window_title
      
    cmp  [is_new_record], 1
     jnz @f
    push    dword edit1
      call      [edit_box.draw]
  @@:

      call      Draw_decorations
      call      Draw_game_over_picture
      call      Draw_game_over_strings

      mcall     12,2

Game_over_Wait_for_event:
      mcall     10                              ; wait for event
                                                ; ok, what an event?
    dec  al                                     ; has the window been moved or resized?
     jz  Game_over_Redraw
    dec  al                                     ; was a key pressed?
     jz  Game_over_key
    dec  al                                     ; was a button pressed?
     jz  Game_over_button


Game_over_mouse:                                ; mouse event received
    push dword edit1
      call      [edit_box.mouse]
    
     jmp Game_over_Wait_for_event


Game_over_key:                                  ; a key was pressed
      mcall     2                               ; get keycode

    cmp  [is_new_record], 1
     jnz .skip
     
    cmp  ah,  0x0D                              ; Enter
     jnz @f
      call      Score_and_name_store
    mov  [is_new_record],   0
      mcall     40,111b                         ; set events: standart
     jmp First_menu

  @@:
    push    dword edit1
    call    [edit_box.key]
    
     jmp Game_over_Wait_for_event
  .skip:

    cmp  ah,  0x1B                              ; Escape - go to menu
     jne  Game_over_Wait_for_event

      mcall     40,111b                         ; set events: standart
     jmp First_menu


Game_over_button:                               ; a button was pressed
      mcall     17                              ; get button number
    shr  eax, 8                                 ; we should do it to get the real button code

    cmp  eax, 1
     je  Exit

     jmp Game_over_Wait_for_event

;;---Game_over_mode------------------------------------------------------------------------------------------------------------


;;===Some_functions============================================================================================================

Draw_game_over_picture:
    ;;===Draw_game_over_picture================================================================================================

    mov  al,  11
    mov  bh,  0
    mov  ecx, picture_game_over
    mov  edx, [game_over_picture_color]
      call      Draw_picture

    ret

    ;;---Draw_game_over_picture------------------------------------------------------------------------------------------------


Draw_game_over_strings:
    ;;===Draw_game_over_strings================================================================================================
        
    
    
    cmp  [is_new_record], 1
     jnz @f

      mcall     4,40*65536+BOTTOM_TOP_STRINGS,[game_over_strings_color],string_congratulations
      mcall      ,244*65536+BOTTOM_BOTTOM_STRINGS, ,string_enter_your_name
      mcall      ,210*65536+TOP_STRINGS,[navigation_strings_color],string_apply_name_enter
      mcall     47,0x00070000,[score],(399)*65536+BOTTOM_TOP_STRINGS,[game_over_hiscore_color]

    ret
    
  @@:
    
    call    Draw_menu_esc

    ret
        
    ;;---Draw_game_over_strings------------------------------------------------------------------------------------------------
    
    
Score_and_name_store:
    ;;===Name_store============================================================================================================
    

      invoke    ini.set_str, cur_dir_path, aScore, aChampion_name, hed, 15
      invoke    ini.set_int, cur_dir_path, aScore, aHiscore, [score]

    ret

    ;;---Name_store------------------------------------------------------------------------------------------------------------

;;---Some_functions------------------------------------------------------------------------------------------------------------