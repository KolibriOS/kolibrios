#define MEMSIZE 1024*80

//===================================================//
//                                                   //
//                       LIB                         //
//                                                   //
//===================================================//

#include "../lib/gui.h"
#include "../lib/list_box.h"
#include "../lib/collection.h"
#include "../lib/patterns/restart_process.h"

#include "../lib/mem.h" 

#include "../lib/obj/libini.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

int current_process_id = 0;
int proc_list[256];
collection attached=0;

llist list;

_ini ini = { "/sys/settings/appicons.ini", "icons"};

proc_info Form;
proc_info Process;

enum {
	ATTACHEMENT_BOTTOM,
	ATTACHEMENT_LEFT,
	ATTACHEMENT_TOP,
	ATTACHEMENT_RIGHT
};
int attachement = ATTACHEMENT_BOTTOM;

#define CELLW 40
#define CELLH 40

dword COLOR_BG      = 0x3B3B3B;
dword COLOR_MENU_BG = 0x323232;
dword COLOR_OPENED  = 0x999999;
dword COLOR_ACTIVE  = 0x0099FF;
dword COLOR_TEXT    = 0xFFFfff;


//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void main()
{
	int btn;
	load_dll(libini, #lib_init,1);

	ini_get_int stdcall ("/sys/appicons.ini", "taskbar", "attachement", ATTACHEMENT_BOTTOM); 
	attachement = EAX;

	SetAttachement();
	GetAttachedItems();
	
	GetProcessInfo(#Form, SelfInfo);
	SetWindowLayerBehaviour(-1, ZPOS_DESKTOP);
	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);
	loop() switch(@WaitEventTimeout(50))
	{
	   	case evMouse:
			if (!CheckActiveProcess(Form.ID)) break;
			mouse.get();
			if (mouse.down) {}
			//if (list.ProcessMouse()) DrawProcessList();
			break;
		case evKey:
			GetKeys();
			if (key_scancode == SCAN_CODE_ESC) {
				RunProgram(#program_path, NULL);
				ExitProcess();
			}
			break;
		case evButton:
			btn = @GetButtonID();
			btn -= 100;
			if (btn < attached.count) RunProgram(attached.get(btn), NULL);
			else EventSetActiveProcess(btn);
			break;
		case evReDraw:
			DefineUnDragableWindow(NULL, NULL, NULL, NULL);
			list.SetSizes(0, 0, Form.width+1, Form.height+2, CELLH);
		default:
			DrawProcessList();
	}
}


void GetProcessList()
{
	int i, j;
	list.count=0;

	for (i=0; i<attached.count; i++) 
	{
		proc_list[list.count] = 0;
		list.count++;
	}

	for (i=0; i<MAX_PROCESS_COUNT; i++)
	{
		GetProcessInfo(#Process, i);
		if (Process.name) 
		{
			for (j=0; j<11; j++) if (Process.name[j]!=' ') { 				
				//do not show system process
				if (Process.name[0]=='@') break;
				if (!strcmp(#Process.name, "IDLE")) break;
				if (!strcmp(#Process.name, "OS")) break;
				if (!Process.width) && (!Process.height) break;
				proc_list[list.count] = i;
				list.count++;
				break; 
			}
		}
	}
}

void DrawProcessList()
{
	#define ACTIVE_SIZE 3
	#define CELL_MARGIN_X 10
	#define CELL_MARGIN_Y 4
	#define CELL_PADDING 4
	int i;
	int posy=list.y;
	int posx=list.x;
	int icon_n;
	dword status_color;

	GetProcessList();

	for (i=0; i<list.count; i++)
	{
		if (proc_list[i+list.first]==0) {
			status_color = COLOR_BG;
			icon_n = ini.GetInt(attached.get(i+list.first)+strrchr(attached.get(i+list.first),'/'), 2);
		} 
		else {
			GetProcessInfo(#Process, proc_list[i+list.first]);
			strlwr(#Process.name);
			icon_n = ini.GetInt(#Process.name, 2);
			if (CheckActiveProcess(Process.ID)) && (Process.status_window!=2) {
				current_process_id = Process.ID;
				status_color = COLOR_ACTIVE;
			} 
			else {
				status_color = COLOR_OPENED;
			}			
		} 
		DrawWideRectangle(posx, posy, 40, 40, CELL_PADDING, COLOR_BG);
		DefineButton(posx, posy, CELLW-1, CELLH, 100+i+BT_HIDE+BT_NOFRAME, NULL);
		draw_icon_32(posx+CELL_PADDING, posy+CELL_PADDING, COLOR_BG, icon_n);

		if (ATTACHEMENT_BOTTOM==attachement) DrawBar(posx, posy+CELLH-ACTIVE_SIZE, CELLW, ACTIVE_SIZE, status_color);
		if (ATTACHEMENT_LEFT  ==attachement) DrawBar(posx, posy, ACTIVE_SIZE, CELLH, status_color);
		if (ATTACHEMENT_TOP   ==attachement) DrawBar(posx, posy, CELLW, ACTIVE_SIZE, status_color);
		if (ATTACHEMENT_RIGHT ==attachement) DrawBar(posx+CELLW-ACTIVE_SIZE, posy, ACTIVE_SIZE, CELLH, status_color);

		if (ATTACHEMENT_TOP==attachement) || (ATTACHEMENT_BOTTOM==attachement) {
			posx += CELLW;
			DrawBar(posx, posy, CELL_MARGIN_X, list.h, COLOR_BG);
			posx += CELL_MARGIN_X;
		}
		if (ATTACHEMENT_LEFT==attachement) || (ATTACHEMENT_RIGHT==attachement) {
			posy += CELLH;
			DrawBar(posx, posy, list.w, CELL_MARGIN_Y, COLOR_BG);
			posy += CELL_MARGIN_Y;
		}
	}
	DrawBar(posx, posy, list.w, list.h, COLOR_BG);
}

void SetAttachement()
{
	if (attachement==ATTACHEMENT_LEFT) {
		DefineUnDragableWindow(0, 0, CELLW-1, screen.h);
		SetClientScreenArea(CELLW, screen.w-CELLW, 0, screen.h);
	}
	if (attachement==ATTACHEMENT_RIGHT) {
		DefineUnDragableWindow(screen.w - CELLW, 0, CELLW, screen.h);
		SetClientScreenArea(0, screen.w-CELLW, 0, screen.h);
	}
	if (attachement==ATTACHEMENT_TOP) {
		DefineUnDragableWindow(0, 0, screen.w, CELLH-1);
		SetClientScreenArea(0, 0, CELLH, screen.h);
	}
	if (attachement==ATTACHEMENT_BOTTOM) {
		DefineUnDragableWindow(0, screen.h, screen.w, CELLH);
		SetClientScreenArea(0, 0, 0, screen.h - CELLH);
	}
}

byte draw_icons_from_section(dword key_value, key_name, sec_name, f_name)
{
	attached.add(key_name);
	return true;
}

void GetAttachedItems()
{
	attached.drop();
	ini_enum_keys stdcall ("/sys/settings/appicons.ini", "attached", #draw_icons_from_section);
}
//===================================================//
//                                                   //
//                      EVENTS                       //
//                                                   //
//===================================================//

//If we clicked on current active process then minimize it
//else set active
void EventSetActiveProcess(dword i)
{
	GetProcessInfo(#Process, proc_list[i+list.first]);
	Process.slot = GetProcessSlot(Process.ID);
	if (Process.ID) ActivateWindow(Process.slot);
	if (current_process_id == Process.ID) && (Process.status_window!=2) { 
		MinimizeWindow();
		//TODO: make another window active
	}
	DrawProcessList();
}


stop: