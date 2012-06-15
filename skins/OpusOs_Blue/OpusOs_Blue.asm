include 'my_skin.inc'

SKIN_PARAMS \
  height          = bmp_base.height,\                       ; skin height
  margins         = [8:1:37:1],\                            ; margins [left:top:right:bottom]
  colors active   = [binner=0x4C5E7C:\                      ; border inner color
                     bouter=0x465774:\                      ; border outer color
                     bframe=0x5F769B],\                     ; border frame color
  colors inactive = [binner=0xADAAA3:\                      ; border inner color
                     bouter=0xADAAA3:\                      ; border outer color
                     bframe=0xC8C4BD],\                     ; border frame color
  dtp             = 'OpusOs_Blue.dtp'                       ; dtp colors

SKIN_BUTTONS \
  close    = [-19:4][14:14],\                               ; buttons coordinates
  minimize = [-35:4][14:14]                                 ; [left:top][width:height]

SKIN_BITMAPS \
  left active   = bmp_left,\                                ; skin bitmaps pointers
  left inactive = bmp_left1,\
  oper active   = bmp_oper,\
  oper inactive = bmp_oper1,\
  base active   = bmp_base,\
  base inactive = bmp_base1

BITMAP bmp_left ,'active/left.bmp'                          ; skin bitmaps
BITMAP bmp_oper ,'active/oper.bmp'
BITMAP bmp_base ,'active/base.bmp'
BITMAP bmp_left1,'inactive/left.bmp'
BITMAP bmp_oper1,'inactive/oper.bmp'
BITMAP bmp_base1,'inactive/base.bmp'
