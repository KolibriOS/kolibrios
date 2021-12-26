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


