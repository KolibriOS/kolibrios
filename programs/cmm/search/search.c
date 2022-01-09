#define MEMSIZE 100*1000

#include "../lib/fs.h"
#include "../lib/gui.h"
#include "../lib/list_box.h"
#include "../lib/obj/box_lib.h"
#include "../lib/obj/proc_lib.h"
#include "../lib/obj/libini.h"
#include "../lib/patterns/select_list.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

#ifdef LANG_RUS
	?define T_WINDOW_HEADER "Поиск"
	?define T_BUTTON_SEARCH "Найти"
	?define T_SEARCH_PATH "Каталог поиска:"
	?define T_SEARCH_NAME "Имя файла:"
#else
	?define T_WINDOW_HEADER "Search"
	?define T_BUTTON_SEARCH "Search"
	?define T_SEARCH_PATH "Search in:"
	?define T_SEARCH_NAME "File name:"
#endif

char search_name[64];
char search_path[4096];

enum {
	BTN_SEARCH = 10,
	BTN_CHOOSE_PATH
};

proc_info Form;
#define TOOLBAR_H 100

edit_box edit_name = {230,13, 30,0xffffff,0x94AECE,0xffffff,0xffffff,0x10000000,sizeof(search_name)-2,#search_name,0, ed_focus,0,0};
edit_box edit_path = {310,260,30,0xffffff,0x94AECE,0xffffff,0xffffff,0x10000000,sizeof(search_path)-2,#search_path,0, 0b,0,0};

opendialog open_folder_dialog = 
{
  2, //0-file, 2-save, 3-select folder
  #Form,
  #communication_area_name,
  0,
  0, //dword opendir_path,
  #search_path, //dword dir_default_path,
  #open_dialog_path,
  #draw_window,
  0,
  #search_path, //dword openfile_path,
  0, //dword filename_area,
  0, //dword filter_area,
  420,
  NULL,
  320,
  NULL
};

//===================================================//
//                                                   //
//                      RESULTS                      //
//                                                   //
//===================================================//

struct RESULTS {
	collection name;
	collection path;
	collection_int type;
	unsigned count;
	void add();
	void drop();
} results;

void RESULTS::add(dword _name, _path, _type)
{
	name.add(_name);
	path.add(_path);
	type.add(_type);
	count++;
}

void RESULTS::drop()
{
	name.drop();
	path.drop();
	type.drop();
	count = 0;
}

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void main()
{  
	int prev_first, prev_cur_y;
	load_dll(boxlib, #box_lib_init,0);
	load_dll(libini, #lib_init,1);
	load_dll(Proc_lib, #OpenDialog_init,0);
	OpenDialog_init stdcall (#open_folder_dialog);

	edit_box_set_text stdcall (#edit_name, ".ini");
	edit_box_set_text stdcall (#edit_path, "/kolibrios");
	
	@SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
	loop() switch(@WaitEvent())
	{
	  	case evMouse:
			edit_box_mouse stdcall (#edit_name);
			edit_box_mouse stdcall (#edit_path);
			if (SelectList_ProcessMouse()) {
				SelectList_Draw();
			} else {
				SelectList_DrawLine(select_list.cur_y);
			}
			if (mouse.key&MOUSE_RIGHT) && (mouse.up) && (select_list.MouseOver()) EventOpenFile(false);
	  		break;

		case evButton:
			switch (@GetButtonID()) {
				case 1: @ExitProcess(); break;
				case BTN_SEARCH: EventSearch(); break;
				case BTN_CHOOSE_PATH: EventChooseSearchInPath();
			}
			break;
	  
		case evKey:
			@GetKeys(); 
			edit_box_key stdcall (#edit_name);
			edit_box_key stdcall (#edit_path);
			if (edit_name.flags & ed_focus) || (edit_path.flags & ed_focus) {
				if (SCAN_CODE_ENTER == key_scancode) EventSearch();
			} else {
				if (SCAN_CODE_ENTER == key_scancode) EventOpenFile(true);
				prev_first = select_list.first;
				prev_cur_y = select_list.cur_y;
				if (select_list.ProcessKey(key_scancode)) {
					if (prev_first == select_list.first) {
						SelectList_DrawLine(prev_cur_y);
						SelectList_DrawLine(select_list.cur_y);
					} else {
						SelectList_Draw();
					}
				}
			}
			break;
		 
		case evReDraw:
			draw_window();
			break;
   }
}

void draw_window()
{
	sc.get();			
	DefineAndDrawWindow(screen.w-600/2,100,640,600,0x73,sc.work,T_WINDOW_HEADER,0);
	GetProcessInfo(#Form, SelfInfo);
	IF (Form.status_window&ROLLED_UP) return;
	if (Form.width  < 270) { MoveSize(OLD,OLD,270,OLD); return; }
	if (Form.height < 200) { MoveSize(OLD,OLD,OLD,200); return; }
	SelectList_Init( 0, TOOLBAR_H, 
		Form.cwidth-scroll1.size_x-1, 
		Form.cheight-TOOLBAR_H-1);
	SelectList_Draw();
	DrawBar(0, TOOLBAR_H-1, Form.cwidth, 1, sc.line);
	DrawBar(0, 0, Form.cwidth, TOOLBAR_H-1, sc.work);
	DrawEditBox(#edit_name);
	WriteText(edit_name.left-2, edit_name.top-20, 0x90, sc.work_text, T_SEARCH_NAME);
	edit_path.width = Form.cwidth - 314;
	DrawFileBox(#edit_path, T_SEARCH_PATH, BTN_CHOOSE_PATH);
	DrawStandartCaptButton(10, 63, BTN_SEARCH, T_BUTTON_SEARCH);
}

_ini ini = { "/sys/File managers/icons.ini", "icons16" };
void SelectList_DrawLine(dword i)
{
	int yyy = i*select_list.item_h+select_list.y;
	dword bg = 0xFFFfff;
	dword col = 0;
	int icon;
	char tname[4096];
	
	if (!select_list.count) return;

	if (select_list.cur_y-select_list.first==i)
	{
		bg = sc.button;
		col = sc.button_text;
		if (edit_name.flags & ed_focus) || (edit_path.flags & ed_focus) bg = 0xAAAaaa;
	}

	if (results.type.get(select_list.first + i)==true) {
		icon=0;
	} else {
		strcpy(#tname, results.name.get(select_list.first + i));
		strlwr(#tname);
		icon = ini.GetInt(#tname + strrchr(#tname, '.'), 2);
	}

	#define ICONX 7
	DrawBar(select_list.x, yyy, ICONX-1, select_list.item_h, 0xFFFfff);
	DrawBar(select_list.x+ICONX+18, yyy, select_list.w-ICONX-18, select_list.item_h, bg);
	draw_icon_16(select_list.x+ICONX, yyy+1, icon);

	WriteText(select_list.x + ICONX+18+4,yyy+select_list.text_y,0x90, col, results.name.get(select_list.first + i));
	WriteText(select_list.x + ICONX+18+206,yyy+select_list.text_y,0x90, col, results.path.get(select_list.first + i));
}

void SelectList_LineChanged() 
{
	return;
}

//===================================================//
//                                                   //
//                     EVENTS                        //
//                                                   //
//===================================================//

void EventChooseSearchInPath()
{
	OpenDialog_start stdcall (#open_folder_dialog);
	if (open_folder_dialog.status) {
		edit_box_set_text stdcall (#edit_path, #search_path);		
	}
}

void getfullpath(dword to, path, name) {
	strcpy(to, path);
	chrcat(to, '/');
	strcat(to, name);
}

void EventOpenFile(int run_file_not_show_in_folder)
{
	char full_path[4096];
	int pos = select_list.cur_y;
	getfullpath(#full_path, results.path.get(pos), results.name.get(pos));
	if (run_file_not_show_in_folder) {
		RunProgram("/sys/@open", #full_path);
	} else {
		RunProgram("/sys/file managers/eolite", #full_path);	
	}
}

void EventSearch()
{
	results.drop();
	find_loop(#search_path);
	select_list.count = results.count;
	SelectList_Draw();
}

void find_loop(dword way)
{
	dword dirbuf, fcount, i, filename;
	dword cur_file;
	bool folder;

	if (way) && (dir_exists(way))
	{
		cur_file = malloc(4096);
		// In the process of recursive descent, memory must be allocated dynamically, 
		// because the static memory -> was a bug !!! But unfortunately pass away to sacrifice speed.
		GetDir(#dirbuf, #fcount, way, DIRS_ONLYREAL);
		for (i=0; i<fcount; i++)
		{
			filename = i*304+dirbuf+72;
			getfullpath(cur_file,way,filename);
			if (ESDWORD[filename-40] & ATR_FOLDER) folder = true; else folder = false;

			if (strstri(filename, #search_name)) {
				results.add(filename, way, folder);
			}

			if (folder) {
				find_loop(cur_file);
			}
		}
		free(cur_file);
		free(dirbuf);
	}
}

stop:


