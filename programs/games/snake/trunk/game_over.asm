;;===Game_over_mode============================================================================================================

Game_over:

    cmp  [play_mode],   LEVELS_MODE
     jne @f
    dec  [lives]
     jz  @f
      call      Draw_splash
     jmp Level_begin
  @@:

    mov  byte[window_title+5],  0
      mcall     71,1,window_title
      mcall     66,1,0                          ; set ascii mode for keyboard
      call      Show_cursor

    mov  ebx, [score]
    
    cmp  [play_mode],   CLASSIC_MODE
     jne @f
    cmp  ebx, [hi_score_classic]
     jng .redraw
    mov  [is_new_record], 1
     jmp .done
  @@:
    cmp  ebx, [hi_score_levels]
     jng .redraw
    mov  [is_new_record], 1
     jmp .done
  .done:

      mcall     40,100111b                      ; set events: standart + mouse

  .redraw:
      call      Set_geometry
      mcall     12,1
      mcall     0, , ,[window_style], ,window_title
    test [proc_info.wnd_state], 0x04		; is rolled up?
     jnz @f

      call      Draw_decorations
      call      Draw_game_over_picture
      call      Draw_game_over_strings          ; edit_box is here
      mcall     12,2
  @@:
  .still:
      mcall     10                              ; wait for event
                                                ; ok, what an event?
    dec  al                                     ; has the window been moved or resized?
     jz  .redraw
    dec  al                                     ; was a key pressed?
     jz  .key
    dec  al                                     ; was a button pressed?
     jz  .button


  .mouse:                                       ; mouse event received
      invoke        edit_box.mouse, edit1
    
     jmp .still


  .key:                                         ; a key was pressed
      mcall     2                               ; get keycode

    cmp  [is_new_record],   1
     je  .key.is_record

    cmp  ah,  0x1B                              ; Escape - go to menu
     jne .still
     jmp First_menu

  .key.is_record:
    cmp  ah,  0x0D                              ; Enter
     jnz @f
      call      Score_and_name_store
     jmp First_menu
  @@:
      invoke        edit_box.key, edit1
     jmp .still


  .button:                                      ; a button was pressed
      mcall     17                              ; get button number
    shr  eax, 8                                 ; we should do it to get the real button code

    cmp  eax, 1
     je  Save_do_smth_else_and_exit

     jmp .still

;;---Game_over_mode------------------------------------------------------------------------------------------------------------


;;===Some_functions============================================================================================================

Draw_game_over_picture:
    ;;===Draw_game_over_picture================================================================================================

    cmp  [is_new_record],   1
     je  @f
    mov  ax,  1*0x100+26
    mov  cx,  1*0x100+13
    mov  edx, [game_over_picture_color]
    mov  esi, picture_game_over
     jmp .done
  @@:
    mov  ax,  1*0x100+26
    mov  cx,  1*0x100+12
    mov  edx, [you_win_picture_color]
    mov  esi, picture_you_win
  .done:
      call      Draw_picture

    ret

    ;;---Draw_game_over_picture------------------------------------------------------------------------------------------------


Draw_game_over_strings:
    ;;===Draw_game_over_strings================================================================================================

    cmp  [is_new_record], 1
     jnz @f

    mov  ebx, [window_width]
    shr  ebx, 1
    sub  ebx, string_congratulations.size*3+6*6
    shl  ebx, 16
    add  ebx, [bottom_top_strings]
      mcall     4, ,[game_over_strings_color],string_congratulations
    mov  ebx, [window_width]
    shr  ebx, 1
    sub  ebx, string_enter_your_name.size*3+6*6
    shl  ebx, 16
    add  ebx, [bottom_bottom_strings]
      mcall      , , ,string_enter_your_name
    mov  ebx, [window_width]
    shr  ebx, 1
    sub  ebx, string_apply_name_enter.size*3
    shl  ebx, 16
    add  ebx, [top_strings]
      mcall      , ,[navigation_strings_color],string_apply_name_enter
    mov  edx, [window_width]
    shr  edx, 1
    add  edx, string_congratulations.size*3-6*6-1
    shl  edx, 16
    add  edx, dword[bottom_top_strings]
      mcall     47,0x00070000,[score], ,[game_over_hiscore_color]
    mov  ebx, [window_width]
    shr  ebx, 1
    add  ebx, string_enter_your_name.size*3-6*6-3
    mov  [edit1.left],  ebx
      invoke        edit_box.draw, edit1

    ret

  @@:

    call    Draw_menu_esc

    ret

    ;;---Draw_game_over_strings------------------------------------------------------------------------------------------------


Score_and_name_store:
    ;;===Name_store============================================================================================================

    mov  esi, hed

    cmp  [play_mode],   CLASSIC_MODE
     jne @f
    mov  edi, champion_name_classic
     jmp .done
  @@:
    mov  edi, champion_name_levels
  .done:

    mov  ecx, CHAMPION_NAME_LENGTH
    rep  movsb
    
    mov  eax, [score]

    cmp  [play_mode],   CLASSIC_MODE
     jne @f
    mov  [hi_score_classic],    eax
     jmp .done2
  @@:
    mov  [hi_score_levels], eax
  .done2:

    ret

    ;;---Name_store------------------------------------------------------------------------------------------------------------

;;---Some_functions------------------------------------------------------------------------------------------------------------
