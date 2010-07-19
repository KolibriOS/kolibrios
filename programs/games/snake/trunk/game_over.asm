;;===Game_over_mode============================================================================================================

Game_over:

    mov  ebx, [score]
    cmp  ebx, [hi_score]
     jng .redraw
     
    mov  [is_new_record], 1

      mcall     40,100111b                      ; set events: standart + mouse

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
      call      Draw_game_over_picture
      call      Draw_game_over_strings          ; edit_box is here

      mcall     12,2

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
    push dword edit1
      call      [edit_box.mouse]
    
     jmp .still


  .key:                                         ; a key was pressed
      mcall     2                               ; get keycode

    cmp  [is_new_record], 1
     jnz .key.skip
     
    cmp  ah,  0x0D                              ; Enter
     jnz @f
      call      Score_and_name_store
    mov  [is_new_record],   0
      mcall     40,111b                         ; set events: standart
     jmp First_menu

  @@:
    push    dword edit1
    call    [edit_box.key]
    
     jmp .still
  .key.skip:

    cmp  ah,  0x1B                              ; Escape - go to menu
     jne  .still

      mcall     40,111b                         ; set events: standart
     jmp First_menu


  .button:                                      ; a button was pressed
      mcall     17                              ; get button number
    shr  eax, 8                                 ; we should do it to get the real button code

    cmp  eax, 1
     je  Exit

     jmp .still

;;---Game_over_mode------------------------------------------------------------------------------------------------------------


;;===Some_functions============================================================================================================

Draw_game_over_picture:
    ;;===Draw_game_over_picture================================================================================================

    mov  ax,  0*0x100+29
    mov  cx,  1*0x100+13
    mov  edx, [game_over_picture_color]
    mov  esi, picture_game_over
      call      Draw_picture

    ret

    ;;---Draw_game_over_picture------------------------------------------------------------------------------------------------


Draw_game_over_strings:
    ;;===Draw_game_over_strings================================================================================================

    cmp  [is_new_record], 1
     jnz @f

    mov  ebx, [window_width]
    shr  ebx, 1
    sub  ebx, (string_enter_your_name-string_congratulations-1+8)*3+6
    shl  ebx, 16
    add  ebx, [bottom_top_strings]
      mcall     4, ,[game_over_strings_color],string_congratulations
    mov  ebx, [window_width]
    shr  ebx, 1
    sub  ebx, (strings_end-string_enter_your_name-1+8)*3+6
;    add  ebx, (strings_end-string_enter_your_name)*6
    shl  ebx, 16
    add  ebx, [bottom_bottom_strings]
      mcall      , , ,string_enter_your_name
    mov  ebx, [window_width]
    shr  ebx, 1
    sub  ebx, (press_to_start-string_apply_name_enter-1)*3+6
    shl  ebx, 16
    add  ebx, [top_strings]
      mcall      , ,[navigation_strings_color],string_apply_name_enter
    mov  edx, [window_width]
    shr  edx, 1
    sub  edx, (string_enter_your_name-string_congratulations-1+8)*3+7
    add  edx, (string_enter_your_name-string_congratulations)*6
    shl  edx, 16
    add  edx, dword[bottom_top_strings]
      mcall     47,0x00070000,[score], ,[game_over_hiscore_color]
    mov  ebx, [window_width]
    shr  ebx, 1
    sub  ebx, (strings_end-string_enter_your_name-1+8)*3+9
    add  ebx, (strings_end-string_enter_your_name)*6
    mov  [edit1+0x04],  ebx
    push    dword edit1
      call      [edit_box.draw]

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