;;===Game_over_mode============================================================================================================
Game_over:
    cmpne   [play_mode], LEVELS_MODE, @f
    dec     [lives]
    jz      @f
    call    Draw_splash
    jmp     Level_begin
  @@:
    mcall   SF_KEYBOARD, SSF_SET_INPUT_MODE, 0
    call    Show_cursor

    ; set name game in window_title
    mov     byte[window_title + 5], 0
    mov     ebx, [numberScore.value]

    cmpne   [play_mode], CLASSIC_MODE, @f
    cmpng   ebx, [numberHiscoreClassic], .redraw
    mov     [is_new_record], 1
    jmp     .done
  @@:
    cmpng   ebx, [numberHiscoreLevels], .redraw
    mov     [is_new_record], 1
    jmp     .done
  .done:
    mcall   SF_SET_EVENTS_MASK, EVM_MOUSE or EVM_BUTTON or EVM_KEY or EVM_REDRAW

  .redraw:
    call    Set_geometry
    mcall   SF_REDRAW, SSF_BEGIN_DRAW
    mcall   SF_CREATE_WINDOW, , ,[window_style], ,window_title
    mcall   SF_REDRAW, SSF_END_DRAW
    ; is rolled up?
    test    [proc_info.wnd_state], WINDOW_STATE_ROLLED
    jnz     .still

    call    Draw_decorations
    cmpe    [is_new_record], 1, @f
    call    Draw_game_over
    jmp     .still
  @@:
    call    Draw_new_record
  .still:
    mcall   SF_WAIT_EVENT
    cmpe    al, EV_REDRAW, .redraw
    cmpe    al, EV_KEY, .press_key
    cmpe    al, EV_BUTTON, .press_button

  .mouse:
    invoke  edit_box.mouse, edit1
    jmp     .still

  .press_key:
    mcall   SF_GET_KEY
    ; get scancode key
    mov     ebx, eax
    shr     ebx, 8
    cmpe    [is_new_record], 1, .key.is_record
    cmpne   bh, KEY_CODE_ESC, .still
    jmp     First_menu
  .key.is_record:
    cmpne   bh, KEY_CODE_ENTER, @f
    call    Score_and_name_store
    jmp     First_menu
  @@:
    invoke  edit_box.key, edit1
    jmp     .still

  .press_button:
    mcall   SF_GET_BUTTON
    cmpe    ah, BUTTON_CLOSE, Save_do_smth_else_and_exit
    jmp     .still
;;---Game_over_mode------------------------------------------------------------------------------------------------------------

; -----------------------------
; Draws picture game over and navigation text
; -----------------------------
Draw_game_over:
    stdcall draw.Picture, 1, 26, 1, 13, [game_over_picture_color], picture_game_over
    stdcall draw.Navigation, labelMenu, [posLabel.yTop], 0
    ret
; -----------------------------
; Draws picture of congratulations and navigation text
; Draws text field for input champion name
; -----------------------------
Draw_new_record:
    stdcall draw.Picture, 1, 26, 1, 12, [you_win_picture_color], picture_you_win
    stdcall draw.Navigation, labelApply, [posLabel.yTop], 0
    mov     eax, [numberScore.value]
    mov     [numberGameOver.value], eax
    stdcall draw.Navigation, labelCongratulations, [posLabel.yCenter], -numberGameOver.len
    stdcall draw.NavigationNumber, numberGameOver, [posLabel.yCenter], labelCongratulations.len
    stdcall draw.Navigation, labelEnterName, [posLabel.yBottom], -numberGameOver.len - 3
    ; set position for edit box
    stdcall draw.GetNavigationX, -labelCongratulations.len+numberGameOver.len
    mov     [edit1.left], eax
    mov     eax, [posLabel.yBottom]
    sub     eax, 3
    mov     ebx, [configFont.height]
    shr     ebx, 1
    sub     eax, ebx
    mov     [edit1.top], eax
    mov     eax, [configFont.mask]
    or      [edit1.text_color], eax
    invoke  edit_box.draw, edit1
    ret
; -----------------------------
; Save new hiscore and champion name depending on the mode
; -----------------------------
Score_and_name_store:
    mov     eax, [numberScore.value]
    mov     esi, hed
    cmpne   [play_mode], CLASSIC_MODE, @f
    mov     edi, labelChampionNameClassic
    mov     [numberHiscoreClassic], eax
    jmp     .done
  @@:
    mov     [numberHiscoreLevels], eax
    mov     edi, labelChampionNameLevels
  .done:
    mov     ecx, CHAMPION_NAME_LENGTH
    rep     movsb
    ret
