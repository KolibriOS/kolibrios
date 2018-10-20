/*
SOFTWARE CENTER v2.81
*/

#define MEMSIZE 4096 * 15
#include "..\lib\strings.h" 
#include "..\lib\mem.h" 
#include "..\lib\io.h"
#include "..\lib\gui.h"

#include "..\lib\obj\libio.h"
#include "..\lib\obj\libimg.h"
#include "..\lib\obj\libini.h"
#include "..\lib\kfont.h"
#include "..\lib\list_box.h"
#include "..\lib\collection.h"

proc_info Form;
llist list;
collection app_path_collection;
byte kolibrios_mounted;

int window_width,
	window_height;

int list_pos, 
	row,
	col,
	default_icon;

char window_title[128],
	 settings_ini_path[256] = "/sys/settings/";

#define LIST_BACKGROUND_COLOR 0xF3F3F3

block ipos[128];

void load_config()
{
	ini_get_str stdcall (#settings_ini_path, "Config", "window_title", #window_title, sizeof(window_title), "Software widget");
	ini_get_int stdcall (#settings_ini_path, "Config", "window_width", 690);
	window_width = EAX;
	ini_get_int stdcall (#settings_ini_path, "Config", "cell_w", 73);
	list.item_w = EAX;
	ini_get_int stdcall (#settings_ini_path, "Config", "cell_h", 71);
	list.item_h = EAX;
	ini_get_int stdcall (#settings_ini_path, "Config", "default_icon", 0);
	default_icon = EAX;
}


void main()
{   
	dword id;
	kfont.init(DEFAULT_FONT);
	load_dll(libio,  #libio_init,1);
	load_dll(libimg, #libimg_init,1);
	load_dll(libini, #lib_init,1);

	Libimg_LoadImage(#skin, "/sys/icons32.png");
	Libimg_FillTransparent(skin.image, skin.w, skin.h, LIST_BACKGROUND_COLOR);
	kolibrios_mounted = dir_exists("/kolibrios");

	if (param)
	{
		strcpy(#settings_ini_path, #param);
	}
	else
	{
		strcat(#settings_ini_path, I_Path + strrchr(I_Path, '/'));
		strcat(#settings_ini_path, ".ini");		
	}
	
	load_config();
	list.cur_y = -1;
	list.y = 32;

	DrawList();
	window_height = row+1*list.item_h + list_pos + skin_height + 15;

	loop() switch(WaitEvent())
	{
		case evKey:
			GetKeys();
			if (SCAN_CODE_LEFT == key_scancode) key_scancode = SCAN_CODE_UP;
			if (SCAN_CODE_RIGHT == key_scancode) key_scancode = SCAN_CODE_DOWN;
			if (list.ProcessKey(key_scancode)) DrawSelection();
			if (SCAN_CODE_ENTER == key_scancode) EventRunApp(list.cur_y);
			break;

		case evButton:
			id=GetButtonID();               
			if (id==1) ExitProcess();
			if (id>=100) EventRunApp(id-100);
			break;

		case evReDraw:
			system.color.get();
			DefineAndDrawWindow(screen.width-window_width/2,screen.height-window_height/2,window_width,window_height,0x74,system.color.work,"",0);
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window>2) { DrawTitle(#window_title); break; } else DrawTitle("");
			draw_top_bar();
			DrawList();
			DrawBar(0, row +1 * list.item_h + list_pos, Form.cwidth, -row - 1 * list.item_h - list_pos + Form.cheight, LIST_BACKGROUND_COLOR);
			//if (list.cur_y == list.count) 
				DrawSelection();
			break;
	}
}

void DrawList() {
	list.count = 0;
	row = -1;
	app_path_collection.drop();
	list_pos = list.y;
	list.column_max = window_width - 10 / list.item_w;
	ini_enum_sections stdcall (#settings_ini_path, #process_sections);
	list.visible = list.count;
}

byte draw_icons_from_section(dword key_value, key_name, sec_name, f_name)
{
	int tmp,
		icon_id,
		icon_char_pos;
	int text_w;

	//do not show items located in /kolibrios/ if this directory not mounted
	if (!strncmp(key_value, "/kolibrios/", 11)) || (!strncmp(key_value, "/k/", 3))
		if (!kolibrios_mounted) return true;

	if (col==list.column_max) {
		row++;
		col=0;
	}

	if (col==0) DrawBar(0, row * list.item_h + list_pos, Form.cwidth, list.item_h, LIST_BACKGROUND_COLOR);
	DefineButton(col*list.item_w+6, row*list.item_h + list_pos,list.item_w,list.item_h-5,list.count + 100 + BT_HIDE,0);
	tmp = list.item_w/2;

	icon_char_pos = strchr(key_value, ',');
	if (icon_char_pos) icon_id = atoi(icon_char_pos+1); else icon_id = default_icon;
	img_draw stdcall(skin.image, col*list.item_w+tmp-10, row*list.item_h+5 + list_pos, 32, 32, 0, icon_id*32);
	if (icon_char_pos) ESBYTE[icon_char_pos] = '\0'; //delete icon from string
	app_path_collection.add(key_value);
	//kfont.WriteIntoWindowCenter(col*list.item_w+7,row*list.item_h+47 + list_pos, list.item_w,0, LIST_BACKGROUND_COLOR, 0xDCDCDC, 12, key_name);
	text_w = kfont.WriteIntoWindowCenter(col*list.item_w+5,row*list.item_h+46 + list_pos, list.item_w,0, LIST_BACKGROUND_COLOR, 0x000000, 12, key_name);
	ipos[list.count].x = list.item_w-text_w/2+calc(col*list.item_w)+5;
	ipos[list.count].y = row*list.item_h+46 + list_pos + 16;
	ipos[list.count].w = text_w;
	list.count++;
	col++;
	return true;
}


int old_row; //to detect empty sections
byte process_sections(dword sec_name, f_name)
{
	int text_len;
	if (!strcmp(sec_name, "Config")) return true;

	if ((col==0) && (row==old_row)) 
	{
		list_pos -= 28;
	}
	else
	{
		row++;
	}
	col = 0;
	old_row = row;
	DrawBar(0, row * list.item_h + list_pos, Form.cwidth , 29, LIST_BACKGROUND_COLOR);
	text_len = kfont.WriteIntoWindow(10, row * list.item_h + 10 + list_pos, LIST_BACKGROUND_COLOR, 0, 15, sec_name);
	DrawBar(text_len+20, row * list.item_h + list_pos + 20, Form.cwidth-text_len-20, 1, 0xDCDCDC);
	DrawBar(text_len+20, row * list.item_h + list_pos + 21, Form.cwidth-text_len-20, 1, 0xFCFCFC);
	list_pos += 29;
	ini_enum_keys stdcall (f_name, sec_name, #draw_icons_from_section);
	return true;
}

void draw_top_bar()
{
	DrawBar(0,0,Form.cwidth, list.y-2, system.color.work);
	DrawBar(0,list.y-2, Form.cwidth, 1, MixColors(system.color.work, system.color.work_graph, 180));
	DrawBar(0,list.y-1, Form.cwidth, 1, system.color.work_graph);
	kfont.WriteIntoWindowCenter(0,5, Form.cwidth, list.y, system.color.work, system.color.work_text, 16, #window_title);
}

void EventRunApp(dword appid)
{
	char run_app_path[4096]=0;
	dword app_path = app_path_collection.get(appid);
	dword param_pos = strchr(app_path, '|');
	if (param_pos) {
		ESBYTE[param_pos] = NULL;
		param_pos++;
	}

	// the next block is created to save some space in ramdisk{
	//
	// convert relative path to absolute      "calc"    => "/sys/calc"
	// convert short kolibrios path to full   "/k/calc" => "/kolibrios/calc"
	// other copy => as is
	if (ESBYTE[app_path]!='/') {
		strcpy(#run_app_path, "/sys/");
	}
	else if (!strncmp(app_path, "/k/",3)) {
		strcpy(#run_app_path, "/kolibrios/");
		app_path+=3;
	}
	strcat(#run_app_path, app_path);
	// }end

	if (file_exists(#run_app_path))
	{
		io.run(#run_app_path, param_pos); //0 or offset
		if (param_pos) ESBYTE[param_pos - 1] = '|';
	}
	else
	{
		notify("'Application not found' -E");
	}
}

void DrawSelection()
{
	int i;
	dword col;
	for (i=0; i<list.count; i++) {
		if (i==list.cur_y) col=0x0080FF; else col=LIST_BACKGROUND_COLOR;
		DrawBar(ipos[i].x, ipos[i].y, ipos[i].w+2, 3, col);
	}
}


stop:
