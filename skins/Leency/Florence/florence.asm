;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

include '../../skin.inc'

SKIN_PARAMS \
  height          = bmp_base.height,\         ; skin height
  margins         = [5:1:58:1],\	          ; margins [left:top:right:bottom]
  colors active   = [binner=0x626262:bouter=0x131313:bframe=0x131313],\
  colors inactive = [binner=0x626262:bouter=0x131313:bframe=0x2C2C2C],\
  dtp             = 'col.dtp'

SKIN_BUTTONS \
  close    = [-29:1][24:18],\		      ; buttons coordinates
  minimize = [-56:1][22:18]		          ; [left:top][width:height]

SKIN_BITMAPS \
  left active	= bmp_left,\
  left inactive = bmp_left1,\
  oper active	= bmp_oper,\
  oper inactive = bmp_oper1,\
  base active	= bmp_base,\
  base inactive = bmp_base1

BITMAP bmp_left ,'la.bmp'
BITMAP bmp_oper ,'ra.bmp'
BITMAP bmp_base ,'ca.bmp'
BITMAP bmp_left1,'li.bmp'
BITMAP bmp_oper1,'ri.bmp'
BITMAP bmp_base1,'ci.bmp'
