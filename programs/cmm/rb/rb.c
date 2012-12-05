//@RB - v0.6

#include "..\lib\kolibri.h" 
#include "..\lib\figures.h"
#include "..\lib\file_system.h"

#include "lang.h--" 

#ifdef LANG_RUS
	#define ITEM_HEIGHT 18
	#define ITEM_WIDTH  138
	char *ITEMS_LIST[]={
	"Сменить тему окон   ", "/sys/SKINSEL",       0,
	"Выбрать цвет фона   ", "/sys/media/palitra", 0,
	"Управление иконками ", "/sys/ICON",          0,
	"Настройка устройств ", "/sys/SETUP",         0,
	"Обновить стол       ", "/sys/REFRSCRN",      0,
	"Процессы            ", "/sys/CPU",           0,
	0};
#else
	#define ITEM_HEIGHT 18
	#define ITEM_WIDTH  122
	char *ITEMS_LIST[]={
	"Window skin         ", "/sys/SKINSEL",       0,
	"Background          ", "/sys/media/palitra", 0,
	"Icon manager        ", "/sys/ICON",          0,
	"Device setup        ", "/sys/SETUP",         0,
	"Refresh desktop     ", "/sys/REFRSCRN",      0,
	"Processes           ", "/sys/CPU",           0,
	0};
#endif

dword stak[100];


void main()
{
	mouse mm;
	byte thread_id;
	SetEventMask(100000b);
	
	loop() switch(WaitEvent())
	{
		case evMouse:
			mm.get();

			if (GetPointOwner(mm.x, mm.y)==1) && (mm.pkm)
			{
				SwitchToAnotherThread();
				CreateThread(#window,#stak);
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
				m.get();

				GetProcessInfo(#MenuForm, SelfInfo);
				id1=GetProcessSlot(MenuForm.ID);
				if (id1<>GetActiveProcess()) ExitProcess();			
				id1=m.y-1/ITEM_HEIGHT;
				if (m.y<0) || (id1+1>items_num) || (m.x<0) || (m.x>ITEM_WIDTH) break;
				if (m.lkm) || (m.pkm)
				{
					//feel clicking
					DrawBar(1, items_cur*ITEM_HEIGHT+2, ITEM_WIDTH-1, ITEM_HEIGHT-2, sc.work_graph);
					WriteText(8,items_cur*ITEM_HEIGHT+6,0x80,sc.work_button_text,ITEMS_LIST[items_cur*3],0);
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
				DefineAndDrawWindow(m.x+1,m.y,ITEM_WIDTH,items_num*ITEM_HEIGHT+1,0x01,sc.work,0, 0x01fffFFF);
				DrawRectangle(0,0,ITEM_WIDTH,items_num*ITEM_HEIGHT+1,sc.work_graph); //юсюфюъ
				
				_ITEMS_DRAW:
				for (i=0; i<items_num; i++;)
				{
					if (i==items_cur)
					{
						DrawBar(1, i*ITEM_HEIGHT+1, ITEM_WIDTH-1, 1, sc.work_graph);
						DrawBar(1, i+1*ITEM_HEIGHT, ITEM_WIDTH-1, 1, 0xFFFfff);
						DrawBar(1, i*ITEM_HEIGHT+2, ITEM_WIDTH-1, ITEM_HEIGHT-2, sc.work_button);
						WriteText(8,i*ITEM_HEIGHT+6,0x80,sc.work_button_text,ITEMS_LIST[i*3],0);
					}
					else
					{
						DrawBar(1, i*ITEM_HEIGHT+1, ITEM_WIDTH-1, ITEM_HEIGHT, sc.work);
						WriteText(8,i*ITEM_HEIGHT+6,0x80,sc.work_text,ITEMS_LIST[i*3],0);
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
