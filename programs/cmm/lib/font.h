#ifndef INCLUDE_FONT_H
#define INCLUDE_FONT_H

#ifndef INCLUDE_MATH_H
#include "../lib/math.h"
#endif

#ifndef INCLUDE_IO_H
#include "../lib/io.h"
#endif

:struct FONT
{
	byte width,height,offsetLine;
	word block;
	dword data;
	dword size_file;
	byte load(...);
	byte symbol(word x,y;byte s;dword c);
	dword text(word x,y;dword text,c);
	dword textarea(word x,y;dword text,c);
};
FONT font = 0;
:dword FONT::text(word x,y;dword text1,c)
{
	dword len=0;
	WHILE(DSBYTE[text1])
	{
		len += symbol(x+len,y,DSBYTE[text1],c);
		text1++;
	}
	return len;
}
:dword FONT::textarea(word x,y;dword text1,c)
{
	dword len=0;
	WHILE(DSBYTE[text1])
	{
		IF(DSBYTE[text1]=='\r'){ y+=height; len=0;}
		ELSE	len += symbol(x+len,y,DSBYTE[text1],c);
		text1++;
	}
	return len;
}
:byte FONT::symbol(signed x,y;byte s;dword c)
{
	dword xi,yi;
	dword tmp,_;
	dword iii;
	byte rw=0;
	IF(offsetLine)y+=offsetLine;
	IF(s==32)return width/4;
	yi = 0;
	iii = 0;
	tmp = 4*block*s;
	tmp +=data;
	while(yi<height)
	{
		xi = 0;
		WHILE(xi<width)
		{
			IF(!(iii%32))
			{
				tmp += 4;
				_ = DSDWORD[tmp];
			}
			ELSE _ >>= 1;
			IF(_&1)
			{
				IF(xi>rw)rw=xi;
				PutPixel(x+xi,y+yi,c);
			}
			xi++;
			iii++;
		}
		yi++;
	}
	return rw;
}
:byte FONT::load(dword path)
{
	dword tmp;
	IF(data)free(data);
	tmp = io.read(path);
	data = tmp;
	size_file = io.FILES_SIZE;
	tmp +=size_file;
	tmp--;
	height = DSBYTE[tmp];
	tmp--;
	width = DSBYTE[tmp];
	block = math.ceil(height*width/32);
}

#endif