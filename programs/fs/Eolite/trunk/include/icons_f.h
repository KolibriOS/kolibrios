char *ext[]={
"..",   17,
"<DIR>",16,
"txt", 1,
"doc", 1,
"rtf", 1,
"odt", 1,
"log", 1,
"docx",1,
"htm", 2,
"html",2,
"mht", 2,
"ini", 3,
"conf",3,
"xlt", 4,
"xls", 4,
"ods", 4,
"xlsx",4,
"cmd", 5,
"bat", 5,
"sh",  5,
"com", 5,
"kex", 6,
"exe", 7,
"msi", 7,
"sys", 8,
"ocx", 8,
"drv", 8,
"so",  8,
"inc", 9,
"chr", 10,
"mt",  10,
"ttf", 10,
"fon", 10,
"asm", 11,
"skn", 13,
"djvu",15,
"pdf", 15,
"fb2", 15,
"nes", 18,
"smc", 18,
"img", 21,
"ima", 21,
"dll", 22,
"obj", 22,
"iso", 24,
"cue", 24,
"nrg", 24,
"mdf", 24,
"gif", 19,
"bmp", 19,
"tga", 19,
"pcx", 19,
"png", 19,
"jpg", 19,
"xcf", 19,
"ai",  19,
"jpeg",19,
"raw", 19,
"psd", 19,
"ico", 20,
"cur", 20,
"ani", 20,
"vox", 20,
"rar", 23,
"zip", 23,
"cab", 23,
"tar", 23,
"ajr", 23,
"jar", 23,
"7z",  23,
"gz",  23,
"mp3", 12,
"wav", 12,
"mid", 12,
"midi",12,
"ogg", 12,
"wma", 12,
"flac",12,
"avi", 14,
"flv", 14,
"mpg", 14,
"wmv", 14,
"mov", 14,
"mkv", 14,
"mp4", 14,
"vob", 14,
0};


#include "imgs\icons.txt"


void Put_icon(dword extension, yy, fairing_color)
{ 
	int icon_n=0, i;

	for (i=0; ext[i]<>0; i+=2;)
		if (!strcmp(extension, ext[i])) icon_n = ext[i+1];

	PutPaletteImage(icon_n*16*15+#ficons,16,15,195,yy,#ficons_pal);
	if (icon_n<>17) && (strlen(extension)<9) WriteText(-strlen(extension)*3+onLeft(168,0)+36,yy+4,0x80,0,extension,0);
	if (fairing_color<>0xFFFfff) IconFairing(icon_n, yy, fairing_color); //закрашиваем иконку
}



void IconFairing(dword filenum, y, color)
{
	switch(filenum)
	{
		case 0...1: //файлик
			DrawBar(195,y,2,15,color);
			RIGHT_PAINT:
			DrawBar(209,y,2,15,color);
			DrawBar(205,y,4,1,color);
			DrawBar(206,y+1,3,1,color);
			DrawBar(207,y+2,2,1,color);
			PutPixel(208,y+3,color);
			return;
		case 2: //html
			DrawBar(195,y,1,7,color);
			DrawBar(196,y,1,6,color);
			DrawBar(195,y+10,1,5,color);
			DrawBar(196,y+11,1,4,color);
			GOTO RIGHT_PAINT;
		case 3: //настройки
			DrawBar(195,y,2,7,color);
			DrawBar(195,y+7,1,2,color);
			DrawBar(195,y+12,1,3,color);
			PutPixel(196,y+14,color);
			GOTO RIGHT_PAINT;
		case 9: //inc
			DrawBar(195,y,1,12,color);
			DrawBar(196,y,1,11,color);
			DrawBar(197,y,1,10,color);
			DrawBar(198,y,1,5,color);
			DrawBar(199,y,1,4,color);
			DrawBar(200,y,1,3,color);
			DrawBar(201,y,1,2,color);
			DrawBar(202,y,1,3,color);
			DrawBar(203,y,1,2,color);
			PutPixel(204,y,color);
			PutPixel(205,y+4,color);
			PutPixel(206,y+3,color);
			PutPixel(207,y,color);
			PutPixel(207,y+2,color);
			DrawBar(208,y,1,3,color);
			DrawBar(209,y,2,4,color);
			DrawBar(210,y,1,15,color);
			PutPixel(198,y+14,color);
			DrawBar(199,y+13,1,2,color);
			DrawBar(200,y+12,10,3,color);
			PutPixel(205,y+11,color);
			DrawBar(206,y+10,1,2,color);
			DrawBar(207,y+9,1,3,color);
			PutPixel(207,y+7,color);
			DrawBar(208,y+6,2,7,color);
			PutPixel(209,y+5,color);
			return;
		case 10: //font
			DrawRegion_3D(195,y,15,14,color,color);
			DrawBar(196,y+1,1,13,color);
			DrawBar(197,y+1,1,11,color);
			DrawBar(198,y+1,1,10,color);
			DrawBar(199,y+1,1,9,color);
			DrawBar(200,y+1,1,7,color);
			DrawBar(201,y+1,1,5,color);
			DrawBar(202,y+1,1,4,color);
			DrawBar(203,y+1,1,2,color);
			DrawBar(209,y+1,1,13,color);
			DrawBar(208,y+1,1,11,color);
			PutPixel(204,y+6,color);
			DrawBar(203,y+10,2,1,color);
			DrawBar(202,y+11,2,3,color);
			return;
		case 11: //asm
			DrawBar(195,y,1,15,color);
			return;
		case 12: //audio
			DrawBar(195,y,16,1,color);
			DrawBar(195,y,1,15,color);
			DrawBar(196,y+1,4,8,color);
			DrawBar(196,y+9,1,2,color);
			PutPixel(197,y+9,color);
			PutPixel(196,y+14,color);
			DrawBar(210,y+1,1,14,color);
			DrawBar(202,y+4,6,4,color);
			DrawBar(202,y+8,4,1,color);
			DrawBar(202,y+9,3,1,color);
			DrawBar(202,y+10,2,3,color);
			DrawBar(201,y+13,4,1,color);
			DrawBar(200,y+14,14,1,color);
			DrawBar(209,y+12,1,2,color);
			PutPixel(208,y+13,color);
			return;
		case 13: //skin
			PutPixel(210,y,color); 
			return;
		case 14...15: //video, book
			DrawBar(195,y,1,15,color);
			DrawBar(210,y,1,15,color);
			return;
		case 16...17: //папка
			DrawBar(195,y,1,15,color);
			DrawBar(203,y,8,2,color);
			IF (filenum==17) PutPixel(206,y+1,0x1A7B17); //зелёная точка стрелки
			DrawBar(196,y+13,15,2,color);
			PutPixel(196,y,color); //.точки
			PutPixel(202,y,color);
			PutPixel(210,y+2,color);
			PutPixel(196,y+12,color);
			PutPixel(210,y+12,color);
			return;
		case 18: //картридж
			DrawBar(195,y,16,2,color);
			DrawBar(195,y+11,1,2,color);
			DrawBar(210,y+11,1,2,color);
			DrawBar(195,y+13,16,2,color);
			return;
		case 19: //изображение
		case 20: //изображение
			DrawBar(195,y+14,16,1,color);
			return;
		case 21: //дискета
			PutPixel(195,y,color);
			PutPixel(210,y,color);
			return;
		case 22: //библиотека
			DrawBar(195,y,16,1,color); //сверху слева
			DrawBar(195,y+1,3,1,color);
			DrawBar(195,y+2,2,1,color);
			PutPixel(195,y+3,color);
			PutPixel(210,y+11,color); //справа справа
			DrawBar(209,y+12,2,1,color); 
			DrawBar(208,y+13,3,1,color);
			DrawBar(207,y+14,4,1,color);
			return;
		case 23: //архив
			PutPixel(195,y+3,color);
			PutPixel(195,y+11,color);
			PutPixel(210,y+3,color);
			PutPixel(210,y+11,color);

			DrawBar(195,y,7,1,color);
			DrawBar(204,y,7,1,color);
			DrawBar(195,y+14,7,1,color);
			DrawBar(204,y+14,7,1,color);

			DrawBar(195,y+1,5,1,color);
			DrawBar(206,y+1,5,1,color);
			DrawBar(195,y+13,5,1,color);
			DrawBar(206,y+13,5,1,color);

			DrawBar(195,y+2,3,1,color);
			DrawBar(208,y+2,3,1,color);
			DrawBar(195,y+12,3,1,color);
			DrawBar(208,y+12,3,1,color);
			return;
		case 24: //образ
			DrawBar(195,y,6,1,color);
			DrawBar(195,y+1,4,1,color);
			DrawBar(195,y+2,3,1,color);
			DrawBar(195,y+3,2,2,color);
			
			DrawBar(195,y+5,1,5,color);
			
			DrawBar(195,y+10,2,2,color);
			DrawBar(195,y+12,3,1,color);
			DrawBar(195,y+13,4,1,color);
			DrawBar(195,y+14,6,1,color);			

			DrawBar(195+11,y,5,1,color);
			DrawBar(195+13,y+1,3,1,color);
			DrawBar(195+14,y+2,2,1,color);
			DrawBar(195+15,y+3,1,2,color);
			
			DrawBar(195+15,y+10,1,2,color);
			DrawBar(195+14,y+12,2,1,color);
			DrawBar(195+13,y+13,3,1,color);
			DrawBar(195+11,y+14,5,1,color);			

			return;
	}
}
