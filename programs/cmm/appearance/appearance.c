//11.03.12 - start!
//ver 2.31

#define MEMSIZE 1024*70
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
char ss_available[200];

int screensaver_timeout;

_tabs tabs = { -sizeof(t_skins)-sizeof(t_wallpapers)-sizeof(t_screensaver)-3*8+WIN_W
	 - TAB_PADDING / 2, LP, NULL, BASE_TAB_BUTTON_ID };

checkbox update_docky = { T_UPDATE_DOCK, false };

checkbox optionbox_stretch = { T_CHECKBOX_STRETCH, false };
checkbox optionbox_tiled = { T_CHECKBOX_TILED, false };
checkbox optionbox_auto = { T_CHECKBOX_AUTO, true };

collection list;

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void main()
{   
	int id, i;
	load_dll(boxlib, #box_lib_init,0);
	load_dll(libini, #lib_init,1);
	load_dll(Proc_lib, #OpenDialog_init,0);
	o_dialog.type = 2; //select folder
	OpenDialog_init stdcall (#o_dialog);

	tabs.add(#t_skins, #EventTabSkinsClick);	
	tabs.add(#t_wallpapers, #EventTabWallpappersClick);
	tabs.add(#t_screensaver, #EventTabScreensaverClick);
	tabs.draw_active_tab();

	GetScreensaverIniSettings();

	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
	loop() switch(WaitEvent()) 
	{
	  	case evMouse:
			SelectList_ProcessMouse();
	  		break;

		case evButton:
			id=GetButtonID();
			if (id==1) EventExit();
			tabs.click(id);
			if (tabs.active_tab == TAB_SKINS) {
				checkbox1.click(id);
				spinbox1.click(id);
				if (update_docky.click(id)) EventUpdateDocky();
			}
			if (tabs.active_tab == TAB_WALLPAPERS) {
				if (id==BTN_SELECT_WALLP_FOLDER) EventSelectWallpFolder();
				if (optionbox_stretch.click(id)) EventSetWallpMode(1,0,0);
				if (optionbox_tiled.click(id)) EventSetWallpMode(0,1,0);
				if (optionbox_auto.click(id)) EventSetWallpMode(0,0,1);
			}
			if (tabs.active_tab == TAB_SCREENSAVERS) {
				if (id==BTN_TEST_SCREENSAVER) EventOpenFile();
			}
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
				break;
			}

			if (! edit_cmm.flags & ed_focus) && (! edit_st.flags & ed_focus)
			for (i=select_list.cur_y+1; i<select_list.count; i++)
			{
				id = ESBYTE[list.get(i)];
				if (id==ESBYTE[EAX]) || (id==key_ascii-32)
				{
					select_list.cur_y = i - 1;
					select_list.KeyDown();
					EventApply();
					break;
				}
			}

			if (tabs.active_tab == TAB_SKINS) {
				EAX = key_editbox;
				edit_box_key stdcall (#edit_cmm);
				edit_box_key stdcall (#edit_st);				
			}
			break;
		 
		 case evReDraw:		
			draw_window();
   }
}

void draw_window()
{
	sc.get();
	DefineAndDrawWindow(screen.width-WIN_W-9/2,80,WIN_W+9,WIN_H+4+skin_height,0x34,sc.work,WINDOW_HEADER,0);
	DrawWindowContent();
}

void DrawWindowContent()
{
	sc.get();	

	tabs.draw();
	draw_icon_16w(tabs.x + TAB_PADDING, LP+5, 17);
	draw_icon_16w(sizeof(t_skins)-1*8 + TAB_PADDING + TAB_PADDING + tabs.x, LP+5, 6);
	draw_icon_16w(sizeof(t_wallpapers)+sizeof(t_skins)-2*8 + TAB_PADDING + TAB_PADDING + TAB_PADDING + tabs.x, LP+5, 61);

	$push select_list.cur_y
	SelectList_Init(
		LP + TAB_PADDING,
		PANEL_H, 
		LIST_W, 
		WIN_H - LP - TAB_PADDING - PANEL_H
		);
	$pop select_list.cur_y

	DrawBar(RIGHTx, PANEL_H, RIGHTw, WIN_H-PANEL_H-LP, sc.work);

	SelectList_Draw();
	SelectList_DrawBorder();

	if (tabs.active_tab == TAB_SKINS)
	{
		DrawFrame(RIGHTx, PANEL_H+5, RIGHTw, RIGHTh, T_UI_PREVIEW);
		DrawUiElementsPreview(RIGHTx+20, PANEL_H+5, RIGHTh);
		if (CheckProcessExists("@DOCKY")) update_docky.draw(RIGHTx, PANEL_H+250);
	}
	if (tabs.active_tab == TAB_WALLPAPERS)
	{
		DrawFrame(RIGHTx, PANEL_H+5, 180, 105, T_PICTURE_MODE);
		optionbox_stretch.draw(RIGHTx+14, PANEL_H+25);
		optionbox_tiled.draw(RIGHTx+14, PANEL_H+52);
		optionbox_auto.draw(RIGHTx+14, PANEL_H+79);
		DrawStandartCaptButton(RIGHTx, PANEL_H+130, BTN_SELECT_WALLP_FOLDER, T_SELECT_FOLDER);
	}
	if (tabs.active_tab == TAB_SCREENSAVERS)
	{
		DrawStandartCaptButton(RIGHTx, PANEL_H, BTN_TEST_SCREENSAVER, T_SCREENSAVER_PREVIEW);
	}
}

bool strreqi(dword _left, _right)
{
	return strcmp(_left+strrchr(_left,'.'), _right);
}

dword files_mas[400];
void Open_Dir()
{
	int j;
	char fname[4096];
	select_list.ClearList();
	if(io.dir.buffer)free(io.dir.buffer);
	io.dir.load(#folder_path,DIR_ONLYREAL);
	for (j=0; j<io.dir.count; j++)
	{
		strcpy(#fname, io.dir.position(j));
		strlwr(#fname);
		if (tabs.active_tab==TAB_SKINS) {
			if (strreqi(#fname,"skn")!=0) continue;
		}
		if (tabs.active_tab==TAB_WALLPAPERS) {
			if (strreqi(#fname,"png")!=0)
			&& (strreqi(#fname,"jpg")!=0) 
			&& (strreqi(#fname,"jpeg")!=0)
			&& (strreqi(#fname,"bmp")!=0)
			&& (strreqi(#fname,"gif")!=0) continue;
		}
		ESDWORD[select_list.count*4 + #files_mas] = j;
		select_list.count++;
	}
	Sort_by_Name(0, select_list.count-1);
	list.drop();
	for (j=0; j<select_list.count; j++) {
		list.add(io.dir.position(files_mas[j]));
	}
	if (!select_list.count) notify(T_NO_FILES);
	//save current item for tab change
	switch(tabs.active_tab) {
		CASE TAB_SKINS: select_list.cur_y = active_skin; BREAK;
		CASE TAB_WALLPAPERS: select_list.cur_y = active_wallpaper; BREAK;
		CASE TAB_SCREENSAVERS: select_list.cur_y = active_screensaver;
	}
	if (select_list.cur_y>select_list.visible) select_list.first=select_list.cur_y; 
	select_list.CheckDoesValuesOkey();	
	if (LIST_W) draw_window();
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
	int draw_y = i*SELECT_LIST_ITEMH+PANEL_H;
	int i_abs = select_list.first + i;
	char filename_buf[4096];
	char* filename = #filename_buf;

	strcpy(filename, list.get(i_abs));
	EAX = math.min(strrchr(filename,'.')-1, LIST_W - 24 / 8);
	filename_buf[EAX] = '\0';
	if (EAX = strrchr(filename,'/')) filename += EAX;

	//save current item for tab change
	switch(tabs.active_tab) {
		CASE TAB_SKINS: active_skin = select_list.cur_y; BREAK;
		CASE TAB_WALLPAPERS: active_wallpaper = select_list.cur_y; BREAK;
		CASE TAB_SCREENSAVERS: active_screensaver = select_list.cur_y;
	}
	
	if (select_list.cur_y == i_abs)
	{
		DrawBar(select_list.x, draw_y, LIST_W, SELECT_LIST_ITEMH, sc.button);
		WriteText(select_list.x+12,draw_y+select_list.text_y,select_list.font_type,sc.button_text, filename);
	}
	else
	{
		DrawBar(select_list.x,draw_y,LIST_W, SELECT_LIST_ITEMH, 0xFFFfff);
		WriteText(select_list.x+12,draw_y+select_list.text_y,select_list.font_type,0, filename);
	}
}

void SelectList_LineChanged() 
{
	EventApply();
}

dword GetRealKolibriosPath()
{
	char real_kolibrios_path[4096];
	SetCurDir("/kolibrios");
	GetCurDir(#real_kolibrios_path, sizeof(real_kolibrios_path));
	return #real_kolibrios_path;
}

void GetScreensaverIniSettings()
{
	ini.section = "screensaver";
	screensaver_timeout = ini.GetInt("timeout", 10);
	ini.GetString("available", #ss_available, sizeof(ss_available), 0);
}

//===================================================//
//                                                   //
//                     EVENTS                        //
//                                                   //
//===================================================//

void EventTabSkinsClick()
{
	miniprintf(#folder_path, "%s/res/skins", GetRealKolibriosPath());
	Open_Dir();
}

void EventTabWallpappersClick()
{
	if (opendir_path) {
		strcpy(#folder_path, #opendir_path);
	} else {
		miniprintf(#folder_path, "%s/res/wallpapers", GetRealKolibriosPath());
	}
	Open_Dir();
}

void EventTabScreensaverClick()
{
	dword j;
	char ssmas[sizeof(ss_available)];
	list.drop();
	select_list.ClearList();

	strcpy(#ssmas, #ss_available);
	do {
		j = strrchr(#ss_available, '|');
		miniprintf(#param, "/sys/%s", #ss_available + j);
		list.add(#param);
		ESBYTE[#ss_available + j - 1] = '\0';
		select_list.count++;
	} while (j);

	if (LIST_W) draw_window();
}

void EventDeleteFile()
{
	DeleteFile(#cur_file_path);
	Open_Dir();
	EventApply();
}

void EventSelectWallpFolder()
{
	OpenDialog_start stdcall (#o_dialog);
	if (o_dialog.status) EventTabWallpappersClick();
}

void EventSetWallpMode(dword _stretch, _titled, _auto)
{
	optionbox_stretch.checked = _stretch;
	optionbox_tiled.checked = _titled;
	optionbox_auto.checked = _auto;
	optionbox_tiled.redraw();
	optionbox_stretch.redraw();
	optionbox_auto.redraw();
	EventApply();
}

void EventApply()
{
	char kivparam[4096+10];
	dword file_name = list.get(select_list.cur_y);
	miniprintf(#cur_file_path,"%s/",#folder_path);
	strcat(#cur_file_path, list.get(select_list.cur_y));
	if (tabs.active_tab==TAB_SKINS)
	{
		SetSystemSkin(#cur_file_path);
		SelectList_Draw();
		strcpy(#cur_skin_path, #cur_file_path);
		EventUpdateDocky();
	} 
	if (tabs.active_tab==TAB_WALLPAPERS)
	{
		SelectList_Draw();
		miniprintf(#kivparam, "\\S__%s", #cur_file_path);
		if (optionbox_tiled.checked) kivparam[1]='T';
		if (optionbox_auto.checked) {
			if (ESBYTE[file_name+1] == ' ') && (ESBYTE[file_name] == 'T') {
				kivparam[1]='T';
			}
		}
		RunProgram("/sys/media/kiv", #kivparam);
	}
	if (tabs.active_tab==TAB_SCREENSAVERS)
	{
		SelectList_Draw();
	}
}

void EventUpdateDocky()
{
	if (!update_docky.checked) return;
	KillProcessByName("@docky", MULTIPLE);
	RunProgram("/sys/@docky",NULL);
	pause(50);
	ActivateWindow_Self();
}

void EventOpenFile()
{
	if (tabs.active_tab==TAB_SKINS) RunProgram("/sys/skincfg", #cur_file_path);
	if (tabs.active_tab==TAB_WALLPAPERS) RunProgram("/sys/media/kiv", #cur_file_path);
	if (tabs.active_tab==TAB_SCREENSAVERS) RunProgram(list.get(select_list.cur_y), "@ss");
}

void EventExit()
{
	if (cur_skin_path) {
		ini.section = "style";
		ini.SetString("skin", #cur_skin_path, strlen(#cur_skin_path));
	}
	ExitProcess();
}

stop:
