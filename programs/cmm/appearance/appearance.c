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
#include "..\lib\patterns\select_list.h"


//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

#ifdef LANG_RUS
	?define WINDOW_HEADER "Настройки оформления"
	?define T_SKINS       "Стиль окон"
	?define T_WALLPAPERS  "Обои"
#else
	?define WINDOW_HEADER "Appearance"
	?define T_SKINS       "Skins"
	?define T_WALLPAPERS  "Wallpappers"
#endif

#define PANEL_H 40
#define LP 10 //LIST_PADDING
#define SKINS_STANDART_PATH "/kolibrios/res/skins"							
#define WALP_STANDART_PATH "/kolibrios/res/wallpapers"

signed int active_skin=-1, active_wallpaper=-1;
enum { SKINS=2, WALLPAPERS };

char folder_path[4096];
char cur_file_path[4096];
char temp_filename[4096];
int files_mas[400];

int cur;

proc_info Form;
block skp;

_tabs tabs = { LP, LP, NULL, NULL, SKINS };

checkbox checkbox1 = { "Checkbox", true };

#define MENU_LIST "Open file     Enter\nDelete          Del"

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
			SelectList_ProcessMouse();

	  		if (mouse.pkm)&&(select_list.MouseOver(mouse.x, mouse.y)) {
	  			select_list.ProcessMouse(mouse.x, mouse.y);
				SelectList_Draw();
	  			menu.show(Form.left+mouse.x, Form.top+mouse.y+skin_height, 185, MENU_LIST, 10); 
	  		}
	  		break;

		case evButton:
			id=GetButtonID();
			if (id==1) ExitProcess();
			if (id==SKINS) EventTabClick(SKINS);
			if (id==WALLPAPERS) EventTabClick(WALLPAPERS);
			checkbox1.click(id);
			break;
	  
		case evKey:
			GetKeys(); 
			if (select_list.ProcessKey(key_scancode)) EventApply();
			if (key_scancode==SCAN_CODE_ENTER) EventOpenFile();
			if (key_scancode==SCAN_CODE_TAB) 
				if (tabs.active_tab==SKINS) EventTabClick(WALLPAPERS); 
				else EventTabClick(SKINS);
			if (key_scancode==SCAN_CODE_DEL) EventDeleteFile();
			for (id=select_list.cur_y+1; id<select_list.count; id++)
			{
				strcpy(#temp_filename, io.dir.position(files_mas[id]));
				if (temp_filename[0]==key_ascii) || (temp_filename[0]==key_ascii-32)
				{
					select_list.cur_y = id - 1;
					select_list.KeyDown();
					EventApply();
					break;
				}
			}
			break;
		 
		 case evReDraw:
			system.color.get();			
			DefineAndDrawWindow(screen.width-600/2,80,600,404+skin_height,0x73,0xE4DFE1,WINDOW_HEADER,0);
			GetProcessInfo(#Form, SelfInfo);
			IF (Form.status_window>=2) break;
		 	DrawWindowContent();
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

	DrawWideRectangle(0, 0, Form.cwidth, Form.cheight, LP, system.color.work);

	tabs.w = Form.cwidth-LP-LP;
	tabs.h = Form.cheight-LP-LP;
	tabs.draw_wrapper();
	
	tabs.draw_button(tabs.x+TAB_PADDING, SKINS, T_SKINS);	
	tabs.draw_button(strlen(T_SKINS)*8+tabs.x+TAB_PADDING+TAB_PADDING, WALLPAPERS, T_WALLPAPERS);

	id = select_list.cur_y;
	SelectList_Init(
		tabs.x+TAB_PADDING,
		tabs.y+TAB_HEIGHT+TAB_PADDING, 
		250, 
		tabs.h - TAB_PADDING - TAB_PADDING - TAB_HEIGHT, 
		false
		);
	select_list.cur_y = id;

	skp.set_size(
		select_list.x + select_list.w + TAB_PADDING + scroll1.size_x,
		select_list.y,
		250,
		250
	);

	SelectList_Draw();
	SelectList_DrawBorder();

	if (tabs.active_tab == SKINS)
	{
		DrawBar(skp.x, skp.y, skp.w, skp.h, system.color.work);
		DrawFrame(skp.x, skp.y, skp.w, skp.h, " Components Preview ");
		WriteText(skp.x+20, skp.y+30, 0x90, system.color.work_text, "Lorem ipsum");
		checkbox1.draw(skp.x+20, skp.y+70);
		DrawStandartCaptButton(skp.x+20, skp.y+skp.h-40, GetFreeButtonId(), "Apply");
		DrawStandartCaptButton(skp.x+120, skp.y+skp.h-40, GetFreeButtonId(), "Close");
	}
}



void Open_Dir()
{
	int j;
	select_list.count = 0;
	if(io.dir.buffer)free(io.dir.buffer);
	io.dir.load(#folder_path,DIR_ONLYREAL);
	for (j=0; j<io.dir.count; j++)
	{
		strcpy(#temp_filename, io.dir.position(j));
		strlwr(#temp_filename);
		if (tabs.active_tab==SKINS) if (strcmpi(#temp_filename+strlen(#temp_filename)-4,".skn")!=0) continue;
		if (tabs.active_tab==WALLPAPERS) if (strcmpi(#temp_filename+strlen(#temp_filename)-4,".txt")==0) continue;
		cur = select_list.count;
		files_mas[cur]=j;
		if (!strcmpi("default.skn",#temp_filename)) files_mas[0]><files_mas[select_list.count];
		select_list.count++;
	}
}

void SelectList_DrawLine(dword i)
{
	int yyy, list_last;

	cur = select_list.first + i;
	strcpy(#temp_filename, io.dir.position(files_mas[cur]));
	temp_filename[strlen(#temp_filename)-4] = 0;
	yyy = i*select_list.item_h+select_list.y;
	
	if (select_list.cur_y-select_list.first==i)
	{
		DrawBar(select_list.x, yyy, select_list.w, select_list.item_h, system.color.work_button);
		WriteText(select_list.x+12,yyy+select_list.text_y,select_list.font_type,system.color.work_button_text, #temp_filename);
	}
	else
	{
		DrawBar(select_list.x,yyy,select_list.w, select_list.item_h, 0xFFFfff);
		WriteText(select_list.x+12,yyy+select_list.text_y,select_list.font_type,0, #temp_filename);
	}
}

void SelectList_LineChanged() 
{
	EventApply();
}

//===================================================//
//                                                   //
//                     EVENTS                        //
//                                                   //
//===================================================//

void EventTabClick(int N)
{
	tabs.click(N);
	if (tabs.active_tab == SKINS) 
	{
		active_wallpaper = select_list.cur_y;
		strcpy(#folder_path, SKINS_STANDART_PATH);
		select_list.ClearList();
		Open_Dir();
		if (!select_list.count) notify("'No skins were found' -E");
		select_list.cur_y = active_skin;
	}
	if (tabs.active_tab == WALLPAPERS)
	{
		active_skin = select_list.cur_y;
		strcpy(#folder_path, WALP_STANDART_PATH);
		select_list.ClearList();
		Open_Dir();
		if (!select_list.count) notify("'No wallpapers were found' -E");
		select_list.cur_y = active_wallpaper;
	}
	if (select_list.cur_y>select_list.visible) select_list.first=select_list.cur_y; select_list.CheckDoesValuesOkey();
	if (select_list.w) DrawWindowContent();
}

void EventDeleteFile()
{
	io.del(#cur_file_path);
	Open_Dir();
	EventApply();
}

void EventApply()
{
	if (tabs.active_tab==SKINS)
	{
		cur = select_list.cur_y;
		sprintf(#cur_file_path,"%s/%s",#folder_path,io.dir.position(files_mas[cur]));
		SetSystemSkin(#cur_file_path);
	} 
	if (tabs.active_tab==WALLPAPERS)
	{
		cur = select_list.cur_y;
		sprintf(#cur_file_path,"\\S__%s/%s",#folder_path,io.dir.position(files_mas[cur]));
		RunProgram("/sys/media/kiv", #cur_file_path);
		SelectList_Draw();
	}
}

void EventOpenFile()
{
	if (tabs.active_tab==SKINS) RunProgram("/sys/skincfg", #cur_file_path);
	if (tabs.active_tab==WALLPAPERS) RunProgram("/sys/media/kiv", #cur_file_path);
}

stop:
