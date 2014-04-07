include 'me_skin.inc'

SKIN_PARAMS \
  height	  = bmp_base.height,\	  ; skin height
  margins	  = [5:1:43:1],\	  ; margins [left:top:right:bottom]
  colors active   = [binner=0x3355dd:\	  ; border inner color
		     bouter=0x1133dd:\	  ; border outer color
		     bframe=0x6688dd],\   ; border frame color
  colors inactive = [binner=0x3355dd:\	  ; border inner color
		     bouter=0x1133dd:\	  ; border outer color
		     bframe=0x6688dd],\   ; border frame color
  dtp		  = 'myblue.dtp'	  ; dtp colors

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