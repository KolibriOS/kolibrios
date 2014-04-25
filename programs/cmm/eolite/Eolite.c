//Leency & Veliant 2008-2013
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
#include "..\lib\lib.obj\box_lib.h"
#include "..\lib\file_system.h"
#include "..\lib\figures.h"
#include "..\lib\encoding.h"
#include "..\lib\list_box.h"
#include "..\lib\copyf.h"
#include "..\lib\random.h"
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
	?define T_DEL_ERROR_1 "Ошибка. Папка не пустая."
	?define WAIT_DELETING_FOLDER "Удаляется папка. Подожите..."
	?define NOT_CREATE_FOLDER "Не удалось создать папку."
	?define NOT_CREATE_FILE "Не удалось создать файл."
	?define ERROR_1 "Ошибка при загрузке библиотеки /rd/1/lib/box_lib.obj"
	?define T_PASTE_WINDOW "Копирую..."
	?define T_PASTE_WINDOW_TEXT "Копируется файл:"
	?define T_CANCEL_PASTE "Копирование прекращено. Папка скопирована не полностью."
	?define T_SELECT_APP_TO_OPEN_WITH "Выберите программу для открытия файла"
#elif LANG_EST
	?define T_FILE "Fail"
	?define T_TYPE "T№№p"
	?define T_SIZE "Suurus"
	?define T_NEW_FOLDER "Uus kataloog"
	?define T_NEW_FILE "Uus fail"
	?define T_DELETE_FILE "Kas sa tahad tїesti kustutada"
	?define T_YES "Jah"
	?define T_NO "Ei"
	?define T_DEL_ERROR_1 "Viga. Kataloog ei ole t№hi."
	?define WAIT_DELETING_FOLDER "Deleting folder. Please, wait..."
	?define NOT_CREATE_FOLDER "Kataloogi ei saa luua."
	?define NOT_CREATE_FILE "Faili ei saa luua."
	?define ERROR_1 "Viga teegi laadimisel /rd/1/lib/box_lib.obj"
	?define T_PASTE_WINDOW "Kopeerin..."
	?define T_PASTE_WINDOW_TEXT "Kopeerin faili:"
	?define T_CANCEL_PASTE "Copy process terminated. Folder copied incompletely."
	?define T_SELECT_APP_TO_OPEN_WITH "Select application to open file"
#else
	?define T_FILE "File"
	?define T_TYPE "Type"
	?define T_SIZE "Size"
	?define T_NEW_FOLDER "New folder"
	?define T_NEW_FILE "New file"
	?define T_DELETE_FILE "Do you really want to delete"
	?define T_YES "Yes"
	?define T_NO "No"
	?define T_DEL_ERROR_1 "Error. Folder isn't empty."
	?define WAIT_DELETING_FOLDER "Deleting folder. Please, wait..."
	?define NOT_CREATE_FOLDER "Folder can not be created."
	?define NOT_CREATE_FILE "File can not be created."
	?define ERROR_1 "Error while loading library /rd/1/lib/box_lib.obj"
	?define T_PASTE_WINDOW "Copying..."
	?define T_PASTE_WINDOW_TEXT "Copying file:"
	?define T_CANCEL_PASTE "Copy process terminated. Folder copied incompletely."
	?define T_SELECT_APP_TO_OPEN_WITH "Select application to open file"
#endif

enum {ONLY_SHOW, WITH_REDRAW, ONLY_OPEN}; //OpenDir

#define TITLE "Eolite File Manager v2.3"
#define ABOUT_TITLE "Eolite v2.3"
dword col_padding, col_selec, col_lpanel;

int toolbar_buttons_x[7]={9,46,85,134,167,203};
char tmp_disk_del_param[3]="d0";
struct path_string { char Item[4096]; };

byte active_about=0;
word about_window;

byte menu_call_mouse=0;

llist files;

byte
	path[4096],
	file_path[4096],
	file_name[256],
	temp[4096];	 
byte
	rename_active=0,
	del_active=0,
	show_dev_name=1,
	drw_ram_disk_space=0,
	real_files_names_case=0,
	sort_num=2,
	itdir;

proc_info Form;
system_colors sc;
mouse m;
int mouse_dd, scroll_used, scroll_size, sorting_arrow_x, kolibrios_drive;
dword buf, off;
dword file_mas[6898];
int j, i;
int action_buf;
int rand_n;

edit_box edit2 = {250,213,80,0xFFFFCC,0x94AECE,0xFFFFCC,0xffffff,0,248,#file_name,#mouse_dd,64,6,6};
PathShow_data PathShow = {0, 17,250, 6, 250, 0, 0, 0x0, 0xFFFfff, #path, #temp, 0};
PathShow_data FileShow = {0, 56,215, 6, 100, 0, 0, 0x0, 0xFFFfff, #file_name, #temp, 0};

#include "include\copy.h"
#include "include\other.h"
#include "include\sorting.h"
#include "include\icons.h"
#include "include\ini.h"
#include "include\left_panel.h"
#include "include\history.h"
#include "include\menu.h"
#include "include\about.h"
#include "include\open_with.h"

void SetAppColors()
{
	sc.work = 0xE4DFE1;
	sc.work_text = 0;
	sc.work_graph  = 0x9098B0; //A0A0B8; //0x819FC5;
	sc.work_button_text = 0x000000;
	col_padding = 0xC8C9C9;
	col_selec   = 0x94AECE;
	col_lpanel  = 0x00699C;
	/*
	sc.get();
	for (i=0; i<=14; i++) col_palette[i] = sc.work;
	toolbar_pal[0]= goto_about_pal[0] = sc.work = sc.work;
	col_lpanel = sc.work_graph;
	for (i=0; i<=99; i++) blue_hl_pal[i] = sc.work_graph;
	*/
}

void main() 
{
	word key, id, can_show, can_select, m_selected;
	randomize();
	rand_n = random(40);

	files.line_h=18;
	mem_Init();
	if (load_dll2(boxlib, #box_lib_init,0)!=0) notify(ERROR_1);
	SystemDiscsGet();
	GetIni(1);
	SetAppColors();
	if (param)
	{
		strcpy(#path, #param);
		if (path[strlen(#path)-1]!='/') chrcat(#path, '/'); //add "/" to the end of the string
	}
	else
	{
		strcpy(#path, "/rd/1/");		
	}
	Open_Dir(#path,ONLY_OPEN);
	SetEventMask(0x27);
	loop() switch(WaitEvent())
	{
		case evMouse:
			if (del_active) || (!CheckActiveProcess(Form.ID)) || (Form.status_window>2) break;
			if (rename_active) { edit_box_mouse stdcall(#edit2); break; }
			
			m.get();

			if (files.MouseOver(m.x, m.y)) && (!can_select)
			{
				m_selected = m.y - files.y / files.line_h;
				if (m.lkm) can_select = 1;
				if (m.pkm)
				{
					can_show = 1;
					if (m.y - files.y / files.line_h != files.current) can_select = 1;
				}
			}

			//select/open file {
			if (!m.lkm) && (!m.pkm) && (can_select)
			{
				can_select = 0;
				if (m.y>=files.y)
				{
					id = m.y - files.y / files.line_h;
					if (id!=m_selected)
					{
						can_show=0;
						break;
					}
					if (files.current!=id)
					{
						if (id<files.visible) List_Current(id-files.current);
					}
					else
						Open();
				}
			};
			// } select/open file

			//file menu {
			if (!m.pkm) && (!m.lkm) && (can_show)
			{
				can_show = 0;
				menu_call_mouse = 1;
				if (m.y>=files.y)
				{
					SwitchToAnotherThread();
					CreateThread(#FileMenu,#menu_stak+4092);
				}
				break;
			}
			// } file menu

			if (m.vert)
			{
				if (files.MouseScroll(m.vert)) List_ReDraw();
				break;
			}

			if (m.x>=Form.width-26) && (m.x<=Form.width-6) && (m.y>40) && (m.y<files.y)
			{
				IF (m.lkm==1) DrawRectangle3D(onLeft(26,0),41,14,14,0xC7C7C7,0xFFFFFF);
				WHILE (m.lkm==1) && (files.first>0)
				{
					pause(8);
					files.first--;
					List_ReDraw();
					m.get();
				}
				DrawRectangle3D(onLeft(26,0),41,14,14,0xFFFFFF,0xC7C7C7);
			}

			if (m.x>=Form.width-26) && (m.x<=Form.width-6) && (m.y>onTop(22,0)+1) && (m.y<onTop(22,0)+16)
			{
				IF (m.lkm==1) DrawRectangle3D(onLeft(26,0),onTop(21,0),14,14,0xC7C7C7,0xFFFFFF);
				while (m.lkm==1) && (files.first<files.count-files.visible)
				{
					pause(8);
					files.first++;
					List_ReDraw();
					m.get();
				}
				DrawRectangle3D(onLeft(26,0),onTop(21,0),14,14,0xFFFFFF,0xC7C7C7);
			}

			//Scrooll
			if (!m.lkm) && (scroll_used) { scroll_used=NULL; Scroll(); }
			if (m.x>=Form.width-26) && (m.x<=Form.width-6) && (m.y>56) && (m.y<Form.height) && (m.lkm) && (!scroll_used) {scroll_used=1;Scroll();}
			
			if (scroll_used)
			{
				IF (scroll_size/2+files.y>m.y) || (m.y<0) || (m.y>4000) m.y=scroll_size/2+files.y; //anee eo?ni? iaa ieiii
				id=files.first;
				j= scroll_size/2;
				files.first = m.y -j -files.y * files.count;
				files.first /= onTop(22,files.y);
				IF (files.visible+files.first>files.count) files.first=files.count-files.visible;
				IF (id!=files.first) List_ReDraw();
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
			if (rename_active) break;
			if (del_active)
			{
				IF (id==301) || (id==302) Del_File(302-id);
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
				case 23: //up!
						Dir_Up();
						break;
				case 24: //cut
						Copy(#file_path, CUT);
						break;
				case 25: //copy
						Copy(#file_path, NOCUT);
						break;
				case 26: //paste
						CreateThread(#Paste,#copy_stak+4092);
						break;
				case 31...33: //sort
						IF(sort_num==1) DrawFilledBar(sorting_arrow_x,42,6,10);
						IF(sort_num==2) DrawFilledBar(sorting_arrow_x,42,6,10);
						IF(sort_num==3) DrawFilledBar(sorting_arrow_x,42,6,10);
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
				case 130...160:
						tmp_disk_del_param[1]=disk_list[id-130].Item[4];
						RunProgram("/sys/tmpdisk", #tmp_disk_del_param);
						pause(10);
						SystemDiscsGet();
						Open_Dir(#path,WITH_REDRAW);
						DrawLeftPanel();
						break;
			}
			break;
//Key pressed-----------------------------------------------------------------------------
		case evKey:
			key = GetKey();
			if (Form.status_window>2) break;
			IF (del_active)
			{
				IF (key==013) Del_File(true);
				IF (key==027) Del_File(false);
				break;
			}
			IF (edit2.flags!=64) && (key!=13) && (key!=27)
			{
				EAX=key<<8;
				edit_box_key stdcall (#edit2);
				break;
			}
			switch (key)
			{
					case 209...217:
							id=key-110;
							IF (id-100>=disc_num) break;
							GOTO DEVICE_MARK;
					case 008:
							//GoBack();
							Dir_Up();
							break; 
					case 004: //Ctrl+D set as bg
							strcpy(#temp, "\\S__");
							strcat(#temp, #file_path);
							RunProgram("/sys/media/kiv", #temp);
							break;
					case 014: //Ctrl+N new window
							IF (Form.left==98) MoveSize(Form.left-20,Form.top-20,OLD,OLD);
							RunProgram("/sys/File Managers/Eolite", #path);
							break; 
					case 024: //Ctrl+X
							Copy(#file_path, CUT);
							break;
					case 003: //Ctrl+C
							Copy(#file_path, NOCUT);
							break;
					case 022: //Ctrl+V
							CreateThread(#Paste,#copy_stak+4092);
							break;
					case 027: //Esc
							IF (rename_active==1) ReName(false);
							break;
					case 013: //Enter
							IF (rename_active==1) {ReName(true); break;}
							Open();
							break; 
					case 074: //menu
							menu_call_mouse=0;
							SwitchToAnotherThread();
							CreateThread(#FileMenu,#menu_stak+4092);
							break;
					case 173: //Ctrl+Enter
							if (!itdir) ShowOpenWithDialog();
							break;
					case 178: //up
							List_Current(-1);
							break;
					case 177: //down
							List_Current(1);
							break;
					case 180: //home
							if (files.KeyHome()) List_ReDraw();
							break;
					case 181: //end
							if (files.KeyEnd()) List_ReDraw();
							break;
					case 183: //Page Down
							List_Current(files.visible-1);
							break;
					case 184: //Page Up
							List_Current(-files.visible+1);
							break;
					case 182: //del
							Del_Form();
							break;
					case 185: //ins
							add_to_copy_active=1;
							add_to_copy(#file_path);
							notify("'Eolite\nFile was added to copy queue' -tI");
							break;
					case 050...059: //F1-F10
							FnProcess(key-49);
							break; 
					default:    
							for (i=files.current+files.first+1; i<files.count; i++)
							{
								strcpy(#temp, file_mas[i]*304+buf+72);
								IF (temp[0]==key) || (temp[0]==key-32)
								{
									List_Current(i-files.current-files.first);
									break;
								}
							}
			}                         
			break;
		case evReDraw:
			draw_window();
			if (action_buf) { menu_action(action_buf); action_buf=0;}
	}
}



void menu_action(dword id)
{
	if (id==COPY_PASTE_END)
	{
		FnProcess(5);
		SelectFile(#copy_to+strrchr(#copy_to,'/'));
	}
	if (id==100) Open();
	if (id==201) ShowOpenWithDialog();
	if (id==202) FnProcess(3); //F3
	if (id==203) FnProcess(4); //F4
	if (id==104) Copy(#file_path, NOCUT);
	if (id==105) Copy(#file_path, CUT);
	if (id==106) CreateThread(#Paste,#copy_stak+4092);
	if (id==207) FnProcess(2);
	if (id==108) Del_Form();
	if (id==109) FnProcess(5);
}


void draw_window()
{
	DefineAndDrawWindow(GetScreenWidth()-550/4+rand_n,rand_n+30,550,500,0x73,sc.work,TITLE,0);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return;
	files.SetSizes(192, 57, onLeft(192,27), onTop(57,6), disc_num*16+195,files.line_h);
	if (Form.height < files.min_h) MoveSize(OLD,OLD,OLD,files.min_h);
	if (Form.width<480) MoveSize(OLD,OLD,480,OLD);
	GetProcessInfo(#Form, SelfInfo); //if win_size changed

	PutPaletteImage(#toolbar,246,34,0,0,8,#toolbar_pal);
	DrawBar(127, 8, 1, 25, sc.work_graph);
	for (j=0; j<3; j++) DefineButton(toolbar_buttons_x[j]+2,5+2,31-5,29-5,21+j+BT_HIDE,sc.work);
	for (j=3; j<6; j++) DefineButton(toolbar_buttons_x[j],5,31,29,21+j+BT_HIDE,sc.work);
	DrawBar(246,0,onLeft(246,60),12, sc.work); //upper editbox
	DrawBar(246,29,onLeft(246,60),5,sc.work);  //under editbox
	DrawRectangle(246,12,onLeft(66,246),16,sc.work_graph);
	DefineButton(onLeft(34,0),6,27,28,51+BT_HIDE+BT_NOFRAME,0); //about
	PutPaletteImage(#goto_about,56,34,Form.width-65,0,8,#goto_about_pal);
	//main rectangles
	DrawRectangle(1,40,Form.cwidth-3,onTop(46,0),sc.work_graph);
	DrawRectangle(0,39,Form.cwidth-1,onTop(44,0),col_palette[4]); //bg
	for (i=0; i<5; i++) DrawBar(0, 34+i, Form.cwidth, 1, col_palette[8-i]);	
	DrawLeftPanel();
	//ListBox
	DrawFlatButton(files.x,40,onLeft(files.x,168),16,31,sc.work,T_FILE);
	DrawFlatButton(onLeft(168,0),40,73,16,32,sc.work,T_TYPE);
	DrawFlatButton(onLeft(95,0),40,68,16,33,sc.work,T_SIZE);
	DrawBar(files.x+files.w,files.y,1,onTop(22,files.y),sc.work_graph); //line to the left from the scroll
	DrawFlatButton(files.x+files.w,40,16,16,0,sc.work,"\x18");
	DrawFlatButton(files.x+files.w,onTop(22,0),16,16,0,sc.work,"\x19");
	Open_Dir(#path,ONLY_SHOW);
	if (del_active) Del_Form();
	//if (itdir) ShowMessage(WAIT_DELETING_FOLDER, 0);
	if (rename_active) FnProcess(2);
}


void KEdit()
{
	if (Form.width<480) return;
	PathShow.area_size_x = Form.cwidth-306;
	DrawBar(PathShow.start_x-3, PathShow.start_y-4, PathShow.area_size_x+2, 15, 0xFFFfff);
	PathShow_prepare stdcall(#PathShow);
	PathShow_draw stdcall(#PathShow);
}


void List_Current(int cur)
{
	if (cur<=0) //up
	{
		IF (files.first==0) && (files.current<=0) return;
		IF (-cur-1<files.current)
		{
			Line_ReDraw(0xFFFFFF, files.current);
			files.current+=cur;
			Line_ReDraw(col_selec, files.current);
			return;
		}
		ELSE
		{
			IF (-cur<files.first) files.first+=cur; ELSE files.first=0;
			files.current=0;
			List_ReDraw();
			return;
		}
	}
	else  //down
	{
		IF (files.first==files.count-files.visible) && (files.current==files.visible-1) return;
		IF (files.visible-files.current>cur)
		{
			Line_ReDraw(0xFFFFFF, files.current);
			files.current+=cur;
			Line_ReDraw(col_selec, files.current);
			return;
		}
		else
		{
			IF(files.first+files.current+cur>=files.count)
			{
				files.first=files.count-files.visible;
				files.current=cur-files.first+files.current;
				}
			ELSE
			{
				files.first+=cur+files.current-files.visible+1;
				files.current=files.visible-1;
			}
			
			IF (files.current<0) || (files.current>files.visible)
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
	      name_len=0,
	      attr,
	      ext1,
	      y=filenum*files.line_h+files.y;
	if (filenum==-1) return;
	DrawBar(files.x,y,3,files.line_h,color); 
	DrawBar(files.x+19,y,files.w-19,files.line_h,color);
	DrawBar(files.x+3,y+17,16,1,color);
	if (files.line_h>18) DrawBar(files.x+3,y+18,16,files.line_h-18,color);
	if (files.line_h>15) DrawBar(files.x+3,y,16,files.line_h-15,color); 

	off=file_mas[filenum+files.first]*304 + buf+72;
	attr = ESDWORD[off - 40];

	if (! TestBit(attr, 4) ) //file or folder?
	{	
		Put_icon(off+_strrchr(off,'.'), files.x+3, files.line_h/2-7+y, color, 0);
		WriteText(7-strlen(ConvertSize(ESDWORD[off-8]))*6+onLeft(75,0),files.line_h-6/2+y,0x80,0,ConvertSize(ESDWORD[off-8])); //size
	}
	else
	{
		if (strcmp("..",off)==0) ext1=".."; else ext1="<DIR>";
		Put_icon(ext1, files.x+3, files.line_h/2-7+y, color, 0);		
	}

	if ( TestBit(attr, 1) ) || ( TestBit(attr, 2) ) text_col=0xA6A6B7; //system or hiden?
	if (color!=0xFFFfff)
	{
		itdir = TestBit(attr, 4);
		strcpy(#file_name, off);
		strcpy(#file_path, #path);
		strcat(#file_path, #file_name);
		if (text_col==0xA6A6B7) text_col=0xFFFFFF;
	}
	if (Form.width>=480)
	{
		FileShow.start_x = files.x + 23;
		FileShow.font_color = text_col;
		FileShow.area_size_x = Form.width - 380;
		FileShow.text_pointer = off;
		FileShow.start_y = files.line_h/2-3+y;
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
		if (drw_ram_disk_space) DrawRamDiskSpace();
	}
	if (files.count!=-1)
	{
		KEdit();
		HistoryPath(ADD_NEW_PATH);
		files.visible = files.h / files.line_h;
		IF (files.count < files.visible) files.visible = files.count;
		IF (sort_num==1) sorting_arrow_x = Form.width+60/2;
		IF (sort_num==2) sorting_arrow_x = Form.width-115;
		IF (sort_num==3) sorting_arrow_x = strlen(T_SIZE)*3-30+files.x+files.w;
		WriteText(sorting_arrow_x,45,0x80,sc.work_graph,"\x19");
		IF (redraw!=ONLY_SHOW) Sorting();
		IF (redraw!=ONLY_OPEN) List_ReDraw();
	}
	IF (files.count==-1) && (redraw!=ONLY_OPEN) {files.visible=files.count=0; List_ReDraw();}
}


inline Sorting()
{
	dword k=0, l=1;
	int i;
	if (!strcmp(#path,"/")) //do not sort
	{
		FOR(k=1;k<files.count;k++;) file_mas[k]=k;
		return;
	}
	FOR (j=files.count-1, off=files.count-1*304+buf+32; j>=0; j--, off-=304;)  //files | folders
	{
		if (!real_files_names_case) strttl(off+40);
		if (TestBit(ESDWORD[off],4)) //directory?
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
	IF (sort_num==1) Sort_by_Name(k,files.count-1);
	IF (sort_num==2) Sort_by_Type(k,files.count-1);
	IF (sort_num==3) Sort_by_Size(k,files.count-1);
	//".." should be first
	IF (k>0) && (strcmp(file_mas[0]*304+buf+72,"..")!=0)
		FOR(k--; k>0; k--;) IF (!strcmp(file_mas[k]*304+buf+72,"..")) {file_mas[k]><file_mas[0]; break;}
}


void Del_Form()
{
	int dform_x=files.w-220/2+files.x;
	if (strcmp(#file_name,".")==0) || (strcmp(#file_name,"..")==0) return;
	if (del_active==2)
	{
		if (itdir) ShowMessage(WAIT_DELETING_FOLDER, 0);
	}
	else
	{
		if (!files.count) return;
		DrawPopup(dform_x,160,220,80,1,sc.work,sc.work_graph);
		WriteText(-strlen(T_DELETE_FILE)*3+110+dform_x,175,0x80,sc.work_text,T_DELETE_FILE);
		IF (strlen(#file_name)<28) 
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
			strcpy(#del_from, way);
			chrcat(#del_from, '/');
			strcat(#del_from, filename);
			if ( TestBit(ESDWORD[filename-40], 4) )
				Del_File2(#del_from);
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
	if (dodel==true)
	{
		del_active=2;
		if (itdir) ShowMessage(WAIT_DELETING_FOLDER, 0);
		del_error = 0;
		Del_File2(#file_path);
		if (del_error) Write_Error(del_error);
 	}
	del_active=0;
	DeleteButton(301);
	DeleteButton(302);
	Open_Dir(#path,WITH_REDRAW);
}


void ReName(byte rename)
{
	int del_rezult, copy_rezult;
	char edit_name[256];
	rename_active=0;
	edit2.flags=64;
	if (rename==true)
	{
		strcpy(#temp, #path);
		strcpy(#edit_name, #file_name); //save edit name to select it later
		strcat(#temp, #file_name);
		if (strcmp(#file_path,#temp)!=0) && (file_name)
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
			SelectFile(#edit_name);
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
				SelectFile(#edit_name);
			}
		}
	}
	Line_ReDraw(col_selec,files.current);
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

void Open()
{
	if (!files.count) return;
	if (!itdir)
	{
		GetIni(0);
	} 
	else
	{
		if (!strcmp(#file_name,"..")) { Dir_Up(); return; }
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

void FnProcess(char N)
{
	switch(N)
	{
		case 1:
			if (!active_about) 
			{
				SwitchToAnotherThread();
				about_window=CreateThread(#about_dialog,#about_stak+4092);
				break;
			}
			else
			{
				ActivateWindow(GetProcessSlot(about_window));
			}
			break;
		case 2:
			if (!files.count) break;
			edit2.flags = 100000000000010b; //set active
			edit2.left = files.x + 21;
			edit2.width = files.w - 26;
			edit2.top=files.current*files.line_h+59;
			edit2.size=edit2.pos=strlen(#file_name);
			edit_box_draw  stdcall (#edit2);
			DrawBar(edit2.left,files.current*files.line_h+58,edit2.width+1,1,0xFFFFCC); //bg
			rename_active=1;
			break;
		case 3:
			IF (!itdir) RunProgram("/sys/tinypad", #file_path);
			break;
		case 4:
			IF (!itdir) RunProgram("/sys/develop/heed", #file_path);
			break;
		case 5: //refresh cur dir & devs
			Tip(56, T_DEVICES, 55, "-");
			Open_Dir(#path,WITH_REDRAW);
			pause(10);
			GetIni(1);
			SystemDiscsGet();
			Open_Dir(#path,WITH_REDRAW);
			DrawLeftPanel();
			break;
		case 6:
			strcpy(#temp, #path);
			strcat(#temp, T_NEW_FOLDER);
			CreateDir(#temp);
			if (!EAX){
				SelectFile(T_NEW_FOLDER);
				FnProcess(2);
			}
			else
			{
				Write_Error(EAX);
				ShowMessage(NOT_CREATE_FOLDER, 150);
			}
			break;
		case 7:
			strcpy(#temp, #path);
			strcat(#temp, T_NEW_FILE);
			WriteFile(0, 0, #temp);
			if (!EAX){
				SelectFile(T_NEW_FILE);
				FnProcess(2);
			}
			else
			{
				Write_Error(EAX);
				ShowMessage(NOT_CREATE_FILE, 150);
			}
			break;
		case 10: //F10
			RunProgram(EDITOR_PATH, abspath("Eolite.ini"));
			break;
	}
}


stop:

char menu_stak[4096];
char copy_stak[4096];
char open_with_stak[4096];
char about_stak[4096];