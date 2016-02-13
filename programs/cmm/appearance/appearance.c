//11.03.12 - start!
//ver 2.0

#ifndef AUTOBUILD
	?include "lang.h--"
#endif

#define MEMSIZE 0xFE800
#include "..\lib\mem.h"
#include "..\lib\strings.h"
#include "..\lib\io.h"
#include "..\lib\list_box.h"
#include "..\lib\menu.h"
#include "..\lib\gui.h"
#include "..\lib\obj\box_lib.h"


//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

#ifdef LANG_RUS
	?define WINDOW_HEADER "Настройки оформления"
	?define T_SKINS       "   Стиль окон"
	?define T_WALLPAPERS  "   Обои"
#else
	?define WINDOW_HEADER "Appearance"
	?define T_SKINS       "   Skins"
	?define T_WALLPAPERS  "   Wallpappers"
#endif

unsigned char icons[]= FROM "icons.raw";

#define PANEL_H 40
#define LIST_PADDING 20
#define TAB_PADDING 16
#define TAB_HEIGHT 25
#define SKINS_STANDART_PATH "/kolibrios/res/skins"							
#define WALP_STANDART_PATH "/kolibrios/res/wallpapers"

llist list;
signed int active_tab, active_skin=-1, active_wallpaper=-1;
enum { SKINS=2, WALLPAPERS };

char folder_path[4096];
char cur_file_path[4096];
char temp_filename[4096];
int files_mas[400];

int cur;

proc_info Form;

scroll_bar scroll1 = { 18,200,398, 44,18,0,115,15,0,0xeeeeee,0xD2CED0,0x555555,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};


//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void main()
{   
	int id, mouse_clicked;

	SetEventMask(0x27);
	load_dll(boxlib, #box_lib_init,0);
	EventTabClick(SKINS);
	loop() switch(WaitEvent()) 
	{
	  	case evMouse:
			if (!CheckActiveProcess(Form.ID)) break;
			mouse.get();
			scrollbar_v_mouse (#scroll1);
			if (list.first != scroll1.position)
			{
				list.first = scroll1.position;
				Draw_List();
				break;
			}

	  		if (mouse.vert) && (list.MouseScroll(mouse.vert)) Draw_List();

	  		if (mouse.up)&&(mouse_clicked)
	  		{
	  			if (mouse.lkm) && (list.ProcessMouse(mouse.x, mouse.y)) EventApply();
	  			mouse_clicked=false;
	  		}
	  		else if (mouse.down)&&(mouse.lkm) && (list.MouseOver(mouse.x, mouse.y)) mouse_clicked=true;

	  		if (mouse.down)&&(mouse.pkm) {
	  			list.ProcessMouse(mouse.x, mouse.y);
				Draw_List();
	  			menu.show(Form.left+mouse.x, Form.top+mouse.y+skin_height, 136, "Open file     Enter\nDelete          Del", 10); 
	  		}

	  		break;


		case evButton:
			id=GetButtonID();
			if (id==1) ExitProcess();
			if (id==SKINS) EventTabClick(SKINS);
			if (id==WALLPAPERS) EventTabClick(WALLPAPERS);
			break;
	  
		case evKey:
			GetKeys(); 
			if (list.ProcessKey(key_scancode)) EventApply();
			if (key_scancode==SCAN_CODE_ENTER) EventOpenFile();
			if (key_scancode==SCAN_CODE_TAB) if (active_tab==SKINS) EventTabClick(WALLPAPERS); else EventTabClick(SKINS);
			if (key_scancode==SCAN_CODE_DEL) EventDeleteFile();
			for (id=list.cur_y+1; id<list.count; id++)
			{
				strcpy(#temp_filename, io.dir.position(files_mas[id]));
				if (temp_filename[0]==key_ascii) || (temp_filename[0]==key_ascii-32)
				{
					list.cur_y = id - 1;
					list.KeyDown();
					EventApply();
					break;
				}
			}
			break;
		 
		 case evReDraw:
			system.color.get();			
			DefineAndDrawWindow(screen.width-400/2,80,400,404+skin_height,0x73,0xE4DFE1,WINDOW_HEADER,0);
			GetProcessInfo(#Form, SelfInfo);
			IF (Form.status_window>=2) break;
		 	DrawWindowContent();
		 	debugi(menu.list.cur_y);
	 		if (menu.list.cur_y) {
				if (menu.list.cur_y == 10) EventOpenFile();
				if (menu.list.cur_y == 11) EventDeleteFile();
				menu.list.cur_y = 0;
			};
   }
}

void DrawWindowContent()
{
	int id;
	list.SetFont(8, 14, 0x90);
	id = list.cur_y;
	list.SetSizes(LIST_PADDING, PANEL_H, Form.cwidth-scroll1.size_x-LIST_PADDING-LIST_PADDING, Form.cheight-PANEL_H-LIST_PADDING, 20);
	list.cur_y = id;

	DrawBar(0,0, Form.cwidth, PANEL_H-LIST_PADDING, system.color.work);
	DrawRectangle3D(list.x-2, list.y-2, list.w+3+scroll1.size_x, list.h+3, system.color.work_dark, system.color.work_light);
	DrawWideRectangle(list.x-LIST_PADDING, list.y-LIST_PADDING, LIST_PADDING*2+list.w+scroll1.size_x, LIST_PADDING*2+list.h, LIST_PADDING-2, system.color.work);
	DrawTab(list.x+10, list.y, SKINS, T_SKINS);
	if (isdir(WALP_STANDART_PATH)) DrawTab(strlen(T_SKINS)*8+TAB_PADDING+list.x+21, list.y, WALLPAPERS, T_WALLPAPERS);
	DrawRectangle(list.x-1, list.y-1, list.w+1+scroll1.size_x, list.h+1, system.color.work_graph);

	Draw_List();
}

void DrawTab(dword x,y, but_id, text)
{
	dword col_bg, col_text;
	dword w=strlen(text)*8+TAB_PADDING, h=TAB_HEIGHT;
	y -= h;

	if (but_id==active_tab)
	{
		col_bg=system.color.work_button;
		col_text=system.color.work_button_text;
	}
	else
	{
		col_bg=system.color.work;
		col_text=system.color.work_text;
	} 
	DrawCaptButton(x,y, w-1,h+1, but_id, col_bg, col_text, text);
	_PutImage(x+10,h-16/2+y+1,  16,15,   but_id-2*16*15*3+#icons);
}

void DrawScroller()
{
	scroll1.bckg_col = 0xBBBbbb;
	scroll1.frnt_col = system.color.work;
	scroll1.line_col = system.color.work_graph;

	scroll1.max_area = list.count;
	scroll1.cur_area = list.visible;
	scroll1.position = list.first;

	scroll1.all_redraw=1;
	scroll1.start_x = list.x + list.w;
	scroll1.start_y = list.y-1;
	scroll1.size_y = list.h+2;

	scrollbar_v_draw(#scroll1);
}

void Open_Dir()
{
	int j;
	list.count = 0;
	if(io.dir.buffer)free(io.dir.buffer);
	io.dir.load(#folder_path,DIR_ONLYREAL);
	for (j=0; j<io.dir.count; j++)
	{
		strcpy(#temp_filename, io.dir.position(j));
		strlwr(#temp_filename);
		if (active_tab==SKINS) if (strcmpi(#temp_filename+strlen(#temp_filename)-4,".skn")!=0) continue;
		if (active_tab==WALLPAPERS) if (strcmpi(#temp_filename+strlen(#temp_filename)-4,".txt")==0) continue;
		cur = list.count;
		files_mas[cur]=j;
		if (!strcmpi("default.skn",#temp_filename)) files_mas[0]><files_mas[list.count];
		list.count++;
	}
}

void Draw_List()
{
	int i, yyy, list_last;

	if (list.count > list.visible) list_last = list.visible; else list_last = list.count;

	for (i=0; (i<list_last); i++;)
	{
		cur = list.first + i;
		strcpy(#temp_filename, io.dir.position(files_mas[cur]));
		temp_filename[strlen(#temp_filename)-4] = 0;
		yyy = i*list.item_h+list.y;
		
		if (list.cur_y-list.first==i)
		{
			DrawBar(list.x, yyy, list.w, list.item_h, system.color.work_button);
			WriteText(list.x+12,yyy+list.text_y,list.font_type,system.color.work_button_text, #temp_filename);
		}
		else
		{
			DrawBar(list.x,yyy,list.w, list.item_h, 0xFFFfff);
			WriteText(list.x+12,yyy+list.text_y,list.font_type,0, #temp_filename);
		}
	}
	DrawBar(list.x,i*list.item_h+list.y, list.w, -i*list.item_h+ list.h, 0xFFFfff);
	DrawScroller();
}

//===================================================//
//                                                   //
//                     EVENTS                        //
//                                                   //
//===================================================//

void EventTabClick(int N)
{
	active_tab = N;
	if (active_tab == SKINS) 
	{
		active_wallpaper = list.cur_y;
		strcpy(#folder_path, SKINS_STANDART_PATH);
		list.ClearList();
		Open_Dir();
		if (!list.count) notify("'No skins were found' -E");
		list.cur_y = active_skin;
	}
	if (active_tab == WALLPAPERS) 
	{
		active_skin = list.cur_y;
		strcpy(#folder_path, WALP_STANDART_PATH);
		list.ClearList();
		Open_Dir();
		if (!list.count) notify("'No wallpapers were found' -E");
		list.cur_y = active_wallpaper;
	}
	if (list.w) DrawWindowContent();
}

void EventDeleteFile()
{
	io.del(#cur_file_path);
	Open_Dir();
	EventApply();
}

void EventApply()
{
	if (active_tab==SKINS)
	{
		cur = list.cur_y;
		sprintf(#cur_file_path,"%s/%s",#folder_path,io.dir.position(files_mas[cur]));
		SetSystemSkin(#cur_file_path);
	} 
	if (active_tab==WALLPAPERS)
	{
		cur = list.cur_y;
		sprintf(#cur_file_path,"\\S__%s/%s",#folder_path,io.dir.position(files_mas[cur]));
		RunProgram("/sys/media/kiv", #cur_file_path);
		Draw_List();
	}
}

void EventOpenFile()
{
	if (active_tab==SKINS) RunProgram("/sys/skincfg", #cur_file_path);
	if (active_tab==WALLPAPERS) RunProgram("/sys/media/kiv", #cur_file_path);
}

stop:
