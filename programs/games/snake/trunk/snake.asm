;;===HEADER====================================================================================================================

use32
    org 0x0
    db  'MENUET01'
    dd  0x1,start,i_end,d_end,stacktop,0x0,cur_dir_path

;;---HEADER--------------------------------------------------------------------------------------------------------------------

include '../../../proc32.inc'
include '../../../macros.inc'
include '../../../system/launch/trunk/mem.inc'
include '../../../develop/libraries/libs-dev/.test/dll.inc'
include '../../../develop/libraries/box_lib/trunk/box_lib.mac'
;include '../../../system/board/trunk/debug.inc'

;;===Define_chapter============================================================================================================

SCORE_EAT                   equ     100

LEFT                        equ     0
DOWN                        equ     1
UP                          equ     2
RIGHT                       equ     3

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
      invoke  ini.get_int, cur_dir_path, aPreferences, aSquare_side_length, 19
    mov  [square_side_length],  eax
      invoke  ini.get_int, cur_dir_path, aPreferences, aSpace_between_squares, 1
    mov  [space_between_squares],   eax
      invoke  ini.get_str, cur_dir_path, aPreferences, aTheme, aTheme_name, 31, aTheme_name

      invoke  ini.get_int, cur_dir_path, aTheme_name, aDecorations, 2
    mov  [decorations], al
      invoke  ini.get_color, cur_dir_path, aTheme_name, aBackground_color, 0x000000
    or   [background_color],    eax
    or   [window_style],    eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aDecorations_color, 0x00aaaa00
    or   [decorations_color],  eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aSnake_color, 0x1111ff
    or   [snake_color], eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aSnake_head_color, 0x1111ff
    or   [snake_head_color], eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aSnake_picture_color, 0x4488ff
    or   [snake_picture_color], eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aVersion_picture_color, 0x55ff55
    or   [version_picture_color],   eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aPause_picture_color, 0x11ff11
    or   [pause_picture_color], eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aGame_over_picture_color, 0xff1111
    or   [game_over_picture_color], eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aEat_color, 0xffff11
    or   [eat_color],   eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aNavigation_strings_color, 0x80ff7777
    or   [navigation_strings_color], eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aGame_over_strings_color, 0x80ff9900
    or   [game_over_strings_color],  eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aScore_string_color, 0x80ffffff
    or   [score_string_color],   eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aHiscore_string_color, 0x80ffffff
    or   [hiscore_string_color],   eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aChampion_string_color, 0x80ffffff
    or   [champion_string_color],   eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aGame_over_hiscore_color, 0x80ffdd44
    or   [game_over_hiscore_color], eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aScore_number_color, 0xffffff
    or   [score_number_color],   eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aHiscore_number_color, 0x00ffffff
    or   [hiscore_number_color],   eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aChampion_name_color, 0x80ffffff
    or   [champion_name_color],   eax
      invoke  ini.get_color, cur_dir_path, aTheme_name, aEdit_box_selection_color, 0x00aa00
    or   [edit1+0x10],  eax

    mov  eax, [background_color]
    mov  [edit1+0x0C],  eax
    mov  [edit1+0x14],  eax
    mov  [edit1+0x18],  eax
    mov  eax, [game_over_hiscore_color]
    mov  [edit1+0x1C],  eax

      call      Set_geometry

include 'first_menu.asm'            ; First menu body and functions
include 'level.asm'                 ; Level body and functions (game process)
include 'pause.asm'                 ; Pause body and functions
include 'game_over.asm'             ; Game_over body and functions

;;===Some_functions============================================================================================================

Exit:
    ;;===Exit==================================================================================================================

    or  eax,    -1
    int 0x40
    
    ;;---Exit------------------------------------------------------------------------------------------------------------------


Set_geometry:
    ;;===Set_geometry==========================================================================================================

    mov  eax, [space_between_squares]
    add  eax, [square_side_length]
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

    mov  eax, [g_w]
    mul  word[g_h]
    mov  [gw_mul_gh],   eax

    mov  edx, [g_w]
    mov  eax, [g_s]
    mul  dx
    mov  [gw_mul_gs],   eax

    mov  edx, [g_h]
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

    mov  eax, [gh_mul_gs]
    add  eax, [gbym1]
    add  eax, [g_e]
    add  eax, 30
    add  eax, 22+5                                  ; skin height
    mov  [window_height],   eax

      mcall     48, 5
    mov  dx,  ax
    shr  eax, 16
    sub  dx,  ax
    cmp  dx, word[window_width]                     ; does window fit to work area width?
     jnl @f
    dec  [square_side_length]
;    dps  'snake: Window does not fit to screen.'
;    newline
;    dps  'Square_side_length was decreased.'
;    newline
;    dps  'Check you config file! (snake.ini)'
;    newline
     jmp Set_geometry
  @@:

    mov  cx,  bx
    shr  ebx, 16
    sub  cx,  bx
    cmp  cx, word[window_height]                     ; does window fit to work area height?
     jnl @f
    dec  [square_side_length]
;    dps  'snake: Window does not fit to screen.'
;    newline
;    dps  'Square_side_length was decreased.'
;    newline
;    dps  'Check you config file! (snake.ini)'
;    newline
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
    mov  [edit1+0x08],  eax

    ret

    ;;---Set_geometry------------------------------------------------------------------------------------------------------


Draw_decorations:
    ;;===Draw_decorations==================================================================================================

    mov  al, [decorations]
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
    mov  esi, [g_w]
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
    mov  esi, [g_h]
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
    mov  esi, [g_w]
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
    mov  esi, [g_h]
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

    ret

    ;;---Draw_square-------------------------------------------------------------------------------------------------------
    
    
Draw_menu_esc:
    ;;===Draw_menu_esc=====================================================================================================

    mov  ebx, [window_width]
    shr  ebx, 1
    sub  ebx, (string_apply_name_enter-string_menu_esc-1)*3+6
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
    sub  edx, 6
    add  edx, (string_hi_score-string_score)*6
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
    sub  ebx, (string_player-string_hi_score)*6+7*6+5
    shl  ebx, 16
    add  ebx, dword[bottom_top_strings]
      mcall     4, ,[hiscore_string_color],string_hi_score
    
    ret
    
    ;;---Draw_hiscore_string-----------------------------------------------------------------------------------------------

    
Draw_hiscore_number:
    ;;===Draw_hiscore_number===============================================================================================

    mov  edx, [window_width]
    shr  edx, 3
    neg  edx
    add  edx, [window_width]
    sub  edx, 7*6+6
    shl  edx, 16
    add  edx, dword[bottom_top_strings]
      mcall     47,0x00070000,[hi_score], ,[hiscore_number_color]
    
    ret
    
    ;;---Draw_hiscore_number-----------------------------------------------------------------------------------------------
    
    
Draw_champion_string:
    ;;===Draw_champion_string==============================================================================================

    mov  ebx, [window_width]
    shr  ebx, 3
    neg  ebx
    add  ebx, [window_width]
    sub  ebx, (string_level-string_champion)*6+7*6+5
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
    sub  ebx, (press_to_start-champion_name)*6+7*6+6
    add  ebx, (press_to_start-champion_name)*6
    shl  ebx, 16
    add  ebx, dword[bottom_bottom_strings]
      mcall     4, ,[champion_name_color],champion_name

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

    and  eax, 0x0000ffff
    xor  bx,  bx
    mov  bl,  al
    shr  ax,  8
    mov  dx,  word[g_w]
    mul  dx
    add  ax,  bx
    mov  edi, field_map
    add  edi, eax
    mov  [edi], cl

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

    push eax
    
    and  eax, 0x0000ffff
    xor  bx,  bx
    mov  bl,  al
    shr  ax,  8
    mov  dx,  word[g_w]
    mul  dx
    add  ax,  bx
    mov  edi, field_map
    add  edi, eax
    mov  bl,  [edi]

    pop  eax

    ret

    ;;---Get_from_map------------------------------------------------------------------------------------------------------


;;---Some_functions--------------------------------------------------------------------------------------------------------


;;===Variables=============================================================================================================

window_title                db      'Snake',0
window_style                dd      0x34000000
time_before_waiting         dd      0x0
time_to_wait                dd      0x0
time_wait_limit             dd      101


gbxm1                       dd      30
gbym1                       dd      30
g_w                         dd      29
g_h                         dd      15
g_e                         dd      13

field_map                   db      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

string_score                db      'SCORE :',0
string_hi_score             db      'HI-SCORE :',0
string_player               db      'PLAYER :',0
string_champion             db      'CHAMPION :',0
string_level                db      'LEVEL :',0
string_pause_space          db      'PAUSE - ',0x27,'SPACE',0x27,0
string_resume_space         db      'RESUME - ',0x27,'SPACE',0x27,0
string_menu_esc             db      'MENU - ',0x27,'ESC',0x27,0
string_apply_name_enter     db      'APPLY NAME - ',0x27,'ENTER',0x27,0
press_to_start              db      'PRESS ',0x27,'SPACE',0x27,' OR ',0x27,'ENTER',0x27,' TO START',0
press_esc_to_exit           db      'PRESS ',0x27,'ESC',0x27,' TO EXIT',0
;press_F2_to_options         db      'PRESS ',0x27,'F2',0x27,' TO OPTIONS',0

string_congratulations      db      '   Congratulations!!! New hi-score is :',0
string_enter_your_name      db      'You are the champion! Enter your name :',0
strings_end:

snake_dots                  db      3,3, 4,3, 5,3,  865    dup (0)
snake_napravlenie           db      3
snake_napravlenie_next      db      3
snake_length_x2             dd      6

score                       dd      0
hi_score                    dd      777
is_new_record               db      0

action                      db      0

picture_first_menu_snake    db      1,1,1,1,1,0,1,0,0,0,1,0,0,1,1,1,0,0,1,0,0,1,0,0,1,1,1,1,1,\
                                    1,0,0,0,0,0,1,1,0,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,0,\
                                    1,0,0,0,0,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,0,\
                                    1,1,1,1,1,0,1,0,1,0,1,0,1,1,1,1,1,0,1,1,0,0,0,0,1,1,1,1,0,\
                                    0,0,0,0,1,0,1,0,0,1,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,0,\
                                    1,1,1,1,1,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,1,1,0,1,1,1,1,1

picture_first_menu_version  db      1,1,1,1,0,0,0,1,1,1,1,\
                                    1,0,0,1,0,0,0,0,0,0,1,\
                                    1,0,0,1,0,0,0,0,1,1,1,\
                                    1,0,0,1,0,0,0,0,0,0,1,\
                                    1,1,1,1,0,1,0,1,1,1,1

picture_pause               db      0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,1,0,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,1,1,0,1,1,1,1,0,0,\
                                    0,0,1,0,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,0,0,1,0,0,0,0,0,\
                                    0,0,1,1,1,1,0,0,1,0,0,1,0,1,0,0,1,0,1,1,1,1,0,1,1,1,0,0,0,\
                                    0,0,1,0,0,0,0,0,1,1,1,1,0,1,0,0,1,0,0,0,0,1,0,1,0,0,0,0,0,\
                                    0,0,1,0,0,0,0,0,1,0,0,1,0,0,1,1,0,0,1,1,1,1,0,1,1,1,1,0,0

picture_game_over           db      0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,1,0,0,1,0,0,1,1,1,0,0,1,0,0,0,1,0,1,1,1,1,0,0,0,0,0,0,0,\
                                    1,0,0,0,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,0,0,0,0,0,0,0,\
                                    1,0,0,1,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,1,1,0,0,0,0,0,0,0,0,\
                                    1,0,0,0,1,0,1,1,1,1,1,0,1,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,\
                                    0,1,1,1,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,1,1,1,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,1,1,1,0,1,1,1,1,0,0,\
                                    0,0,0,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,0,1,0,0,0,1,0,\
                                    0,0,0,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,1,1,0,0,1,0,0,0,1,0,\
                                    0,0,0,0,0,0,1,0,0,0,1,0,0,1,0,1,0,0,1,0,0,0,0,1,1,1,1,0,0,\
                                    0,0,0,0,0,0,0,1,1,1,0,0,0,0,1,0,0,0,1,1,1,1,0,1,0,0,0,1,0

start_map                   db      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

background_color            dd      0x000000
decorations_color           dd      0x00000000
snake_color                 dd      0x000000
snake_head_color            dd      0x000000
snake_picture_color         dd      0x000000
version_picture_color       dd      0x000000
pause_picture_color         dd      0x000000
game_over_picture_color     dd      0x000000
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
aSquare_side_length         db      'Square_side_length',0
aSpace_between_squares      db      'Space_between_squares',0
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

aScore                      db      'Score',0
aHiscore                    db      'Hiscore',0
aChampion_name              db      'Champion_name',0

edit1 edit_box 65,397,0x0,0x000000,0x000000,0x000000,0x000000,0x80000000,15,hed,mouse_dd,ed_focus,hed_end-hed-1,hed_end-hed-1

hed                         db      '',0
;;---Variables-------------------------------------------------------------------------------------------------------------
i_end:
hed_end:
rb  256
mouse_dd                    rd      1

decorations                 rb      1
number_of_free_dots         rw      1

eat                         rb      1

square_side_length          rd      1
space_between_squares       rd      1
g_s                         rd      1

window_width                rd      1
window_height               rd      1
wp_x                        rd      1
wp_y                        rd      1

gw_mul_gh                   rd      1
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

champion_name               rb      16

cur_dir_path                rb      4096
@PARAMS                     rb      4096

rb 4096
stacktop:
d_end: