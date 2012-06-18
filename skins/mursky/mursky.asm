;;=====================================================================
;; dunkaist, Mario | 2010
;;---------------------------------------------------------------------

include 'my_skin.inc'

SKIN_PARAMS \
    height          =   bmp_base.height,    \   ; skin height
    margins         =   [5:1:42:1],         \   ; margins [left:top:right:bottom]
  colors                                    \
    active          =   [                   \
    binner          =   0x95afc6:           \   ; border inner color
    bouter          =   0x607180:           \   ; border outer color
    bframe          =   0xd3e0eb],          \   ; border frame color
  colors                                    \
    inactive        =   [                   \
    binner          =   0x95afc6:           \   ; border inner color
    bouter          =   0x7c92a5:           \   ; border outer color
    bframe          =   0xeaf0f5],          \   ; border frame color
\
  dtp               =   'mursky.dtp'            ; dtp colors

SKIN_BUTTONS \
    close           =   [-23:2][20:20],     \   ; buttons coordinates
    minimize        =   [-43:2][20:20]          ; [left:top][width:height]

SKIN_BITMAPS \
    left active     =   bmp_left,           \   ; skin bitmaps pointers
    left inactive   =   bmp_left1,          \
    oper active     =   bmp_oper,           \
    oper inactive   =   bmp_oper1,          \
    base active     =   bmp_base,           \
    base inactive   =   bmp_base1

BITMAP bmp_left ,'active/left.bmp'       ; skin bitmaps
BITMAP bmp_oper ,'active/oper.bmp'
BITMAP bmp_base ,'active/base.bmp'
BITMAP bmp_left1,'inactive/left.bmp'
BITMAP bmp_oper1,'inactive/oper.bmp'
BITMAP bmp_base1,'inactive/base.bmp'
