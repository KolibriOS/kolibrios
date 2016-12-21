#ifndef INCLUDE_LABEL_H
#define INCLUDE_LABEL_H

#ifndef INCLUDE_MATH_H
#include "../lib/math.h"
#endif

#ifndef INCLUDE_IO_H
#include "../lib/io.h"
#endif

#include "../lib/patterns/rgb.h"


#define DEFAULT_FONT "/sys/fonts/Tahoma.kf"

:struct __SIZE
{
	dword width,height;
	signed offset_x, offset_y;
	byte pt;
};
:struct LABEL
{
	__SIZE size;
	int width,height;
	byte bold,smooth;
	dword color, background;
	dword font,font_begin;
	word block;
	dword raw;
	dword raw_size;

	byte init();
	bool changeSIZE();
	byte symbol();
	byte symbol_size();
	dword getsize();

	void ApplySmooth();
	int WriteIntoWindow();
	int WriteIntoWindowCenter();
	void WriteIntoBuffer();
	void ShowBuffer();
} label;

:bool LABEL::changeSIZE()
{
	dword file_size;
	dword ofs;
	if(size.pt<9) size.pt = 9;
	font = font_begin;
	ofs = DSDWORD[calc(size.pt-8<<2+font_begin)];
	if(ofs==-1)return false;
	font += ofs + 156;
	file_size = DSDWORD[calc(font)];
	height = DSBYTE[calc(font+file_size) - 1];
	width =  DSBYTE[calc(font+file_size) - 2];
	block = math.ceil(height*width/32);
	return true;
}
:dword LABEL::getsize(byte fontSizePoints, dword text1)
{
	size.height = size.width = 0;
	size.offset_x = size.offset_y = -1;
	size.pt = fontSizePoints;
	if(size.pt)if(!changeSIZE())return 0;
	WHILE(DSBYTE[text1])
	{
		symbol_size(DSBYTE[text1]);
		text1++;
	}
	$neg size.offset_y
	$neg size.offset_x
	size.height += size.offset_y+1;
	size.width += size.offset_x+1;
	return size.width;
}
:byte LABEL::symbol_size(byte s)
{
	//return symbol_size(s);
	dword xi,yi;
	dword tmp,_;
	dword iii = 0;
	byte rw=0;
	byte X;
	if(bold) size.width+=math.ceil(size.pt/17);
	if(s==32)
	{
		size.width += width/4;
		return;
	}
	if(s==9)
	{
		size.width += width;
		return;
	}
	s = Cp866ToAnsi(s);
	tmp = block*s << 2 + font;
	for(yi=0; yi<height; yi++)
	{
		for(xi=0; xi<width; xi++)
		{
			if(iii%32) _ >>= 1;
			else
			{
				tmp += 4;
				_ = DSDWORD[tmp];
			}
			if(_&1)
			{
				if(xi>rw)rw=xi;
				if(size.height<yi)size.height = yi;
				if(size.offset_y<0)size.offset_y = yi;
				else if(yi<size.offset_y)size.offset_y = yi;
				if(!X) X = xi;
				else if(X>xi)X = xi;
			}
			iii++;
		}
	}
	size.width += rw;
	if(size.offset_x<0)size.offset_x = X;
}
:byte LABEL::symbol(signed x,y; byte s; dword image_raw)
{
	dword xi,yi;
	dword iii = 0;
	dword offs;
	byte rw=0;
	if(s==32)return width/4;
	if(s==9)return width;
	s = Cp866ToAnsi(s);
	EBX = block*s << 2 + font;
	for(yi=0; yi<height; yi++)
	{
		EDI = size.offset_y + yi + y * size.width * 3 + image_raw;
		for(xi=0; xi<width; xi++)
		{
			if(iii%32) $shr ecx,1
			else
			{
					EBX += 4;
					ECX = DSDWORD[EBX];
			}
			if(ECX&true)
			{
					if(xi>rw)rw=xi;
					offs = x + xi *3 + EDI;
					DSDWORD[offs] = DSDWORD[offs] & 0xFF000000 | color;
					if(bold) DSDWORD[offs+3] = DSDWORD[offs+3] & 0xFF000000 | color;
			}
			iii++;
		}
	}
	return rw;
}

inline fastcall Cp866ToAnsi(AL) {
	if (AL>=128)&&(AL<=175) return AL+64;
	if (AL>=224)&&(AL<=239) return AL+16;
	if (AL==241) return 184; //e ruAL with dotAL (yo)
	if (AL==240) return 168; //E ruAL with dotAL (yo)
	if (AL==242) return 'E'; //E ukr (ye)
	if (AL==243) return 186; //e ukr (ye)
	if (AL==244) return 'I'; //I ukr (yi)
	if (AL==245) return 191; //i ukr (yi)
	return AL;
}

:byte LABEL::init(dword font_path)
{
	IO label_io;
	if(font)free(font);
	label_io.read(font_path);
	if(!EAX) {
		debugln(font_path);
		label_io.run("/sys/@notify", "'Error: KFONT is not loaded.' -E"); 
		return false;
	}
	font_begin = label_io.buffer_data;
	changeSIZE();
	smooth = true;
	return true;
}


/*=====================================================================================
===========================                                 ===========================
===========================               RAW               ===========================
===========================                                 ===========================
=====================================================================================*/


inline fastcall dword b24(EAX) { return DSDWORD[EAX] & 0x00FFFFFF; }
:void LABEL::ApplySmooth()
{
	dword i,line_w,to,dark_background;
	line_w = size.width * 3;
	to = size.height - 1 * line_w + raw - 3;
	for(i=raw; i < to; i+=3)
	{
		if(i-raw%line_w +3 == line_w) continue;
		// pixels position, where b - black, w - write
		// bw
		// wb
		if(b24(i)!=background) && (b24(i+3)==background) && (b24(i+line_w)==background) && (b24(i+3+line_w)!=background)
		{
			dark_background = MixColors(background,b24(i),210);
			DSDWORD[i+3] = DSDWORD[i+3] & 0xFF000000 | dark_background;
			DSDWORD[i+line_w] = DSDWORD[i+line_w] & 0xFF000000 | dark_background;			
		}
		// wb
		// bw
		else if(b24(i)==background) && (b24(i+3)!=background) && (b24(i+line_w)!=background) && (b24(i+3+line_w)==background)
		{
			dark_background = MixColors(background,b24(i+3),210);
			DSDWORD[i] = DSDWORD[i] & 0xFF000000 | dark_background;
			DSDWORD[i+3+line_w] = DSDWORD[i+3+line_w] & 0xFF000000 | dark_background;
		}
	}
}

:void LABEL::WriteIntoBuffer(int x,y,w,h; dword _background, _color; byte fontSizePoints; dword text1)
{
	dword new_raw_size;
	if(!text1)return;
	if(size.pt)if(!changeSIZE())return;
	
	if (size.pt != fontSizePoints) {
		getsize(fontSizePoints, text1);
		y -= size.offset_y;
	}
	color = _color;
	background = _background;

	size.width = w;
	size.height = h;

	new_raw_size = w*h*3;
	if(raw_size != new_raw_size)
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
		if(bold)x+=math.ceil(size.pt/17);
		text1++;
	}
	return;
}

:int LABEL::WriteIntoWindow(int x,y; dword _background, _color; byte fontSizePoints; dword text1)
{
	if(!text1)return 0;
	getsize(fontSizePoints, text1);
	raw_size = NULL;
	WriteIntoBuffer(0, -size.offset_y, size.width-size.offset_x, 
		size.height-size.offset_y, _background, _color, fontSizePoints, text1);
	if (smooth) ApplySmooth();
	ShowBuffer(x,y);
	return size.offset_x + size.width;
}

:int LABEL::WriteIntoWindowCenter(dword x,y,w,h; dword _background, _color; byte fontSizePoints; dword text1)
{
	getsize(fontSizePoints, text1);
	return WriteIntoWindow(w-size.width/2+x,y, _background, _color, fontSizePoints, text1);
}

:void LABEL::ShowBuffer(dword x, y){
	_PutImage(x, y, size.width, size.height, raw);
}



#endif