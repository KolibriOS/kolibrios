#define MEMSIZE 0x3E80
#include "..\lib\kolibri.h" 
#include "..\lib\figures.h" 
#include "..\lib\strings.h" 
#include "..\lib\random.h" 
#include "..\lib\mem.h" 
#include "..\lib\file_system.h"

#ifndef ru
	char *BUTTONS_CAPTIONS[]={
	"Перезагрузка    [Enter]"w, 13,
	"Выключение        [End]"w, 181,
	//"Ядро             [Home]"w, 180,
	"Отмена            [Esc]"w, 27,
	0};
#else
	char *BUTTONS_CAPTIONS[]={
	" Reboot        [Enter]",13,
	" Power off       [End]",181,
	//" Kernel         [Home]",180,
	" Close           [Esc]",27,
	0};
#endif

system_colors sc;

unsigned char moon[6*6] = FROM "moon.raw";

int WIN_SIZE_X, WIN_SIZE_Y;
int PANEL_X, PANEL_Y;

#define NIGHT_PALEL_HEIGHT	50
#define STARS_COUNT			25

#define PANEL_SIZE_X		260
#define PANEL_SIZE_Y		148

:void ShadowScreen(dword img, w, h)
{
	dword to = w*h*3 + img;
	for ( ; img < to; img+=4)
	{
		ESDWORD[img] >>= 1;
		$and ESDWORD[img], 7F7F7F7Fh
	}
	//for ( ; img < to; img+=4) { ESDWORD[img] >>= 2;	$and ESDWORD[img], 3F3F3F3Fh }
}


void main()
{   
	int key;
	dword s1,s2, s3, s4, sides_w;

	mem_Init();
	WIN_SIZE_X=GetScreenWidth()+1;
	WIN_SIZE_Y=GetScreenHeight()+1;
	PANEL_X=WIN_SIZE_X-PANEL_SIZE_X/2;
	PANEL_Y=WIN_SIZE_Y-PANEL_SIZE_Y/2;

	sides_w = WIN_SIZE_X-PANEL_SIZE_X/2;
	s1 = mem_Alloc(WIN_SIZE_X*PANEL_Y*3);
	s2 = mem_Alloc(sides_w*PANEL_Y*3);
	s3 = mem_Alloc(sides_w*PANEL_Y*3);
	s4 = mem_Alloc(WIN_SIZE_X*PANEL_Y*3);	

	CopyScreen(s1, 0, 0, WIN_SIZE_X, PANEL_Y);
	ShadowScreen(s1, WIN_SIZE_X, PANEL_Y);

	CopyScreen(s2, 0, PANEL_Y, sides_w, PANEL_SIZE_Y+1);
	ShadowScreen(s2, sides_w, PANEL_SIZE_Y+1);

	CopyScreen(s3, sides_w+PANEL_SIZE_X+1, PANEL_Y, sides_w-1, PANEL_SIZE_Y+1);
	ShadowScreen(s3, sides_w, PANEL_SIZE_Y+1);

	CopyScreen(s4, 0, PANEL_Y+PANEL_SIZE_Y+1, WIN_SIZE_X, PANEL_Y-1);
	ShadowScreen(s4, WIN_SIZE_X, PANEL_Y-1);


	goto _DRAW;
	loop()
   {
		WaitEventTimeout(330);
		switch(EAX & 0xFF)
		{
		case evButton:
			key=GetButtonID();               
			if (key==1) ExitProcess();
			GOTO _BUTTON_MARK;
			
		case evKey:
			key = GetKey();
			_BUTTON_MARK:
			if (key== 13) ExitSystem(REBOOT);   //ENTER
			if (key==180) ExitSystem(KERNEL);   //HOME
			if (key==181) ExitSystem(TURN_OFF); //END
			if (key== 27) ExitProcess();        //ESC
			if (key== 19) //CTRL+S
			{
				RunProgram("rdsave",0);
				ExitProcess();
			}
			break;
         
		case evReDraw:
			sc.get();
			DefineAndDrawWindow(0,0,WIN_SIZE_X, WIN_SIZE_Y, 0x01, 0, 0, 0x01fffFFF);
			//_PutImage(0,0,WIN_SIZE_X,WIN_SIZE_Y,shadow_buf);
			_PutImage(0,0,WIN_SIZE_X, PANEL_Y,s1);
			draw_main_area(PANEL_X, PANEL_Y, PANEL_SIZE_X, PANEL_SIZE_Y);
			_PutImage(0,PANEL_Y,sides_w, PANEL_SIZE_Y+1,s2);
			_PutImage(sides_w+PANEL_SIZE_X+1,PANEL_Y,sides_w-1, PANEL_SIZE_Y+1,s3);
			_PutImage(0,PANEL_Y+PANEL_SIZE_Y+1,WIN_SIZE_X, PANEL_Y-1,s4);
			break;
		default: _DRAW:
			draw_stars();
      }
   }
}


void draw_main_area()
{
	int i=0;
	
	DrawRectangle(PANEL_X, PANEL_Y, PANEL_SIZE_X, PANEL_SIZE_Y, 0);
	DrawBar(PANEL_X+1, PANEL_Y+NIGHT_PALEL_HEIGHT+1, PANEL_SIZE_X-1, PANEL_SIZE_Y-NIGHT_PALEL_HEIGHT-1, sc.work);
	
	for (i=0; i<3; i++)
	{
		DefineButton(PANEL_X+33, i*23 + PANEL_Y+NIGHT_PALEL_HEIGHT+16, 190,19, BUTTONS_CAPTIONS[i*2+1],sc.work_button);
		WriteText(PANEL_X+59, i*23 + PANEL_Y+NIGHT_PALEL_HEIGHT+22, 0x80,sc.work_button_text, BUTTONS_CAPTIONS[i*2]);
	}
		
	draw_stars();
}

dword stars_col[4]={0xD2CF19, 0x716900, 0x002041, 0xEAE0DE}; //0x005BFF - голубой, редко

void draw_stars()
{

	int i, x_pic, y_pic, col;
	
	DrawBar(PANEL_X+1, PANEL_Y+1, PANEL_SIZE_X-1, NIGHT_PALEL_HEIGHT, 0x002041);
	
	for (i=0; i<STARS_COUNT; i++)
	{
		x_pic = random(PANEL_SIZE_X-2);
		y_pic = random(NIGHT_PALEL_HEIGHT-2);
		col = random(4);
		PutPixel(PANEL_X+2 +x_pic, PANEL_Y+2 +y_pic, stars_col[col]);
		if (stars_col[col]==0xD2CF19)
		{
			PutPixel(PANEL_X+2 +x_pic+1, PANEL_Y+2 +y_pic, stars_col[col+1]);
			PutPixel(PANEL_X+2 +x_pic-1, PANEL_Y+2 +y_pic, stars_col[col+1]);
			PutPixel(PANEL_X+2 +x_pic, PANEL_Y+2 +y_pic-1, stars_col[col+1]);
			PutPixel(PANEL_X+2 +x_pic, PANEL_Y+2 +y_pic+1, stars_col[col+1]);
		}

	}
	_PutImage(PANEL_X+PANEL_SIZE_X-60+random(3),PANEL_Y+10+random(3), 6,6, #moon);
}





stop:
