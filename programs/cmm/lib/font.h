#ifndef INCLUDE_FONT_H
#define INCLUDE_FONT_H

#ifndef INCLUDE_MATH_H
#include "../lib/math.h"
#endif

#ifndef INCLUDE_FS_H
#include "../lib/obj/fs.h"
#endif

#define DEFAULT_FONT "/sys/fonts/Tahoma.kf"

:struct __OFFSET_FONT
{
	signed x,y;
};
:struct __SIZE
{
	dword width,height;
	__OFFSET_FONT offset;
	float offset_i,w_italic;
	byte text;
	byte TMP_WEIGHT;
};
:struct FONT
{
	__SIZE size;
	int left,top,width,height;
	byte bold,italic,smooth;
	dword bg_color;
	dword color;
	dword file_size;
	dword buffer;
	dword buffer_size;
	word block;
	dword data;
	dword begin;
	byte load(...);
	byte changeSIZE();
	byte symbol(signed x,y;byte s;dword c);
	byte symbol_size(byte s);
	dword getsize(dword text1);
	void apply_smooth();
	int write_center(dword x,y,w,h;dword txt);
	int write(int x,y;dword text1);
	void write_buf(int x,y,w,h, text1);
	void show_buf();
};
FONT font = 0;

:byte FONT::changeSIZE()
{
	dword TMP_DATA;
	dword ofs;
	IF(size.text<9) size.text = 8;
	TMP_DATA = data = begin;
	TMP_DATA +=size.text-8*4;
	ofs = DSDWORD[TMP_DATA];
	IF(ofs==-1)return false;
	data += ofs + 156;
	TMP_DATA = data;
	file_size = DSDWORD[TMP_DATA];
	TMP_DATA = data + file_size;
	height = DSBYTE[TMP_DATA - 1];
	width =  DSBYTE[TMP_DATA - 2];
	block = math.ceil(height*width/32);
	return true;
}
:dword FONT::getsize(dword text1)
{
	size.height = size.width = 0;
	size.offset.x = size.offset.y = -1;
	IF(size.text)IF(!changeSIZE())return 0;
	WHILE(DSBYTE[text1])
	{
		symbol_size(DSBYTE[text1]);
		text1++;
	}
	$neg size.offset.y
	$neg size.offset.x
	size.height += size.offset.y; size.height++;
	size.width += size.offset.x; size.width++;
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
		dword iii = 0;
		byte rw=0;
		byte X;
		size.TMP_WEIGHT = math.ceil(size.text/17);
		IF(s==32)
		{
			size.width += width/4;
			IF(bold) size.width+=size.TMP_WEIGHT;
			return;
		}
		IF(s==9)
		{
			size.width += width;
			IF(bold) size.width+=size.TMP_WEIGHT;
			return;
		}
		s = AnsiToCp866(s);
		tmp = 4*block*s + data;
		for(yi=0; yi<height; yi++)
		{
			for(xi=0; xi<width; xi++)
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
					IF(size.offset.y<0)size.offset.y = yi;
					ELSE IF(yi<size.offset.y)size.offset.y = yi;
					IF(!X) X = xi;
					ELSE IF(X>xi)X = xi;
				}
				iii++;
			}
		}
		size.width += rw;
		IF(bold) size.width+=size.TMP_WEIGHT;
		IF(s=='_') size.width--;
		IF(size.offset.x<0)size.offset.x = X;
}
:byte FONT::symbol(signed x,y;byte s)
{
		dword xi,yi;
		dword iii = 0;
		dword offs;
		float ital = -size.w_italic;
		dword ___x;
		byte rw=0;
		IF(s==32)return width/4;
		IF(s==9)return width;
		s = AnsiToCp866(s);
		EBX = block*s << 2 + data;
		for(yi=0; yi<height; yi++)
		{
			EDI = size.offset.y + yi + y * size.width * 3 + buffer;
			for(xi=0; xi<width; xi++)
			{
				IF(iii%32) $shr ecx,1
				ELSE
				{
						EBX += 4;
						ECX = DSDWORD[EBX];
				}
				IF(ECX&true)
				{
						IF(xi>rw)rw=xi;
						___x = x+xi;
						IF(italic)___x+=math.ceil(ital);
						offs = ___x*3 + EDI;
						DSDWORD[offs] = DSDWORD[offs] & 0xFF000000 | color;
						IF(bold) DSDWORD[offs+3] = DSDWORD[offs+3] & 0xFF000000 | color;
				}
				iii++;
			}
			if (italic) ital-=size.offset_i;
		}
		return rw;
}

byte AnsiToCp866(byte s) {
	IF(s>=128)&&(s<=175)s+=64;
	ELSE IF(s>=224)&&(s<=239)s+=16;
	ELSE IF(s==241)s=184; //yo
	ELSE IF(s==240)s=168; //YO
	return s;
}

inline fastcall dword b24(EBX) { return DSDWORD[EBX] << 8; }
:void FONT::apply_smooth()
{
	dword i,line_w,to;
	line_w = font.size.width * 3;
	to = font.size.height - 1 * line_w + font.buffer - 3;
	for(i=font.buffer; i < to; i+=3)	
	{
		IF(i-font.buffer%line_w +3 == line_w) continue;
		IF(b24(i)==0x000000) && (b24(i+3)!=0x000000) && (b24(i+line_w)!=0x000000) && (b24(i+3+line_w)==0x000000)
		{ 
			ShadowPixel(i+3, 2);
			ShadowPixel(i+line_w, 2);
		}
		ELSE IF(b24(i)!=0x000000) && (b24(i+3)==0x000000) && (b24(i+line_w)==0x000000) && (b24(i+3+line_w)!=0x000000)
		{
			ShadowPixel(i, 2);
			ShadowPixel(i+3+line_w, 2);
		}
	}
}
:byte FONT::load(dword path)
{
	lib_init_fs();
	buffer_size = 0;
	smooth = true;
	IF(data)free(data);
	IF(!fs.read(path)) { debug("Error while loading font: "); debugln(path); return false; }
	begin = data = EAX;
	EBX = begin + ECX;
	height = DSBYTE[EBX-1];
	width = DSBYTE[EBX-2];
	block = math.ceil(height*width/32);
	return true;
}

:int FONT::write_center(dword x,y,w,h;dword txt)
{
	getsize(txt);
	return write(w-size.width/2+x,y,txt);
}

:int FONT::write(int x,y; dword text1)
{
	signed len=0;
	IF(!text1)return false;
	IF(size.text)IF(!changeSIZE())return false;
	left = x;
	getsize(text1);
	y -= size.offset.y;
	top = y;
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
	// Fill background color {
	EBX = bg_color;
	EAX = buffer_size+buffer;
	for (EDI=buffer; EDI<EAX; EDI+=3) ESDWORD[EDI] = EBX;
	// }
	len = size.offset.x;
	WHILE(DSBYTE[text1])
	{
		IF(DSBYTE[text1]=='_') len--;
		len+=symbol(len,0,DSBYTE[text1]);
		IF(bold)len+=math.ceil(size.text/17);
		text1++;
	}
	IF (smooth) apply_smooth();
	show_buf(left,top);
	return len;
}

:void FONT::write_buf(int x,y,w,h; dword text1)
{
	dword new_buffer_size;
	IF(!text1)return;
	IF(size.text)IF(!changeSIZE())return;
	getsize(text1);
	y -= size.offset.y;

	size.width = w;
	size.height = h;

	new_buffer_size = w*h*3;
	IF(buffer_size != w*h*3)
	{
		buffer_size = new_buffer_size; 
		free(buffer);
		buffer = malloc(buffer_size);
		// Fill background color
		EBX = bg_color;
		EAX = buffer_size+buffer;
		for (EDI=buffer; EDI<EAX; EDI+=3) ESDWORD[EDI] = EBX;
	}
	WHILE(DSBYTE[text1])
	{
		x+=symbol(x,y,DSBYTE[text1]);
		IF(bold)x+=math.ceil(size.text/17);
		text1++;
	}
	return;
}

:void FONT::show_buf(dword left1, top1){
	_PutImage(left1,top1,size.width,size.height,buffer);
}




#endif