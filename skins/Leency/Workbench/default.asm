include '../../skin.inc'

SKIN_PARAMS \
  height          = bmp_center1.height,\  ; skin height
  margins         = [7:5:43:5],\          ; margins [left:top:right:bottom]
  colors active   = [binner=0x083040:\    ; border inner
                     bouter=0x083040:\    ; border outer
                     bframe=0x588090],\   ; border middle
  colors inactive = [binner=0x34404C:\    ; border inner
                     bouter=0x34404C:\    ; border outer
                     bframe=0x808C98],\   ; border middle
  dtp             = 'default.dtp'          ; dtp colors

SKIN_BUTTONS \
  close    = [-23:1][22:22],\             ; buttons coordinates
  minimize = [-46:1][22:22]               ; [left:top][width:height]

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
