include 'skin.inc'

SKIN_PARAMS \
  height          = bmp_base.height,\                       ; skin height
  margins         = [8:3:10:2],\                            ; margins [left:top:right:bottom]
  colors active   = [binner=0xDCD6CB:\                      ; border inner color
                     bouter=0x948B7B:\                      ; border outer color
                     bframe=0xDCD6CB],\                     ; border frame color
  colors inactive = [binner=0xDCD6CB:\                      ; border inner color
                     bouter=0x948B7B:\                      ; border outer color
                     bframe=0xDCD6CB],\                     ; border frame color
  dtp             = 'Opus_D.dtp'                            ; dtp colors

SKIN_BUTTONS \
  close    = [-21:5][15:15],\                               ; buttons coordinates
  minimize = [-40:5][15:15]                                 ; [left:top][width:height]

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
