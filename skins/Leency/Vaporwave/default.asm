include '../../skin.inc'

SKIN_PARAMS \
  height          = bmp_center1.height,\  ; skin height
  margins         = [6:5:65:3],\          ; margins [left:top:right:bottom]
  colors active   = [binner=0xE8E0EF:\    ; border inner
                     bouter=0x493673:\    ; border outer
                     bframe=0xE8E0EF],\   ; border middle
  colors inactive = [binner=0xE8E0EF:\    ; border inner
                     bouter=0x349FC9:\    ; border outer
                     bframe=0xE8E0EF],\   ; border middle
  dtp             = 'dtp.dtp'          ; dtp colors

SKIN_BUTTONS \
  close    = [-30:1][26:18],\             ; buttons coordinates
  minimize = [-60:1][26:18]               ; [left:top][width:height]

SKIN_BITMAPS \
  left active   = bmp_left1,\              ; skin bitmaps pointers
  left inactive = bmp_left2,\
  oper active   = bmp_right1,\
  oper inactive = bmp_right2,\
  base active   = bmp_center1,\
  base inactive = bmp_center2

BITMAP bmp_left1,  'left1.bmp'               ; skin bitmaps
BITMAP bmp_left2,  'left2.bmp'
BITMAP bmp_right1, 'right1.bmp'
BITMAP bmp_right2, 'right2.bmp'
BITMAP bmp_center1,'center1.bmp'
BITMAP bmp_center2,'center2.bmp'
