;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

include 'me_skin.inc'

SKIN_PARAMS \
  height	  = bmp_base.height,\	  ; skin height
  margins	  = [5:1:43:1],\	  ; margins [left:top:right:bottom]
  colors active   = [binner=0x595246:\	  ; border inner color
		     bouter=0x595246:\	  ; border outer color
		     bframe=0x343a3e],\   ; border frame color
  colors inactive = [binner=0x595246:\	  ; border inner color
		     bouter=0x595246:\	  ; border outer color
		     bframe=0x343a3e],\   ; border frame color
  dtp		  = 'myblue.dtp'	  ; dtp colors

SKIN_BUTTONS \
  close    = [-22:3][17:16],\		  ; buttons coordinates
  minimize = [-40:3][17:16]		  ; [left:top][width:height]

SKIN_BITMAPS \
  left active	= bmp_left,\		  ; skin bitmaps pointers
  left inactive = bmp_left1,\
  oper active	= bmp_oper,\
  oper inactive = bmp_oper1,\
  base active	= bmp_base,\
  base inactive = bmp_base1

BITMAP bmp_left ,'fall_left.bmp'	       ; skin bitmaps
BITMAP bmp_oper ,'fall_oper.bmp'
BITMAP bmp_base ,'fall_base.bmp'
BITMAP bmp_left1,'fall_left.bmp'
BITMAP bmp_oper1,'fall_oper1.bmp'
BITMAP bmp_base1,'fall_base.bmp'
