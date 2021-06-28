//11.03.12 - start!
//ver 2.31

#define MEMSIZE 200*1024
#include "../lib/mem.h"
#include "../lib/strings.h"
#include "../lib/io.h"
#include "../lib/list_box.h"
#include "../lib/obj/libimg.h"
#include "../lib/gui.h"

#include "../lib/obj/box_lib.h"
#include "../lib/obj/proc_lib.h"
#include "../lib/obj/libini.h"

#include "../lib/patterns/select_list.h"
#include "../lib/patterns/simple_open_dialog.h"

#include "ui_elements_preview.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

#ifdef LANG_RUS
	?define WINDOW_HEADER "Настройки оформления"
	?define T_SKINS       "   Стиль окон"
	?define T_WALLPAPERS  "   Обои"
	?define T_SELECT_FOLDER "Выбрать папку"
	?define MENU_LIST "Открыть файл   |Enter\nУдалить файл     |Del"
	?define T_PICTURE_MODE " Положение картинки "
	?define T_CHECKBOX_STRETCH "Растянуть"
	?define T_CHECKBOX_TILED "Замостить"
	?define T_UPDATE_DOCK "Обновлять Dock-панель"
#else
	?define WINDOW_HEADER "Appearance"
	?define T_SKINS       "   Skins"
	?define T_WALLPAPERS  "   Wallpapers"
	?define T_SELECT_FOLDER "Select folder"
	?define MENU_LIST "Open file      |Enter\nDelete file      |Del"
	?define T_PICTURE_MODE " Picture Mode "
	?define T_CHECKBOX_STRETCH "Stretch"
	?define T_CHECKBOX_TILED "Tiled"
	?define T_UPDATE_DOCK "Update Dock"
#endif

#define PANEL_H 40
#define LP 10 //LIST_PADDING
char skins_folder_path[4096];
char wallp_folder_path[4096];

signed int active_skin=-1, active_wallpaper=-1;
enum { 
	BASE_TAB_BUTTON_ID=2, 
	BTN_SELECT_WALLP_FOLDER=10 };

char folder_path[4096];
char cur_file_path[4096];
char cur_skin_path[4096];
char temp_filename[4096];
int files_mas[400];

int cur;

proc_info Form;
block skp;

enum {SKINS, WALLPAPERS};

_tabs tabs = { LP, LP, NULL, BASE_TAB_BUTTON_ID };

checkbox update_docky = { T_UPDATE_DOCK, false };

char default_dir[] = "/rd/1";
od_filter filter2 = { 8, "TXT\0\0" };

checkbox optionbox_stretch = { T_CHECKBOX_STRETCH, true };
checkbox optionbox_tiled = { T_CHECKBOX_TILED, false };

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void GetRealFolderPathes()
{
	char real_skin_path[4096];
	SetCurDir("/kolibrios");
	GetCurDir(#real_skin_path, sizeof(real_skin_path));
	sprintf(#skins_folder_path, "%s/res/skins", #real_skin_path);
	sprintf(#wallp_folder_path, "%s/res/wallpapers", #real_skin_path);
}

void main()
{   
	int id;

	GetRealFolderPathes();

	load_dll(boxlib, #box_lib_init,0);
	load_dll(libini, #lib_init,1);
	load_dll(libimg, #libimg_init,1);
	load_dll(Proc_lib, #OpenDialog_init,0);
	o_dialog.type = 2; //select folder
	OpenDialog_init stdcall (#o_dialog);

	tabs.add(T_SKINS, #EventTabSkinsClick);	
	tabs.add(T_WALLPAPERS, #EventTabWallpappersClick);
	tabs.draw_active_tab();

	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
	loop() switch(WaitEvent()) 
	{
	  	case evMouse:
			SelectList_ProcessMouse();

			if (tabs.active_tab == SKINS) {
				edit_box_mouse stdcall (#edit_cmm);
				edit_box_mouse stdcall (#edit_st);
			}

	  		if (mouse.key&MOUSE_RIGHT) && (mouse.up) 
	  		&&(select_list.MouseOver(mouse.x, mouse.y)) {
	  			select_list.ProcessMouse(mouse.x, mouse.y);
				SelectList_Draw();
				EventSetNewCurrent();
	  			open_lmenu(mouse.x, mouse.y, MENU_TOP_RIGHT, NULL, MENU_LIST);
	  		}
	  		break;

		case evButton:
			id=GetButtonID();
			if (id==1) EventExit();
			if (id==BTN_SELECT_WALLP_FOLDER) EventSelectWallpFolder();
			tabs.click(id);
			checkbox1.click(id);
			spinbox1.click(id);
			if (update_docky.click(id)) EventUpdateDocky();
			if (!optionbox_stretch.checked) && (optionbox_stretch.click(id)) EventSetWallpMode_Stretch();
			if (!optionbox_tiled.checked) && (optionbox_tiled.click(id)) EventSetWallpMode_Tiled();
			break;
	  
		case evKey:
			GetKeys(); 
			if (select_list.ProcessKey(key_scancode)) EventApply();
			if (key_scancode==SCAN_CODE_ENTER) EventOpenFile();
			if (key_scancode==SCAN_CODE_TAB) tabs.click(tabs.active_tab ^ 1);
			if (key_scancode==SCAN_CODE_DEL) EventDeleteFile();

			if (! edit_cmm.flags & ed_focus) && (! edit_st.flags & ed_focus)
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

			if (tabs.active_tab == SKINS) {
				EAX = key_ascii << 8;
				edit_box_key stdcall (#edit_cmm);
				edit_box_key stdcall (#edit_st);				
			}
			break;
		 
		 case evReDraw:		
			draw_window();
	 		EventHandleMenuClick();
   }
}

void draw_window()
{
	sc.get();
	DefineAndDrawWindow(screen.width-600/2,80,630,504+skin_height,0x34,sc.work,WINDOW_HEADER,0);
	GetProcessInfo(#Form, SelfInfo);
	IF (Form.status_window&ROLLED_UP) return;
	DrawWindowContent();
}

void DrawWindowContent()
{
	int id;
	int list_w;

	sc.get();	

	if (tabs.active_tab == SKINS) list_w=250; else list_w=350;

	tabs.w = Form.cwidth-LP-LP;
	tabs.draw();
	DrawIcon16(tabs.x + TAB_PADDING, 15, sc.work, 17);
	DrawIcon16(strlen(T_SKINS)*8 + tabs.x + TAB_PADDING + TAB_PADDING, 15, sc.work, 6);

	id = select_list.cur_y;
	SelectList_Init(
		tabs.x+TAB_PADDING,
		tabs.y+TAB_HEIGHT+TAB_PADDING, 
		list_w, 
		Form.cheight-LP-LP - TAB_PADDING - TAB_PADDING - TAB_HEIGHT
		);
	select_list.cur_y = id;

	skp.set_size(
		select_list.x + select_list.w + TAB_PADDING + scroll1.size_x + 20,
		select_list.y + 30 + 50,
		list_w,
		230 //select_list.h - 50 - 50
	);

	SelectList_Draw();
	SelectList_DrawBorder();
	//DrawWideRectangle(0, 0, Form.cwidth, Form.cheight, LP, sc.work);

	if (tabs.active_tab == SKINS)
	{
		update_docky.draw(skp.x, select_list.y+15);
		DrawFrame(skp.x, skp.y, skp.w, skp.h, " Components Preview ");
		DrawUiElementsPreview(skp.x+20, skp.y, skp.h);
	}
	if (tabs.active_tab == WALLPAPERS)
	{
		skp.x -= TAB_PADDING + 3;
		DrawStandartCaptButton(skp.x, select_list.y, BTN_SELECT_WALLP_FOLDER, T_SELECT_FOLDER);
		DrawFrame(skp.x, select_list.y+50, 180, 80, T_PICTURE_MODE);
		optionbox_stretch.draw(skp.x+14, select_list.y+70);
		optionbox_tiled.draw(skp.x+14, select_list.y+97);
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
		if (tabs.active_tab==SKINS) {
			if (strcmpi(#temp_filename+strlen(#temp_filename)-4,".skn")!=0) continue;
		}
		if (tabs.active_tab==WALLPAPERS) {
			if (strcmpi(#temp_filename+strlen(#temp_filename)-4,".png")!=0)
			&& (strcmpi(#temp_filename+strlen(#temp_filename)-4,".jpg")!=0) 
			&& (strcmpi(#temp_filename+strlen(#temp_filename)-5,".jpeg")!=0)
			&& (strcmpi(#temp_filename+strlen(#temp_filename)-4,".gif")!=0) continue;
		}
		cur = select_list.count;
		files_mas[cur]=j;
		select_list.count++;
	}
	Sort_by_Name(0, select_list.count-1);
}

void Sort_by_Name(int a, b) // for the first call: a = 0, b = sizeof(mas) - 1
{                                        
	int j;
	int isn = a;
	if (a >= b) return;
	for (j = a; j <= b; j++) {
		if (strcmpi(io.dir.position(files_mas[j]), io.dir.position(files_mas[b]))<=0) { 
			files_mas[isn] >< files_mas[j]; 
			isn++;
		}
	}
	Sort_by_Name(a, isn-2);
	Sort_by_Name(isn, b);
}

void SelectList_DrawLine(dword i)
{
	int yyy;

	cur = select_list.first + i;
	strcpy(#temp_filename, io.dir.position(files_mas[cur]));
	temp_filename[strlen(#temp_filename)-4] = 0;
	yyy = i*select_list.item_h+select_list.y;
	
	if (select_list.cur_y-select_list.first==i)
	{
		DrawBar(select_list.x, yyy, select_list.w, select_list.item_h, sc.button);
		WriteText(select_list.x+12,yyy+select_list.text_y,select_list.font_type,sc.button_text, #temp_filename);
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

void ActivateTab(int _id)
{
	select_list.ClearList();
	Open_Dir();
	if (!select_list.count) notify("'No files were found' -E");
	select_list.cur_y = _id;
	if (select_list.cur_y>select_list.visible) select_list.first=select_list.cur_y; 
	select_list.CheckDoesValuesOkey();	
	if (select_list.w) draw_window();
}

//===================================================//
//                                                   //
//                     EVENTS                        //
//                                                   //
//===================================================//

void EventTabSkinsClick()
{
	active_wallpaper = select_list.cur_y;
	strcpy(#folder_path, #skins_folder_path);
	ActivateTab(active_skin);
}

void EventTabWallpappersClick()
{
	active_skin = select_list.cur_y;
	strcpy(#folder_path, #wallp_folder_path);
	ActivateTab(active_wallpaper);
}

void EventDeleteFile()
{
	io.del(#cur_file_path);
	Open_Dir();
	EventApply();
}

void EventSetNewCurrent()
{
	cur = select_list.cur_y;
	sprintf(#cur_file_path,"%s/%s",#folder_path,io.dir.position(files_mas[cur]));
}

void EventSelectWallpFolder()
{
	OpenDialog_start stdcall (#o_dialog);
	if (o_dialog.status) {
		strcpy(#wallp_folder_path, #opendir_path);
		EventTabWallpappersClick();
	}
}

void EventSetWallpMode_Stretch()
{
	optionbox_tiled.checked = false;
	optionbox_tiled.redraw();
	EventApply();
}

void EventSetWallpMode_Tiled()
{
	optionbox_stretch.checked = false;
	optionbox_stretch.redraw();
	EventApply();
}

#include "..\lib\patterns\restart_process.h"
void EventApply()
{
	char kivpath[4096+10];
	EventSetNewCurrent();
	if (tabs.active_tab==SKINS)
	{
		cur = select_list.cur_y;
		SetSystemSkin(#cur_file_path);
		SelectList_Draw();
		strcpy(#cur_skin_path, #cur_file_path);
		EventUpdateDocky();
	} 
	if (tabs.active_tab==WALLPAPERS)
	{
		SelectList_Draw();
		if (optionbox_stretch.checked) strcpy(#kivpath, "\\S__");
		if (optionbox_tiled.checked) strcpy(#kivpath, "\\T__");
		strcat(#kivpath, #cur_file_path);
		RunProgram("/sys/media/kiv", #kivpath);
	}
}

void EventUpdateDocky()
{
	if (!update_docky.checked) return;
	KillProcessByName("@docky", MULTIPLE);
	RunProgram("/sys/@docky",NULL);
	pause(50);
	ActivateWindow(GetProcessSlot(Form.ID));
}

void EventOpenFile()
{
	if (tabs.active_tab==SKINS) RunProgram("/sys/skincfg", #cur_file_path);
	if (tabs.active_tab==WALLPAPERS) RunProgram("/sys/media/kiv", #cur_file_path);
}

_ini ini = { "/sys/settings/system.ini", "style" };
void EventExit()
{
	if (cur_skin_path) ini.SetString("skin", #cur_skin_path, strlen(#cur_skin_path));
	ExitProcess();
}

void EventHandleMenuClick()
{
	switch (get_menu_click()) 
	{
		case 1: 
			EventOpenFile(); 
			break;
		case 2: 
			EventDeleteFile();
			break;
	};
}

stop:
