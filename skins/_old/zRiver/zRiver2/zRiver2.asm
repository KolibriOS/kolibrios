include 'skin.inc'

SKIN_PARAMS \
  height	  = bmp_base.height,\	  ; skin height
  margins	  = [6:1:43:1],\	  ; margins [left:top:right:bottom]
  colors active   = [binner=0x370B00:\	  ; border inner color
		     bouter=0x370B00:\	  ; border outer color
		     bframe=0x4E1100],\   ; border frame color
  colors inactive = [binner=0x4A2921:\	  ; border inner color
		     bouter=0x4A2921:\	  ; border outer color
		     bframe=0x5B2F21],\   ; border frame color
  dtp		  = 'ORANGE.DTP'	 ; dtp colors

SKIN_BUTTONS \
  close    = [-30:3][25:17],\		  ; buttons coordinates
  minimize = [-57:3][25:17]		  ; [left:top][width:height]

SKIN_BITMAPS \
  left active	= bmp_left,\		  ; skin bitmaps pointers
  left inactive = bmp_left1,\
  oper active	= bmp_oper,\
  oper inactive = bmp_oper1,\
  base active	= bmp_base,\
  base inactive = bmp_base1

BITMAP bmp_left ,'active/left.bmp'		 ; skin bitmaps
BITMAP bmp_oper ,'active/oper.bmp'
BITMAP bmp_base ,'active/base.bmp'
BITMAP bmp_left1,'inactive/left.bmp'
BITMAP bmp_oper1,'inactive/oper.bmp'
BITMAP bmp_base1,'inactive/base.bmp'

;================================;
; Created by Rock_maniak_forever ;
;================================;

;AC2600
;A72500
;9B1F00
;8C1C00
;751800
;601500



