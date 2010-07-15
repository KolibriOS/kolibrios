;;===Pause_mode================================================================================================================

Pause_mode:

    mov  [action],  0
    mov  eax, [time_wait_limit]
    mov  [time_to_wait],    eax

Pause_Redraw_window:
      mcall     12,1
      mcall     0,200*65536+WINDOW_WIDTH,326*65536+WINDOW_HEIGHT,[window_style], ,window_title

      call      Draw_decorations
      call      Draw_pause_picture
      call      Draw_pause_strings

      mcall     12,2
    
    
Pause_Wait_for_event:
      mcall     10                              ; wait for event
                                                ; ok, what an event?
    dec  al                                     ; has the window been moved or resized?
     jz  Pause_Redraw_window
    dec  al                                     ; was a key pressed?
     jz  Pause_Is_key


Pause_Is_button:                                ; a button was pressed
      mcall     17                              ; get button number
    shr  eax, 8                                 ; we should do it to get the real button code

    cmp  eax, 1
     je  Exit

     jmp Pause_Wait_for_event


Pause_Is_key:                                   ; a key was pressed
      mcall     2                               ; get keycode
    
    cmp  ah,  0x1B                              ; Escape - go to menu
     je  First_menu
    cmp  ah,  0x20                              ; Space - resume game
     je  Level_body
    
     jmp Pause_Wait_for_event

;;---Pause_mode----------------------------------------------------------------------------------------------------------------


;;===Some_functions============================================================================================================

Draw_pause_picture:
    ;;===Draw_pause_picture========================================================================================================

    mov  al,  6
    mov  bh,  2
    mov  ecx, picture_pause
    mov  edx, [pause_picture_color]
      call      Draw_picture

    ret
            
    ;;---Draw_pause_picture--------------------------------------------------------------------------------------------------------
                
            
Draw_pause_strings:
    ;;===Draw_pause_strings================================================================================================

      mcall     4,219*65536+BOTTOM_MIDDLE_STRINGS,[navigation_strings_color],string_resume_space ; Show 'RESUME - SPACE' string
      
    call    Draw_menu_esc                       ; Show 'MENU - ESC' string


    ret

    ;;---Draw_pause_strings------------------------------------------------------------------------------------------------
        
;;---Some_functions------------------------------------------------------------------------------------------------------------