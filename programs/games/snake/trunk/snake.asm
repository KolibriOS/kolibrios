;;===HEADER====================================================================================================================

use32
    org 0x0
    db  'MENUET01'
    dd  0x01,start,i_end,d_end,stacktop,0x0,cur_dir_path

;;---HEADER--------------------------------------------------------------------------------------------------------------------

include '../../../proc32.inc'
include '../../../macros.inc'
include '../../../system/launch/trunk/mem.inc'
include '../../../develop/libraries/libs-dev/.test/dll.inc'
include '../../../develop/libraries/box_lib/trunk/box_lib.mac'
;include '../../../system/board/trunk/debug.inc'

;;===Define_chapter============================================================================================================

GRID_WIDTH                  equ     28
GRID_HEIGHT                 equ     14

MIN_SQUARE_SIDE_LENGTH      equ     9

SCORE_EAT                   equ     100

LEFT                        equ     0
DOWN                        equ     1
UP                          equ     2
RIGHT                       equ     3

struct  LEVEL
    field                   db      GRID_WIDTH*GRID_HEIGHT  dup (?)
    snake_dots              db      6   dup (?)
    snake_direction         dd      ?
    snake_direction_next    dd      ?
    number_of_stones        dd      ?
ends

CLASSIC_MODE                equ     0
LEVELS_MODE                 equ     1

CLASSIC_MODE_FIRST_LEVEL    equ     0
LEVELS_MODE_FIRST_LEVEL     equ     1

EAT_TO_END_LEVEL            equ     12
PAUSE_BETWEEN_LEVELS        equ     250
PAUSE_WHILE_DRAWING_SPLASH  equ     4

CHAMPION_NAME_LENGTH        equ     15
LAST_LEVEL_NUMBER           equ     12

;;---Define_chapter------------------------------------------------------------------------------------------------------------

start:

stdcall dll.Load,@IMPORT
    or   eax, eax
    jnz  Exit
    
align 4

    mov  eax, cur_dir_path
  @@:
    cmp  byte[eax], 0
     jz  @f
    inc  eax
     jmp @b
  @@:
    mov  dword[eax],    '.ini'

      invoke  ini.get_int, cur_dir_path, aPreferences, aSpeed, 80
    neg  eax
    add  [time_wait_limit],    eax
      invoke  ini.get_str, cur_dir_path, aPreferences, aTheme, aTheme_name, 31, aTheme_name

      invoke  ini.get_int, cur_dir_path, aTheme_name, aDecorations, 2
    mov  [decorations], eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aBackground_color, 0x000000
    or   [background_color],    eax
    or   [window_style],    eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aDecorations_color, 0xAAAA00
    or   [decorations_color],  eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aSnake_color, 0x1111ff
    or   [snake_color], eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aSnake_head_color, 0x6B6Bff
    or   [snake_head_color], eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aSnake_picture_color, 0x4488ff
    or   [snake_picture_color], eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aVersion_picture_color, 0x55ff55
    or   [version_picture_color],   eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aPause_picture_color, 0x11ff11
    or   [pause_picture_color], eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aGame_over_picture_color, 0xff1111
    or   [game_over_picture_color], eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aYou_win_picture_color, 0xffff11
    or   [you_win_picture_color],   eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aEat_color, 0xffff11
    or   [eat_color],   eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aNavigation_strings_color, 0x80ff7777
    or   [navigation_strings_color], eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aGame_over_strings_color, 0x80ff9900
    or   [game_over_strings_color],  eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aScore_string_color, 0x80ffffff
    or   [score_string_color],   eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aLevel_string_color, 0xffffff
    or   [level_string_color],  eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aHiscore_string_color, 0x80ffffff
    or   [hiscore_string_color],   eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aChampion_string_color, 0x80ffffff
    or   [champion_string_color],   eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aGame_over_hiscore_color, 0x80ffdd44
    or   [game_over_hiscore_color], eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aScore_number_color, 0xffffff
    or   [score_number_color],   eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aLevel_number_color, 0xffffff
    or   [level_number_color],  eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aHiscore_number_color, 0x00ffffff
    or   [hiscore_number_color],   eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aChampion_name_color, 0x80ffffff
    or   [champion_name_color],   eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aEdit_box_selection_color, 0x00aa00
    or   [edit1.shift_color],   eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aButton_color, 0xDDDDDD
    or   [button_color],    eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aButton_text_color, 0x000000
    or   [button_text_color],   eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aStone_color, 0x5f8700
    or   [stone_color], eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aSplash_background_color, 0xAAAA00
    or   [splash_background_color],    eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aSplash_level_number_color, 0x000000
    or   [splash_level_number_color],   eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aSplash_level_string_color, 0x000000
    or   [splash_level_string_color],   eax

      invoke  ini.get_int, cur_dir_path, aReserved, aSquare_side_length, 19
    mov  [square_side_length],  eax
      invoke  ini.get_str, cur_dir_path, aReserved, aChampion_name_classic, champion_name_classic, CHAMPION_NAME_LENGTH, champion_name_classic
      invoke  ini.get_int, cur_dir_path, aReserved, aHiscore_classic, 777
    or   [hi_score_classic],    eax
      invoke  ini.get_str, cur_dir_path, aReserved, aChampion_name_levels, champion_name_levels, CHAMPION_NAME_LENGTH, champion_name_levels
      invoke  ini.get_int, cur_dir_path, aReserved, aHiscore_levels, 777
    or   [hi_score_levels], eax

    mov  eax, [background_color]
    mov  [edit1.color], eax
    mov  [edit1.focus_border_color],    eax
    mov  [edit1.blur_border_color], eax
    mov  eax, [game_over_hiscore_color]
    mov  [edit1.text_color],    eax

      mcall     37,4,cursor_data,2                  ; load empty cursor (for "hiding" cursor while level_mode)
    mov  [cursor_handle],   eax

      call      Set_geometry

include 'first_menu.asm'            ; First menu body and functions
include 'level.asm'                 ; Level body and functions (game process)
include 'pause.asm'                 ; Pause body and functions
include 'game_over.asm'             ; Game_over body and functions

;;===Some_functions============================================================================================================


Save_do_smth_else_and_exit:
    ;;===Save_do_smth_else_and_exit============================================================================================

      mcall     37,6,[cursor_handle]                ; delete cursor

      invoke    ini.set_int, cur_dir_path, aReserved, aSquare_side_length, [square_side_length]

    mov  edi, champion_name_classic
    xor  al,  al
    mov  ecx, CHAMPION_NAME_LENGTH+1
    cld
    repne scasb
    neg  ecx
    add  ecx, CHAMPION_NAME_LENGTH
      invoke    ini.set_str, cur_dir_path, aReserved, aChampion_name_classic, champion_name_classic, ecx
      invoke    ini.set_int, cur_dir_path, aReserved, aHiscore_classic, [hi_score_classic]

    mov  edi, champion_name_levels
    xor  al,  al
    mov  ecx, CHAMPION_NAME_LENGTH+1
    cld
    repne scasb
    neg  ecx
    add  ecx, CHAMPION_NAME_LENGTH
      invoke    ini.set_str, cur_dir_path, aReserved, aChampion_name_levels, champion_name_levels, ecx
      invoke    ini.set_int, cur_dir_path, aReserved, aHiscore_levels, [hi_score_levels]

    ;;---Save_do_smth_else_and_exit--------------------------------------------------------------------------------------------


Exit:
    ;;===Exit==================================================================================================================

    or  eax,    -1
    int 0x40
    
    ;;---Exit------------------------------------------------------------------------------------------------------------------


Set_geometry:
    ;;===Set_geometry==========================================================================================================

    mov  eax, [square_side_length]
    inc  eax                                            ; space between squares
    mov  [g_s],   eax

    mov  eax, [g_s]
    shr  eax, 1
    mov  ebx, eax
    shr  ebx, 1
    add  eax, ebx
    mov  [g_e], eax

    mov  eax, [g_s]
    add  eax, [g_e]
    mov  [gbxm1],   eax

    mov  eax, [g_e]
    add  eax, 25
    mov  [gbym1],   eax

    mov  edx, GRID_WIDTH
    mov  eax, [g_s]
    mul  dx
    mov  [gw_mul_gs],   eax

    mov  edx, GRID_HEIGHT
    mov  eax, [g_s]
    mul  dx
    mov  [gh_mul_gs],   eax

    mov  eax, [gbxm1]
    add  eax, [gw_mul_gs]
    mov  [gbxm1_plus_gw_mul_gs],    eax

    mov  eax, [gbym1]
    add  eax, [gh_mul_gs]
    mov  [gbym1_plus_gh_mul_gs],    eax

    mov  eax, [g_s]
    shl  eax, 16
    add  eax, [g_s]
    mov  [gs_shl16_gs], eax

    mov  eax, [gbxm1]
    shl  eax, 16
    add  eax, [gbxm1]
    mov  [gbxm1_shl16_gbxm1],   eax

    mov  eax, [gbym1]
    shl  eax, 16
    add  eax, [gbym1]
    mov  [gbym1_shl16_gbym1],   eax


    mov  eax, [gw_mul_gs]
    add  eax, [gbxm1]
    add  eax, [gbxm1]
    add  eax, 5*2                                   ; skin width
    mov  [window_width],    eax

      mcall     48,4                                ; get skin header height
    add  eax, [gh_mul_gs]
    add  eax, [gbym1]
    add  eax, [g_e]
    add  eax, 30
    add  eax, 5                                      ; skin height (bottom part)
    mov  [window_height],   eax

      mcall     48, 5
    mov  dx,  ax
    shr  eax, 16
    sub  dx,  ax
    cmp  dx, word[window_width]                     ; does window fit to work area width?
     jnl @f
    dec  [square_side_length]
     jmp Set_geometry
  @@:

    mov  cx,  bx
    shr  ebx, 16
    sub  cx,  bx
    cmp  cx, word[window_height]                     ; does window fit to work area height?
     jnl @f
    dec  [square_side_length]
     jmp Set_geometry
  @@:

    sub  dx,  word[window_width]
    shr  dx,  1
    mov  word[wp_x],    dx
    sub  cx,  word[window_height]
    shr  cx,  1
    mov  dx,  cx
    shr  cx,  1
    add  cx,  dx
    mov  word[wp_y],    cx

    mov  [top_strings], 8
    mov  eax, [window_height]
    sub  eax, 50
    mov  [bottom_top_strings],  eax
    add  eax, 6
    mov  [bottom_middle_strings],  eax
    add  eax, 6
    mov  [bottom_bottom_strings],  eax

    sub  eax, 4
    mov  [edit1.top],   eax


    mov  eax, [g_s]
    shl  eax, 2
    sub  eax, 2
    mov  [button_width_short],  eax
    mov  eax, [g_s]
    shl  eax, 3
    add  eax, [g_s]
    sub  eax, 2
    mov  [button_width_long],   eax
    mov  eax, [g_s]
    sub  eax, 2
    mov  [button_height],   eax

    mov  bl,  0x10
    mov  cl,  0x08

    mov  al,  byte[g_s]
    mul  bl
    mov  bx,  ax
    add  bx,  word[gbxm1]
    inc  bx

    mov  al,  byte[g_s]
    mul  cl
    mov  cx,  ax
    add  cx,  word[gbym1]
    inc  cx
    
    mov  [button_x_left],   ebx
    mov  [button_y_top],    ecx
    
    add  ebx, [g_s]
    add  ebx, [g_s]
    add  ebx, [g_s]
    add  ebx, [g_s]
    add  ebx, [g_s]
    
    mov  [button_x_right],  ebx
    
    add  ecx,  [g_s]
    add  ecx,  [g_s]
    
    mov  [button_y_middle], ecx
    
    add  ecx,  [g_s]
    add  ecx,  [g_s]
    
    mov  [button_y_bottom], ecx


    ret

    ;;---Set_geometry------------------------------------------------------------------------------------------------------


Increase_geometry:
    ;;===Increase_geometry=================================================================================================

    inc  [square_side_length]
      call      Set_geometry
      mcall     67,[wp_x],[wp_y],[window_width],[window_height]

    ret

    ;;---Increase_geometry-------------------------------------------------------------------------------------------------


Decrease_geometry:
    ;;===Decrease_geometry=================================================================================================

    cmp  [square_side_length],  MIN_SQUARE_SIDE_LENGTH
     je  @f
    dec  [square_side_length]
      call      Set_geometry
      mcall     67,[wp_x],[wp_y],[window_width],[window_height]

  @@:
    ret

    ;;---Decrease_geometry-------------------------------------------------------------------------------------------------


Draw_decorations:
    ;;===Draw_decorations==================================================================================================

    mov  al, byte[decorations]
    dec  al
     jz  grid_lines
    dec  al
     jz  grid_lines_with_ends
    dec  al
     jz  grid_lines_with_corners
    dec  al
     jz  grid_dots
    dec  al
     jz  borders_lines
    dec  al
     jz  borders_lines_with_corners
    dec  al
     jz  borders_dots
    dec  al
     jz  corners_dots
    dec  al
     jz  corners_inner
    dec  al
     jz  corners_outer
    dec  al
     jz  corners_crosses
    ret


  grid_lines:

    mov  eax, 38
;mov  ebx, (GRID_BEGIN_X-1)*65536+(GRID_BEGIN_X-1)
    mov  ebx, [gbxm1_shl16_gbxm1]
;mov  ecx, (GRID_BEGIN_Y-1)*65536+(GRID_BEGIN_Y-1+GRID_HEIGHT*GRID_STEP)
    mov  ecx, [gbym1_shl16_gbym1]
    add  ecx, [gh_mul_gs]
    mov  edx, [decorations_color]
    mov  esi, GRID_WIDTH
    add  esi, 1

  @@:
      mcall
    add  ebx, [gs_shl16_gs]
    dec  esi
     jnz @b

;mov  ebx, (GRID_BEGIN_X-1)*65536+(GRID_BEGIN_X-1+GRID_WIDTH*GRID_STEP)
    mov  ebx, [gbxm1_shl16_gbxm1]
    add  ebx, [gw_mul_gs]
    mov  ecx, [gbym1_shl16_gbym1]
    mov  esi, GRID_HEIGHT
    add  esi, 1
    
  @@:
      mcall
    add  ecx, [gs_shl16_gs]
    dec  esi
     jnz @b

    ret


  grid_lines_with_ends:

    mov  eax, 38
    mov  ebx, [gbxm1_shl16_gbxm1]
    mov  ecx, [gbym1]
    sub  ecx, [g_e]
    shl  ecx, 16
    add  ecx, [gbym1_plus_gh_mul_gs]
    add  ecx, [g_e]
    mov  edx, [decorations_color]
    mov  esi, GRID_WIDTH
    add  esi, 1

  @@:
      mcall
    add  ebx, [gs_shl16_gs]
    dec  esi
     jnz @b

    mov  ebx, [gbxm1]
    sub  ebx, [g_e]
    shl  ebx, 16
    add  ebx, [gbxm1_plus_gw_mul_gs]
    add  ebx, [g_e]
    mov  ecx, [gbym1_shl16_gbym1]
    mov  esi, GRID_HEIGHT
    add  esi, 1
    
  @@:
      mcall
    add  ecx, [gs_shl16_gs]
    dec  esi
     jnz @b

    ret


  grid_lines_with_corners:

      call      grid_lines
      call      corners_outer

    ret


  grid_dots:

    mov  eax, 1
    mov  ebx, [gbxm1]
    mov  ecx, [gbym1]
    mov  edx, [decorations_color]

  @@:
      mcall
    add  ebx, [g_s]
    cmp  ebx, [gbxm1_plus_gw_mul_gs]
     jng @b
    add  ecx, [g_s]
    cmp  ecx, [gbym1_plus_gh_mul_gs]
     jg  @f
    mov  ebx, [gbxm1]
     jmp @b

  @@:
    ret


  borders_lines:

    mov  eax, 38
    mov  ebx, [gbxm1_shl16_gbxm1]
    mov  ecx, [gbym1_shl16_gbym1]
    add  ecx, [gh_mul_gs]
    mov  edx, [decorations_color]
      mcall

    mov  ebx, [gbxm1_plus_gw_mul_gs]
    shl  ebx, 16
    add  ebx, [gbxm1_plus_gw_mul_gs]
      mcall

    mov  ebx, [gbxm1_shl16_gbxm1]
    add  ebx, [gw_mul_gs]
    mov  ecx, [gbym1_shl16_gbym1]
      mcall

    mov  ecx, [gbym1_plus_gh_mul_gs]
    shl  ecx, 16
    add  ecx, [gbym1_plus_gh_mul_gs]
      mcall

    ret


  borders_lines_with_corners:

      call      borders_lines
      call      corners_outer

    ret


  borders_dots:

    mov  eax, 1
    mov  ebx, [gbxm1]
    mov  ecx, [gbym1]
    mov  edx, [decorations_color]
  @@:
      mcall
    add  ebx, [g_s]
    cmp  ebx, [gbxm1_plus_gw_mul_gs]
     jng @b

    mov  ebx, [gbxm1]
    mov  ecx, [gbym1_plus_gh_mul_gs]
  @@:
      mcall
    add  ebx, [g_s]
    cmp  ebx, [gbxm1_plus_gw_mul_gs]
     jng @b

    mov  ebx, [gbxm1]
    mov  ecx, [gbym1]
  @@:
      mcall
    add  ecx, [g_s]
    cmp  ecx, [gbym1_plus_gh_mul_gs]
     jng @b

    mov  ebx, [gbxm1_plus_gw_mul_gs]
    mov  ecx, [gbym1]
  @@:
      mcall
    add  ecx, [g_s]
    cmp  ecx, [gbym1_plus_gh_mul_gs]
     jng @b

    ret


  corners_dots:

    mov  eax, 13
    mov  ebx, [gbxm1]
    dec  ebx
    shl  ebx, 16
    add  ebx, 2
    mov  ecx, [gbym1]
    dec  ecx
    shl  ecx, 16
    add  ecx, 2
    mov  edx, [decorations_color]
      mcall

    mov  ebx, [gbxm1_plus_gw_mul_gs]
    shl  ebx, 16
    add  ebx, 2
      mcall

    mov  ebx, [gbxm1]
    dec  ebx
    shl  ebx, 16
    add  ebx, 2
    mov  ecx, [gbym1_plus_gh_mul_gs]
    shl  ecx, 16
    add  ecx, 2
      mcall

    mov  ebx, [gbxm1_plus_gw_mul_gs]
    shl  ebx, 16
    add  ebx, 2
      mcall

    ret


  corners_inner:

    mov  eax, 38
    mov  ebx, [gbxm1_shl16_gbxm1]
    add  ebx, [g_e]
    mov  ecx, [gbym1_shl16_gbym1]
    mov  edx, [decorations_color]
      mcall

    mov  ecx, [gbym1_plus_gh_mul_gs]
    shl  ecx, 16
    add  ecx, [gbym1_plus_gh_mul_gs]
      mcall

    mov  ebx, [gbxm1_plus_gw_mul_gs]
    sub  ebx, [g_e]
    shl  ebx, 16
    add  ebx, [gbxm1_plus_gw_mul_gs]
      mcall

    mov  ecx, [gbym1_shl16_gbym1]
      mcall

    mov  ebx, [gbxm1_shl16_gbxm1]
    mov  ecx, [gbym1_shl16_gbym1]
    add  ecx, [g_e]
      mcall

    mov  ebx, [gbxm1_plus_gw_mul_gs]
    shl  ebx, 16
    add  ebx, [gbxm1_plus_gw_mul_gs]
      mcall

    mov  ecx, [gbym1_plus_gh_mul_gs]
    sub  ecx, [g_e]
    shl  ecx, 16
    add  ecx, [gbym1_plus_gh_mul_gs]
      mcall

    mov  ebx, [gbxm1_shl16_gbxm1]
      mcall

    ret


  corners_outer:

    mov  eax, 38
    mov  ebx, [gbxm1_shl16_gbxm1]
    sub  ebx, [g_e]
    mov  ecx, [gbym1_shl16_gbym1]
    mov  edx, [decorations_color]
      mcall

    mov  ecx, [gbym1_plus_gh_mul_gs]
    shl  ecx, 16
    add  ecx, [gbym1_plus_gh_mul_gs]
      mcall

    mov  ebx, [gbxm1_plus_gw_mul_gs]
    shl  ebx, 16
    add  ebx, [gbxm1_plus_gw_mul_gs]
    add  ebx, [g_e]
      mcall

    mov  ecx, [gbym1_shl16_gbym1]
      mcall

    mov  ebx, [gbxm1_shl16_gbxm1]
    mov  ecx, [gbym1_shl16_gbym1]
    sub  ecx, [g_e]
      mcall

    mov  ebx, [gbxm1_plus_gw_mul_gs]
    shl  ebx, 16
    add  ebx, [gbxm1_plus_gw_mul_gs]
      mcall

    mov  ecx, [gbym1_plus_gh_mul_gs]
    shl  ecx, 16
    add  ecx, [gbym1_plus_gh_mul_gs]
    add  ecx, [g_e]
      mcall

    mov  ebx, [gbxm1_shl16_gbxm1]
      mcall

    ret


  corners_crosses:

      call      corners_inner
      call      corners_outer

    ret


    ;;---Draw_decorations--------------------------------------------------------------------------------------------------


Draw_square:
    ;;===Draw_square=======================================================================================================
    ;; bl   -   x_coord
    ;; bh   -   y_coord
    ;; edx  -   color

    push eax ebx ecx edx

    mov  cl,  bh

    mov  al,  byte[g_s]
    mul  bl
    mov  bx,  ax
    add  bx,  word[gbxm1]
    inc  bx
    shl  ebx, 16
    add  ebx, [g_s]
    dec  ebx

    mov  al,  byte[g_s]
    mul  cl
    mov  cx,  ax
    add  cx,  word[gbym1]
    inc  cx
    shl  ecx, 16
    add  ecx, [g_s]
    dec  ecx

      mcall     13

    pop  edx ecx ebx eax

    ret

    ;;---Draw_square-------------------------------------------------------------------------------------------------------
    
    
Draw_menu_esc:
    ;;===Draw_menu_esc=====================================================================================================

    mov  ebx, [window_width]
    shr  ebx, 1
    sub  ebx, string_menu_esc.size*3+6
    shl  ebx, 16
    add  ebx, dword[top_strings]
      mcall     4, ,[navigation_strings_color],string_menu_esc
    
    ret
    
    ;;---Draw_menu_esc-----------------------------------------------------------------------------------------------------


Draw_score_string:
    ;;===Draw_score_string=================================================================================================

    mov  ebx, [window_width]
    shr  ebx, 3
    sub  ebx, 5
    shl  ebx, 16
    add  ebx, dword[bottom_top_strings]
      mcall     4, ,[score_string_color],string_score

    ret

    ;;---Draw_score_string-------------------------------------------------------------------------------------------------
    
    
Draw_score_number:
    ;;===Draw_score_number================================================================================================= 

    mov  edx, [window_width]
    shr  edx, 3
    sub  edx, 5+1
    add  edx, string_score.size*6
    shl  edx, 16
    add  edx, dword[bottom_top_strings]
      mcall     47,0x00070000,[score], ,[score_number_color],[background_color]

    ret

    ;;---Draw_score_number-------------------------------------------------------------------------------------------------


Draw_hiscore_string:
    ;;===Draw_hiscore_string===============================================================================================

    mov  ebx, [window_width]
    shr  ebx, 3
    neg  ebx
    add  ebx, [window_width]
    sub  ebx, string_hi_score.size*6+7*6+5
    shl  ebx, 16
    add  ebx, dword[bottom_top_strings]
      mcall     4, ,[hiscore_string_color],string_hi_score
    
    ret
    
    ;;---Draw_hiscore_string-----------------------------------------------------------------------------------------------


Draw_hiscore_number:
    ;;===Draw_hiscore_number===================================================================================================

    mov  edx, [window_width]
    shr  edx, 3
    neg  edx
    add  edx, [window_width]
    sub  edx, 7*6+6
    shl  edx, 16
    add  edx, dword[bottom_top_strings]
    
    cmp  [play_mode],   CLASSIC_MODE
     jne @f
    mov  ecx, [hi_score_classic]
     jmp .done
  @@:
    mov  ecx, [hi_score_levels]

  .done:
      mcall     47,0x00070000, , ,[hiscore_number_color]
    
    ret
    
    ;;---Draw_hiscore_number---------------------------------------------------------------------------------------------------


Draw_champion_string:
    ;;===Draw_champion_string==================================================================================================

    mov  ebx, [window_width]
    shr  ebx, 3
    neg  ebx
    add  ebx, [window_width]
    sub  ebx, string_champion.size*6+7*6+5
    shl  ebx, 16
    add  ebx, dword[bottom_bottom_strings]
      mcall     4, ,[champion_string_color],string_champion

    ret

    ;;---Draw_champion_string----------------------------------------------------------------------------------------------


Draw_champion_name:
    ;;===Draw_champion_name================================================================================================

    mov  ebx, [window_width]
    shr  ebx, 3
    neg  ebx
    add  ebx, [window_width]
    sub  ebx, CHAMPION_NAME_LENGTH/2*6+7*6+6                ; there is no difference between length of champion names for other play_modes
    add  ebx, CHAMPION_NAME_LENGTH/2*6
    shl  ebx, 16
    add  ebx, dword[bottom_bottom_strings]

    cmp  [play_mode],   CLASSIC_MODE
     jne @f
    mov  edx, champion_name_classic
     jmp .done
  @@:
    mov  edx, champion_name_levels

  .done:
      mcall     4, ,[champion_name_color],

    ret

    ;;---Draw_champion_name------------------------------------------------------------------------------------------------


Draw_picture:
    ;;===Draw_picture======================================================================================================
    ;;  in  :
    ;;           ax =   number of left square *0x100+ picture width (in squares)
    ;;           cx =   number of top square *0x100+ picture height (in squares)
    ;;          edx =   picture color
    ;;          esi =   pointer to picture data
    ;;

    add  al,  ah
    add  cl,  ch
    mov  bh,  ch

  .draw:
    mov  bl,  ah
    
  .loop:
    cmp  byte[esi], 0
     jz  @f
    push eax ebx ecx esi
      call      Draw_square
    pop  esi ecx ebx eax
    
  @@:
    inc  esi
    inc  bl
    cmp  bl,  al
     jne .loop
    
    inc  bh
    cmp  bh,  cl
     jne .draw
    ret

    ;;---Draw_picture------------------------------------------------------------------------------------------------------


Draw_on_map:
    ;;===Draw_on_map=======================================================================================================
    ;;  in  :
    ;;           al =   x coord
    ;;           ah =   y coord
    ;;           cl =   value to draw
    ;;

    push eax ebx edx

    and  eax, 0x0000ffff
    xor  bx,  bx
    mov  bl,  al
    shr  ax,  8
    mov  dx,  GRID_WIDTH
    mul  dx
    add  ax,  bx
    mov  edx, field_map
    add  edx, eax
    mov  [edx], cl
    
    pop edx ebx eax

    ret

    ;;---Draw_on_map-----------------------------------------------------------------------------------------------------------


Get_from_map:
    ;;===Get_from_map==========================================================================================================
    ;;  in  :
    ;;           al =   x coord
    ;;           ah =   y coord
    ;;  out :
    ;;           bl =   value on map
    ;;

    push eax
    
    and  eax, 0x0000ffff
    xor  bx,  bx
    mov  bl,  al
    shr  ax,  8
    mov  dx,  GRID_WIDTH
    mul  dx
    add  ax,  bx
    mov  edi, field_map
    add  edi, eax
    mov  bl,  [edi]

    pop  eax

    ret

    ;;---Get_from_map-----------------------------------------------------------------------------------------------------------


Load_level:
    ;;===Load_level=============================================================================================================
    ;;  in  :
    ;;          cur_level_number    =   level number to load
    ;;

    mov  eax, [cur_level_number]
    mov  edx, stage_00
  @@:
    test al,  al
     jz  @f
    add  edx, 410
    dec  al
     jmp @b
  @@:
  
    mov  [cur_level],   edx

    mov  esi, edx
    add  esi, LEVEL.field
    mov  edi, field_map
    mov  ecx, GRID_WIDTH*GRID_HEIGHT/4
    rep  movsd

    mov  esi, edx
    add  esi, LEVEL.snake_dots
    mov  edi, snake_dots
    mov  ecx, 3
    rep  movsw
    
    mov  esi, edx
    add  esi, LEVEL.snake_direction
    mov  eax, [esi]
    mov  [snake_direction], eax
    
    mov  esi, edx
    add  esi, LEVEL.snake_direction_next
    mov  eax, [esi]
    mov  [snake_direction_next],    eax

    mov  esi, edx
    add  esi, LEVEL.number_of_stones
    mov  eax, [esi]
    mov  [number_of_free_dots], GRID_WIDTH*GRID_HEIGHT-3
    sub  [number_of_free_dots], eax
    
    mov  ax,  word[snake_dots]
    mov  cl,  1
      call      Draw_on_map
    mov  ax,  word[snake_dots+2]
    mov  cl,  1
      call      Draw_on_map
    mov  ax,  word[snake_dots+4]
    mov  cl,  1
      call      Draw_on_map

    mov  [action],  0
    mov  [snake_length_x2], 6

    ret

    ;;---Load_level-------------------------------------------------------------------------------------------------------------


Draw_stones:
    ;;===Draw_stones============================================================================================================

    mov  ax,  0*0x100+GRID_WIDTH
    mov  cx,  0*0x100+GRID_HEIGHT
    mov  edx, [stone_color]
    mov  esi, [cur_level]
    add  esi, LEVEL.field
      call      Draw_picture

    ret

    ;;---Draw_stones------------------------------------------------------------------------------------------------------------


Hide_cursor:
    ;;===Hide_cursor===========================================================================================================

      mcall     37,5,[cursor_handle]

    ret

    ;;---Show_cursor-----------------------------------------------------------------------------------------------------------


Show_cursor:
    ;;===Hide_cursor===========================================================================================================

      mcall     37,5,0

    ret

    ;;---Show_cursor-----------------------------------------------------------------------------------------------------------


;;---Some_functions-------------------------------------------------------------------------------------------------------------


;;===Variables==================================================================================================================

window_title                db      'Snake',0
window_style                dd      0x34000000
time_before_waiting         dd      0x0
time_to_wait                dd      0x0
time_wait_limit             dd      101

play_mode                   dd      0x0

square_side_length          dd      19

gbxm1                       dd      30
gbym1                       dd      30

szZ string_score            ,'SCORE : '
szZ string_hi_score         ,'HI-SCORE : '
;szZ string_player           ,'PLAYER :'
szZ string_champion         ,'CHAMPION : '
szZ string_level            ,'LEVEL : '
;szZ string_hi_level         ,'HI-LEVEL :'
szZ string_pause_space      ,'PAUSE - ',0x27,'SPACE',0x27
szZ string_resume_space     ,'RESUME - ',0x27,'SPACE',0x27
szZ string_menu_esc         ,'MENU - ',0x27,'ESC',0x27
szZ string_apply_name_enter ,'APPLY NAME - ',0x27,'ENTER',0x27
szZ press_to_start          ,'PRESS ',0x27,'ENTER',0x27,' TO START'
szZ press_esc_to_exit       ,'PRESS ',0x27,'ESC',0x27,' TO EXIT'
;press_F2_to_options         db      'PRESS ',0x27,'F2',0x27,' TO OPTIONS',0

szZ string_congratulations  ,'   Congratulations!!! New hi-score is : '
szZ string_enter_your_name  ,'You are the champion! Enter your name : '

szZ string_button_play      ,'PLAY'
szZ string_button_exit      ,'EXIT'
szZ string_button_inc       ,'+INC+'
szZ string_button_dec       ,'-dec-'
szZ string_button_pm_classic,'CLASSIC mode'
szZ string_button_pm_levels ,'LEVELS mode'

is_new_record               dd      0

action                      dd      0

picture_first_menu_snake    db      1,1,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,1,1,\
                                    1,0,0,0,0,1,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,0,1,\
                                    1,1,1,1,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,0,1,1,1,1,\
                                    0,0,0,1,0,1,0,1,1,0,1,1,1,1,0,1,0,1,0,0,1,0,0,0,\
                                    1,1,1,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,1,1,1
                                    

picture_first_menu_version  db      1,1,1,1,0,0,0,1,1,1,1,\
                                    1,0,0,1,0,0,0,1,0,0,0,\
                                    1,0,0,1,0,0,0,1,1,1,1,\
                                    1,0,0,1,0,0,0,0,0,0,1,\
                                    1,1,1,1,0,1,0,1,1,1,1

picture_pause               db      1,1,1,0,0,0,1,1,0,0,1,0,0,1,0,1,1,1,1,0,1,1,1,1,\
                                    1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,0,0,1,0,0,0,\
                                    1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,0,0,1,0,0,0,\
                                    1,1,1,0,0,1,1,1,1,0,1,0,0,1,0,1,1,1,1,0,1,1,1,1,\
                                    1,0,0,0,0,1,0,0,1,0,1,0,0,1,0,0,0,0,1,0,1,0,0,0,\
                                    1,0,0,0,0,1,0,0,1,0,0,1,1,0,0,1,1,1,1,0,1,1,1,1

picture_game_over           db      0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,1,0,0,1,0,0,1,1,1,0,0,1,0,0,0,1,0,1,1,1,1,0,0,0,0,\
                                    1,0,0,0,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,0,0,0,0,\
                                    1,0,0,1,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,1,1,0,0,0,0,0,\
                                    1,0,0,0,1,0,1,1,1,1,1,0,1,0,0,0,1,0,1,0,0,0,0,0,0,0,\
                                    0,1,1,1,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,1,1,1,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,1,1,1,0,1,1,1,1,0,\
                                    0,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,0,1,0,0,0,1,\
                                    0,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,1,1,0,0,1,0,0,0,1,\
                                    0,0,0,0,1,0,0,0,1,0,0,1,0,1,0,0,1,0,0,0,0,1,1,1,1,0,\
                                    0,0,0,0,0,1,1,1,0,0,0,0,1,0,0,0,1,1,1,1,0,1,0,0,0,1

picture_you_win             db      1,0,0,0,1,0,0,1,1,1,0,0,1,0,0,0,1,\
                                    1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,\
                                    0,1,0,1,0,0,1,0,0,0,1,0,1,0,0,0,1,\
                                    0,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1,\
                                    0,0,1,0,0,0,0,1,1,1,0,0,0,1,1,1,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,\
                                    0,1,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,\
                                    0,1,0,1,0,1,0,0,1,0,0,1,0,1,0,1,0,\
                                    0,1,0,1,0,1,0,0,1,0,0,1,0,0,1,1,0,\
                                    0,0,1,0,1,0,0,0,1,0,0,1,0,0,0,1,0

picture_level               db      1,0,0,0,0,1,1,1,1,0,1,0,0,1,0,1,1,1,1,0,1,0,0,0,\
                                    1,0,0,0,0,1,0,0,0,0,1,0,0,1,0,1,0,0,0,0,1,0,0,0,\
                                    1,0,0,0,0,1,1,1,0,0,1,0,0,1,0,1,1,1,0,0,1,0,0,0,\
                                    1,0,0,0,0,1,0,0,0,0,1,0,1,0,0,1,0,0,0,0,1,0,0,0,\
                                    1,1,1,1,0,1,1,1,1,0,1,1,0,0,0,1,1,1,1,0,1,1,1,1

digits_font                 db      1,1,1,1,\
                                    1,0,0,1,\
                                    1,0,0,1,\
                                    1,0,0,1,\
                                    1,1,1,1,\
                                    \
                                    0,0,1,0,\
                                    0,1,1,0,\
                                    0,0,1,0,\
                                    0,0,1,0,\
                                    0,0,1,0,\
                                    \
                                    1,1,1,1,\
                                    0,0,0,1,\
                                    1,1,1,1,\
                                    1,0,0,0,\
                                    1,1,1,1,\
                                    \
                                    1,1,1,1,\
                                    0,0,0,1,\
                                    0,1,1,1,\
                                    0,0,0,1,\
                                    1,1,1,1,\
                                    \
                                    1,0,0,1,\
                                    1,0,0,1,\
                                    1,1,1,1,\
                                    0,0,0,1,\
                                    0,0,0,1,\
                                    \
                                    1,1,1,1,\
                                    1,0,0,0,\
                                    1,1,1,1,\
                                    0,0,0,1,\
                                    1,1,1,1,\
                                    \
                                    1,1,1,1,\
                                    1,0,0,0,\
                                    1,1,1,1,\
                                    1,0,0,1,\
                                    1,1,1,1,\
                                    \
                                    1,1,1,1,\
                                    0,0,0,1,\
                                    0,0,0,1,\
                                    0,0,0,1,\
                                    0,0,0,1,\
                                    \
                                    1,1,1,1,\
                                    1,0,0,1,\
                                    1,1,1,1,\
                                    1,0,0,1,\
                                    1,1,1,1,\
                                    \
                                    1,1,1,1,\
                                    1,0,0,1,\
                                    1,1,1,1,\
                                    0,0,0,1,\
                                    1,1,1,1

stage_00:
.field                      db      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

.snake_dots                 db      3,3, 4,3, 5,3
.snake_direction            dd      RIGHT
.snake_direction_next       dd      RIGHT
.number_of_stones           dd      0

stage_01:
.field                      db      2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,\
                                    2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,\
                                    2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,\
                                    2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,\
                                    2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,\
                                    2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,\
                                    2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,\
                                    2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,\
                                    2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2

.snake_dots                 db      3,3, 4,3, 5,3
.snake_direction            dd      RIGHT
.snake_direction_next       dd      RIGHT
.number_of_stones           dd      36

stage_02:
.field                      db      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,0,0,0,\
                                    0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,\
                                    0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,\
                                    0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,\
                                    0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,\
                                    0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,\
                                    0,0,0,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

.snake_dots                 db      7,5, 8,5, 9,5
.snake_direction            dd      RIGHT
.snake_direction_next       dd      RIGHT
.number_of_stones           dd      40

stage_03:
.field                      db      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,2,2,2,2,2,2,2,2,2,0,0,2,2,2,2,2,2,2,2,2,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,2,2,2,2,2,2,2,2,2,0,0,2,2,2,2,2,2,2,2,2,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

.snake_dots                 db      23,0, 22,0, 21,0
.snake_direction            dd      LEFT
.snake_direction_next       dd      LEFT
.number_of_stones           dd      44

stage_04:
.field                      db      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,\
                                    0,0,0,2,0,0,0,0,0,0,2,0,0,0,0,0,0,2,0,0,0,0,0,0,2,0,0,0,\
                                    0,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,\
                                    0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,\
                                    0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,\
                                    0,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,2,0,0,0,0,0,0,2,0,0,0,0,0,0,2,0,0,0,0,0,0,2,0,0,0,\
                                    0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

.snake_dots                 db      19,6, 19,7, 19,8
.snake_napravlenie          dd      DOWN
.snake_napravlenie_next     dd      DOWN
.number_of_stones           dd      40

stage_05:
.field                      db      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,\
                                    0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,\
                                    0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

.snake_dots                 db      0,0, 0,1, 1,1
.snake_direction            dd      RIGHT
.snake_direction_next       dd      RIGHT
.number_of_stones           dd      112

stage_06:
.field                      db      0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,\
                                    0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,\
                                    0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,\
                                    0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,\
                                    0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,0

.snake_dots                 db      0,0, 0,1, 1,1
.snake_direction            dd      RIGHT
.snake_direction_next       dd      RIGHT
.number_of_stones           dd      128

stage_07:
.field                      db      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,2,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,2,0,2,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,2,0,2,2,2,2,2,2,2,2,2,2,0,2,0,2,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,2,0,2,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,2,0,2,2,2,2,2,2,2,2,2,2,2,2,0,2,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

.snake_dots                 db      8,1, 9,1, 10,1
.snake_direction            dd      RIGHT
.snake_direction_next       dd      RIGHT
.number_of_stones           dd      83

stage_08:
.field                      db      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,2,0,2,0,0,2,0,0,2,0,0,0,0,0,2,0,2,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,\
                                    0,0,0,0,2,2,2,2,0,0,0,0,0,0,0,0,2,2,2,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,2,0,0,0,\
                                    0,0,0,0,0,2,2,2,0,0,2,0,0,0,0,0,2,0,0,0,0,0,2,0,2,0,0,0,\
                                    0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,\
                                    0,0,0,2,0,0,0,0,2,2,0,0,0,0,0,0,0,2,2,2,0,0,2,0,0,0,0,0,\
                                    0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

.snake_dots                 db      0,0, 1,0, 2,0
.snake_direction            dd      RIGHT
.snake_direction_next       dd      RIGHT
.number_of_stones           dd      40

stage_09:
.field                      db      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,1,1,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,1,1,0,0,\
                                    0,0,1,0,0,0,0,1,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,0,0,0,0,\
                                    0,0,1,1,1,1,0,1,0,1,1,0,0,0,0,1,0,1,1,0,0,0,1,1,1,0,0,0,\
                                    0,0,0,0,0,1,0,1,0,1,1,0,0,1,1,1,0,1,0,1,0,0,1,0,0,0,0,0,\
                                    0,0,1,1,1,1,0,1,0,0,1,0,0,0,0,1,0,1,0,0,1,0,1,1,1,1,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

.snake_dots                 db      12,6, 12,7, 12,8
.snake_direction            dd      DOWN
.snake_direction_next       dd      DOWN
.number_of_stones           dd      59

stage_10:
.field                      db      2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,\
                                    2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,\
                                    2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,\
                                    2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,\
                                    2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,\
                                    2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,\
                                    2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,\
                                    2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0,2,2,2,0

.snake_dots                 db      3,2, 3,3, 4,3
.snake_direction            dd      RIGHT
.snake_direction_next       dd      RIGHT
.number_of_stones           dd      231

stage_11:
.field                      db      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,2,2,2,0,0,0,0,0,2,2,2,0,0,0,0,0,2,2,2,0,0,0,0,\
                                    0,0,0,0,2,2,0,2,2,0,0,0,2,2,0,2,2,0,0,0,2,2,0,2,2,0,0,0,\
                                    0,0,0,2,2,0,0,0,2,2,0,2,2,0,0,0,2,2,0,2,2,0,0,0,2,2,0,0,\
                                    0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,2,0,0,\
                                    0,0,0,2,2,0,0,0,2,2,0,2,2,0,0,0,2,2,0,2,2,0,0,0,2,2,0,0,\
                                    0,0,0,0,2,2,0,2,2,0,0,0,2,2,0,2,2,0,0,0,2,2,0,2,2,0,0,0,\
                                    0,0,0,0,0,2,2,2,0,0,0,0,0,2,2,2,0,0,0,0,0,2,2,2,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

.snake_dots                 db      3,12, 4,12, 5,12
.snake_direction            dd      RIGHT
.snake_direction_next       dd      RIGHT
.number_of_stones           dd      69

stage_12:
.field                      db      0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,2,2,2,0,0,0,0,0,\
                                    0,2,2,0,2,2,2,0,0,0,0,2,0,0,0,0,0,0,0,0,2,0,2,0,0,2,0,2,\
                                    0,2,0,0,2,0,2,2,2,2,0,2,2,0,0,2,2,2,0,0,0,0,0,0,0,2,2,2,\
                                    0,2,2,0,0,0,0,2,0,2,0,0,0,0,0,2,0,2,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,2,0,0,0,\
                                    0,2,0,2,0,0,2,2,0,0,0,0,2,2,0,0,2,0,2,0,0,2,2,0,0,0,2,2,\
                                    0,2,2,2,0,0,2,0,0,0,0,0,2,0,0,0,2,2,2,0,0,2,0,0,0,0,0,2,\
                                    0,0,0,0,0,0,2,2,0,0,0,0,2,2,0,0,0,0,0,0,0,2,2,0,0,0,2,2,\
                                    0,0,0,0,0,0,0,0,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,2,0,2,0,0,2,0,2,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,\
                                    2,2,0,2,2,2,0,0,0,0,0,0,0,2,2,0,2,0,2,0,0,2,0,0,0,0,0,0,\
                                    0,2,0,0,0,0,2,2,2,0,0,0,0,0,2,0,2,2,2,0,0,2,2,0,2,0,2,0,\
                                    2,2,0,0,0,0,2,0,2,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,2,2,2,0

.snake_dots                 db      27,0, 26,0, 25,0
.snake_direction            dd      LEFT
.snake_direction_next       dd      LEFT
.number_of_stones           dd      110

background_color            dd      0x000000
decorations_color           dd      0x00000000
snake_color                 dd      0x000000
snake_head_color            dd      0x000000
snake_picture_color         dd      0x000000
version_picture_color       dd      0x000000
pause_picture_color         dd      0x000000
game_over_picture_color     dd      0x000000
you_win_picture_color       dd      0x000000
eat_color                   dd      0x000000
navigation_strings_color    dd      0x80000000
game_over_strings_color     dd      0x80000000
score_string_color          dd      0x80000000
hiscore_string_color        dd      0x80000000
champion_string_color       dd      0x80000000
game_over_hiscore_color     dd      0x80000000
score_number_color          dd      0x40000000
hiscore_number_color        dd      0x00000000
champion_name_color         dd      0x80000000
button_color                dd      0x000000
button_text_color           dd      0x80000000
stone_color                 dd      0x000000
splash_background_color     dd      0x000000
splash_level_string_color   dd      0x000000
splash_level_number_color   dd      0x000000
level_string_color          dd      0x80000000
level_number_color          dd      0x00000000


align 4
@IMPORT:

library \
        libini      ,   'libini.obj'        ,\
        box_lib     ,   'box_lib.obj'

import  libini,\
    ini.get_str     ,   'ini_get_str'       ,\
    ini.get_int     ,   'ini_get_int'       ,\
    ini.set_str     ,   'ini_set_str'       ,\
    ini.set_int     ,   'ini_set_int'       ,\
    ini.get_color   ,   'ini_get_color'

import  box_lib,\
    edit_box.draw   ,   'edit_box'          ,\
    edit_box.key    ,   'edit_box_key'      ,\
    edit_box.mouse  ,   'edit_box_mouse'

bFirstDraw  db  0

aPreferences                db      'Preferences',0
aSpeed                      db      'Speed',0
aTheme                      db      'Theme',0

aTheme_name                 db      32  dup (0)
aDecorations                db      'Decorations',0
aBackground_color           db      'Background_color',0
aDecorations_color          db      'Decorations_color',0
aSnake_color                db      'Snake_color',0
aSnake_head_color           db      'Snake_head_color',0
aSnake_picture_color        db      'Snake_picture_color',0
aVersion_picture_color      db      'Version_picture_color',0
aPause_picture_color        db      'Pause_picture_color',0
aGame_over_picture_color    db      'Game_over_picture_color',0
aYou_win_picture_color      db      'You_win_picture_color',0
aEat_color                  db      'Eat_color',0
aNavigation_strings_color   db      'Navigation_string_color',0
aGame_over_strings_color    db      'Game_over_string_color',0
aScore_string_color         db      'Score_string_color',0
aHiscore_string_color       db      'Hiscore_string_color',0
aChampion_string_color      db      'Champion_string_color',0
aGame_over_hiscore_color    db      'Game_over_hiscore_color',0
aScore_number_color         db      'Score_number_color',0
aHiscore_number_color       db      'Hiscore_number_color',0
aChampion_name_color        db      'Champion_name_color',0
aEdit_box_selection_color   db      'Edit_box_selection_color',0
aButton_color               db      'Button_color',0
aButton_text_color          db      'Button_text_color',0
aStone_color                db      'Stone_color',0
aSplash_background_color    db      'Splash_background_color',0
aSplash_level_string_color  db      'Splash_level_string_color',0
aSplash_level_number_color  db      'Splash_level_number_color',0
aLevel_string_color         db      'Level_string_color',0
aLevel_number_color         db      'Level_number_color',0

aReserved                   db      'Reserved',0
aSquare_side_length         db      'Square_side_length',0
aHiscore_classic            db      'Hiscore_classic',0
aChampion_name_classic      db      'Champion_name_classic',0
aHiscore_levels             db      'Hiscore_levels',0
aChampion_name_levels       db      'Champion_name_levels',0

edit1 edit_box 65,397,0x0,0x000000,0x000000,0x000000,0x000000,0x80000000,15,hed,mouse_dd,ed_focus,hed_end-hed-1,hed_end-hed-1

hed                         db      '',0
;;---Variables-------------------------------------------------------------------------------------------------------------
i_end:
hed_end:
rb  256
mouse_dd                    rd      1

cur_level                   rd      1
cur_level_number            rd      1
hi_level                    rd      1

score                       rd      1
hi_score_classic            rd      1
hi_score_levels             rd      1

champion_name_classic       rb      CHAMPION_NAME_LENGTH
champion_name_levels        rb      CHAMPION_NAME_LENGTH

snake_dots                  rb      GRID_WIDTH*GRID_HEIGHT*2+3          ; +3 bytes for faster dword copying
snake_direction             rd      1
snake_direction_next        rd      1
snake_length_x2             rd      1

decorations                 rd      1
number_of_free_dots         rd      1

eat                         rw      1

g_s                         rd      1
g_e                         rd      1

window_width                rd      1
window_height               rd      1
wp_x                        rd      1
wp_y                        rd      1

gw_mul_gs                   rd      1
gh_mul_gs                   rd      1
gbxm1_plus_gw_mul_gs        rd      1
gbym1_plus_gh_mul_gs        rd      1
gs_shl16_gs                 rd      1
gbxm1_shl16_gbxm1           rd      1
gbym1_shl16_gbym1           rd      1

bottom_top_strings          rd      1
bottom_middle_strings       rd      1
bottom_bottom_strings       rd      1
top_strings                 rd      1

button_x_left               rd      1
button_x_right              rd      1
button_y_top                rd      1
button_y_middle             rd      1
button_y_bottom             rd      1
button_width_short          rd      1
button_width_long           rd      1
button_height               rd      1

cursor_data                 rb      32*32*4
cursor_handle               rd      1

cur_dir_path                rb      4096
@PARAMS                     rb      4096

field_map                   rb      GRID_WIDTH*GRID_HEIGHT*2

rb 4096
stacktop:
d_end: