;;===Pause_mode================================================================================================================

Pause_mode:

      mcall     66,1,1                          ; set scan codes mode for keyboard
      call      Show_cursor
    mov  [action],  0
    mov  eax, [time_wait_limit]
    mov  [time_to_wait],    eax

  .redraw:
      call      Set_geometry
      mcall     12,1
      mcall     0, , ,[window_style], ,window_title
    test [proc_info.wnd_state], 0x04		; is rolled up?
     jnz @f

      call      Draw_decorations
      call      Draw_pause_picture
      call      Draw_pause_strings
      mcall     12,2
  @@:
  .still:
      mcall     10                              ; wait for event
                                                ; ok, what an event?
    dec  al                                     ; has the window been moved or resized?
     jz  .redraw
    dec  al                                     ; was a key pressed?
     jz  .key


  .button:                                      ; a button was pressed
      mcall     17                              ; get button number
    shr  eax, 8                                 ; we should do it to get the real button code

    cmp  eax, 1
     je  Save_do_smth_else_and_exit

     jmp .still


  .key:                                         ; a key was pressed
      mcall     2                               ; get keycode
    
    cmp  ah,  0x01                              ; Escape - go to menu
     je  First_menu
    cmp  ah,  0x39                              ; Space - resume game
     je  Level_body
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
    
     jmp .still

;;---Pause_mode----------------------------------------------------------------------------------------------------------------


;;===Some_functions============================================================================================================

Draw_pause_picture:
    ;;===Draw_pause_picture========================================================================================================

    mov  ax,  2*0x100+24
    mov  cx,  4*0x100+6
    mov  edx, [pause_picture_color]
    mov  esi, picture_pause
      call      Draw_picture

    ret
            
    ;;---Draw_pause_picture--------------------------------------------------------------------------------------------------------
                
            
Draw_pause_strings:
    ;;===Draw_pause_strings================================================================================================

    mov  ebx, [window_width]
    shr  ebx, 1
    sub  ebx, string_resume_space.size*3+6
    shl  ebx, 16
    add  ebx, dword[bottom_middle_strings]
      mcall     4, ,[navigation_strings_color],string_resume_space ; Show 'RESUME - SPACE' string
      
    call    Draw_menu_esc                       ; Show 'MENU - ESC' string


    ret

    ;;---Draw_pause_strings------------------------------------------------------------------------------------------------
        
;;---Some_functions------------------------------------------------------------------------------------------------------------
