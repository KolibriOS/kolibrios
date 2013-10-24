//@RB - v0.7

#define MEMSIZE 0x4000
#include "..\lib\kolibri.h" 
#include "..\lib\strings.h"
#include "..\lib\mem.h"
#include "..\lib\figures.h"
#include "..\lib\file_system.h"

#ifndef AUTOBUILD
#include "lang.h--"
#endif

#ifdef LANG_RUS
	char *ITEMS_LIST[]={
	"Настроить окна",   "/sys/desktop",       0,
	//"Сменить тему окон",   "/sys/SKINSEL",       0,
	//"Выбрать обои",        "/sys/BGSEL",         0,
	"Выбрать цвет фона",   "/sys/media/palitra", 0,
	"Управление иконками", "/sys/ICON",          0,
	"Настройка устройств", "/sys/SETUP",         0,
	"Обновить стол",       "/sys/REFRSCRN",      0,
	"Процессы",            "/sys/CPU",           0,
	0};
#else
	char *ITEMS_LIST[]={
	"Window skin",      "/sys/desktop",       0,
	//"Window skin",      "/sys/SKINSEL",       0,
	//"Wallpaper",        "/sys/BGSEL",         0,
	"Background",       "/sys/media/palitra", 0,
	"Icon manager",     "/sys/ICON",          0,
	"Device setup",     "/sys/SETUP",         0,
	"Refresh desktop ", "/sys/REFRSCRN",      0,
	"Processes",        "/sys/CPU",           0,
	0};
#endif

char stak[512];
#define ITEM_HEIGHT 18
int ITEM_WIDTH;


void main()
{
	mouse mm;
	byte i, can_show = 0;
	SetEventMask(100000b);
	for (i=0; ITEMS_LIST[i]!=0; i+=3) if (strlen(ITEMS_LIST[i])>ITEM_WIDTH) ITEM_WIDTH = strlen(ITEMS_LIST[i]);
	ITEM_WIDTH = ITEM_WIDTH * 6 + 20;
	mem_Init();
	
	loop() switch(WaitEvent())
	{
		case evMouse:
			mm.get();

			if (GetPointOwner(mm.x, mm.y)<>1) can_show = 0;
			if (mm.pkm) can_show = 1;
			if (!mm.pkm) && (can_show)
			{
				SwitchToAnotherThread();
				CreateThread(#window,#stak);
				can_show = 0;
			}
	}
}

	
void window()
{
	proc_info MenuForm;
	system_colors sc;
	mouse m;	
	int items_num, items_cur;
	int id1, key, i;
	
	sc.get();
	SetEventMask(100111b);
	
	loop() switch(WaitEvent())
	{
	case evMouse:
				if (!CheckActiveProcess(MenuForm.ID)) ExitProcess();
				m.get();
				id1=m.y-1/ITEM_HEIGHT;
				if (m.y<0) || (id1+1>items_num) || (m.x<0) || (m.x>ITEM_WIDTH) break;
				if (m.lkm) || (m.pkm)
				{
					//feel clicking
					DrawBar(1, items_cur*ITEM_HEIGHT+2, ITEM_WIDTH-1, ITEM_HEIGHT-2, sc.work_graph);
					WriteText(8,items_cur*ITEM_HEIGHT+6,0x80,sc.work_button_text,ITEMS_LIST[items_cur*3]);
					pause(4);
					
					ItemProcess(items_cur);
				}
				if (items_cur<>id1)
				{
					items_cur=id1;
					goto _ITEMS_DRAW;
				}
				
				break;
				
		case evButton:
				break;
				
		case evKey:
				key = GetKey();
				if (key==27) ExitProcess();
				if (key==178) && (items_cur)
				{
					items_cur--;
					goto _ITEMS_DRAW;
				}
				if (key==177) && (items_cur+1<items_num)
				{
					items_cur++;
					goto _ITEMS_DRAW;
				}
				if (key==13)
				{
					ItemProcess(items_cur);
				}
				break;
				
		case evReDraw:
				while (ITEMS_LIST[items_num*3]) items_num++;
				m.get();
				DefineAndDrawWindow(m.x+1,m.y,ITEM_WIDTH+1,items_num*ITEM_HEIGHT+2,0x01,sc.work,0, 0x01fffFFF);
				GetProcessInfo(#MenuForm, SelfInfo);
				DrawRectangle(0,0,ITEM_WIDTH,items_num*ITEM_HEIGHT+1,sc.work_graph); //юсюфюъ
				PutShadow(ITEM_WIDTH+1,1,1,items_num*ITEM_HEIGHT+1,0,1);
				PutShadow(1,items_num*ITEM_HEIGHT+2,ITEM_WIDTH+1,1,0,1);
				
				_ITEMS_DRAW:
				for (i=0; i<items_num; i++;)
				{
					if (i==items_cur)
					{
						DrawBar(1, i*ITEM_HEIGHT+1, ITEM_WIDTH-1, 1, sc.work_graph);
						DrawBar(1, i+1*ITEM_HEIGHT, ITEM_WIDTH-1, 1, 0xFFFfff);
						DrawBar(1, i*ITEM_HEIGHT+2, ITEM_WIDTH-1, ITEM_HEIGHT-2, sc.work_button);
						WriteText(8,i*ITEM_HEIGHT+6,0x80,sc.work_button_text,ITEMS_LIST[i*3]);
					}
					else
					{
						DrawBar(1, i*ITEM_HEIGHT+1, ITEM_WIDTH-1, ITEM_HEIGHT, sc.work);
						WriteText(8,i*ITEM_HEIGHT+6,0x80,sc.work_text,ITEMS_LIST[i*3]);
					}
				}
	}
}


void ItemProcess(int num_id)
{
	RunProgram(ITEMS_LIST[num_id*3+1], ITEMS_LIST[num_id*3+2]);
	ExitProcess();
}

stop:
