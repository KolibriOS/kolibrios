/*
SOFTWARE CENTER v2.87
*/

#define MEMSIZE 1024 * 30
#include "..\lib\strings.h" 
#include "..\lib\mem.h" 
#include "..\lib\gui.h"

#include "..\lib\obj\libini.h"
#include "..\lib\kfont.h"
#include "..\lib\list_box.h"
#include "..\lib\collection.h"

proc_info Form;
llist list;
collection app_path_collection=0;
bool kolibrios_mounted;

int window_width,
	window_height;

int list_pos, 
	row,
	col,
	default_icon;

char window_title[128],
	 settings_ini_path[256];

bool small_screen = false;

struct SW_COLORS 
 {
 	dword list_bg;
 	dword text;
 	dword graph;
 	dword dark;
 	dword light;
 } swc;

block selection[128];

void load_ini_config(dword _ini_path)
{
	_ini ini;
	ini.path = _ini_path;
	ini.section = "Config";
	ini.GetString("title", #window_title, sizeof(window_title), "Software widget");
	window_width = ini.GetInt("win_width", 690);
	list.item_w  = ini.GetInt("cell_w", 73);
	list.item_h  = ini.GetInt("cell_h", 71);
	default_icon = ini.GetInt("default_icon", 2);
}

void main()
{   
	dword id;
	kfont.init(DEFAULT_FONT);
	load_dll(libini, #lib_init,1);

	kolibrios_mounted = dir_exists("/kolibrios");

	if (param) {
		strcpy(#settings_ini_path, #param);
	} else {
		strcpy(#settings_ini_path, "/sys/settings/");
		strcat(#settings_ini_path, I_Path + strrchr(I_Path, '/'));
		strcat(#settings_ini_path, ".ini");		
	}
	
	load_ini_config(#settings_ini_path);
	list.cur_y = -1;
	list.y = 32;

	DrawList();
	window_height = row+1*list.item_h + list_pos + skin_h + 15;
	if (window_height>screen.h) {
		window_width = screen.w;
		list.item_h -= 5;
		window_height = row+1*list.item_h + list_pos + skin_h + 15;
		small_screen = true;
	}

	loop() switch(@WaitEvent())
	{
		case evKey:
			key_scancode = @GetKeyScancode();
			if (SCAN_CODE_LEFT == key_scancode) key_scancode = SCAN_CODE_UP;
			if (SCAN_CODE_RIGHT == key_scancode) key_scancode = SCAN_CODE_DOWN;
			if (list.ProcessKey(key_scancode)) DrawSelection();
			if (SCAN_CODE_ENTER == key_scancode) EventIconClick(list.cur_y);
			break;

		case evButton:
			id = @GetButtonID();               
			if (id==1) ExitProcess();
			if (id>=100) EventIconClick(id-100);
			break;

		case evReDraw:
			SetAppColors();
			DefineAndDrawWindow(screen.w-window_width/2,screen.h-window_height/2,window_width,window_height,0x74,0,"",0);
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window&ROLLED_UP) { 
				DrawTitle(#window_title);
				break;
			}
			if (small_screen) {
				DrawTitle(#window_title);
				list.y = 0;	
			} else {
				DrawTitle(NULL); 
				DrawTopBar();
			}
			DrawList();
			DrawBar(0, row +1 * list.item_h + list_pos, Form.cwidth, -row - 1 * list.item_h - list_pos + Form.cheight, swc.list_bg);
			DrawSelection();
	}
}

void SetAppColors()
{
	dword bg_col, old_list_bg_color;
	sc.get();
	old_list_bg_color = swc.list_bg;
	bg_col = sc.work;
	if (skin_is_dark()) 
	{
		//dark colors
		swc.list_bg = sc.work;
	 	swc.text = sc.work_text;
	 	swc.dark = sc.dark;
	 	swc.light = sc.light;
	} else {
		//light colors
		swc.list_bg = 0xF3F3F3;
	 	swc.text = 0x000000;
	 	swc.dark = 0xDCDCDC;
	 	swc.light = 0xFCFCFC;
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
	int icon_id = default_icon,
		icon_char_pos;
	int space_pos;

	dword icon_x, icon_y, text_x, text_y;

	//do not show items located in /kolibrios/ if this directory not mounted
	if (!strncmp(key_value, "/kolibrios/", 11)) || (!strncmp(key_value, "/k/", 3))
	|| (!strncmp(key_value, "/kg/", 4)) if (!kolibrios_mounted) return true;

	if (col==list.column_max) {
		row++;
		col=0;
	}

	if (col==0) DrawBar(0, row * list.item_h + list_pos, Form.cwidth, list.item_h, swc.list_bg);
	DefineButton(col*list.item_w+6, row*list.item_h + list_pos,list.item_w,list.item_h-3,list.count + 100 + BT_HIDE,0);

	icon_char_pos = strchr(key_value, ',');
	icon_x = col*list.item_w+calc(list.item_w/2)-10;
	icon_y = row*list.item_h+5 + list_pos;
	selection[list.count].x = icon_x-2;
	selection[list.count].y = icon_y-2;
	if (icon_char_pos) ESBYTE[icon_char_pos] = '\0'; //delete icon from string
	app_path_collection.add(key_value);

	text_x = col*list.item_w+5;
	text_y = list.item_h - 40 / 2;
	if (!strchr(key_name, ' ')) {//|| (kfont.getsize(key_name)+30<list.item_w) <== too slow
		kfont.WriteIntoWindowCenter(text_x, row*list.item_h+46 + list_pos, list.item_w,0, swc.list_bg, swc.text, 12, key_name);
	} else {
		space_pos = strrchr(key_name, ' ');
		ESBYTE[key_name+space_pos-1] = '\0';
		kfont.WriteIntoWindowCenter(text_x, row*list.item_h+46 + list_pos - 2, list.item_w,0, swc.list_bg, swc.text, 12, key_name);
		kfont.WriteIntoWindowCenter(text_x, row*list.item_h+46 + list_pos + 13, list.item_w,0, swc.list_bg, swc.text, 12, key_name+space_pos);
	}
	if (icon_char_pos) icon_id = atoi(icon_char_pos+1);
	if (Form.cwidth) draw_icon_32(icon_x, icon_y, swc.list_bg, icon_id);
	list.count++;
	col++;
	return true;
}


byte process_sections(dword sec_name, f_name)
{
	static int old_row; //to detect empty sections
	int text_len;
	if (!strcmp(sec_name, "Config")) return true;

	if ((col==0) && (row==old_row)) {
		list_pos -= 28;
	} else {
		row++;
	}
	col = 0;
	old_row = row;

	if (!small_screen) {
		DrawBar(0, row * list.item_h + list_pos, Form.cwidth , 29, swc.list_bg);
		text_len = kfont.WriteIntoWindow(10, row * list.item_h + 10 + list_pos, swc.list_bg, swc.text, 15, sec_name);
		DrawBar(text_len+20, row * list.item_h + list_pos + 20, Form.cwidth-text_len-20, 1, swc.dark);
		DrawBar(text_len+20, row * list.item_h + list_pos + 21, Form.cwidth-text_len-20, 1, swc.light);
		list_pos += 29;		
	}
	ini_enum_keys stdcall (f_name, sec_name, #draw_icons_from_section);
	return true;
}

void DrawTopBar()
{
	DrawBar(0,0,Form.cwidth, list.y-2, sc.work);
	DrawBar(0,list.y-2, Form.cwidth, 1, MixColors(sc.work, sc.line, 180));
	DrawBar(0,list.y-1, Form.cwidth, 1, sc.line);
	kfont.WriteIntoWindowCenter(0,5, Form.cwidth, list.y, sc.work, sc.work_text, 16, #window_title);
}

void EventIconClick(dword appid)
{
	char run_app_path[4096];
	dword app_path = app_path_collection.get(appid);
	dword param_pos = strchr(app_path, '|');
	if (param_pos) {
		ESBYTE[param_pos] = NULL;
		param_pos++;
	}

	// the next block is created to save some space in ramdisk{
	//
	// convert relative path to absolute      "calc"     => "/sys/calc"
	// convert short kolibrios path to full   "/k/calc"  => "/kolibrios/calc"
	// convert short kolibrios path to full   "/kg/2048" => "/kolibrios/games/2048"
	// other copy => as is
	if (ESBYTE[app_path]!='/') {
		strcpy(#run_app_path, "/sys/");
	}
	else if (!strncmp(app_path, "/k/",3)) {
		strcpy(#run_app_path, "/kolibrios/");
		app_path+=3;
	}
	else if (!strncmp(app_path, "/kg/",4)) {
		strcpy(#run_app_path, "/kolibrios/games/");
		app_path+=4;
	}
	strcat(#run_app_path, app_path);
	// }end

	if (file_exists(#run_app_path)) {
		RunProgram(#run_app_path, param_pos); //0 or offset
		if (param_pos) ESBYTE[param_pos - 1] = '|';
	} else {
		notify("'Application not found' -E");
	}
}

void DrawSelection()
{
	int i;
	dword col;
	for (i=0; i<list.count; i++) {
		if (i==list.cur_y) col=0x0080FF; else col=swc.list_bg;
		DrawWideRectangle(selection[i].x, selection[i].y, 36, 36, 2, col);
	}
}


stop:
