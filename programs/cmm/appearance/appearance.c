//11.03.12 - start!

#ifndef AUTOBUILD
	?include "lang.h--"
#endif

#define MEMSIZE 0xFE800
#include "..\lib\kolibri.h"
#include "..\lib\mem.h"
#include "..\lib\strings.h"
#include "..\lib\dll.h"
#include "..\lib\file_system.h"
#include "..\lib\list_box.h"
#include "..\lib\figures.h"
#include "..\lib\lib.obj\box_lib.h"

#ifdef LANG_RUS
	?define WINDOW_HEADER "Усправление темой"
	?define T_SKINS       "Тема окон"
	?define T_WALLPAPERS  "Обои рабочего стола"
#else
	?define WINDOW_HEADER "Appearance"
	?define T_SKINS       "Skins"
	?define T_WALLPAPERS  "Wallpappers"
#endif

unsigned char icons[sizeof(file "icons.raw")]= FROM "icons.raw";


#define PANEL_H 30
#define SKINS_STANDART_PATH "/sys/res/skins/"
#define WALP_STANDART_PATH "/sys/res/wallpapers/"

llist list[2];
int active;
enum { WALLPAPERS, SKINS };

char folder_path[4096];
char cur_file_path[4096];
char temp_filename[4096];
int files_mas[100];
dword buf;

int cur;

system_colors sc;
proc_info Form;

scroll_bar scroll1 = { 18,200,398, 44,18,0,115,15,0,0xeeeeee,0xD2CED0,0x555555,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};

#include "other.h"

//icons configurate, delete from list, delete from disk, make default
//remember current

void Open_Dir()
{
	int j, filesnum;

	list[active].count = 0;
	free(buf);
	if (GetDir(#buf, #filesnum, #folder_path, DIRS_ONLYREAL)!=0) return;

	for (j=0; j<filesnum; j++)
	{
		strcpy(#temp_filename, j*304 + buf+72);
		strlwr(#temp_filename);
		if (active==SKINS) if (strcmp(#temp_filename+strlen(#temp_filename)-4,".skn")!=0) continue;
		if (active==WALLPAPERS) if (strcmp(#temp_filename+strlen(#temp_filename)-4,".txt")==0) continue;
		cur = list[active].count;
		files_mas[cur]=j;
		if (!strcmp("default.skn",#temp_filename)) files_mas[0]><files_mas[list[active].count];
		list[active].count++;
	}
	Sort_by_Name(0, list[active].count-1); 
}

void Draw_List()
{
	int i;
	int yyy;
	list[SKINS].SetSizes(0, PANEL_H, Form.cwidth-scroll1.size_x-1, Form.cheight-PANEL_H, 40, 20);
	list[WALLPAPERS].SetSizes(0, PANEL_H, Form.cwidth-scroll1.size_x-1, Form.cheight-PANEL_H, 40, 20);
	
	for (i=0; i<list[active].visible; i++;)
	{
		cur = list[active].first;
		strcpy(#temp_filename, files_mas[i+cur]*304 + buf+72);
		temp_filename[strlen(#temp_filename)-4] = 0;
		yyy = i*list[active].line_h+list[active].y;
		
		if (list[active].current-list[active].first==i)
		{
			if (sc.work_button<>sc.work)
			{
				DrawBar(0, yyy, list[active].w, list[active].line_h, sc.work_button);
				WriteText(11+23,yyy+list[active].text_y,0x80,sc.work_button_text, #temp_filename);
			}
			else
			{
				DrawBar(0, yyy, list[active].w, list[active].line_h, sc.grab_button);
				WriteText(11+23,yyy+list[active].text_y,0x80,sc.grab_button_text, #temp_filename);
			}
		}
		else
		{
			DrawBar(0,yyy,list[active].w, list[active].line_h, 0xFFFfff);
			WriteText(11+23,yyy+list[active].text_y,0x80,0, #temp_filename);
		}
		_PutImage(11,yyy+2,  16,15,   list[WALLPAPERS].active*16*15*3+#icons);
	}
	DrawBar(0,list[active].visible*list[active].line_h+list[active].y, list[active].w, -list[active].visible*list[active].line_h+ list[active].h, 0xFFFfff);
	DrawScroller();
}

void GetFiles()
{
	if (list[SKINS].active)
	{
		strcpy(#folder_path, SKINS_STANDART_PATH);
		Open_Dir();
		if (!list[active].count) notify("No skins were found");
	}
	if (list[WALLPAPERS].active)
	{
		strcpy(#folder_path, WALP_STANDART_PATH);
		Open_Dir();
		if (!list[active].count) notify("No wallpapers were found");
	}
}

void Apply()
{
	if (list[SKINS].active)
	{
		strcpy(#cur_file_path, #folder_path);
		cur = list[SKINS].current;
		strcat(#cur_file_path, files_mas[cur]*304 + buf+72);
		SetSystemSkin(#cur_file_path);
		//Draw_List();
	} 
	if (list[WALLPAPERS].active)
	{
		strcpy(#cur_file_path, "\\S__");
		strcat(#cur_file_path, #folder_path);
		cur = list[WALLPAPERS].current;
		strcat(#cur_file_path, files_mas[cur]*304 + buf+72);
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
	int id, key, mouse_clicked;
	mouse mm;

	mem_Init();
	SetEventMask(0x27);
	if (load_dll2(boxlib, #box_lib_init,0)!=0) {notify("Fatal Error: library doesn't exists /rd/1/lib/box_lib.obj"); ExitProcess();}
	list[SKINS].current = list[WALLPAPERS].current = -1;
	list[SKINS].first = list[WALLPAPERS].first = 0;
	TabClick(WALLPAPERS);
	list[WALLPAPERS].SetSizes(0, 230, 350, 400-PANEL_H, 40, 18);
	list[SKINS].SetSizes(0, 230, 350, 400-PANEL_H, 40, 18);
	loop()
	{
	  switch(WaitEvent()) 
	  {
	  	case evMouse:
			if (!CheckActiveProcess(Form.ID)) break;
			scrollbar_v_mouse (#scroll1);
			if (list[active].first <> scroll1.position)
			{
				list[active].first = scroll1.position;
				Draw_List();
				break;
			}
		
	  		mm.get();

	  		if (mm.vert)
	  		{
	  			if (list[SKINS].active) && (list[SKINS].MouseScroll(mm.vert)) Draw_List();
	  			if (list[WALLPAPERS].active) && (list[WALLPAPERS].MouseScroll(mm.vert)) Draw_List();
	  		} 

	  		if (mouse_clicked)
	  		{
	  			if (!mm.lkm) && (list[SKINS].active) && (list[SKINS].ProcessMouse(mm.x, mm.y)) Apply();
	  			if (!mm.lkm) && (list[WALLPAPERS].active) && (list[WALLPAPERS].ProcessMouse(mm.x, mm.y)) Apply();
	  			mouse_clicked=0;
	  		}
	  		if (mm.lkm) && (list[SKINS].MouseOver(mm.x, mm.y)) mouse_clicked=1;
	  		break;


		case evButton:
			id=GetButtonID();
			if (id==1) ExitProcess();
			if (id==2) TabClick(WALLPAPERS);
			if (id==3) TabClick(SKINS);
			break;
	  
		case evKey:
			key = GetKey();
			if (list[SKINS].active) && (list[SKINS].ProcessKey(key)) Apply();
			if (list[WALLPAPERS].active) && (list[WALLPAPERS].ProcessKey(key)) Apply();
			IF (key==013) OpenFile();
			if (key==9) if (list[SKINS].active) TabClick(WALLPAPERS); else TabClick(SKINS);
			IF (key==182) //Del
			{
				DeleteFile(#cur_file_path);
				Open_Dir();
				Apply();
			}
			break;
		 
		 case evReDraw:
			sc.get();			
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
		col_bg=sc.work_button;
		col_text=sc.work_button_text;
	}
	else
	{
		col_bg=sc.work;
		col_text=sc.work_text;
	} 
	DrawRectangle(x,y, w,h, sc.work_graph);
	DrawCaptButton(x+1,y+1, w-2,h-1, but_id, col_bg, col_text, text);
}


void DrawTabs()
{
	DrawBar(0,0, Form.cwidth, PANEL_H-1, sc.work);

	DrawTab(10,7, 2, list[WALLPAPERS].active, T_WALLPAPERS);
	DrawTab(strlen(T_WALLPAPERS)*6+BT_PADDING+21,7, 3, list[SKINS].active, T_SKINS);

	DrawBar(0,PANEL_H-2, Form.cwidth, 1, sc.work_graph);
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
	scroll1.frnt_col = sc.work;
	scroll1.line_col = sc.work_graph;

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
