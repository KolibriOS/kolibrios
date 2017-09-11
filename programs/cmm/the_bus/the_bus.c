/*
THE BUS
Copyright (C) 2008, 2012, 2017 Leency
Menu image from Freepik.com
D O N K E Y
Copyright (C) 2008 O.Bogomaz
*/

#define MEMSIZE 1024 * 60;

#include "..\lib\kolibri.h"
#include "..\lib\random.h"

#include "..\lib\obj\libio_lib.h"
#include "..\lib\obj\libimg_lib.h"
#include "..\lib\patterns\libimg_load_skin.h"

#define SCALE 2

#include "draw_scalled.h"

libimg_image menu;
libimg_image road;

int lives=0, level=0, score=0;
int don_x, don_y, don_h=40, don_type;
int bus_x, bus_y=147;

#define SCR_MENU_MAIN   1
#define SCR_GAME        2
#define SCR_PAUSE       3


#define WIN_X 258 * SCALE
#define WIN_Y 191 * SCALE

int screen_type=SCR_MENU_MAIN;

#define COLOR_ROAD 0x6D879B;

int active_menu_item=0;

char *ITEMS_LIST[]={
"New game",
"Control keys",
"About",
"Exit",
0}; 


//actually it has to be named DrawObstacle(), but I like Donkey more :)
void DrawDonkey(signed int x, y) { 
	int don_offset_x;
	int don_offset_y;
	int image_h;

	if (don_type<4) {
		don_offset_y=0;
		don_offset_x=don_type*40;
	}
	else {
		don_offset_y = don_h + 1;
		don_offset_x=don_type-3*40;
	}

	image_h = don_h;
	if (y < 0) {
		image_h = don_h + y+1;
		don_offset_y = don_h + y;
		y = 0;
	}

	DrawScaledImage(road.image, x, y, 39, image_h, don_offset_x, don_offset_y); 
}
void DrawBus(dword x, y) { DrawScaledImage(road.image, x, y, 21, 45, 237, 0); }
void DrawBoom(dword x, y) { DrawScaledImage(road.image, x, y, 39, 34, 208, 47); }
void DrawHighway() { DrawScaledImage(road.image, 0,0, WIN_X, WIN_Y, 0, 82); }
void DrawMenuBackground() { DrawScaledImage(menu.image, 0, 0, WIN_X, WIN_Y, 0, 0); }

void main()
{
	randomize();

	load_dll(libio,  #libio_init,1);
	load_dll(libimg, #libimg_init,1);
	Libimg_LoadImage(#menu, "/sys/fonts/home2x.png");
	Libimg_LoadImage(#road, "/sys/fonts/busimg2x.png");
	
	loop()
	{
		WaitEventTimeout(1);

		switch(EAX & 0xFF)
		{
			case evKey: 
				GetKeys();
				if (key_scancode == SCAN_CODE_ESC)
				{
					if (screen_type==SCR_GAME) SetScreen(SCR_MENU_MAIN);
					else if (screen_type==SCR_MENU_MAIN) ExitProcess();
				}
				if (key_scancode == SCAN_CODE_DOWN) && (screen_type==SCR_MENU_MAIN)
				{
					if (active_menu_item<>3) active_menu_item++; ELSE active_menu_item=0;
					DrawMenuList();
				}
				if (key_scancode == SCAN_CODE_UP) && (screen_type==SCR_MENU_MAIN)
				{
					if (active_menu_item<>0) active_menu_item--; ELSE active_menu_item=3;
					DrawMenuList();
				}
				if (key_scancode == SCAN_CODE_ENTER) && (screen_type==SCR_MENU_MAIN)
				{
					if (active_menu_item==0)
					{
						lives=3;
						level=0;
						score=0;
						SetScreen(SCR_GAME);
					}
					if (active_menu_item==1) notify("'The Bus\nControl keys:\nLeft, Right, Space\nPress P key for pause'tI");
					if (active_menu_item==2) notify("'The Bus\nVersion v0.2 Alpha\n\nAuthor: Leency\nMenu image from Freepik.com'tI");
					if (active_menu_item==3) ExitProcess();	
				}				
				if (key_scancode == SCAN_CODE_SPACE) && (screen_type==SCR_GAME)
				{
					DrawScaledBar(bus_x*40+100,147, 21,45, COLOR_ROAD);
					if (bus_x==1) bus_x=0; else bus_x=1;
				}
				if (key_scancode == SCAN_CODE_LEFT) && (screen_type==SCR_GAME)
				{
					if (bus_x==0) break;
					DrawScaledBar(bus_x*40+100,147, 21,45, COLOR_ROAD);
					bus_x=0;
				}
				if (key_scancode == SCAN_CODE_RIGHT) && (screen_type==SCR_GAME)
				{
					if (bus_x==1) break;
					DrawScaledBar(bus_x*40+100,147, 21,45, COLOR_ROAD);
					bus_x=1;
				}
				if (key_scancode == SCAN_CODE_KEY_P)
				{
					if (screen_type==SCR_MENU_MAIN) break;
					else if (screen_type==SCR_GAME) SetScreen(SCR_PAUSE); 
					else if (screen_type==SCR_PAUSE) SetScreen(SCR_GAME); 
				}
				break;
				
			case evReDraw:
				DefineAndDrawWindow(250,150,WIN_X-1+10,WIN_Y-1+skin_height+7,0x74,0,"The Bus",0);
				DrawScreen();
				break;
				
			case evButton:
				ExitProcess();
				break;
				
			default:
				if (screen_type==SCR_GAME) DrawRoad();
				break;
		}
	}
}

void WriteScore() {
	DrawScaledImage(road.image, 10, 83, 60, 12, 10, 83+82);
	WriteScaledText(10, 70, 0x80, 0xFFFFFF, "Score");
	WriteScaledText(10, 83, 0x80, 0xFFFFFF, itoa(score));
}

void SetScreen(dword _screen_type) {
	screen_type = _screen_type;
	DrawScreen();
}

void DrawScreen()
{
	if (screen_type==SCR_MENU_MAIN) 
	{
		DrawMenuBackground();
		WriteScaledText(10, 10, 0x80, 0xE8783F, "TAKE THE CHILDREN HOME");
		$add ebx, 2 << 16
		$int 64
		DrawMenuList();
	}
	if (screen_type==SCR_GAME) || (screen_type==SCR_PAUSE) 
	{
		DrawHighway();
		WriteScaledText(10, 10, 0x80, 0xFFFFFF, "Lives");
		WriteScaledText(10, 23, 0x80, 0xFFFFFF, itoa(lives));
		WriteScaledText(10, 40, 0x80, 0xFFFFFF, "Level");
		WriteScaledText(10, 53, 0x80, 0xFFFFFF, itoa(level));
		WriteScore();
		DrawRoad();
		if (screen_type==SCR_PAUSE) {
			DrawScaledBar(0,0,70,30,0xFF0000);
			WriteScaledText(5,7,0x81,0xFFFfff,"PAUSE");			
		}	
	}
}


void DrawMenuList()
{
	int j;
	for (j=0; j<4; j++) DrawMenuItem(j, j);
}

void DrawMenuItem(int item_n, text_n)
{
	dword color;
	if (active_menu_item==item_n) color = 0xFF0000; else color = 0xFFffff;
	WriteScaledText(10+1, item_n*28+48+1, 0x80, 0xAAAaaa, ITEMS_LIST[text_n]);
	WriteScaledText(10, item_n*28+48, 0x80, color, ITEMS_LIST[text_n]);
}


void DrawGameOverMessage()
{
	DrawScaledBar(0, 0, WIN_X, WIN_Y, 0xF3E1BD);
	WriteScaledText(20, 20, 0x80, 0xA48C74, "GAME OVER");
	WriteScaledText(20, 40, 0x80, 0xA48C74, "FINAL SCORE");
	WriteScaledText(20, 70, 0x84, 0xA48C74, itoa(score));
	$add ecx, 2 << 16
	$int 64
	pause(350);	
	active_menu_item=0;
	SetScreen(SCR_MENU_MAIN);
	return;	
}

void DrawAccident()
{
	DrawBus(bus_x*40+100,bus_y);
	DrawBoom(bus_x*40+90,152);
	pause(150);
	lives--;
	don_y = -don_h;
	DrawScreen();
	if (lives>0) DrawScaledBar(bus_x*40+90, 147-17, 39, 45+23, COLOR_ROAD);
}

#define LINE_LENGTH 10
void DrawRoad()
{
	byte y,line_y;
	
	if ((don_x == bus_x)&&(don_y + don_h > bus_y )&&(don_y + don_h < bus_y )) DrawAccident();
	
	if (lives==0) {
		DrawGameOverMessage();
		return;
	}
	
	if (screen_type != SCR_PAUSE) 
	{
		line_y+=2;
		don_y+=2;
	}

	//the beginning of the white dashed line between two roadways
	if (line_y>=20) {
		line_y=0;
	}
	else 
	{ 
		DrawScaledBar(129, 0, 1, line_y, COLOR_ROAD); 
		DrawScaledBar(129, 0, 1, line_y-LINE_LENGTH, 0xDDE9F2); 
	}
	for (y=0; y<190; y+=20) //white dashed line between two roadways
	{
		DrawScaledBar(129, line_y+y, 1, LINE_LENGTH, 0xDDE9F2);
		DrawScaledBar(129, line_y+y+LINE_LENGTH, 1, LINE_LENGTH, COLOR_ROAD); 
	}
	if (don_y>=210)
	{
		don_x = random(2);
		don_y = -don_h;
		don_type = random(7);
		score++;
		WriteScore();
	}
	DrawScaledBar(don_x*don_h+93, don_y-2, 30, 2, COLOR_ROAD); //Fill donkey old parts
	DrawDonkey(don_x*don_h+90,don_y);
	DrawBus(bus_x*don_h+100,147);
}





stop:
