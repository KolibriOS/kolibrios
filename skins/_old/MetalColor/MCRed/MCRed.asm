include 'skin.inc'

SKIN_PARAMS \
  height          = bmp_base.height,\                       ; skin height
  margins         = [7:2:69:4],\                            ; margins [left:top:right:bottom]
  colors active   = [binner=0x000000:\                      ; border inner color
                     bouter=0x000000:\                      ; border outer color
                     bframe=0x8E262C],\                     ; border frame color
  colors inactive = [binner=0x000000:\                      ; border inner color
                     bouter=0x000000:\                      ; border outer color
                     bframe=0x4E4E4E],\                     ; border frame color
  dtp             = 'MCRed.dtp'                             ; dtp colors

SKIN_BUTTONS \
  close    = [-20:4][15:15],\                               ; buttons coordinates
  minimize = [-37:4][15:15]                                 ; [left:top][width:height]

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
