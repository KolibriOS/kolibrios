/*
SOFTWARE CENTER v2.8
*/

#define MEMSIZE 0x9000
#include "..\lib\strings.h" 
#include "..\lib\mem.h" 
#include "..\lib\io.h"
#include "..\lib\gui.h"

#include "..\lib\obj\libio_lib.h"
#include "..\lib\obj\libimg_lib.h"
#include "..\lib\obj\libini.h"
#include "..\lib\font.h"
#include "..\lib\list_box.h"
#include "..\lib\collection.h"
#include "..\lib\patterns\libimg_load_skin.h"

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
	label.init(DEFAULT_FONT);
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
		// case evKey:
		// 	GetKeys();
		// 	if (list.ProcessKey(key_scancode)) DrawList();
		// 	break;

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
			DrawBar(0, row + 1 * list.item_h + list_pos, Form.cwidth, -row - 1 * list.item_h - list_pos + Form.cheight, LIST_BACKGROUND_COLOR);
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

	//do not show items located in /kolibrios/ if this directory not mounted
	if (!strncmp(key_value, "/kolibrios/", 11)) && (!kolibrios_mounted) return true;

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
	//label.write_center(col*list.item_w+7,row*list.item_h+47 + list_pos, list.item_w,0, LIST_BACKGROUND_COLOR, 0xDCDCDC, 12, key_name);
	label.write_center(col*list.item_w+6,row*list.item_h+46 + list_pos, list.item_w,0, LIST_BACKGROUND_COLOR, 0x000000, 12, key_name);
	if (list.cur_y == list.count) DrawWideRectangle(col*list.item_w+6, row*list.item_h + list_pos,list.item_w,list.item_h-5, 2, 0x0080FF);
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
	text_len = label.write(10, row * list.item_h + 10 + list_pos, LIST_BACKGROUND_COLOR, 0, 15, sec_name);
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
	label.write_center(0,5, Form.cwidth, list.y, system.color.work, system.color.work_text, 16, #window_title);
}

void EventRunApp(dword appid)
{
	if (file_exists(app_path_collection.get(appid))) {
		io.run(app_path_collection.get(appid), "");
	}
	else {
		notify("'Application not found' -E");
	}
}



stop:
