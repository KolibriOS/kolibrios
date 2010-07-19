;;===First_menu_mode===========================================================================================================

First_menu:

    mov [snake_length_x2],  6
    mov word[snake_dots],   0x0303
    mov dword[snake_dots+2],    0x03050304
    mov [snake_napravlenie],    3
    mov [snake_napravlenie_next],   3

  .redraw:
      mcall     12,1
    mov  ebx, [wp_x]
    shl  ebx, 16
    add  ebx, dword[window_width]
    mov  ecx, [wp_y]
    shl  ecx, 16
    add  ecx, dword[window_height]
      mcall     0, , ,[window_style], ,window_title

    call    Draw_decorations
    call    Draw_first_menu_picture
    call    Draw_menu_strings

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
     je  Exit                                   ; if so, jump to quit...

     jmp .still                                 ; jump to wait for another event


  .key:                                         ; a key was pressed
      mcall     2                               ; get keycode

    cmp  ah, 0x1B                               ; Escape
     je  Exit
    cmp  ah, 0x0D                               ; Enter
     je  Level_begin
    cmp  ah, 0x20                               ; Space
     je  Level_begin

     jmp .still                                 ; jump to wait for another event

;;---First_menu_mode-----------------------------------------------------------------------------------------------------------


;;===Some_functions============================================================================================================

Draw_first_menu_picture:
    ;;===Draw_first_menu_picture================================================================================================

    mov  ax,  0*0x100+29
    mov  cx,  1*0x100+6
    mov  edx, [snake_picture_color]
    mov  esi, picture_first_menu_snake
      call      Draw_picture

    mov  ax,  9*0x100+11
    mov  cx,  9*0x100+5
    mov  edx, [version_picture_color]
    mov  esi, picture_first_menu_version
      call      Draw_picture

    ret
            
    ;;---Draw_first_menu_picture------------------------------------------------------------------------------------------------


Draw_menu_strings:
    ;;===Make_menu_strings=========================================================================================

    mov  ebx, [window_width]
    shr  ebx, 1
    sub  ebx, (press_esc_to_exit-press_to_start-1)*3+6
    shl  ebx, 16
    add  ebx, dword[bottom_middle_strings]
      mcall     4, ,[navigation_strings_color],press_to_start
    mov  ebx, [window_width]
    shr  ebx, 1
    sub  ebx, (string_congratulations-press_esc_to_exit-1)*3+6
    shl  ebx, 16
    add  ebx, [top_strings]
      mcall     , ,[navigation_strings_color],press_esc_to_exit
;      mcall     ,406*65536+TOP_STRINGS,[navigation_strings_color],press_F2_to_options

    ret

    ;;---Make_menu_strings-----------------------------------------------------------------------------------------
    
;;---Some_functions------------------------------------------------------------------------------------------------------------