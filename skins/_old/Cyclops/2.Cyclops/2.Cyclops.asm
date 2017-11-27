include 'skin.inc'

SKIN_PARAMS \
  height      = bmp_base.height,\           ; skin height
  margins      = [5:1:43:1],\               ; margins [left:top:right:bottom]
  colors active   = [binner=0x00081d:\      ; border inner color
             bouter=0x00081d:\              ; border outer color
             bframe=0xA2A2A1],\             ; border frame color
  colors inactive = [binner=0x00081d:\      ; border inner color
             bouter=0x00081d:\              ; border outer color
             bframe=0x8A8A8A],\             ; border frame color
  dtp          = 'default.dtp'              ; dtp colors

SKIN_BUTTONS \
  close    = [-23:4][17:17],\               ; buttons coordinates
  minimize = [-39:5][15:15]                 ; [left:top][width:height]

SKIN_BITMAPS \
  left active    = bmp_left,\               ; skin bitmaps pointers
  left inactive = bmp_left1,\
  oper active    = bmp_oper,\
  oper inactive = bmp_oper1,\
  base active    = bmp_base,\
  base inactive = bmp_base1

BITMAP bmp_left ,'active/left.bmp'          ; skin bitmaps
BITMAP bmp_oper ,'active/oper.bmp'  
BITMAP bmp_base ,'active/base.bmp'
BITMAP bmp_left1,'inactive/left.bmp'
BITMAP bmp_oper1,'inactive/oper.bmp'
BITMAP bmp_base1,'inactive/base.bmp'

;================================;
; Created by Rock_maniak_forever ;
;================================;

