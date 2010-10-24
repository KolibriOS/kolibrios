;;===First_menu_mode===========================================================================================================

First_menu:
    mov  byte[window_title+5],  0
      mcall     71,1,window_title
      mcall     40,111b                         ; set events: standart
      mcall     66,1,1                          ; set scan codes mode for keyboard
    mov  [is_new_record],   0
    mov  [lives],   START_LIVES
      call      Show_cursor

    mov  [score],   0
      call      Set_first_level_of_play_mode

    mov  ebx, [time_wait_limit_const]
    mov  [time_wait_limit], ebx

  .redraw:
      call      Set_geometry
      mcall     12,1
      mcall     0, , ,[window_style], ,window_title

      call      Draw_decorations
      call      Draw_first_menu_picture
      call      Draw_menu_strings
      call      Draw_buttons

      mcall     12,2

  .still:
      mcall     10                              ; wait for event
                                                ; ok, what an event?
    dec  al                                     ; has the window been moved or resized?
     jz  .redraw                                ; 
    dec  al                                     ; was a key pressed?
     jz  .key                                   ; 


  .button:                                      ; a button was pressed
      mcall     17                              ; get button number
    shr  eax, 8                                 ; we should do it to get the real button code

    cmp  eax, 1                                 ; is it close button?
     je  Save_do_smth_else_and_exit             ; if so, jump to quit...
    cmp  eax, 0xD0                              ; 'play' button?
     je  Level_begin
    cmp  eax, 0xD1                              ; 'exit' button?
     je  Save_do_smth_else_and_exit
    cmp  eax, 0xD2                              ; change play mode button?
     jne @f
      call      Change_play_mode
      call      Delete_buttons
      call      Draw_buttons
     jmp .still                                 ; jump to wait for another event
  @@:
    cmp  eax, 0xD3                              ; '+INC+' button?
     jne @f
      call      Increase_geometry
     jmp .redraw                                ; jump to wait for another event
  @@:
    cmp  eax, 0xD4                              ; '-dec-' button?
     jne @f
      call      Decrease_geometry
     jmp .redraw                                ; jump to wait for another event
  @@:

     jmp .still                                 ; jump to wait for another event


  .key:                                         ; a key was pressed
      mcall     2                               ; get keycode

    cmp  ah, 0x01                               ; Escape
     je  Save_do_smth_else_and_exit
    cmp  ah, 0x1C                               ; Enter
     je  Level_begin
    cmp  ah, 0x39                               ; Space
     jne @f
      call      Change_play_mode
      call      Delete_buttons
      call      Draw_buttons
     jmp .still                                 ; jump to wait for another event
  @@:
    cmp  ah, [shortcut_increase]
     jne @f
      call      Increase_geometry
     jmp .redraw
  @@:
    cmp  ah, [shortcut_decrease]
     jne @f
      call      Decrease_geometry
     jmp .redraw
  @@:
     jmp .still                                 ; jump to wait for another event

;;---First_menu_mode-----------------------------------------------------------------------------------------------------------


;;===Some_functions============================================================================================================

Draw_first_menu_picture:
    ;;===Draw_first_menu_picture================================================================================================

    mov  ax,  2*0x100+24
    mov  cx,  1*0x100+5
    mov  edx, [snake_picture_color]
    mov  esi, picture_first_menu_snake
      call      Draw_picture

    mov  ax,  3*0x100+11
    mov  cx,  8*0x100+5
    mov  edx, [version_picture_color]
    mov  esi, picture_first_menu_version
      call      Draw_picture

    ret
            
    ;;---Draw_first_menu_picture------------------------------------------------------------------------------------------------


Draw_menu_strings:
    ;;===Make_menu_strings======================================================================================================

    mov  ebx, [window_width]
    shr  ebx, 1
    sub  ebx, press_to_start.size*3+6
    shl  ebx, 16
    add  ebx, dword[bottom_middle_strings]
      mcall     4, ,[navigation_strings_color],press_to_start
    mov  ebx, [window_width]
    shr  ebx, 1
    sub  ebx, press_esc_to_exit.size*3+6
    shl  ebx, 16
    add  ebx, [top_strings]
      mcall     , ,[navigation_strings_color],press_esc_to_exit
;      mcall     ,406*65536+TOP_STRINGS,[navigation_strings_color],press_F2_to_options

    ret

    ;;---Make_menu_strings------------------------------------------------------------------------------------------------------


Draw_buttons:
    ;;===Draw_buttons===========================================================================================================

    mov  ebx, [button_x_left]
    shl  ebx, 16
    mov  bx,  word[button_width_short]
    mov  ecx, [button_y_top]
    shl  ecx, 16
    add  cx,  word[button_height]
      mcall     8, , ,0x000000D0,[button_color]                             ; top left button
    shr  ecx, 16
    mov  bx,  cx
    mov  edx, [button_height]
    shr  edx, 1
    sub  edx, 3                                                             ; ~half of font height
    add  bx,  dx
    ror  ebx, 16
    mov  edx, [button_width_short]
    shr  edx, 1
    sub  edx, string_button_play.size*3
    add  bx,  dx
    ror  ebx, 16
      mcall     4, ,[button_text_color],string_button_play

    mov  ebx, [button_x_right]
    shl  ebx, 16
    mov  bx,  word[button_width_short]
    mov  ecx, [button_y_top]
    shl  ecx, 16
    add  cx,  word[button_height]
      mcall     8, , ,0x000000D1,                                           ; top right button
    shr  ecx, 16
    mov  bx,  cx
    mov  edx, [button_height]
    shr  edx, 1
    sub  edx, 3                                                             ; ~half of font height
    add  bx,  dx
    ror  ebx, 16
    mov  edx, [button_width_short]
    shr  edx, 1
    sub  edx, string_button_exit.size*3
    add  bx,  dx
    ror  ebx, 16
      mcall     4, ,[button_text_color],string_button_exit

    mov  ebx, [button_x_left]
    shl  ebx, 16
    mov  bx,  word[button_width_long]
    mov  ecx, [button_y_middle]
    shl  ecx, 16
    add  cx,  word[button_height]
      mcall     8, , ,0x000000D2,                                           ; middle button
    shr  ecx, 16
    mov  bx,  cx
    mov  edi, [button_height]
    shr  edi, 1
    sub  edi, 3                                                             ; ~half of font height
    add  bx,  di
    ror  ebx, 16
    mov  edi, [button_width_long]
    shr  edi, 1
    cmp  [play_mode],   0
     jne @f
    sub  edi, string_button_pm_classic.size*3
    mov  edx, string_button_pm_classic
     jmp .skip
  @@:
    sub  edi, string_button_pm_levels.size*3
    mov  edx, string_button_pm_levels
  .skip:
    add  bx,  di
    ror  ebx, 16
      mcall     4, ,[button_text_color],

    mov  ebx, [button_x_left]
    shl  ebx, 16
    mov  bx,  word[button_width_short]
    mov  ecx, [button_y_bottom]
    shl  ecx, 16
    add  cx,  word[button_height]
      mcall     8, , ,0x000000D3,                                           ; bottom left button
    shr  ecx, 16
    mov  bx,  cx
    mov  edx, [button_height]
    shr  edx, 1
    sub  edx, 3                                                             ; ~half of font height
    add  bx,  dx
    ror  ebx, 16
    mov  edx, [button_width_short]
    shr  edx, 1
    sub  edx, string_button_inc.size*3
    add  bx,  dx
    ror  ebx, 16
      mcall     4, ,[button_text_color],string_button_inc

    mov  ebx, [button_x_right]
    shl  ebx, 16
    mov  bx,  word[button_width_short]
    mov  ecx, [button_y_bottom]
    shl  ecx, 16
    add  cx,  word[button_height]
      mcall     8, , ,0x000000D4,
    shr  ecx, 16
    mov  bx,  cx
    mov  edx, [button_height]
    shr  edx, 1
    sub  edx, 3                                                             ; ~half of font height
    add  bx,  dx
    ror  ebx, 16
    mov  edx, [button_width_short]
    shr  edx, 1
    sub  edx, string_button_dec.size*3
    add  bx,  dx
    ror  ebx, 16
      mcall     4, ,[button_text_color],string_button_dec

    ret

    ;;---Draw_buttons----------------------------------------------------------------------------------------------------------


Delete_buttons:
    ;;===Delete_buttons========================================================================================================

      mcall     8,,,0x800000D0
      mcall      ,,,0x800000D1
      mcall      ,,,0x800000D2
      mcall      ,,,0x800000D3
      mcall      ,,,0x800000D4

    ret

    ;;---Delete_buttons--------------------------------------------------------------------------------------------------------


Change_play_mode:
    ;;===Change_play_mode======================================================================================================

    cmp  [play_mode],   LEVELS_MODE
     jne @f
    mov  [play_mode],   CLASSIC_MODE
    mov  [cur_level_number],   0
    ret
  @@:
    inc  [play_mode]

      call      Set_first_level_of_play_mode

    ret

    ;;---Change_play_mode------------------------------------------------------------------------------------------------------


Set_first_level_of_play_mode:
    ;;===Set_first_level_of_play_mode==========================================================================================

    cmp  [play_mode],   CLASSIC_MODE
     jne @f
    mov  [cur_level_number],   CLASSIC_MODE_FIRST_LEVEL
  @@:
    cmp  [play_mode],   LEVELS_MODE
     jne @f
    mov  [cur_level_number],   LEVELS_MODE_FIRST_LEVEL
  @@:

    ret

    ;;---Set_first_level_of_play_mode-------------------------------------------------------------------------------------------

;;---Some_functions-------------------------------------------------------------------------------------------------------------