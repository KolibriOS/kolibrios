#include "glue.h"
#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"

#define __stdcall __attribute__((stdcall))

// "Fast and economical" UTF-8 to codepoint decoder by Bjoern Hoehrmann.
// Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

#define UTF8_ACCEPT 0
#define UTF8_REJECT 12

static const unsigned char utf8d[] = {
  // The first part of the table maps bytes to character classes that
  // to reduce the size of the transition table and create bitmasks.
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
   8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

  // The second part is a transition table that maps a combination
  // of a state of the automaton and a character class to a state.
   0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
  12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
  12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
  12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
  12,36,12,12,12,12,12,12,12,12,12,12, 
};

unsigned int inline decode(unsigned int* state, unsigned int* codep, unsigned int byte) {
  unsigned int type = utf8d[byte];

  *codep = (*state != UTF8_ACCEPT) ?
    (byte & 0x3fu) | (*codep << 6) :
    (0xff >> type) & (byte);

  *state = utf8d[256 + *state + type];
  return *state;
}

inline void PutImage(unsigned x, unsigned y, unsigned w, unsigned h, char * image){
unsigned size, psize;
size=x*65536+y;
psize=w*65536+h;
__asm__ __volatile__("int $0x40"::"a"(7),"b"(image),"c"(psize),"d"(size));
}

void font_blit(unsigned char *dst,unsigned char *src, int textcolor, int size) {
	int lp;
	float mixed, tp;
	unsigned char color1;
	int red, green, blue;
	red=((textcolor)&0x000000FF);
	green=(((textcolor)&0x00FF00)>>8);
	blue=(((textcolor)&0xFF0000)>>16);
	
	for (lp=0;lp <= size;lp++) {
		if (src[lp]>0) {
			tp=((float)src[lp])/255;

			color1=dst[lp*3];
			mixed=(color1 * (1-tp)+red * tp);
			dst[lp*3]=(unsigned char) mixed;

			color1=dst[lp*3+1];
			mixed=(color1 * (1-tp)+green * tp);
			dst[lp*3+1]=(unsigned char) mixed;

			color1=dst[lp*3+2];
			mixed=(color1 * (1-tp)+blue * tp);
			dst[lp*3+2]=(unsigned char) mixed;

		}
	}

}


inline unsigned kol_process_info(unsigned slot, char buf1k[]){
	asm ("int $0x40"::"a"(9), "b"(buf1k), "c"(slot));
}

int __stdcall init_font(stbtt_fontinfo *font,unsigned char *FontData)
{
 	stbtt_InitFont(font, FontData,stbtt_GetFontOffsetForIndex(FontData,0) );
}

int __stdcall get_width_utf8(unsigned char *s, stbtt_fontinfo *buffer, int height)
{
	stbtt_fontinfo *font;
	font=buffer;  
	float scale, xpos=0;

	unsigned int codepoint;
	unsigned int state = 0;

	scale = stbtt_ScaleForPixelHeight(font, height*3/4);
	int advance,lsb;

	for (; *s; s++) {			
		if (!decode(&state, &codepoint, *s)) {
			stbtt_GetCodepointHMetrics(font, codepoint, &advance, &lsb);
			xpos += (advance * scale);
		}
	}

	return xpos;
}


int __stdcall get_length_utf8(unsigned char *s, char *buffer, int height, int max_len) 
{
	stbtt_fontinfo *font;
	font=buffer;
	int ch=0, xpos=0;
	float scale=0;

	scale = stbtt_ScaleForPixelHeight(&font, height*3/4);
	int advance, lsb;	
	
	unsigned int codepoint;
    unsigned int state = 0;

    for (; s[ch]; ch++) {
		if (!decode(&state, &codepoint, s[ch])) {
		
		stbtt_GetCodepointHMetrics(&font, codepoint, &advance, &lsb);
		xpos += (advance * scale);
		
		if ((int)xpos>max_len) 
			return ch;
		}
	}
	return ch;
}


int __stdcall picture_utf8(unsigned char *s, stbtt_fontinfo *buffer, char *screen1, int width, int height)
{
	stbtt_fontinfo *font;
	font=buffer;
	int ascent, baseline, descent;
	int advance, lsb, x0, y0, x1, y1;
	float scale, xpos=0;

	scale = stbtt_ScaleForPixelHeight(font, height*3/4);
	stbtt_GetFontVMetrics(font, &ascent, &descent,0); 
	baseline = (int) ((ascent-descent)*scale); 
	
	unsigned int codepoint;
    unsigned int state = 0;

    for (; *s; s++) {
		if (!decode(&state, &codepoint, *s)) {
			stbtt_GetCodepointHMetrics(font, codepoint, &advance, &lsb);									///////////////////////////////////
			stbtt_GetCodepointBitmapBoxSubpixel(font, codepoint, scale, scale, 0, 0, &x0, &y0, &x1, &y1);	///////////////////////////////////
			stbtt_MakeCodepointBitmapSubpixel(font, &screen1[(baseline+y0)*width+(int)xpos+x0], x1-x0, y1-y0, width, scale, scale, 0, 0, codepoint);

			xpos += (advance*scale);
		}
		//if (state != UTF8_ACCEPT)
			//kol_board_puts("The string is not well-formed\n");		
	}

	return 0;
}


void __stdcall SetBackColor(int back_color, int width, int height, char *from_zone)
{
	unsigned char bcr = back_color>>16;
	unsigned char bcg = back_color>>8;
	unsigned char bcb = back_color;

	unsigned int i;
	unsigned int max_i = width * height * 3;
	for (i=0; i < max_i; i+=3) 
	{
		from_zone[i]   = bcb;
		from_zone[i+1] = bcg;
		from_zone[i+2] = bcr;		
	}

}




int __stdcall text_out(char *string, char *buffer, int height, int color,int back_color, int x, int y) {
	unsigned char *from_zone;
	unsigned char *to_zone;
	int px, py;
	unsigned char app_data[1024];
	int width;

	width = get_width_utf8(string,buffer,height);
	
	from_zone=(char*)zmalloc(3*height*width);
	to_zone=(char*)zmalloc(height*width);

	kol_process_info(-1, app_data);
	//px=app_data[35]*256+app_data[34];
	px=app_data[35]*256+app_data[34]+app_data[55]*256+app_data[54];//lev
	//py=app_data[39]*256+app_data[38];
	py=app_data[39]*256+app_data[38]+app_data[59]*256+app_data[58];//lev

	//getzone(px+x, py+y, width, height, from_zone);
	SetBackColor(back_color, width, height,from_zone);
	picture_utf8(string, buffer, to_zone, width, height);
	font_blit(from_zone,to_zone, color, width*height);
//f65(x,y,width,height,from_zone);
	PutImage(x,y,width,height,from_zone);//lev

	zfree(from_zone);
	zfree(to_zone);
	return 0;
}

unsigned char*  __stdcall text_out_mem(short *string, stbtt_fontinfo *buffer, int height, int color,int back_color) {
	unsigned char *from_zone;
	unsigned char *to_zone;
	int width;

	width = get_width_utf8(string,buffer,height);
	from_zone=(char*)zmalloc(3*height*width+8);
	to_zone=(char*)zmalloc(height*width);
	*(int*)from_zone = width;
	*(int*)(from_zone+4) = height;	
	SetBackColor(back_color, width, height,from_zone+8);
	picture_utf8(string, buffer, to_zone, width, height);		
	font_blit(from_zone+8, to_zone, color, width*height);

	zfree(to_zone);
	return from_zone;
}

int __stdcall start(){
      return 1;
}

int __stdcall version_major(){
      return 1;
}

int __stdcall version_minor(){
     return 0;
}

typedef struct{
  char *name;
  void *f;
}export_t;

char szStart[]		="START";
char szVersion[]	="version";
char szVersionM[]	="version_min";
char szTrueType[]	="truetype";
char szGetLength[]	="get_length";
char szGetWidth[]	="get_width";
char szTextOut[]	="text_out";
char szTextOutMem[] ="text_out_mem";
char szInitFont[]	="init_font";


export_t EXPORTS[] __asm__("EXPORTS") =
	{
		{ szStart,		start },
		{ szVersion,	version_major },
		{ szVersionM,	version_minor },
		{ szTrueType,	picture_utf8 },	
		{ szGetLength,	get_length_utf8},
		{ szGetWidth,	get_width_utf8},
		{ szTextOut,	text_out },
		{ szTextOutMem, text_out_mem},
		{ szInitFont,	init_font},
		{ NULL, NULL },
	};
