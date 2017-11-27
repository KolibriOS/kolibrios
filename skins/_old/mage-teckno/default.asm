;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

include 'skin.inc'

SKIN_PARAMS \
  height	  = bmp_base.height,\	  ; skin height
  margins	  = [27:1:69:1],\	  ; margins [left:top:right:bottom]
  colors active   = [binner=0x204059:\	  ; border inner color
		     bouter=0x103049:\	  ; border outer color
		     bframe=0x39667e],\   ; border frame color
  colors inactive = [binner=0x7b8f9e:\	  ; border inner color
		     bouter=0x6b7f8e:\	  ; border outer color
		     bframe=0x7994a6],\   ; border frame color
  dtp		  = 'colour.dtp'	  ; dtp colors

SKIN_BUTTONS \
  close    = [-29:3][22:14],\		  ; buttons coordinates
  minimize = [-55:5][22:14]		  ; [left:top][width:height]

SKIN_BITMAPS \
  left active	= bmp_left,\		  ; skin bitmaps pointers
  left inactive = bmp_left1,\
  oper active	= bmp_oper,\
  oper inactive = bmp_oper1,\
  base active	= bmp_base,\
  base inactive = bmp_base1

BITMAP bmp_left ,'left.bmp'		  ; skin bitmaps
BITMAP bmp_oper ,'oper.bmp'
BITMAP bmp_base ,'base.bmp'
BITMAP bmp_left1,'left_1.bmp'
BITMAP bmp_oper1,'oper_1.bmp'
BITMAP bmp_base1,'base_1.bmp'
