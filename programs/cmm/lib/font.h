#ifndef INCLUDE_LABEL_H
#define INCLUDE_LABEL_H

#ifndef INCLUDE_MATH_H
#include "../lib/math.h"
#endif

#ifndef INCLUDE_FS_H
#include "../lib/obj/fs.h"
#endif

#define DEFAULT_FONT "/sys/fonts/Tahoma.kf"

:struct __SIZE
{
	dword width,height;
	signed offset_x, offset_y;
	float offset_i,w_italic;
	byte pt;
	byte TMP_WEIGHT;
};
:struct LABEL
{
	__SIZE size;
	int width,height;
	byte bold,italic,smooth;
	dword color;
	dword font,font_begin;
	word block;
	byte init();
	byte changeSIZE();
	byte symbol();
	byte symbol_size();
	dword getsize();

	dword raw;
	dword raw_size;
	void apply_smooth();
	int write_center();
	int write();
	void write_buf();
	void show_buf();
} label;

:byte LABEL::changeSIZE()
{
	dword file_size;
	dword TMP_DATA;
	dword ofs;
	IF(size.pt<9) size.pt = 8;
	TMP_DATA = font = font_begin;
	TMP_DATA +=size.pt-8*4;
	ofs = DSDWORD[TMP_DATA];
	IF(ofs==-1)return false;
	font += ofs + 156;
	TMP_DATA = font;
	file_size = DSDWORD[TMP_DATA];
	TMP_DATA = font + file_size;
	height = DSBYTE[TMP_DATA - 1];
	width =  DSBYTE[TMP_DATA - 2];
	block = math.ceil(height*width/32);
	return true;
}
:dword LABEL::getsize(dword text1)
{
	size.height = size.width = 0;
	size.offset_x = size.offset_y = -1;
	IF(size.pt)IF(!changeSIZE())return 0;
	WHILE(DSBYTE[text1])
	{
		symbol_size(DSBYTE[text1]);
		text1++;
	}
	$neg size.offset_y
	$neg size.offset_x
	size.height += size.offset_y; size.height++;
	size.width += size.offset_x; size.width++;
	IF(italic)
	{
		size.w_italic = size.height/3;
		size.offset_i = size.w_italic/size.height;
		size.width += size.w_italic;
		size.w_italic = -size.w_italic;
	}
	return size.width;
}
:byte LABEL::symbol_size(byte s)
{
		dword xi,yi;
		dword tmp,_;
		dword iii = 0;
		byte rw=0;
		byte X;
		size.TMP_WEIGHT = math.ceil(size.pt/17);
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
		s = Cp866ToAnsi(s);
		tmp = 4*block*s + font;
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
					IF(size.offset_y<0)size.offset_y = yi;
					ELSE IF(yi<size.offset_y)size.offset_y = yi;
					IF(!X) X = xi;
					ELSE IF(X>xi)X = xi;
				}
				iii++;
			}
		}
		size.width += rw;
		IF(bold) size.width+=size.TMP_WEIGHT;
		IF(s=='_') size.width--;
		IF(size.offset_x<0)size.offset_x = X;
}
:byte LABEL::symbol(signed x,y; byte s; dword image_raw)
{
		dword xi,yi;
		dword iii = 0;
		dword offs;
		float ital = -size.w_italic;
		dword ___x;
		byte rw=0;
		IF(s==32)return width/4;
		IF(s==9)return width;
		s = Cp866ToAnsi(s);
		EBX = block*s << 2 + font;
		for(yi=0; yi<height; yi++)
		{
			EDI = size.offset_y + yi + y * size.width * 3 + image_raw;
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

byte Cp866ToAnsi(byte s) {
	IF(s>=128)&&(s<=175)s+=64;
	ELSE IF(s>=224)&&(s<=239)s+=16;
	ELSE IF(s==241)s=184; //yo
	ELSE IF(s==240)s=168; //YO
	return s;
}

:byte LABEL::init(dword font_path)
{
	lib_init_fs();
	IF(font)free(font);
	IF(!fs.read(font_path)) {
		debug("Error while loading font: "); 
		debugln(font_path); 
		//io.run("/sys/@notify","'Error: Font is not loaded.' -E");
		return false;
	}
	font_begin = font = EAX;
	EBX = font_begin + ECX;
	height = DSBYTE[EBX-1];
	width = DSBYTE[EBX-2];
	block = math.ceil(height*width/32);
	smooth = true;
	return true;
}


/*=====================================================================================
===========================                                 ===========================
===========================               RAW               ===========================
===========================                                 ===========================
=====================================================================================*/


inline fastcall dword b24(EBX) { return DSDWORD[EBX] << 8; }
:void LABEL::apply_smooth()
{
	dword i,line_w,to;
	line_w = size.width * 3;
	to = size.height - 1 * line_w + raw - 3;
	for(i=raw; i < to; i+=3)
	{
		IF(i-raw%line_w +3 == line_w) continue;
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

:int LABEL::write_center(dword x,y,w,h; dword background, color1; byte fontSizePoints; dword txt)
{
	size.pt = fontSizePoints;
	getsize(txt);
	return write(w-size.width/2+x,y, background, color1, fontSizePoints, txt);
}

:int LABEL::write(int x,y; dword background, color1; byte fontSizePoints; dword text1)
{
	signed len=0;
	IF(!text1)return false;
	IF(size.pt)IF(!changeSIZE())return false;
	size.pt = fontSizePoints;
	getsize(text1);
	color = color1;
	y -= size.offset_y;
	EDX = size.width*size.height*3;
	IF(!raw_size)
	{
		raw_size = EDX;
		raw = malloc(raw_size);
	}
	ELSE IF(raw_size<EDX)
	{
		raw_size = EDX;
		raw = realloc(raw,raw_size);
	}
	// Fill background color {
	EBX = background;
	EAX = raw_size+raw;
	for (EDI=raw; EDI<EAX; EDI+=3) ESDWORD[EDI] = EBX;
	// }
	len = size.offset_x;
	WHILE(DSBYTE[text1])
	{
		IF(DSBYTE[text1]=='_') len--;
		len+=symbol(len,0,DSBYTE[text1], raw);
		IF(bold)len+=math.ceil(size.pt/17);
		text1++;
	}
	IF (smooth) apply_smooth();
	show_buf(x,y);
	return len;
}

:void LABEL::write_buf(int x,y,w,h; dword background, color1; byte fontSizePoints; dword text1)
{
	dword new_raw_size;
	IF(!text1)return;
	IF(size.pt)IF(!changeSIZE())return;
	
	size.pt = fontSizePoints;
	getsize(text1);
	y -= size.offset_y;
	color = color1;

	size.width = w;
	size.height = h;

	new_raw_size = w*h*3;
	IF(raw_size != new_raw_size)
	{
		raw_size = new_raw_size; 
		free(raw);
		raw = malloc(raw_size);
		// Fill background color
		EBX = background;
		EAX = raw_size+raw;
		for (EDI=raw; EDI<EAX; EDI+=3) ESDWORD[EDI] = EBX;
	}
	WHILE(DSBYTE[text1])
	{
		x+=symbol(x,y,DSBYTE[text1], raw);
		IF(bold)x+=math.ceil(size.pt/17);
		text1++;
	}
	return;
}

:void LABEL::show_buf(dword x, y){
	_PutImage(x, y, size.width, size.height, raw);
}



#endif