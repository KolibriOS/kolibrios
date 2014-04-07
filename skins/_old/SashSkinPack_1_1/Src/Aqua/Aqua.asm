include 'me_skin.inc'

SKIN_PARAMS \
  height	  = bmp_base.height,\	  ; skin height
  margins	  = [5:1:43:1],\	  ; margins [left:top:right:bottom]
  colors active   = [binner=0x49b2d5:\	  ; border inner color
		     bouter=0x4fc0e5:\	  ; border outer color
		     bframe=0x2cb9e9],\   ; border frame color
  colors inactive = [binner=0x47899f:\	  ; border inner color
		     bouter=0x47899f:\	  ; border outer color
		     bframe=0x47899f],\   ; border frame color
  dtp		  = 'colors.dtp'	  ; dtp colors

SKIN_BUTTONS \
  close    = [-21:0][21:22],\		  ; buttons coordinates
  minimize = [-42:0][21:22]		  ; [left:top][width:height]

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
