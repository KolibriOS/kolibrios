/*
 * System Monitor
 * version 1.41
 * Author: Leency
*/

#define MEMSIZE 1024*60
//#define NO_DLL_INIT

//===================================================//
//                                                   //
//                       LIB                         //
//                                                   //
//===================================================//

#include "../lib/gui.h"
#include "../lib/fs.h"
#include "../lib/list_box.h"

#include "../lib/obj/box_lib.h"
#include "../lib/obj/libini.h"

#include "../lib/patterns/select_list.h"
#include "../lib/patterns/restart_process.h"

//===================================================//
//                                                   //
//                      CONST                        //
//                                                   //
//===================================================//

#define GAP 16   //Window padding
#define WIN_CONTENT_X GAP
#define WIN_CONTENT_Y GAP+15
#define PROCESS_LIST_W 260
#define RIGHT_X PROCESS_LIST_W + GAP + GAP + 22
#define ICONGAP 26
#define BOTPANEL_H 36

#ifdef LANG_RUS
	#define T_APP_TITLE      "Системный монитор"
	#define T_SHOW_SYSTEM    "Системные"
	#define T_DETAILS        "Подробнее"
	#define T_PROC_KILL      "Снять задачу"
	#define T_PROC_INFO      "Инфо"
	#define T_PROC_HEADER    "Процесс        ОЗУ Кб    ЦП %"
	#define T_CPU_LOAD       "Загрузка процессора %i%%   "
	#define T_RAM_USAGE      "Память ОЗУ: %i Мб свободно из %i Мб"
	#define T_RD_USAGE       "Системный диск: %i Кб свободно из 1.4 Мб"
	#define T_TMP_USAGE      "TMP%i диск: %i Мб свободно из %i Мб"
#else
	#define T_APP_TITLE      "System Monitor"
	#define T_SHOW_SYSTEM    "System"
	#define T_DETAILS        "Details"
	#define T_PROC_KILL      "Terminate"
	#define T_PROC_INFO      "Info"
	#define T_PROC_HEADER    "Process        RAM KB   CPU %"
	#define T_CPU_LOAD       "CPU load %i%%   "
	#define T_RAM_USAGE      "RAM usage: %i MB free of %i MB"
	#define T_RD_USAGE       "System disk usage: %i MB free of 1.4 MB"
	#define T_TMP_USAGE      "TMP%i usage: %i MB free of %i MB"
#endif

enum {
	BTN_SHOW_SYSTEM_PROCESSES=200,
	BTN_PROC_KILL,
	BTN_PROC_INFO,
	BTN_MENU,
	BTN_SHOWHIDE_SENSORS
};

//===================================================//
//                                                   //
//                       VARS                        //
//                                                   //
//===================================================//

int current_process_id = 0;
int proc_list[MAX_PROCESS_COUNT];

checkbox show_system = { T_SHOW_SYSTEM, false };

sensor cpu;
sensor ram;
sensor rd;
sensor tmp;

proc_info Form;

int right_w;

bool show_sensors;

_ini ini = { "/sys/settings/app.ini", "Sysmon" };

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void main()
{
	int btn;
	load_dll(boxlib, #box_lib_init,0);
	load_dll(libini, #lib_init,1);
	ReadIni();
	@SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
	loop() switch(@WaitEventTimeout(50))
	{
		case evMouse:
			SelectList_ProcessMouse(); 
			if (mouse.up) && (mouse.pkm) || (mouse.mkm) EventShowTinfo();
			break;
		case evKey:
			GetKeys();
			switch(key_scancode) {
				case SCAN_CODE_ESC:
						EventExit();
				case SCAN_CODE_DEL:
						EventKillCurrentProcess();
						break;
				case SCAN_CODE_SPACE:
						show_sensors ^= 1;
						goto _DRAW_WINDOW;
				case SCAN_CODE_ENTER:
						EventShowTinfo();
						break;
				case SCAN_CODE_TAB:
						show_system.checked ^= 1;
						SelectList_LineChanged();
						break;
				default:
						if (select_list.ProcessKey(key_scancode)) {
							SelectList_LineChanged();
						}
			}
			break;
		case evButton:
			btn = @GetButtonID();
			if (1==btn) EventExit();

			if (show_system.click(btn)) {
				SelectList_LineChanged();
			}
			if (BTN_PROC_KILL == btn) {
				EventKillCurrentProcess();
			}
			if (BTN_PROC_INFO == btn) {
				EventShowTinfo();
			}
			if (BTN_SHOWHIDE_SENSORS == btn) {
				show_sensors ^= 1;
				GOTO _DRAW_WINDOW;
			}
			break;
		case evReDraw:
			sc.get();
			DefineAndDrawWindow(Form.left, Form.top, Form.width, Form.height, 0x33, sc.work, T_APP_TITLE,0);
			_DRAW_WINDOW:
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window&ROLLED_UP) break;
			if (Form.height < 420) { MoveSize(OLD,OLD,OLD,420); break; }
			if (show_sensors) {
				if (Form.width < RIGHT_X+370) { MoveSize(OLD,OLD,RIGHT_X+370,OLD); break; }
			} else {
				if (Form.width != RIGHT_X+5) { MoveSize(OLD,OLD,RIGHT_X+5,OLD); break; }
			}
			right_w = Form.cwidth - RIGHT_X - GAP;
			right_w &= ~1; // make sure the number is even
			WriteText(GAP+5, WIN_CONTENT_Y-20, 0x90, sc.work_text, T_PROC_HEADER);

			DefineButton(RIGHT_X-38,WIN_CONTENT_Y-25,18,18,BTN_SHOWHIDE_SENSORS,sc.button);
			DrawRectangle3D(RIGHT_X-38,WIN_CONTENT_Y-25,19,18,sc.line,sc.light);
			PutPixel(RIGHT_X-38+19,WIN_CONTENT_Y-25,sc.light);
			EDX = "<\0>";
			EDX += show_sensors * 2;
			WriteText(RIGHT_X-38+5,WIN_CONTENT_Y-25+2,0x90,sc.button_text, EDX);
			//EBX += 5 << 16;
			//$int 64

			//bool burger_active = false;
			//if (menu_id == OPEN_FILE) burger_active = true;
			//DrawTopPanelButton(BTN_MENU, Form.cwidth-GAP-3, GAP, -1, burger_active);

			SelectList_Init(GAP, WIN_CONTENT_Y, PROCESS_LIST_W, 
				Form.cheight-BOTPANEL_H-WIN_CONTENT_Y);
			SelectList_DrawBorder();

			DrawBar(select_list.x-2, select_list.y+select_list.h+2, 
				select_list.w+scroll1.size_x+4, BOTPANEL_H, sc.work);
			DrawCaptButton(PROCESS_LIST_W+GAP-110+18, select_list.y+select_list.h+5,
				110,23,BTN_PROC_KILL,0xF38181, 0xFFFfff, T_PROC_KILL);
			DrawCaptButton(PROCESS_LIST_W+GAP-165+18, select_list.y+select_list.h+5,
				46,23,BTN_PROC_INFO,sc.button, sc.button_text, T_PROC_INFO);
			show_system.draw(GAP-1, select_list.y+select_list.h+10);

			//WriteText(RIGHT_X, WIN_CONTENT_Y+25, 0x90, sc.work, "Update period: 5 seconds");
			if (show_sensors) {
				cpu.set_size(RIGHT_X, WIN_CONTENT_Y+25, right_w, 100);
				ram.set_size(RIGHT_X, WIN_CONTENT_Y+170, right_w, 23);
				rd.set_size(RIGHT_X, WIN_CONTENT_Y+240, right_w, 23);				
			}
		default:
			SelectList_LineChanged();
			if (show_sensors) {
				MonitorCpu();
				MonitorRam();
				MonitorRd();
				MonitorTmp();				
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
	int posy, j, len;
	char cpu_use[16], mem_use[16], mem_use_pretty[16];
	dword bg_color;
	proc_info Process;
	static unsigned maxcpu;
	if (!maxcpu) maxcpu = GetCpuFrequency();

	GetProcessInfo(#Process, proc_list[i+select_list.first]);
	
	posy = i *select_list.item_h + select_list.y;
	if (i % 2) bg_color = 0xFFFfff; else bg_color = 0xF0F0F0;
	if (i+select_list.first == select_list.cur_y) {
		current_process_id = Process.ID; 
		bg_color = 0x67CCEB;
	}
	DrawBar(select_list.x, posy, select_list.w, select_list.item_h, bg_color);

	WriteText(GAP+5, posy+select_list.text_y, 0x90, 0, #Process.name);

	if (Process.use_memory < 3670016000) 
	{
		sprintf(#mem_use, "%i", Process.use_memory/1024);
		len = strlen(#mem_use);
		strcpy(#mem_use_pretty, "               ");

		for (j=1; j<=len; j++) {
			EDI = sizeof(mem_use_pretty)-1-j - calc(j/4);
			mem_use_pretty[EDI] = mem_use[len-j];
		}

		WriteText(GAP+109, posy+select_list.text_y, 0x90, 0x444444, #mem_use_pretty+16-9);
	}

	sprintf(#cpu_use, "%i", Process.use_cpu*100/maxcpu);
	if (maxcpu) WriteText(GAP+203 - calc(strlen(#cpu_use)-4*8), 
		posy+select_list.text_y, 0x90, 0x444444, #cpu_use);
}

void SelectList_LineChanged() 
{
	Processes__GetProcessList();
	SelectList_Draw();
}

void MonitorRd()
{
	dword rdempty = malloc(1440*1024);
	CreateFile(0, 1440*1024, rdempty, "/rd/1/rdempty");
	free(rdempty);
	rdempty = get_file_size("/rd/1/rdempty") / 1024;
	DeleteFile("/rd/1/rdempty");

	sprintf(#param, T_RD_USAGE, rdempty);
	DrawIconWithText(RIGHT_X, rd.y - 25, 5, #param);

	rd.draw_progress(rdempty * rd.w / 1440);	
}

dword GetTmpDiskFreeSpace(int _id)
{
	DIR_SIZE dir_size;
	sprintf(#param, "/tmp%i/1", _id);
	dir_size.get(#param);
	dir_size.sizelo += dir_size.files/2 + 32 * 512; //file attr size + FAT table size
	dir_size.sizelo /= 1024*1024; //convert to MiB
	return dir_size.sizelo;	
}

void MonitorTmp()
{
	char text_status[64];
	int i, yy=WIN_CONTENT_Y+300;
	dword tmp_size[10];
	dword free_space;
	for (i=0; i<=9; i++) 
	{
		get_file_size( sprintf(#param, "/tmp%i/1", i) );
		if (EAX) {
			tmp_size[i] =  EAX / 1024 / 1024;
			free_space = tmp_size[i] - GetTmpDiskFreeSpace(i);
			sprintf(#text_status, T_TMP_USAGE, i, free_space, tmp_size[i]);
			tmp.set_size(RIGHT_X, yy, right_w, 23);
			tmp.draw_progress(free_space * right_w / tmp_size[i]);
			DrawIconWithText(RIGHT_X, tmp.y - 25, 50, #text_status);
			yy += 65;
		}
	}
}

void DrawIconWithText(dword _x, _y, _icon, _title)
{
	if (draw_icon_16w(_x, _y, _icon)) _x += ICONGAP;
	WriteTextWithBg(_x, _y + 2, 0xD0, sc.work_text, _title, sc.work);
}

dword GetCpuLoad(dword max_h)
{
	dword idle;
	dword CPU_SEC = GetCpuFrequency() >> 20 + 1;
	dword IDLE_SEC = GetCpuIdleCount() >> 20 * max_h;

	EAX = IDLE_SEC;
	EBX = CPU_SEC;
	$cdq
	$div ebx
	idle = EAX;

	return max_h - idle;
}

int pos=0;
void MonitorCpu()
{
	static dword cpu_stack[1980*3];
	int i;
	if (!cpu.w) return;

	cpu_stack[pos] = GetCpuLoad(cpu.h);
	if (cpu_stack[pos]<=2) || (cpu_stack[pos]>cpu.h) cpu_stack[pos]=2;
	
	sprintf(#param, T_CPU_LOAD, cpu_stack[pos]);
	DrawIconWithText(RIGHT_X, cpu.y - 25, 48, #param);
	
	#define LINEW 8
	for (i=0; i<right_w; i+=LINEW) {
		DrawBar(i+cpu.x, cpu.y, LINEW, cpu.h-cpu_stack[i], PROGRESS_BG);
		DrawBar(i+cpu.x, cpu.h-cpu_stack[i]+cpu.y, LINEW, cpu_stack[i], 0xDFA13B);
		//DrawBar(i+LINEW+cpu.x, cpu.y, 1, cpu.h, PROGRESS_BG);
	}

	pos++;
	if (pos>=right_w) {
		pos = right_w-1;
		for (i=0; i<pos; i++) {
			cpu_stack[i] = cpu_stack[i+1];
		}
	}
}

void MonitorRam()
{
	ram.draw_progress(GetFreeRAM()*ram.w/GetTotalRAM());
	sprintf(#param, T_RAM_USAGE, GetFreeRAM()/1024, GetTotalRAM()/1024);
	DrawIconWithText(RIGHT_X, ram.y - 25, 51, #param);
}

void EventShowTinfo()
{
	RunProgram("/sys/tinfo", itoa(GetProcessSlot(current_process_id)));
}

void ReadIni()
{
	Form.left    = ini.GetInt("x", screen.w/2 - 350); 
	Form.top     = ini.GetInt("y", 100); 
	Form.width   = ini.GetInt("w", 700); 
	Form.height  = ini.GetInt("h", 490); 
	show_sensors = ini.GetInt("show_sensors", true); 
	show_system.checked = ini.GetInt("show_system", false); 
}

void EventExit()
{
	ini.SetInt("x", Form.left);
	ini.SetInt("y", Form.top);
	ini.SetInt("w", Form.width);
	ini.SetInt("h", Form.height);
	ini.SetInt("show_sensors", show_sensors);
	ini.SetInt("show_system", show_system.checked);
	ExitProcess();
}
