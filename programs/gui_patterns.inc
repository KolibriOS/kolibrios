macro DrawBar  x, y, width, height, color 
{
	mcall 13, (x) shl 16 + (width), (y) shl 16 + (height), color
}

macro DrawRectangle  x, y, w, h, color 
{
	DrawBar x,y,w,1,color
	DrawBar x,y+h,w,1
	DrawBar x,y,1,h
	DrawBar x+w,y,1,h+1
}

macro DrawRectangle3D  x, y, w, h, color1, color2
{
	DrawBar x,y,w,1,color1
	DrawBar x,y,1,h
	DrawBar x,y+h,w,1,color2
	DrawBar x+w,y,1,h+1
}

macro DrawWideRectangle  x, y, w, h, boder, color
{
	DrawBar x, y, w, boder, color
	DrawBar x, y+h-boder, w, boder
	DrawBar x, y+boder, boder, h-boder-boder
	DrawBar x+w-boder, y+boder, boder, h-boder-boder
}

macro WriteText  x, y, fontType, color, text
{
	mcall 4, (x) shl 16 + (y), (fontType) shl 24 + (color), text
}

macro WriteTextBold  x, y, fontType, color, text
{
	mcall 4, (x) shl 16 + (y), (fontType) shl 24 + (color), text
	add ebx,1 shl 16
	mcall
}

BT_DEL     equ 0x80000000
BT_HIDE    equ 0x40000000
BT_NOFRAME equ 0x20000000

macro DefineButton  x, y, w, h, id, background_color
{
	mcall 8, (x) shl 16 + (w), (y) shl 16 + (h), id, background_color
}
