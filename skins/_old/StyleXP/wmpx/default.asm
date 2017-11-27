;SKIN (.SKN) - COMPILE WITH FASM

include 'skin.inc'

SKIN_PARAMS \
  height          = bmp_base.height,\     ; skin height
  margins         = [5:1:44:1],\          ; margins [left:top:right:bottom]
  colors active   = [binner=0x6B7DA6:\    ; border inner color
                     bouter=0x6B7DA6:\    ; border outer color
                     bframe=0x7D95C1],\   ; border frame color
  colors inactive = [binner=0x97A4C1:\    ; border inner color
                     bouter=0x97A4C1:\    ; border outer color
                     bframe=0xA4B5D3],\   ; border frame color
  dtp             = 'default.dtp'          ; dtp colors

SKIN_BUTTONS \
  close    = [-21:3][16:16],\             ; buttons coordinates
  minimize = [-39:3][16:16]               ; [left:top][width:height]

SKIN_BITMAPS \
  left active   = bmp_left,\              ; skin bitmaps pointers
  left inactive = bmp_left1,\
  oper active   = bmp_oper,\
  oper inactive = bmp_oper1,\
  base active   = bmp_base,\
  base inactive = bmp_base1

BITMAP bmp_left ,'left.bmp'               ; skin bitmaps
BITMAP bmp_oper ,'oper.bmp'
BITMAP bmp_base ,'base.bmp'
BITMAP bmp_left1,'left_1.bmp'
BITMAP bmp_oper1,'oper_1.bmp'
BITMAP bmp_base1,'base_1.bmp'
