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
	byte width,height,offsetLine,r,g,b;
	word width_buffer;
	dword file_size;
	dword buffer;
	dword buffer_size;
	word block;
	dword data;
	dword begin;
	dword size_file;
	byte load(...);
	byte symbol(word x;byte s;dword c);
	byte symbol_size(byte s);
	dword text(word x,y;dword text1,c;byte size);
	dword text_width(dword text1;byte size);
	dword textarea(word x,y;dword text,c;byte size);
	byte changeSIZE(byte size);
	void PixelRGB(word x,y);
	dword tmp_y,tmp_height,tmp_x;
	byte no_bg_copy;
	dword bg_color;
};
FONT font = 0;

:void FONT::PixelRGB(dword x,y)
{
	dword tmp;
	tmp = y*width_buffer*3;
	tmp += x*3;
	tmp += buffer;

	DSBYTE[tmp] = r;
	tmp++;
	DSBYTE[tmp] = g;
	tmp++;
	DSBYTE[tmp] = b;
}
:byte FONT::changeSIZE(byte size)
{
	dword TMP_DATA;
	dword ofs;
	byte s;
	IF(size<9) size = 8;
	IF(size>45)size = 45;
		s = size-8;
		data = begin;
		TMP_DATA = data;
		TMP_DATA +=s*4;
		ofs = DSDWORD[TMP_DATA];
		IF(ofs==-1)return false;
		data += ofs;
		data += 156;
		TMP_DATA = data;
		file_size = DSDWORD[TMP_DATA];
		TMP_DATA += file_size;
		TMP_DATA--;
		height = DSBYTE[TMP_DATA];
		TMP_DATA--;
		width =  DSBYTE[TMP_DATA];
		block = math.ceil(height*width/32);
		return true;
}
:proc_info Form_SELF_FONTS;

:dword FONT::text_width(dword text1;byte size)
{
	dword len=0;
	IF(size)IF(!changeSIZE(size))return 0;
	WHILE(DSBYTE[text1])
	{
		len += symbol_size(DSBYTE[text1]);
		text1++;
	}
	return len;
}
:byte FONT::symbol_size(byte s)
{
        dword xi,yi;
        dword tmp,_;
        dword iii;
        byte rw=0;
        IF(s==32)return width/4;
		IF(s==9)return width;
        yi = 0;
        iii = 0;
        tmp = 4*block*s;
        tmp +=data;
        WHILE(yi<height)
        {
                xi = 0;
                WHILE(xi<width)
                {
                        IF(iii%32) _ >>= 1;
						ELSE
						{
                                tmp += 4;
                                _ = DSDWORD[tmp];
                        }
                        IF(_&1) IF(xi>rw)rw=xi;
                        xi++;
                        iii++;
                }
                yi++;
        }
        return rw;
}
:dword FONT::text(word x,y;dword text1,c;byte size)
{
	signed len=0;
	IF(size)IF(!changeSIZE(size))return 0;
	GetProcessInfo(#Form_SELF_FONTS, SelfInfo); 
	IF(y>Form_SELF_FONTS.cheight) return 0;
	IF(x>Form_SELF_FONTS.cwidth) return 0;
	tmp_y = y;
	AX = c;
	r = AL;
	g = AH;
	c>>=16;
	AX = c;
	b = AL;
	width_buffer = text_width(text1,size);
	//width_buffer *= strlen(text1);
	IF(!buffer_size)
	{
		buffer_size = width_buffer*height*3;
		buffer = malloc(buffer_size);
	}
	ELSE IF(buffer_size<width_buffer*height*3)
	{
		buffer_size = width_buffer*height*3;
		buffer = realloc(buffer,buffer_size);
	}
	IF (no_bg_copy)
	{
		EBX = bg_color;
		EAX = buffer_size+buffer;
		EDI = buffer;
		WHILE (EDI<EAX)
		{
			ESDWORD[EDI] = EBX;
			$add edi,3
		}
	}
	ELSE CopyScreen(buffer,x+Form_SELF_FONTS.left+5,y+Form_SELF_FONTS.top+GetSkinHeight(),width_buffer,height);
	//width_buffer = text_width(text1);
	WHILE(DSBYTE[text1])
	{
		symbol(len,DSBYTE[text1],c);
		len+=EAX;
		text1++;
	}
	_PutImage(x,y,width_buffer,height,buffer);
	return len;
}
:dword FONT::textarea(word x,y;dword text1,c;byte size)
{
	
}

:byte FONT::symbol(signed x;byte s;dword c)
{
        dword xi,yi;
        dword tmp,_;
        dword iii;
		dword ___x;
        byte rw=0;
		x -= 2;
        IF(s==32)return width/4;
		IF(s==9)return width;
        yi = 0;
        iii = 0;
        tmp = 4*block*s;
        tmp +=data;
        while(yi<height)
        {
                xi = 0;
                WHILE(xi<width)
                {
                        IF(iii%32) _ >>= 1;
						ELSE
						{
                                tmp += 4;
                                _ = DSDWORD[tmp];
                        }
                        IF(_&1)
                        {
                                IF(xi>rw)rw=xi;
								___x = x+xi;
								IF(___x<Form_SELF_FONTS.cwidth)&&(tmp_y+yi<Form_SELF_FONTS.cheight)PixelRGB(___x,yi);
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
	buffer_size = 0;
	IF(data)free(data);
	if (io.read(path)!=0)
	{
		debug("Error while loading font: ");
		debugln(path);
		return 0;
	}
	tmp = io.buffer_data;
	data = tmp;
	begin = data;
	size_file = io.FILES_SIZE;
	tmp +=size_file;
	tmp--;
	height = DSBYTE[tmp];
	tmp--;
	width = DSBYTE[tmp];
	block = math.ceil(height*width/32);
}

#endif