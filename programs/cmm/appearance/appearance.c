//11.03.12 - start!

#ifndef AUTOBUILD
	?include "lang.h--"
#endif

#define MEMSIZE 0xFE800
#include "..\lib\mem.h"
#include "..\lib\strings.h"
#include "..\lib\io.h"
#include "..\lib\list_box.h"
#include "..\lib\gui.h"
#include "..\lib\obj\box_lib.h"

#ifdef LANG_RUS
	?define WINDOW_HEADER "Усправление темой"
	?define T_SKINS       "   Тема окон"
	?define T_WALLPAPERS  "   Обои рабочего стола"
#else
	?define WINDOW_HEADER "Appearance"
	?define T_SKINS       "   Skins"
	?define T_WALLPAPERS  "   Wallpappers"
#endif

unsigned char icons[]= FROM "icons.raw";

#define PANEL_H 30
#define SKINS_STANDART_PATH "/kolibrios/res/skins"							
#define WALP_STANDART_PATH "/kolibrios/res/wallpapers"

llist list[2];
int active;
enum { WALLPAPERS, SKINS };

char folder_path[4096];
char cur_file_path[4096];
char temp_filename[4096];
int files_mas[100];

int cur;

proc_info Form;

scroll_bar scroll1 = { 18,200,398, 44,18,0,115,15,0,0xeeeeee,0xD2CED0,0x555555,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};

void Open_Dir()
{
	int j;
	list[active].count = 0;
	if(io.dir.buffer)free(io.dir.buffer);
	io.dir.load(#folder_path,DIR_ONLYREAL);
	for (j=0; j<io.dir.count; j++)
	{
		strcpy(#temp_filename, io.dir.position(j));
		strlwr(#temp_filename);
		if (active==SKINS) if (strcmpi(#temp_filename+strlen(#temp_filename)-4,".skn")!=0) continue;
		if (active==WALLPAPERS) if (strcmpi(#temp_filename+strlen(#temp_filename)-4,".txt")==0) continue;
		cur = list[active].count;
		files_mas[cur]=j;
		if (!strcmpi("default.skn",#temp_filename)) files_mas[0]><files_mas[list[active].count];
		list[active].count++;
	}
}

void Draw_List()
{
	int i;
	int yyy;
	int list_last;
	list[SKINS].SetFont(6, 6, 10000000b);
	list[WALLPAPERS].SetFont(6, 6, 10000000b);
	list[SKINS].SetSizes(0, PANEL_H, Form.cwidth-scroll1.size_x-1, Form.cheight-PANEL_H, 20);
	list[WALLPAPERS].SetSizes(0, PANEL_H, Form.cwidth-scroll1.size_x-1, Form.cheight-PANEL_H, 20);
	if (list[active].count > list[active].visible) list_last = list[active].visible; else list_last = list[active].count;

	for (i=0; i<list_last; i++;)
	{
		cur = list[active].first;
		strcpy(#temp_filename, io.dir.position(files_mas[i+cur]));
		temp_filename[strlen(#temp_filename)-4] = 0;
		yyy = i*list[active].item_h+list[active].y;
		
		if (list[active].cur_y-list[active].first==i)
		{
			if (system.color.work_button!=system.color.work)
			{
				DrawBar(0, yyy, list[active].w, list[active].item_h, system.color.work_button);
				if (i<list[active].count) WriteText(12,yyy+list[active].text_y,0x80,system.color.work_button_text, #temp_filename);
			}
			else
			{
				DrawBar(0, yyy, list[active].w, list[active].item_h, system.color.grab_button);
				if (i<list[active].count) WriteText(12,yyy+list[active].text_y,0x80,system.color.grab_button_text, #temp_filename);
			}
		}
		else
		{
			DrawBar(0,yyy,list[active].w, list[active].item_h, 0xFFFfff);
			WriteText(12,yyy+list[active].text_y,0x80,0, #temp_filename);
		}
	}
	DrawBar(0,list_last*list[active].item_h+list[active].y, list[active].w, -list_last*list[active].item_h+ list[active].h, 0xFFFfff);
	DrawScroller();
}

void GetFiles()
{
	if (list[SKINS].active)
	{
		strcpy(#folder_path, SKINS_STANDART_PATH);
		Open_Dir();
		if (!list[active].count) notify("'No skins were found' -E");
	}
	if (list[WALLPAPERS].active)
	{
		strcpy(#folder_path, WALP_STANDART_PATH);
		Open_Dir();
		if (!list[active].count) notify("'No wallpapers were found' -E");
	}
}

void Apply()
{
	if (list[SKINS].active)
	{
		cur = list[SKINS].cur_y;
		sprintf(#cur_file_path,"%s/%s",#folder_path,io.dir.position(files_mas[cur]));
		SetSystemSkin(#cur_file_path);
	} 
	if (list[WALLPAPERS].active)
	{
		cur = list[WALLPAPERS].cur_y;
		sprintf(#cur_file_path,"\\S__%s/%s",#folder_path,io.dir.position(files_mas[cur]));
		RunProgram("/sys/media/kiv", #cur_file_path);
		Draw_List();
	}
}

OpenFile()
{
	if (list[SKINS].active) RunProgram("/sys/desktop", #cur_file_path);
	if (list[WALLPAPERS].active) RunProgram("/sys/media/kiv", #cur_file_path);
}


void main()
{   
	int id, mouse_clicked;

	SetEventMask(0x27);
	load_dll(boxlib, #box_lib_init,0);
	list[SKINS].cur_y = list[WALLPAPERS].cur_y = -1;
	list[SKINS].first = list[WALLPAPERS].first = 0;
	TabClick(WALLPAPERS);
	list[WALLPAPERS].SetSizes(0, 230, 350, 400-PANEL_H, 18);
	list[SKINS].SetSizes(0, 230, 350, 400-PANEL_H, 18);
	loop()
	{
	  switch(WaitEvent()) 
	  {
	  	case evMouse:
			if (!CheckActiveProcess(Form.ID)) break;
			mouse.get();
			scrollbar_v_mouse (#scroll1);
			if (list[active].first != scroll1.position)
			{
				list[active].first = scroll1.position;
				Draw_List();
				break;
			}

	  		if (mouse.vert)
	  		{
	  			if (list[SKINS].active) && (list[SKINS].MouseScroll(mouse.vert)) Draw_List();
	  			if (list[WALLPAPERS].active) && (list[WALLPAPERS].MouseScroll(mouse.vert)) Draw_List();
	  		} 

	  		if (mouse.up)&&(mouse_clicked)
	  		{
	  			if (mouse.lkm) &&(list[SKINS].active) && (list[SKINS].ProcessMouse(mouse.x, mouse.y)) Apply();
	  			if (mouse.lkm) &&(list[WALLPAPERS].active) && (list[WALLPAPERS].ProcessMouse(mouse.x, mouse.y)) Apply();
	  			mouse_clicked=false;
	  		}
	  		else if (mouse.down)&&(mouse.lkm) && (list[SKINS].MouseOver(mouse.x, mouse.y)) mouse_clicked=true;
	  		break;


		case evButton:
			id=GetButtonID();
			if (id==1) ExitProcess();
			if (id==2) TabClick(WALLPAPERS);
			if (id==3) TabClick(SKINS);
			break;
	  
		case evKey:
			GetKeys(); 
			if (list[SKINS].active) && (list[SKINS].ProcessKey(key_scancode)) Apply();
			if (list[WALLPAPERS].active) && (list[WALLPAPERS].ProcessKey(key_scancode)) Apply();
			IF (key_scancode==SCAN_CODE_ENTER) OpenFile();
			if (key_scancode==SCAN_CODE_TAB) if (list[SKINS].active) TabClick(WALLPAPERS); else TabClick(SKINS);
			IF (key_scancode==SCAN_CODE_TAB) //Del
			{
				DeleteFile(#cur_file_path);
				Open_Dir();
				Apply();
			}
			break;
		 
		 case evReDraw:
			system.color.get();			
			DefineAndDrawWindow(30,80,list[active].w+9,list[active].h+4+GetSkinHeight(),0x73,0xE4DFE1,WINDOW_HEADER,0);
			GetProcessInfo(#Form, SelfInfo);
			IF (Form.status_window>=2) break;
			DrawTabs();
			Draw_List();
	  }
   }
}

#define BT_PADDING 16

void DrawTab(dword x,y, but_id, is_active, text)
{
	dword col_bg, col_text;
	dword w=strlen(text)*6+BT_PADDING, h=21;

	if (is_active)
	{
		col_bg=system.color.work_button;
		col_text=system.color.work_button_text;
	}
	else
	{
		col_bg=system.color.work;
		col_text=system.color.work_text;
	} 
	DrawRectangle(x,y, w,h, system.color.work_graph);
	DrawCaptButton(x+1,y+1, w-2,h-1, but_id, col_bg, col_text, text);
	_PutImage(x+6,y+4,  16,15,   but_id-2*16*15*3+#icons);
}


void DrawTabs()
{
	DrawBar(0,0, Form.cwidth, PANEL_H-1, system.color.work);
	DrawTab(10,7, 2, list[WALLPAPERS].active, T_WALLPAPERS);
	DrawTab(strlen(T_WALLPAPERS)*6+BT_PADDING+21,7, 3, list[SKINS].active, T_SKINS);
	DrawBar(0,PANEL_H-2, Form.cwidth, 1, system.color.work_graph);
	DrawBar(0,PANEL_H-1, Form.cwidth, 1, 0xEEEeee);
}

void TabClick(int N)
{
	if (N==SKINS) 
	{
		list[SKINS].active = 1;	
		list[WALLPAPERS].active = 0;	
	}
	if (N==WALLPAPERS) 
	{
		list[SKINS].active = 0;	
		list[WALLPAPERS].active = 1;	
	}
	active = N;
	GetFiles();
	DrawTabs();
	Draw_List();
}


void DrawScroller()
{
	scroll1.bckg_col = 0xBBBbbb;
	scroll1.frnt_col = system.color.work;
	scroll1.line_col = system.color.work_graph;

	scroll1.max_area = list[active].count;
	scroll1.cur_area = list[active].visible;
	scroll1.position = list[active].first;

	scroll1.all_redraw=1;
	scroll1.start_x = list[active].x + list[active].w;
	scroll1.start_y = list[active].y-2;
	scroll1.size_y = list[active].h+2;

	scrollbar_v_draw(#scroll1);
}


stop:
