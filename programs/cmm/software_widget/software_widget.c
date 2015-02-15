/*
SOFTWARE CENTER v2.2
*/

#define MEMSIZE 0x3E80
#include "..\lib\kolibri.h" 
#include "..\lib\strings.h" 
#include "..\lib\mem.h" 
#include "..\lib\file_system.h"
#include "..\lib\dll.h"
#include "..\lib\figures.h"
#include "..\lib\lib.obj\libio_lib.h"
#include "..\lib\lib.obj\libimg_lib.h"
#include "..\lib\lib.obj\libini.h"


system_colors sc;
proc_info Form;
mouse m;

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
    col,
    default_icon;

char window_title[128],
     settings_ini_path[256] = "/sys/settings/";

#define LIST_BACKGROUND_COLOR 0xF3F3F3




struct struct_skin {
	dword image, w, h;
	int load();
} skin;


int struct_skin::load()
{
	int i, max_i;
	dword image_data;
	skin.image = load_image("/sys/iconstrp.png");
	if (!skin.image) notify("'iconstrp.png not found' -E");
	skin.w = DSWORD[skin.image + 4];
	skin.h = DSWORD[skin.image + 8];
	image_data = DSDWORD[skin.image + 24];
	sc.get();
	max_i = w * h * 4 + image_data;
	for (i = image_data; i < max_i; i += 4)	if (DSDWORD[i]==0) DSDWORD[i] = LIST_BACKGROUND_COLOR;
}

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
	int id, key;
	mem_Init();
	if (load_dll2(libio,  #libio_init,1)!=0) notify("Error: library doesn't exists - libio");
	if (load_dll2(libimg, #libimg_init,1)!=0) notify("Error: library doesn't exists - libimg");
	if (load_dll2(libini, #lib_init,1)!=0) notify("Error: library doesn't exists - libini");
	skin.load();

	if (#param)
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
            	ini_enum_sections stdcall (#settings_ini_path, #draw_section);
            	item_id_need_to_run = -1;
            }
			break;

         case evReDraw:
			sc.get();
			DefineAndDrawWindow(GetScreenWidth()-window_width/2,GetScreenHeight()-window_height/2,window_width,window_height,0x74,sc.work,"");
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window>2) { DrawTitle(#window_title); break; } else DrawTitle("");
			col_max = Form.cwidth - 10 / cell_w;
			current_item_id = 0;
			draw_top_bar();
			ini_enum_sections stdcall (#settings_ini_path, #draw_section);
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
		if (icon_char_pos) ESBYTE[key_value + icon_char_pos - 1] = 0; //delete icon from string
		RunProgram(key_value, "");
	}
	current_item_id++;
	return 1;
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
	if (col==0) DrawBar(0, row * cell_h + list_pos, Form.cwidth, cell_h, LIST_BACKGROUND_COLOR);
	DefineButton(col*cell_w+6,row*cell_h + list_pos,cell_w,cell_h-5,current_item_id + 100 + BT_HIDE,0);
	tmp = cell_w/2;

	icon_char_pos = strchr(key_value, ',');
	if (icon_char_pos) icon_id = atoi(key_value + icon_char_pos); else icon_id = default_icon;
	img_draw stdcall(skin.image, col*cell_w+tmp-10, row*cell_h+5 + list_pos, 32, 32, 0, icon_id*32);
	WriteTextCenter(col*cell_w+7,row*cell_h+47 + list_pos,cell_w,0xDCDCDC,key_name);
	WriteTextCenter(col*cell_w+6,row*cell_h+46 + list_pos,cell_w,0x000000,key_name);
	current_item_id++;
	col++;
	return 1;
}


byte draw_section(dword sec_name, f_name)
{
	if (strcmp(sec_name, "Config")==0) return 1;

	if (item_id_need_to_run!=-1)
	{
		ini_enum_keys stdcall (f_name, sec_name, #search_for_id_need_to_run);
	}
	else
	{
		row++;
		col = 0;
		DrawBar(0, row * cell_h + list_pos, Form.cwidth , 20, LIST_BACKGROUND_COLOR);
		WriteTextB(10, row * cell_h + 9 + list_pos, 0x90, 0x000000, sec_name);
		list_pos += 20;
		ini_enum_keys stdcall (f_name, sec_name, #draw_icons_from_section);
	}
	return 1;
}

void draw_top_bar()
{
	int top_position = 25;
	DrawBar(0,0,Form.cwidth, top_position-1, sc.work);
	DrawBar(0,top_position-1, Form.cwidth, 1, sc.work_graph);
	WriteTextB(Form.cwidth/2-70, 9, 0x90, sc.work_text, #window_title);
	list_top = top_position;
	list_pos = list_top;
	row = -1;
}



stop:
