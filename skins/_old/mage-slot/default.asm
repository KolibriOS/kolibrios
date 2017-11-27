;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

include 'skin.inc'

SKIN_PARAMS \
  height          = bmp_base.height,\     ; skin height
  margins         = [12:1:43:1],\          ; margins [left:top:right:bottom]
  colors active   = [binner=0x00081d:\    ; border inner color
                     bouter=0x00081d:\    ; border outer color
                     bframe=0x4269A4],\   ; border frame color
  colors inactive = [binner=0x00081d:\    ; border inner color
                     bouter=0x00081d:\    ; border outer color
                     bframe=0x607492],\   ; border frame color
  dtp             = 'colour.dtp'           ; dtp colors

SKIN_BUTTONS \
  close    = [-18:4][12:12],\             ; buttons coordinates
  minimize = [-34:6][12:12]               ; [left:top][width:height]

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
