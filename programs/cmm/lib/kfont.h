// Сам шрифт представляет.

// Голова:
// [2 байта символы:KF]
// [4 байта:указатель на название шрифта]
// [1 байт:размер массива указателей на размеры шрифтов, указатель 4 байта, т.е. размер = размер массива*4]
// [размер массива*4 байт:указатели..]

// Тело файла:
// [4 байта:масштаб ширина][4 байта:масштаб высота][255*4 байт:указатели на символы][масштаб ширина*масштаб высота байт: данные символов..]

// Конец:
// [Название шрифта:"Times New Roman"]


#ifndef INCLUDE_KFONT_H
#define INCLUDE_KFONT_H

#ifndef INCLUDE_MATH_H
#include "../lib/math.h"
#endif

#ifndef INCLUDE_IO_H
#include "../lib/io.h"
#endif

#include "../lib/patterns/rgb.h"


#define DEFAULT_FONT "/sys/fonts/Tahoma.kf"

#ifndef KFONT_BPP
#define KFONT_BPP 4
#endif

int kfont_char_width[255];

:struct __SIZE
{
	dword width,height;
	signed offset_x, offset_y;
	byte pt;
};
:struct KFONT
{
	__SIZE size;
	int width,height;
	byte bold,smooth;
	dword color, background;
	dword font,font_begin;
	word block;
	dword raw;
	dword raw_size;

	bool init();
	bool changeSIZE();
	byte symbol();
	byte symbol_size();
	dword getsize();
	int get_label_width();

	void ApplySmooth();
	int WriteIntoWindow();
	int WriteIntoWindowCenter();
	void WriteIntoBuffer();
	void ShowBuffer();
	void ShowBufferPart();
} kfont;

:bool KFONT::init(dword font_path)
{
	IO label_io;
	if(font)free(font);
	label_io.read(font_path);
	if(!EAX) {
		//debugln(font_path);
		label_io.run("/sys/@notify", "'Error: KFONT is not loaded.' -E"); 
		return false;
	}
	font_begin = label_io.buffer_data;
	changeSIZE();
	smooth = true;
	return true;
}

:bool KFONT::changeSIZE()
{
	int i;
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
	for (i=0; i<256; i++) {
		kfont_char_width[i] = symbol_size((byte) i);
	}
	return true;
}

:dword KFONT::getsize(byte fontSizePoints, dword text1)
{
	size.height = size.width = 0;
	size.offset_x = size.offset_y = -1;
	if (size.pt != fontSizePoints) {
		size.pt = fontSizePoints;
		if(!changeSIZE())return 0;
	}
	WHILE(DSBYTE[text1])
	{
		size.width += symbol_size(DSBYTE[text1]);
		text1++;
	}
	$neg size.offset_y
	$neg size.offset_x
	size.height += size.offset_y+1;
	size.width += size.offset_x+1;
	return size.width;
}

//WILL NOT WORK if requested fontSizePoints 
//is differ from precalculated kfont_char_width[]
:int KFONT::get_label_width(dword _label) 
{
	int len=0;
	while (ESBYTE[_label]) {
		len += kfont_char_width[ ESBYTE[_label] ];
		_label++;
	}
	return len;
}

:byte KFONT::symbol_size(byte s)
{
	int chaw_width;
	chaw_width = symbol(0,0, s, 0);
	if(bold) chaw_width += math.ceil(size.pt/17);
	return chaw_width;
}

:byte KFONT::symbol(signed x,y; byte s; dword image_raw)
{
	dword xi,yi;
	dword iii = 0;
	dword offs;
	dword tmp, _;
	byte X;
	byte chaw_width=0;
	if(s==32)return width/4;
	if(s==9)return width;
	s = Cp866ToAnsi(s);
	tmp = block*s << 2 + font;
	for(yi=0; yi<height; yi++)
	{
		EDI = size.offset_y + yi + y * size.width * KFONT_BPP + image_raw;
		for(xi=0; xi<width; xi++)
		{
			if(iii%32) _ >>= 1;
			else
			{
					tmp += 4;
					_ = DSDWORD[tmp];
			}
			if(_&1) //check does the pixel set
			{
				if(xi>chaw_width)chaw_width=xi;
				//in case of image_raw!=0 draw font into bug
				//in case of image_raw==0 calculate size
				if (image_raw)
				{
					offs = x + xi * KFONT_BPP + EDI;
					DSDWORD[offs] = color;
					if(bold) DSDWORD[offs+KFONT_BPP] = color;
				}
				else
				{
					if(size.height<yi)size.height = yi;
					if(size.offset_y<0)size.offset_y = yi; else if(yi<size.offset_y)size.offset_y = yi;
					if(!X) X = xi; else if(X>xi)X = xi;
					if(size.offset_x<0)size.offset_x = X;
				}
			}
			iii++;
		}
	}
	return chaw_width;
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

/*=====================================================================================
===========================                                 ===========================
===========================               RAW               ===========================
===========================                                 ===========================
=====================================================================================*/

inline fastcall dword b32(EAX) { return DSDWORD[EAX]; }
:void KFONT::ApplySmooth()
{
	dword i,line_w,to,dark_background;
	line_w = size.width * KFONT_BPP;
	to = size.height - 1 * line_w + raw - KFONT_BPP;
	for(i=raw; i < to; i+=KFONT_BPP)
	{
		if(i-raw%line_w +KFONT_BPP == line_w) continue;
		// pixels position, where b - black, w - write
		// bw
		// wb
		if(b32(i)!=background) {
			if (b32(i+KFONT_BPP)==background) 
			&& (b32(i+line_w)==background) && (b32(i+KFONT_BPP+line_w)!=background)
			{
				dark_background = MixColors(background,b32(i),200);
				DSDWORD[i+KFONT_BPP] = dark_background;
				DSDWORD[i+line_w] = dark_background;	
			}
		}
		// wb
		// bw
		else if (b32(i+KFONT_BPP)!=background) 
		&& (b32(i+line_w)!=background) && (b32(i+KFONT_BPP+line_w)==background)
		{
			dark_background = MixColors(background,b32(i+KFONT_BPP),200);
			DSDWORD[i] = dark_background;
			DSDWORD[i+KFONT_BPP+line_w] = dark_background;	
		}
	}
}

:void KFONT::WriteIntoBuffer(int x,y,w,h; dword _background, _color; byte fontSizePoints; dword text1)
{
	dword new_raw_size;
	if(!text1)return;
	
	if (size.pt != fontSizePoints) {
		getsize(fontSizePoints, text1);
		y -= size.offset_y;
	}
	color = _color;
	background = _background;

	size.width = w;
	size.height = h;

	new_raw_size = w*h*KFONT_BPP;
	if(raw_size != new_raw_size)
	{
		raw_size = new_raw_size; 
		free(raw);
		raw = malloc(raw_size);
		// Fill background color
		EBX = background;
		EAX = raw_size+raw;
		for (EDI=raw; EDI<EAX; EDI+=KFONT_BPP) ESDWORD[EDI] = EBX;
	}
	WHILE(DSBYTE[text1])
	{
		x+=symbol(x,y,DSBYTE[text1], raw);
		if(bold)x+=math.ceil(size.pt/17);
		text1++;
	}
	return;
}

:int KFONT::WriteIntoWindow(int x,y; dword _background, _color; byte fontSizePoints; dword text1)
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

:int KFONT::WriteIntoWindowCenter(dword x,y,w,h; dword _background, _color; byte fontSizePoints; dword text1)
{
	getsize(fontSizePoints, text1);
	return WriteIntoWindow(w-size.width/2+x-1,y, _background, _color, fontSizePoints, text1);
}

:void KFONT::ShowBuffer(dword _x, _y)
{
	if (4==KFONT_BPP) PutPaletteImage(raw, size.width, size.height, _x, _y, 32, 0);
	//if (1==KFONT_BPP) PutPaletteImage(raw, size.width, size.height, _x, _y, 8, #palette);
}

:void KFONT::ShowBufferPart(dword _x, _y, _w, _h, _buf_offset)
{
	if (4==KFONT_BPP) PutPaletteImage(_buf_offset * KFONT_BPP + raw, _w, _h, _x, _y, 32, 0);
	//if (1==KFONT_BPP) PutPaletteImage(_buf_offset * KFONT_BPP + raw, _w, _h, _x, _y, 8, #palette);
}

#endif