;;===Pause_mode================================================================================================================
Pause_mode:
    mcall   SF_KEYBOARD, SSF_SET_INPUT_MODE, 1
    call    Show_cursor
    mov     [action], 0

  .redraw:
    call    Set_geometry
    mcall   SF_REDRAW, SSF_BEGIN_DRAW
    mcall   SF_CREATE_WINDOW, , ,[window_style], ,window_title
    mcall   SF_REDRAW, SSF_END_DRAW
    ; is rolled up?
    test    [proc_info.wnd_state], WINDOW_STATE_ROLLED
    jnz     .still

    call    Draw_decorations
    ; draw picture pause
    stdcall draw.Picture, 2, 24, 4, 6, [pause_picture_color], picture_pause
    ; draw navigation texts
    stdcall draw.Navigation, labelMenu, [posLabel.yTop], 0
    stdcall draw.Navigation, labelResume, [posLabel.yCenter], 0
  .still:
    mcall   SF_WAIT_EVENT
    cmpe    al, EV_REDRAW, .redraw
    cmpe    al, EV_KEY, .press_key

  .press_button:
    mcall   SF_GET_BUTTON
    cmpe    ah, BUTTON_CLOSE, Save_do_smth_else_and_exit
    jmp     .still

  .press_key:
    mcall   SF_GET_KEY
    cmpe    ah, KEY_CODE_ESC, First_menu
    cmpe    ah, KEY_CODE_SPACE, Level_body
    cmpne   ah, [shortcut_increase], @f
    call    Increase_geometry
    jmp     .redraw
  @@:
    cmpne   ah, [shortcut_decrease], @f
    call    Decrease_geometry
    jmp     .redraw
  @@:
    jmp     .still
;;---Pause_mode----------------------------------------------------------------------------------------------------------------
