include 'me_skin.inc'

SKIN_PARAMS \
  height	  = bmp_base.height,\	  ; skin height
  margins	  = [5:1:43:1],\	  ; margins [left:top:right:bottom]
  colors active   = [binner=0x7B0F6B:\	  ; border inner color
		     bouter=0x7B0F6B:\	  ; border outer color
		     bframe=0x981084],\   ; border frame color
  colors inactive = [binner=0x7B0F6B:\	  ; border inner color
		     bouter=0x7B0F6B:\	  ; border outer color
		     bframe=0x8B5784],\   ; border frame color
  dtp		  = 'PINK.DTP'	  ; dtp colors

SKIN_BUTTONS \
  close    = [-33:5][26:14],\		  ; buttons coordinates
  minimize = [-52:5][20:14]		  ; [left:top][width:height]

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