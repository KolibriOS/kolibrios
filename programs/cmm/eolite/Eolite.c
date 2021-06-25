//Leency, Veliant, Punk_Joker, PavelYakov & KolibriOS Team 2008-2021
//GNU GPL license.

// 70.5 - get volume info and label

#define ABOUT_TITLE "EOLITE 5 RC4"
#define TITLE_EOLITE "Eolite File Manager 5 RC4"
#define TITLE_KFM "Kolibri File Manager 2 RC4";

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
#include "../lib/patterns/toolbar_button.h"

#include "imgs/images.h"

//Button IDs
enum {
	PATH_BTN = 10,
	POPUP_BTN1 = 201,
	POPUP_BTN2 = 202,
	KFM_DEV_DROPDOWN_1 = 205,
	KFM_DEV_DROPDOWN_2 = 207,
	BREADCRUMB_ID = 300,

	BACK_BTN = 400,
	FWRD_BTN,
	GOUP_BTN,
	COPY_BTN,
	CUT_BTN,
	PASTE_BTN,
	KFM_FUNC_ID = 450
};

//NewElement options
enum {
	CREATE_FILE=1, 
	CREATE_FOLDER, 
	RENAME_ITEM
}; 

//OpenDir options
enum {
	WITH_REDRAW, 
	ONLY_OPEN
};

_history history;

struct Eolite_colors
{
	bool  def;
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

bool efm = false;

int toolbar_buttons_x[7]={9,46,85,134,167,203};

bool active_about = false;
bool active_settings = false;
bool _not_draw = false;
bool dir_at_fat16 = NULL;

bool disk_popin_active_on_panel=0;

dword about_thread_id;
dword settings_window;

byte del_active=0;
byte new_element_active=0;

llist files, files_active, files_inactive;

byte list_full_redraw;

dword buf;
collection_int items=0;
int selected_count;
int count_dir;

byte path[4096];
byte file_path[4096];
byte file_name[256];
byte new_element_name[256];
byte temp[4096];
bool itdir;

char active_path[4096], inactive_path[4096];

dword eolite_ini_path[4096];
_ini ini;

char scroll_used=false;

dword about_stak=0,properties_stak=0,settings_stak=0;

proc_info Form;
int sc_slider_h;
int action_buf;
int rand_n;

char sort_type=2;
bool sort_desc=false;
int active_panel=1;

libimg_image icons16_default;
libimg_image icons16_selected;

libimg_image icons32_default;
libimg_image icons32_selected;

int status_bar_h;

int icon_size = 18;

edit_box new_file_ed = {200,213,180,0xFFFFFF,0x94AECE,0xFFFFFF,0xFFFFFF,0x10000000,
	248,#new_element_name,0,ed_focus+ed_always_focus,6,0};
PathShow_data FileShow = {0, 56,215, 8, 100, 1, 0, 0x0, 0xFFFfff, #file_name, #temp, 0};
byte cmd_free=0;
#include "include\translations.h"

#include "include\gui.h"
#include "include\settings.h"
#include "include\progress_dialog.h"
#include "include\copy_and_delete.h"
#include "include\sorting.h"
#include "include\icons.h"
#include "include\left_panel.h"
#include "include\menu.h"
#include "include\about.h"
#include "include\properties.h"
#include "include\breadcrumbs.h"

void load_libraries()
{
	load_dll(boxlib, #box_lib_init,0);
	load_dll(libini, #lib_init,1);
	load_dll(libimg, #libimg_init,1);
}

void handle_param()
{
	//-p <path> : just show file/folder properties dialog
	//-d <path> : delete file/folder
	//-v : paste files/folder from clipboard
	dword p = #param;
	if (param[0]=='\\') && (param[1]=='E') && (param[2]=='F') && (param[3]=='M') {
		efm = true;
		p += 4;
		if (param[4]==' ') p++;
	}

	LoadIniSettings();

	if (ESBYTE[p]=='\0') return;

	if (ESBYTE[p]=='-') switch (ESBYTE[p+1]) 
	{
		case 'p':
			strcpy(#file_path, p + 3);
			itdir = dir_exists(#file_path);
			strcpy(#file_name, p + strrchr(p, '/'));
			ESBYTE[strrchr(p, '/')+p-1] = '\0';
			strcpy(#path, p + 3);
			properties_dialog();
			ExitProcess();
		case 'd':
			strcpy(#path, p + 3);
			DeleteThread();
			ExitProcess();
		case 'v':
			cut_active = ESBYTE[p+2] - '0';
			strcpy(#path, p + 4);
			PasteThread();
			ExitProcess();
	}

	ESBYTE[0] = NULL;

	if (param[strlen(#param)-1]=='/') ESBYTE[strlen(#param)-1]=NULL; //no "/" at the end

	if (dir_exists(p)) {
		strcpy(#path, p);
	} else {
		if (file_exists(p)) {
			ESBYTE[strrchr(p, '/')+p-1] = '\0';
			strcpy(#path, p);
			SelectFileByName(p+strlen(#path)+1);
		} else {
			notify(T_NOTIFY_APP_PARAM_WRONG);
		}
	}	
}

void main() 
{
	dword id;
	int old_cur_y;

	load_libraries();
	SetAppColors();

	ESBYTE[0] = NULL;

	handle_param();
	rand_n = random(80);

	ESBYTE[0] = NULL;

	SystemDiscs.Get();
	Open_Dir(#path,ONLY_OPEN);
	strcpy(#inactive_path, #path);
	llist_copy(#files_inactive, #files);
	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);
	loop() switch(@WaitEventTimeout(80))
	{
		case evMouse:
			if (del_active) || (disk_popin_active_on_panel) || (Form.status_window>2) break;
			if (new_element_active) 
			{
				edit_box_mouse stdcall(#new_file_ed);
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
					if (getElementSelectedFlag(files.cur_y) == false) selected_count = 0; //on redraw selection would be flashed, see [L001] 
					EventShowListMenu();
				}
			}

			if (mouse.x>=files.x+files.w) && (mouse.x<=files.x+files.w+16) && (mouse.y>files.y-17) && (mouse.y<files.y)
			{
				if (mouse.lkm) DrawRectangle3D(files.x+files.w+1,files.y-16,14,14,sc.work_dark,sc.work_light);
				WHILE (mouse.lkm) && (files.first>0)
				{
					pause(8);
					files.first--;
					List_ReDraw();
					mouse.get();
				}
				DrawRectangle3D(files.x+files.w+1,files.y-16,14,14,sc.work_light,sc.work_dark);
			}

			if (mouse.x>=files.x+files.w) && (mouse.x<=files.x+files.w+16) && (mouse.y>files.y+files.h-16) && (mouse.y<files.y+files.h)
			{
				if (mouse.lkm) DrawRectangle3D(files.x+files.w+1,files.y+files.h-15,14,14,sc.work_dark,sc.work_light);
				while (mouse.lkm) && (files.first<files.count-files.visible)
				{
					pause(8);
					files.first++;
					List_ReDraw();
					mouse.get();
				}
				DrawRectangle3D(files.x+files.w+1,files.y+files.h-15,14,14,sc.work_light,sc.work_dark);
			}

			//Scrooll
			if (mouse.x>=files.x+files.w) && (mouse.x<=files.x+files.w+18) && (mouse.y>files.y) 
				&& (mouse.y<files.y+files.h-18) && (mouse.lkm) && (!scroll_used) {scroll_used=true; Scroll();}
			if (scroll_used) && (!mouse.key&MOUSE_LEFT) { scroll_used=false; Scroll(); }
			
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
					if (active_panel!=1) ChangeActivePanel();
				} else {
					if (active_panel!=2) ChangeActivePanel();
				}
			}
			break;  
//Button pressed-----------------------------------------------------------------------------
		case evButton:
			id = GetButtonID();

			if (id==CLOSE_BTN) {
				KillProcess(about_thread_id);
				SaveIniSettings();
				ExitProcess();
			}

			if (new_element_active) || (del_active) || (disk_popin_active_on_panel) {
				if (POPUP_BTN1==id) && (del_active) EventDelete();
				if (POPUP_BTN1==id) && (new_element_active) NewElement();
				if (POPUP_BTN2==id) EventClosePopinForm();
				if (disk_popin_active_on_panel) {
					if (id>=100) && (id<=120) EventDriveClick(id);
					else EventClosePopinForm();
				}
				break;					
			}

			switch(id) 
			{
				case PATH_BTN:
						notify(COPY_PATH_STR);
						Clipboard__CopyText(#path);
						break;
				case KFM_DEV_DROPDOWN_1:
				case KFM_DEV_DROPDOWN_1+1:
						EventOpenDiskPopin(1);
						break;
				case KFM_DEV_DROPDOWN_2:
				case KFM_DEV_DROPDOWN_2+1:
						EventOpenDiskPopin(2);
						break;
				case BACK_BTN...PASTE_BTN:
						if (active_panel==2) ChangeActivePanel();
						EventToolbarButtonClick(id);
						break;
				case BACK_BTN+100...PASTE_BTN+100:
						if (active_panel==1) ChangeActivePanel();
						EventToolbarButtonClick(id-100);
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
				case 61: // Set path as default
						SetDefaultPath(#path);
						break;
				case 100...120:
					EventDriveClick(id);
					break;
				case BREADCRUMB_ID...360:
					ClickOnBreadCrumb(id-BREADCRUMB_ID);
					break;
				case KFM_FUNC_ID...KFM_FUNC_ID+10:
					FnProcess(id-KFM_FUNC_ID);
					break;
			}
			break;
			
//Key pressed-----------------------------------------------------------------------------
		case evKey:
			GetKeys();

			if (Form.status_window>2) break;

			if (new_element_active) || (del_active) || (disk_popin_active_on_panel)
			{
				if (key_scancode == SCAN_CODE_ESC) EventClosePopinForm();

				if (del_active) {
					if (key_scancode == SCAN_CODE_ENTER) EventDelete();
				}
				if (new_element_active) {
					if (key_scancode == SCAN_CODE_ENTER) NewElement();
					EAX = key_editbox;
					edit_box_key stdcall (#new_file_ed);
				}
				break;
			}

			if (key_modifier&KEY_LALT) || (key_modifier&KEY_RALT) {
				if (key_scancode == SCAN_CODE_F1) EventOpenDiskPopin(1);
				if (key_scancode == SCAN_CODE_F2) EventOpenDiskPopin(2);
				break;
			}
			if (key_modifier&KEY_LSHIFT) || (key_modifier&KEY_RSHIFT) {

				if (key_scancode == SCAN_CODE_ENTER) {
					EventOpenSelected(); 
					break;
				}

				old_cur_y = files.cur_y;
				files.ProcessKey(key_scancode);
				EventChooseFilesRange(old_cur_y, files.cur_y);
				break;
			}

			if (files.ProcessKey(key_scancode))
			{
				List_ReDraw();
				break;
			}

			if (key_modifier&KEY_LCTRL) || (key_modifier&KEY_RCTRL)
			{
				switch(key_scancode)
				{
					case SCAN_CODE_F1...SCAN_CODE_F3:
							EventSort(key_scancode - 58);
							break;
					case SCAN_CODE_1...SCAN_CODE_10:
							key_scancode-=2;
							if (key_scancode >= SystemDiscs.list.count) break;
							if (!efm) {
								DrawRectangle(17,key_scancode*16+74,159,16, 0); //display click
								pause(7);										
							}
							SystemDiscs.Click(key_scancode);
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
							EventPaste(#path);
							break;
					case SCAN_CODE_KEY_D: //set image as bg
							strlcpy(#temp, "\\S__",4);
							strcat(#temp, #file_path);
							RunProgram("/sys/media/kiv", #temp);
							break;
					case SCAN_CODE_KEY_N:
							EventOpenNewEolite();
							break; 
					case SCAN_CODE_KEY_R:
							EventRefreshDisksAndFolders();
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

			switch (key_scancode)
			{
					case SCAN_CODE_BS:
							Dir_Up();
							break; 
					case SCAN_CODE_ENTER:
							EventOpen(0);
							break; 
					case SCAN_CODE_TAB:
							if (!efm) break;
							ChangeActivePanel();
							break;
					case SCAN_CODE_MENU:
							mouse.x = files.x+15;
							mouse.y = files.cur_y - files.first * files.item_h + files.y + 5;
							EventShowListMenu();
							break;
					case SCAN_CODE_DEL:
							Del_Form();
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
					default:
							EventSelectFileByKeyPress();
			}                         
			break;
		case evIPC:
		case evReDraw:
			draw_window();
			if (CheckActiveProcess(Form.ID)) && (GetMenuClick()) break;
			if (action_buf==OPERATION_END) {
				EventRefresh();
				action_buf=0;
			}
			break;
		default:
			if (Form.status_window<=2) EventRefreshDisksAndFolders();
			//sprintf(#param, "/tmp0/1/%i", random(99999)); //for testing purpose
			//CreateFile(0, 0, #param);
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
	if (show_status_bar.checked) {
		#define STBAR_EOLITE_H 16;
		#define STBAR_KFM_H 21+16;
		if (efm) status_bar_h = STBAR_KFM_H;
		else status_bar_h = STBAR_EOLITE_H;
	} else {
		status_bar_h = 0;
	}
	if (efm) title = TITLE_KFM; else title = TITLE_EOLITE;
	DefineAndDrawWindow(Form.left+rand_n,Form.top+rand_n,Form.width,Form.height,0x73,NULL,title,0);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return;
	if (Form.height < 356) { MoveSize(OLD,OLD,OLD,356); return; }
	GetProcessInfo(#Form, SelfInfo);
	SetAppColors();
	if (efm) {
		if (screen.width > 693) && (Form.width < 693) { MoveSize(OLD,OLD,693,OLD); return; }
		DrawBar(0, 0, Form.cwidth, 34, sc.work);
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
	} else {
		if (Form.width < 480) { MoveSize(OLD,OLD,480,OLD); return; }
		ESDWORD[#toolbar_pal] = sc.work;
		ESDWORD[#toolbar_pal+4] = MixColors(0, sc.work, 35);
		PutPaletteImage(#toolbar, 246, 34, 0, 0, 8, #toolbar_pal);		
		for (i=0; i<3; i++) DefineHiddenButton(toolbar_buttons_x[i]+2,7,31-5,29-5,BACK_BTN+i);
		for (i=3; i<6; i++) DefineHiddenButton(toolbar_buttons_x[i],  5,31,  29,  BACK_BTN+i);
		DrawBar(127, 8, 1, 25, sc.work_graph);
		DrawBar(246,0, Form.cwidth - 246, 34, sc.work);
		DrawDot(Form.cwidth-17,12);
		DrawDot(Form.cwidth-17,12+6);
		DrawDot(Form.cwidth-17,12+12);
		DefineHiddenButton(Form.cwidth-24,7,20,25,51+BT_NOFRAME); //dots
	}
	//main rectangles
	DrawRectangle(1,40,Form.cwidth-3,Form.cheight - 42-status_bar_h,sc.work_graph);
	DrawRectangle(0,39,Form.cwidth-1,-show_status_bar.checked*status_bar_h + Form.cheight - 40,col.work_gradient[4]); //bg
	for (i=0; i<6; i++) DrawBar(0, 34+i, Form.cwidth, 1, MixColors(sc.work_dark, sc.work, i*10));
	for (i=0; i<6; i++) DrawBar(0, 5-i, Form.cwidth, 1, MixColors(sc.work_light, sc.work, i*10));
	llist_copy(#files_active, #files);
	strcpy(#active_path, #path);
	DrawStatusBar();
	if (!selected_count) Open_Dir(#path,ONLY_OPEN); //if there are no selected files -> refresh folder [L001] 
	DrawFilePanels();
	disk_popin_active_on_panel = 0;
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
	DrawBar(files.x+files.w,files.y,1,files.h,sc.work_graph);
	if (efm) && (files.x<5) DrawBar(files.x+files.w+16,files.y,1,files.h,sc.work_graph);	
}

void DrawFuncButtonsInKfm()
{
	int i, x=0, len, min_w=0, padding;
	for (i=0; i<10; i++) min_w += strlen(kfm_func[i])+2*6 + 2;
	padding = Form.cwidth - min_w + 4 / 10;
	for (i=0; i<10; i++) {
		len = strlen(kfm_func[i])+2*6 + padding;
		if (i==9) len = Form.cwidth - x - 3;
		DrawFuncButton(x+1, Form.cheight - 19, len, i+KFM_FUNC_ID+1, i+1, kfm_func[i]);
		x += len + 2;
	}
}

void DrawStatusBar()
{
	char status_bar_str[80];
	int go_up_folder_exists=0;

	if (efm) { 
		DrawBar(0, Form.cheight - status_bar_h+15, Form.cwidth,  status_bar_h-15, sc.work);
		DrawFuncButtonsInKfm();
		DrawPathBar();
		return;
	}

	if (!show_status_bar.checked) return;
	if (files.count>0) && (streq(items.get(0)*304+buf+72,"..")) go_up_folder_exists=1;
	DrawBar(0, Form.cheight - status_bar_h, Form.cwidth,  status_bar_h, sc.work);
	sprintf(#status_bar_str, T_STATUS_EVEMENTS, count_dir-go_up_folder_exists, files.count-count_dir);
	WriteText(6,Form.cheight - 13,0x80,sc.work_text,#status_bar_str);
	if (selected_count) {
		sprintf(#status_bar_str, T_STATUS_SELECTED, selected_count);
		WriteText(Form.cwidth - calc(strlen(#status_bar_str)*6)-6,Form.cheight - 13,
			0x80,sc.work_text,#status_bar_str);
	}
}

void DrawFilePanels()
{
	int files_y;
	int w2 = -Form.cwidth-1/2+Form.cwidth;
	int h2;
	if (!efm)
	{
		DrawDeviceAndActionsLeftPanel();
		files.SetSizes(192, 57, Form.cwidth - 210, Form.cheight - 59 - status_bar_h, files.item_h);
		DrawButtonsAroundList();
		List_ReDraw();
	}
	else
	{
		llist_copy(#files, #files_inactive);
		strcpy(#path, #inactive_path);
		files_y = files.y;
		h2 = Form.cheight-files_y-2 - status_bar_h;
		col.selec = col.selec_inactive;  //this is a bad code: need to use some var to set inactive panel for DrawButtonsAroundList();

		if (active_panel==1)
		{
			files.SetSizes(Form.cwidth/2, files_y, w2-17, h2, files.item_h);
			DrawButtonsAroundList();
			Open_Dir(#path,WITH_REDRAW);
			files_inactive.count = files.count;
			llist_copy(#files, #files_active);
			strcpy(#path, #active_path);
			col.selec = col.selec_active;
			files.SetSizes(2, files_y, Form.cwidth/2-2-17, h2, files.item_h);
			DrawButtonsAroundList();
			Open_Dir(#path,WITH_REDRAW);
		}
		if (active_panel==2)
		{
			files.SetSizes(2, files_y, Form.cwidth/2-2-17, h2, files.item_h);
			DrawButtonsAroundList();
			Open_Dir(#path,WITH_REDRAW);
			files_inactive.count = files.count;
			llist_copy(#files, #files_active);
			strcpy(#path, #active_path);
			col.selec = col.selec_active;
			files.SetSizes(Form.cwidth/2, files_y, w2 -17, h2, files.item_h);
			DrawButtonsAroundList();
			Open_Dir(#path,WITH_REDRAW);
		}
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
	Scroll();

	if (del_active) Del_Form();
	if (new_element_active) && (col.selec != 0xCCCccc) NewElement_Form(new_element_active, #new_element_name);
}

bool file_name_is_8_3(dword name)
{
	int name_len = strlen(name);
	int dot_pos = strrchr(name, '.');
	if (name_len<=12) 
	{
		if (dot_pos) {
			if (name_len - dot_pos > 3) return false;
		}
		else {
			if (name_len>8) return false; 
		}
		return true;
	}
	return false;
}

void Line_ReDraw(dword bgcol, filenum){
	dword text_col=col.list_gb_text,
		  ext1, attr,
		  file_offet,
		  file_name_off,
		  file_size=0,
		  y=filenum*files.item_h+files.y,
		  icon_y = files.item_h-icon_size/2+y;
		  BDVK file;
		  char full_path[4096];
		  dword separator_color;
	char label_file_name[4096];
	if (filenum==-1) return;

	DrawBar(files.x,y,4,files.item_h,bgcol);
	DrawBar(files.x+4,y,icon_size,icon_y-y,bgcol);
	if (files.item_h>icon_size) DrawBar(files.x+4,icon_y+icon_size-1,icon_size,y+files.item_h-icon_y-icon_size+1,bgcol);
	if (colored_lines.checked) {
		if (bgcol!=col.selec) && (filenum%2) bgcol=col.odd_line;
		separator_color = bgcol;
	} else {
		separator_color = col.list_vert_line;
	}
	DrawBar(files.x+icon_size+4,y,files.w-icon_size-4,files.item_h,bgcol);
	DrawBar(files.x+files.w-141,y,1,files.item_h, separator_color);
	DrawBar(files.x+files.w-68,y,1,files.item_h, separator_color);

	file_offet = items.get(filenum+files.first)*304 + buf+32;
	attr = ESDWORD[file_offet];
	file.selected = ESBYTE[file_offet+7];
	file.sizelo   = ESDWORD[file_offet+32];
	file.sizehi   = ESDWORD[file_offet+36];
	file_name_off = file_offet+40;

	if (! TestBit(attr, 4) ) //file or folder?
	{	
		ext1 = strrchr(file_name_off,'.') + file_name_off;
		if (ext1==file_name_off) ext1 = NULL; //if no extension then show nothing
		file_size = ConvertSize64(file.sizelo, file.sizehi);
		if (ext1) && (strlen(ext1)<9) WriteTextCenter(files.x+files.w-140, files.text_y+y+1, 72, col.list_gb_text, ext1);
	}
	else
	{
		if (!strcmp(file_name_off,"..")) ext1="<up>"; else {
			ext1="<DIR>";
			WriteTextCenter(files.x+files.w-140, files.text_y+y+1, 72, col.list_gb_text, ext1);
		}
		if (chrnum(#path, '/')==1) && (streq(#path, "/kolibrios")==false) file_size = GetDeviceSizeLabel(#full_path);
	}
	if (file_size) WriteText(7-strlen(file_size)*6+files.x+files.w-58, 
			files.text_y+y+1, files.font_type, col.list_gb_text, file_size);

	if (TestBit(attr, 1)) || (TestBit(attr, 2)) text_col=col.list_text_hidden; //system or hiden?
	if (bgcol==col.selec)
	{
		file_name_is_8_3(file_name_off);
		itdir = TestBit(attr, 4);
		strcpy(#file_name, file_name_off);
		if (!strcmp(#path,"/")) sprintf(#file_path,"%s%s",#path,file_name_off);
			else sprintf(#file_path,"%s/%s",#path,file_name_off);
		if (text_col==col.list_text_hidden) {
			text_col=MixColors(col.selec_text, col.list_text_hidden, 65); 
		} else text_col=col.selec_text;
	}
	if (file.selected) text_col=0xFF0000;
	if (kfont.size.pt==9) || (!kfont.font)
	{
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
	}
	else
	{
		strcpy(#label_file_name, file_name_off);
		if (kfont.getsize(kfont.size.pt, #label_file_name) + 141 + 26 > files.w)
		{
			while (kfont.getsize(kfont.size.pt, #label_file_name) + 141 + 26 > files.w) {
				ESBYTE[#label_file_name+strlen(#label_file_name)-1] = NULL;
			}
			strcpy(#label_file_name+strlen(#label_file_name)-2, "...");			
		}
		kfont.WriteIntoWindow(files.x + icon_size+7, files.item_h - kfont.height / 2 + y, 
			bgcol, text_col, kfont.size.pt, #label_file_name);
	}
	if (bgcol == col.selec_inactive) DrawWideRectangle(files.x+2, y, files.w-4, files.item_h, 2, col.selec_active);

	sprintf(#full_path,"%s/%s",#path,file_name_off);
	DrawIconByExtension(#full_path, ext1, files.x+4, icon_y, bgcol);
}


void Open_Dir(dword dir_path, redraw){
	int errornum;

	selected_count = 0;
	if (buf) free(buf);
	errornum = GetDir(#buf, #files.count, dir_path, DIRS_NOROOT);
	if (errornum)
	{
		history.add(#path);
		EventHistoryGoBack();
		Write_Error(errornum);
		return;
	}
	if (files.count>0) && (files.cur_y-files.first==-1) files.cur_y=0;

	if (files.count!=-1)
	{
		if(!_not_draw) DrawPathBar();
		history.add(#path);
		SystemDiscs.Draw();
		files.visible = files.h / files.item_h;
		if (files.count < files.visible) files.visible = files.count;
		if (!strncmp(dir_path, "/rd/1/",5)) || (!strncmp(dir_path, "/sys/",4)) 
			dir_at_fat16 = true; else dir_at_fat16 = false; 
		Sorting();
		list_full_redraw = true;
		if (redraw!=ONLY_OPEN)&&(!_not_draw) {DrawStatusBar(); List_ReDraw();}
		SetCurDir(dir_path);
	}
	if (files.count==-1) && (redraw!=ONLY_OPEN) 
	{
		files.KeyHome();
		if(!_not_draw) { list_full_redraw=true; DrawStatusBar(); List_ReDraw(); }
	}
}

inline Sorting()
{
	dword d=0, f=1;
	int j=0;
	dword file_off;

	items.drop();

	if (!strcmp(#path,"/")) //do not sort root folder
	{
		for(d=1;d<files.count;d++;) items.set(d, d);
		count_dir = d;
		return;
	}
	for (j=files.count-1, file_off=files.count-1*304+buf+32; j>=0; j--, file_off-=304;)  //files | folders
	{
		if (dir_at_fat16) && (file_name_is_8_3(file_off+40)) strttl(file_off+40);
		if (TestBit(ESDWORD[file_off],4)) //directory?
		{
			items.set(d, j);
			d++;
		}
		else
		{
			items.set(files.count-f, j);
			f++;
		}
	}
	count_dir = d;
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


void Del_Form()
{
	byte f_count[128];
	int dform_x = files.w - 220 / 2 + files.x;
	if (!selected_count) && (!strncmp(#file_name,"..",2)) return;
	else
	{
		if (!files.count) return;
		DrawEolitePopup(T_YES, T_NO);
		WriteText(-strlen(T_DELETE_FILE)*3+110+dform_x,175,0x80,sc.work_text,T_DELETE_FILE);
		if (selected_count)
		{
			sprintf(#f_count,"%s%d%s",DEL_MORE_FILES_1,selected_count,DEL_MORE_FILES_2);
			WriteText(-strlen(#f_count)*3+110+dform_x,190,0x80,sc.work_text,#f_count);
		}
		else
		{
			if (strlen(#file_name)<28) 
			{
				WriteText(strlen(#file_name)*3+110+dform_x+2,190,0x80,sc.work_text,"?");
				WriteText(-strlen(#file_name)*3+110+dform_x,190,0x80,sc.work_text,#file_name);
			}
			else
			{
				WriteText(164+dform_x,190,0x80,0,"...?");
				ESI = 24;
				WriteText(dform_x+20,190,0,0,#file_name);
			}
		}		
		del_active=1;
	}
}

void SelectFileByName(dword that_file)
{
	int ind;
	files.KeyHome();
	Open_Dir(#path,ONLY_OPEN);
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
	iii=strlen(#path)-1;
	if (iii==0) return;
	iii = strrchr(#path, '/');
	strcpy(#old_folder_name, #path+iii);
	if (iii>1) path[iii-1]=NULL; else path[iii]=NULL;
	SelectFileByName(#old_folder_name);
}

void EventOpenSelected()
{
	int i;
	for (i=0; i<files.count; i++) if (getElementSelectedFlag(i)) { 
		EDX = items.get(i)*304 + buf+32;
		if (TestBit(ESDWORD[EDX], 4)) continue; //is foder
		sprintf(#param,"%s/%s",#path, EDX+40);
		RunProgram("/sys/@open", #param);
	}
}

void EventOpen(byte _new_window)
{
	if (selected_count) && (!itdir) notify(T_USE_SHIFT_ENTER);
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
		strcpy(#path, #file_path);
		files.first=files.cur_y=0;
		Open_Dir(#path,WITH_REDRAW);
	}
}

inline fastcall void EventHistoryGoBack()
{
	char cur_folder[4096];
	strcpy(#cur_folder, #path);
	if (history.back()) {
		strcpy(#path, history.current());
		SelectFileByName(#cur_folder+strrchr(#cur_folder,'/'));
	}
}

void ShowOpenWithDialog()
{
	byte open_param[4097];
	sprintf(#open_param,"~%s",#file_path);
	RunProgram("/sys/@open", #open_param);
}

void NewElement()
{
	BDVK element_info;
	byte copy_result, info_result;

	sprintf(#temp,"%s/%s",#path,new_file_ed.text);
	info_result = GetFileInfo(#temp, #element_info);
	switch(new_element_active)
	{
		case CREATE_FILE:
			if (info_result!=5) {
				notify(FS_ITEM_ALREADY_EXISTS);
			} else {
				CreateFile(0, 0, #temp);
				if (EAX)
				{
					if (EAX==5) notify(NOT_CREATE_FILE);
					else Write_Error(EAX);
				}
			}
			break;
		case CREATE_FOLDER:
			if (info_result!=5) {
				notify(FS_ITEM_ALREADY_EXISTS);
			} else {
				CreateDir(#temp);
				if (EAX)
				{
					if (EAX==5) notify(NOT_CREATE_FOLDER);
					else Write_Error(EAX);
				}
			}
			break;
		case RENAME_ITEM:
			if (info_result!=5) {
				notify(FS_ITEM_ALREADY_EXISTS);
			} else {
				if (RenameMove(new_file_ed.text, #file_path))
				{
					if (itdir) {
						notify("'Error renaming folder' -E");
						return;
					} else {
						if (copy_result = CopyFile(#file_path,#temp)) {
							Write_Error(copy_result);
						} else {
							DeleteFile(#file_path);
							SelectFileByName(new_file_ed.text);
						}
					}
				}
			}
	}
	Open_Dir(#path,WITH_REDRAW);
	SelectFileByName(new_file_ed.text);
	EventClosePopinForm();
}

void NewElement_Form(byte crt, dword strng)
{
	int dform_x=files.w-220/2+files.x;
	if (!new_element_active)
	{
		new_element_active = crt;
		edit_box_set_text stdcall (#new_file_ed, strng);
	}
	if (new_element_active==3) DrawEolitePopup(T_RENAME, T_CANCEL);
	else DrawEolitePopup(T_CREATE, T_CANCEL);
	new_file_ed.left = dform_x+10;
	DrawEditBox(#new_file_ed);
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
			if (files.count) NewElement_Form(RENAME_ITEM, #file_name);
			break;
		case 3:
			if (files.count) && (!itdir) RunProgram("/sys/quark", #file_path);
			break;
		case 4:
			if (files.count) && (!itdir) RunProgram("/sys/develop/cedit", #file_path);
			break;
		case 5:
			if (efm) {
				CopyFilesListToClipboard(COPY);
				EventPaste(#inactive_path);
			} else {
				EventRefreshDisksAndFolders();
			}
			break;
		case 6:
			if (efm) {
				CopyFilesListToClipboard(CUT);
				EventPaste(#inactive_path);
			}
			break;
		case 7:
			NewElement_Form(CREATE_FOLDER, T_NEW_FOLDER);
			break;
		case 8:
			Del_Form();
			break;
		case 9:
			NewElement_Form(CREATE_FILE, T_NEW_FILE);
			break;
		case 10: //F10
			if (!active_settings) 
			{
				settings_stak = malloc(4096);
				settings_window = CreateThread(#settings_dialog, settings_stak+4092);
				break;
			}
			else
			{
				ActivateWindow(GetProcessSlot(settings_window));
			}
			break;
	}
}

void EventRefresh()
{
	if (efm)
	{
		DrawFilePanels();
	} else {
		Tip(56, T_DEVICES, 55, "-");
		Open_Dir(#path,WITH_REDRAW);
		pause(10);
		DrawDeviceAndActionsLeftPanel();				
	}
}

void ChangeActivePanel()
{
	if (active_panel==1) active_panel=2; else active_panel=1;
	llist_copy(#files_active, #files_inactive);
	llist_copy(#files_inactive, #files);
	strcpy(#active_path, #inactive_path);
	strcpy(#inactive_path, #path);
	DrawFilePanels();
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

dword GetDeviceSizeLabel(dword path)
{
	BDVK bdvk;
	if (ESBYTE[path+1] == '/') path++;
	if (ESBYTE[path+1] == 'c') && (ESBYTE[path+2] == 'd')
		&& (ESBYTE[path+4] == 0) return 0;
	GetFileInfo(path, #bdvk);
	return ConvertSize64(bdvk.sizelo, bdvk.sizehi);
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
	if(GetRealFileCountInFolder("/")+dir_exists("/kolibrios") != SystemDiscs.dev_num) {
		SystemDiscs.Get();
		EventRefresh();
	} else {
		if(GetRealFileCountInFolder(#path) != files.count) EventRefresh();
		else if(efm) && (GetRealFileCountInFolder(#inactive_path) != files_inactive.count) {
			EventRefresh();
		}
	}
}

void EventSort(dword id)
{
	char selected_filename[256];
	if (sort_type == id) sort_desc ^= 1;
	else sort_type = id;
	strcpy(#selected_filename, #file_name);
	DrawButtonsAroundList();
	Open_Dir(#path,WITH_REDRAW);
	SelectFileByName(#selected_filename);
}

void EventHistoryGoForward()
{
	if (history.forward()) {
		strcpy(#path, history.current());
		files.KeyHome();
		Open_Dir(#path,WITH_REDRAW);
	}
}

void EventOpenNewEolite()
{
	RunProgram(I_Path, #path);
}

void EventOpenConsoleHere()
{
	sprintf(#param, "pwd cd %s", #path);
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

void EventDelete() 
{
	char line_param[4096+5];
	CopyFilesListToClipboard(DELETE);
	EventClosePopinForm();
	sprintf(#line_param, "-d %s", #file_path);
	RunProgram(#program_path, #line_param);	
}

void EventClosePopinForm()
{
	del_active = 0;
	new_element_active = 0;
	disk_popin_active_on_panel = 0;
	draw_window();
}

void EventShowProperties()
char line_param[4096+5];
{
	if (!selected_count) {
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
		case PASTE_BTN:EventPaste(#path); break;		
	}
}

void EventDriveClick(int __id)
{
	if (disk_popin_active_on_panel != active_panel) {
		ChangeActivePanel();
	}

	SystemDiscs.Click(__id-100);
	if (efm) {
		draw_window();
	}
}

void EventOpenDiskPopin(int panel_n)
{
	DefineHiddenButton(0,0,5000,3000,9999+BT_NOFRAME);
	disk_popin_active_on_panel = panel_n;
	if (disk_popin_active_on_panel==1) {
		SystemDiscs.DrawOptions(Form.cwidth/2-DDW, 8+DEV_H_HOR+3);
	} else {
		SystemDiscs.DrawOptions(Form.cwidth-DDW-2, 8+DEV_H_HOR+3);
	}
}

stop:
