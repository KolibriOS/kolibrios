#ifndef INCLUDE_FONT_H
#define INCLUDE_FONT_H

#ifndef INCLUDE_MATH_H
#include "../lib/math.h"
#endif

#ifndef INCLUDE_IO_H
#include "../lib/io.h"
#endif

:struct __SIZE
{
	word width,height;
	signed offset_x,offset_y;
	float offset_i,w_italic;
	byte text;
	byte TMP_WEIGHT;
};
:struct FONT
{
	__SIZE size;
	byte width,height,offsetLine,r,g,b,weight,italic;
	dword color;
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
	dword text(word x,y;dword text1);
	byte textcenter(word x,y,w,h;dword txt);
	dword getsize(dword text1);
	dword textarea(word x,y;dword text,c);
	byte changeSIZE();
	void PixelRGB(word x,y);
	dword GetPixel(word x,y);
	dword tmp_y,tmp_height,tmp_x;
	byte no_bg_copy;
	dword bg_color;
};
FONT font = 0;

:dword FONT::GetPixel(word x,y)
{
	dword tmp;
	tmp = y*size.width*3;
	tmp += x*3;
	tmp += buffer;
	r = DSBYTE[tmp];
	tmp++;
	g = DSBYTE[tmp];
	tmp++;
	b = DSBYTE[tmp];
}
:void FONT::PixelRGB(dword x,y)
{
	dword tmp;
	tmp = y*size.width*3;
	tmp += x*3;
	tmp += buffer;

	DSBYTE[tmp] = r;
	tmp++;
	DSBYTE[tmp] = g;
	tmp++;
	DSBYTE[tmp] = b;
}
:byte FONT::changeSIZE()
{
	dword TMP_DATA;
	dword ofs;
	byte s;
	IF(size.text<9) size.text = 8;
	s = size.text-8;
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
:byte FONT::textcenter(word x,y,w,h;dword txt)
{
	getsize(txt);
	EDX = w/2;
	ECX = size.width/2;
	EDX -= ECX;
	x += EDX;
	return text(x,y,txt);
}
:dword FONT::getsize(dword text1)
{
	size.height = size.width = 0;
	size.offset_x = size.offset_y = -1;
	IF(size.text)IF(!changeSIZE())return 0;
	WHILE(DSBYTE[text1])
	{
		symbol_size(DSBYTE[text1]);
		text1++;
	}
	$neg size.offset_y
	$neg size.offset_x
	size.height++;
	size.height += size.offset_y;
	size.width += size.offset_x;
	size.width++;
	IF(italic)
	{
		size.w_italic = size.height/3;
		size.offset_i = size.w_italic/size.height;
		size.width += size.w_italic;
		size.w_italic = -size.w_italic;
	}
	return size.width;
}
:byte FONT::symbol_size(byte s)
{
        dword xi,yi;
        dword tmp,_;
        dword iii;
        byte rw=0;
		byte X;
		size.TMP_WEIGHT = math.ceil(size.text/17);
        IF(s==32)
		{
			size.width += width/4;
			IF(weight) size.width+=size.TMP_WEIGHT;
			return;
		}
		IF(s==9)
		{
			size.width += width;
			IF(weight) size.width+=size.TMP_WEIGHT;
			return;
		}
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
							IF(size.height<yi)size.height = yi;
							IF(size.offset_y<0)size.offset_y = yi;
							ELSE IF(yi<size.offset_y)size.offset_y = yi;
							IF(!X) X = xi;
							ELSE IF(X>xi)X = xi;
						}
                        xi++;
                        iii++;
                }
                yi++;
        }
		size.width += rw;
		IF(weight) size.width+=size.TMP_WEIGHT;
		IF(s=='_') size.width--;
		IF(size.offset_x<0)size.offset_x = X;
}
:dword FONT::text(word x,y;dword text1)
{
	signed len=0;
	dword c;
	word _tmp_h;
	c = color;
	IF(!text1)return false;
	IF(size.text)IF(!changeSIZE())return false;
	GetProcessInfo(#Form_SELF_FONTS, SelfInfo); 
	IF(y>Form_SELF_FONTS.cheight) return false;
	IF(x>Form_SELF_FONTS.cwidth) return false;
	AX = c;
	r = AL;
	g = AH;
	c>>=16;
	AX = c;
	b = AL;
	getsize(text1);
	y -= size.offset_y;
	tmp_y = y;
	
	EDX = size.width*size.height*3;
	IF(!buffer_size)
	{
		buffer_size = EDX;
		buffer = malloc(buffer_size);
	}
	ELSE IF(buffer_size<EDX)
	{
		buffer_size = EDX;
		buffer = realloc(buffer,buffer_size);
	}
	IF (no_bg_copy)
	{
		EBX = bg_color;
		EDI = buffer;
		EAX = buffer_size+EDI;
		WHILE (EDI<EAX)
		{
			ESDWORD[EDI] = EBX;
			$add edi,3
		}
	}
	ELSE CopyScreen(buffer,x+Form_SELF_FONTS.left+5,y+Form_SELF_FONTS.top+GetSkinHeight(),size.width,size.height);
	len = size.offset_x;
	WHILE(DSBYTE[text1])
	{
		IF(DSBYTE[text1]=='_') len--;
		len+=symbol(len,DSBYTE[text1]);
		IF(weight)len+=math.ceil(size.text/17);
		text1++;
	}
	_PutImage(x,y,size.width,size.height,buffer);
	return len;
}
:dword FONT::textarea(word x,y;dword text1,c;byte size)
{
	
}

:byte FONT::symbol(signed x;byte s)
{
        dword xi,yi;
        dword tmp,_;
        dword iii;
		float ital = -size.w_italic;
		dword ___x;
		word TMP;
		byte _TMP_WEIGHT;
        byte rw=0;
        IF(s==32)return width/4;
		IF(s==9)return width;
        yi = 0;
        iii = 0;
        tmp = 4*block*s;
        tmp +=data;
        while(yi<height)
        {
                xi = 0;
				TMP = size.offset_y+yi;
                while(xi<width)
                {
					IF(iii%32) _ >>= 1;
					ELSE
					{
							tmp += 4;
							_ = DSDWORD[tmp];
					}
					if(_&1)
					{
							IF(xi>rw)rw=xi;
							___x = x+xi;
							IF(italic)___x+=math.ceil(ital);
							if(___x<Form_SELF_FONTS.cwidth)&&(tmp_y+yi<Form_SELF_FONTS.cheight)
							{
								PixelRGB(___x,TMP);
								_TMP_WEIGHT = size.TMP_WEIGHT;
								WHILE(_TMP_WEIGHT)
								{
									IF(weight) PixelRGB(___x+_TMP_WEIGHT,TMP);
									_TMP_WEIGHT--;
								}
							}
					}
					xi++;
					iii++;
                }
                yi++;
				IF(italic) ital-=size.offset_i;
        }
        return rw;
}
:byte FONT::load(dword path)
{
	dword tmp;
	buffer_size = 0;
	IF(data)free(data);
	if (!io.read(path))
	{
		debug("Error while loading font: ");
		debugln(path);
		return false;
	}
	begin = tmp = data = io.buffer_data;
	size_file = io.FILES_SIZE;
	tmp +=size_file;
	tmp--;
	height = DSBYTE[tmp];
	tmp--;
	width = DSBYTE[tmp];
	block = math.ceil(height*width/32);
	return true;
}

#endif