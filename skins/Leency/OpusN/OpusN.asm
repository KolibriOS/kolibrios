include 'my_skin.inc'

SKIN_PARAMS \
  height          = bmp_base.height,\                       ; skin height
  margins         = [8:3:10:2],\                            ; margins [left:top:right:bottom]
  colors active   = [binner=0xDCD6CB:\                      ; border inner color
                     bouter=0x948B7B:\                      ; border outer color
                     bframe=0xDCD6CB],\                     ; border frame color
  colors inactive = [binner=0xDCD6CB:\                      ; border inner color
                     bouter=0x948B7B:\                      ; border outer color
                     bframe=0xDCD6CB],\                     ; border frame color
  dtp             = 'OpusN.dtp'                            ; dtp colors

SKIN_BUTTONS \
  close    = [-20:6][14:14],\                               ; buttons coordinates
  minimize = [-39:6][14:14]                                 ; [left:top][width:height]

SKIN_BITMAPS \
  left active   = bmp_left,\                                ; skin bitmaps pointers
  left inactive = bmp_left1,\
  oper active   = bmp_oper,\
  oper inactive = bmp_oper1,\
  base active   = bmp_base,\
  base inactive = bmp_base1

BITMAP bmp_left ,'a_left.bmp'                          ; skin bitmaps
BITMAP bmp_oper ,'a_oper.bmp'
BITMAP bmp_base ,'a_base.bmp'
BITMAP bmp_left1,'i_left.bmp'
BITMAP bmp_oper1,'i_oper.bmp'
BITMAP bmp_base1,'i_base.bmp'
