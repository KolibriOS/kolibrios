char *ext[]={
"..",   17,
"<DIR>",16,
"txt", 1, "doc", 1, "rtf", 1, "odt", 1, "log", 1, "docx",1,
"htm", 2, "html",2, "mht", 2,
"ini", 3, "js",  3, "conf",3, "inf", 3,
"xlt", 4, "xls", 4, "ods", 4, "xlsx",4,
"cmd", 5, "bat", 5, "py",  5, "sh",  5, "ksh", 5, "com", 5,
"kex", 6, "lua", 6,
"exe", 7, "msi", 7,
"sys", 8, "ocx", 8, "drv", 8, "so",  8,
"inc", 9,
"chr", 10, "mt",  10, "ttf", 10, "fon", 10,
"asm", 11,
"mp3", 12, "wav", 12, "mid", 12, "midi",12, "ogg", 12, "wma", 12, "flac",12,
"skn", 13,
"avi", 14, "flv", 14, "mpg", 14, "wmv", 14, "mov", 14, "mkv", 14, "mp4", 14, "vob", 14, "webm", 14,
"djvu",15, "pdf", 15, "fb2", 15,
"nes", 18, "smc", 18,
"gif", 19, "bmp", 19, "tga", 19, "pcx", 19, "png", 19, "pnm", 19, "jpg", 19, "xcf", 19, "ai",  19,
"jpeg",19, "raw", 19, "psd", 19, "wbmp",19, "tiff",19, "tif", 19, 
"3ds", 20, "ico", 20, "cur", 20, "ani", 20, "vox", 20,
"img", 21, "ima", 21,
"dll", 22, "obj", 22, "dict",22,
"rar", 23, "zip", 23, "cab", 23, "tar", 23, "ajr", 23, "jar", 23, "7z", 23, "gz", 23, "kexp", 23,
"iso", 24, "cue", 24, "nrg", 24, "mdf", 24,
"grf", 25,
0,0};


#include "imgs\icons.txt"

void Put_icon(dword extension, xx, yy, fairing_color, default_icon)
{ 
	int icon_n=default_icon, i;
	for (i=0; ext[i]!=0; i+=2;)	if (!strcmp(extension, ext[i]))	{ icon_n = ext[i+1]; break;	}

	ficons_pal[0] = fairing_color;
	PutPaletteImage(icon_n*16*15+#ficons,16,15,xx,yy,8,#ficons_pal);
	if (fairing_color!=0xFFFfff) IconFairing(icon_n, xx, yy, fairing_color);
	if (icon_n<>17) && (strlen(extension)<9) WriteText(-strlen(extension)*3+Form.cwidth-120,yy+4,0x80,0,extension);
}


void IconFairing(dword filenum, x,y, color)
{
	switch(filenum)
	{
		case 0...1: //file
		case 3: //настройки
			RIGHT_PAINT:
			PutPixel(x+10,y,color);
			PutPixel(x+11,y+1,color);
			PutPixel(x+12,y+2,color);
			PutPixel(x+13,y+3,color);
			return;
		case 2: //html
			DrawBar(x,y,1,7,color);
			DrawBar(x+1,y,1,6,color);
			DrawBar(x,y+10,1,5,color);
			DrawBar(x+1,y+11,1,4,color);
			GOTO RIGHT_PAINT;
		case 9: //inc
			DrawBar(x+13,y,1,3,color);
			DrawBar(x+14,y,2,4,color);
			DrawBar(x+15,y,1,15,color);
			PutPixel(x+3,y+14,color);
			DrawBar(x+4,y+13,1,2,color);
			DrawBar(x+5,y+12,10,3,color);
			PutPixel(x+10,y+11,color);
			DrawBar(x+11,y+10,1,2,color);
			DrawBar(x+12,y+9,1,3,color);
			PutPixel(x+12,y+7,color);
			DrawBar(x+13,y+6,2,7,color);
			PutPixel(x+14,y+5,color);
			return;
		case 10: //font
			DrawBar(x+1,y+1,1,13,color);
			DrawBar(x+2,y+1,1,11,color);
			DrawBar(x+3,y+1,1,10,color);
			DrawBar(x+4,y+1,1,9,color);
			DrawBar(x+5,y+1,1,7,color);
			DrawBar(x+6,y+1,1,5,color);
			DrawBar(x+7,y+1,1,4,color);
			DrawBar(x+8,y+1,1,2,color);
			DrawBar(x+14,y+1,1,13,color);
			DrawBar(x+13,y+1,1,11,color);
			PutPixel(x+9,y+6,color);
			DrawBar(x+8,y+10,2,1,color);
			DrawBar(x+7,y+11,2,3,color);
			return;
		case 12: //audio
			PutPixel(x+2,y+9,color);
			PutPixel(x+1,y+10,color);
			PutPixel(x+10,y+8,color);
			PutPixel(x+9,y+9,color);

			PutPixel(x+6,y+13,color);
			PutPixel(x+5,y+14,color);
			PutPixel(x+14,y+12,color);
			PutPixel(x+13,y+13,color);
			return;
		case 13: //skin
			PutPixel(x+15,y,color); 
			return;
		case 16...17: //folder
			DrawBar(x,y,1,15,color);
			DrawBar(x+8,y,8,2,color);
			IF (filenum==17) PutPixel(x+11,y+1,0x1A7B17); //green arrow part
			DrawBar(x+1,y+13,15,2,color);
			PutPixel(x+1,y,color); //.точки
			PutPixel(x+7,y,color);
			PutPixel(x+15,y+2,color);
			PutPixel(x+1,y+12,color);
			PutPixel(x+15,y+12,color);
			return;
		case 18: //картридж
			DrawBar(x,y+11,1,2,color);
			DrawBar(x+15,y+11,1,2,color);
			DrawBar(x,y+13,16,1,color);
			return;
		case 24: //образ
			DrawBar(x,y,6,1,color);
			DrawBar(x,y+1,4,1,color);
			DrawBar(x,y+2,3,1,color);
			DrawBar(x,y+3,2,2,color);
			
			DrawBar(x,y+5,1,5,color);
			
			DrawBar(x,y+10,2,2,color);
			DrawBar(x,y+12,3,1,color);
			DrawBar(x,y+13,4,1,color);
			DrawBar(x,y+14,6,1,color);			

			DrawBar(x+11,y,5,1,color);
			DrawBar(x+13,y+1,3,1,color);
			DrawBar(x+14,y+2,2,1,color);
			DrawBar(x+15,y+3,1,2,color);
			
			DrawBar(x+15,y+10,1,2,color);
			DrawBar(x+14,y+12,2,1,color);
			DrawBar(x+13,y+13,3,1,color);
			DrawBar(x+11,y+14,5,1,color);
	}
}
