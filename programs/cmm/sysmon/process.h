//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

#define T_WINDOW_TITLE "Process Manager"
#define T_SHOW_SYSTEM_PROCESSES "Show system"
#define T_DETAILS "Details"
#define T_END_PROCESS "End process"

#define BOTPANEL_H 36

enum {
	BTN_ID_SHOW_SYSTEM_PROCESSES=200,
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

void Processes__Main()
{
	int btn;
	maxcpu = GetCpuFrequency();
	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
	goto _PROCESS_REDRAW_3;
	loop()
	{
	  WaitEventTimeout(50);
	  switch(EAX & 0xFF)
	  {
	   	case evMouse:
			SelectList_ProcessMouse();
			break;
		case evKey:
			Sysmon__KeyEvent();
			if (key_scancode == SCAN_CODE_DEL) EventKillCurrentProcess();
			if (select_list.ProcessKey(key_scancode)) SelectList_LineChanged();
			break;
		case evButton:
			btn = Sysmon__ButtonEvent();

			if (show_system.click(btn)) {
				SelectList_LineChanged();
			}
			if (BTN_ID_KILL_PROCESS == btn) {
				EventKillCurrentProcess();
			}
			if (BTN_ID_SHOW_PROCESS_INFO == btn) {
				io.run("/sys/tinfo", itoa(GetProcessSlot(current_process_id))); 
			}
			break;
		case evReDraw: 
			_PROCESS_REDRAW_3:
			if (!Sysmon__DefineAndDrawWindow()) break;

			SelectList_Init(WIN_PAD, WIN_CONTENT_Y, 
				WIN_CONTENT_W-scroll1.size_x, 
				WIN_CONTENT_H-BOTPANEL_H-WIN_CONTENT_Y, false);
			SelectList_DrawBorder();

			//DrawWideRectangle(0, 0, Form.cwidth, Form.cheight, 4, sc.work);
			DrawBar(select_list.x-2, select_list.y+select_list.h+2, 
				select_list.w+scroll1.size_x+4, BOTPANEL_H, sc.work);
			DrawCaptButton(Form.cwidth-110-WIN_PAD,
				select_list.y+select_list.h+5,
				110,25,BTN_ID_KILL_PROCESS,0xF38181, 0xFFFfff, T_END_PROCESS);
			DrawCaptButton(Form.cwidth-230-WIN_PAD,
				select_list.y+select_list.h+5,
				110,25,BTN_ID_SHOW_PROCESS_INFO,
				sc.button, sc.button_text, T_DETAILS);
			show_system.draw(select_list.x + 3, select_list.y+select_list.h+10);
		default:
			SelectList_LineChanged();
	  }
	}
}

void EventKillCurrentProcess()
{
	KillProcess(current_process_id);
	pause(10);
	SelectList_LineChanged(); 
}

void Processes__GetProcessList()
{
	int i, j;
	proc_info Process;

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
	proc_info Process;

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

void SelectList_LineChanged() 
{
	Processes__GetProcessList();
	SelectList_Draw();
}
