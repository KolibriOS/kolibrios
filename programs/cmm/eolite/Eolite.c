//Leency, Veliant, Punk_Joker & KolibriOS Team 2008-2015
//GNU GPL licence.

#ifndef AUTOBUILD
#include "lang.h--"
#endif

//libraries
#define MEMSIZE 0xD0000
#include "..\lib\kolibri.h"
#include "..\lib\clipboard.h"
#include "..\lib\strings.h"
#include "..\lib\mem.h"
#include "..\lib\dll.h"
#include "..\lib\file_system.h"
#include "..\lib\gui.h"
#include "..\lib\list_box.h"
#include "..\lib\copyf.h"
#include "..\lib\random.h"
//obj
#include "..\lib\obj\libini.h"
#include "..\lib\obj\box_lib.h"
//images
#include "imgs\toolbar.txt"
#include "imgs\left_p.txt"
#include "imgs\icons.txt"

#ifdef LANG_RUS
	?define T_FILE "Файл"
	?define T_TYPE "Тип"
	?define T_SIZE "Размер"
	?define T_NEW_FOLDER "Новая папка"
	?define T_NEW_FILE "Новый файл"
	?define T_DELETE_FILE "Вы действительно хотите удалить"
	?define T_YES "Да"
	?define T_NO "Нет"
	?define T_CANCEL "Отмена"
	?define T_CREATE "Создать"
	?define T_RENAME "Переименовать"
	?define FILE_EXISTS "Файл с таким именем существует"
	?define FOLDER_EXISTS "Папка с таким именем существует"
	?define T_DEL_ERROR_1 "Ошибка. Папка не пустая."
	?define WAIT_DELETING_FOLDER "Удаляется папка. Подожите..."
	?define NOT_CREATE_FOLDER "Не удалось создать папку."
	?define NOT_CREATE_FILE "Не удалось создать файл."
	?define ERROR_1 "Ошибка при загрузке библиотеки /rd/1/lib/box_lib.obj"
	?define T_PASTE_WINDOW_TITLE "Копирую..."
	?define T_PASTE_WINDOW_TEXT "Копируется файл:"
	?define T_PASTE_WINDOW_BUTTON "Прервать"	
	?define INFO_AFTER_COPY "Копирование завершено"
	?define T_CANCEL_PASTE "Копирование прекращено. Папка скопирована не полностью."
	?define T_SELECT_APP_TO_OPEN_WITH "Выберите программу для открытия файла"
	?define DEL_MORE_FILES_1 "выбранные элементы ("
	?define DEL_MORE_FILES_2 " шт.)?"
#elif LANG_EST
	?define T_FILE "Fail"
	?define T_TYPE "T№№p"
	?define T_SIZE "Suurus"
	?define T_NEW_FOLDER "Uus kataloog"
	?define T_NEW_FILE "Uus fail"
	?define T_DELETE_FILE "Kas sa tahad tїesti kustutada"
	?define T_YES "Jah"
	?define T_NO "Ei"
	?define T_CANCEL "Cancel"
	?define T_CREATE "Create"
	?define T_RENAME "Rename"
	?define FILE_EXISTS "The file with the same name exists"
	?define FOLDER_EXISTS "A folder with the same name exists"
	?define T_DEL_ERROR_1 "Viga. Kataloog ei ole t№hi."
	?define WAIT_DELETING_FOLDER "Deleting folder. Please, wait..."
	?define NOT_CREATE_FOLDER "Kataloogi ei saa luua."
	?define NOT_CREATE_FILE "Faili ei saa luua."
	?define ERROR_1 "Viga teegi laadimisel /rd/1/lib/box_lib.obj"
	?define T_PASTE_WINDOW_TITLE "Kopeerin..."
	?define T_PASTE_WINDOW_TEXT "Kopeerin faili:"
	?define T_PASTE_WINDOW_BUTTON "Abort"
	?define INFO_AFTER_COPY "Copy finished"
	?define T_CANCEL_PASTE "Copy process terminated. Folder copied incompletely."
	?define T_SELECT_APP_TO_OPEN_WITH "Select application to open file"
	?define DEL_MORE_FILES_1 "selected items("
	?define DEL_MORE_FILES_2 " pcs.)?"
#else
	?define T_FILE "File"
	?define T_TYPE "Type"
	?define T_SIZE "Size"
	?define T_NEW_FOLDER "New folder"
	?define T_NEW_FILE "New file"
	?define T_DELETE_FILE "Do you really want to delete"
	?define T_YES "Yes"
	?define T_NO "No"
	?define T_CANCEL "Cancel"
	?define T_CREATE "Create"
	?define T_RENAME "Rename"
	?define FILE_EXISTS "The file with the same name exists"
	?define FOLDER_EXISTS "A folder with the same name exists"
	?define T_DEL_ERROR_1 "Error. Folder isn't empty."
	?define WAIT_DELETING_FOLDER "Deleting folder. Please, wait..."
	?define NOT_CREATE_FOLDER "Folder can not be created."
	?define NOT_CREATE_FILE "File can not be created."
	?define ERROR_1 "Error while loading library /rd/1/lib/box_lib.obj"
	?define T_PASTE_WINDOW_TITLE "Copying..."
	?define T_PASTE_WINDOW_TEXT "Copying file:"
	?define T_PASTE_WINDOW_BUTTON "Abort"
	?define INFO_AFTER_COPY "Copy finished"
	?define T_CANCEL_PASTE "Copy process terminated. Folder copied incompletely."
	?define T_SELECT_APP_TO_OPEN_WITH "Select application to open file"
	?define DEL_MORE_FILES_1 "selected items("
	?define DEL_MORE_FILES_2 " pcs.)?"
#endif

enum {ONLY_SHOW, WITH_REDRAW, ONLY_OPEN}; //OpenDir
enum { CREATE_FILE=1, CREATE_FOLDER, RENAME_ITEM }; //NewElement

#define TITLE "Eolite File Manager v2.81"
#define ABOUT_TITLE "Eolite v2.81"
dword col_padding, col_selec, col_lpanel;

int toolbar_buttons_x[7]={9,46,85,134,167,203};
struct path_string { char Item[4096]; };

byte active_about=0;
word about_window;
byte active_settings=0;
word settings_window;
dword _not_draw = false;
byte menu_call_mouse=0;

llist files;

byte
	path[4096],
	file_path[4096],
	file_name[256],
	new_element_name[256],
	temp[4096];	 
byte
	del_active=0,
	new_element_active=0,
	show_dev_name=1,
	real_files_names_case=0,
	use_big_fonts=0,
	font_type,
	info_after_copy=0,
	sort_num=2,
	itdir;

dword eolite_ini_path;

dword menu_stak,about_stak,properties_stak,settings_stak,copy_stak;

proc_info Form;
system_colors sc;
mouse m;
int mouse_dd, scroll_used, sc_slider_h, sorting_arrow_x, kolibrios_drive;
dword buf;
dword file_mas[6898];
int j, i;
int action_buf;
int rand_n;
int selected_count;
byte CMD_REFRESH;

mouse gestures;
signed x_old, y_old, dif_x, dif_y, adif_x, adif_y;
byte stats;

edit_box edit2 = {250,213,80,0xFFFFCC,0x94AECE,0xFFFFCC,0xFFFFFF,0,248,#file_name,#mouse_dd,64,6,6};
edit_box new_file_ed = {171,213,180,0xFFFFFF,0x94AECE,0xFFFFFF,0xFFFFFF,0,248,#new_element_name,#mouse_dd,100000000000010b,6,0};
PathShow_data PathShow = {0, 17,250, 6, 250, 0, 0, 0x0, 0xFFFfff, #path, #temp, 0};
PathShow_data FileShow = {0, 56,215, 6, 100, 0, 0, 0x0, 0xFFFfff, #file_name, #temp, 0};
byte cmd_free;
#include "include\copy.h"
#include "include\gui.h"
#include "include\sorting.h"
#include "include\icons.h"
#include "include\left_panel.h"
#include "include\history.h"
#include "include\menu.h"
#include "include\about.h"
#include "include\settings.h"
#include "include\properties.h"


void main() 
{
	word key, id, can_show, can_select, m_selected;
	dword selected_offset;
	dword IPC_LEN,IPC_ID;
	char IPC_BUF[10];
	dword tmp;
	rand_n = random(40);
	gestures.get();
	mem_Init();
	if (load_dll2(boxlib, #box_lib_init,0)!=0) notify(ERROR_1);
    if (load_dll2(libini, #lib_init,1)!=0) notify("Error: library doesn't exists - libini");
	eolite_ini_path = abspath("Eolite.ini"); 
	LoadIniSettings();
	GetSystemDiscs();
	SetAppColors();
	if (param)
	{
		tmp = strlen(#path);
		strncpy(#path, #param, tmp);
		$dec tmp
		if (path[tmp]!='/') DSBYTE[#path+tmp] = '/'; //add "/" to the end of the string
	}
	else
	{
		strncpy(#path, "/rd/1/", 6);		
	}
	Open_Dir(#path,ONLY_OPEN);
	SetEventMask(1100111b);
	loop(){
		switch(WaitEvent())
		{
			case evMouse:
				if (del_active) || (!CheckActiveProcess(Form.ID)) || (Form.status_window>2) break;
				if (new_element_active) || (!CheckActiveProcess(Form.ID)) || (Form.status_window>2)
				{
					edit_box_mouse stdcall(#new_file_ed);
					break;
				}				
				
				m.get();
				
				
				gestures.get();
				if (!gestures.mkm) && (stats>0) stats = 0;
				if (gestures.mkm) && (stats==0)
				{
					x_old = gestures.x;
					y_old = gestures.y;
					stats = 1;
				}
				if (gestures.mkm) && (stats==1)
				{
					dif_x = gestures.x-x_old;
					dif_y = gestures.y-y_old;
					adif_x = fabs(dif_x);
					adif_y = fabs(dif_y);
					
					if (adif_x>adif_y)
					{
						if (dif_x > 150)
						{
							if (HistoryPath(GO_FORWARD))
								{
									files.first=files.current=NULL;
									Open_Dir(#path,WITH_REDRAW);
								}
							stats = 0;
						}
						if (dif_x < -150)
						{
							GoBack();
							stats = 0;
						}
					}
					else
					{
						if (dif_y < -100)
						{
							Dir_Up();
							stats = 0;
						}
					}
				}	
				if (files.MouseOver(m.x, m.y))&&((m.up)||(m.down)||(m.dblclick))
				{
					//select/open file {
					if (m.key&MOUSE_LEFT)&&((m.down)||(m.dblclick))
					{
						if (m.y>=files.y)//&&(m.click)
						{
							id = m.y - files.y / files.line_h;
							if (files.current!=id)
							{
								m.clearTime();
								if (id<files.visible) List_Current(id-files.current);
							}
							else if(m.dblclick)Open(0);
						}
					}
					// } select/open file
					else
					//file menu {
					if (m.key&MOUSE_RIGHT)&&(m.up)
					{
						menu_call_mouse = 1;
						if (m.y>=files.y)//&&(m.click)
						{
							id = m.y - files.y / files.line_h;
							if (files.current!=id) List_Current(id-files.current);
							//SwitchToAnotherThread();
							menu_stak = malloc(4096);
							CreateThread(#FileMenu,menu_stak+4092);
						}
						break;
					}
					// } file menu
				}

				if (m.vert)
				{
					if (files.MouseScroll(m.vert)) List_ReDraw();
					break;
				}

				if (m.x>=Form.width-26) && (m.x<=Form.width-6) && (m.y>40) && (m.y<files.y)
				{
					if (m.lkm==1) DrawRectangle3D(Form.cwidth - 17,41,14,14,0xC7C7C7,0xFFFFFF);
					WHILE (m.lkm==1) && (files.first>0)
					{
						pause(8);
						files.first--;
						List_ReDraw();
						m.get();
					}
					DrawRectangle3D(Form.cwidth - 17,41,14,14,0xFFFFFF,0xC7C7C7);
				}

				if (m.x>=Form.width-26) && (m.x<=Form.width-6) && (m.y>onTop(22,0)+1) && (m.y<onTop(22,0)+16)
				{
					if (m.lkm==1) DrawRectangle3D(Form.cwidth - 17,onTop(21,0),14,14,0xC7C7C7,0xFFFFFF);
					while (m.lkm==1) && (files.first<files.count-files.visible)
					{
						pause(8);
						files.first++;
						List_ReDraw();
						m.get();
					}
					DrawRectangle3D(Form.cwidth - 17,onTop(21,0),14,14,0xFFFFFF,0xC7C7C7);
				}

				//Scrooll
				if (!m.lkm) && (scroll_used) { scroll_used=NULL; Scroll(); }
				if (m.x>=Form.width-26) && (m.x<=Form.width-6) && (m.y>56) && (m.y<Form.height) && (m.lkm) && (!scroll_used) {scroll_used=1;Scroll();}
				
				if (scroll_used)
				{
					if (sc_slider_h/2+files.y>m.y) || (m.y<0) || (m.y>4000) m.y=sc_slider_h/2+files.y; //anee eo?ni? iaa ieiii
					id=files.first;
					j= sc_slider_h/2;
					files.first = m.y -j -files.y * files.count;
					files.first /= onTop(22,files.y);
					if (files.visible+files.first>files.count) files.first=files.count-files.visible;
					if (id!=files.first) List_ReDraw();
				}
				break;  
	//Button pressed-----------------------------------------------------------------------------
			case evButton:
				id=GetButtonID();
				if (id==1)
				{
					KillProcess(about_window);
					ExitProcess();
				}
				if (del_active)
				{
					if (id==301) || (id==302) Del_File(302-id);
					break;
				}
				if (new_element_active)
				{
					if (id==301) || (id==302) NewElement(302-id);
					break;
				}
				
				switch(id) 
				{
					case 21: //Back
							GoBack();
							break;
					case 22: //Forward
							if (HistoryPath(GO_FORWARD))
							{
								files.first=files.current=NULL; //aaa?o nienea
								Open_Dir(#path,WITH_REDRAW);
							}
							break;
					case 23:
							Dir_Up();
							break;
					case 24:
							Copy(#file_path, CUT);
							break;
					case 25:
							Copy(#file_path, NOCUT);
							break;
					case 26:
							Paste();
							break;
					case 31...33: //sort
							if(sort_num==1) DrawFilledBar(sorting_arrow_x,42,6,10);
							if(sort_num==2) DrawFilledBar(sorting_arrow_x,42,6,10);
							if(sort_num==3) DrawFilledBar(sorting_arrow_x,42,6,10);
							sort_num=id-30;
							Open_Dir(#path,WITH_REDRAW);
							break;
					case 50...60: //Actions
							FnProcess(id-50);
							break;
					case 100...120:
						DEVICE_MARK:
							DrawRectangle(17,id-100*16+74,159,16, 0); //auaaeaiea
							strcpy(#path, #disk_list[id-100].Item);
							files.first=files.current=0;
							Open_Dir(#path,WITH_REDRAW);
							pause(5);
							DrawRectangle(17,id-100*16+74,159,16, 0xFFFFFF);
							break;
				}
				break;
	//Key pressed-----------------------------------------------------------------------------
			case evKey:
				key = GetKey();
				if (Form.status_window>2) break;
				if (del_active)
				{
					if (key==013) Del_File(true);
					if (key==027) Del_File(false);
					break;
				}
				if (new_element_active)
				{
					if (key==027) NewElement(0);
					if (key==013) NewElement(1);
					EAX=key<<8;
					edit_box_key stdcall (#new_file_ed);
					break;
				}
				if (edit2.flags!=64) && (key!=13) && (key!=27)
				{
					EAX=key<<8;
					edit_box_key stdcall (#edit2);
					break;
				}
				switch (key)
				{
						case 209...217:
								id=key-110;
								if (id-100>=disc_num) break;
								GOTO DEVICE_MARK;
						case ASCII_KEY_BS:
								//GoBack();
								Dir_Up();
								break; 
						case 004: //Ctrl+D set as bg
								strncpy(#temp, "\\S__",4);
								strcat(#temp, #file_path);
								RunProgram("/sys/media/kiv", #temp);
								break;
						case 014: //Ctrl+N new window
								if (Form.left==98) MoveSize(Form.left-20,Form.top-20,OLD,OLD);
								RunProgram("/sys/File Managers/Eolite", #path);
								break; 
						case 024: //Ctrl+X
								Copy(#file_path, CUT);
								break;
						case 003: //Ctrl+C
								Copy(#file_path, NOCUT);
								break;
						case 022: //Ctrl+V
								Paste();
								break;
						case 001: //Ctrl+A
								debugln("press Ctrl+A");
								for (i=0; i<files.count; i++) 
								{
									selected_offset = file_mas[i]*304 + buf+32 + 7;
									ESBYTE[selected_offset] = 1;
									selected_count++;
								}
								List_ReDraw();
								break;
						case 021: //Ctrl+U
								debugln("press Ctrl+A");
								for (i=0; i<files.count; i++) 
								{
									selected_offset = file_mas[i]*304 + buf+32 + 7;
									ESBYTE[selected_offset] = 0;
								}
								selected_count = 0;
								List_ReDraw();
								break;
						case ASCII_KEY_ESC:
								break;
						case ASCII_KEY_ENTER:
								Open(0);
								break; 
						case 074: //menu
								menu_call_mouse=0;
								//SwitchToAnotherThread();
								menu_stak = malloc(4096);
								CreateThread(#FileMenu,menu_stak+4092);
								break;
						case 173: //Ctrl+Enter
								if (!itdir) ShowOpenWithDialog();
								else Open(1);
								break;
						case ASCII_KEY_UP:
								List_Current(-1);
								break;
						case ASCII_KEY_DOWN:
								List_Current(1);
								break;
						case ASCII_KEY_HOME:
								if (files.KeyHome()) List_ReDraw();
								break;
						case ASCII_KEY_END:
								if (files.KeyEnd()) List_ReDraw();
								break;
						case ASCII_KEY_PGDN:
								List_Current(files.visible-1);
								break;
						case ASCII_KEY_PGUP:
								List_Current(-files.visible+1);
								break;
						case ASCII_KEY_DEL:
								Del_Form();
								break;
						case ASCII_KEY_INS:
								selected_offset = file_mas[files.current+files.first]*304 + buf+32 + 7;
								if (ESBYTE[selected_offset])
								{
									ESBYTE[selected_offset]=0;
									selected_count--;
								}
								else
								{
									ESBYTE[selected_offset] = 1;
									selected_count++;
								}
								List_Current(1);
								break;
						case 048...059: //F1-F10
								FnProcess(key-49);
								break; 
						default:    
								for (i=files.current+files.first+1; i<files.count; i++)
								{
									strcpy(#temp, file_mas[i]*304+buf+72);
									if (temp[0]==key) || (temp[0]==key-32)
									{
										List_Current(i-files.current-files.first);
										break;
									}
								}
				}                         
			break;
			case evReDraw:
				DRAW_WINDOW:
				draw_window();
				if (action_buf) 
				{
					menu_action(action_buf); 
					action_buf=0;
				}
			break;
			case evIPC:
				goto DRAW_WINDOW;
			break;
		}
		
		if(cmd_free)
		{
			if(cmd_free==1)     free(menu_stak);
			else if(cmd_free==2)free(about_stak);
			else if(cmd_free==3)free(properties_stak);
			else if(cmd_free==4)free(settings_stak);
			else if(cmd_free==5)free(copy_stak);
			cmd_free = false;
		}
	}
}


inline fastcall signed int _strrchr( ESI,BL)
{
	int jj=0, last=strlen(ESI);
	do {
		jj++;
		$lodsb
		if(AL==BL) last=jj;
	} while(AL!=0);
	return last;
}


void menu_action(dword id)
{
	if (id==COPY_PASTE_END)
	{
		FnProcess(5);
		SelectFile(#copy_to+strrchr(#copy_to,'/'));
	}
	if (id==100) Open(0);
	if (id==201) ShowOpenWithDialog();
	if (id==202) FnProcess(3); //F3
	if (id==203) FnProcess(4); //F4
	if (id==104) Copy(#file_path, NOCUT);
	if (id==105) Copy(#file_path, CUT);
	if (id==106) Paste();
	if (id==207) FnProcess(2);
	if (id==108) Del_Form();
	if (id==109) FnProcess(5);
	if (id==110) FnProcess(8);
	if (id==300)
	{ 
		FnProcess(5); 
		List_ReDraw(); 
	}
}


void draw_window()
{
	DefineAndDrawWindow(GetScreenWidth()-550/4+rand_n,rand_n+30,550,500,0x73,sc.work,TITLE,0);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return;
	files.SetSizes(192, 57, Form.cwidth - 210, onTop(57,6), disc_num*16+195,files.line_h);
	if (Form.height < files.min_h) MoveSize(OLD,OLD,OLD,files.min_h);
	if (Form.width<480) MoveSize(OLD,OLD,480,OLD);
	GetProcessInfo(#Form, SelfInfo); //if win_size changed

	PutPaletteImage(#toolbar,246,34,0,0,8,#toolbar_pal);
	DrawBar(127, 8, 1, 25, sc.work_graph);
	for (j=0; j<3; j++) DefineButton(toolbar_buttons_x[j]+2,5+2,31-5,29-5,21+j+BT_HIDE,sc.work);
	for (j=3; j<6; j++) DefineButton(toolbar_buttons_x[j],5,31,29,21+j+BT_HIDE,sc.work);
	DrawBar(246,0,Form.cwidth - 297,12, sc.work); //upper editbox
	DrawBar(246,29,Form.cwidth - 297,5,sc.work);  //under editbox
	DrawRectangle(246,12,Form.cwidth - 303,16,sc.work_graph);
	DefineButton(Form.cwidth - 32,6,27,28,51+BT_HIDE+BT_NOFRAME,0); //about
	PutPaletteImage(#goto_about,56,34,Form.width-65,0,8,#goto_about_pal);
	//main rectangles
	DrawRectangle(1,40,Form.cwidth-3,onTop(46,0),sc.work_graph);
	DrawRectangle(0,39,Form.cwidth-1,onTop(44,0),col_palette[4]); //bg
	for (i=0; i<5; i++) DrawBar(0, 34+i, Form.cwidth, 1, col_palette[8-i]);	
	DrawLeftPanel();
	//ListBox
	DrawFlatButton(files.x,40,Form.cwidth - files.x - 159,16,31,sc.work,T_FILE);
	DrawFlatButton(Form.cwidth - 159,40,73,16,32,sc.work,T_TYPE);
	DrawFlatButton(Form.cwidth - 86,40,68,16,33,sc.work,T_SIZE);
	DrawBar(files.x+files.w,files.y,1,onTop(22,files.y),sc.work_graph); //line to the left from the scroll
	DrawFlatButton(files.x+files.w,40,16,16,0,sc.work,"\x18");
	DrawFlatButton(files.x+files.w,onTop(22,0),16,16,0,sc.work,"\x19");
	Open_Dir(#path,ONLY_SHOW);
	if (del_active) Del_Form();
	if (new_element_active) NewElement_Form(new_element_active, #new_element_name);
}


void KEdit()
{
	if (Form.width<480) return;
	PathShow.area_size_x = Form.cwidth-306;
	DrawBar(PathShow.start_x-3, PathShow.start_y-4, PathShow.area_size_x+2, 15, 0xFFFfff);
	PathShow_prepare stdcall(#PathShow);
	PathShow_draw stdcall(#PathShow);
}


void List_Current(signed int cur)
{
	if (cur<=0) //up
	{
		if (files.first==0) && (files.current<=0) return;
		if (-cur-1<files.current)
		{
			Line_ReDraw(0xFFFFFF, files.current);
			files.current+=cur;
			Line_ReDraw(col_selec, files.current);
			return;
		}
		else
		{
			if (-cur<files.first) files.first+=cur; else files.first=0;
			files.current=0;
			List_ReDraw();
			return;
		}
	}
	else  //down
	{
		if (files.first==files.count-files.visible) && (files.current==files.visible-1) 
		{
			Line_ReDraw(col_selec, files.current);
			return;
		}
		if (files.visible-files.current>cur)
		{
			Line_ReDraw(0xFFFFFF, files.current);
			files.current+=cur;
			Line_ReDraw(col_selec, files.current);
			return;
		}
		else
		{
			if(files.first+files.current+cur>=files.count)
			{
				files.first=files.count-files.visible;
				files.current=cur-files.first+files.current;
				}
			else
			{
				files.first+=cur+files.current-files.visible+1;
				files.current=files.visible-1;
			}
			
			if (files.current<0) || (files.current>files.visible)
			{
				files.current=files.visible-1;
			}
			List_ReDraw();
		}
	}
}


void List_ReDraw()
{
	int paint_y;
	//we are in the end of the list => maximize window => there will be white lines after the last element
	if (files.count-files.first<files.visible) || (files.current>files.visible-1)
	{ files.first=files.count-files.visible; files.current=files.visible-1; }
	for (j=0; j<files.visible; j++) if (files.current!=j) Line_ReDraw(0xFFFFFF, j); else Line_ReDraw(col_selec, files.current);
	//in the bottom
	paint_y = j * files.line_h + files.y;
	DrawBar(files.x,paint_y,files.w,onTop(paint_y,6),0xFFFFFF);
	DrawBar(Form.cwidth-159,paint_y,1,onTop(paint_y,6),sc.work);
	DrawBar(Form.cwidth-86,paint_y,1,onTop(paint_y,6),sc.work);
	Scroll();
}


void Line_ReDraw(dword color, filenum){
	dword text_col=0,
	      ext1, attr,
	      file_offet,
	      file_name_off,
	      y=filenum*files.line_h+files.y;
	      BDVK file;
	if (filenum==-1) return;
	DrawBar(files.x,y,3,files.line_h,color); 
	DrawBar(files.x+19,y,files.w-19,files.line_h,color);
	DrawBar(files.x+3,y+17,16,1,color);
	if (files.line_h>18) DrawBar(files.x+3,y+18,16,files.line_h-18,color);
	if (files.line_h>15) DrawBar(files.x+3,y,16,files.line_h-15,color); 

	file_offet = file_mas[filenum+files.first]*304 + buf+32;
	attr = ESDWORD[file_offet];
	file.selected = ESBYTE[file_offet+7];
	file.sizelo   = ESDWORD[file_offet+32];
	file_name_off = file_offet+40;

	if (! TestBit(attr, 4) ) //file or folder?
	{	
		Put_icon(file_name_off+_strrchr(file_name_off,'.'), files.x+3, files.line_h/2-7+y, color, 0);
		WriteText(7-strlen(ConvertSize(file.sizelo))*6+Form.cwidth - 76,files.line_h-6/2+y,font_type,0,ConvertSize(file.sizelo));
	}
	else
	{
		if (!strncmp(file_name_off,"..",3)) ext1=".."; else ext1="<DIR>";
		Put_icon(ext1, files.x+3, files.line_h/2-7+y, color, 0);		
	}

	if (TestBit(attr, 1)) || (TestBit(attr, 2)) text_col=0xA6A6B7; //system or hiden?
	if (color!=0xFFFfff)
	{
		itdir = TestBit(attr, 4);
		strcpy(#file_name, file_name_off);
		sprintf(#file_path,"%s%s",#path,file_name_off);
		if (text_col==0xA6A6B7) text_col=0xFFFFFF;
	}
	if (file.selected) text_col=0xFF0000;
	if (Form.width>=480)
	{
		FileShow.start_x = files.x + 23;
		FileShow.font_color = text_col;
		FileShow.area_size_x = Form.width - 380;
		FileShow.text_pointer = file_name_off;
		FileShow.start_y = files.text_y+y;
		PathShow_prepare stdcall(#FileShow);
		PathShow_draw stdcall(#FileShow);
	}
	DrawBar(Form.cwidth-159,y,1,files.line_h,sc.work); //gray line 1
	DrawBar(Form.cwidth-86,y,1,files.line_h,sc.work); //gray line 2
}


void Open_Dir(dword dir_path, redraw){
	int errornum, maxcount;

	if (redraw!=ONLY_SHOW)
	{
		if (ESBYTE[dir_path+1]!='\0') ESBYTE[dir_path+strlen(dir_path)-1] = '\0';
		if (buf) free(buf);
		errornum = GetDir(#buf, #files.count, dir_path, DIRS_NOROOT);
		if (ESBYTE[dir_path+1]!='\0') chrcat(dir_path, '/');
		if (errornum)
		{
			HistoryPath(ADD_NEW_PATH);
			GoBack();
			Write_Error(errornum);
			return;
		}
		maxcount = sizeof(file_mas)/sizeof(dword)-1;
		if (files.count>maxcount) files.count = maxcount;
		if (files.count>0) && (files.current==-1) files.current=0;
	}
	if (files.count!=-1)
	{
		if(!_not_draw)KEdit();
		HistoryPath(ADD_NEW_PATH);
		files.visible = files.h / files.line_h;
		if (files.count < files.visible) files.visible = files.count;
		if (sort_num==1) sorting_arrow_x = Form.width+60/2;
		if (sort_num==2) sorting_arrow_x = Form.width-115;
		if (sort_num==3) sorting_arrow_x = strlen(T_SIZE)*3-30+files.x+files.w;
		WriteText(sorting_arrow_x,45,0x80,sc.work_graph,"\x19");
		if (redraw!=ONLY_SHOW) Sorting();
		if (redraw!=ONLY_OPEN)&&(!_not_draw) List_ReDraw();
	}
	if (files.count==-1) && (redraw!=ONLY_OPEN) {files.visible=files.count=0; if(!_not_draw)List_ReDraw();}
}


inline Sorting()
{
	dword k=0, l=1;
	dword file_off;
	int i;
	if (!strncmp(#path,"/",2)) //do not sort
	{
		for(k=1;k<files.count;k++;) file_mas[k]=k;
		return;
	}
	for (j=files.count-1, file_off=files.count-1*304+buf+32; j>=0; j--, file_off-=304;)  //files | folders
	{
		if (!real_files_names_case) strttl(file_off+40);
		if (TestBit(ESDWORD[file_off],4)) //directory?
		{
			file_mas[k]=j;
			k++;
		}
		else
		{
			file_mas[files.count-l]=j;
			l++;
		}
	}
	//sorting: files first, then folders
	Sort_by_Name(0,k-1);
	if (sort_num==1) Sort_by_Name(k,files.count-1);
	if (sort_num==2) Sort_by_Type(k,files.count-1);
	if (sort_num==3) Sort_by_Size(k,files.count-1);
	//make ".." first item in list
	if (k>0) && (!strncmp(file_mas[0]*304+buf+72,"..",3))
		for(k--; k>0; k--;) if (!strncmp(file_mas[k]*304+buf+72,"..",3)) {file_mas[k]><file_mas[0]; break;}
}


void Del_Form()
{
	dword selected_offset2;
	int cont = 0;
	byte f_count[128];
	int dform_x = files.w - 220 / 2 + files.x;
	if (!strncmp(#file_name,".",2)) || (!strncmp(#file_name,"..",2)) return;
	if (del_active==2)
	{
		if (itdir) ShowMessage(WAIT_DELETING_FOLDER, 0);
	}
	else
	{
		if (!files.count) return;
		DrawPopup(dform_x,160,220,85,1,sc.work,sc.work_graph);
		WriteText(-strlen(T_DELETE_FILE)*3+110+dform_x,175,0x80,sc.work_text,T_DELETE_FILE);
		for (i=0; i<files.count; i++) 
		{
			selected_offset2 = file_mas[i]*304 + buf+32 + 7;
			if (ESBYTE[selected_offset2]) cont++;
		}
		if (cont)
		{
			sprintf(#f_count,"%s%d%s",DEL_MORE_FILES_1,cont,DEL_MORE_FILES_2);
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
		DrawFlatButton(dform_x+27,208,70,20,301,0xFFB6B5,T_YES);
		DrawFlatButton(dform_x+120,208,70,20,302,0xC6DFC6,T_NO);		
		del_active=1;
	}
}

int del_error;
int Del_File2(dword way)
{    
	dword dirbuf, fcount, i, filename;
	int error;
	char del_from[4096];
	if (isdir(way))
	{
		if (error = GetDir(#dirbuf, #fcount, way, DIRS_ONLYREAL)) del_error = error;
		for (i=0; i<fcount; i++)
		{
			if (CheckEvent()==evReDraw) draw_window();
			filename = i*304+dirbuf+72;
			sprintf(#del_from,"%s/%s",way,filename);
			if ( TestBit(ESDWORD[filename-40], 4) )
			{
				Del_File2(#del_from);
			}
			else
			{
				if (error = DeleteFile(#del_from)) del_error = error;
			}
		}
	}
	if (error = DeleteFile(way)) del_error = error;
}


void Del_File(byte dodel)
{   
	byte del_from[4096];
	dword selected_offset2;
	int tst, count, i;
	
	if (dodel==true)
	{
		del_active=2;
		if (itdir) ShowMessage(WAIT_DELETING_FOLDER, 0);
		del_error = 0;
		
		if (selected_count)
		{
   		   for (i=0; i<files.count; i++) 
           {
                selected_offset2 = file_mas[i]*304 + buf+32 + 7;
                if (ESBYTE[selected_offset2]) {
					sprintf(#del_from,"%s%s",#path,file_mas[i]*304+buf+72);
                    Del_File2(#del_from);
                }
            }
		}
		else
		{
		   Del_File2(#file_path);			
		}
		if (del_error) Write_Error(del_error);
 	}
	del_active=0;
	DeleteButton(301);
	DeleteButton(302);
	Open_Dir(#path,WITH_REDRAW);
}


void SelectFile(dword that_file)
{
	files.first=files.current=0;
   	Open_Dir(#path,ONLY_OPEN);
	if (!real_files_names_case) strttl(that_file);
	for (i=files.count-1; i>=0; i--;)
		if (!strcmp(file_mas[i]*304+buf+72,that_file)) break;
	List_Current(i);
	List_ReDraw();
}


void Dir_Up()
{
	char cur_folder[4096];
	i=strlen(#path)-1;
	if (i==0) return;
	path[i]=0x00;
	i = strrchr(#path, '/');
	strcpy(#cur_folder, #path+i);
	path[i]=0x00;
	SelectFile(#cur_folder);
}

void Open(byte rez)
{
	byte temp[4096];
	selected_count = 0;
	if (rez)
	{
		if (!strncmp(#file_name,"..",3)) return;
		strcpy(#temp, #file_path);
		if (path[strlen(#temp)-1]!='/') chrcat(#temp, '/'); //need "/" in the end
		RunProgram("/sys/File Managers/Eolite", #temp);
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
		if (path[strlen(#path)-1]!='/') chrcat(#path, '/'); //need "/" in the end
		files.first=files.current=0;
		Open_Dir(#path,WITH_REDRAW);
	}
}


inline fastcall void GoBack()
{
	char cur_folder[4096];
	strcpy(#cur_folder, GetCurrentFolder());
	if (HistoryPath(GO_BACK)) SelectFile(#cur_folder);
}

void ShowOpenWithDialog()
{
	byte param[4097];
	sprintf(#param,"~%s",#file_path);
	RunProgram("/sys/@open", #param);
}

void NewElement(byte newf)
{
	BDVK element_info;
	byte del_rezult, copy_rezult, info_result;
	if (newf)
	{
		sprintf(#temp,"%s%s",#path,new_file_ed.text);
		info_result = GetFileInfo(#temp, #element_info);
		switch(new_element_active)
		{
			case CREATE_FILE:
				if (info_result==5)
				{
					WriteFile(0, 0, #temp);
					if (EAX)
					{
						Write_Error(EAX);
						ShowMessage(NOT_CREATE_FILE, 150);
					}
				}
				else
				{
					notify(FILE_EXISTS);
				}
				break;
			case CREATE_FOLDER:
				if (info_result==5)
				{
					CreateDir(#temp);
					if (EAX)
					{
						Write_Error(EAX);
						ShowMessage(NOT_CREATE_FOLDER, 150);
					}
				}
				else
				{
					notify(FOLDER_EXISTS);
				}
				break;
			case RENAME_ITEM:
				if (info_result==5)
				{
					if (itdir)
					{
						if (del_rezult = DeleteFile(#file_path))
						{
							Write_Error(del_rezult);
							ShowMessage(T_DEL_ERROR_1, 150);
							return;
						}
						if (CreateDir(#temp)) CreateDir(#file_path);
						Open_Dir(#path,WITH_REDRAW);
						SelectFile(new_file_ed.text);
					}
					else
					{
						if (copy_rezult = CopyFile(#file_path,#temp))
						{
							Write_Error(copy_rezult);
						}
						else
						{
							Del_File(true);
							SelectFile(new_file_ed.text);
						}
					}
				}
				else
				{
					notify(FILE_EXISTS);
				}
		}
		new_element_active = 0;
		Open_Dir(#path,WITH_REDRAW);
		SelectFile(new_file_ed.text);
	}
	new_element_active = 0;
	Open_Dir(#path,WITH_REDRAW);
}

void NewElement_Form(byte crt, dword strng)
{
	int dform_x=files.w-220/2+files.x;
	if (!new_element_active)
	{
		new_element_active = crt;
		strcpy(#new_element_name, strng);
		new_file_ed.size = new_file_ed.pos = strlen(strng);
	}
	DrawPopup(dform_x,160,220,85,1,sc.work,sc.work_graph);
	new_file_ed.left = dform_x+24;
	edit_box_draw  stdcall (#new_file_ed);
	DrawRectangle(new_file_ed.left-1, new_file_ed.top-1, new_file_ed.width+2, 16, 0xFFFfff);
	DrawRectangle(new_file_ed.left-2, new_file_ed.top-2, new_file_ed.width+4, 18, sc.work_graph);
	if (new_element_active==3) DrawFlatButton(dform_x+22,208,85,22,301,0xFFB6B5,T_RENAME);
	else DrawFlatButton(dform_x+27,208,70,22,301,0xFFB6B5,T_CREATE);
	DrawFlatButton(dform_x+120,208,70,22,302,0xC6DFC6,T_CANCEL);
}

void FnProcess(byte N)
{
	switch(N)
	{
		case 1:
			if (!active_about) 
			{
				//SwitchToAnotherThread();
				about_stak = malloc(4096);
				about_window = CreateThread(#about_dialog,about_stak+4092);
				break;
			}
			else
			{
				ActivateWindow(GetProcessSlot(about_window));
			}
			break;
		case 2:
			if (!files.count) break;
			NewElement_Form(RENAME_ITEM, #file_name);
			break;
		case 3:
			if (!itdir) RunProgram("/sys/tinypad", #file_path);
			break;
		case 4:
			if (!itdir) RunProgram("/sys/develop/heed", #file_path);
			break;
		case 5: //refresh cur dir & devs
			Tip(56, T_DEVICES, 55, "-");
			Open_Dir(#path,WITH_REDRAW);
			pause(10);
			LoadIniSettings();
			GetSystemDiscs();
			Open_Dir(#path,WITH_REDRAW);
			DrawLeftPanel();
			break;
		case 6:
			NewElement_Form(CREATE_FOLDER, T_NEW_FOLDER);
			break;
		case 7:
			NewElement_Form(CREATE_FILE, T_NEW_FILE);
			break;
		case 8:
			//SwitchToAnotherThread();
			properties_stak = malloc(8096);
			CreateThread(#properties_dialog, properties_stak+8092);
			break;
		case 10: //F10
			if (!active_settings) 
			{
				//SwitchToAnotherThread();
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

//need to remove these functiones, they are a very old shit :)
dword onTop(dword down,up) {EAX=Form.height-GetSkinHeight()-down-up;}


stop:
