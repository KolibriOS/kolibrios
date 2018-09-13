#define MEMSIZE 4096*20

//===================================================//
//                                                   //
//                       LIB                         //
//                                                   //
//===================================================//

#include "../lib/gui.h"
#include "../lib/list_box.h"
#include "../lib/obj/box_lib.h"
#include "../lib/io.h"
#include "../lib/patterns/select_list.h"
#include "../lib/patterns/restart_process.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

#define T_WINDOW_TITLE "Process Manager"
#define T_SHOW_SYSTEM_PROCESSES "Show system"
#define T_DETAILS "Details"
#define T_END_PROCESS "End process"


#define BOTPANEL_H 34
proc_info Form;
proc_info Process;

enum {
	BTN_ID_SHOW_SYSTEM_PROCESSES=20,
	BTN_ID_KILL_PROCESS,
	BTN_ID_SHOW_PROCESS_INFO
};

int current_process_id = 0;
unsigned maxcpu;
int proc_list[256];

checkbox show_system = { T_SHOW_SYSTEM_PROCESSES, false };

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void main()
{
	int btn;
	load_dll(boxlib, #box_lib_init,0);
	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
	maxcpu = GetCpuFrequency();
	loop()
	{
	  WaitEventTimeout(50);
	  switch(EAX & 0xFF)
	  {
	   	case evMouse:
			if (!CheckActiveProcess(Form.ID)) break;
			SelectList_ProcessMouse();
			break;
		case evKey:
			GetKeys();
			if (select_list.ProcessKey(key_scancode)) SelectList_LineChanged();
			break;
		case evButton:
			btn = GetButtonID();
			if (1 == btn) 
			{
				ExitProcess();
			}
			if (show_system.click(btn))  
			{
				SelectList_LineChanged();
			}
			if (BTN_ID_KILL_PROCESS == btn)  
			{
				KillProcess(current_process_id);
				pause(10);
				SelectList_LineChanged(); 
			}
			if (BTN_ID_SHOW_PROCESS_INFO == btn)  
			{
				io.run("/sys/tinfo", itoa(GetProcessSlot(current_process_id))); 
			}
			break;
		case evReDraw:
			system.color.get();
			DefineAndDrawWindow(screen.width-400/2,screen.height-450/2,400,454,0x73,0,T_WINDOW_TITLE,0);
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window>2) break;
			if (Form.width  < 300) { MoveSize(OLD,OLD,300,OLD); break; }
			if (Form.height < 200) { MoveSize(OLD,OLD,OLD,200); break; }
			SelectList_Init(6, 6, Form.cwidth-12 - scroll1.size_x, Form.cheight-12-BOTPANEL_H, false);
			SelectList_DrawBorder();
			DrawWideRectangle(0, 0, Form.cwidth, Form.cheight, 4, system.color.work);
			DrawBar(select_list.x-2, select_list.y+select_list.h+2, 
				select_list.w+scroll1.size_x+4, BOTPANEL_H, system.color.work);
			DrawCaptButton(Form.cwidth-116,
				select_list.y+select_list.h+5,
				110,25,BTN_ID_KILL_PROCESS,0xF38181, 0xFFFfff, T_END_PROCESS);
			DrawCaptButton(Form.cwidth-236,
				select_list.y+select_list.h+5,
				110,25,BTN_ID_SHOW_PROCESS_INFO,
				system.color.work_button, system.color.work_button_text, T_DETAILS);
			show_system.draw(select_list.x + 3, select_list.y+select_list.h+10);
		default:
			SelectList_LineChanged();
	  }
	}
}

void SelectList_LineChanged() 
{
	GetProcessList();
	SelectList_Draw();
}


void GetProcessList()
{
	int i, j;
	select_list.count=0;
	for (i=0; i<MAX_PROCESS_COUNT; i++)
	{
		GetProcessInfo(#Process, i);
		if (Process.name) 
		{
			for (j=0; j<11; j++) if (Process.name[j]!=' ') { 
				if (show_system.checked==false) {
					//do not show system process
					if (Process.name[0]=='@') break;
					if (!strcmp(#Process.name, "IDLE")) break;
					if (!strcmp(#Process.name, "OS")) break;
				}
				proc_list[select_list.count] = i;
				select_list.count++;
				break; 
			}
		}
	}
}

void SelectList_DrawLine(dword i)
{
	int posy;
	char cpu_use[16];
	dword bg_color;
	GetProcessInfo(#Process, proc_list[i+select_list.first]);
	
	posy = i *select_list.item_h + select_list.y;
	if (i % 2) bg_color = 0xFFFfff; else bg_color = 0xF0F0F0;
	if (i+select_list.first == select_list.cur_y) {
		current_process_id = Process.ID; 
		bg_color = 0x67CCEB;
	}
	DrawBar(select_list.x, posy, select_list.w, select_list.item_h, bg_color);
	WriteText(select_list.x+005, posy+select_list.text_y, select_list.font_type, 0, #Process.name);
	WriteText(select_list.w/10*5+select_list.x, posy+select_list.text_y, select_list.font_type, 0x444444, ConvertSizeToKb(Process.use_memory));
	sprintf(#cpu_use, "%i %%", Process.use_cpu*100/maxcpu);
	if (maxcpu) WriteText(select_list.w/10*8+select_list.x - calc(strlen(#cpu_use)-4*8), 
		posy+select_list.text_y, select_list.font_type, 0x444444, #cpu_use);
}






stop: