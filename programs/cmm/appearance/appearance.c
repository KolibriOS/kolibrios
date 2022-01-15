//11.03.12 - start!

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

dword fmas;

#include "ui_elements_preview.h"
#include "const.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

int active_skin=-1, active_wallpaper=-1, active_screensaver=-1;

checkbox optionbox_stretch = { T_CHECKBOX_STRETCH, false };
checkbox optionbox_tiled = { T_CHECKBOX_TILED, false };
checkbox optionbox_auto = { T_CHECKBOX_AUTO, true };

char ss_available[200];

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
	OpenDialog_init stdcall (#o_dialog);

	GetIniSettings();

	tabs.add(#t_skins, #EventTabSkinsClick);	
	tabs.add(#t_wallpapers, #EventTabWallpappersClick);
	tabs.add(#t_screensaver, #EventTabScreensaverClick);
	tabs.draw_active_tab();

	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
	loop() switch(WaitEvent()) 
	{
	  	case evMouse:
			SelectList_ProcessMouse();
			if (tabs.active_tab == TAB_SCREENSAVERS) {
				scrollbar_h_mouse stdcall (#ss_timeout);
				if (ss_timeout.redraw) {
					draw_timeout();
					ss_timeout.redraw = false; //reset flag
				}
			}
	  		break;

		case evButton:
			id=GetButtonID();
			if (id==1) EventExit();
			tabs.click(id);
			if (tabs.active_tab == TAB_SKINS) {
				checkbox1.click(id);
				spinbox1.click(id);
			}
			if (tabs.active_tab == TAB_WALLPAPERS) {
				if (id==BTN_SELECT_WALLP_FOLDER) EventSelectWallpFolder();
				if (optionbox_stretch.click(id)) EventSetWallpMode(1,0,0);
				if (optionbox_tiled.click(id)) EventSetWallpMode(0,1,0);
				if (optionbox_auto.click(id)) EventSetWallpMode(0,0,1);
			}
			if (tabs.active_tab == TAB_SCREENSAVERS) {
				if (id==BTN_TEST_SCREENSAVER) EventOpenFile();
				if (id==BTN_SET_SCREENSAVER) EventSetSs();
			}
			break;
	  
		case evKey:
			GetKeys(); 
			if (select_list.ProcessKey(key_scancode)) { EventApply(); break; }
			if (key_scancode==SCAN_CODE_ENTER) { EventOpenFile(); break; }
			if (key_scancode==SCAN_CODE_DEL) { EventDeleteFile(); break; }
			if (key_scancode==SCAN_CODE_TAB) {
				id = tabs.active_tab+1; 
				if(id==3)id=0;
				tabs.click(id + tabs.base_id);
				break;
			}
			for (i=select_list.cur_y+1; i<select_list.count; i++)
			{
				id = list.get(i) + strrchr(list.get(i), '/');
				if (ESBYTE[id]==key_ascii) || (ESBYTE[id]==key_ascii-32)
				{
					select_list.cur_y = i - 1;
					select_list.KeyDown();
					EventApply();
					break;
				}
			}
			break;
		 
		 case evReDraw:		
			draw_window();
   }
}

void draw_window()
{
	sc.get();
	DefineAndDrawWindow(screen.w-WIN_W-9/2,80,WIN_W+9,WIN_H+4+skin_h,0x74,sc.work,WINDOW_HEADER,0);

	DrawBar(0, 0, WIN_W, PANEL_H-2, sc.work); //top
	DrawBar(0, PANEL_H-2, LP-2, WIN_H-PANEL_H-LP+4, EDX); //left
	DrawBar(LIST_W+LP+20, PANEL_H-2, WIN_W-LIST_W-26, WIN_H-PANEL_H-LP+4, EDX); //right
	DrawBar(0, WIN_H-LP+2, WIN_W, LP-2, EDX); //bottom

	tabs.draw();
	draw_icon_16w(tabs.x + TAB_P, LP+5, 17);
	draw_icon_16w(sizeof(t_skins)-1*8 + TAB_P + TAB_P + tabs.x, LP+5, 6);
	draw_icon_16w(sizeof(t_wallpapers)+sizeof(t_skins)-2*8 + TAB_P + TAB_P + TAB_P + tabs.x, LP+5, 61);

	if (tabs.active_tab == TAB_SKINS)
	{
		DrawFrame(RIGHTx, PANEL_H+5, RIGHTw, RIGHTh, T_UI_PREVIEW);
		DrawUiElementsPreview(RIGHTx+20, PANEL_H+5, RIGHTh);
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
		draw_timeout();
		ss_timeout.line_col = sc.line;
		ss_timeout.frnt_col = sc.work;
		scrollbar_h_draw stdcall (#ss_timeout);
		DrawRectangle(RIGHTx, RIGHTy+25, RIGHTw-20, 15, sc.line);
		ESI = DrawStandartCaptButton(RIGHTx, PANEL_H + 65, BTN_TEST_SCREENSAVER, T_SS_PREVIEW);
		DrawStandartCaptButton(RIGHTx+ESI, PANEL_H + 65, BTN_SET_SCREENSAVER, T_SS_SET);
	}

	$push select_list.cur_y
	SelectList_Init(
		LP,
		PANEL_H, 
		LIST_W, 
		WIN_H - LP - PANEL_H
		);
	$pop select_list.cur_y

	SelectList_Draw();
	SelectList_DrawBorder();
}

void draw_timeout()
{
	miniprintf(#param, T_SS_TIMEOUT, ss_timeout.position+1);
	WriteTextWithBg(RIGHTx, PANEL_H, 0xD0, sc.work_text, #param, sc.work);	
}

void add_filesnames_into_the_list()
{
	int j;
	for (j=0; j<select_list.count; j++) {
		miniprintf(#param,"%s/",#folder_path);
		strcat(#param, io.dir.position(ESDWORD[j*4+fmas]));
		list.add(#param);
	}
}

void Open_Dir()
{
	int j;
	dword ext;
	select_list.ClearList();

	if (io.dir.buffer) free(io.dir.buffer);
	io.dir.load(#folder_path,DIR_ONLYREAL);

	if (fmas) free(fmas);
	fmas = malloc(io.dir.count * 4);

	for (j=0; j<io.dir.count; j++)
	{
		strncpy(#param, io.dir.position(j), PATHLEN);
		strlwr(#param);
		ext = #param + strrchr(#param,'.');
		if (tabs.active_tab==TAB_SKINS) {
			if (!streq(ext,"skn")) continue;
		}
		if (tabs.active_tab==TAB_WALLPAPERS) {
			if (!streq(ext,"png")) && (!streq(ext,"jpg")) 
			&& (!streq(ext,"jpeg")) && (!streq(ext,"bmp"))
			&& (!streq(ext,"gif")) continue;
		}
		ESDWORD[select_list.count*4 + fmas] = j;
		select_list.count++;
	}
	sort_by_name(0, select_list.count-1);

	list.drop();
	//save current item for tab change
	//add default item
	if(tabs.active_tab == TAB_SKINS) 
	{
		select_list.count++;
		list.add(#default_skin);
		add_filesnames_into_the_list();
		if (active_skin==-1) && (ESBYTE[#previous_skin]) 
		{
			for (j=0; j<select_list.count; j++) {
				if (streq(list.get(j), #previous_skin)) {
					active_skin = j;
					break;
				}
			}
		}
		select_list.cur_y = active_skin;
	} else { 
		select_list.count++;
		list.add(#default_wallp);
		add_filesnames_into_the_list();
		if (active_wallpaper==-1) && (ESBYTE[#previous_wallp]=='\\') 
		{
			for (j=0; j<select_list.count; j++) {
				if (streq(list.get(j), #previous_wallp+4)) {
					active_wallpaper = j;
					break;
				}
			}
		}
		select_list.cur_y = active_wallpaper; 
	}

	if (!select_list.count) notify(T_NO_FILES);
	if (select_list.cur_y>SL_VISIBLE) {
		select_list.first = -SL_VISIBLE/2 + select_list.cur_y; 
	}
	select_list.CheckDoesValuesOkey();	
	if (LIST_W) draw_window();
}

void SelectList_DrawLine(dword i)
{
	int draw_y = i*SELECT_LIST_ITEMH+PANEL_H;
	int i_abs = select_list.first + i;
	dword text_color = 0xFFFfff;
	dword bg_color = 0x000000;
	char filename_buf[PATHLEN];
	char* filename = #filename_buf;

	strcpy(filename, list.get(i_abs));
	if (EAX = strrchr(filename,'/')) filename += EAX;
	if (ESBYTE[filename]=='T') && (ESBYTE[filename+1]=='_') filename+=2;
	EAX = math.min(strrchr(filename,'.')-1, LIST_W - 24 / 8);
	if(EAX) ESBYTE[filename+EAX] = '\0';

	//save current item for tab change
	switch(tabs.active_tab) {
		CASE TAB_SKINS: 
				active_skin = select_list.cur_y;
				if (!i_abs) filename = T_DEFAULT;
				BREAK;
		CASE TAB_WALLPAPERS: 
				active_wallpaper = select_list.cur_y; 
				if (!i_abs) filename = T_DEFAULT;
				BREAK;
		CASE TAB_SCREENSAVERS: 
				active_screensaver = select_list.cur_y;
				if (!i_abs) filename = T_NO_SS;
	}
	
	if (select_list.cur_y == i_abs) {
		text_color = sc.button;
		bg_color = sc.button_text;
	}
	DrawBar(select_list.x, draw_y, LIST_W, SELECT_LIST_ITEMH, text_color);
	WriteText(select_list.x+12,draw_y+select_list.text_y,select_list.font_type,bg_color, filename);
}



void GetIniSettings()
{
	ini.section = "style";
	ini.GetString("default_skin", #default_skin, PATHLEN, 0);
	ini.GetString("default_wallp", #default_wallp, PATHLEN, 0);
	ini.GetString("skin", #previous_skin, PATHLEN, 0);
	ini.GetString("bg_param", #previous_wallp, PATHLEN, 0);

	ini.section = "screensaver";
	ss_timeout.position = ini.GetInt("timeout", 10) - 1;
	ini.GetString("available", #ss_available, sizeof(ss_available), 0);
	ini.GetString("program", #previous_ss, PATHLEN, 0);
}

//===================================================//
//                                                   //
//                     EVENTS                        //
//                                                   //
//===================================================//

void EventTabSkinsClick()
{
	miniprintf(#folder_path, "%s/res/skins", get_real_kolibrios_path());
	Open_Dir();
}

void EventTabWallpappersClick()
{
	if (opendir_path) {
		strcpy(#folder_path, #opendir_path);
	} else {
		miniprintf(#folder_path, "%s/res/wallpapers", get_real_kolibrios_path());
	}
	Open_Dir();
}

void EventTabScreensaverClick()
{
	dword j;
	char ssmas[sizeof(ss_available)];
	list.drop();
	select_list.ClearList();

	select_list.count++;
	list.add("");

	strcpy(#ssmas, #ss_available);
	do {
		j = strrchr(#ssmas, '|');
		if (ssmas[j]=='/') {
			strcpy(#param, #ssmas + j);
		} else {
			miniprintf(#param, "/sys/%s", #ssmas + j);
		}
		list.add(#param);
		ESBYTE[#ssmas + j - 1] = '\0';
		select_list.count++;
	} while (j);

	if (active_screensaver == -1) && (ESBYTE[#previous_ss]) {
		for (j=0; j<select_list.count; j++) {
			if (strstr(list.get(j), #previous_ss)) active_screensaver = j;
		}
	}
	select_list.cur_y = active_screensaver;

	if (LIST_W) draw_window();
}

void EventDeleteFile()
{
	if (select_list.cur_y) DeleteFile(list.get(select_list.cur_y)); //no not delete default
	Open_Dir();
	EventApply();
}

void EventSelectWallpFolder()
{
	o_dialog.type = 2; //select folder
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
	if (tabs.active_tab==TAB_SKINS)
	{
		strcpy(#cur_skin_path, list.get(select_list.cur_y));
		SetSystemSkin(#cur_skin_path);
		MoveSize(OLD, OLD, OLD, WIN_H+4+GetSkinHeight());
	} 
	if (tabs.active_tab==TAB_WALLPAPERS)
	{
		SelectList_Draw();
		miniprintf(#kivparam, "\\S__%s", list.get(select_list.cur_y));
		if (optionbox_tiled.checked) || (!select_list.cur_y) kivparam[1]='T';
		if (optionbox_auto.checked) {
			file_name += strrchr(file_name, '/');
			if (ESBYTE[file_name] == 'T') && (ESBYTE[file_name+1] == '_') {
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

void EventOpenFile()
{
	switch (tabs.active_tab) {
		case TAB_SKINS: 
				RunProgram("/sys/skincfg", list.get(select_list.cur_y)); 
				break;
		case TAB_WALLPAPERS: 
				RunProgram("/sys/media/kiv", list.get(select_list.cur_y)); 
				break;
		case TAB_SCREENSAVERS: 
				if(select_list.cur_y) {
					KillProcessByName("@ss", MULTIPLE);
					RunProgram(list.get(select_list.cur_y), "@ss");
				}
	}
}

void EventExit()
{
	if (get_real_kolibrios_path()) && (ESBYTE[#cur_skin_path]) {
		ini.section = "style";
		ini.SetString("skin", #cur_skin_path, strlen(#cur_skin_path));
	}
	ExitProcess();
}

void EventSetSs()
{
	dword cur_ss = list.get(select_list.cur_y);
	ini.section = "screensaver";
	ini.SetString("program", cur_ss, strlen(cur_ss));
	ini.SetInt("timeout", ss_timeout.position+1);
	RestartProcessByName("/sys/@ss", MULTIPLE);
}

stop:

char folder_path[PATHLEN];
char cur_skin_path[PATHLEN];

char default_skin[PATHLEN];
char default_wallp[PATHLEN];

char previous_skin[PATHLEN];
char previous_wallp[PATHLEN];
char previous_ss[PATHLEN];