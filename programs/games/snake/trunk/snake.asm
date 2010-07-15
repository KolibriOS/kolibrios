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

;;===Define_chapter============================================================================================================

WINDOW_WIDTH                equ     550
WINDOW_HEIGHT               equ     320
GRID_STEP                   equ     20
GRID_BEGIN_X                equ     31
GRID_BEGIN_Y                equ     31
GRID_WIDTH                  equ     24
GRID_HEIGHT                 equ     11
GRID_ENDS_WIDTH             equ     13

BOTTOM_TOP_STRINGS          equ     270
BOTTOM_MIDDLE_STRINGS       equ     276
BOTTOM_BOTTOM_STRINGS       equ     282
TOP_STRINGS                 equ     5

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

      invoke  ini.get_int, cur_dir_path, aPreferences, aSpeed, 70
    neg  eax
    add  [time_wait_limit],    eax
      invoke  ini.get_int, cur_dir_path, aPreferences, aDecorations, 1
    mov  [decorations], al

      invoke  ini.get_color, cur_dir_path, aColors, aBackground_color, 0x000000
    or   [background_color],    eax
    or   [window_style],    eax
      invoke  ini.get_color, cur_dir_path, aColors, aDecorations_color, 0x00aaaa00
    or   [decorations_color],  eax
      invoke  ini.get_color, cur_dir_path, aColors, aSnake_color, 0x1111ff
    or   [snake_color], eax
      invoke  ini.get_color, cur_dir_path, aColors, aSnake_head_color, 0x1111ff
    or   [snake_head_color], eax
      invoke  ini.get_color, cur_dir_path, aColors, aSnake_picture_color, 0x4488ff
    or   [snake_picture_color], eax
      invoke  ini.get_color, cur_dir_path, aColors, aVersion_picture_color, 0x55ff55
    or   [version_picture_color],   eax
      invoke  ini.get_color, cur_dir_path, aColors, aPause_picture_color, 0x11ff11
    or   [pause_picture_color], eax
      invoke  ini.get_color, cur_dir_path, aColors, aGame_over_picture_color, 0xff1111
    or   [game_over_picture_color], eax
      invoke  ini.get_color, cur_dir_path, aColors, aEat_color, 0xffff11
    or   [eat_color],   eax
      invoke  ini.get_color, cur_dir_path, aColors, aNavigation_strings_color, 0x80ff7777
    or   [navigation_strings_color], eax
      invoke  ini.get_color, cur_dir_path, aColors, aGame_over_strings_color, 0x80ff9900
    or   [game_over_strings_color],  eax
      invoke  ini.get_color, cur_dir_path, aColors, aScore_string_color, 0x80ffffff
    or   [score_string_color],   eax
      invoke  ini.get_color, cur_dir_path, aColors, aHiscore_string_color, 0x80ffffff
    or   [hiscore_string_color],   eax
      invoke  ini.get_color, cur_dir_path, aColors, aChampion_string_color, 0x80ffffff
    or   [champion_string_color],   eax
      invoke  ini.get_color, cur_dir_path, aColors, aGame_over_hiscore_color, 0x80ffdd44
    or   [game_over_hiscore_color], eax
      invoke  ini.get_color, cur_dir_path, aColors, aScore_number_color, 0xffffff
    or   [score_number_color],   eax
      invoke  ini.get_color, cur_dir_path, aColors, aHiscore_number_color, 0x00ffffff
    or   [hiscore_number_color],   eax
      invoke  ini.get_color, cur_dir_path, aColors, aChampion_name_color, 0x80ffffff
    or   [champion_name_color],   eax

include 'first_menu.asm'            ; First menu body and functions
include 'level.asm'                 ; Level body and functions (game process)
include 'pause.asm'                 ; Pause body and functions
include 'game_over.asm'             ; Game_over body and functions

;;===Some_functions============================================================================================================

Exit:
    ;;===Exit==============================================================================================================

    or  eax,    -1
    int 0x40
    
    ;;---Exit--------------------------------------------------------------------------------------------------------------
    
    
Draw_decorations:
    ;;===Draw_decorations==================================================================================================

    cmp  [decorations], 1
     je  grid_lines
    cmp  [decorations], 2
     je  grid_lines_with_ends
    cmp  [decorations], 3
     je  grid_lines_with_corners
    cmp  [decorations], 4
     je  grid_dots
    cmp  [decorations], 5
     je  borders_lines
    cmp  [decorations], 6
     je  borders_lines_with_ends
    cmp  [decorations], 7
     je  borders_dots
    cmp  [decorations], 8
     je  corners_dots
    cmp  [decorations], 9
     je  corners_inner
    cmp  [decorations], 10
     je  corners_outer
    cmp  [decorations], 11
     je  corners_crosses
    ret


  grid_lines:

    mov  eax, 38
    mov  ebx, (GRID_BEGIN_X-1)*65536+(GRID_BEGIN_X-1)
    mov  ecx, (GRID_BEGIN_Y-1)*65536+(GRID_BEGIN_Y-1+GRID_HEIGHT*GRID_STEP)
    mov  edx, [decorations_color]

  @@:
      mcall
    add  ebx, GRID_STEP*65536+GRID_STEP
    cmp  ebx, (GRID_BEGIN_X-1+GRID_WIDTH*GRID_STEP)*65536+(GRID_BEGIN_X-1+GRID_WIDTH*GRID_STEP)
     jng @b

    mov  ebx, (GRID_BEGIN_X-1)*65536+(GRID_BEGIN_X-1+GRID_WIDTH*GRID_STEP)
    mov  ecx, (GRID_BEGIN_Y-1)*65536+(GRID_BEGIN_Y-1)
    
  @@:
      mcall
    add  ecx, GRID_STEP*65536+GRID_STEP
    cmp  ecx, (GRID_BEGIN_Y-1+GRID_HEIGHT*GRID_STEP)*65536+(GRID_BEGIN_Y-1+GRID_HEIGHT*GRID_STEP)
     jng @b

    ret


  grid_lines_with_ends:

    mov  eax, 38
    mov  ebx, (GRID_BEGIN_X-1)*65536+(GRID_BEGIN_X-1)
    mov  ecx, (GRID_BEGIN_Y-1-GRID_ENDS_WIDTH)*65536+(GRID_BEGIN_Y-1+GRID_HEIGHT*GRID_STEP+GRID_ENDS_WIDTH)
    mov  edx, [decorations_color]

  @@:
      mcall
    add  ebx, GRID_STEP*65536+GRID_STEP
    cmp  ebx, (GRID_BEGIN_X-1+GRID_WIDTH*GRID_STEP)*65536+(GRID_BEGIN_X-1+GRID_WIDTH*GRID_STEP)
     jng @b

    mov  ebx, (GRID_BEGIN_X-1-GRID_ENDS_WIDTH)*65536+(GRID_BEGIN_X-1+GRID_WIDTH*GRID_STEP+GRID_ENDS_WIDTH)
    mov  ecx, (GRID_BEGIN_Y-1)*65536+(GRID_BEGIN_Y-1)
    
  @@:
      mcall
    add  ecx, GRID_STEP*65536+GRID_STEP
    cmp  ecx, (GRID_BEGIN_Y-1+GRID_HEIGHT*GRID_STEP)*65536+(GRID_BEGIN_Y-1+GRID_HEIGHT*GRID_STEP)
     jng @b

    ret


  grid_lines_with_corners:

      call      grid_lines
      call      corners_outer

    ret


  grid_dots:

    mov  eax, 1
    mov  ebx, GRID_BEGIN_X-1
    mov  ecx, GRID_BEGIN_Y-1
    mov  edx, [decorations_color]

  @@:
      mcall
    add  ebx, GRID_STEP
    cmp  ebx, GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-1
     jng @b
    add  ecx, GRID_STEP
    cmp  ecx, GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-1
     jg  @f
    mov  ebx, GRID_BEGIN_X-1
     jmp @b

  @@:
    ret


  borders_lines:

      mcall     38,(GRID_BEGIN_X-1)*65536+(GRID_BEGIN_X-1),17*65536+263,[decorations_color]
      mcall       ,(GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-1)*65536+(GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-1), ,
      mcall       ,17*65536+523,(GRID_BEGIN_Y-1)*65536+(GRID_BEGIN_Y-1)
      mcall       ,17*65536+523,(GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-1)*65536+(GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-1)

    ret


  borders_lines_with_ends:

      call      borders_lines
      call      corners_outer

    ret


  borders_dots:

    mov  eax, 1
    mov  ebx, GRID_BEGIN_X-1
    mov  ecx, GRID_BEGIN_Y-1
    mov  edx, [decorations_color]
  @@:
      mcall
    add  ebx, GRID_STEP
    cmp  ebx, GRID_BEGIN_X-1+GRID_WIDTH*GRID_STEP
     jng @b

    mov  ebx, GRID_BEGIN_X-1
    mov  ecx, GRID_BEGIN_Y-1+GRID_HEIGHT*GRID_STEP
  @@:
      mcall
    add  ebx, GRID_STEP
    cmp  ebx, GRID_BEGIN_X-1+GRID_WIDTH*GRID_STEP
     jng @b

    mov  ebx, GRID_BEGIN_X-1
    mov  ecx, GRID_BEGIN_Y-1
  @@:
      mcall
    add  ecx, GRID_STEP
    cmp  ecx, GRID_BEGIN_Y-1+GRID_HEIGHT*GRID_STEP
     jng @b

    mov  ebx, GRID_BEGIN_X-1+GRID_WIDTH*GRID_STEP
    mov  ecx, GRID_BEGIN_Y-1
  @@:
      mcall
    add  ecx, GRID_STEP
    cmp  ecx, GRID_BEGIN_Y-1+GRID_HEIGHT*GRID_STEP
     jng @b

    ret


  corners_dots:

      mcall     13,(GRID_BEGIN_X-2)*65536+2,(GRID_BEGIN_Y-2)*65536+2,[decorations_color]
      mcall       ,(GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-1)*65536+2,(GRID_BEGIN_Y-2)*65536+2,
      mcall     13,(GRID_BEGIN_X-2)*65536+2,(GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-1)*65536+2,
      mcall       ,(GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-1)*65536+2,(GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-1)*65536+2,

    ret


  corners_inner:

      mcall     38,(GRID_BEGIN_X-1)*65536+(GRID_BEGIN_X-1+GRID_ENDS_WIDTH),(GRID_BEGIN_Y-1)*65536+(GRID_BEGIN_Y-1),[decorations_color]
      mcall       ,(GRID_BEGIN_X-1)*65536+(GRID_BEGIN_X-1+GRID_ENDS_WIDTH),(GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-1)*65536+(GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-1),
      mcall       ,(GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-GRID_ENDS_WIDTH-1)*65536+(GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-1),(GRID_BEGIN_Y-1)*65536+(GRID_BEGIN_Y-1),
      mcall       ,(GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-GRID_ENDS_WIDTH-1)*65536+(GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-1),(GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-1)*65536+(GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-1),
      mcall       ,(GRID_BEGIN_X-1)*65536+(GRID_BEGIN_X-1),(GRID_BEGIN_Y-1)*65536+(GRID_BEGIN_Y-1+GRID_ENDS_WIDTH),
      mcall       ,(GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-1)*65536+(GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-1),(GRID_BEGIN_Y-1)*65536+(GRID_BEGIN_Y-1+GRID_ENDS_WIDTH),
      mcall       ,(GRID_BEGIN_X-1)*65536+(GRID_BEGIN_X-1),(GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-GRID_ENDS_WIDTH-1)*65536+(GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-1),
      mcall       ,(GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-1)*65536+(GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-1),(GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-GRID_ENDS_WIDTH-1)*65536+(GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-1),

    ret


  corners_outer:

      mcall     38,(GRID_BEGIN_X-1-GRID_ENDS_WIDTH)*65536+(GRID_BEGIN_X-1),(GRID_BEGIN_Y-1)*65536+(GRID_BEGIN_Y-1),[decorations_color]
      mcall       ,(GRID_BEGIN_X-1-GRID_ENDS_WIDTH)*65536+(GRID_BEGIN_X-1),(GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-1)*65536+(GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-1),
      mcall       ,(GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-1)*65536+(GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-1+GRID_ENDS_WIDTH),(GRID_BEGIN_Y-1)*65536+(GRID_BEGIN_Y-1),
      mcall       ,(GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-1)*65536+(GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-1+GRID_ENDS_WIDTH),(GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-1)*65536+(GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-1),
      mcall       ,(GRID_BEGIN_X-1)*65536+(GRID_BEGIN_X-1),(GRID_BEGIN_Y-1-GRID_ENDS_WIDTH)*65536+(GRID_BEGIN_Y-1),
      mcall       ,(GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-1)*65536+(GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-1),(GRID_BEGIN_Y-1-GRID_ENDS_WIDTH)*65536+(GRID_BEGIN_Y-1),
      mcall       ,(GRID_BEGIN_X-1)*65536+(GRID_BEGIN_X-1),(GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-1)*65536+(GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-1+GRID_ENDS_WIDTH),
      mcall       ,(GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-1)*65536+(GRID_BEGIN_X+GRID_WIDTH*GRID_STEP-1),(GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-1)*65536+(GRID_BEGIN_Y+GRID_HEIGHT*GRID_STEP-1+GRID_ENDS_WIDTH),

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

    mov  al,  20
    mul  bl
    mov  bx,  ax
    add  bx,  31
    shl  ebx, 16
    add  ebx, 19

    mov  al,  20
    mul  cl
    mov  cx,  ax
    add  cx,  31
    shl  ecx, 16
    add  ecx, 19
        
      mcall     13

    ret

    ;;---Draw_square-------------------------------------------------------------------------------------------------------
    
    
Draw_menu_esc:
    ;;===Draw_menu_esc=====================================================================================================

      mcall     4,234*65536+TOP_STRINGS,[navigation_strings_color],string_menu_esc
    
    ret
    
    ;;---Draw_menu_esc-----------------------------------------------------------------------------------------------------


Draw_score_string:
    ;;===Draw_score_string=================================================================================================
    
      mcall     4,56*65536+BOTTOM_TOP_STRINGS,[score_string_color],string_score
      
    ret
    
    ;;---Draw_score_string-------------------------------------------------------------------------------------------------
    
    
Draw_score_number:
    ;;===Draw_score_number================================================================================================= 
    
      mcall     47,0x00070000,[score],104*65536+BOTTOM_TOP_STRINGS,[score_number_color],[background_color]
        
    ret
        
    ;;---Draw_score_number-------------------------------------------------------------------------------------------------


Draw_hiscore_string:
    ;;===Draw_hiscore_string===============================================================================================

      mcall     4,376*65536+BOTTOM_TOP_STRINGS,[hiscore_string_color],string_hi_score
    
    ret
    
    ;;---Draw_hiscore_string-----------------------------------------------------------------------------------------------

    
Draw_hiscore_number:
    ;;===Draw_hiscore_number===============================================================================================

      mcall     47,0x00070000,[hi_score],442*65536+BOTTOM_TOP_STRINGS,[hiscore_number_color]
    
    ret
    
    ;;---Draw_hiscore_number-----------------------------------------------------------------------------------------------
    
    
Draw_champion_string:
    ;;===Draw_champion_string==============================================================================================

      mcall     4,376*65536+BOTTOM_BOTTOM_STRINGS,[champion_string_color],string_champion

    ret

    ;;---Draw_champion_string----------------------------------------------------------------------------------------------


Draw_champion_name:
    ;;===Draw_champion_name================================================================================================

      mcall     4,442*65536+BOTTOM_BOTTOM_STRINGS,[champion_name_color],champion_name

    ret

    ;;---Draw_champion_name------------------------------------------------------------------------------------------------


Draw_picture:
    ;;===Draw_picture======================================================================================================
    ;;  in  :
    ;;           al =   picture height (in squares)
    ;;           bh =   number of top square
    ;;          ecx =   pointer to picture data
    ;;          edx =   picture color
    ;;

  .draw:
    xor  bl,  bl
    
  .loop:
    cmp  byte[ecx], 0
     jz  @f
    push eax ebx ecx
      call      Draw_square
    pop  ecx ebx eax
    
  @@:
    inc  ecx
    inc  bl
    cmp  bl,  GRID_WIDTH
     jne .loop
    
    dec  al
    test al,  al
     jnz @f
    ret
  @@:
    inc  bh
     jmp .draw
    

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
    mov  dx,  24
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
    ;;           al =   value on map
    ;;

    push eax
    
    and  eax, 0x0000ffff
    xor  bx,  bx
    mov  bl,  al
    shr  ax,  8
    mov  dx,  24
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
decorations                 db      0x0
number_of_free_dots         dw      0x0

field_map                   db      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

string_score                db      'SCORE :',0
string_hi_score             db      'HI-SCORE :',0
string_player               db      'PLAYER :',0
string_champion             db      'CHAMPION :',0
string_level                db      'LEVEL :',0
string_pause_space          db      'PAUSE - ',0x27,'SPACE',0x27,0
string_resume_space         db      'RESUME - ',0x27,'SPACE',0x27,0
string_menu_esc             db      'MENU - ',0x27,'ESC',0x27,0
string_apply_name_enter     db      'APPLY NAME - ',0x27,'ENTER',0x27,0

champion_name               db      'dunkaist',0x20,0x20,0x20,0x20,0x20,0x20,0x20,0

press_to_start              db      '...PRESS ',0x27,'SPACE',0x27,' OR ',0x27,'ENTER',0x27,' TO START...',0
press_esc_to_exit           db      'PRESS ',0x27,'ESC',0x27,' TO EXIT',0
;press_F2_to_options         db      'PRESS ',0x27,'F2',0x27,' TO OPTIONS',0

snake_dots                  db      3,3, 4,3, 5,3,  522  dup (0)     ; 264 dots
snake_napravlenie           db      3
snake_napravlenie_next      db      3
snake_length_x2             dd      6

eat                         db      0,0

score                       dd      0
hi_score                    dd      777
is_new_record               db      0

action                      db      0

string_congratulations      db      'Congratulations!!! You are the champion!! New hi-score is :',0
string_enter_your_name      db      'Enter your name, please :',0

picture_first_menu_snake    db      1,1,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,1,1,\
                                    1,0,0,0,0,1,1,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,0,1,\
                                    1,1,1,1,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,0,1,1,1,1,\
                                    0,0,0,1,0,1,0,1,1,0,1,1,1,1,0,1,0,1,0,0,1,0,0,0,\
                                    1,1,1,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,1,1,1

picture_first_menu_version  db      0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,1,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,1,1,1,0,1,0,0,1,0,0,0,0,0,0,0,0

picture_pause               db      1,1,1,0,0,0,1,1,0,0,1,0,0,1,0,1,1,1,1,0,1,1,1,1,\
                                    1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,0,0,1,0,0,0,\
                                    1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,0,0,1,0,0,0,\
                                    1,1,1,0,0,1,1,1,1,0,1,0,0,1,0,1,1,1,1,0,1,1,1,0,\
                                    1,0,0,0,0,1,0,0,1,0,1,0,0,1,0,0,0,0,1,0,1,0,0,0,\
                                    1,0,0,0,0,1,0,0,1,0,0,1,1,0,0,1,1,1,1,0,1,1,1,1

picture_game_over           db      0,0,1,1,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,1,1,1,1,0,\
                                    0,1,0,0,1,0,0,1,0,1,0,0,1,1,0,1,1,0,1,0,0,0,0,0,\
                                    1,0,0,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,1,1,1,1,0,0,\
                                    1,0,0,1,0,0,1,1,1,1,1,0,1,0,0,0,1,0,1,0,0,0,0,0,\
                                    0,1,1,1,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,1,1,1,1,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,1,1,1,0,0,1,0,0,0,1,0,1,1,1,1,1,0,1,1,1,1,0,\
                                    0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,0,0,1,0,0,0,1,\
                                    0,1,0,0,0,1,0,1,0,0,0,1,0,1,1,1,1,0,0,1,0,0,0,1,\
                                    0,1,0,0,0,1,0,0,1,0,1,0,0,1,0,0,0,0,0,1,1,1,1,0,\
                                    0,0,1,1,1,0,0,0,0,1,0,0,0,1,1,1,1,1,0,1,0,0,0,1

start_map                   db      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

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

aScore                      db      'Score',0
aHiscore                    db      'Hiscore',0
aChampion_name              db      'Champion_name',0

aPreferences                db      'Preferences',0
aSpeed                      db      'Speed',0
aDecorations                db      'Decorations',0

aColors                     db      'Colors',0
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

edit1 edit_box 100,397,278,0x000000,0x00aa00,0x000000,0x000000,0x80ffdd44,15,hed,mouse_dd,ed_focus,hed_end-hed-1,hed_end-hed-1

hed                         db      '',0
hed_end:
rb  256
ed_buffer   rb 100

mouse_dd                    rd      1

i_end:
cur_dir_path                rb      4096
@PARAMS                     rb      4096
;;---Variables-------------------------------------------------------------------------------------------------------------

rb 4096
stacktop:
d_end: