//11.03.12 - start!
//ver 2.31

#define MEMSIZE 200*1024
#include "../lib/mem.h"
#include "../lib/strings.h"
#include "../lib/io.h"
#include "../lib/list_box.h"
#include "../lib/gui.h"

#include "../lib/obj/box_lib.h"
#include "../lib/obj/proc_lib.h"
#include "../lib/obj/libini.h"

#include "../lib/patterns/select_list.h"
#include "../lib/patterns/simple_open_dialog.h"
#include "../lib/patterns/restart_process.h"

#include "ui_elements_preview.h"
#include "const.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

signed int active_skin=-1, active_wallpaper=-1, active_screensaver=-1;
enum { 
	BASE_TAB_BUTTON_ID=3, 
	BTN_SELECT_WALLP_FOLDER=10,
	BTN_TEST_SCREENSAVER };

char folder_path[4096];
char cur_file_path[4096];
char cur_skin_path[4096];
char temp_filename[4096];
int files_mas[400];

_ini ini = { "/sys/settings/system.ini" };

int cur;

proc_info Form;
block skp;

enum {SKINS, WALLPAPERS, SCREENSAVERS};

_tabs tabs = { -sizeof(t_skins)-sizeof(t_wallpapers)-sizeof(t_screensaver)-3*8+WIN_W
	 - TAB_PADDING / 2, LP, NULL, BASE_TAB_BUTTON_ID };

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

void main()
{   
	int id;
	load_dll(boxlib, #box_lib_init,0);
	load_dll(libini, #lib_init,1);
	load_dll(Proc_lib, #OpenDialog_init,0);
	o_dialog.type = 2; //select folder
	OpenDialog_init stdcall (#o_dialog);

	tabs.add(#t_skins, #EventTabSkinsClick);	
	tabs.add(#t_wallpapers, #EventTabWallpappersClick);
	tabs.add(#t_screensaver, #EventTabScreensaverClick);
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
			if (key_scancode==SCAN_CODE_DEL) EventDeleteFile();
			if (key_scancode==SCAN_CODE_TAB) {
				id = tabs.active_tab+1; 
				if(id==3)id=0;
				tabs.click(id + tabs.base_id);
				DrawWindowContent();
				break;
			}

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
	DefineAndDrawWindow(screen.width-600/2,80,WIN_W+9,WIN_H+skin_height,0x34,sc.work,WINDOW_HEADER,0);
	GetProcessInfo(#Form, SelfInfo);
	IF (Form.status_window&ROLLED_UP) return;
	DrawWindowContent();
}

void DrawWindowContent()
{
	int id;

	sc.get();	

	//tabs.w = Form.cwidth-LP-LP;
	tabs.draw();
	draw_icon_16w(tabs.x + TAB_PADDING, LP+5, 17);
	draw_icon_16w(sizeof(t_skins)-1*8 + TAB_PADDING + TAB_PADDING + tabs.x, LP+5, 6);
	draw_icon_16w(sizeof(t_wallpapers)+sizeof(t_skins)-2*8 + TAB_PADDING + TAB_PADDING + TAB_PADDING + tabs.x, LP+5, 61);

	id = select_list.cur_y;
	SelectList_Init(
		LP + TAB_PADDING,
		PANEL_H, 
		LIST_W, 
		Form.cheight-LP - TAB_PADDING - PANEL_H
		);
	select_list.cur_y = id;

	skp.set_size(
		LP + TAB_PADDING + LIST_W + TAB_PADDING + 30,
		PANEL_H,
		226,
		230 //select_list.h - 50 - 50
	);
	DrawBar(skp.x, skp.y, skp.w, WIN_H, sc.work);

	SelectList_Draw();
	SelectList_DrawBorder();

	if (tabs.active_tab == SKINS)
	{
		DrawFrame(skp.x, PANEL_H+5, skp.w, skp.h, " Components Preview ");
		DrawUiElementsPreview(skp.x+20, PANEL_H+5, skp.h);
		if (CheckProcessExists("@DOCKY")) update_docky.draw(skp.x, PANEL_H+250);
	}
	if (tabs.active_tab == WALLPAPERS)
	{
		DrawFrame(skp.x, PANEL_H+5, 180, 80, T_PICTURE_MODE);
		optionbox_stretch.draw(skp.x+14, PANEL_H+25);
		optionbox_tiled.draw(skp.x+14, PANEL_H+52);
		DrawStandartCaptButton(skp.x, PANEL_H+100, BTN_SELECT_WALLP_FOLDER, T_SELECT_FOLDER);
	}
	if (tabs.active_tab == SCREENSAVERS)
	{
		DrawStandartCaptButton(skp.x, PANEL_H, BTN_TEST_SCREENSAVER, "Test");
	}
}

bool strreqi(dword _left, _right)
{
	return strcmpi(_left+strrchr(_left,'.')-1, _right);
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
			if (strreqi(#temp_filename,".skn")!=0) continue;
		}
		if (tabs.active_tab==WALLPAPERS) {
			if (strreqi(#temp_filename,".png")!=0)
			&& (strreqi(#temp_filename,".jpg")!=0) 
			&& (strreqi(#temp_filename,".jpeg")!=0)
			&& (strreqi(#temp_filename,".gif")!=0) continue;
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
	if (select_list.w) DrawWindowContent();
}

dword GetRealKolibriosPath()
{
	char real_kolibrios_path[4096];
	SetCurDir("/kolibrios");
	GetCurDir(#real_kolibrios_path, sizeof(real_kolibrios_path));
	return #real_kolibrios_path;
}

//===================================================//
//                                                   //
//                     EVENTS                        //
//                                                   //
//===================================================//

void EventTabSkinsClick()
{
	active_wallpaper = select_list.cur_y;
	miniprintf(#folder_path, "%s/res/skins", GetRealKolibriosPath());
	ActivateTab(active_skin);
}

void EventTabWallpappersClick()
{
	active_skin = select_list.cur_y;
	if (opendir_path) {
		strcpy(#folder_path, #opendir_path);
	} else {
		miniprintf(#folder_path, "%s/res/wallpapers", GetRealKolibriosPath());
	}
	ActivateTab(active_wallpaper);
}

void EventTabScreensaverClick()
{
	//strcpy(#folder_path, #wallp_folder_path);
	ActivateTab(active_screensaver);
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
	miniprintf(#cur_file_path,"%s/",#folder_path);
	strcat(#cur_file_path, io.dir.position(files_mas[cur]));
}

void EventSelectWallpFolder()
{
	OpenDialog_start stdcall (#o_dialog);
	if (o_dialog.status) EventTabWallpappersClick();
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
		if (optionbox_stretch.checked) miniprintf(#kivpath, "\\S__%s", #cur_file_path);
		if (optionbox_tiled.checked) miniprintf(#kivpath, "\\T__%s", #cur_file_path);
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

void EventExit()
{
	if (cur_skin_path) {
		ini.section = "style";
		ini.SetString("skin", #cur_skin_path, strlen(#cur_skin_path));
	}
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
