include 'skin.inc'

SKIN_PARAMS \
  height	  = bmp_base.height,\	  ; skin height
  margins	  = [5:1:43:1],\	  ; margins [left:top:right:bottom]
  colors active   = [binner=0x95A767:\	  ; border inner color
		     bouter=0x9DB17C:\	  ; border outer color
		     bframe=0xBBCA8F],\   ; border frame color
  colors inactive = [binner=0x8A9174:\	  ; border inner color
		     bouter=0x8E947C:\	  ; border outer color
		     bframe=0x989C89],\   ; border frame color
  dtp		  = 'WinXP_olive_green.dtp'	; dtp colors

SKIN_BUTTONS \
  close    = [-22:5][17:17],\		  ; buttons coordinates
  minimize = [-40:5][17:17]		  ; [left:top][width:height]

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
