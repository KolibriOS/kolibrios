#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb.h"
//#include "font_droid.h"

#define __stdcall __attribute__((stdcall))

//inline void getzone(unsigned int x, unsigned int y, unsigned int w, unsigned int h, char * image){
//unsigned size, psize;
//
//size=x*65536+y;
//psize=w*65536+h;
//__asm__ __volatile__("int $0x40"::"a"(36),"b"(image),"c"(psize),"d"(size));
//}
//



// inline void f65(unsigned x, unsigned y, unsigned w, unsigned h, char *d)
//{
//asm("pusha");
//asm ("nop"::"D"(0), "c"(w*65536+h), "d"(x*65536+y), "b"(d));
//asm ("xor %eax, %eax");
//asm ("movl %eax, %ebp");
//asm ("pushl $24");
//asm ("popl %esi");
//asm ("int $0x40"::"a"(65));
//asm("popa");
//}

inline void PutImage(unsigned x, unsigned y, unsigned w, unsigned h, char * image){
unsigned size, psize;
size=x*65536+y;
psize=w*65536+h;
__asm__ __volatile__("int $0x40"::"a"(7),"b"(image),"c"(psize),"d"(size));
}

//void kputc(char *c){
//	asm ("int $0x40"::"a"(63), "b"(1), "c"(c));
//}
//
//void kol_board_puti(int n){
//	char c;
//	int i = 0;
//	do {
//		if (n<0) {
//			n=-n;
//			kputc("-");
//		}
//		c = n % 10 + '0';
//		asm ("int $0x40"::"a"(63), "b"(1), "c"(c));
//		i++;
//	}
//	while ((n /= 10) > 0);
//}

void font_blit(unsigned char *dst,unsigned char *src, int x, int size) {
	int lp;
	float mixed, tp;
	unsigned char color1;
	int color2, color3, color4;
	color2=((x)&0x000000FF);
	color3=((((x)&0x00FF00)>>8)&0xFF);
	color4=((((x)&0xFF0000)>>16)&0xFF);
//kol_board_puti(size);
	for (lp=0;lp <= size;lp++) {
		//kol_board_puti(lp);
		//kputc("\n");
		if (src[lp]>0) {
			tp=((float)src[lp])/255;

			color1=dst[lp*3];
			mixed=(color1 * (1-tp)+color2 * tp);
			dst[lp*3]=(unsigned char) mixed;

			color1=dst[lp*3+1];
			mixed=(color1 * (1-tp)+color3 * tp);
			dst[lp*3+1]=(unsigned char) mixed;

			color1=dst[lp*3+2];
			mixed=(color1 * (1-tp)+color4 * tp);
			dst[lp*3+2]=(unsigned char) mixed;

		}
	}

}


inline unsigned kol_process_info(unsigned slot, char buf1k[]){
	asm ("int $0x40"::"a"(9), "b"(buf1k), "c"(slot));
}

//void kol_board_puts(char *s){
//	unsigned i;
//	i = 0;
//	while (*(s+i))
//	{
//		asm ("int $0x40"::"a"(63),"b"(1),"c"(*(s+i)));
//		i++;
//	}
//}

//unsigned char screen[40*160];

int __stdcall init_font(stbtt_fontinfo *font,unsigned char *FontData)
{
	//stbtt_InitFont(font, pdf_font_DroidSans,stbtt_GetFontOffsetForIndex(pdf_font_DroidSans,0) );
	stbtt_InitFont(font, FontData,stbtt_GetFontOffsetForIndex(FontData,0) );
//	char *yay;
//	yay=font;
//	int i;
//	for (i=0;i<28;i++)
//		kputc(*(yay+i));
//	kol_board_puts("init_font\n");
}

float __stdcall get_width(short *text, stbtt_fontinfo *buffer, int height)
{
	stbtt_fontinfo *font;
	font=buffer;
	int ch=0;	//baseline,i,j,ascent,  
	float scale,xpos=0;

	//baseline=10;

 // if (buffer==-1) {stbtt_InitFont(&font, pdf_font_DroidSans,stbtt_GetFontOffsetForIndex(pdf_font_DroidSans,0) );} else {stbtt_InitFont(&font, buffer,stbtt_GetFontOffsetForIndex(buffer,0) );kol_board_puts("Font loaded..\n");}
//	kol_board_puts("Engine init..\n");
//kol_board_puti(&screen);
	scale = stbtt_ScaleForPixelHeight(font, height*3/4);
	int advance,lsb;

	while (text[ch]) {
//	   kol_board_puts("new symbol...\n");

		stbtt_GetCodepointHMetrics(font, text[ch], &advance, &lsb);
		xpos += (advance * scale);
		//if (text[ch+1])
		//	xpos += scale*stbtt_GetCodepointKernAdvance(font, text[ch],text[ch+1]);
		++ch;
	}
	//stbtt_GetCodepointHMetrics(font, text[ch], &advance, &lsb);
	//xpos += (advance * scale);
	return xpos;
}

int __stdcall get_length(short *text, char *buffer, int height, int max_len){
	stbtt_fontinfo font;
	int i,j,ascent,baseline,ch,xpos=0;
	float scale=0;

	baseline=10;

//	if (buffer==-1) {
//		stbtt_InitFont(&font, pdf_font_DroidSans,stbtt_GetFontOffsetForIndex(pdf_font_DroidSans,0) );
//	} 
//	else {
//		stbtt_InitFont(&font, buffer,stbtt_GetFontOffsetForIndex(buffer,0) );
//		kol_board_puts("Font loaded..\n");
//	}
//kol_board_puts("Engine init..\n");
//kol_board_puti(&screen);
	scale = stbtt_ScaleForPixelHeight(&font, height*3/4);

	while (text[ch]) {
	  // kol_board_puts("new symbol...\n");
		int advance,lsb;
		stbtt_GetCodepointHMetrics(&font, text[ch], &advance, &lsb);
		xpos += (advance * scale);
		if ((int)xpos>max_len) return ch;
		//if (text[ch+1])
		//	xpos += scale*stbtt_GetCodepointKernAdvance(&font, text[ch],text[ch+1]);
		++ch;
	}
	return ch;

}


int __stdcall picture(short *text, stbtt_fontinfo *buffer, char *screen1, int width, int height){

 //unsigned char *screen;
 //screen=zmalloc(20*78);
	//kol_board_puts(screen);
	//kol_board_puts("It was text\n");

	stbtt_fontinfo *font;
	font=buffer;
	int i,j,ascent,baseline,descent,ch=0;
	float scale, xpos=0;

   //baseline=10;
	//kol_board_puts("Font address:\n");
   //kol_board_puti(buffer);
// if (buffer==-1) {stbtt_InitFont(&font, pdf_font_DroidSans,stbtt_GetFontOffsetForIndex(pdf_font_DroidSans,0) );kol_board_puts("default font\n");} else {stbtt_InitFont(&font, buffer,stbtt_GetFontOffsetForIndex(buffer,0) );kol_board_puts("Font loaded..\n");}

//kol_board_puti(&screen);
	scale = stbtt_ScaleForPixelHeight(font, height*3/4);
   //stbtt_GetFontVMetrics(&font, &ascent,0,0);
	stbtt_GetFontVMetrics(font, &ascent,&descent,0); //lev
   //baseline = (int) (ascent*scale);
	baseline = (int) ((ascent-descent)*scale); //lev


//kol_board_puts("Text render:\n");

	while (text[ch]) {
	   //kol_board_puts("new symbol...\n");
		int advance,lsb,x0,y0,x1,y1;
      //float x_shift = xpos - (float) i_floor(xpos);

     // kol_board_puts("floor called!\n");
		stbtt_GetCodepointHMetrics(font, text[ch], &advance, &lsb);
		stbtt_GetCodepointBitmapBoxSubpixel(font, text[ch], scale,scale,0,0, &x0,&y0,&x1,&y1);


     //10= y0, 20=y1-y0 or so
		stbtt_MakeCodepointBitmapSubpixel(font, &screen1[(baseline + y0)*width+ (int)xpos + x0], x1-x0,y1-y0, width, scale,scale,0,0, text[ch]);

      // note that this stomps the old data, so where character boxes overlap (e.g. 'lj') it's wrong
      // because this API is really for baking character bitmaps into textures. if you want to do this,
      // you need to render the bitmap to a temp buffer, then\n\t"alpha blend" that into the working buffer
		xpos += (advance * scale);
		//if (text[ch+1])
			//xpos += scale*stbtt_GetCodepointKernAdvance(font, text[ch],text[ch+1]);
		++ch;
	}

  //zmemcpy(screen1,bitmap,20*20);

//kol_board_puts("finished...\n");
	return 0;
}

void __stdcall SetBackColor(int back_color, int width, int height, char *from_zone)
{
	int i,j;
	unsigned char bcr,bcg,bcb;
	
	bcr=back_color<<16;
	bcg=back_color<<8;
	bcb=back_color;
		
	for (j=0;j<height ;j++)
	{
		for (i=0;i<width ;i++)
		{
			from_zone[(j*width+i)*3]=back_color;
			from_zone[(j*width+i)*3+1]=back_color>>8;
			from_zone[(j*width+i)*3+2]=back_color>>16;
		}
	}
}

int __stdcall text_out(short *string, char *buffer, int height, int color,int back_color, int x, int y) {
	unsigned char *from_zone;
	unsigned char *to_zone;
	int px, py;
	unsigned char app_data[1024];

	int width;
//kol_board_puts("width...\n");
//kol_board_puts("\n and now height is ...");
//kol_board_puti(height);

	//width=600; 
	width = get_width(string,buffer,height);
	
//kol_board_puts("\n Width is ...");
//kol_board_puti(width);
//kol_board_puts(" and height is ...");
//kol_board_puti(height);

//kol_board_puts("\n malloc...\n");
	from_zone=(char*)zmalloc(3*height*width);
	to_zone=(char*)zmalloc(height*width);
//kol_board_puts("malloc done...\n");

	kol_process_info(-1, app_data);
	//px=app_data[35]*256+app_data[34];
	px=app_data[35]*256+app_data[34]+app_data[55]*256+app_data[54];//lev
	//py=app_data[39]*256+app_data[38];
	py=app_data[39]*256+app_data[38]+app_data[59]*256+app_data[58];//lev

//kol_board_puts("\nzone...\n");
	//getzone(px+x, py+y, width, height, from_zone);
	SetBackColor(back_color, width, height,from_zone);
//kol_board_puts("render...\n");
	picture(string, buffer, to_zone, width, height);
//kol_board_puts("blit...\n");
	font_blit(from_zone,to_zone, color, width*height);
//kol_board_puts("out...\n");
 //f65(x,y,width,height,from_zone);
	PutImage(x,y,width,height,from_zone);//lev
	zfree(from_zone);
	zfree(to_zone);
	return 0;
}

unsigned char*  __stdcall text_out_mem(short *string, char *buffer, int height, int color,int back_color) {
	unsigned char *from_zone;
	unsigned char *to_zone;
	int px, py;
	unsigned char app_data[1024];
	int width;

	width = get_width(string,buffer,height);
	from_zone=(char*)zmalloc(3*height*width+4);
	to_zone=(char*)zmalloc(height*width);
	*(int*)from_zone = width;
	//kol_process_info(-1, app_data);
	//px=app_data[35]*256+app_data[34]+app_data[55]*256+app_data[54];//lev
	//py=app_data[39]*256+app_data[38]+app_data[59]*256+app_data[58];//lev
	SetBackColor(back_color, width, height,from_zone+4);
	picture(string, buffer, to_zone, width, height);
	font_blit(from_zone+4,to_zone, color, width*height);
	//PutImage(x,y,width,height,from_zone);//lev
	
	//zfree(from_zone);
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
		{ szTrueType,	picture },
		{ szGetLength,	get_length},
		{ szGetWidth,	get_width},
		{ szTextOut,	text_out },
		{ szTextOutMem, text_out_mem},
		{ szInitFont,	init_font},
		{ NULL, NULL },
	};
