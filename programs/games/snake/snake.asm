;;===HEADER====================================================================================================================
use32
org 0x0
db 'MENUET01'
dd 0x01
dd start
dd i_end
dd d_end
dd stacktop
dd 0x0
dd cur_dir_path
;;---HEADER--------------------------------------------------------------------------------------------------------------------
include '../../KOSfuncs.inc'
include '../../proc32.inc'
include '../../macros.inc'
include '../../dll.inc'
include '../../develop/libraries/box_lib/box_lib.mac'
include '../../encoding.inc'
;;===Define_chapter============================================================================================================
WINDOW_MODE_WINDOWED        equ    0
WINDOW_MODE_FULLSCREEN      equ    1
WINDOW_STATE_MAXIMIZED      equ    1
WINDOW_STATE_ROLLED         equ    4

KEY_CODE_ESC                equ    0x01
KEY_CODE_SPACE              equ    0x39
KEY_CODE_ENTER              equ    0x1C
KEY_CODE_LEFT               equ    0x4B
KEY_CODE_DOWN               equ    0x50
KEY_CODE_UP                 equ    0x48
KEY_CODE_RIGHT              equ    0x4D

BUTTON_CLOSE                equ    1
BUTTON_PLAY                 equ    0xD0
BUTTON_EXIT                 equ    0xD1
BUTTON_MODE                 equ    0xD2
BUTTON_INC                  equ    0xD3
BUTTON_DEC                  equ    0xD4

GRID_WIDTH                  equ    28
GRID_HEIGHT                 equ    14

MIN_SQUARE_SIDE_LENGTH      equ    10
MIN_SQUARE_SIDE_LENGTH_FONT equ    20
FONT_SMALL                  equ    0
FONT_LARGE                  equ    1
DEFAULT_NAVIGATION_COLOR    equ    0xff7777
DEFAULT_LABEL_COLOR         equ    0xffffff
DEFAULT_LABEL_END_COLOR     equ    0xff9900
DEFAULT_NUMBER_END_COLOR    equ    0xffdd44
DEFAULT_HISCORE             equ    777
DEFAULT_BUTTON_TEXT_COLOR   equ    0x000000

SCORE_EAT                   equ    100

LEFT                        equ    0
DOWN                        equ    1
UP                          equ    2
RIGHT                       equ    3

CLASSIC_MODE                equ    0
LEVELS_MODE                 equ    1

CLASSIC_MODE_FIRST_LEVEL    equ    0
LEVELS_MODE_FIRST_LEVEL     equ    1

EAT_TO_END_LEVEL            equ    7
PAUSE_BETWEEN_LEVELS        equ    200
PAUSE_WHILE_DRAWING_SPLASH  equ    3

CHAMPION_NAME_LENGTH        equ    15
LAST_LEVEL_NUMBER           equ    36

START_LIVES                 equ    3

struct LEVEL
    field                   db    4*GRID_HEIGHT dup (?)
    snake_dots              db    6 dup (?)
    snake_direction         dd    ?
    snake_direction_next    dd    ?
    number_of_stones        dd    ?
    name                    dd    ?
ends
struct LABEL
    color                   dd    ?
    size                    dd    ?
    value                   dd    ?
ends
struct COORD_SQUARE
    x                       rd    GRID_WIDTH
    y                       rd    GRID_HEIGHT
ends
;;---Define_chapter------------------------------------------------------------------------------------------------------------

start:
    stdcall dll.Load,@IMPORT
    or   eax, eax
    jnz  Exit

align 4
    mov     edi, cur_dir_path
    mov     al, 0
    mov     ecx, 4096
    repne   scasb
    mov     dword[edi - 1], '.ini'

    invoke  ini.get_int, cur_dir_path, aPreferences, aSpeed, 80
    neg     eax
    add     [time_wait_limit], eax
    mov     ebx, [time_wait_limit]
    mov     [time_wait_limit_const], ebx
    sub     ebx, 4
    mov     eax, 200
    div     bl
    mov     byte[speed_up_counter], al
    mov     byte[speed_up_counter + 1], al
    invoke  ini.get_str, cur_dir_path, aPreferences, aTheme, aTheme_name, 31, aTheme_name
    invoke  ini.get_int, cur_dir_path, aPreferences, aSmart_reverse, 0
    mov     [smart_reverse], eax
    invoke  ini.get_int, cur_dir_path, aPreferences, aShow_lives_style, 2
    mov     [show_lives_style], eax
    invoke  ini.get_int, cur_dir_path, aPreferences, aDraw_level_name_in_title, 1
    mov     [draw_level_name_in_title], eax
    invoke  ini.get_str, cur_dir_path, aPreferences, aSeparating_symbol, separating_symbol, 3, default_separating_symbol

    invoke  ini.get_shortcut, cur_dir_path, aShortcuts, aMove_left, 0x23, 0
    mov     [shortcut_move_left], al
    invoke  ini.get_shortcut, cur_dir_path, aShortcuts, aMove_down, 0x24, 0
    mov     [shortcut_move_down], al
    invoke  ini.get_shortcut, cur_dir_path, aShortcuts, aMove_up, 0x25, 0
    mov     [shortcut_move_up], al
    invoke  ini.get_shortcut, cur_dir_path, aShortcuts, aMove_right, 0x26, 0
    mov     [shortcut_move_right], al
    invoke  ini.get_shortcut, cur_dir_path, aShortcuts, aReverse, 0x0F, 0
    mov     [shortcut_reverse], al
    invoke  ini.get_shortcut, cur_dir_path, aShortcuts, aIncrease, 0x16, 0
    mov     [shortcut_increase], al
    invoke  ini.get_shortcut, cur_dir_path, aShortcuts, aDecrease, 0x17, 0
    mov     [shortcut_decrease], al

    invoke  ini.get_int, cur_dir_path, aTheme_name, aDecorations, 3
    mov     [decorations], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, aBackground_color, 0x000000
    or      [background_color], eax
    or      [window_style_windowed], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, aDecorations_color, 0x232300
    or      [decorations_color], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, aSnake_color, 0x1111ff
    or      [snake_color], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, aSnake_head_color, 0x6B6Bff
    or      [snake_head_color], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, aLives_in_head_number_color, 0xff8800
    or      [lives_in_head_number_color], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, aSnake_picture_color, 0x4488ff
    or      [snake_picture_color], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, aVersion_picture_color, 0x55ff55
    or      [version_picture_color], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, aPause_picture_color, 0x11ff11
    or      [pause_picture_color], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, aGame_over_picture_color, 0xff1111
    or      [game_over_picture_color], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, aYou_win_picture_color, 0xffff11
    or      [you_win_picture_color], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, aEat_color, 0xffff11
    or      [eat_color], eax
    ; ini navigation color
    invoke  ini.get_color, cur_dir_path, aTheme_name, configColor.labelNavigation, DEFAULT_NAVIGATION_COLOR
    mov     [labelMenu.color], eax
    mov     [labelExit.color], eax
    mov     [labelStart.color], eax
    mov     [labelPause.color], eax
    mov     [labelApply.color], eax
    mov     [labelResume.color], eax
    ; ini color score
    invoke  ini.get_color, cur_dir_path, aTheme_name, configColor.labelScore, DEFAULT_LABEL_COLOR
    mov     [labelScore.color], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, configColor.numberScore, DEFAULT_LABEL_COLOR
    mov     [numberScore.color], eax
    ; ini color hiscore
    invoke  ini.get_color, cur_dir_path, aTheme_name, configColor.labelHiscore, DEFAULT_LABEL_COLOR
    mov     [labelHiscore.color], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, configColor.numberHiscore, DEFAULT_LABEL_COLOR
    mov     [numberHiscore.color], eax
    ; ini color champion
    invoke  ini.get_color, cur_dir_path, aTheme_name, configColor.labelChampion, DEFAULT_LABEL_COLOR
    mov     [labelChampion.color], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, configColor.labelChampionName, DEFAULT_LABEL_COLOR
    mov     [labelChampionName.color], eax
    ; ini color level
    invoke  ini.get_color, cur_dir_path, aTheme_name, configColor.labelLevel, DEFAULT_LABEL_COLOR
    mov     [labelLevel.color], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, configColor.numberLevel, DEFAULT_LABEL_COLOR
    mov     [numberLevel.color], eax
    ; ini color game over
    invoke  ini.get_color, cur_dir_path, aTheme_name, configColor.labelGameOver, DEFAULT_LABEL_END_COLOR
    mov     [labelCongratulations.color], eax
    mov     [labelEnterName.color], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, configColor.numberGameOver, DEFAULT_NUMBER_END_COLOR
    mov     [numberGameOver.color], eax
    mov     [edit1.text_color], eax
    ; ini color edit box
    invoke  ini.get_color, cur_dir_path, aTheme_name, aEdit_box_selection_color, 0x00aa00
    mov     [edit1.shift_color], eax
    mov     eax, [background_color]
    mov     [edit1.color], eax
    mov     [edit1.focus_border_color], eax
    mov     [edit1.blur_border_color], eax
    ;
    invoke  ini.get_color, cur_dir_path, aTheme_name, aButton_color, 0xDDDDDD
    or      [button_color], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, configColor.buttonText, DEFAULT_BUTTON_TEXT_COLOR
    mov     [labelButtonPlay.color], eax
    mov     [labelButtonExit.color], eax
    mov     [labelButtonClassic.color], eax
    mov     [labelButtonLevels.color], eax
    mov     [labelButtonInc.color], eax
    mov     [labelButtonDec.color], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, aStone_color, 0x5f8700
    or      [stone_color], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, aSplash_background_color, 0xAAAA00
    or      [splash_background_color], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, aSplash_level_number_color, 0x000000
    or      [splash_level_number_color], eax
    invoke  ini.get_color, cur_dir_path, aTheme_name, aSplash_level_string_color, 0x000000
    or      [splash_level_string_color], eax

    invoke  ini.get_int, cur_dir_path, aReserved, aSquare_side_length, 19
    mov     [square_side_length], eax
    invoke  ini.get_str, cur_dir_path, aReserved, config.championNameClassic, labelChampionNameClassic, CHAMPION_NAME_LENGTH+1, labelChampionNameClassic
    invoke  ini.get_str, cur_dir_path, aReserved, config.championNameLevels, labelChampionNameLevels, CHAMPION_NAME_LENGTH+1, labelChampionNameLevels
    invoke  ini.get_int, cur_dir_path, aReserved, config.hiscoreClassic, DEFAULT_HISCORE
    mov     [numberHiscoreClassic], eax
    invoke  ini.get_int, cur_dir_path, aReserved, config.hiscoreLevels, DEFAULT_HISCORE
    mov     [numberHiscoreLevels], eax
    ; load empty cursor (for "hiding" cursor while level_mode)
    mcall   SF_MOUSE_GET, SSF_LOAD_CURSOR, cursor_data, 2
    mov     [cursor_handle], eax

    mov     eax, WINDOW_MODE_WINDOWED
    call    Set_window_mode
    ; create empty window. Set_geometry will set all parameters
    mcall   SF_CREATE_WINDOW, 0, 0, [window_style_windowed]
    call    Set_geometry.by_hotkey
    mcall   SF_SET_CAPTION, 1, window_title

include 'first_menu.asm'            ; First menu body and functions
include 'level.asm'                 ; Level body and functions (game process)
include 'pause.asm'                 ; Pause body and functions
include 'game_over.asm'             ; Game_over body and functions
include 'functions.asm'
;;===Some_functions============================================================================================================

Save_do_smth_else_and_exit:
    ;;===Save_do_smth_else_and_exit============================================================================================
    ; delete cursor
    mcall   SF_MOUSE_GET, SSF_DEL_CURSOR, [cursor_handle]
    invoke  ini.set_int, cur_dir_path, aReserved, aSquare_side_length, [square_side_length]

    mov     edi, labelChampionNameClassic
    xor     al, al
    mov     ecx, CHAMPION_NAME_LENGTH+1
    cld
    repne   scasb
    neg     ecx
    add     ecx, CHAMPION_NAME_LENGTH
    invoke  ini.set_str, cur_dir_path, aReserved, config.championNameClassic, labelChampionNameClassic, ecx
    invoke  ini.set_int, cur_dir_path, aReserved, config.hiscoreClassic, [numberHiscoreClassic]

    mov     edi, labelChampionNameLevels
    xor     al, al
    mov     ecx, CHAMPION_NAME_LENGTH + 1
    cld
    repne   scasb
    neg     ecx
    add     ecx, CHAMPION_NAME_LENGTH
    invoke  ini.set_str, cur_dir_path, aReserved, config.championNameLevels, labelChampionNameLevels, ecx
    invoke  ini.set_int, cur_dir_path, aReserved, config.hiscoreLevels, [numberHiscoreLevels]
    ;;---Save_do_smth_else_and_exit--------------------------------------------------------------------------------------------

Exit:
    ;;===Exit==================================================================================================================
    mcall   SF_TERMINATE_PROCESS
    ;;---Exit------------------------------------------------------------------------------------------------------------------

Set_geometry:
    ;;===Set_geometry==========================================================================================================
    cmp     [resized_by_hotkey], 1
    je      .by_hotkey

    test    [proc_info.wnd_state], WINDOW_STATE_MAXIMIZED
    jnz     .by_hotkey

    mcall   SF_THREAD_INFO, proc_info, -1
    test    [proc_info.wnd_state], WINDOW_STATE_ROLLED
    jz      @f
    mov     eax, [proc_info.box.width]
    mov     [window_width], eax
    mov     eax, [proc_info.box.height]
    mov     [window_height], eax
    jmp     .quit
  @@:
    mov     eax, [proc_info.box.width]
    cmp     eax, [window_width]
    jne     @f
    mov     eax, [proc_info.box.height]
    cmp     eax, [window_height]
    jne     @f
    jmp     .quit
  @@:
    mov     eax, [proc_info.box.width]
    mov     [window_width], eax
    mov     eax, [proc_info.box.height]
    mov     [window_height], eax

  .by_mouse:; or any other kind of resizing. for example, double click on window title
    test    [proc_info.wnd_state], WINDOW_STATE_MAXIMIZED
    jnz     .by_hotkey
    cmp     [window_width], 250
    jnl     @f
    mov     [window_width], 250
  @@:
    cmp     [window_height], 150
    jnl     @f
    mov     [window_height], 150
  @@:
    mcall   SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
    mov     ecx, [window_height]
    sub     ecx, eax
    sub     ecx, 5
    sub     ecx, [g_e]
    sub     ecx, [g_e]
    sub     ecx, 25 + 30
    mov     esi, ecx

    mov     eax, [window_width]
    sub     eax, 5 + 5
    sub     eax, [g_e]
    sub     eax, [g_e]
    sub     eax, [g_s]
    sub     eax, [g_s]
    mov     edi, eax

    mov     dx, 0
    div     cx
    cmp     ax, 2
    jl      .fit_to_width

  .fit_to_height:
    mov     eax, esi
    mov     ebx, GRID_HEIGHT
    div     bl
    cmp     al, MIN_SQUARE_SIDE_LENGTH
    jnl     @f
    mov     al, MIN_SQUARE_SIDE_LENGTH
  @@:
    dec     al
    mov     byte[square_side_length], al
    jmp     .by_hotkey

  .fit_to_width:
    mov     eax, edi
    mov     ebx, GRID_WIDTH
    div     bl
    cmp     al, MIN_SQUARE_SIDE_LENGTH
    jnl     @f
    mov     al, MIN_SQUARE_SIDE_LENGTH
  @@:
    dec     al
    mov     byte[square_side_length], al
    jmp .by_hotkey

  .by_hotkey:
    mcall   SF_THREAD_INFO, proc_info, -1
    mov     [resized_by_hotkey], 0
    test    [proc_info.wnd_state], WINDOW_STATE_ROLLED
    jz      @f
    mov     eax, [proc_info.box.width]
    mov     [window_width], eax
    mov     eax, [proc_info.box.height]
    mov     [window_height], eax
    jmp     .quit
  @@:
    mov     eax, [square_side_length]
    ; space between squares
    inc     eax
    mov     [g_s], eax
    stdcall draw.setConfigFont, eax

    mov     eax, [g_s]
    shr     eax, 1
    mov     ebx, eax
    shr     ebx, 1
    add     eax, ebx
    mov     [g_e], eax

    mov     edx, GRID_WIDTH
    mov     eax, [g_s]
    mul     dx
    mov     [gw_mul_gs], eax

    mov     edx, GRID_HEIGHT
    mov     eax, [g_s]
    mul     dx
    mov     [gh_mul_gs], eax

    mov     eax, [gw_mul_gs]
    add     eax, [g_s]
    add     eax, [g_s]
    add     eax, [g_e]
    add     eax, [g_e]
    add     eax, 5*2                                   ; skin width
    mov     esi, eax
    test    [proc_info.wnd_state], WINDOW_STATE_MAXIMIZED
    jz      @f
    mov     eax, [proc_info.box.width]
  @@:
    mov     [window_width], eax

    sub     eax, [gw_mul_gs]
    sub     eax, 5*2
    shr     eax, 1
    mov     [gbxm1], eax

    ; get skin header height
    mcall   SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
    mov     ebx, eax
    add     eax, [gh_mul_gs]
    add     eax, [g_e]
    add     eax, 25
    add     eax, [g_e]
    add     eax, 30
    add     eax, 5                                      ; skin height (bottom part)
    mov     edi, eax
    test    [proc_info.wnd_state], WINDOW_STATE_MAXIMIZED
    jz      @f
    mov     eax, [proc_info.box.height]
  @@:
    mov     [window_height], eax

    sub     eax, [gh_mul_gs]
    sub     eax, ebx
    sub     eax, 5
    sub     eax, 5

    shr     eax, 1
    mov     [gbym1], eax
    stdcall draw.InitCoordSquare, [gbxm1], GRID_WIDTH, [g_s], COORD_SQUARE.x
    stdcall draw.InitCoordSquare, [gbym1], GRID_HEIGHT, [g_s], COORD_SQUARE.y

    mov     eax, [g_s]
    shl     eax, 16
    add     eax, [g_s]
    mov     [gs_shl16_gs], eax

    mov     eax, [gbxm1]
    shl     eax, 16
    add     eax, [gbxm1]
    mov     [gbxm1_shl16_gbxm1], eax

    mov     eax, [gbym1]
    shl     eax, 16
    add     eax, [gbym1]
    mov     [gbym1_shl16_gbym1], eax

    mov     eax, [gbxm1]
    add     eax, [gw_mul_gs]
    mov     [gbxm1_plus_gw_mul_gs], eax

    mov     eax, [gbym1]
    add     eax, [gh_mul_gs]
    mov     [gbym1_plus_gh_mul_gs], eax

    mcall   SF_STYLE_SETTINGS, SSF_GET_SCREEN_AREA
    mov     dx, ax
    shr     eax, 16
    sub     dx, ax
    cmp     dx, si                                    ; does window fit to work area width?
    jnl     @f
    dec     [square_side_length]
    jmp     Set_geometry.by_hotkey
  @@:
    mov     cx, bx
    shr     ebx, 16
    sub     cx, bx
    cmp     cx, di                                    ; does window fit to work area height?
    jnl     @f
    dec     [square_side_length]
    jmp     Set_geometry.by_hotkey
  @@:
    sub     dx, si
    shr     dx, 1
    mov     word[wp_x], dx
    sub     cx, word[window_height]
    shr     cx, 1
    mov     dx, cx
    shr     cx, 1
    add     cx, dx
    mov     word[wp_y], cx
    ; fixing the positions for the labels
    ; Y position
    mov     ebx, 13                                 ; margin from grid borders
    mov     eax, [gbym1]
    sub     eax, ebx
    mov     [posLabel.yTop], eax
    mov     eax, [gbym1_plus_gh_mul_gs]
    add     eax, ebx
    mov     [posLabel.yCenter], eax
    add     eax, ebx
    shr     ebx, 1
    add     eax, ebx
    mov     [posLabel.yBottom], eax
    ; X position
    mov     eax, [gbxm1]
    mov     [posLabel.xLeft], eax
    mov     eax, [gbxm1_plus_gw_mul_gs]
    mov     [posLabel.xRight], eax
    ; init size button
    mov     eax, [g_s]
    shl     eax, 2
    sub     eax, 2
    mov     [button_width_short], eax
    mov     eax, [g_s]
    shl     eax, 3
    add     eax, [g_s]
    sub     eax, 2
    mov     [button_width_long], eax
    mov     eax, [g_s]
    sub     eax, 2
    mov     [button_height], eax
    ; init position button
    mov     ebx, 17
    movzx   eax, word[COORD_SQUARE.x + ebx*4 - 2]
    mov     [button_x_left], eax
    add     ebx, 5
    movzx   eax, word[COORD_SQUARE.x + ebx*4 - 2]
    mov     [button_x_right], eax
    mov     ebx, 9
    movzx   eax, word[COORD_SQUARE.y + ebx*4 - 2]
    mov     [button_y_top], eax
    add     ebx, 2
    movzx   eax, word[COORD_SQUARE.y + ebx*4 - 2]
    mov     [button_y_middle], eax
    add     ebx, 2
    movzx   eax, word[COORD_SQUARE.y + ebx*4 - 2]
    mov     [button_y_bottom], eax

  .done:
    mcall   SF_CHANGE_WINDOW, [wp_x], [wp_y], [window_width], [window_height]
  .quit:
    ret
    ;;---Set_geometry------------------------------------------------------------------------------------------------------


Increase_geometry:
    ;;===Increase_geometry=================================================================================================
    inc     [square_side_length]
    mov     [resized_by_hotkey], 1
    ret
    ;;---Increase_geometry-------------------------------------------------------------------------------------------------

Decrease_geometry:
    ;;===Decrease_geometry=================================================================================================
    cmp     [square_side_length], MIN_SQUARE_SIDE_LENGTH
    je      @f
    dec     [square_side_length]
    mov     [resized_by_hotkey], 1
  @@:
    ret
    ;;---Decrease_geometry-------------------------------------------------------------------------------------------------

Draw_decorations:
    ;;===Draw_decorations==================================================================================================
    mov     al, byte[decorations]
    dec     al
    jz      grid_lines
    dec     al
    jz      grid_lines_with_ends
    dec     al
    jz      grid_lines_with_corners
    dec     al
    jz      grid_dots
    dec     al
    jz      borders_lines
    dec     al
    jz      borders_lines_with_corners
    dec     al
    jz      borders_dots
    dec     al
    jz      corners_dots
    dec     al
    jz      corners_inner
    dec     al
    jz      corners_outer
    dec     al
    jz      corners_crosses
    ret

  grid_lines:
    mov     eax, 38
    mov     ebx, [gbxm1_shl16_gbxm1]
    mov     ecx, [gbym1_shl16_gbym1]
    add     ecx, [gh_mul_gs]
    mov     edx, [decorations_color]
    mov     esi, GRID_WIDTH
    add     esi, 1
  @@:
    mcall
    add     ebx, [gs_shl16_gs]
    dec     esi
    jnz     @b

    mov     ebx, [gbxm1_shl16_gbxm1]
    add     ebx, [gw_mul_gs]
    mov     ecx, [gbym1_shl16_gbym1]
    mov     esi, GRID_HEIGHT
    add     esi, 1
  @@:
    mcall
    add     ecx, [gs_shl16_gs]
    dec     esi
    jnz     @b
    ret

  grid_lines_with_ends:
    mov     eax, 38
    mov     ebx, [gbxm1_shl16_gbxm1]
    mov     ecx, [gbym1]
    sub     ecx, [g_e]
    shl     ecx, 16
    add     ecx, [gbym1_plus_gh_mul_gs]
    add     ecx, [g_e]
    mov     edx, [decorations_color]
    mov     esi, GRID_WIDTH
    add     esi, 1
  @@:
    mcall
    add     ebx, [gs_shl16_gs]
    dec     esi
    jnz     @b

    mov     ebx, [gbxm1]
    sub     ebx, [g_e]
    shl     ebx, 16
    add     ebx, [gbxm1_plus_gw_mul_gs]
    add     ebx, [g_e]
    mov     ecx, [gbym1_shl16_gbym1]
    mov     esi, GRID_HEIGHT
    add     esi, 1

  @@:
    mcall
    add     ecx, [gs_shl16_gs]
    dec     esi
    jnz @b
    ret

  grid_lines_with_corners:
    call    grid_lines
    call    corners_outer
    ret

  grid_dots:
    mov     eax, 1
    mov     ebx, [gbxm1]
    mov     ecx, [gbym1]
    mov     edx, [decorations_color]
  @@:
    mcall
    add     ebx, [g_s]
    cmp     ebx, [gbxm1_plus_gw_mul_gs]
    jng     @b
    add     ecx, [g_s]
    cmp     ecx, [gbym1_plus_gh_mul_gs]
    jg      @f
    mov     ebx, [gbxm1]
    jmp     @b
  @@:
    ret

  borders_lines:
    mov     eax, 38
    mov     ebx, [gbxm1_shl16_gbxm1]
    mov     ecx, [gbym1_shl16_gbym1]
    add     ecx, [gh_mul_gs]
    mov     edx, [decorations_color]
    mcall

    mov     ebx, [gbxm1_plus_gw_mul_gs]
    shl     ebx, 16
    add     ebx, [gbxm1_plus_gw_mul_gs]
    mcall

    mov     ebx, [gbxm1_shl16_gbxm1]
    add     ebx, [gw_mul_gs]
    mov     ecx, [gbym1_shl16_gbym1]
    mcall

    mov     ecx, [gbym1_plus_gh_mul_gs]
    shl     ecx, 16
    add     ecx, [gbym1_plus_gh_mul_gs]
    mcall
    ret

  borders_lines_with_corners:
    call    borders_lines
    call    corners_outer
    ret

  borders_dots:
    mov     eax, 1
    mov     ebx, [gbxm1]
    mov     ecx, [gbym1]
    mov     edx, [decorations_color]
  @@:
    mcall
    add     ebx, [g_s]
    cmp     ebx, [gbxm1_plus_gw_mul_gs]
    jng     @b

    mov     ebx, [gbxm1]
    mov     ecx, [gbym1_plus_gh_mul_gs]
  @@:
    mcall
    add     ebx, [g_s]
    cmp     ebx, [gbxm1_plus_gw_mul_gs]
    jng     @b

    mov     ebx, [gbxm1]
    mov     ecx, [gbym1]
  @@:
    mcall
    add     ecx, [g_s]
    cmp     ecx, [gbym1_plus_gh_mul_gs]
    jng     @b

    mov     ebx, [gbxm1_plus_gw_mul_gs]
    mov     ecx, [gbym1]
  @@:
    mcall
    add     ecx, [g_s]
    cmp     ecx, [gbym1_plus_gh_mul_gs]
    jng     @b
    ret

  corners_dots:
    mov     eax, 13
    mov     ebx, [gbxm1]
    dec     ebx
    shl     ebx, 16
    add     ebx, 2
    mov     ecx, [gbym1]
    dec     ecx
    shl     ecx, 16
    add     ecx, 2
    mov     edx, [decorations_color]
    mcall

    mov     ebx, [gbxm1_plus_gw_mul_gs]
    shl     ebx, 16
    add     ebx, 2
    mcall

    mov     ebx, [gbxm1]
    dec     ebx
    shl     ebx, 16
    add     ebx, 2
    mov     ecx, [gbym1_plus_gh_mul_gs]
    shl     ecx, 16
    add     ecx, 2
    mcall

    mov     ebx, [gbxm1_plus_gw_mul_gs]
    shl     ebx, 16
    add     ebx, 2
    mcall
    ret

  corners_inner:
    mov     eax, 38
    mov     ebx, [gbxm1_shl16_gbxm1]
    add     ebx, [g_e]
    mov     ecx, [gbym1_shl16_gbym1]
    mov     edx, [decorations_color]
    mcall

    mov     ecx, [gbym1_plus_gh_mul_gs]
    shl     ecx, 16
    add     ecx, [gbym1_plus_gh_mul_gs]
    mcall

    mov     ebx, [gbxm1_plus_gw_mul_gs]
    sub     ebx, [g_e]
    shl     ebx, 16
    add     ebx, [gbxm1_plus_gw_mul_gs]
    mcall

    mov     ecx, [gbym1_shl16_gbym1]
    mcall

    mov     ebx, [gbxm1_shl16_gbxm1]
    mov     ecx, [gbym1_shl16_gbym1]
    add     ecx, [g_e]
    mcall

    mov     ebx, [gbxm1_plus_gw_mul_gs]
    shl     ebx, 16
    add     ebx, [gbxm1_plus_gw_mul_gs]
    mcall

    mov     ecx, [gbym1_plus_gh_mul_gs]
    sub     ecx, [g_e]
    shl     ecx, 16
    add     ecx, [gbym1_plus_gh_mul_gs]
    mcall

    mov     ebx, [gbxm1_shl16_gbxm1]
    mcall
    ret

  corners_outer:
    mov     eax, 38
    mov     ebx, [gbxm1_shl16_gbxm1]
    sub     ebx, [g_e]
    mov     ecx, [gbym1_shl16_gbym1]
    mov     edx, [decorations_color]
    mcall

    mov     ecx, [gbym1_plus_gh_mul_gs]
    shl     ecx, 16
    add     ecx, [gbym1_plus_gh_mul_gs]
    mcall

    mov     ebx, [gbxm1_plus_gw_mul_gs]
    shl     ebx, 16
    add     ebx, [gbxm1_plus_gw_mul_gs]
    add     ebx, [g_e]
    mcall

    mov     ecx, [gbym1_shl16_gbym1]
    mcall

    mov     ebx, [gbxm1_shl16_gbxm1]
    mov     ecx, [gbym1_shl16_gbym1]
    sub     ecx, [g_e]
    mcall

    mov     ebx, [gbxm1_plus_gw_mul_gs]
    shl     ebx, 16
    add     ebx, [gbxm1_plus_gw_mul_gs]
    mcall

    mov     ecx, [gbym1_plus_gh_mul_gs]
    shl     ecx, 16
    add     ecx, [gbym1_plus_gh_mul_gs]
    add     ecx, [g_e]
    mcall

    mov     ebx, [gbxm1_shl16_gbxm1]
    mcall
    ret

  corners_crosses:
    call    corners_inner
    call    corners_outer
    ret
    ;;---Draw_decorations--------------------------------------------------------------------------------------------------

Draw_on_map:
    ;;===Draw_on_map=======================================================================================================
    ;;  in  :
    ;;           al =   x coord
    ;;           ah =   y coord
    ;;           cl =   value to draw
    ;;
    and     eax, 0x0000ffff
    xor     bx, bx
    mov     bl, al
    shr     ax, 8
    mov     dx, GRID_WIDTH
    mul     dx
    add     ax, bx
    mov     edi, field_map
    add     edi, eax
    mov     [edi], cl
    ret
    ;;---Draw_on_map-------------------------------------------------------------------------------------------------------

Get_from_map:
    ;;===Get_from_map======================================================================================================
    ;;  in  :
    ;;           al =   x coord
    ;;           ah =   y coord
    ;;  out :
    ;;           bl =   value on map
    ;;
    push    eax
    and     eax, 0x0000ffff
    xor     bx, bx
    mov     bl, al
    shr     ax, 8
    mov     dx, GRID_WIDTH
    mul     dx
    add     ax, bx
    mov     edi, field_map
    add     edi, eax
    mov     bl, [edi]
    pop     eax
    ret
    ;;---Get_from_map-----------------------------------------------------------------------------------------------------------

Load_level:
    ;;===Load_level=============================================================================================================
    ;;  in  :
    ;;          cur_level_number    =   level number to load
    ;;
    mov     eax, [numberLevel.value]
    mov     edx, stage_00
  @@:
    test    al, al
    jz      @f
    add     edx, stage_01 - stage_00
    dec     al
    jmp     @b
  @@:
    mov     esi, window_title_with_lives
    mov     edi, window_title + 5

  .lives_in_title:
    cmp     [play_mode], LEVELS_MODE
    jne     .level_name_in_title
    test    [show_lives_style], 1
    jz      .level_name_in_title
    mov     [edi], byte ' '
    mov     al, byte[separating_symbol]
    mov     [edi+1], byte al
    mov     [edi+2], byte ' '
    add     edi, 3
    mov     eax, [lives]
    add     al, 0x30
    mov     [window_title_with_lives], al
    mov     ecx, 10
    rep     movsb
    dec     edi

  .level_name_in_title:
    cmp     [draw_level_name_in_title], 1
    jne     @f
    mov     [edi], byte ' '
    mov     al, byte[separating_symbol]
    mov     [edi+1], byte al
    mov     [edi+2], byte ' '
    add     edi, 3
    mov     esi, edx
    add     esi, LEVEL.name
    mov     esi, [esi]
    mov     ecx, 16
    rep     movsd

  @@:
    mcall   SF_SET_CAPTION, 1, window_title
    mov     [cur_level], edx
    mov     esi, edx
    add     esi, LEVEL.field
    mov     edi, field_map
    mov     ecx, 2
    mov     ah, GRID_HEIGHT
  .begin:
    mov     ebx, 7
    mov     al, GRID_WIDTH
  .loop:
    bt      [esi], ebx
    jnc     @f
    mov     byte[edi], cl
    jmp     .skip
  @@:
    mov     byte[edi], 0
  .skip:
    dec     ebx
    jns     @f
    mov     ebx, 7
    inc     esi
  @@:
    inc     edi
    dec     al
    jnz     .loop
    inc     esi
    dec     ah
    jnz     .begin

    mov     esi, edx
    add     esi, LEVEL.snake_dots
    mov     edi, snake_dots
    mov     ecx, 3
    rep     movsw

    mov     esi, edx
    add     esi, LEVEL.snake_direction
    mov     eax, [esi]
    mov     [snake_direction], eax

    mov     esi, edx
    add     esi, LEVEL.snake_direction_next
    mov     eax, [esi]
    mov     [snake_direction_next], eax

    mov     esi, edx
    add     esi, LEVEL.number_of_stones
    mov     eax, [esi]
    mov     [number_of_free_dots], GRID_WIDTH*GRID_HEIGHT-3
    sub     [number_of_free_dots], eax

    mov     ax, word[snake_dots]
    mov     cl, 1
    call    Draw_on_map
    mov     ax, word[snake_dots+2]
    mov     cl, 1
    call    Draw_on_map
    mov     ax, word[snake_dots+4]
    mov     cl, 1
    call    Draw_on_map

    mov     [action], 0
    mov     [snake_length_x2], 6
    ret
    ;;---Load_level-------------------------------------------------------------------------------------------------------------

Hide_cursor:
    ;;===Hide_cursor===========================================================================================================
    mcall   SF_MOUSE_GET, SSF_SET_CURSOR, [cursor_handle]
    ret
    ;;---Show_cursor-----------------------------------------------------------------------------------------------------------

Show_cursor:
    ;;===Hide_cursor===========================================================================================================
    mcall   SF_MOUSE_GET, SSF_SET_CURSOR, 0
    ret
    ;;---Show_cursor-----------------------------------------------------------------------------------------------------------

Set_window_mode:
    ;;===Set_window_mode=======================================================================================================
    test    al, al
    jnz     .fullscreen
    mov     eax, [window_style_windowed]
    jmp     .quit
  .fullscreen:
    mov     eax, [window_style_fullscreen]
  .quit:
    mov     [window_style], eax
    ret

;;---Some_functions------------------------------------------------------------------------------------------------------------


Reverse_snake:
    ;;===Reverse_snake=========================================================================================================
    cmp     [snake_direction], LEFT
    jne     @f
    mov     [snake_direction_next], RIGHT
    jmp     .quit
  @@:
    cmp     [snake_direction], RIGHT
    jne     @f
    mov     [snake_direction_next], LEFT
    jmp     .quit
  @@:
    cmp     [snake_direction], UP
    jne     @f
    mov     [snake_direction_next], DOWN
    jmp     .quit
  @@:
    cmp     [snake_direction], DOWN
    jne     .quit
    mov     [snake_direction_next], UP
  .quit:
    ret
    ;;---Reverse_snake---------------------------------------------------------------------------------------------------------

;;===Variables==================================================================================================================

window_title                db    'Snake',64+15 dup (0)
window_title_with_lives     db    '_ live(s)',0
default_separating_symbol   db    '|',0
window_style_windowed       dd    0x33000000              ; scalable skinned window
window_style_fullscreen     dd    0x00000000
time_before_waiting         dd    0x0
time_to_wait                dd    0x0
time_wait_limit             dd    101
time_wait_limit_const       dd    0x0

play_mode                   dd    0x0
playModeLabelButton         dd    labelButtonClassic
lives                       dd    START_LIVES
acceleration_mask           dd    0x0

resized_by_hotkey           dd    0x0

is_new_record               dd    0

action                      dd    0

picture_first_menu_snake    db    0xf4,0x99,0x2f,\
                                  0x86,0xa5,0x49,\
                                  0xf5,0xa5,0x8f,\
                                  0x15,0xbd,0x48,\
                                  0xf4,0xa5,0x2f

picture_first_menu_version  db    0xf1,0xe0,\
                                  0x90,0x20,\
                                  0x90,0x20,\
                                  0x90,0x20,\
                                  0xf4,0x20

picture_pause               db    11100011b,00100101b,11101111b,\
                                  10010100b,10100101b,00001000b,\
                                  10010100b,10100101b,00001000b,\
                                  11100111b,10100101b,11101111b,\
                                  10000100b,10100100b,00101000b,\
                                  10000100b,10011001b,11101111b

picture_game_over           db    00110000b,00000000b,00000000b,00000000b,\
                                  01001001b,11001000b,10111100b,00000000b,\
                                  10000010b,00101101b,10100000b,00000000b,\
                                  10011010b,00101010b,10111000b,00000000b,\
                                  10001011b,11101000b,10100000b,00000000b,\
                                  01110010b,00101000b,10111100b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000111b,00000000b,00000000b,00000000b,\
                                  00001000b,10100010b,11110111b,10000000b,\
                                  00001000b,10100010b,10000100b,01000000b,\
                                  00001000b,10100010b,11100100b,01000000b,\
                                  00001000b,10010100b,10000111b,10000000b,\
                                  00000111b,00001000b,11110100b,01000000b

picture_you_win             db    01000100b,01000011b,10001000b,10000000b,\
                                  01000100b,01000100b,01001000b,10000000b,\
                                  01000100b,01000100b,00001000b,10000000b,\
                                  01111100b,01000100b,11001111b,10000000b,\
                                  01000100b,01000100b,01001000b,10000000b,\
                                  01000100b,01000011b,10001000b,10000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  11110011b,10011100b,11110011b,11000000b,\
                                  10000100b,00100010b,10001010b,00000000b,\
                                  11110100b,00100010b,10001011b,10000000b,\
                                  00010100b,00100010b,11110010b,00000000b,\
                                  11110011b,10011100b,10001011b,11000000b

picture_level               db    10000111b,10100101b,11101000b,\
                                  10000100b,00100101b,00001000b,\
                                  10000111b,00100101b,11001000b,\
                                  10000100b,00101001b,00001000b,\
                                  11110111b,10110001b,11101111b

digits_font                 db    0xf0,0x90,0x90,0x90,0xf0,\
                                  0x20,0x60,0x20,0x20,0x20,\
                                  0xf0,0x10,0xf0,0x80,0xf0,\
                                  0xf0,0x10,0x70,0x10,0xf0,\
                                  0x90,0x90,0xf0,0x10,0x10,\
                                  0xf0,0x80,0xf0,0x10,0xf0,\
                                  0xf0,0x80,0xf0,0x90,0xf0,\
                                  0xf0,0x10,0x10,0x10,0x10,\
                                  0xf0,0x90,0xf0,0x90,0xf0,\
                                  0xf0,0x90,0xf0,0x10,0xf0

stage_00:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    3,3, 4,3, 5,3
.snake_direction            dd    RIGHT
.snake_direction_next       dd    RIGHT
.number_of_stones           dd    0
.name                       dd    stage_00_name

stage_01:
.field                      db    11111000b,00000000b,00000001b,11110000b,\
                                  10000000b,00000000b,00000000b,00010000b,\
                                  10000000b,00000000b,00000000b,00010000b,\
                                  10000000b,00000000b,00000000b,00010000b,\
                                  10000000b,00000000b,00000000b,00010000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  10000000b,00000000b,00000000b,00010000b,\
                                  10000000b,00000000b,00000000b,00010000b,\
                                  10000000b,00000000b,00000000b,00010000b,\
                                  10000000b,00000000b,00000000b,00010000b,\
                                  11111000b,00000000b,00000001b,11110000b

.snake_dots                 db    3,3, 4,3, 5,3
.snake_direction            dd    RIGHT
.snake_direction_next       dd    RIGHT
.number_of_stones           dd    36
.name                       dd    stage_01_name

stage_02:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00011111b,11000000b,00111111b,10000000b,\
                                  00010000b,00000000b,00000000b,10000000b,\
                                  00010000b,00000000b,00000000b,10000000b,\
                                  00010000b,00000000b,00000000b,10000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00010000b,00000000b,00000000b,10000000b,\
                                  00010000b,00000000b,00000000b,10000000b,\
                                  00010000b,00000000b,00000000b,10000000b,\
                                  00011111b,11000000b,00111111b,10000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    7,5, 8,5, 9,5
.snake_direction            dd    RIGHT
.snake_direction_next       dd    RIGHT
.number_of_stones           dd    40
.name                       dd    stage_02_name

stage_03:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00001001b,00000000b,00000000b,\
                                  00000000b,00001001b,00000000b,00000000b,\
                                  00000000b,00001001b,00000000b,00000000b,\
                                  00001111b,11111001b,11111111b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00001111b,11111001b,11111111b,00000000b,\
                                  00000000b,00001001b,00000000b,00000000b,\
                                  00000000b,00001001b,00000000b,00000000b,\
                                  00000000b,00001001b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    23,0, 22,0, 21,0
.snake_direction            dd    LEFT
.snake_direction_next       dd    LEFT
.number_of_stones           dd    48
.name                       dd    stage_03_name

stage_04:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00100000b,01000000b,00000000b,\
                                  00000010b,00100000b,01000100b,00000000b,\
                                  00000010b,00000000b,00000100b,00000000b,\
                                  00010000b,00100000b,01000000b,10000000b,\
                                  00000010b,00100000b,01000100b,00000000b,\
                                  00010010b,00000000b,00000100b,10000000b,\
                                  00010010b,00000000b,00000100b,10000000b,\
                                  00000010b,00100000b,01000000b,00000000b,\
                                  00010000b,00100000b,01000000b,10000000b,\
                                  00000010b,00000000b,00000100b,00000000b,\
                                  00000010b,00100000b,01000100b,00000000b,\
                                  00000000b,00100000b,01000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    19,6, 19,7, 19,8
.snake_napravlenie          dd    DOWN
.snake_napravlenie_next     dd    DOWN
.number_of_stones           dd    39
.name                       dd    stage_04_name

stage_05:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000001b,11111111b,11111000b,00000000b,\
                                  00000001b,11111111b,11111000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000111b,11111111b,11111110b,00000000b,\
                                  00000111b,11111111b,11111110b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00111111b,11111111b,11111111b,11000000b,\
                                  00111111b,11111111b,11111111b,11000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    0,0, 0,1, 1,1
.snake_direction            dd    RIGHT
.snake_direction_next       dd    RIGHT
.number_of_stones           dd    112
.name                       dd    stage_05_name

stage_06:
.field                      db    00000001b,10000000b,00000000b,00000000b,\
                                  00000001b,11111111b,11111000b,00000000b,\
                                  00000001b,11111111b,11111000b,00000000b,\
                                  00000000b,00000000b,00011000b,00000000b,\
                                  00000000b,00000000b,00011000b,00000000b,\
                                  00011111b,11111111b,11111000b,00000000b,\
                                  00011111b,11111111b,11111000b,00000000b,\
                                  00011000b,00000000b,00000000b,00000000b,\
                                  00011000b,00000000b,00000000b,00000000b,\
                                  00011111b,11111111b,11111111b,11100000b,\
                                  00011111b,11111111b,11111111b,11100000b,\
                                  00000000b,00000000b,00000000b,01100000b,\
                                  00000000b,00000000b,00000000b,01100000b,\
                                  00000000b,00000000b,00000000b,01100000b

.snake_dots                 db    0,0, 0,1, 1,1
.snake_direction            dd    RIGHT
.snake_direction_next       dd    RIGHT
.number_of_stones           dd    128
.name                       dd    stage_06_name

stage_07:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000011b,11111111b,11111100b,00000000b,\
                                  00000000b,00000000b,00000100b,00000000b,\
                                  00000011b,11111111b,11110100b,00000000b,\
                                  00000010b,00000000b,00010100b,00000000b,\
                                  00000010b,11111111b,11010100b,00000000b,\
                                  00000010b,00000000b,00010100b,00000000b,\
                                  00000010b,11111111b,11110100b,00000000b,\
                                  00000010b,00000000b,00000100b,00000000b,\
                                  00000011b,11111111b,11111100b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    8,1, 9,1, 10,1
.snake_direction            dd    RIGHT
.snake_direction_next       dd    RIGHT
.number_of_stones           dd    83
.name                       dd    stage_07_name

stage_08:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00010000b,00000000b,\
                                  00001001b,00000001b,00000000b,00000000b,\
                                  00000001b,01001001b,00000101b,00000000b,\
                                  00000000b,01000000b,00000100b,00000000b,\
                                  00001111b,00000000b,11100000b,00000000b,\
                                  00000000b,00000000b,00001000b,10000000b,\
                                  00000111b,00100000b,10000010b,10000000b,\
                                  00010000b,00000000b,00000010b,00000000b,\
                                  00010000b,11000000b,01110010b,00000000b,\
                                  00010010b,00000000b,00000010b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    0,0, 1,0, 2,0
.snake_direction            dd    RIGHT
.snake_direction_next       dd    RIGHT
.number_of_stones           dd    40
.name                       dd    stage_08_name

stage_09:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00111101b,00100110b,01001011b,11000000b,\
                                  00100001b,10101001b,01010010b,00000000b,\
                                  00111101b,01100001b,01100011b,10000000b,\
                                  00000101b,01100111b,01010010b,00000000b,\
                                  00111101b,00100001b,01001011b,11000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    12,6, 12,7, 12,8
.snake_direction            dd    DOWN
.snake_direction_next       dd    DOWN
.number_of_stones           dd    59
.name                       dd    stage_09_name

stage_10:
.field                      db    11101110b,11101110b,11101110b,11100000b,\
                                  11101110b,11101110b,11101110b,11100000b,\
                                  11101110b,11101110b,11101110b,11100000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  11101110b,11101110b,11101110b,11100000b,\
                                  11101110b,11101110b,11101110b,11100000b,\
                                  11101110b,11101110b,11101110b,11100000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  11101110b,11101110b,11101110b,11100000b,\
                                  11101110b,11101110b,11101110b,11100000b,\
                                  11101110b,11101110b,11101110b,11100000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  11101110b,11101110b,11101110b,11100000b,\
                                  11101110b,11101110b,11101110b,11100000b

.snake_dots                 db    3,2, 3,3, 4,3
.snake_direction            dd    RIGHT
.snake_direction_next       dd    RIGHT
.number_of_stones           dd    231
.name                       dd    stage_10_name

stage_11:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000111b,00000111b,00000111b,00000000b,\
                                  00001101b,10001101b,10001101b,10000000b,\
                                  00011000b,11011000b,11011000b,11000000b,\
                                  00000000b,01000000b,01000000b,01000000b,\
                                  00011000b,11011000b,11011000b,11000000b,\
                                  00001101b,10001101b,10001101b,10000000b,\
                                  00000111b,00000111b,00000111b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    3,12, 4,12, 5,12
.snake_direction            dd    RIGHT
.snake_direction_next       dd    RIGHT
.number_of_stones           dd    69
.name                       dd    stage_11_name

stage_12:
.field                      db    00000000b,00011000b,00001110b,00000000b,\
                                  01101110b,00010000b,00001010b,01010000b,\
                                  01001011b,11011001b,11000000b,01110000b,\
                                  01100001b,01000001b,01000000b,00000000b,\
                                  00000000b,00000000b,00000011b,10000000b,\
                                  00000000b,00000000b,00000010b,10000000b,\
                                  01010011b,00001100b,10100110b,00110000b,\
                                  01110010b,00001000b,11100100b,00010000b,\
                                  00000011b,00001100b,00000110b,00110000b,\
                                  00000000b,11100000b,00000000b,00000000b,\
                                  00010100b,10100000b,00000110b,00000000b,\
                                  11011100b,00000110b,10100100b,00000000b,\
                                  01000011b,10000010b,11100110b,10100000b,\
                                  11000010b,10000110b,00000000b,11100000b

.snake_dots                 db    27,0, 26,0, 25,0
.snake_direction            dd    LEFT
.snake_direction_next       dd    LEFT
.number_of_stones           dd    110
.name                       dd    stage_12_name

stage_13:
.field                      db    00111000b,00100000b,00000000b,00000000b,\
                                  01111100b,11110011b,11000011b,10000000b,\
                                  11111100b,01110011b,10000001b,11000000b,\
                                  11110000b,00000011b,11000000b,00000000b,\
                                  00000000b,00000010b,00000000b,00000000b,\
                                  00000000b,00000010b,00000001b,00000000b,\
                                  00011110b,00000111b,00000111b,00000000b,\
                                  00000111b,10001111b,11111110b,00000000b,\
                                  00000011b,11111111b,11111100b,00000000b,\
                                  00110001b,11111111b,11111001b,01100000b,\
                                  00001110b,11101011b,00100111b,10000000b,\
                                  01111000b,10000011b,10000010b,00000000b,\
                                  00000011b,11100110b,00011010b,11000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    0,5, 0,6, 0,7
.snake_direction            dd    DOWN
.snake_direction_next       dd    DOWN
.number_of_stones           dd    141
.name                       dd    stage_13_name

stage_14:
.field                      db    00000110b,00000000b,00000000b,00000000b,\
                                  00001000b,00000000b,00011000b,00000000b,\
                                  00010000b,00000000b,00000100b,00000000b,\
                                  00100001b,10000000b,11000010b,00000000b,\
                                  01000010b,01000001b,00100001b,00000000b,\
                                  10000100b,00000010b,00010000b,10000000b,\
                                  10001000b,00000100b,00001000b,01000000b,\
                                  10010000b,00001000b,00000100b,01000000b,\
                                  01001000b,00010000b,00001000b,10000000b,\
                                  00100100b,00100000b,00010001b,00000000b,\
                                  00010010b,01000001b,00100010b,00000000b,\
                                  00001001b,10000000b,11000100b,00000000b,\
                                  00000100b,00000000b,00001000b,00000000b,\
                                  00000000b,00000000b,00110000b,00000000b

.snake_dots                 db    8,0, 9,0, 10,0
.snake_direction            dd    RIGHT
.snake_direction_next       dd    RIGHT
.number_of_stones           dd    60
.name                       dd    stage_14_name

stage_15:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,01110000b,00110000b,00000000b,\
                                  00000000b,10000000b,00001010b,00000000b,\
                                  00100001b,00000000b,00110010b,00000000b,\
                                  00010001b,00111111b,10011100b,00000000b,\
                                  00001001b,00100000b,11000000b,00000000b,\
                                  00010000b,00000010b,01000000b,00000000b,\
                                  00100000b,00000010b,00100001b,10000000b,\
                                  00010000b,00000010b,00110010b,01000000b,\
                                  00001000b,01000100b,00011100b,01000000b,\
                                  00010000b,00111000b,00010000b,01000000b,\
                                  00100000b,00000000b,00010000b,01000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    13,3, 13,2, 14,2
.snake_direction            dd    RIGHT
.snake_direction_next       dd    RIGHT
.number_of_stones           dd    60
.name                       dd    stage_15_name

stage_16:
.field                      db    00000000b,10000010b,00000000b,00000000b,\
                                  00000000b,01001010b,10000010b,10100000b,\
                                  01111111b,11100111b,00000001b,11000000b,\
                                  00000000b,01000010b,00000000b,10000000b,\
                                  00000100b,10000000b,01000000b,00000001b,\
                                  00000100b,00100000b,10000000b,00000010b,\
                                  11000100b,00010001b,11111000b,00000111b,\
                                  00010101b,00001000b,10000000b,00000010b,\
                                  00001110b,00010000b,01000000b,00000001b,\
                                  00000100b,00100010b,00000000b,10000000b,\
                                  00000000b,00000010b,00000000b,01000000b,\
                                  00111111b,11111111b,11111111b,11100000b,\
                                  00000000b,00000010b,00000000b,01000000b,\
                                  00000000b,00000010b,00000000b,10000000b

.snake_dots                 db    11,7, 10,7, 9,7
.snake_direction            dd    LEFT
.snake_direction_next       dd    LEFT
.number_of_stones           dd    96
.name                       dd    stage_16_name

stage_17:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000111b,10000001b,11100000b,00000000b,\
                                  00001000b,01000010b,00010000b,00000000b,\
                                  00010001b,00100100b,01001000b,00000000b,\
                                  00010001b,00000000b,01001000b,00000000b,\
                                  00001000b,01000010b,00010000b,00000000b,\
                                  00000111b,10000001b,11100000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,01000000b,00000000b,00000000b,\
                                  00000000b,00111110b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    11,7, 11,8, 12,8
.snake_direction            dd    RIGHT
.snake_direction_next       dd    RIGHT
.number_of_stones           dd    40
.name                       dd    stage_17_name

stage_18:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  01000100b,01000100b,01000100b,01000000b,\
                                  00101010b,10101010b,10101010b,10100000b,\
                                  00010001b,00010001b,00010001b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  01000100b,01000100b,01000100b,01000000b,\
                                  00101010b,10101010b,10101010b,10100000b,\
                                  00010001b,00010001b,00010001b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  01000100b,01000100b,01000100b,01000000b,\
                                  00101010b,10101010b,10101010b,10100000b,\
                                  00010001b,00010001b,00010001b,00000000b

.snake_dots                 db    2,5, 3,5, 4,5
.snake_direction            dd    RIGHT
.snake_direction_next       dd    RIGHT
.number_of_stones           dd    78
.name                       dd    stage_18_name

stage_19:
.field                      db    01000010b,00100100b,10000101b,00000000b,\
                                  00010000b,00010000b,00100000b,10000000b,\
                                  00111001b,00000010b,00010000b,00100000b,\
                                  01000100b,00001000b,00000010b,00000000b,\
                                  01010101b,01000000b,01000000b,01000000b,\
                                  01010100b,00010000b,00001000b,11100000b,\
                                  00000100b,10001001b,00100001b,00000000b,\
                                  01111100b,00100000b,00000001b,01010000b,\
                                  00111001b,00000100b,00010001b,01010000b,\
                                  00010000b,00000000b,10000001b,00010000b,\
                                  11010111b,11100000b,00011101b,11110000b,\
                                  00010000b,01000000b,00100000b,11100000b,\
                                  00010000b,10000000b,00100000b,01000000b,\
                                  00000001b,00000000b,00010000b,01000000b

.snake_dots                 db    27,6, 0,6, 1,6
.snake_direction            dd    RIGHT
.snake_direction_next       dd    RIGHT
.number_of_stones           dd    95
.name                       dd    stage_19_name

stage_20:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,10001000b,00000000b,\
                                  00000000b,00011100b,10000100b,00000000b,\
                                  00000000b,01100100b,10000010b,00000000b,\
                                  00000011b,10000100b,10000100b,00000000b,\
                                  00000010b,00000100b,10001000b,00000000b,\
                                  00000010b,00000100b,10000100b,00000000b,\
                                  00000010b,00000100b,10000010b,00000000b,\
                                  00000010b,00111100b,10010010b,00000000b,\
                                  00011110b,00111100b,10010100b,00000000b,\
                                  00011110b,00111100b,10001000b,00000000b,\
                                  00011110b,00000000b,10000000b,00000000b,\
                                  00000000b,00000000b,10000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    17,2, 17,3, 17,4
.snake_direction            dd    DOWN
.snake_direction_next       dd    DOWN
.number_of_stones           dd    65
.name                       dd    stage_20_name

stage_21:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000011b,11111111b,11111110b,00000000b,\
                                  00000001b,01000100b,01000100b,00000000b,\
                                  00000010b,01000100b,01000100b,00000000b,\
                                  00001100b,01000100b,01000100b,00000000b,\
                                  01110000b,01000100b,01000100b,00000000b,\
                                  01110000b,01000100b,01000100b,00000000b,\
                                  01110000b,11101110b,11101110b,00000000b,\
                                  00000000b,11101110b,11101110b,00000000b,\
                                  00000000b,11101110b,11101110b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    10,1, 11,1, 12,1
.snake_direction            dd    RIGHT
.snake_direction_next       dd    RIGHT
.number_of_stones           dd    86
.name                       dd    stage_21_name

stage_22:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  00000011b,10000000b,00000000b,00000000b,\
                                  00000111b,11100000b,00000000b,00000000b,\
                                  00001111b,11110000b,00000000b,00000000b,\
                                  00011111b,11100000b,00000000b,00000000b,\
                                  00011111b,11000011b,00001100b,00110000b,\
                                  00011111b,10000111b,10011110b,01110000b,\
                                  00011111b,11000111b,10011110b,01110000b,\
                                  00011111b,11100011b,00001100b,00110000b,\
                                  00001111b,11110000b,00000000b,00000000b,\
                                  00000111b,11100000b,00000000b,00000000b,\
                                  00000011b,10000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    1,7, 1,6, 1,5
.snake_direction            dd    UP
.snake_direction_next       dd    UP
.number_of_stones           dd    104
.name                       dd    stage_22_name

stage_23:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  00100000b,01000101b,00010000b,00100000b,\
                                  00011010b,00100101b,00100010b,11000000b,\
                                  00000100b,10101000b,10101001b,00000000b,\
                                  00000100b,10010010b,01001001b,00000000b,\
                                  00001011b,00110000b,01100110b,10000000b,\
                                  00000000b,11001010b,10011000b,00000000b,\
                                  00000001b,00000111b,00000100b,00000000b,\
                                  00001110b,01001010b,10010011b,10000000b,\
                                  00000010b,00110000b,01100010b,00000000b,\
                                  00000101b,00010010b,01000101b,00000000b,\
                                  00001001b,00001000b,10000100b,10000000b,\
                                  00000000b,00001000b,10000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    15,0, 14,0, 13,0
.snake_direction            dd    LEFT
.snake_direction_next       dd    LEFT
.number_of_stones           dd    85
.name                       dd    stage_23_name

stage_24:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  00111111b,11111111b,11111111b,10000000b,\
                                  00100000b,00000000b,00000000b,10000000b,\
                                  00100011b,11111111b,11111000b,10000000b,\
                                  00100010b,00000000b,00001000b,10000000b,\
                                  00100010b,00111111b,10001000b,10000000b,\
                                  00100010b,00100000b,10001000b,10000000b,\
                                  00101010b,10101010b,10101010b,10000000b,\
                                  00001000b,10001110b,00100010b,00000000b,\
                                  00001000b,10000000b,00100010b,00000000b,\
                                  00001000b,11111111b,11100010b,00000000b,\
                                  00001000b,00000000b,00000010b,00000000b,\
                                  00001111b,11111111b,11111110b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    1,0, 0,0, 0,1
.snake_direction            dd    DOWN
.snake_direction_next       dd    DOWN
.number_of_stones           dd    120
.name                       dd    stage_24_name

stage_25:
.field                      db    00000100b,11000000b,00000000b,00000000b,\
                                  00000011b,10000000b,00110010b,00000000b,\
                                  10011010b,10000000b,00011100b,00000000b,\
                                  01110000b,00000000b,00010101b,10010000b,\
                                  01010000b,00000111b,00000000b,11100000b,\
                                  00000100b,00000101b,00000000b,10100000b,\
                                  00000100b,00000100b,00000000b,00000000b,\
                                  00000011b,11111100b,00011001b,00000000b,\
                                  00000010b,10010100b,00001110b,00000000b,\
                                  00000010b,10010100b,00001010b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000011b,00100001b,10010011b,00100000b,\
                                  00000001b,11000000b,11100001b,11000000b,\
                                  00000001b,01000000b,10100001b,01000000b

.snake_dots                 db    11,2, 12,2, 13,2
.snake_direction            dd    RIGHT
.snake_direction_next       dd    RIGHT
.number_of_stones           dd    88
.name                       dd    stage_25_name

stage_26:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  00111100b,01001111b,01111010b,01000000b,\
                                  00100000b,01000001b,00001010b,01000000b,\
                                  00100100b,01001111b,01111011b,11000000b,\
                                  00000100b,01001000b,00001000b,01000000b,\
                                  00111100b,01001111b,01111000b,01000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00111101b,11101111b,01111011b,11000000b,\
                                  00100001b,00000001b,00000000b,00000000b,\
                                  00111101b,11100001b,01111011b,11000000b,\
                                  00000100b,00000001b,00000000b,01000000b,\
                                  00111101b,11100001b,01111011b,11000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    1,5, 0,5, 0,6
.snake_direction            dd    DOWN
.snake_direction_next       dd    DOWN
.number_of_stones           dd    115
.name                       dd    stage_26_name

stage_27:
.field                      db    00000000b,10000000b,00000000b,01000000b,\
                                  00000000b,10000000b,01000000b,11100000b,\
                                  00100011b,11100000b,01000000b,01000000b,\
                                  01110000b,10000001b,11110000b,00000000b,\
                                  00100000b,10000000b,01000000b,00000000b,\
                                  00000000b,00000000b,01000010b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000010b,00000000b,00000000b,00000000b,\
                                  00000111b,00000000b,00000000b,10000000b,\
                                  00000010b,00001000b,00000001b,11000000b,\
                                  00000000b,00000000b,10000000b,10000000b,\
                                  00000000b,01000001b,11000000b,00000000b,\
                                  01000000b,11100000b,10000000b,00000000b,\
                                  00000000b,01000000b,00000000b,00000000b

.snake_dots                 db    12,8, 12,7, 12,6
.snake_direction            dd    UP
.snake_direction_next       dd    UP
.number_of_stones           dd    51
.name                       dd    stage_27_name

stage_28:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000100b,00000000b,00000010b,00000000b,\
                                  00010100b,00000000b,00000010b,10000000b,\
                                  01010100b,00000000b,00000010b,10100000b,\
                                  01010101b,11111111b,11111010b,10100000b,\
                                  01010100b,00000000b,00000010b,10100000b,\
                                  00010100b,00000000b,00000010b,10000000b,\
                                  00000100b,00000000b,00000010b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    13,8, 12,8, 11,8
.snake_direction            dd    LEFT
.snake_direction_next       dd    LEFT
.number_of_stones           dd    44
.name                       dd    stage_28_name

stage_29:
.field                      db    00000000b,01110000b,00000000b,00000000b,\
                                  00000100b,01000110b,00000001b,10000000b,\
                                  01001110b,00001100b,01100000b,11000000b,\
                                  01000000b,00000000b,01100000b,00000000b,\
                                  01100000b,01111000b,00000001b,10010000b,\
                                  00000000b,00000010b,10000101b,10110000b,\
                                  00110000b,00110010b,10001100b,00100000b,\
                                  00011011b,00110110b,10000100b,00000000b,\
                                  00000001b,00000000b,10010000b,10000000b,\
                                  00100001b,00000000b,00111000b,10000000b,\
                                  00111001b,00110011b,00000011b,10000000b,\
                                  01111111b,11111111b,00011011b,11010000b,\
                                  11111111b,11111111b,00011111b,11110000b,\
                                  11111111b,11111111b,00111111b,11110000b

.snake_dots                 db    0,0, 1,0, 2,0
.snake_direction            dd    RIGHT
.snake_direction_next       dd    RIGHT
.number_of_stones           dd    151
.name                       dd    stage_29_name

stage_30:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000001b,01011100b,00000000b,\
                                  00000000b,00000001b,11001000b,00000000b,\
                                  00000100b,00000001b,01001000b,00000000b,\
                                  00000100b,00000100b,00000000b,00000000b,\
                                  00000100b,00000100b,00000100b,00000000b,\
                                  00000100b,00000100b,00000100b,00000000b,\
                                  00000100b,00000100b,00000100b,00000000b,\
                                  01111111b,11000100b,11111111b,11100000b,\
                                  00000100b,00000100b,00000100b,00000000b,\
                                  00011111b,00111111b,10001110b,00000000b,\
                                  00000100b,00000100b,00000100b,00000000b,\
                                  00011111b,00011111b,00011111b,00000000b,\
                                  11111111b,11111111b,11111111b,11110000b

.snake_dots                 db    8,2, 9,2, 10,2
.snake_direction            dd    RIGHT
.snake_direction_next       dd    RIGHT
.number_of_stones           dd    109
.name                       dd    stage_30_name

stage_31:
.field                      db    00000101b,00010000b,00000100b,01000000b,\
                                  01000100b,01010101b,00010100b,01000000b,\
                                  01000101b,01010101b,01000101b,00010000b,\
                                  01010000b,00010100b,01000000b,01010000b,\
                                  00010101b,01000101b,01010100b,01000000b,\
                                  01000001b,00010000b,01010101b,01000000b,\
                                  01010101b,00010100b,00000101b,00010000b,\
                                  00000101b,01010000b,01000101b,01010000b,\
                                  01010000b,01000100b,00000000b,01010000b,\
                                  00010101b,00000101b,00010100b,00010000b,\
                                  01010001b,00010001b,01000001b,01000000b,\
                                  01000100b,00000101b,01010100b,01010000b,\
                                  00010001b,01010100b,00010001b,00010000b,\
                                  00000100b,01000001b,00010001b,00000000b

.snake_dots                 db    18,8, 17,8, 16,8
.snake_direction            dd    LEFT
.snake_direction_next       dd    LEFT
.number_of_stones           dd    112
.name                       dd    stage_31_name

stage_32:
.field                      db    11111111b,11111111b,11111111b,11110000b,\
                                  10010010b,01001001b,00100100b,10010000b,\
                                  10010000b,01000001b,00000100b,00010000b,\
                                  10010010b,01001001b,00100100b,10010000b,\
                                  10010010b,01001001b,00100100b,10010000b,\
                                  10010010b,01001001b,00100100b,10010000b,\
                                  10010010b,01001001b,00100100b,10010000b,\
                                  10010010b,01001001b,00100100b,10010000b,\
                                  10010010b,01001001b,00100100b,10010000b,\
                                  10010010b,01001001b,00100100b,10010000b,\
                                  10010010b,01001001b,00100100b,10010000b,\
                                  10000010b,00001000b,00100000b,10010000b,\
                                  10010010b,01001001b,00100100b,10010000b,\
                                  11111111b,11111111b,11111111b,11110000b

.snake_dots                 db    1,1, 1,2, 1,3
.snake_direction            dd    DOWN
.snake_direction_next       dd    DOWN
.number_of_stones           dd    168
.name                       dd    stage_32_name

stage_33:
.field                      db    00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  01111111b,11001111b,11111111b,11100000b,\
                                  01000100b,01001000b,01000000b,00100000b,\
                                  01000100b,01001000b,01000001b,10100000b,\
                                  01000100b,01001000b,01001101b,10100000b,\
                                  00000000b,00000000b,00000110b,00000000b,\
                                  01000100b,01001000b,01000010b,00100000b,\
                                  01000100b,01001000b,01001111b,00100000b,\
                                  01000100b,01001000b,01001111b,00100000b,\
                                  01000100b,01001000b,01000110b,00100000b,\
                                  01111111b,11001111b,11111111b,11100000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    6,6, 7,6, 8,6
.snake_direction            dd    RIGHT
.snake_direction_next       dd    RIGHT
.number_of_stones           dd    109
.name                       dd    stage_33_name

stage_34:
.field                      db    01110000b,00000000b,00000011b,10000000b,\
                                  00010010b,00010000b,01111100b,00000000b,\
                                  00011110b,00010000b,00100100b,00000000b,\
                                  00000100b,00011001b,00100111b,00000000b,\
                                  00001111b,10001001b,00100000b,00110000b,\
                                  00000001b,00001011b,00101000b,00100000b,\
                                  00000001b,00011110b,01111000b,00100000b,\
                                  00111000b,00000010b,00001100b,10100000b,\
                                  00001110b,00100010b,00000000b,10100000b,\
                                  01100011b,11111110b,01000011b,11100000b,\
                                  00111110b,00100010b,01000000b,10000000b,\
                                  00000000b,01100011b,11000010b,10000000b,\
                                  00000000b,01000000b,01100111b,10000000b,\
                                  00000000b,00000000b,00000010b,00000000b

.snake_dots                 db    7,0, 8,0, 9,0
.snake_direction            dd    RIGHT
.snake_direction_next       dd    RIGHT
.number_of_stones           dd    113
.name                       dd    stage_34_name

stage_35:
.field                      db    00000100b,00000000b,00001010b,00000000b,\
                                  00010100b,01000000b,00101010b,00000000b,\
                                  00010100b,10000010b,00010010b,10000000b,\
                                  00010001b,00000000b,00010010b,10000000b,\
                                  00010001b,00000010b,00001010b,10000000b,\
                                  01010010b,00000010b,00001000b,10100000b,\
                                  01000100b,00000000b,00001000b,10100000b,\
                                  01000100b,00000010b,00000100b,10100000b,\
                                  01001000b,00000010b,00000100b,00100000b,\
                                  01001000b,00000000b,00000010b,00100000b,\
                                  00010000b,00000010b,00000010b,00100000b,\
                                  00110000b,00000010b,00000010b,00100000b,\
                                  00100000b,00000010b,00000001b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b

.snake_dots                 db    13,11, 13,10, 13,9
.snake_direction            dd    UP
.snake_direction_next       dd    UP
.number_of_stones           dd    66
.name                       dd    stage_35_name

stage_36:
.field                      db    10101110b,10001110b,00110100b,11100000b,\
                                  11101000b,10001110b,00101010b,10000000b,\
                                  10101110b,11101000b,00101010b,11100000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00000000b,10110010b,01000100b,00000000b,\
                                  00000001b,10100101b,01010100b,00000000b,\
                                  00000001b,10100101b,00101000b,00000000b,\
                                  00000000b,00000000b,00000000b,00000000b,\
                                  00010001b,11010101b,11010001b,11000000b,\
                                  00010001b,11010101b,11010001b,00000000b,\
                                  00010001b,00010101b,00010000b,10000000b,\
                                  00011101b,11001001b,11011100b,01000000b,\
                                  00000000b,00000000b,00000001b,11010000b

.snake_dots                 db    27,11, 27,10, 27,9
.snake_direction            dd    UP
.snake_direction_next       dd    UP
.number_of_stones           dd    112
.name                       dd    stage_36_name


stage_00_name               db    'Classic mode',0
stage_01_name               db    'Begin',0
stage_02_name               db    'Frame',0
stage_03_name               db    'Sight',0
stage_04_name               db    'Dashed',0
stage_05_name               db    'Beams',0
stage_06_name               db    'Pipe',0
stage_07_name               db    'Labyrinth',0
stage_08_name               db    'Sea battle',0
stage_09_name               db    'Recursion',0
stage_10_name               db    'Narrow corridors',0
stage_11_name               db    'CCC',0
stage_12_name               db    'Deadlocks',0
stage_13_name               db    'Boat',0
stage_14_name               db    'Pattern',0
stage_15_name               db    'Guernica',0
stage_16_name               db    'Goto',0
stage_17_name               db    'Smiling face',0
stage_18_name               db    'Waves',0
stage_19_name               db    'First snow',0
stage_20_name               db    'Music and silence',0
stage_21_name               db    'Experiment',0
stage_22_name               db    'Pacman',0
stage_23_name               db    'Intricate pattern',0
stage_24_name               db    'Square arcs',0
stage_25_name               db    'In the animal world',0
stage_26_name               db    'Digits',0
stage_27_name               db    'Pluses',0
stage_28_name               db    'Rod',0
stage_29_name               db    'Tetris',0
stage_30_name               db    'Towers of Hanoi',0
stage_31_name               db    'Ruins',0
stage_32_name               db    'Walls of Akendora',0
stage_33_name               db    'Geranium in the window',0
stage_34_name               db    'Algae',0
stage_35_name               db    'The road ahead',0
stage_36_name               db    'Help me draw levels!',0

background_color            dd    0x000000
decorations_color           dd    0x00000000
snake_color                 dd    0x000000
snake_head_color            dd    0x000000
lives_in_head_number_color  dd    0x000000
snake_picture_color         dd    0x000000
version_picture_color       dd    0x000000
pause_picture_color         dd    0x000000
game_over_picture_color     dd    0x000000
you_win_picture_color       dd    0x000000
eat_color                   dd    0x000000
button_color                dd    0x000000
stone_color                 dd    0x000000
splash_background_color     dd    0x000000
splash_level_string_color   dd    0x000000
splash_level_number_color   dd    0x000000

align 4
@IMPORT:

library \
    libini          , 'libini.obj'      ,\
    box_lib         , 'box_lib.obj'

import libini,\
    ini.get_str     , 'ini_get_str'     ,\
    ini.get_int     , 'ini_get_int'     ,\
    ini.set_str     , 'ini_set_str'     ,\
    ini.set_int     , 'ini_set_int'     ,\
    ini.get_color   , 'ini_get_color'   ,\
    ini.get_shortcut, 'ini_get_shortcut'

import box_lib,\
    edit_box.draw   , 'edit_box'        ,\
    edit_box.key    , 'edit_box_key'    ,\
    edit_box.mouse  , 'edit_box_mouse'

aPreferences                db    'Preferences',0
aSpeed                      db    'Speed',0
aTheme                      db    'Theme',0
aSmart_reverse              db    'Smart_reverse',0
aShow_lives_style           db    'Show_lives_style',0
aDraw_level_name_in_title   db    'Draw_level_name_in_window_title',0
aSeparating_symbol          db    'Separating_symbol',0

aShortcuts                  db    'Shortcuts',0
aMove_left                  db    'Move_left',0
aMove_down                  db    'Move_down',0
aMove_up                    db    'Move_up',0
aMove_right                 db    'Move_right',0
aReverse                    db    'Reverse',0
aIncrease                   db    'Increase',0
aDecrease                   db    'Decrease',0

aTheme_name                 db    32 dup (0)
aDecorations                db    'Decorations',0
aBackground_color           db    'Background_color',0
aDecorations_color          db    'Decorations_color',0
aSnake_color                db    'Snake_color',0
aSnake_head_color           db    'Snake_head_color',0
aLives_in_head_number_color db    'Lives_in_head_number_color',0
aSnake_picture_color        db    'Snake_picture_color',0
aVersion_picture_color      db    'Version_picture_color',0
aPause_picture_color        db    'Pause_picture_color',0
aGame_over_picture_color    db    'Game_over_picture_color',0
aYou_win_picture_color      db    'You_win_picture_color',0
aEat_color                  db    'Eat_color',0
aEdit_box_selection_color   db    'Edit_box_selection_color',0
aButton_color               db    'Button_color',0
aStone_color                db    'Stone_color',0
aSplash_background_color    db    'Splash_background_color',0
aSplash_level_string_color  db    'Splash_level_string_color',0
aSplash_level_number_color  db    'Splash_level_number_color',0

aReserved                   db    'Reserved',0
aSquare_side_length         db    'Square_side_length',0

config:
    .championNameClassic    db    'Champion_name_classic',0
    .championNameLevels     db    'Champion_name_levels',0
    .hiscoreClassic         db    'Hiscore_classic',0
    .hiscoreLevels          db    'Hiscore_levels',0

configFont:
    .flag                   dd    0
    .width                  dd    6
    .height                 dd    7
    .mask                   dd    0x80000000
    .maskNumber             dd    0x40000000

configColor:
    .labelNavigation        db    'Navigation_string_color',0
    .labelScore             db    'Score_string_color',0
    .numberScore            db    'Score_number_color',0
    .labelLevel             db    'Level_string_color',0
    .numberLevel            db    'Level_number_color',0
    .labelHiscore           db    'Hiscore_string_color',0
    .numberHiscore          db    'Hiscore_number_color',0
    .labelChampion          db    'Champion_string_color',0
    .labelChampionName      db    'Champion_name_color',0
    .labelGameOver          db    'Game_over_string_color',0
    .numberGameOver         db    'Game_over_hiscore_color',0
    .buttonText             db    'Button_text_color',0

macro defLabel name, color, [text]
{
    common
        local ..str
        if lang eq ru_RU
            ..str cp866 text, 0
        else if lang eq ru_RU
            ..str cp850 text, 0
        else
            ..str db text, 0
        end if
        name#.len = $ - ..str - 1
        name LABEL color, name#.len, ..str
}
macro defNumber name, color, size
{
    common
        name#.len = size
        name LABEL color, name#.len, 0
}

if lang eq ru_RU
    defLabel labelButtonPlay   , DEFAULT_BUTTON_TEXT_COLOR, 'ИГРАТЬ'
    defLabel labelButtonExit   , DEFAULT_BUTTON_TEXT_COLOR, 'ВЫХОД'
    defLabel labelButtonClassic, DEFAULT_BUTTON_TEXT_COLOR, 'режим КЛАССИЧЕСКИЙ'
    defLabel labelButtonLevels , DEFAULT_BUTTON_TEXT_COLOR, 'режим УРОВНЕЙ'
    defLabel labelButtonInc    , DEFAULT_BUTTON_TEXT_COLOR, 'УВЕЛИЧИТЬ'
    defLabel labelButtonDec    , DEFAULT_BUTTON_TEXT_COLOR, 'УМЕНЬШИТЬ'

    defLabel labelScore   , DEFAULT_LABEL_COLOR, 'СЧЕТ : '
    defLabel labelLevel   , DEFAULT_LABEL_COLOR, 'УРОВЕНЬ : '
    defLabel labelHiscore , DEFAULT_LABEL_COLOR, 'ЛУЧШИЙ СЧЕТ : '
    defLabel labelChampion, DEFAULT_LABEL_COLOR, 'ЧЕМПИОН : '

    defLabel labelMenu  , DEFAULT_NAVIGATION_COLOR, 'НАЖМИТЕ ',0x27,'ESC',0x27,' ДЛЯ ВЫХОДА В МЕНЮ'
    defLabel labelExit  , DEFAULT_NAVIGATION_COLOR, 'НАЖМИТЕ ',0x27,'ESC',0x27,' ДЛЯ ВЫХОДА ИЗ ИГРЫ'
    defLabel labelStart , DEFAULT_NAVIGATION_COLOR, 'НАЖМИТЕ ',0x27,'ENTER',0x27,' ЧТОБЫ НАЧАТЬ'
    defLabel labelPause , DEFAULT_NAVIGATION_COLOR, 'НАЖМИТЕ ',0x27,'SPACE',0x27, ' ДЛЯ ПАУЗЫ'
    defLabel labelApply , DEFAULT_NAVIGATION_COLOR, 'ВВЕДИТЕ ИМЯ И НАЖМИТЕ ',0x27,'ENTER',0x27
    defLabel labelResume, DEFAULT_NAVIGATION_COLOR, 'НАЖМИТЕ ',0x27,'SPACE',0x27, ' ДЛЯ ПРОДОЛЖЕНИЯ'

    defLabel labelCongratulations, DEFAULT_LABEL_END_COLOR, 'Поздравляем!!! Новый лучший счет : '
    defLabel labelEnterName      , DEFAULT_LABEL_END_COLOR, 'Теперь вы чемпион! Введите свое имя : '
else
    defLabel labelButtonPlay   , DEFAULT_BUTTON_TEXT_COLOR, 'PLAY'
    defLabel labelButtonExit   , DEFAULT_BUTTON_TEXT_COLOR, 'EXIT'
    defLabel labelButtonClassic, DEFAULT_BUTTON_TEXT_COLOR, 'CLASSIC mode'
    defLabel labelButtonLevels , DEFAULT_BUTTON_TEXT_COLOR, 'LEVELS mode'
    defLabel labelButtonInc    , DEFAULT_BUTTON_TEXT_COLOR, '+INC+'
    defLabel labelButtonDec    , DEFAULT_BUTTON_TEXT_COLOR, '-DEC-'

    defLabel  labelScore   , DEFAULT_LABEL_COLOR, 'SCORE : '
    defLabel  labelLevel   , DEFAULT_LABEL_COLOR, 'LEVEL : '
    defLabel  labelHiscore , DEFAULT_LABEL_COLOR, 'HI-SCORE : '
    defLabel  labelChampion, DEFAULT_LABEL_COLOR, 'CHAMPION : '

    defLabel  labelMenu  , DEFAULT_NAVIGATION_COLOR, 'MENU - ',0x27,'ESC',0x27
    defLabel  labelExit  , DEFAULT_NAVIGATION_COLOR, 'PRESS ',0x27,'ESC',0x27,' TO EXIT'
    defLabel  labelStart , DEFAULT_NAVIGATION_COLOR, 'PRESS ',0x27,'ENTER',0x27,' TO START'
    defLabel  labelPause , DEFAULT_NAVIGATION_COLOR, 'PAUSE - ',0x27,'SPACE',0x27
    defLabel  labelApply , DEFAULT_NAVIGATION_COLOR, 'APPLY NAME - ',0x27,'ENTER',0x27
    defLabel  labelResume, DEFAULT_NAVIGATION_COLOR, 'RESUME - ',0x27,'SPACE',0x27

    defLabel  labelCongratulations, DEFAULT_LABEL_END_COLOR, 'Congratulations!!! New hi-score is : '
    defLabel  labelEnterName      , DEFAULT_LABEL_END_COLOR, 'You are the champion! Enter your name : '
end if

defNumber numberScore      , DEFAULT_LABEL_COLOR, 7
defNumber numberLevel      , DEFAULT_LABEL_COLOR, 2
defNumber numberHiscore    , DEFAULT_LABEL_COLOR, 7
defLabel  labelChampionName, DEFAULT_LABEL_COLOR, CHAMPION_NAME_LENGTH dup (0x20)
defNumber numberGameOver   , DEFAULT_NUMBER_END_COLOR, 7

edit1 edit_box 65,397,0x0,0x000000,0x000000,0x000000,0x000000,0x80000000,CHAMPION_NAME_LENGTH,hed,mouse_dd,ed_focus,hed_end-hed-1,hed_end-hed-1
hed                         db    '',0
;;---Variables-------------------------------------------------------------------------------------------------------------
i_end:
hed_end:                 rb    256
proc_info                process_information

posLabel:
    .xLeft               rd    1
    .xRight              rd    1
    .yTop                rd    1
    .yCenter             rd    1
    .yBottom             rd    1

labelChampionNameClassic rb    CHAMPION_NAME_LENGTH
labelChampionNameLevels  rb    CHAMPION_NAME_LENGTH
numberHiscoreClassic     rd    1
numberHiscoreLevels      rd    1

mouse_dd                 rd    1

window_style             rd    1

cur_level                rd    1

snake_dots               rb    GRID_WIDTH*GRID_HEIGHT*2+3          ; +3 bytes for faster dword copying
snake_direction          rd    1
snake_direction_next     rd    1
snake_length_x2          rd    1

decorations              rd    1
number_of_free_dots      rd    1

eat                      rw    1

g_s                      rd    1
g_e                      rd    1

window_width             rd    1
window_height            rd    1
wp_x                     rd    1
wp_y                     rd    1

gw_mul_gs                rd    1
gh_mul_gs                rd    1
gbxm1_plus_gw_mul_gs     rd    1
gbym1_plus_gh_mul_gs     rd    1
gs_shl16_gs              rd    1
gbxm1_shl16_gbxm1        rd    1
gbym1_shl16_gbym1        rd    1

button_x_left            rd    1
button_x_right           rd    1
button_y_top             rd    1
button_y_middle          rd    1
button_y_bottom          rd    1
button_width_short       rd    1
button_width_long        rd    1
button_height            rd    1

cursor_data              rb    32*32*4
cursor_handle            rd    1

cur_dir_path             rb    4096

field_map                rb    GRID_WIDTH*GRID_HEIGHT*2

smart_reverse            rd    1
show_lives_style         rd    1
draw_level_name_in_title rd    1
separating_symbol        rd    1

shortcut_move_left       rb    1
shortcut_move_down       rb    1
shortcut_move_up         rb    1
shortcut_move_right      rb    1
shortcut_reverse         rb    1
shortcut_increase        rb    1
shortcut_decrease        rb    1

square_side_length       rd    1

gbxm1                    rd    1
gbym1                    rd    1
speed_up_counter         rw    1

rb 4096
stacktop:
d_end:
