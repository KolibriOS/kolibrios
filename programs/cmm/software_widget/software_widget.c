/*
SOFTWARE CENTER v2.4
*/

#define MEMSIZE 0x9000
#include "..\lib\strings.h" 
#include "..\lib\mem.h" 
#include "..\lib\file_system.h"
#include "..\lib\gui.h"

#include "..\lib\obj\libio_lib.h"
#include "..\lib\obj\libimg_lib.h"
#include "..\lib\obj\libini.h"
#include "..\lib\font.h"
#include "..\lib\patterns\libimg_load_skin.h"

proc_info Form;

byte kolibrios_mounted;

int item_id_need_to_run=-1,
    current_item_id;

int window_width,
    window_height;

int col_max,
    cell_w,
    cell_h, 
    list_pos, 
    list_top,
    row,
    old_row, //to detect empty sections
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
	ini_get_int stdcall (#settings_ini_path, "Config", "window_height", 540);
	window_height = EAX;
	ini_get_int stdcall (#settings_ini_path, "Config", "cell_w", 66);
	cell_w = EAX;
	ini_get_int stdcall (#settings_ini_path, "Config", "cell_h", 64);
	cell_h = EAX;
	ini_get_int stdcall (#settings_ini_path, "Config", "default_icon", 0);
	default_icon = EAX;
}


void main()
{   
	dword id, key;
	label.init("/sys/fonts/Tahoma.kf");
	load_dll(libio,  #libio_init,1);
	load_dll(libimg, #libimg_init,1);
	load_dll(libini, #lib_init,1);

	Libimg_LoadImage(#skin, "/sys/icons32.png");
	Libimg_FillTransparent(skin.image, skin.w, skin.h, LIST_BACKGROUND_COLOR);

	if (param)
	{
		strcpy(#settings_ini_path, #param);
	}
	else
	{
		strcat(#settings_ini_path, #program_path + strrchr(#program_path, '/'));
		strcat(#settings_ini_path, ".ini");		
	}
	load_config();

	loop()
	{
      switch(WaitEvent())
      {
         case evButton:
            id=GetButtonID();               
            if (id==1) ExitProcess();
            if (id>=100)
            {
            	item_id_need_to_run = id - 100;
            	current_item_id = 0;
            	ini_enum_sections stdcall (#settings_ini_path, #process_sections);
            	item_id_need_to_run = -1;
            }
			break;

         case evReDraw:
			system.color.get();
			DefineAndDrawWindow(GetScreenWidth()-window_width/2,GetScreenHeight()-window_height/2,window_width,window_height,0x74,system.color.work,"");
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window>2) { DrawTitle(#window_title); break; } else DrawTitle("");
			kolibrios_mounted = isdir("/kolibrios");
			col_max = Form.cwidth - 10 / cell_w;
			current_item_id = 0;
			draw_top_bar();
			ini_enum_sections stdcall (#settings_ini_path, #process_sections);
			DrawBar(0, row + 1 * cell_h + list_pos, Form.cwidth, -row - 1 * cell_h - list_pos + Form.cheight, LIST_BACKGROUND_COLOR);
			break;
      }
	}
}

byte search_for_id_need_to_run(dword key_value, key_name, sec_name, f_name)
{
	int icon_char_pos;
	if (item_id_need_to_run == current_item_id)
	{
		icon_char_pos = strchr(key_value, ',');
		if (icon_char_pos) ESBYTE[icon_char_pos] = 0; //delete icon from string
		RunProgram(key_value, "");
	}
	current_item_id++;
	if (!strncmp(key_value, "/kolibrios/", 11)) && (!kolibrios_mounted) current_item_id--;
	return true;
}


byte draw_icons_from_section(dword key_value, key_name, sec_name, f_name)
{
	int tmp,
	    icon_id,
	    icon_char_pos;

	if (col==col_max) {
		row++;
		col=0;
	}

	//do not show items located in /kolibrios/ if this directory not mounted
	if (!strncmp(key_value, "/kolibrios/", 11)) && (!kolibrios_mounted) return true;

	if (col==0) DrawBar(0, row * cell_h + list_pos, Form.cwidth, cell_h, LIST_BACKGROUND_COLOR);
	DefineButton(col*cell_w+6,row*cell_h + list_pos,cell_w,cell_h-5,current_item_id + 100 + BT_HIDE,0);
	tmp = cell_w/2;

	icon_char_pos = strchr(key_value, ',');
	if (icon_char_pos) icon_id = atoi(icon_char_pos+1); else icon_id = default_icon;
	img_draw stdcall(skin.image, col*cell_w+tmp-10, row*cell_h+5 + list_pos, 32, 32, 0, icon_id*32);
	label.bold = false;
	label.write_center(col*cell_w+7,row*cell_h+47 + list_pos, cell_w,0, LIST_BACKGROUND_COLOR, 0xDCDCDC, 12, key_name);
	label.write_center(col*cell_w+6,row*cell_h+46 + list_pos, cell_w,0, LIST_BACKGROUND_COLOR, 0x000000, 12, key_name);
	current_item_id++;
	col++;
	return true;
}


byte process_sections(dword sec_name, f_name)
{
	int text_len;
	if (!strcmp(sec_name, "Config")) return true;

	if (item_id_need_to_run!=-1)
	{
		ini_enum_keys stdcall (f_name, sec_name, #search_for_id_need_to_run);
	}
	else
	{
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
		DrawBar(0, row * cell_h + list_pos, Form.cwidth , 29, LIST_BACKGROUND_COLOR);
		//WriteTextB(10, row * cell_h + 9 + list_pos, 0x90, 0x000000, sec_name);
		label.bold=false;
		text_len = label.write(10, row * cell_h + 10 + list_pos, LIST_BACKGROUND_COLOR, 0, 15, sec_name);
		DrawBar(text_len+20, row * cell_h + list_pos + 20, Form.cwidth-text_len-20, 1, 0xDCDCDC);
		DrawBar(text_len+20, row * cell_h + list_pos + 21, Form.cwidth-text_len-20, 1, 0xFCFCFC);
		list_pos += 29;
		ini_enum_keys stdcall (f_name, sec_name, #draw_icons_from_section);
	}
	return true;
}

void draw_top_bar()
{
	int top_position = 26;
	DrawBar(0,0,Form.cwidth, top_position-1, system.color.work);
	DrawBar(0,top_position-1, Form.cwidth, 1, system.color.work_graph);
	label.bold = false;
	label.write_center(0,0, Form.cwidth, top_position, system.color.work, system.color.work_text, 17, #window_title);
	list_top = top_position;
	list_pos = list_top;
	row = -1;
}



stop:
