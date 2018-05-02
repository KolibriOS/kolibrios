#define MEMSIZE 4096*20

//===================================================//
//                                                   //
//                       LIB                         //
//                                                   //
//===================================================//

#include "../lib/gui.h"
#include "../lib/list_box.h"
#include "../lib/io.h"
#include "../lib/collection.h"
#include "../lib/patterns/restart_process.h"

#include "../lib/mem.h" 

#include "../lib/obj/libio.h"
#include "../lib/obj/libimg.h"
#include "../lib/obj/libini.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

int current_process_id = 0;
int proc_list[256];
collection attached;

llist list;

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
	load_dll(libio,  #libio_init,1);
	load_dll(libimg, #libimg_init,1);
	load_dll(libini, #lib_init,1);

	Libimg_LoadImage(#skin, "/sys/icons32.png");
	Libimg_FillTransparent(skin.image, skin.w, skin.h, COLOR_BG);

	ini_get_int stdcall ("/sys/appicons.ini", "taskbar", "attachement", ATTACHEMENT_BOTTOM); 
	attachement = EAX;

	SetAttachement();
	GetAttachedItems();
	
	GetProcessInfo(#Form, SelfInfo);
	SetWindowLayerBehaviour(-1, ZPOS_DESKTOP);
	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);
	loop()
	{
	  WaitEventTimeout(50);
	  switch(EAX & 0xFF)
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
			btn = GetButtonID();
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
			ini_get_int stdcall (
				"/sys/appicons.ini", 
				"icons", 
				attached.get(i+list.first)+strrchr(attached.get(i+list.first),'/'),
				0
				); 
			icon_n = EAX;
		} 
		else {
			GetProcessInfo(#Process, proc_list[i+list.first]);
			strlwr(#Process.name);
			ini_get_int stdcall ("/sys/appicons.ini", "icons", #Process.name, 0); 
			icon_n = EAX;
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
		img_draw stdcall(skin.image, posx+CELL_PADDING, posy+CELL_PADDING, 32, 32, 0, 32*icon_n);

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
		DefineUnDragableWindow(0, 0, CELLW-1, screen.height);
		SetClientScreenArea(CELLW, screen.width-CELLW, 0, screen.height);
	}
	if (attachement==ATTACHEMENT_RIGHT) {
		DefineUnDragableWindow(screen.width - CELLW, 0, CELLW, screen.height);
		SetClientScreenArea(0, screen.width-CELLW, 0, screen.height);
	}
	if (attachement==ATTACHEMENT_TOP) {
		DefineUnDragableWindow(0, 0, screen.width, CELLH-1);
		SetClientScreenArea(0, 0, CELLH, screen.height);
	}
	if (attachement==ATTACHEMENT_BOTTOM) {
		DefineUnDragableWindow(0, screen.height, screen.width, CELLH);
		SetClientScreenArea(0, 0, 0, screen.height - CELLH);
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
	ini_enum_keys stdcall ("/sys/appicons.ini", "attached", #draw_icons_from_section);
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