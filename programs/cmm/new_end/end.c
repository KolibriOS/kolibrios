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
	"Ядро             [Home]"w, 180,
	"Отмена            [Esc]"w, 27,
	0};
#else
	char *BUTTONS_CAPTIONS[]={
	" Reboot        [Enter]",13,
	" Power off       [End]",181,
	" Kernel         [Home]",180,
	" Close           [Esc]",27,
	0};
#endif

system_colors sc;

unsigned char moon[6*6] = FROM "moon.raw";

int WIN_SIZE_X, WIN_SIZE_Y;
int PANEL_X, PANEL_Y;

#define NIGHT_PALEL_HEIGHT	45
#define STARS_COUNT			30

#define PANEL_SIZE_X		260
#define PANEL_SIZE_Y		165

dword stars_col[4]={0xD2CF19, 0x716900, 0x002041}; //0x005BFF - голубой, редко




void main()
{   
	int key;
	
	Pause(10);
	
	WIN_SIZE_X=GetScreenWidth()+1;
	WIN_SIZE_Y=GetScreenHeight()+1;

	PANEL_X=WIN_SIZE_X-PANEL_SIZE_X/2;
	PANEL_Y=WIN_SIZE_Y-PANEL_SIZE_Y/2;
	
	loop()
   {
		WaitEventTimeout(130);
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
			draw_window();
			break;
		default:
			draw_stars();
      }
   }
}



void draw_window()
{
	int x, y;
	sc.get();
	
	DefineAndDrawWindow(0,0,WIN_SIZE_X, WIN_SIZE_Y, 0x01, 0, 0, 0x01fffFFF);

	draw_main_area(PANEL_X, PANEL_Y, PANEL_SIZE_X, PANEL_SIZE_Y);

	//draw grid
	for (y=0; y<WIN_SIZE_Y; y++)
	{
		for (x=0; x<WIN_SIZE_X; x++)	
		{
			if (y>PANEL_Y) && (y<PANEL_Y+PANEL_SIZE_Y) && (x==PANEL_X) x+=PANEL_SIZE_X; //на панели не рисуем
			if (! y&1) && (! x&1) {PutPixel(x, y, 0); continue;}
			if (  y&1) && (  x&1) PutPixel(x, y, 0);
		}
	}
}



void draw_main_area()
{
	int i=0;
	
	DrawRectangle(PANEL_X, PANEL_Y, PANEL_SIZE_X, PANEL_SIZE_Y, 0);
	DrawBar(PANEL_X+1, PANEL_Y+NIGHT_PALEL_HEIGHT+1, PANEL_SIZE_X-1, PANEL_SIZE_Y-NIGHT_PALEL_HEIGHT-1, sc.work);
	
	for (i=0; i<4; i++)
	{
		DefineButton(PANEL_X+33, i*23 + PANEL_Y+NIGHT_PALEL_HEIGHT+16, 190,19, BUTTONS_CAPTIONS[i*2+1],sc.work_button);
		WriteText(PANEL_X+59, i*23 + PANEL_Y+NIGHT_PALEL_HEIGHT+22, 0x80,sc.work_button_text, BUTTONS_CAPTIONS[i*2], 0);
	}
		
	draw_stars();
}

void draw_stars()
{

	int i, x_pic, y_pic, col;
	
	DrawBar(PANEL_X+1, PANEL_Y+1, PANEL_SIZE_X-1, NIGHT_PALEL_HEIGHT, 0x002041);
	
	for (i=0; i<STARS_COUNT; i++)
	{
		x_pic = random(PANEL_SIZE_X-1);
		y_pic = random(NIGHT_PALEL_HEIGHT-1);
		col = random(3);
		PutPixel(PANEL_X+1 +x_pic, PANEL_Y+1 +y_pic, stars_col[col]);
	}
	/*for (i=0; i<3; i++)
	{
		x_pic = random(PANEL_SIZE_X-8)+4;
		y_pic = random(NIGHT_PALEL_HEIGHT-8)+4;
		
		PutPixel(PANEL_X +x_pic, PANEL_Y +y_pic, stars_col[0]);
		PutPixel(PANEL_X+1 +x_pic, PANEL_Y +y_pic, stars_col[1]);
		PutPixel(PANEL_X-1 +x_pic, PANEL_Y +y_pic, stars_col[1]);
		PutPixel(PANEL_X +x_pic, PANEL_Y +y_pic+1, stars_col[1]);
		PutPixel(PANEL_X +x_pic, PANEL_Y +y_pic-1, stars_col[1]);
	}*/
	
	PutImage(#moon,6,6, PANEL_X+PANEL_SIZE_X-60,PANEL_Y+10);
}





stop:
