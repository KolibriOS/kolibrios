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
  colors active   = [binner=0x2c2a2c:\	  ; border inner color
		     bouter=0x2c2a2c:\	  ; border outer color
		     bframe=0x343a3e],\   ; border frame color
  colors inactive = [binner=0x2c2a2c:\	  ; border inner color
		     bouter=0x2c2a2c:\	  ; border outer color
		     bframe=0x343a3e],\   ; border frame color
  dtp		  = 'GREEN.dtp' 	 ; dtp colors

SKIN_BUTTONS \
  close    = [-18:5][15:14],\		  ; buttons coordinates
  minimize = [-35:5][15:14]		  ; [left:top][width:height]

SKIN_BITMAPS \
  left active	= bmp_left,\		  ; skin bitmaps pointers
  left inactive = bmp_left1,\
  oper active	= bmp_oper,\
  oper inactive = bmp_oper1,\
  base active	= bmp_base,\
  base inactive = bmp_base1

BITMAP bmp_left ,'left_1.bmp'		    ; skin bitmaps
BITMAP bmp_oper ,'oper_1.bmp'
BITMAP bmp_base ,'base_1.bmp'
BITMAP bmp_left1,'left_2.bmp'
BITMAP bmp_oper1,'oper_2.bmp'
BITMAP bmp_base1,'base_2.bmp'
