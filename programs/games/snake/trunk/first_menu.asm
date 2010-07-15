;;===First_menu_mode===========================================================================================================

First_menu:

    mov [snake_length_x2],  6
    mov word[snake_dots],   0x0303
    mov dword[snake_dots+2],    0x03050304
    mov [snake_napravlenie],    3
    mov [snake_napravlenie_next],   3

Redraw_window:
      mcall     12,1
      mcall     0,200*65536+WINDOW_WIDTH,326*65536+WINDOW_HEIGHT,[window_style], ,window_title

    call    Draw_decorations
    call    Draw_first_menu_picture
    call    Draw_menu_strings

      mcall     12,2

Wait_for_event:
      mcall     10                              ; wait for event
                                                ; ok, what an event?
    dec  al                                     ; has the window been moved or resized?
     jz  Redraw_window                          ; 
    dec  al                                     ; was a key pressed?
     jz  Is_key                                 ; 


Is_button:                                      ; a button was pressed
      mcall     17                              ; get button number
    shr  eax, 8                                 ; we should do it to get the real button code

    cmp  eax, 1                                 ; is it close button?
     je  Exit                                   ; if so, jump to quit...

     jmp Wait_for_event                         ; jump to wait for another event


Is_key:                                         ; a key was pressed
      mcall     2                               ; get keycode

    cmp  ah, 0x1B                               ; Escape
     je  Exit
    cmp  ah, 0x0D                               ; Enter
     je  Level_begin
    cmp  ah, 0x20                               ; Space
     je  Level_begin

     jmp Wait_for_event                         ; jump to wait for another event

;;---First_menu_mode-----------------------------------------------------------------------------------------------------------


;;===Some_functions============================================================================================================

Draw_first_menu_picture:
    ;;===Draw_first_menu_picture================================================================================================

    mov  al,  5
    mov  bh,  0
    mov  ecx, picture_first_menu_snake
    mov  edx, [snake_picture_color]
      call      Draw_picture

    mov  al,  4
    mov  bh,  7
    mov  ecx, picture_first_menu_version
    mov  edx, [version_picture_color]
      call      Draw_picture

    ret
            
    ;;---Draw_first_menu_picture------------------------------------------------------------------------------------------------


Draw_menu_strings:
    ;;===Make_menu_strings=========================================================================================

      mcall     4,153*65536+BOTTOM_MIDDLE_STRINGS,[navigation_strings_color],press_to_start
      mcall     ,213*65536+TOP_STRINGS,[navigation_strings_color],press_esc_to_exit
;      mcall     ,406*65536+TOP_STRINGS,[navigation_strings_color],press_F2_to_options

    ret

    ;;---Make_menu_strings-----------------------------------------------------------------------------------------
    
;;---Some_functions------------------------------------------------------------------------------------------------------------