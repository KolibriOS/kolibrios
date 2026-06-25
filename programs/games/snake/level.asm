;;===Level_mode================================================================================================================
Level_begin:
    mcall   SF_KEYBOARD, SSF_SET_INPUT_MODE, 1
    call    Load_level
    call    Get_eat
Level_body:
    ;;===Level_body========================================================================================================
    ;call    Hide_cursor
    mcall   SF_SYSTEM_GET, SSF_TIME_COUNT
    mov     [time_before_waiting], eax
    mov     eax, [time_wait_limit]
    mov     [time_to_wait], eax

  .redraw:
    call    Set_geometry
    mcall   SF_REDRAW, SSF_BEGIN_DRAW
    mcall   SF_CREATE_WINDOW,,, [window_style],, window_title
    mcall   SF_REDRAW, SSF_END_DRAW
    ; is rolled up?
    test    [proc_info.wnd_state], WINDOW_STATE_ROLLED
    jnz     Pause_mode

    call    Draw_decorations
    stdcall draw.Picture, 0, GRID_WIDTH, 0, GRID_HEIGHT, [stone_color], [cur_level]
    call    Draw_snake
    call    Draw_eat
    call    Draw_level_strings

  .still:
    mcall   SF_SYSTEM_GET, SSF_TIME_COUNT
    push    eax
    sub     eax, [time_before_waiting]
    pop     [time_before_waiting]
    cmp     [time_to_wait], eax
    jg      @f
    cmp     [action], 0
    jne     Game_step
  @@:
    sub     [time_to_wait], eax
    mcall   SF_WAIT_EVENT_TIMEOUT, [time_to_wait]
    test    al, al
    jnz     .message
    cmp     [action], 0
    jne     Game_step
    mcall   SF_SYSTEM_GET, SSF_TIME_COUNT
    mov     [time_before_waiting], eax
    mov     eax, [time_wait_limit]
    mov     [time_to_wait], eax
    jmp     .still

  .message:
    cmpe    al, EV_REDRAW, .redraw
    cmpe    al, EV_KEY, .event_key
    cmpe    al, EV_BUTTON, .event_button

  .event_button:
    mcall   SF_GET_BUTTON
    cmpe    ah, BUTTON_CLOSE, Save_do_smth_else_and_exit
    jmp     .still

  .event_key:
    mcall   SF_GET_KEY
    cmpe    ah, KEY_CODE_ESC, First_menu
    cmpe    ah, KEY_CODE_SPACE, Pause_mode
    cmpe    ah, KEY_CODE_LEFT, .key.left
    cmpe    ah, [shortcut_move_left], .key.left
    cmpe    ah, KEY_CODE_DOWN, .key.down
    cmpe    ah, [shortcut_move_down], .key.down
    cmpe    ah, KEY_CODE_UP, .key.up
    cmpe    ah, [shortcut_move_up], .key.up
    cmpe    ah, KEY_CODE_RIGHT, .key.right
    cmpe    ah, [shortcut_move_right], .key.right
    ; key released
    cmpe    ah, KEY_CODE_LEFT + 0x80, .key.released.left
    mov     al, [shortcut_move_left]
    add     al, 0x80
    cmpe    ah, al, .key.released.left
    cmpe    ah, KEY_CODE_DOWN + 0x80, .key.released.down
    mov     al, [shortcut_move_down]
    add     al, 0x80
    cmpe    ah, al, .key.released.down
    cmpe    ah, KEY_CODE_UP + 0x80, .key.released.up
    mov     al, [shortcut_move_up]
    add     al, 0x80
    cmpe    ah, al, .key.released.up
    cmpe    ah, KEY_CODE_RIGHT + 0x80, .key.released.right
    mov     al, [shortcut_move_right]
    add     al, 0x80
    cmpe    ah, al, .key.released.right

    cmpne   ah, [shortcut_reverse], @f
    call    Reverse_snake
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
    jmp     .still                                 ; jump to wait for another event

  .key.left:
    bts     [action], 0
    jc      @f
    mov     [time_to_wait], 0
  @@:
    cmpne   [smart_reverse], 1, @f
    cmpe    [snake_direction], RIGHT, .still
  @@:
    mov     [snake_direction_next], LEFT
    bts     [acceleration_mask], LEFT
    jc      Game_step
    jmp     .still

  .key.down:
    bts     [action], 0
    jc      @f
    mov     [time_to_wait], 0
  @@:
    cmpne   [smart_reverse], 1, @f
    cmpe    [snake_direction], UP, .still
  @@:
    mov     [snake_direction_next], DOWN
    bts     [acceleration_mask], DOWN
    jc      Game_step
    jmp     .still

  .key.up:
    bts     [action], 0
    jc      @f
    mov     [time_to_wait], 0
  @@:
    cmpne   [smart_reverse], 1, @f
    cmpe    [snake_direction], DOWN, .still
  @@:
    mov     [snake_direction_next], UP
    bts     [acceleration_mask], UP
    jc      Game_step
    jmp     .still

  .key.right:
    bts     [action], 0
    jc      @f
    mov     [time_to_wait], 0
  @@:
    cmpne   [smart_reverse], 1, @f
    cmpe    [snake_direction], LEFT, .still
  @@:
    mov     [snake_direction_next], RIGHT
    bts     [acceleration_mask], RIGHT
    jc      Game_step
    jmp     .still

  .key.released.left:
    btr     [acceleration_mask], LEFT
    jmp     .still

  .key.released.down:
    btr     [acceleration_mask], DOWN
    jmp     .still

  .key.released.up:
    btr     [acceleration_mask], UP
    jmp     .still

  .key.released.right:
    btr     [acceleration_mask], RIGHT
    jmp     .still

  Game_step:
    cmpe    [snake_direction], LEFT, .left
    cmpe    [snake_direction], DOWN, .down
    cmpe    [snake_direction], UP, .up
    jmp     .right                                 ; then right

  .left:
    cmpe    [snake_direction_next], RIGHT, .with_reverse
    jmp     .without_reverse

  .down:
    cmpe    [snake_direction_next], UP, .with_reverse
    jmp     .without_reverse

  .up:
    cmpe    [snake_direction_next], DOWN, .with_reverse
    jmp     .without_reverse

  .right:
    cmpe    [snake_direction_next], LEFT, .with_reverse
    jmp     .without_reverse

  .with_reverse:
    call    Set_reverse_direction
    call    Reverse

  .without_reverse:
    mov     edx, snake_dots - 2
    add     edx, [snake_length_x2]

    cmpe    [snake_direction_next], LEFT, .to_left
    cmpe    [snake_direction_next], DOWN, .to_down
    cmpe    [snake_direction_next], UP, .to_up
    cmpe    [snake_direction_next], RIGHT, .to_right

  .to_left:
    mov     [snake_direction], LEFT
    mov     ax,  [edx]
    dec     al
    cmp     al, -1
    jne     @f
    mov     al, GRID_WIDTH
    dec     al
  @@:
    jmp     Snake_move

  .to_down:
    mov     [snake_direction], DOWN
    mov     ax, [edx]
    inc     ah
    cmp     ah, GRID_HEIGHT
    jne     @f
    mov     ah, 0
  @@:
    jmp     Snake_move

  .to_up:
    mov     [snake_direction], UP
    mov     ax, [edx]
    dec     ah
    cmp     ah, -1
    jne     @f
    mov     ah, GRID_HEIGHT
    dec     ah
  @@:
    jmp     Snake_move

  .to_right:
    mov     [snake_direction], RIGHT
    mov     ax, [edx]
    inc     al
    cmp     al, GRID_WIDTH
    jne     @f
    mov     al, 0
  @@:
    jmp     Snake_move

  ;;---Level_body--------------------------------------------------------------------------------------------------------

;;---Level_mode----------------------------------------------------------------------------------------------------------------

;;===Some_functions============================================================================================================

Draw_snake:
    ;;===Draw_snake========================================================================================================
    call    Draw_head_prehead
    mov     ebx, snake_dots - 6
    add     ebx, [snake_length_x2]
  @@:
    stdcall draw.Square, [ebx], [snake_color]
    sub     ebx, 2
    cmpne   ebx, snake_dots - 2, @b
    ret
    ;;---Draw_snake--------------------------------------------------------------------------------------------------------

Draw_head_prehead:
    ;;===Draw_head_prehead=================================================================================================
    mov     ebx, snake_dots - 2
    add     ebx, [snake_length_x2]
    stdcall draw.Square, [ebx], [snake_head_color]
    sub     ebx, 2
    stdcall draw.Square, [ebx], [snake_color]
    call    Draw_lives_in_head
    ret
    ;;---Draw_head_prehead-------------------------------------------------------------------------------------------------

Draw_level_strings:
    ;;===Draw_level_strings================================================================================================
    stdcall draw.Navigation, labelPause, [posLabel.yTop], 0

    stdcall draw.Label, labelScore, [posLabel.xLeft], [posLabel.yCenter], 3
    stdcall draw.Number, numberScore, [posLabel.xLeft], [posLabel.yCenter], labelScore.len+3

    stdcall draw.Label, labelChampion, [posLabel.xRight], [posLabel.yBottom], -labelChampion.len-numberHiscore.len-3
    stdcall draw.Label, labelChampionName, [posLabel.xRight], [posLabel.yBottom], -numberHiscore.len-3

    stdcall draw.Label, labelHiscore, [posLabel.xRight], [posLabel.yCenter], -labelHiscore.len-numberHiscore.len-3
    stdcall draw.Number, numberHiscore, [posLabel.xRight], [posLabel.yCenter], -numberHiscore.len-3

    cmpne   [play_mode], LEVELS_MODE, @f

    stdcall draw.Label, labelLevel, [posLabel.xLeft], [posLabel.yBottom], 3
    stdcall draw.Number, numberLevel, [posLabel.xLeft], [posLabel.yBottom], labelLevel.len+3
  @@:
    ret
    ;;---Draw_level_strings------------------------------------------------------------------------------------------------

Reverse:
    ;;===Reverse===========================================================================================================
    mov     ecx, [snake_length_x2]
    shr     ecx, 2
    mov     esi, snake_dots
    mov     edi, snake_dots - 2
    add     edi, [snake_length_x2]

  @@:
    mov     ax, [edi]
    xchg    ax, [esi]
    mov     [edi], ax

    add     esi, 2
    sub     edi, 2
    dec     cx
    jnz     @b

    ret
    ;;---Reverse-----------------------------------------------------------------------------------------------------------

Draw_eat:
    stdcall draw.Square, dword[eat], [eat_color]
    ret

Get_eat:
    ;;===Get_eat===========================================================================================================
    ;;  in  :
    ;;
    ;;  out :
    ;;          ax  =   coord's of the eat square (al=x, ah=y)
    ;;
    mcall   SF_SYSTEM_GET, SSF_TIME_COUNT
    shl     eax, 1
    xor     edx, edx
    div     word[number_of_free_dots]
    mov     ebx, field_map
  .loop:
    cmp     byte[ebx], 0
    jne     @f
    test    dx, dx
    jz      .place_found
    dec     dx
  @@:
    inc     ebx
    jmp     .loop

  .place_found:
    sub     ebx, field_map
    mov     eax, ebx
    mov     bl, GRID_WIDTH
    div     bl
    xchg    al, ah
    mov     word[eat], ax
    ret
    ;;---Get_eat-----------------------------------------------------------------------------------------------------------

Sdvig:
    ;;===Sdvig=============================================================================================================

    mov     esi, snake_dots + 2
    mov     edi, snake_dots
    mov     ecx, [snake_length_x2]
    shr     ecx, 1
    cld
    rep     movsw
    ret
    ;;---Sdvig-------------------------------------------------------------------------------------------------------------

Set_reverse_direction:
    ;;===Set_reverse_direction==================================================================================================
    mov     eax, snake_dots
    mov     ebx, snake_dots+2
    mov     cl, [eax]                             ; The last dot x_coord
    mov     ch, [ebx]                             ; The pre_last dot x_coord
    cmp     cl, ch
    je      .X_ravny
    cmp     cl, 0
    jne     .skip2

    mov     dl, GRID_WIDTH
    dec     dl
    cmp     ch, dl
    jne     .Normal_y_ravny
    mov     [snake_direction_next], RIGHT
    ret

  .skip2:
    mov     dl, GRID_WIDTH
    dec     dl
    cmp     cl, dl
    jne     .Normal_y_ravny
    cmp     ch, 0
    jne     .Normal_y_ravny
    mov     [snake_direction_next], LEFT
    ret

  .Normal_y_ravny:
    cmp     cl, ch
    jg      .Napravlenie_to_right
    mov     [snake_direction_next], LEFT
    ret

  .Napravlenie_to_right:
    mov     [snake_direction_next], RIGHT
    ret

  .X_ravny:
    inc     eax
    inc     ebx
    mov     cl, [eax]
    mov     ch, [ebx]
    cmp     cl, 0
    jne     .skip3
    mov     dl, GRID_HEIGHT
    dec     dl
    cmp     ch, dl
    jne     .Normal_x_ravny
    mov     [snake_direction_next], DOWN
    ret

  .skip3:
    mov     dl, GRID_HEIGHT
    dec     dl
    cmp     ch, dl
    jne     .Normal_x_ravny
    cmp     ch, 0
    jne     .Normal_x_ravny
    mov     [snake_direction_next], UP
    ret

  .Normal_x_ravny:
    cmp     cl, ch                              ; !!!
    jg      .Napravlenie_to_down                ; 0 1 2 ...
    mov     [snake_direction_next], UP          ; 1
    ret                                         ; 2
                                                ; .
  .Napravlenie_to_down:                         ; .
    mov     [snake_direction_next], DOWN        ; .
    ret
    ;;---Set_reverse_direction--------------------------------------------------------------------------------------------------

Snake_move:
    ;;===Snake_move=============================================================================================================
    ;;  in  :
    ;;           ax =   coord's of new head
    ;;          edx =   snake_dots+[snake_length_x2]-2 (snake head)
    ;;
    add     edx, 2
    mov     [edx], ax
    cmp     ax, word[eat]
    jne     .eat_and_new_head_are_different

    add     [snake_length_x2], 2
    add     [numberScore.value], SCORE_EAT
    dec     [number_of_free_dots]
    cmp     [number_of_free_dots], 0
    je      Game_over
    mov     ax, word[eat]
    mov     cl, 1
    call    Draw_on_map
    call    Draw_head_prehead

    cmp     [play_mode], CLASSIC_MODE
    jne     .is_not_classic_mode
    dec     byte[speed_up_counter]
    jns     @f
    mov     al, byte[speed_up_counter + 1]
    mov     byte[speed_up_counter], al
    cmp     [time_wait_limit], 4
    jl      @f
    dec     [time_wait_limit]
  @@:

  .is_not_classic_mode:
    cmp     [play_mode], LEVELS_MODE
    jne     .is_not_levels_mode
    cmp     [snake_length_x2], (EAT_TO_END_LEVEL+3)*2
    je      .skip

  .is_not_levels_mode:
    call    Get_eat
    call    Draw_eat
  .skip:
    jmp     Keys_done

  .eat_and_new_head_are_different:
    push    ax
    mov     ax, word[snake_dots]
    mov     cl, 0
    call    Draw_on_map
    pop     ax
    call    Get_from_map
    test    bl, bl
    jnz     Game_over
    mov     cl, 1
    call    Draw_on_map
    stdcall draw.Square, dword[snake_dots], [background_color]
    call    Sdvig
    call    Draw_head_prehead

  Keys_done:
    cmp     [numberScore.value], 0
    je      @f
    dec     [numberScore.value]
    stdcall draw.Number, numberScore, [posLabel.xLeft], [posLabel.yCenter], labelScore.len+3
  @@:
    cmp     [play_mode], LEVELS_MODE
    jne     @f
    cmp     [snake_length_x2], (EAT_TO_END_LEVEL+3)*2
    je      Do_smth_between_levels
  @@:
    mcall   SF_SYSTEM_GET, SSF_TIME_COUNT
    mov     [time_before_waiting], eax
    mov     eax, [time_wait_limit]
    mov     [time_to_wait], eax
    jmp     Level_body.still
    ;;---Snake_move------------------------------------------------------------------------------------------------------------

Do_smth_between_levels:
    ;;===Do_smth_between_levels================================================================================================
    inc     [numberLevel.value]
    cmp     [numberLevel.value], LAST_LEVEL_NUMBER + 1
    jne     @f
    mov     [numberLevel.value], LEVELS_MODE_FIRST_LEVEL
  @@:
    call    Draw_splash
    jmp     Level_begin
    ;;---Do_smth_between_levels------------------------------------------------------------------------------------------------

; -----------------------------
; Animation when changing the level
; -----------------------------
Draw_splash:
    mov     cl, GRID_WIDTH - 1
  .x_loop:
    mov     ch, GRID_HEIGHT - 1
  .y_loop:
    ; fill in the line on the left
    stdcall draw.Square, ecx, [splash_background_color]
    ; fill in the next line on the right
    dec     ch
    ; forming the coordinate
    mov     eax, ecx
    mov     al, GRID_WIDTH - 1
    sub     al, cl
    stdcall draw.Square, eax, [splash_background_color]
    dec     ch
    ; fill in all lines until y >= 0
    jns     .y_loop
    dec     cl
    ; draw picture if all filled
    js      .picture

    ; run pause draw press key for skip splash
    mov     eax, PAUSE_WHILE_DRAWING_SPLASH
    call    Wait_level_pause
    cmpne   eax, -1, .quit
    jmp     .x_loop

  .picture:
    ; draw picture text 'level'
    stdcall draw.Picture, 2, 24, 1, 5, [splash_level_string_color], picture_level
    ; draw picture number level
    mov     eax, [numberLevel.value]
    mov     ebx, 10
    xor     ecx, ecx
  .convert:
    xor     edx, edx
    div     ebx
    push    edx
    inc     ecx
    test    eax, eax
    jnz     .convert
    cmpge   [numberLevel.value], ebx, .draw_number
    ; add leading zero
    push    0
    inc     ecx
  .draw_number:
    ; begin position X
    mov     ebx, 9
  .draw_loop:
    pop     esi
    lea     esi, [esi + esi*4]
    add     esi, digits_font
    stdcall draw.Picture, ebx, 4, 8, 5, [splash_level_number_color], esi
    add     ebx, 6
    dec     ecx
    jnz     .draw_loop
    ; pause
    mov     eax, PAUSE_BETWEEN_LEVELS
    call    Wait_level_pause
  .quit:
    ret
; -----------------------------
; Starts waiting between levels with the option to skip
; waiting by press key space or enter
; input: eax - pause duration (ms)
; output: eax - key code (from SF_GET_KEY), -1 if timeout
; -----------------------------
Wait_level_pause:
    mov     esi, eax
    ; start time
    mcall   SF_SYSTEM_GET, SSF_TIME_COUNT
    mov     edi, eax
  .wait_loop:
    mcall   SF_SYSTEM_GET, SSF_TIME_COUNT
    sub     eax, edi
    ; check if elapsed >= pause
    cmp     eax, esi
    jae     .timeout
    ; remaining time, wait for event
    mov     ebx, esi
    sub     ebx, eax
    mcall   SF_WAIT_EVENT_TIMEOUT
    ; check keyboard input, skip if space or enter
    mcall   SF_GET_KEY
    cmpe    ah, KEY_CODE_SPACE, .return
    cmpe    ah, KEY_CODE_ENTER, .return
    jmp     .wait_loop
  .timeout:
    mov eax, -1
  .return:
    ret
; -----------------------------
; Draws the number of lives in the snake's head
; -----------------------------
Draw_lives_in_head:
    cmp     [play_mode], LEVELS_MODE
    jne     .return
    test    [show_lives_style], 2
    jz      .return
    ; get head position in grid
    mov     eax, [snake_length_x2]
    movzx   eax, word[snake_dots + eax - 2]
    ; get index cell X and Y
    movzx   edx, al
    movzx   ecx, ah
    ; get offset half font
    mov     eax, [g_s]
    sub     eax, [configFont.width]
    shr     eax, 1
    mov     ebx, [g_s]
    sub     ebx, [configFont.height]
    shr     ebx, 1
    ; forming coordinate
    movzx   edx, word[COORD_SQUARE.x + edx*4 + 2]
    add     edx, eax
    shl     edx, 16
    movzx   ecx, word[COORD_SQUARE.y + ecx*4 + 2]
    add     ecx, ebx
    add     edx, ecx
    ; set font and color
    mov     esi, [configFont.mask]
    or      esi, [lives_in_head_number_color]
    mcall   SF_DRAW_NUMBER, 0x80010001, lives
  .return:
    ret
;;---Some_functions------------------------------------------------------------------------------------------------------------
