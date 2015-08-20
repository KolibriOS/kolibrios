//Leency, Veliant, Punk_Joker & KolibriOS Team 2008-2015
//GNU GPL licence.

#ifndef AUTOBUILD
#include "lang.h--"
#endif

//libraries
#define MEMSIZE 530000
#include "..\lib\clipboard.h"
#include "..\lib\strings.h"
#include "..\lib\mem.h"
#include "..\lib\file_system.h"
#include "..\lib\gui.h"
#include "..\lib\list_box.h"
#include "..\lib\copyf.h"
#include "..\lib\random.h"
#include "..\lib\font.h"
//obj
#include "..\lib\obj\libini.h"
#include "..\lib\obj\box_lib.h"
//images
#include "imgs\toolbar.txt"
#include "imgs\left_p.txt"
#include "imgs\icons.txt"

enum {ONLY_SHOW, WITH_REDRAW, ONLY_OPEN}; //OpenDir
enum {CREATE_FILE=1, CREATE_FOLDER, RENAME_ITEM }; //NewElement

dword col_padding, col_selec, col_lpanel;

int toolbar_buttons_x[7]={9,46,85,134,167,203};
struct path_string { char Item[4096]; };

byte active_about=0;
word about_window;
word settings_window;
byte active_settings=0;
dword _not_draw = false;
byte menu_call_mouse=0;

byte del_active=0;
byte new_element_active=0;

llist files, files_active, files_inactive;

byte list_full_redraw;

dword buf;
dword file_mas[6898];
int selected_count;


byte
	path[4096],
	file_path[4096],
	file_name[256],
	new_element_name[256],
	temp[4096],
	itdir;

char active_path[4096], inactive_path[4096];

dword eolite_ini_path;

char scroll_used=false;

dword menu_stak,about_stak,properties_stak,settings_stak,copy_stak;

proc_info Form;
int mouse_dd, sc_slider_h, kolibrios_drive;
int j, i;
int action_buf;
int rand_n;
byte CMD_REFRESH;

//struct t_settings {
byte sort_num=2,
	show_dev_name=true,
	real_files_names_case=false,
	info_after_copy=false,
	two_panels=false,
	active_panel=1;
//} settings;

edit_box new_file_ed = {171,213,180,0xFFFFFF,0x94AECE,0xFFFFFF,0xFFFFFF,0,248,#new_element_name,#mouse_dd,100000000000010b,6,0};
PathShow_data PathShow = {0, 17,250, 6, 250, 0, 0, 0x0, 0xFFFfff, #path, #temp, 0};
PathShow_data FileShow = {0, 56,215, 6, 100, 0, 0, 0x0, 0xFFFfff, #file_name, #temp, 0};
byte cmd_free=0;
#include "include\translations.h"
#include "include\settings.h"
#include "include\copy.h"
#include "include\gui.h"
#include "include\sorting.h"
#include "include\icons.h"
#include "include\left_panel.h"
#include "include\history.h"
#include "include\menu.h"
#include "include\about.h"
#include "include\properties.h"


void main() 
{
	word id;
	signed x_old, y_old, dif_x, dif_y, adif_x, adif_y;
	char can_show, can_select, stats;
	dword selected_offset;
	dword IPC_LEN,IPC_ID;
	char IPC_BUF[10];
	rand_n = random(40);

	load_dll(boxlib, #box_lib_init,0);
    load_dll(libini, #lib_init,1);
	eolite_ini_path = abspath("Eolite.ini");
	LoadIniSettings();
	GetSystemDiscs();
	SetAppColors();
	if (param)
	{
		strcpy(#path, #param);
		if (path[strlen(#path)-1]!='/') chrcat(#path, '/'); //add "/" to the end of the string
	}
	else
	{
		strlcpy(#path, "/rd/1/", 6);		
	}
	Open_Dir(#path,ONLY_OPEN);
	strcpy(#inactive_path, #path);
	llist_copy(#files_inactive, #files);
	font.no_bg_copy = true;
	font.load("/sys/font/Tahoma.kf");
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
				
				mouse.get();
				
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
					
					if (adif_x>adif_y)
					{
						if (dif_x > 150)
						{
							if (HistoryPath(GO_FORWARD))
								{
									files.KeyHome();
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
				if (files.MouseOver(mouse.x, mouse.y))&&((mouse.down|mouse.dblclick|mouse.up))
				{
					//select/open file {
					if (mouse.key&MOUSE_LEFT)&&((mouse.down)||(mouse.dblclick))
					{
						if (mouse.y>=files.y)//&&(mouse.click)
						{
							id = mouse.y - files.y / files.line_h + files.first;
							if (files.current!=id)
							{
								mouse.clearTime();
								if(!mouse.up)&&(id-files.first<files.visible) {
									files.current = id;
									List_ReDraw();
								}
							}
							else IF(mouse.dblclick)Open(0);
						}
					}
					// } select/open file
					else
					//file menu {
					if (mouse.key&MOUSE_RIGHT)&&(mouse.up)
					{
						menu_call_mouse = 1;
						
						if (files.MouseOver(mouse.x, mouse.y))
						{
							files.current = mouse.y - files.y / files.line_h + files.first;
							List_ReDraw();
							menu_stak = malloc(4096);
							CreateThread(#FileMenu,menu_stak+4092);	
						}
						break;
					}
					// } file menu
				}

				if (mouse.vert)
				{
					if (files.MouseScroll(mouse.vert)) List_ReDraw();
					break;
				}

				if (mouse.x>=files.x+files.w) && (mouse.x<=files.x+files.w+16) && (mouse.y>files.y-17) && (mouse.y<files.y)
				{
					if (mouse.lkm==1) DrawRectangle3D(files.x+files.w+1,files.y-16,14,14,0xC7C7C7,0xFFFFFF);
					WHILE (mouse.lkm==1) && (files.first>0)
					{
						pause(8);
						files.first--;
						List_ReDraw();
						mouse.get();
					}
					DrawRectangle3D(files.x+files.w+1,files.y-16,14,14,0xFFFFFF,0xC7C7C7);
				}

				if (mouse.x>=files.x+files.w) && (mouse.x<=files.x+files.w+16) && (mouse.y>files.y+files.h-16) && (mouse.y<files.y+files.h)
				{
					if (mouse.lkm==1) DrawRectangle3D(files.x+files.w+1,files.y+files.h-15,14,14,0xC7C7C7,0xFFFFFF);
					while (mouse.lkm==1) && (files.first<files.count-files.visible)
					{
						pause(8);
						files.first++;
						List_ReDraw();
						mouse.get();
					}
					DrawRectangle3D(files.x+files.w+1,files.y+files.h-15,14,14,0xFFFFFF,0xC7C7C7);
				}

				//Scrooll
				if (!mouse.lkm) && (scroll_used) { scroll_used=false; Scroll(); }
				if (mouse.x>=files.x+files.w) && (mouse.x<=files.x+files.w+18) && (mouse.y>files.y) && (mouse.y<files.y+files.h-18) && (mouse.lkm) && (!scroll_used) {scroll_used=true; Scroll();}
				
				if (scroll_used)
				{
					if (sc_slider_h/2+files.y>mouse.y) || (mouse.y<0) || (mouse.y>4000) mouse.y=sc_slider_h/2+files.y; //anee eo?ni? iaa ieiii
					id = files.first;
					files.first = -sc_slider_h / 2 + mouse.y -j -files.y * files.count;
					files.first /= files.h - 18;
					if (files.visible+files.first>files.count) files.first=files.count-files.visible;
					if (files.first<0) files.first=0;
					if (id!=files.first) List_ReDraw();
					break;
				}


				if (two_panels) && (mouse.y > files.y) && (mouse.down) {
					if (mouse.x<Form.cwidth/2)
					{
						if (active_panel!=1)
						{
							active_panel = 1;
							ChangeActivePanel();
						}
					}
					else
					{
						if (active_panel!=2)
						{
							active_panel = 2;
							ChangeActivePanel();
						}
					}
				}
				break;  
	//Button pressed-----------------------------------------------------------------------------
			case evButton:
				id=GetButtonID();
				if (id==1)
				{
					KillProcess(about_window);
					SaveIniSettings();
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
								files.KeyHome();
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
					case 31...33: //sorting
							sort_num = id - 30;
							DrawList();
							Open_Dir(#path,WITH_REDRAW);
							break;
					case 50...60: //Actions
							FnProcess(id-50);
							break;
					case 100...120:
						ClickOnDisk(id-100);
						break;
				}
				break;
	//Key pressed-----------------------------------------------------------------------------
			case evKey:
				GetKeys();

				if (Form.status_window>2) break;
				if (del_active)
				{
					if (key_scancode == SCAN_CODE_ENTER) Del_File(true);
					if (key_scancode == SCAN_CODE_ESC) Del_File(false);
					break;
				}
				if (new_element_active)
				{
					if (key_scancode == SCAN_CODE_ESC) NewElement(0);
					if (key_scancode == SCAN_CODE_ENTER) NewElement(1);
					EAX= key_ascii << 8;
					edit_box_key stdcall (#new_file_ed);
					break;
				}
				if (files.ProcessKey(key_scancode))
				{
					List_ReDraw();
					break;
				}

				if (TestBit(key_modifier, 2))
				{
					switch(key_scancode)
					{
						case 059...068:
								key_scancode -= 59;
								if (key_scancode<disc_num)
								{
									if (!two_panels)
									{
										DrawRectangle(17,key_scancode*16+74,159,16, 0); //display click
										pause(7);										
									}
									ClickOnDisk(key_scancode);
								}
								break;
						case 45:  //Ctrl+X
								Copy(#file_path, CUT);
								break;						
						case 46:  //Ctrl+C
								Copy(#file_path, NOCUT);
								break;
						case 47:  //Ctrl+V
								Paste();
								break;
						case 032: //Ctrl+D - set as bg
								strncpy(#temp, "\\S__",4);
								strcat(#temp, #file_path);
								RunProgram("/sys/media/kiv", #temp);
								break;
						case 049: //Ctrl+N - create new window
								if (Form.left==98) MoveSize(Form.left-20,Form.top-20,OLD,OLD);
								RunProgram("/sys/File Managers/Eolite", #path);
								break; 
						case 028: //Ctrl+Enter
								if (!itdir) ShowOpenWithDialog();
								else Open(1);
								break;
						case 030: //Ctrl+A - select all files
								for (i=0; i<files.count; i++) 
								{
									selected_offset = file_mas[i]*304 + buf+32 + 7;
									if (!i) if (!strncmp(selected_offset+33, "..", 2)) continue; //do not selec ".." directory
									ESBYTE[selected_offset] = 1;
									selected_count++;
								}
								List_ReDraw();
								break;
						case 022: //Ctrl+U - unselect all files
								for (i=0; i<files.count; i++) 
								{
									selected_offset = file_mas[i]*304 + buf+32 + 7;
									ESBYTE[selected_offset] = 0;
								}
								selected_count = 0;
								List_ReDraw();
								break;
					}
					break;
				}

				switch (key_scancode)
				{
						case SCAN_CODE_BS:
								//GoBack();
								Dir_Up();
								break; 
						case SCAN_CODE_ENTER:
								Open(0);
								break; 
						case SCAN_CODE_TAB:
								if (!two_panels) break;
								if (active_panel==1) active_panel=2; else active_panel=1;
								ChangeActivePanel();
								break;
						case 093: //menu
								menu_call_mouse=0;
								menu_stak = malloc(4096);
								CreateThread(#FileMenu,menu_stak+4092);
								break;
						case SCAN_CODE_DEL:
								Del_Form();
								break;
						case SCAN_CODE_INS:
								selected_offset = file_mas[files.current]*304 + buf+32 + 7;
								if (files.current==0) && (!strncmp(selected_offset+33, "..", 2)) goto _INSERT_END; //do not selec ".." directory
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
								_INSERT_END:
								if (files.KeyDown()) List_ReDraw();
								break;
						case 059...068: //F1-F10
								FnProcess(key_scancode-58);
								break; 
						default:    
								for (i=files.current+1; i<files.count; i++)
								{
									strcpy(#temp, file_mas[i]*304+buf+72);
									if (temp[0]==key_ascii) || (temp[0]==key_ascii-32)
									{
										files.current = i - 1;
										files.KeyDown();
										List_ReDraw();
										break;
									}
								}
				}                         
			break;
			case evIPC:
			case evReDraw:
				draw_window();
				if (action_buf) 
				{
					if (action_buf==COPY_PASTE_END)
					{
						FnProcess(5);
						SelectFileByName(#copy_to+strrchr(#copy_to,'/'));
					}
					if (action_buf==100) Open(0);
					if (action_buf==201) ShowOpenWithDialog();
					if (action_buf==202) FnProcess(3); //F3
					if (action_buf==203) FnProcess(4); //F4
					if (action_buf==104) Copy(#file_path, NOCUT);
					if (action_buf==105) Copy(#file_path, CUT);
					if (action_buf==106) Paste();
					if (action_buf==207) FnProcess(2);
					if (action_buf==108) Del_Form();
					if (action_buf==109) FnProcess(5);
					if (action_buf==110) FnProcess(8);
					action_buf=0;
				}
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

void draw_window()
{
	DefineAndDrawWindow(WinX+rand_n,WinY+rand_n,WinW,WinH,0x73,system.color.work,TITLE,0);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return;
	if (Form.height < 350) MoveSize(OLD,OLD,OLD,350);
	if (Form.width  < 480) MoveSize(OLD,OLD,480,OLD);
	GetProcessInfo(#Form, SelfInfo); //if win_size changed
	PutPaletteImage(#toolbar,246,34,0,0,8,#toolbar_pal);
	DrawBar(127, 8, 1, 25, system.color.work_graph);
	for (j=0; j<3; j++) DefineButton(toolbar_buttons_x[j]+2,5+2,31-5,29-5,21+j+BT_HIDE,system.color.work);
	for (j=3; j<6; j++) DefineButton(toolbar_buttons_x[j],5,31,29,21+j+BT_HIDE,system.color.work);
	DrawBar(246,0,Form.cwidth - 297,12, system.color.work); //upper editbox
	DrawBar(246,29,Form.cwidth - 297,5,system.color.work);  //under editbox
	DrawRectangle(246,12,Form.cwidth - 303,16,system.color.work_graph);
	DefineButton(Form.cwidth - 32,6,27,28,51+BT_HIDE+BT_NOFRAME,0); //about
	PutPaletteImage(#goto_about,56,34,Form.width-65,0,8,#goto_about_pal);
	//main rectangles
	DrawRectangle(1,40,Form.cwidth-3,Form.cheight - 42,system.color.work_graph);
	DrawRectangle(0,39,Form.cwidth-1,Form.cheight - 40,col_palette[4]); //bg
	for (i=0; i<5; i++) DrawBar(0, 34+i, Form.cwidth, 1, col_palette[8-i]);
	llist_copy(#files_active, #files);
	strcpy(#active_path, #path);
	DrawFilePanels();
	if (del_active) Del_Form();
	if (new_element_active) NewElement_Form(new_element_active, #new_element_name);
}

void DrawList() 
{
	word sorting_arrow_x;
	DrawFlatButton(files.x, files.y-17, files.w - 141,16,31,system.color.work,T_FILE);
	DrawFlatButton(files.x + files.w - 141, files.y-17,73,16,32,system.color.work,T_TYPE);
	DrawFlatButton(files.x + files.w -  68, files.y-17,68,16,33,system.color.work,T_SIZE);
	DrawFlatButton(files.x + files.w,       files.y-17,16,16, 0,system.color.work,"\x18");
	DrawFlatButton(files.x + files.w,files.y+files.h-16,16,16,0,system.color.work,"\x19");
	if (sort_num==1) sorting_arrow_x = files.w - 141 / 2 + files.x + 18;
	if (sort_num==2) sorting_arrow_x = files.x + files.w - 90;
	if (sort_num==3) sorting_arrow_x = strlen(T_SIZE)*3-30+files.x+files.w;
	WriteText(sorting_arrow_x,files.y-12,0x80,system.color.work_graph,"\x19");
	DrawBar(files.x+files.w,files.y,1,files.h,system.color.work_graph);
	if (two_panels) && (files.x<5) DrawBar(files.x+files.w+16,files.y,1,files.h,system.color.work_graph);	
}

void DrawFilePanels()
{
	if (!two_panels)
	{
		DrawDeviceAndActionsLeftPanel();
		files.SetSizes(192, 57, Form.cwidth - 210, Form.cheight - 59, files.line_h);
		DrawList();
		Open_Dir(#path,ONLY_SHOW);
	}
	else
	{
		llist_copy(#files, #files_inactive);
		strcpy(#path, #inactive_path);
		col_selec = 0xCCCccc;

		if (active_panel==1)
		{
			files.SetSizes(Form.cwidth/2, 57+22, Form.cwidth/2 -17, Form.cheight-59-22, files.line_h);
			DrawList();
			Open_Dir(#path,WITH_REDRAW);
			llist_copy(#files, #files_active);
			strcpy(#path, #active_path);
			col_selec = 0x94AECE;
			files.SetSizes(2, 57+22, Form.cwidth/2-2-17, Form.cheight-59-22, files.line_h);
			DrawList();
			Open_Dir(#path,WITH_REDRAW);
		}
		if (active_panel==2)
		{
			files.SetSizes(2, 57+22, Form.cwidth/2-2-17, Form.cheight-59-22, files.line_h);
			DrawList();
			Open_Dir(#path,WITH_REDRAW);
			llist_copy(#files, #files_active);
			strcpy(#path, #active_path);
			col_selec = 0x94AECE;
			files.SetSizes(Form.cwidth/2, 57+22, Form.cwidth/2 -17, Form.cheight-59-22, files.line_h);
			DrawList();
			Open_Dir(#path,WITH_REDRAW);
		}
	}
}


void List_ReDraw()
{
	int all_lines_h;
	static int old_current, old_first;

	files.CheckDoesValuesOkey(); //prevent some shit

	if (list_full_redraw) || (old_first != files.first)
	{
		old_current = files.current;
		old_first = files.first;
		list_full_redraw = false;
		goto _ALL_LIST_REDRAW;
	}
	if (old_current != files.current)
	{
		if (old_current-files.first<files.visible) Line_ReDraw(0xFFFFFF, old_current-files.first);
		Line_ReDraw(col_selec, files.current-files.first);
		old_current = files.current;
		return;
	}

	_ALL_LIST_REDRAW:

	for (j=0; j<files.visible; j++) if (files.current-files.first!=j) Line_ReDraw(0xFFFFFF, j); else Line_ReDraw(col_selec, files.current-files.first);
	//in the bottom
	all_lines_h = j * files.line_h;
	DrawBar(files.x,all_lines_h + files.y,files.w,files.h - all_lines_h,0xFFFFFF);
	DrawBar(files.x+files.w-141,all_lines_h + files.y,1,files.h - all_lines_h,system.color.work);
	DrawBar(files.x+files.w-68,all_lines_h + files.y,1,files.h - all_lines_h,system.color.work);
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
		ext1 = strrchr(file_name_off,'.') + file_name_off;
		if (ext1==file_name_off) ext1 = " \0"; //if no extension then show nothing 
		Put_icon(ext1, files.x+3, files.line_h/2-7+y, color, 0);
		WriteText(7-strlen(ConvertSize(file.sizelo))*6+files.x+files.w - 58, files.text_y + y,files.font_type,0,ConvertSize(file.sizelo));
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
	if (font.size.text==9) || (!font.data)
	{
		if (Form.width>=480)
		{
			FileShow.start_x = files.x + 23;
			FileShow.font_color = text_col;
			FileShow.area_size_x = files.w - 164;
			FileShow.text_pointer = file_name_off;
			FileShow.start_y = files.text_y + y;
			PathShow_prepare stdcall(#FileShow);
			PathShow_draw stdcall(#FileShow);
		}		
	}
	else
	{
		font.bg_color = color;
		font.text(files.x + 23, files.line_h - font.height / 2 - 1 + y, file_name_off);
	}
	DrawBar(files.x+files.w-141,y,1,files.line_h,system.color.work); //gray line 1
	DrawBar(files.x+files.w-68,y,1,files.line_h,system.color.work); //gray line 2
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
		if (files.count>0) && (files.current-files.first==-1) files.current=0;
	}
	if (files.count!=-1)
	{
		if(!_not_draw)
		{
			PathShow.area_size_x = Form.cwidth-306;
			DrawBar(PathShow.start_x-3, PathShow.start_y-4, PathShow.area_size_x+2, 15, 0xFFFfff);
			PathShow_prepare stdcall(#PathShow);
			PathShow_draw stdcall(#PathShow);
		}
		HistoryPath(ADD_NEW_PATH);
		files.visible = files.h / files.line_h;
		if (files.count < files.visible) files.visible = files.count;
		if (redraw!=ONLY_SHOW) Sorting();
		list_full_redraw = true;
		if (redraw!=ONLY_OPEN)&&(!_not_draw) List_ReDraw();
		DrawSystemDiscs();
	}
	if (files.count==-1) && (redraw!=ONLY_OPEN) 
	{
		files.KeyHome();
		if(!_not_draw) { list_full_redraw=true; List_ReDraw(); }
	}
}


inline Sorting()
{
	dword k=0, l=1;
	dword file_off;
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
	if (k>0) && (strncmp(file_mas[0]*304+buf+72,"..",2)!=0)
		for(k--; k>0; k--;) if (!strncmp(file_mas[k]*304+buf+72,"..",2)) {file_mas[k]><file_mas[0]; break;}
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
		DrawPopup(dform_x,160,220,85,1,system.color.work,system.color.work_graph);
		WriteText(-strlen(T_DELETE_FILE)*3+110+dform_x,175,0x80,system.color.work_text,T_DELETE_FILE);
		for (i=0; i<files.count; i++) 
		{
			selected_offset2 = file_mas[i]*304 + buf+32 + 7;
			if (ESBYTE[selected_offset2]) cont++;
		}
		if (cont)
		{
			sprintf(#f_count,"%s%d%s",DEL_MORE_FILES_1,cont,DEL_MORE_FILES_2);
			WriteText(-strlen(#f_count)*3+110+dform_x,190,0x80,system.color.work_text,#f_count);
		}
		else
		{
			if (strlen(#file_name)<28) 
			{
				WriteText(strlen(#file_name)*3+110+dform_x+2,190,0x80,system.color.work_text,"?");
				WriteText(-strlen(#file_name)*3+110+dform_x,190,0x80,system.color.work_text,#file_name);
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


void SelectFileByName(dword that_file)
{
	int ind;
	files.KeyHome();
   	Open_Dir(#path,ONLY_OPEN);
	if (!real_files_names_case) strttl(that_file);
	for (ind=files.count-1; ind>=0; ind--;) { if (!strcmp(file_mas[ind]*304+buf+72,that_file)) break; }
	files.current = ind - 1;
	files.KeyDown();
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
	SelectFileByName(#cur_folder);
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
	if (HistoryPath(GO_BACK)) SelectFileByName(#cur_folder);
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
					notify(FS_ITEM_ALREADY_EXISTS);
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
					notify(FS_ITEM_ALREADY_EXISTS);
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
						SelectFileByName(new_file_ed.text);
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
							SelectFileByName(new_file_ed.text);
						}
					}
				}
				else
				{
					notify(FS_ITEM_ALREADY_EXISTS);
				}
		}
		new_element_active = 0;
		Open_Dir(#path,WITH_REDRAW);
		SelectFileByName(new_file_ed.text);
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
	DrawPopup(dform_x,160,220,85,1,system.color.work,system.color.work_graph);
	new_file_ed.left = dform_x+24;
	edit_box_draw  stdcall (#new_file_ed);
	DrawRectangle(new_file_ed.left-1, new_file_ed.top-1, new_file_ed.width+2, 16, 0xFFFfff);
	DrawRectangle(new_file_ed.left-2, new_file_ed.top-2, new_file_ed.width+4, 18, system.color.work_graph);
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
			if (two_panels)
			{
				DrawFilePanels();
			}
			else 
			{
				Tip(56, T_DEVICES, 55, "-");
				Open_Dir(#path,WITH_REDRAW);
				pause(10);
				LoadIniSettings();
				GetSystemDiscs();
				Open_Dir(#path,WITH_REDRAW);
				DrawDeviceAndActionsLeftPanel();				
			}
			break;
		case 6:
			NewElement_Form(CREATE_FOLDER, T_NEW_FOLDER);
			break;
		case 7:
			NewElement_Form(CREATE_FILE, T_NEW_FILE);
			break;
		case 8:
			properties_stak = malloc(8096);
			CreateThread(#properties_dialog, properties_stak+8092);
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

void ChangeActivePanel()
{
	llist_copy(#files_active, #files_inactive);
	llist_copy(#files_inactive, #files);
	strcpy(#active_path, #inactive_path);
	strcpy(#inactive_path, #path);
	DrawFilePanels();
}


stop:
