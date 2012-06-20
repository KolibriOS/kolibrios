
#include "imgs\icons.txt"
dword Put_icon(dword extension, yy)
{ 
	dword ftype="", fnum=0; //еши неизвесный файл 

	IF (!strcmp(extension,"htm")) || (!strcmp(extension,"html")) ||
	   (!strcmp(extension,"mht")) {fnum = 2; ftype="Web-page";}
	IF (!strcmp(extension,"ini")) || (!strcmp(extension,"conf")) {fnum = 3; ftype="Config";}
	IF (!strcmp(extension,"xlt")) || (!strcmp(extension,"xls")) ||
		(!strcmp(extension,"ods")) || (!strcmp(extension,"xlsx")) {fnum = 4; ftype="Table";}
	IF (!strcmp(extension,"cmd")) || (!strcmp(extension,"bat")) || (!strcmp(extension,"sh")) {fnum = 5; ftype="Script";}
	IF (!strcmp(extension,"com")) {fnum = 5; ftype="DOS Exec";}
	IF (!strcmp(extension,"kex")) {fnum = 6; ftype="Program";}
	IF (!strcmp(extension,"exe")) || (!strcmp(extension,"msi")) {fnum = 7; ftype="Win32 Exec";}
	IF (!strcmp(extension,"sys")) || (!strcmp(extension,"ocx")) ||
       (!strcmp(extension,"drv")) || (!strcmp(extension,"so")) fnum = 8;
	IF (!strcmp(extension,"inc"))  fnum = 9;
	IF (!strcmp(extension,"chr")) || (!strcmp(extension,"mt")) ||
	   (!strcmp(extension,"ttf")) || (!strcmp(extension,"fon")) {fnum = 10; ftype="Font";}
	IF (!strcmp(extension,"asm")) {fnum = 11; ftype="Source";}
	IF (!strcmp(extension,"skn")) {fnum = 13; ftype="Skin";}
	IF (!strcmp(extension,"djvu")) || (!strcmp(extension,"pdf"))  || (!strcmp(extension,"fb2")) {fnum = 15; ftype="Book";}
	IF (!strcmp(extension,"nes")) || (!strcmp(extension,"smc")) {fnum = 18; ftype="Cartridge";}
	IF (!strcmp(extension,"img")) || (!strcmp(extension,"ima")) {fnum = 21; ftype="Image";}
	IF (!strcmp(extension,"dll")) || (!strcmp(extension,"obj")) {fnum = 22; ftype="Library";}
	IF (!strcmp(extension,"iso")) || (!strcmp(extension,"cue")) ||
	   (!strcmp(extension,"nrg")) || (!strcmp(extension,"mdf")) {fnum = 24; ftype="Disc image";}
	
	//text
	IF (!strcmp(extension,"txt")) || (!strcmp(extension,"doc")) ||
	   (!strcmp(extension,"rtf")) || (!strcmp(extension,"odt")) ||
	   (!strcmp(extension,"log")) || (!strcmp(extension,"docx")) {fnum = 1; ftype="Text";}
	//изображения
	IF (!strcmp(extension,"gif")) || (!strcmp(extension,"bmp")) ||
	   (!strcmp(extension,"tga")) || (!strcmp(extension,"pcx")) ||
	   (!strcmp(extension,"png")) || (!strcmp(extension,"jpg")) {fnum = 19; ftype="Image";}
	IF (!strcmp(extension,"xcf")) || (!strcmp(extension,"ai")) ||
	   (!strcmp(extension,"jpeg")) || (!strcmp(extension,"raw")) ||
	   (!strcmp(extension,"psd")) {fnum = 19; ftype="Image";}
	//пиктограммы
	IF (!strcmp(extension,"ico")) {fnum = 20; ftype="Icon";}
	IF (!strcmp(extension,"cur")) || (!strcmp(extension,"ani")) {fnum = 20; ftype="Cursor";}
	//архивы
	IF (!strcmp(extension,"rar")) || (!strcmp(extension,"zip")) ||
	   (!strcmp(extension,"cab")) || (!strcmp(extension,"tar")) ||
	   (!strcmp(extension,"ajr")) || (!strcmp(extension,"jar")) || 
	   (!strcmp(extension,"7z")) ||(!strcmp(extension,"gz")) {fnum = 23; ftype="Archive";}
	//audio
	IF (!strcmp(extension,"mp3")) {fnum = 12; ftype="Music";}
	IF (!strcmp(extension,"wav")) || (!strcmp(extension,"mid")) ||
	   (!strcmp(extension,"midi")) || (!strcmp(extension,"ogg")) ||
	   (!strcmp(extension,"wma"))  || (!strcmp(extension,"flac")) {fnum = 12; ftype="Audio";}
	//video
	IF (!strcmp(extension,"avi")) || (!strcmp(extension,"flv")) ||
	   (!strcmp(extension,"mpg")) || (!strcmp(extension,"wmv")) ||
	   (!strcmp(extension,"mov")) || (!strcmp(extension,"mkv")) ||
	   (!strcmp(extension,"mp4")) || (!strcmp(extension,"vob")) {fnum = 14; ftype="Video";}	
	//папки
	IF (!strcmp(extension,"<DIR>"))	{ fnum=16;  WriteText(onLeft(145,0),yy+4,0x80,0,extension,0);}
	IF (!strcmp(extension,"..")) fnum=17;
	//
	PutPaletteImage(fnum*16*15+#ficons,16,15,195,yy,#ficons_pal);
	WriteText(onLeft(160,0),yy+4,0x80,0,ftype,0);
	return fnum;
}



void IconFairing(int filenum, y, color)
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
