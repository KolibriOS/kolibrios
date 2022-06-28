//Leency, Veliant, Punk_Joker, PavelYakov & KolibriOS Team 2008-2022
//GNU GPL license.

/*
BUGS:
- F1 in KFM (move Properties to an external app)
- Ctrl+1+2+3+4 in KFM
- Highlight another commands on Ctrl|Shift in KFM like in Classic KFM
TODO:
- add option Preserve all timestamps (Created, Opened, Modified)
  http://board.kolibrios.org/viewtopic.php?f=23&t=4521&p=77334#p77334
*/

#define ABOUT_TITLE "EOLITE 5.26"
#define TITLE_EOLITE "Eolite File Manager 5.26"
#define TITLE_KFM "Kolibri File Manager 2.26";

#define MEMSIZE 1024 * 250
#include "../lib/clipboard.h"
#include "../lib/strings.h"
#include "../lib/mem.h"
#include "../lib/fs.h"
#include "../lib/gui.h"
#include "../lib/list_box.h"
#include "../lib/random.h"
#include "../lib/kfont.h"
#include "../lib/collection.h"
#include "../lib/copyf.h"

#include "../lib/obj/libini.h"
#include "../lib/obj/box_lib.h"
#include "../lib/obj/libimg.h"

#include "../lib/patterns/history.h"

#include "imgs/images.h"
#include "include/const.h"

struct Eolite_colors
{
	bool  skin_is_dark;
	dword lpanel;
	dword list_vert_line; //vertical line between columns in list
	dword selec;
	dword selec_active;
	dword selec_inactive;
	dword selec_text;
	dword list_bg;
	dword list_gb_text;
	dword list_text_hidden;
	dword work_gradient[24];
	dword slider_bg_big;
	dword slider_bg_left;
	dword odd_line;
} col;
dword waves_pal[256];

//Global data
	bool efm = false;
	_history history;

//Folder data
	dword buf, buf_inactive;
	collection_int items=0;
	int folder_count;
	dword path;
	bool dir_at_fat16 = NULL;

//Selected element data
	byte file_path[4096];
	byte file_name[256];
	byte temp[4096];
	bool itdir;

//Window data
	proc_info Form;
	int sc_slider_h;
	bool scroll_used=false;
	char sort_type=2;
	bool sort_desc=false;
	int status_bar_h;
	int icon_size = 18;
	char active_popin = NULL;
	bool list_full_redraw;

//Threads data
	dword about_thread_id;
	dword settings_window;
	bool active_about = false;
	bool active_settings = false;
	dword about_stak=0,properties_stak=0,settings_stak=0;
	byte cmd_free=0;

//Multipanes
	int active_panel=0;
	#define PANES_COUNT 2
	dword location[PANES_COUNT];
	llist files, files_active, files_inactive;
	collection_int selected0, selected1;
	dword selected_count[PANES_COUNT]=0;

libimg_image icons16_default;
libimg_image icons16_selected;
libimg_image icons32_default;
libimg_image icons32_selected;

byte popin_string[4096];
edit_box popin_text = {200,213,180,0xFFFFFF,0x94AECE,0xFFFFFF,0xFFFFFF,0x10000000,
	248,#popin_string,0,ed_focus+ed_always_focus,6,0};

PathShow_data FileShow = {0, 56,215, 8, 100, 1, 0, 0x0, 0xFFFfff, #file_name, #temp, 0};

#include "include\settings.h"
#include "include\gui.h"
#include "include\progress_dialog.h"
#include "include\copy_and_delete.h"
#include "include\sorting.h"
#include "include\icons.h"
#include "include\left_panel.h"
#include "include\menu.h"
#include "include\about.h"
#include "include\properties.h"

void handle_param()
{
	//-p <path> : just show file/folder properties dialog
	//-d <path> : delete file/folder
	//-v : paste files/folder from clipboard
	int i;
	dword p = #param;
	if (streq(#program_path+strrchr(#program_path,'/'), "KFM")) {
		efm = true;
		//now we need to restore the original app name
		//without this external operations down won't work
		strcpy(#program_path+strrchr(#program_path,'/'), "EOLITE");
	}

	SetAppColors();
	LoadIniSettings();

	for (i=0; i<PANES_COUNT; i++) {
		location[i] = malloc(4096);
		strcpy(location[i], #path_start);
	}
	path = location[0];

	if (ESBYTE[p]=='\0') return;

	if (ESBYTE[p]=='-') switch (ESBYTE[p+1])
	{
		case 'p':
			strcpy(#file_path, p + 3);
			itdir = dir_exists(#file_path);
			strcpy(#file_name, p + strrchr(p, '/'));
			ESBYTE[strrchr(p, '/')+p-1] = '\0';
			strcpy(path, p + 3);
			properties_dialog();
			ExitProcess();
		case 'd':
			strcpy(path, p + 3);
			DeleteThread();
			ExitProcess();
		case 'v':
			cut_active = ESBYTE[p+2] - '0';
			strcpy(path, p + 4);
			PasteThread();
			ExitProcess();
	}

	if (param[strlen(#param)-1]=='/') param[strlen(#param)-1]='\0'; //no "/" at the end

	if (dir_exists(p)) {
		strcpy(path, p);
	} else {
		if (file_exists(p)) {
			ESBYTE[strrchr(p, '/')+p-1] = '\0';
			strcpy(path, p);
			SelectFileByName(p+strlen(path)+1);
		} else {
			notify(T_NOTIFY_APP_PARAM_WRONG);
		}
	}
}

void main()
{
	dword id;
	int old_cur_y;

#ifndef __COFF__
	load_dll(boxlib, #box_lib_init,0);
	load_dll(libini, #lib_init,1);
	load_dll(libimg, #libimg_init,1);
#endif

	handle_param();

	SystemDiscs.Get();
	OpenDir(ONLY_OPEN);
	llist_copy(#files_inactive, #files);
	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);
	loop() switch(@WaitEventTimeout(150))
	{
		case evMouse:
			if (Form.status_window&ROLLED_UP) break;

			if (active_popin) {
				if (popin_string[0]!=-1) edit_box_mouse stdcall(#popin_text);
				break;
			}

			mouse.get();

			ProceedMouseGestures();

			if (mouse.vert)
			{
				if (files.MouseScroll(mouse.vert)) List_ReDraw();
				break;
			}

			if (files.MouseOver(mouse.x, mouse.y))
			{
				//select file
				if (mouse.key&MOUSE_LEFT) && (mouse.up)
				{
					GetKeyModifier();
					old_cur_y = files.cur_y;
					if (files.ProcessMouse(mouse.x, mouse.y)) && (!key_modifier) {
						List_ReDraw();
						break;
					}
					if (key_modifier&KEY_LSHIFT) || (key_modifier&KEY_RSHIFT) {
						EventChooseFilesRange(old_cur_y, files.cur_y);
					} else if (key_modifier&KEY_LCTRL) || (key_modifier&KEY_RCTRL) {
						EventChooseFile(files.cur_y);
						DrawStatusBar();
						List_ReDraw();
					} else {
						if (mouse.y - files.y / files.item_h + files.first == files.cur_y) EventOpen(0);
					}
				}
				//file menu
				if (mouse.key&MOUSE_RIGHT) && (mouse.up)
				{
					if (files.ProcessMouse(mouse.x, mouse.y)) List_ReDraw();
					if (getElementSelectedFlag(files.cur_y) == false) unselectAll(); //on redraw selection would be flashed, see [L001]
					EventShowListMenu();
				}
			}

			if (mouse.x>=files.x+files.w) && (mouse.x<=files.x+files.w+16) && (mouse.y>files.y-17) && (mouse.y<files.y)
			{
				if (mouse.lkm) DrawRectangle3D(files.x+files.w+1,files.y-16,14,14,sc.dark,sc.light);
				WHILE (mouse.lkm) && (files.first>0)
				{
					pause(8);
					files.first--;
					List_ReDraw();
					mouse.get();
				}
				DrawRectangle3D(files.x+files.w+1,files.y-16,14,14,sc.light,sc.dark);
			}

			if (mouse.x>=files.x+files.w) && (mouse.x<=files.x+files.w+16) && (mouse.y>files.y+files.h-16) && (mouse.y<files.y+files.h)
			{
				if (mouse.lkm) DrawRectangle3D(files.x+files.w+1,files.y+files.h-15,14,14,sc.dark,sc.light);
				while (mouse.lkm) && (files.first<files.count-files.visible)
				{
					pause(8);
					files.first++;
					List_ReDraw();
					mouse.get();
				}
				DrawRectangle3D(files.x+files.w+1,files.y+files.h-15,14,14,sc.light,sc.dark);
			}

			//Scrooll
			if (mouse.x>=files.x+files.w) && (mouse.x<=files.x+files.w+18) && (mouse.y>files.y)
				&& (mouse.y<files.y+files.h-18) && (mouse.lkm) && (!scroll_used) {scroll_used=true; DrawScroll(scroll_used);}
			if (scroll_used) && (!mouse.key&MOUSE_LEFT) { scroll_used=false; DrawScroll(scroll_used); }

			if (scroll_used)
			{
				if (sc_slider_h/2+files.y>mouse.y) || (mouse.y<0) || (mouse.y>4000) mouse.y=sc_slider_h/2+files.y; //anee eo?ni? iaa ieiii
				id = files.first;
				files.first = -sc_slider_h / 2 + mouse.y -files.y * files.count;
				files.first /= files.h - 18;
				if (files.visible+files.first>files.count) files.first=files.count-files.visible;
				if (files.first<0) files.first=0;
				if (id!=files.first) List_ReDraw();
				break;
			}

			if (efm) && (mouse.y < files.y + files.h) && (mouse.down) {
				if (mouse.x<Form.cwidth/2) {
					SetActivePanel(0);
				} else {
					SetActivePanel(1);
				}
			}
			break;
//Button pressed-----------------------------------------------------------------------------
		case evButton:
			id = GetButtonID();

			if (CLOSE_BTN == id) {
				KillProcess(about_thread_id);
				SaveIniSettings();
				ExitProcess();
			}

			if (active_popin) {
				if (POPUP_BTN2==id) { EventClosePopinForm(); break; }
				if (POPUP_BTN1==id) { EventPopinClickOkay(); break; }

				if (POPIN_DISK==active_popin) {
					active_popin = NULL;
					EventDriveClick(id-100);
				}

				if (POPIN_BREADCR==active_popin) {
					EventClosePopinForm();
					ClickOnBreadCrumb(id-BREADCRUMB_ID);
				}

				break;
			}

			switch(id)
			{
				case KFM_DEV_DROPDOWN_1:
				case KFM_DEV_DROPDOWN_2:
						ShowPopinForm(POPIN_DISK);
						break;
				case BACK_BTN...PASTE_BTN:
						EventToolbarButtonClick(id);
						break;
				case BTN_PATH:
				case BTN_PATH+1:
						ShowPopinForm(POPIN_PATH);
						break;
				case BTN_BREADCRUMB:
				case BTN_BREADCRUMB+1:
						ShowPopinForm(POPIN_BREADCR);
						break;
				case 31...33:
						EventSort(id-30);
						break;
				case 51:
						EventShowBurgerMenu();
						break;
				case 52...60: //Actions
						FnProcess(id-50);
						break;
				case 100...120:
						EventDriveClick(id-100);
						break;
				case KFM_FUNC_ID...KFM_FUNC_ID+10:
						FnProcess(id-KFM_FUNC_ID);
						break;
			}
			break;

//Key pressed-----------------------------------------------------------------------------
		case evKey:
			GetKeys();

			if (Form.status_window&ROLLED_UP) break;

			if (active_popin)
			{
				if (key_scancode == SCAN_CODE_ESC) EventClosePopinForm();

				if (POPIN_DISK == active_popin) {
					if (key_scancode >= SCAN_CODE_1)
						&& (key_scancode <= SCAN_CODE_10) {
							EventDriveClick(key_scancode-2);
						}
				} else {
					if (key_scancode == SCAN_CODE_ENTER) EventPopinClickOkay();
					if (popin_string[0] != -1) {
						edit_box_key_c stdcall (#popin_text, key_editbox);
					}
				}
				break;
			}

			if (key_modifier&KEY_LSHIFT) || (key_modifier&KEY_RSHIFT) 
			{
				if (key_scancode == SCAN_CODE_ENTER) {
					EventOpenSelected();
					break;
				}
				old_cur_y = files.cur_y;
				files.ProcessKey(key_scancode);
				EventChooseFilesRange(old_cur_y, files.cur_y);
				break;
			}

			if (key_modifier&KEY_LCTRL) || (key_modifier&KEY_RCTRL)
			{
				switch(key_scancode)
				{
					case SCAN_CODE_F1...SCAN_CODE_F3:
							EventSort(key_scancode - 58);
							break;
					case SCAN_CODE_1...SCAN_CODE_2:
							if (efm) {
								active_panel = key_scancode - SCAN_CODE_1;
								ShowPopinForm(POPIN_DISK);
								break;
							}
					case SCAN_CODE_3...SCAN_CODE_10:
							key_scancode-=2;
							if (key_scancode >= SystemDiscs.list.count) break;
							if (!efm) {
								DrawRectangle(17,key_scancode*17+74,159,17, 0); //display click
								pause(7);
							}
							EventDriveClick(key_scancode);
							break;
					case SCAN_CODE_KEY_X:
							CopyFilesListToClipboard(CUT);
							break;
					case SCAN_CODE_KEY_C:
							CopyFilesListToClipboard(COPY);
							break;
					case SCAN_CODE_KEY_G:
							EventOpenConsoleHere();
							break;
					case SCAN_CODE_KEY_V:
							EventPaste(path);
							break;
					case SCAN_CODE_KEY_S: //set image as bg tile
							sprintf(#temp, "\\S__%s",#file_path);
							RunProgram("/sys/media/kiv", #temp);
							break;
					case SCAN_CODE_KEY_T: //set image as bg stretch
							sprintf(#temp, "\\T__%s",#file_path);
							RunProgram("/sys/media/kiv", #temp);
							break;
					case SCAN_CODE_KEY_N:
							EventOpenNewEolite();
							break;
					case SCAN_CODE_KEY_R:
							EventManualFolderRefresh();
							break;
					case SCAN_CODE_ENTER:
							if (!itdir) ShowOpenWithDialog();
							else EventOpen(1);
							break;
					case SCAN_CODE_KEY_A:
							EventChooseAllFiles(true);
							break;
					case SCAN_CODE_KEY_U: //unselect all files
							EventChooseAllFiles(false);
							break;
				}
				break;
			} 

			switch (calc(key_scancode))
			{
					case SCAN_CODE_BS:
							Dir_Up();
							break;
					case SCAN_CODE_ENTER:
							EventOpen(0);
							break;
					case SCAN_CODE_TAB:
							if (!efm) break;
							SetActivePanel(active_panel^1);
							break;
					case SCAN_CODE_MENU:
							mouse.x = files.x+15;
							mouse.y = files.cur_y - files.first * files.item_h + files.y + 5;
							EventShowListMenu();
							break;
					case SCAN_CODE_DEL:
							ShowPopinForm(POPIN_DELETE);
							break;
					case SCAN_CODE_SPACE:
							EventChooseFile(files.cur_y);
							DrawStatusBar();
							Line_ReDraw(col.selec, files.cur_y);
							break;
					case SCAN_CODE_INS:
							EventChooseFile(files.cur_y);
							files.KeyDown();
							DrawStatusBar();
							List_ReDraw();
							break;
					case SCAN_CODE_F1...SCAN_CODE_F10:
							FnProcess(key_scancode-58);
							break;
					case SCAN_CODE_DOWN:
					case SCAN_CODE_UP:
					case SCAN_CODE_HOME:
					case SCAN_CODE_END:
					case SCAN_CODE_PGUP:
					case SCAN_CODE_PGDN:
							if (files.ProcessKey(key_scancode)) List_ReDraw();
							break;
					default:
							EventSelectFileByKeyPress();
			}
			break;
		case evIPC:
		case evReDraw:
			draw_window();
			if (CheckActiveProcess(Form.ID)) && (GetMenuClick()) break;
			break;
		default:
			if (!Form.status_window&ROLLED_UP) 
			&& (ESBYTE[path+1]!='f') && (ESBYTE[path+1]!='c') {
				EventRefreshDisksAndFolders();
			}
	}

	if(cmd_free)
	{
		if(cmd_free==2) about_stak=free(about_stak);
		else if(cmd_free==3) properties_stak=free(properties_stak);
		else if(cmd_free==4) settings_stak=free(settings_stak);
		cmd_free = false;
	}
}

void draw_window()
{
	dword i=0;
	incn x;
	dword title;
	static int rand_n;
	if (!rand_n) rand_n = random(80);

	if (show_status_bar.checked) {
		#define STBAR_EOLITE_H 16
		#define STBAR_KFM_H 21
		if (efm) status_bar_h = STBAR_KFM_H;
		else status_bar_h = STBAR_EOLITE_H;
	} else {
		status_bar_h = 0;
	}
	if (efm) title = TITLE_KFM; else title = TITLE_EOLITE;
	DefineAndDrawWindow(Form.left+rand_n,Form.top+rand_n,Form.width,Form.height,0x73,NULL,title,0);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window&ROLLED_UP) return;
	if (Form.height < 356) { MoveSize(OLD,OLD,OLD,356); return; }
	GetProcessInfo(#Form, SelfInfo);
	SetAppColors();
	if (efm) {
		if (screen.w > 693) && (Form.width < 693) { MoveSize(OLD,OLD,693,OLD); return; }
		DrawBar(0, 4, Form.cwidth, SELECTY-5, sc.work);
		DrawBar(0, SELECTY+KFM2_DEVH+1, Form.cwidth, 3, sc.work);
		DrawBar(0, SELECTY-1, 1, KFM2_DEVH+2, sc.work);
		DrawBar(Form.cwidth-1, SELECTY-1, 1, KFM2_DEVH+2, sc.work);
		DrawBar(Form.cwidth/2-16, SELECTY-1, 15, KFM2_DEVH+2, sc.work);
		/*
		#define PAD 7
		#define GAP_S 26+5
		#define GAP_B 26+14
		x.set(Form.cwidth/2-DDW-203/2-GAP_S);
		while (i<200) {
			DrawTopPanelButton(i+BACK_BTN, x.inc(GAP_S), PAD, 30, false);
			DrawTopPanelButton(i+FWRD_BTN, x.inc(GAP_S), PAD, 31, false);
			DrawTopPanelButton(i+GOUP_BTN, x.inc(GAP_B), PAD, 01, false);
			DrawTopPanelButton(i+COPY_BTN, x.inc(GAP_B), PAD, 55, false);
			DrawTopPanelButton(i+CUT_BTN,  x.inc(GAP_S), PAD, 20, false);
			DrawTopPanelButton(i+PASTE_BTN,x.inc(GAP_S), PAD, 56, false);
			x.set(Form.cwidth/2-DDW-203/2-GAP_S+calc(Form.cwidth/2));
			i+=100;
		}
		//DrawTopPanelButton(51, Form.cwidth-GAP_S-PAD, PAD, -1, false); //burger menu
		*/
	} else {
		if (Form.width < 480) { MoveSize(OLD,OLD,480,OLD); return; }
		ESDWORD[#toolbar_pal] = sc.work;
		ESDWORD[#toolbar_pal+4] = MixColors(0, sc.work, 35);
		PutPaletteImage(#toolbar, 246, 34, 0, 0, 8, #toolbar_pal);
		for (i=0; i<3; i++) DefineHiddenButton(toolbar_buttons_x[i]+2,7,31-5,29-5,BACK_BTN+i);
		for (i=3; i<6; i++) DefineHiddenButton(toolbar_buttons_x[i],  5,31,  29,  BACK_BTN+i);
		DrawBar(127, 8, 1, 25, sc.line);
		DrawBar(246,0, Form.cwidth - 246, 34, sc.work);
		DrawDot(Form.cwidth-17,12);
		DrawDot(Form.cwidth-17,12+6);
		DrawDot(Form.cwidth-17,12+12);
		DefineHiddenButton(Form.cwidth-24,7,20,25,51+BT_NOFRAME); //dots
	}
	//main rectangles
	DrawRectangle(1,40,Form.cwidth-3,Form.cheight - 42-status_bar_h,sc.line);
	DrawBar(0,39,1,-show_status_bar.checked*status_bar_h + Form.cheight - 40, sc.work);
	EBX = Form.cwidth-1 * 65536 + 1;
	$int 64
	for (i=0; i<6; i++) DrawBar(0, 34+i, Form.cwidth, 1, MixColors(sc.dark, sc.work, i*10));
	for (i=0; i<6; i++) DrawBar(0, 5-i, Form.cwidth, 1, MixColors(sc.light, sc.work, i*10));
	llist_copy(#files_active, #files);
	DrawStatusBar();
	if (!getSelectedCount()) {
		OpenDir(ONLY_OPEN); //if there are no selected files -> refresh folder [L001]
	}
	DrawFilePanels();

	if (files.x!=files_inactive.x) {
		if (active_popin) ShowPopinForm(active_popin);
	}
}

void DrawButtonsAroundList()
{
	word sorting_arrow_x;
	dword sorting_arrow_t = "\x19";
	if (sort_desc) sorting_arrow_t = "\x18";
	DrawFlatButtonSmall(files.x - efm,           files.y-17,files.w-141+efm,16,31,T_FILE);
	DrawFlatButtonSmall(files.x + files.w - 141, files.y-17,73,16,32,T_TYPE);
	DrawFlatButtonSmall(files.x + files.w -  68, files.y-17,68,16,33,T_SIZE);
	DrawFlatButtonSmall(files.x + files.w,       files.y-17,16,16, 0,"\x18");
	DrawFlatButtonSmall(files.x + files.w,files.y+files.h-16,16,16,0,"\x19");
	if (sort_type==1) sorting_arrow_x = files.w - 141 / 2 + files.x + 18;
	if (sort_type==2) sorting_arrow_x = files.x + files.w - 90;
	if (sort_type==3) sorting_arrow_x = strlen(T_SIZE)*3-30+files.x+files.w;
	WriteText(sorting_arrow_x,files.y-12,0x80, sc.work_text, sorting_arrow_t);
	DrawBar(files.x+files.w,files.y,1,files.h,sc.line);
	if (efm) && (files.x<5) {
		DrawBar(files.x+files.w+16,files.y,1,files.h,EDX); //line between panel
	}
}

void DrawFuncButtonsInKfm()
{
	int i, x=0, len, min_w=0, padding;
	for (i=0; i<10; i++) min_w += strlen(kfm_func[i])+2*6 + 2;
	padding = Form.cwidth - min_w + 4 / 10;
	for (i=0; i<10; i++) {
		len = strlen(kfm_func[i])+2*6 + padding;
		if (i==9) len = Form.cwidth - x - 3;
		DrawBar(x, Form.cheight - 19, 1, 17, sc.work);
		DrawFuncButton(x+1, Form.cheight - 19, len, i+KFM_FUNC_ID+1, i+1, kfm_func[i]);
		x += len + 2;
	}
}

void DrawStatusBar()
{
	char status_bar_str[80];
	int go_up_folder_exists=0;
	dword topcolor;

	if (show_status_bar.checked) topcolor=sc.light; else topcolor=sc.work;
	DrawBar(0, Form.cheight - status_bar_h-1, Form.cwidth, 1, topcolor);

	if (efm) {
		DrawBar(0, Form.cheight - status_bar_h, Form.cwidth, 2, sc.work);
		DrawBar(0, Form.cheight - 2, Form.cwidth,  2, EDX);
		DrawBar(Form.cwidth-1, Form.cheight - 19, 1,  17, EDX);
		DrawFuncButtonsInKfm();
	} else if (show_status_bar.checked) {
		if (files.count>0) && (streq(items.get(0)*304+buf+72,"..")) go_up_folder_exists=1;
		DrawBar(0, Form.cheight - status_bar_h, Form.cwidth,  status_bar_h, sc.work);
		sprintf(#status_bar_str, T_STATUS_EVEMENTS, folder_count-go_up_folder_exists, files.count-folder_count);
		WriteText(6,Form.cheight - 13,0x80,sc.work_text,#status_bar_str);
		if (getSelectedCount()) {
			sprintf(#status_bar_str, T_STATUS_SELECTED, getSelectedCount());
			WriteText(Form.cwidth - calc(strlen(#status_bar_str)*6)-6,Form.cheight - 13,
				0x80,sc.work_text,#status_bar_str);
		}
	}
}

void DrawFilePanels()
{
	int files_y = files.y;
	int w2 = -Form.cwidth-1/2+Form.cwidth;
	int h2 = Form.cheight-files_y-2 - status_bar_h;
	if (!efm)
	{
		SystemDiscs.Draw();
		files.SetSizes(SIDEBAR_W, 57, Form.cwidth - SIDEBAR_W-18, Form.cheight - 59 - status_bar_h, files.item_h);
		DrawButtonsAroundList();
		List_ReDraw();
		DrawPathBar();
	}
	else
	{
		llist_copy(#files_active, #files);
		llist_copy(#files, #files_inactive);

		if (!active_panel) {
			files.SetSizes(Form.cwidth/2, files_y, w2-17, h2, files.item_h);
		} else {
			files.SetSizes(2, files_y, Form.cwidth/2-2-17, h2, files.item_h);
		}

		files_inactive.x = files.x;
		DrawButtonsAroundList();
		path = location[active_panel^1];
		active_panel ^= 1;
		OpenDir2(WITH_REDRAW);
		active_panel ^= 1;
		if (!getSelectedCount()) files_inactive.count = files.count;
		llist_copy(#files, #files_active);

		if (!active_panel) {
			files.SetSizes(2, files_y, Form.cwidth/2-2-17, h2, files.item_h);
		} else {
			files.SetSizes(Form.cwidth/2, files_y, w2 -17, h2, files.item_h);
		}

		DrawButtonsAroundList();
		path = location[active_panel];
		OpenDir2(WITH_REDRAW);
	}
}

void OpenDir2(char redraw){
	if (buf) free(buf);
	if (GetDir(#buf, #files.count, path, DIRS_NOROOT)) {
		Write_Error(EAX);
		history.add(path);
		EventHistoryGoBack();
		return;
	}
	SetCurDir(path);
	if (files.count>0) && (files.cur_y-files.first==-1) files.cur_y=0;
	files.visible = math.min(files.h / files.item_h, files.count);
	if (!strncmp(path, "/rd/1",5)) || (!strncmp(path, "/sys/",4))
		dir_at_fat16 = true; else dir_at_fat16 = false;
	Sorting();
	SystemDiscs.Draw();
	list_full_redraw = true;
	List_ReDraw();
	DrawPathBar();
}


void OpenDir(char redraw){
	int errornum;
	unselectAll();
	if (buf) free(buf);
	if (errornum = GetDir(#buf, #files.count, path, DIRS_NOROOT)) {
		history.add(path);
		//EventHistoryGoBack();
		Dir_Up();
		Write_Error(errornum);
		return;
	}
	if (files.count>0) && (files.cur_y-files.first==-1) files.cur_y=0;
	history.add(path);
	SystemDiscs.Draw();
	files.visible = math.min(files.h / files.item_h, files.count);
	if (!strncmp(path, "/rd/1",5)) || (!strncmp(path, "/sys/",4))
		dir_at_fat16 = true; else dir_at_fat16 = false;
	Sorting();
	list_full_redraw = true;
	SetCurDir(path);
	if (redraw!=ONLY_OPEN) {
		List_ReDraw();
		DrawStatusBar();
		DrawPathBar();
	}
}


void List_ReDraw()
{
	int all_lines_h;
	dword j;
	static int old_cur_y, old_first;
	dword separator_color;

	files.CheckDoesValuesOkey(); //prevent some shit
	if (files.count < files.visible) files.visible = files.count;

	if (list_full_redraw) || (old_first != files.first)
	{
		old_cur_y = files.cur_y;
		old_first = files.first;
		list_full_redraw = false;
		goto _ALL_LIST_REDRAW;
	}
	if (old_cur_y != files.cur_y)
	{
		if (old_cur_y-files.first<files.visible) Line_ReDraw(col.list_bg, old_cur_y-files.first);
		Line_ReDraw(col.selec, files.cur_y-files.first);
		old_cur_y = files.cur_y;
		return;
	}

	_ALL_LIST_REDRAW:

	for (j=0; j<files.visible; j++) {
		if (files.cur_y-files.first!=j) Line_ReDraw(col.list_bg, j);
		else Line_ReDraw(col.selec, files.cur_y-files.first);
	}
	//in the bottom
	all_lines_h = j * files.item_h;
	DrawBar(files.x,all_lines_h + files.y,files.w,files.h - all_lines_h, col.list_bg);
	if (colored_lines.checked) separator_color = col.list_bg; else separator_color = col.list_vert_line;
	DrawBar(files.x+files.w-141,all_lines_h + files.y,1,files.h - all_lines_h, separator_color);
	DrawBar(files.x+files.w-68,all_lines_h + files.y,1,files.h - all_lines_h, separator_color);
	DrawScroll(scroll_used);
}

void Line_ReDraw(dword bgcol, signed filenum){
	dword text_col=col.list_gb_text,
		  ext1, attr,
		  file_name_off,
		  file_size=0,
		  y=filenum*files.item_h+files.y,
		  icon_y = files.item_h-icon_size/2+y;
		  BDVK file;
		  char full_path[4096];
		  dword separator_color;
		  bool current_inactive = false;
		  char volume_label[64] = 0;
	char label_file_name[4096];
	if (filenum<0) return; //if hold lkm and scroll down by mouse wheel this may be the case

	if (bgcol==col.selec) && (files.x==files_inactive.x) {
		bgcol = col.list_bg;
		current_inactive = true;
	}

	DrawBar(files.x,y,4,files.item_h,bgcol);
	DrawBar(files.x+4,y,icon_size,icon_y-y,bgcol);
	if (files.item_h>icon_size) DrawBar(files.x+4,icon_y+icon_size,icon_size,y+files.item_h-icon_y-icon_size,bgcol);
	if (colored_lines.checked) {
		if (bgcol!=col.selec) && (filenum%2) bgcol=col.odd_line;
		separator_color = bgcol;
	} else {
		separator_color = col.list_vert_line;
	}
	DrawBar(files.x+icon_size+4,y,files.w-icon_size-4,files.item_h,bgcol);
	DrawBar(files.x+files.w-141,y,1,files.item_h, separator_color);
	DrawBar(files.x+files.w-68,y,1,files.item_h, separator_color);

	ESI = items.get(filenum+files.first)*304 + buf+32;
	attr = ESDWORD[ESI];
	file.sizelo   = ESI.BDVK.sizelo;
	file.sizehi   = ESI.BDVK.sizehi;
	file_name_off = #ESI.BDVK.name;
	sprintf(#full_path,"%s/%s",path,file_name_off);

	if (attr&ATR_FOLDER)
	{
		if (!strcmp(file_name_off,"..")) ext1="<up>"; else {
			ext1="<DIR>";
			WriteTextCenter(files.x+files.w-140, files.text_y+y+1, 72, col.list_gb_text, ext1);
		}
		if (chrnum(path, '/')==1) && (streq(path, "/kolibrios")==false)
			&& (streq(path, "/sys")==false) {
				file_size = GetDeviceSize(#full_path);
				if (ESBYTE[path+1]) strlcpy(#volume_label, GetVolumeLabel(#full_path), sizeof(volume_label));
			}
	}
	else
	{
		ext1 = strrchr(file_name_off,'.') + file_name_off;
		if (ext1==file_name_off) ext1 = NULL; //if no extension then show nothing
		file_size = ConvertSize64(file.sizelo, file.sizehi);
		if (ext1) && (strlen(ext1)<9) WriteTextCenter(files.x+files.w-140, files.text_y+y+1, 72, col.list_gb_text, ext1);
	}
	if (file_size) WriteText(7-strlen(file_size)*6+files.x+files.w-58,
			files.text_y+y+1, files.font_type, col.list_gb_text, file_size);

	if (attr&ATR_HIDDEN) || (attr&ATR_SYSTEM) text_col=col.list_text_hidden;
	if (bgcol==col.selec)
	{
		file_name_is_8_3(file_name_off);
		itdir = attr & ATR_FOLDER;
		strcpy(#file_name, file_name_off);
		if (streq(path,"/")) sprintf(#file_path,"%s%s",path,file_name_off);
			else sprintf(#file_path,"%s/%s",path,file_name_off);
		if (text_col==col.list_text_hidden) {
			text_col=MixColors(col.selec_text, col.list_text_hidden, 65);
		} else text_col=col.selec_text;
	}
	if (getElementSelectedFlag(filenum+files.first)) text_col=0xFF0000;
	if (kfont.size.pt==9) || (!kfont.font)
	{
		// Show default font 
		if (Form.width>=480)
		{
			FileShow.start_x = files.x + icon_size + 7;
			FileShow.font_color = text_col;
			FileShow.area_size_x = files.w - 164;
			FileShow.text_pointer = file_name_off;
			FileShow.start_y = files.text_y + y - 3;
			PathShow_prepare stdcall(#FileShow);
			PathShow_draw stdcall(#FileShow);
		}
	} else {
		// Show KFONT
		//that shit must be in a library
		strcpy(#label_file_name, file_name_off);
		if (volume_label) sprintf(#label_file_name, "%s [%s]", file_name_off, #volume_label);
		if (kfont.get_label_width(#label_file_name) + 141 + 26 > files.w)
		{
			while (kfont.get_label_width(#label_file_name) + 141 + 26 > files.w) {
				ESBYTE[#label_file_name+strlen(#label_file_name)-1] = NULL;
			}
			strcpy(#label_file_name+strlen(#label_file_name)-2, "...");
		}
		kfont.WriteIntoWindow(files.x + icon_size+7, files.item_h - kfont.height / 2 + y,
			bgcol, text_col, kfont.size.pt, #label_file_name);
	}
	DrawIconByExtension(#full_path, ext1, files.x+4, icon_y, bgcol);
	if (current_inactive) DrawWideRectangle(files.x+2, y, files.w-4, files.item_h, 2, col.selec);
}

inline Sorting()
{
	dword d=0, f=1;
	int j=0;
	dword file_off;

	items.drop();

	if (streq(path,"/")) //do not sort root folder
	{
		for(d=1;d<files.count;d++;) items.set(d, d);
		folder_count = d;
		return;
	}
	for (j=files.count-1, file_off=files.count-1*304+buf+32; j>=0; j--, file_off-=304;)  //files | folders
	{
		if (dir_at_fat16) && (file_name_is_8_3(file_off+40)) strttl(file_off+40);
		if (ESDWORD[file_off] & ATR_FOLDER) {
			items.set(d, j);
			d++;
		} else {
			items.set(files.count-f, j);
			f++;
		}
	}
	folder_count = d;
	//sorting: files first, then folders
	Sort_by_Name(0,d-1);
	if (sort_type==1) Sort_by_Name(d,files.count-1);
	else if (sort_type==2) Sort_by_Type(d,files.count-1);
	else if (sort_type==3) Sort_by_Size(d,files.count-1);
	//reversed sorting
	if (sort_desc) {
		for (j=0; j<f/2; j++) {
			items.swap(files.count-j-1, d+j);
		}
		//if (sort_type==1) for (j=0; j<d/2; j++) items[d-j]><items[j];
	}
	//make ".." first item in list
	if (d>0) && (strncmp(items.get(0)*304+buf+72,"..",2)!=0)
		for(d--; d>0; d--;) if (!strncmp(items.get(d)*304+buf+72,"..",2)) {items.swap(d,0); break;}
}


void SelectFileByName(dword that_file)
{
	int ind;
	files.KeyHome();
	OpenDir(ONLY_OPEN);
	if (dir_at_fat16) && (file_name_is_8_3(that_file)) strttl(that_file);
	for (ind=files.count-1; ind>=0; ind--;) { if (!strcmpi(items.get(ind)*304+buf+72,that_file)) break; }
	files.cur_y = ind - 1;
	files.KeyDown();
	DrawStatusBar();
	List_ReDraw();
}


void Dir_Up()
{
	int iii;
	char old_folder_name[4096];
	iii=strlen(path)-1;
	if (iii==0) return;
	iii = strrchr(path, '/');
	strcpy(#old_folder_name, path+iii);
	if (iii>1) ESBYTE[path+iii-1]=NULL; else ESBYTE[path+iii]=NULL;
	SelectFileByName(#old_folder_name);
	DrawPathBar();
}

void EventOpenSelected()
{
	int i;
	for (i=0; i<files.count; i++) if (getElementSelectedFlag(i)) {
		EDX = items.get(i)*304 + buf+32;
		if (ESDWORD[EDX]&ATR_FOLDER) continue; //is foder
		sprintf(#param,"%s/%s",path, EDX+40);
		RunProgram("/sys/@open", #param);
	}
}

void EventOpen(byte _new_window)
{
	if (getSelectedCount()) && (!itdir) notify(T_USE_SHIFT_ENTER);
	if (_new_window)
	{
		if (streq(#file_name,"..")) return;
		RunProgram(I_Path, #file_path);
		return;
	}
	if (!files.count) return;
	if (!itdir)
	{
		if (strrchr(#file_name, '.')==0) RunProgram(#file_path, ""); else RunProgram("/sys/@open", #file_path);
	}
	else
	{
		if (!strncmp(#file_name,"..",3)) { Dir_Up(); return; }
		strcpy(path, #file_path);
		files.first=files.cur_y=0;
		OpenDir(WITH_REDRAW);
	}
}

void EventHistoryGoBack()
{
	char cur_folder[4096];
	strcpy(#cur_folder, path);
	if (history.back()) {
		strcpy(path, history.current());
		SelectFileByName(#cur_folder+strrchr(#cur_folder,'/'));
		DrawPathBar();
	}
}

void EventHistoryGoForward()
{
	if (history.forward()) {
		strcpy(path, history.current());
		files.KeyHome();
		OpenDir(WITH_REDRAW);
		DrawPathBar();
	}
}


void ShowOpenWithDialog()
{
	byte open_param[4097];
	sprintf(#open_param,"~%s",#file_path);
	RunProgram("/sys/@open", #open_param);
}

bool EventCreateAndRename()
{
	sprintf(#temp,"%s/%s",path,popin_text.text);
	if (file_exists(#temp)) {
		notify(FS_ITEM_ALREADY_EXISTS);
		return false;
	}
	switch(active_popin)
	{
		case POPIN_NEW_FILE:
				if (CreateFile(0, 0, #temp)) goto __FAIL;
				break;
		case POPIN_NEW_FOLDER:
				if (CreateDir(#temp)) goto __FAIL;
				break;
		case POPIN_RENAME:
				if (RenameMove(popin_text.text, #file_path))
				{
					if (itdir) {
						goto __FAIL;
					} else {
						if (CopyFile(#file_path,#temp)) {
							goto __FAIL;
						} else {
							DeleteFile(#file_path);
						}
					}
				}
	}
	SelectFileByName(popin_text.text);
	return true;
	__FAIL:
	Write_Error(EAX);
	return false;
}

void EventPopinClickOkay()
{
	switch(active_popin) {
		case POPIN_PATH:
			strcpy(path, #popin_string);
			OpenDir(WITH_REDRAW);
			break;
		case POPIN_DELETE:
			CopyFilesListToClipboard(DELETE);
			EventChooseAllFiles(false);
			sprintf(#param, "-d %s", #file_path);
			RunProgram(#program_path, #param);
			break;
		case POPIN_RENAME:
		case POPIN_NEW_FILE:
		case POPIN_NEW_FOLDER:
			if (!EventCreateAndRename()) return;
	}
	EventClosePopinForm();
}

void EventClosePopinForm()
{
	active_popin = NULL;
	draw_window();
}

void ShowPopinForm(byte _popin_type)
{
	int popinx;
	popin_string[0] = -1;
	switch(_popin_type) {
		case POPIN_PATH:
				edit_box_set_text stdcall (#popin_text, path);
				DrawEolitePopup(T_GOPATH, T_CANCEL);
				break;
		case POPIN_NEW_FILE:
				edit_box_set_text stdcall (#popin_text, T_NEW_FILE);
				DrawEolitePopup(T_CREATE, T_CANCEL);
				break;
		case POPIN_NEW_FOLDER:
				edit_box_set_text stdcall (#popin_text, T_NEW_FOLDER);
				DrawEolitePopup(T_CREATE, T_CANCEL);
				break;
		case POPIN_RENAME:
				edit_box_set_text stdcall (#popin_text, #file_name);
				DrawEolitePopup(T_RENAME, T_CANCEL);
				break;
		case POPIN_DELETE:
				if (!files.count) return;
				if (!getSelectedCount()) && (!strncmp(#file_name,"..",2)) return;
				popinx = DrawEolitePopup(T_YES, T_NO);
				WriteTextCenter(popinx, 178, POPIN_W, sc.work_text, T_DELETE_FILE);
				if (getSelectedCount()) {
					sprintf(#param,"%s%d%s",DEL_MORE_FILES_1,getSelectedCount(),DEL_MORE_FILES_2);
				} else {
					if (strlen(#file_name)<28) {
						sprintf(#param,"%s ?",#file_name);
					} else {
						strncpy(#param, #file_name, POPIN_W-20/6-4);
						strcat(#param, "...?");
					}
				}
				WriteTextCenter(popinx, SIDEBAR_W, POPIN_W, sc.work_text, #param);
				break;
		case POPIN_DISK:
				DefineHiddenButton(0,0,5000,3000,POPUP_BTN2+BT_NOFRAME);
				if (active_panel==0) {
					SystemDiscs.DrawOptions(1);
				} else {
					SystemDiscs.DrawOptions(Form.cwidth/2-1);
				}
				break;
		case POPIN_BREADCR:
				DefineHiddenButton(0,0,5000,3000,POPUP_BTN2+BT_NOFRAME);
				DrawBreadCrumbs();
				break;
	}
	active_popin = _popin_type;
}

void EventShowAbout()
{
	if (!active_about) {
		about_stak = malloc(4096);
		about_thread_id = CreateThread(#about_dialog,about_stak+4092);
	} else {
		ActivateWindow(GetProcessSlot(about_thread_id));
	}
}

void FnProcess(byte N)
{
	switch(N)
	{
		case 1:
			EventShowProperties();
			break;
		case 2:
			if (files.count) ShowPopinForm(POPIN_RENAME);
			break;
		case 3:
			if (files.count) && (!itdir) RunProgram("/kolibrios/utils/quark", #file_path);
			break;
		case 4:
			if (files.count) && (!itdir) RunProgram("/sys/develop/cedit", #file_path);
			break;
		case 5:
			if (efm) {
				CopyFilesListToClipboard(COPY);
				EventPaste(location[active_panel^1]);
			} else {
				EventManualFolderRefresh();
			}
			break;
		case 6:
			if (efm) {
				CopyFilesListToClipboard(CUT);
				EventChooseAllFiles(false);
				EventPaste(location[active_panel^1]);
			}
			break;
		case 7:
			ShowPopinForm(POPIN_NEW_FOLDER);
			break;
		case 8:
			ShowPopinForm(POPIN_DELETE);
			break;
		case 9:
			ShowPopinForm(POPIN_NEW_FILE);
			break;
		case 10: //F10
			if (active_settings) {
				ActivateWindow(GetProcessSlot(settings_window));
			} else {
				settings_stak = malloc(4096);
				settings_window = CreateThread(#settings_dialog, settings_stak+4092);
			}
			break;
	}
}

void SetActivePanel(int _active)
{
	if (active_panel != _active) {
		active_panel = _active;
		llist_copy(#files_active, #files_inactive);
		llist_copy(#files_inactive, #files);
		llist_copy(#files, #files_active);
		path = location[active_panel];
		DrawFilePanels();
	}
}

void EventSelectFileByKeyPress()
{
	int i;
	for (i=files.cur_y+1; i<files.count; i++)
	{
		strcpy(#temp, items.get(i)*304+buf+72);
		if (temp[0]==key_ascii) || (temp[0]==key_ascii-32)
		{
			files.cur_y = i - 1;
			files.KeyDown();
			List_ReDraw();
			return;
		}
	}
}

dword GetDeviceSize(dword p)
{
	BDVK bdvk;
	if (ESBYTE[p+1] == '/') p++;
	if (ESBYTE[p+1] == 'c') && (ESBYTE[p+2] == 'd')
		&& (ESBYTE[p+4] == 0) return 0;
	if (GetFileInfo(p, #bdvk)) {
		return 0;
	} else {
		ConvertSize64(bdvk.sizelo, bdvk.sizehi);
	}
}

int GetRealFileCountInFolder(dword folder_path)
{
	int fcount;
	dword countbuf;

	GetDir(#countbuf, #fcount, folder_path, DIRS_NOROOT);
	if (countbuf) free(countbuf);

	return fcount;
}

void EventRefreshDisksAndFolders()
{
	if(efm) {
		if (GetRealFileCountInFolder(location[active_panel^1]) != files_inactive.count) {
			DrawFilePanels();
			return;
		}
	} else {
		if (GetRealFileCountInFolder("/")+KolibriosMounted() != SystemDiscs.dev_num) {
			SystemDiscs.Get();
			SystemDiscs.Draw();
		}
	}
	if(GetRealFileCountInFolder(path) != files.count) {
		OpenDir(WITH_REDRAW);
	}
}

void EventManualFolderRefresh()
{
	Tip(56, T_DEVICES, 55, "-");
	pause(10);
	DrawFilePanels();
}

void EventSort(dword id)
{
	char selected_filename[256];
	if (sort_type == id) sort_desc ^= 1;
	else sort_type = id;
	strcpy(#selected_filename, #file_name);
	DrawButtonsAroundList();
	OpenDir(WITH_REDRAW);
	SelectFileByName(#selected_filename);
}

void EventOpenNewEolite()
{
	RunProgram(I_Path, path);
}

void EventOpenConsoleHere()
{
	sprintf(#param, "pwd cd %s", path);
	RunProgram("/sys/shell", #param);
}

void ProceedMouseGestures()
{
	char stats;
	signed x_old, y_old, dif_x, dif_y, adif_x, adif_y;
	if (!mouse.mkm) && (stats>0) stats = 0;
	if (mouse.mkm) && (!stats)
	{
		x_old = mouse.x;
		y_old = mouse.y;
		stats = 1;
	}
	if (mouse.mkm) && (stats==1)
	{
		dif_x = mouse.x-x_old;
		dif_y = mouse.y-y_old;
		adif_x = fabs(dif_x);
		adif_y = fabs(dif_y);

		if (adif_x>adif_y) {
			if (dif_x > 150) {
				EventHistoryGoForward();
				stats = 0;
			}
			if (dif_x < -150) {
				EventHistoryGoBack();
				stats = 0;
			}
		} else {
			if (dif_y < -100) {
				Dir_Up();
				stats = 0;
			}
		}
	}
}

void EventPaste(dword _into_path) {
	char paste_line[4096+6];
	sprintf(#paste_line, "-v%i %s", cut_active, _into_path);
	RunProgram(#program_path, #paste_line);
	EventClosePopinForm();
}

void EventShowProperties()
char line_param[4096+5];
{
	if (!getSelectedCount()) {
		sprintf(#line_param, "-p %s", #file_path);
		RunProgram(#program_path, #line_param);
	} else {
		properties_stak = malloc(8096);
		CreateThread(#properties_dialog, properties_stak+8092);
	}
}

void EventChooseFile(int _id)
{
	if (getElementSelectedFlag(_id) == true) {
		setElementSelectedFlag(_id, false);
	} else {
		setElementSelectedFlag(_id, true);
	}
}

void EventChooseFilesRange(int _start, _end)
{
	if (_start > _end) _start >< _end;
	if (_end - _start > 1) list_full_redraw = true;
	while (_start < _end) {
		EventChooseFile(_start);
		_start++;
	}
	DrawStatusBar();
	List_ReDraw();
}

void EventChooseAllFiles(dword state)
{
	int i;
	for (i=0; i<files.count; i++) setElementSelectedFlag(i, state);
	List_ReDraw();
	DrawStatusBar();
}

void EventToolbarButtonClick(int _btid)
{
	switch(_btid) {
		case BACK_BTN: EventHistoryGoBack(); break;
		case FWRD_BTN: EventHistoryGoForward(); break;
		case GOUP_BTN: Dir_Up(); break;
		case COPY_BTN: CopyFilesListToClipboard(CUT); break;
		case CUT_BTN:  CopyFilesListToClipboard(COPY); break;
		case PASTE_BTN:EventPaste(path); break;
	}
}

void EventDriveClick(int __id)
{
	if (__id >= SystemDiscs.list.count) return;
	if (efm) {
		EventClosePopinForm();
		draw_window();
	}
	strcpy(path, SystemDiscs.list.get(__id));
	files.KeyHome();
	OpenDir(WITH_REDRAW);
}

stop:
