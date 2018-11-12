; Author Frederic Feret

include '../skin.inc'

SKIN_PARAMS \
  height          = bmp_center1.height,\  ; skin height
  margins         = [6:5:58:3],\          ; margins [left:top:right:bottom]
  colors active   = [binner=0x293455:\    ; border inner
                     bouter=0x000000:\    ; border outer
                     bframe=0x293455],\   ; border middle
  colors inactive = [binner=0x293455:\    ; border inner
                     bouter=0x000000:\    ; border outer
                     bframe=0x293455],\   ; border middle
  dtp             = 'default.dtp'          ; dtp colors

SKIN_BUTTONS \
  close    = [-21:4][16:16],\             ; buttons coordinates
  minimize = [-40:4][16:16]               ; [left:top][width:height]

SKIN_BITMAPS \
  left active   = bmp_left1,\              ; skin bitmaps pointers
  left inactive = bmp_left2,\
  oper active   = bmp_right1,\
  oper inactive = bmp_right2,\
  base active   = bmp_center1,\
  base inactive = bmp_center2

BITMAP bmp_left1,  'left1.bmp'               ; skin bitmaps
BITMAP bmp_left2,  'left1.bmp'
BITMAP bmp_right1, 'right1.bmp'
BITMAP bmp_right2, 'right2.bmp'
BITMAP bmp_center1,'center1.bmp'
BITMAP bmp_center2,'center1.bmp'
