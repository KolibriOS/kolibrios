include '../../skin.inc'

SKIN_PARAMS \
  height	  = bmp_base.height,\	  ; skin height
  margins	  = [5:1:43:1],\	  ; margins [left:top:right:bottom]
  colors active   = [binner=0x00081d:\	  ; border inner color
		     bouter=0x00081d:\	  ; border outer color
		     bframe=0x4d4c4d],\   ; bframe=0x0054e7],\   ; border frame color
  colors inactive = [binner=0x00081d:\	  ; border inner color
		     bouter=0x00081d:\	  ; border outer color
		     bframe=0x3d3c3d],\   ;1a8acc],\   ; border frame color
  dtp		  = 'myblue.dtp'	  ; dtp colors

SKIN_BUTTONS \
  close    = [-21:3][16:16],\		  ; buttons coordinates
  minimize = [-39:3][16:16]		  ; [left:top][width:height]

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
