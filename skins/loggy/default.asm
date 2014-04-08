include 'loggy.inc'                          ; Color Table
include 'parse.inc'

SKIN_PARAMS \
  height          = bmp_center1.height,\     ; skin height
  margins         = [5:1:43:1],\             ; margins [left:top:right:bottom]
  colors active   = [binner=win_border:\
                     bouter=win_frame:\
                     bframe=win_face],\
  colors inactive = [binner=win_inborder:\
                     bouter=win_inframe:\
                     bframe=win_inface],\
  dtp             = 'dtp.dtp'                ; dtp colors

SKIN_BUTTONS \
  close    = [-20:3][14:14],\                ; buttons coordinates
  minimize = [-37:3][14:14]                  ; [left:top][width:height]

SKIN_BITMAPS \
  left active   = bmp_left1,\                ; skin bitmaps pointers
  left inactive = bmp_left2,\
  oper active   = bmp_right1,\
  oper inactive = bmp_right2,\
  base active   = bmp_center1,\
  base inactive = bmp_center2

BITMAP bmp_left1,  'pic_left_a.bmp'          ; skin bitmaps
BITMAP bmp_left2,  'pic_left_b.bmp'
BITMAP bmp_right1, 'pic_right_a.bmp'
BITMAP bmp_right2, 'pic_right_b.bmp'
BITMAP bmp_center1,'pic_center_a.bmp'
BITMAP bmp_center2,'pic_center_b.bmp'
