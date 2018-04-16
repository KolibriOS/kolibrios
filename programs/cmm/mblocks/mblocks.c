/*
   Memory Blocks for KolibriOS v1.1
        Leency&Veliant Edition
              2008-2017
*/

#define MEMSIZE 4096 * 15
#include "..\lib\gui.h"
#include "..\lib\random.h"

#include "..\lib\obj\libio.h"
#include "..\lib\obj\libimg.h"

proc_info Form;

#ifndef AUTOBUILD
#include "lang.h--"
#endif

#define COLOR_CELL_BG 0xFFFfff
#define COLOR_CELL_BORDER 0x94AECE
#define CELL_SIZE 43
#define PANEL_Y CELL_SIZE+4*6 + 4
#define PANEL_H 36

#define strok 6      //cell count x
#define stolbcov 10  //cell count y

#ifdef LANG_RUS
	#define LABEL_NEW_GAME "Новая игра (F2)";
#else
	#define LABEL_NEW_GAME "New game (F2)";
#endif

int bitstat[60], bitpict[60];
dword butonsx[60], butonsy[60];
dword firstbit, secondbit;
int i, count, lang;


void main()
{   
	dword id;
	load_dll(libio,  #libio_init,1);
	load_dll(libimg, #libimg_init,1);

	Libimg_LoadImage(#skin, "/sys/icons32.png");
	Libimg_FillTransparent(skin.image, skin.w, skin.h, COLOR_CELL_BG);

	NewGame();

	loop() switch(WaitEvent())
	{
		case evKey:
			GetKeys();
		 	if (key_scancode==60) NewGame();
		 	break;

		case evButton:
				id = GetButtonID();
				if (id==1) ExitProcess();
				else if (id==5) NewGame();
				else {
						if (bitstat[id-100] == 0)
						{
							if (firstbit <> 0x0BAD)
							{
								if (secondbit <> 0x0BAD)
								{
									if (bitpict[firstbit-100] == bitpict[secondbit-100])
										bitstat[firstbit-100] = bitstat[secondbit-100] = 2;
									else
										bitstat[firstbit-100] = bitstat[secondbit-100] = 0;
									ReDraw_Game_Button(firstbit - 100);
									ReDraw_Game_Button(secondbit - 100);
									secondbit = 0x0BAD;
									firstbit = id;
									bitstat[id-100] = 1;
									ReDraw_Game_Button(id - 100);
									count++;
								}
								else if (firstbit<>id)
								{
									secondbit = id;
									bitstat[id-100] = 1;
									ReDraw_Game_Button(id - 100);
									count++;
								}
							}
							else
							{
								firstbit = id;
								bitstat[id-100] = 1;
								ReDraw_Game_Button(id - 100);
								count++;
							}
						}
						Draw_Count();
				}
				break;

		case evReDraw:
			system.color.get();
			DefineAndDrawWindow(215,100,CELL_SIZE+4*10 + 4 + 9,PANEL_Y + 4 + PANEL_H +skin_height,0x34,0xC0C0C0,"Memory Blocks",0);
			GetProcessInfo(#Form, SelfInfo);
			Draw_Panel();
			Draw_Game_Pole();
			break;
	}
}

void NewGame()
{
	int off;

	FOR (i = 0; i < 60; i++)
	{
		bitpict[i] = 0;
		bitpict[i] = 0;
	}

	count = 0;
	firstbit = secondbit = 0x0BAD;
	FOR (i = 0; i < 30; i++)
	{
		do off = random(60); while (bitpict[off] != 0);
		bitpict[off] = i;
		do off = random(60); while (bitpict[off] != 0);
		bitpict[off] = i;
	}
}

void ReDraw_Game_Button(int id)
{
	DefineButton(butonsx[id], butonsy[id], CELL_SIZE, CELL_SIZE, 100 + id + BT_HIDE, 0);
	switch (bitstat[id])
	{
		case 0:
			Draw_Block(butonsx[id], butonsy[id]);
			break;
		case 1:
			Draw_Pressed_Block(butonsx[id], butonsy[id]);
			img_draw stdcall(skin.image, butonsx[id]+6, butonsy[id]+6, 32, 32, 0, bitpict[id]*32);
			BREAK;
		case 2:
			Draw_Open_Block(butonsx[id], butonsy[id]);
			img_draw stdcall(skin.image, butonsx[id]+6, butonsy[id]+6, 32, 32, 0, bitpict[id]*32);
			BREAK;
	}
}

void Draw_Game_Pole()
{
	byte j;
	for (j = 0; j < stolbcov; j++)	for (i = 0; i < strok; i++)
	{
			butonsx[j*strok+i] = CELL_SIZE+4 * j + 4; //save coordinates to avoid their recalculation after
			butonsy[j*strok+i] = CELL_SIZE+4 * i + 4;
			ReDraw_Game_Button(j*strok + i);
	}
}

void Draw_Block(dword x, y)
{
	DrawRectangle(x, y, CELL_SIZE, CELL_SIZE, COLOR_CELL_BORDER);//border
	DrawRectangle3D(x + 1, y + 1, CELL_SIZE-2, CELL_SIZE-2, 0xFFFFFF, 0xDEDEDE);//bump
	DrawBar(x + 2, y + 2, CELL_SIZE-3, CELL_SIZE-3, 0xBDC7D6);//background
}

void Draw_Open_Block(dword x, y)
{
	DrawRectangle(x, y, CELL_SIZE, CELL_SIZE, COLOR_CELL_BORDER);//border
	DrawBar(x + 1, y + 1, CELL_SIZE-1, CELL_SIZE-1, COLOR_CELL_BG);//background
}

void Draw_Pressed_Block(dword x, y)
{
	DrawRectangle(x, y, CELL_SIZE, CELL_SIZE, COLOR_CELL_BORDER);//border
	DrawWideRectangle(x + 1, y + 1, CELL_SIZE-1, CELL_SIZE-1, 2, 0x94DB00);//border green
	DrawBar(x + 3, y + 3, CELL_SIZE-5, CELL_SIZE-5, COLOR_CELL_BG);//background
}

void Draw_Panel()
{
	DrawBar(0, PANEL_Y, Form.cwidth, 1, system.color.work_dark);
	DrawBar(0, PANEL_Y+1, Form.cwidth, 1, system.color.work_light);
	DrawBar(0, PANEL_Y+2, Form.cwidth, PANEL_H-2, system.color.work);
	DrawStandartCaptButton(9, PANEL_Y+5, 5, LABEL_NEW_GAME);
	Draw_Count();
}

void Draw_Count()
{
	DrawBar(Form.cwidth-32,PANEL_Y + 12,30,12,system.color.work);
	WriteNumber(Form.cwidth-32, PANEL_Y + 12, 0x90, system.color.work_text, 3, count);
}




stop:
