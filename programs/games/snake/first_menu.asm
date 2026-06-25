;;===First_menu_mode===========================================================================================================
First_menu:
    ; set events: standart
    mcall   SF_SET_EVENTS_MASK, EVM_BUTTON or EVM_KEY or EVM_REDRAW
    mcall   SF_KEYBOARD, SSF_SET_INPUT_MODE, 1
    ; set default values
    mov     [is_new_record], 0
    mov     [lives], START_LIVES
    mov     [numberScore.value], 0
    mov     ebx, [time_wait_limit_const]
    mov     [time_wait_limit], ebx
    ; set name game in window_title
    mov     byte[window_title + 5], 0

    call    Show_cursor
    call    Initialize_play_mode

  .redraw:
    call    Set_geometry
    mcall   SF_REDRAW, SSF_BEGIN_DRAW
    mcall   SF_CREATE_WINDOW, , ,[window_style], ,window_title
    mcall   SF_REDRAW, SSF_END_DRAW
    ; is rolled up?
    test    [proc_info.wnd_state], WINDOW_STATE_ROLLED
    jnz     .still

    call    Draw_decorations
    stdcall draw.Picture, 2, 24, 1, 5, [snake_picture_color], picture_first_menu_snake
    stdcall draw.Picture, 3, 11, 8, 5, [version_picture_color], picture_first_menu_version
    stdcall draw.Navigation, labelExit, [posLabel.yTop], 0
    stdcall draw.Navigation, labelStart, [posLabel.yCenter], 0
    call    Draw_buttons

  .still:
    mcall   SF_WAIT_EVENT
    cmpe    al, EV_REDRAW, .redraw
    cmpe    al, EV_KEY, .event_key

  .event_button:
    mcall   SF_GET_BUTTON
    cmpe    ah, BUTTON_CLOSE, Save_do_smth_else_and_exit
    cmpe    ah, BUTTON_EXIT, Save_do_smth_else_and_exit
    cmpe    ah, BUTTON_PLAY, Level_begin
    cmpne   ah, BUTTON_MODE, @f
    call    Change_play_mode
    jmp     .still
  @@:
    cmpne   ah, BUTTON_INC, @f
    call    Increase_geometry
    jmp     .redraw
  @@:
    cmpne   ah, BUTTON_DEC, @f
    call    Decrease_geometry
    jmp     .redraw
  @@:
    jmp     .still

  .event_key:
    mcall   SF_GET_KEY
    cmpe    ah, KEY_CODE_ESC, Save_do_smth_else_and_exit
    cmpe    ah, KEY_CODE_ENTER, Level_begin
    cmpne   ah, KEY_CODE_SPACE, @f
    call    Change_play_mode
    jmp     .still
  @@:
    cmpne   ah, [shortcut_increase], @f
    call    Increase_geometry
    jmp     .redraw
  @@:
    cmpne   ah, [shortcut_decrease], @f
    call    Decrease_geometry
    jmp     .redraw
  @@:
    jmp     .still
;;---First_menu_mode-----------------------------------------------------------------------------------------------------------

; -----------------------------
; Draws game control buttons
; -----------------------------
Draw_buttons:
    stdcall draw.Button, labelButtonPlay, BUTTON_PLAY, [button_x_left], [button_y_top], [button_width_short], [button_height]
    stdcall draw.Button, labelButtonExit, BUTTON_EXIT, [button_x_right], [button_y_top], [button_width_short], [button_height]
    stdcall draw.Button, [playModeLabelButton], BUTTON_MODE, [button_x_left], [button_y_middle], [button_width_long], [button_height]
    stdcall draw.Button, labelButtonInc, BUTTON_INC, [button_x_left], [button_y_bottom], [button_width_short], [button_height]
    stdcall draw.Button, labelButtonDec, BUTTON_DEC, [button_x_right], [button_y_bottom], [button_width_short], [button_height]
    ret
; -----------------------------
; Change the game mode, redraw the game mode button
; -----------------------------
Change_play_mode:
    xor     [play_mode], LEVELS_MODE
    call    Initialize_play_mode
    ; redraw button mode
    cmpne   [play_mode], CLASSIC_MODE, @f
    mov     [playModeLabelButton], labelButtonClassic
  @@:
    cmpne   [play_mode], LEVELS_MODE, @f
    mov     [playModeLabelButton], labelButtonLevels
  @@:
    mcall   SF_DEFINE_BUTTON, , ,0x80000000 or BUTTON_MODE
    stdcall draw.Button, [playModeLabelButton], BUTTON_MODE, [button_x_left], [button_y_middle], [button_width_long], [button_height]
    ret
; -----------------------------
; Initializes the play mode state by setting configuration values
; -----------------------------
Initialize_play_mode:
    cmpne   [play_mode], CLASSIC_MODE, @f
    mov     [numberLevel.value], CLASSIC_MODE_FIRST_LEVEL
    mov     eax, [numberHiscoreClassic]
    mov     esi, labelChampionNameClassic
  @@:
    cmpne   [play_mode], LEVELS_MODE, @f
    mov     [numberLevel.value], LEVELS_MODE_FIRST_LEVEL
    mov     eax, [numberHiscoreLevels]
    mov     esi, labelChampionNameLevels
  @@:
    mov     [numberHiscore.value], eax
    mov     edi, [labelChampionName.value]
    mov     ecx, CHAMPION_NAME_LENGTH
    cld
    rep     movsb
    ret
