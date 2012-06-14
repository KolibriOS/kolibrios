include 'ME_SKIN.INC'

SKIN_PARAMS \
  height	  = bmp_base.height,\	  ; skin height
  margins	  = [5:1:43:1],\	  ; margins [left:top:right:bottom]
  colors active   = [binner=0x00081d:\	  ; border inner color
		     bouter=0x00081d:\	  ; border outer color
		     bframe=0xDEDFDE],\   ; border frame color
  colors inactive = [binner=0x00081d:\	  ; border inner color
		     bouter=0x00081d:\	  ; border outer color
		     bframe=0xEFEFEF],\   ; border frame color          0x1a8acc
  dtp		  = 'myblue.DTP'	  ; dtp colors

SKIN_BUTTONS \
  close    = [-17:5][13:13],\		  ; buttons coordinates
  minimize = [-33:5][13:13]		  ; [left:top][width:height]

SKIN_BITMAPS \
  left active	= bmp_left,\		  ; skin bitmaps pointers
  left inactive = bmp_left1,\
  oper active	= bmp_oper,\
  oper inactive = bmp_oper1,\
  base active	= bmp_base,\
  base inactive = bmp_base1

BITMAP bmp_left ,'left1.BMP'		  ; skin bitmaps
BITMAP bmp_oper ,'right1.BMP'
BITMAP bmp_base ,'center1.BMP'
BITMAP bmp_left1,'left2.BMP'
BITMAP bmp_oper1,'right2.BMP'
BITMAP bmp_base1,'center2.BMP'
