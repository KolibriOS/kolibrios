include 'me_skin.inc'

SKIN_PARAMS \
  height	  = bmp_base.height,\	  ; skin height
  margins	  = [5:1:43:1],\	  ; margins [left:top:right:bottom]
  colors active   = [binner=0xffffff:\	  ; border inner color
		     bouter=0x6e6e6e:\	  ; border outer color
		     bframe=0x7d7d7d],\   ; border frame color
  colors inactive = [binner=0xffffff:\	  ; border inner color
		     bouter=0x6e6e6e:\	  ; border outer color
		     bframe=0x7d7d7d],\   ; border frame color
  dtp		  = 'gray.dtp'	  ; dtp colors

SKIN_BUTTONS \
  close    = [-21:3][16:16],\		  ; buttons coordinates
  minimize = [-39:3][16:16]		  ; [left:top][width:height]

SKIN_BITMAPS \
  left active	= bmp_left,\		  ; skin bitmaps pointers
  left inactive = bmp_left,\
  oper active	= bmp_oper,\
  oper inactive = bmp_oper,\
  base active	= bmp_base,\
  base inactive = bmp_base

BITMAP bmp_left ,'left.bmp'		  ; skin bitmaps
BITMAP bmp_oper ,'oper.bmp'
BITMAP bmp_base ,'base.bmp'