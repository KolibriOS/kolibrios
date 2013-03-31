//Leency & Veliant 2008-2013
//GNU GPL licence.

//копировать через поток

//libraries
#define MEMSIZE 0xA0000
#include "..\lib\kolibri.h"
#include "..\lib\strings.h"
#include "..\lib\mem.h"
#include "..\lib\dll.h"
#include "..\lib\lib.obj\box_lib.h"
#include "..\lib\file_system.h"
#include "..\lib\figures.h"
#include "..\lib\encoding.h"
#include "..\lib\list_box.h"
#include "..\lib\copyf.h"
//images
#include "imgs\toolbar.txt"
#include "imgs\left_p.txt"

//for OpenDir
#define ONLY_SHOW	0
#define WITH_REDRAW	1
#define ONLY_OPEN	2

#define TITLE "Eolite File Manager v1.77"
#define ABOUT_TITLE "Eolite v1.77"
dword col_work    = 0xE4DFE1;
dword col_border  = 0x819FC5;
dword col_padding = 0xC8C9C9;
dword col_selec   = 0x94AECE;
dword col_lpanel  = 0x00699C;

int toolbar_buttons_x[7]={9,46,85,134,167,203};
char tmp_disk_del_param[3]="d0";
struct path_string { char Item[4096]; };

llist files;

byte
	path[4096],
	file_path[4096],
	file_name[256],
	copy_file[4096],
	temp[4096];	 
byte
	cut_active,
	rename_active,
	del_active;
byte
	show_dev_name=1,
	sort_num=2,
	isdir;

proc_info Form;
mouse m;
int mouse_dd, scroll_used, scroll_size;
dword buf, off;
dword file_mas[6898];
int j, i;

edit_box edit2= {250,213,80,0xFFFFCC,0x94AECE,0xFFFFCC,0xffffff,0,248,#file_name,#mouse_dd,64,6,6};
PathShow_data PathShow = {0, 17,250, 6, 250, 0, 0, 0x0, 0xFFFfff, #path, #temp, 0};
PathShow_data FileShow = {0, 56,215, 6, 100, 0, 0, 0x0, 0xFFFfff, #file_name, #temp, 0};

#include "include\some_code.h"
#include "include\about_dialog.h"
#include "include\sorting.h"
#include "include\icons_f.h"
#include "include\ini.h"
#include "include\left_panel.h"
#include "include\history.h"
#include "include\file_menu.h"

void main() 
{
	word key, id, can_show, can_select;

	files.line_h=18;
	mem_Init();
	if (load_dll2(boxlib, #box_lib_init,0)!=0) notify("Error while loading library /rd/1/lib/box_lib.obj");
	SystemDiscsGet();
	GetIni(1);	
	if (param)
	{
		strcpy(#path, #param);
		if (path[strlen(#path)-1]!='/') chrcat(#path, '/'); //add "/" to the end of the string
	}
	else
		strcpy(#path, "/rd/1/");		
	Open_Dir(#path,ONLY_OPEN);
	SetEventMask(0x27);
	loop() switch(WaitEvent())
	{
		case evMouse:
			IF (del_active) break;		
			id=GetProcessSlot(Form.ID); 
			IF (id<>GetActiveProcess()) || (Form.status_window>2) break;
			IF (rename_active) { edit_box_mouse stdcall(#edit2); break; }
			
			m.get();

			if (m.x > files.x) && (m.x < files.x + files.w) && (m.y > files.y) && (m.y < files.y+files.h)
			{
				if (m.lkm) can_select = 1;
				if (m.pkm)
				{
					can_show = 1;
					if (m.y - 57 / files.line_h != files.current) can_select = 1;
				}
			}

			//select/open file {
			if (!m.lkm) && (!m.pkm) && (can_select)
			{
				can_select = 0;
				if (m.y<57) break;
				id = m.y - 57 / files.line_h;
				if (files.current!=id) FileList_ReDraw(id-files.current);
				else Open();
			};
			// } select/open file

			//file menu {
			if (!m.pkm) && (!m.lkm) && (can_show)
			{
				can_show = 0;
				if (m.y<57) break;
				SwitchToAnotherThread();
				CreateThread(#FileMenu,#stak2);
				break;
			}
			// } file menu


			if (m.vert)
			{
				files.MouseScroll(m.vert);
				List_ReDraw();
				break;
			}

			if (m.x>=Form.width-26) && (m.x<=Form.width-6) && (m.y>40) && (m.y<57)
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
				IF (scroll_size/2+57>m.y) || (m.y<0) || (m.y>4000) m.y=scroll_size/2+57; //anee eo?ni? iaa ieiii
				id=files.first;
				j= scroll_size/2;
				files.first = m.y -j -57 * files.count;
				files.first /= onTop(22,57);
				IF (files.visible+files.first>files.count) files.first=files.count-files.visible;
				IF (id<>files.first) List_ReDraw();
			}
			break;  
//Button pressed-----------------------------------------------------------------------------
		case evButton:
			id=GetButtonID();
			IF (id==1) ExitProcess();

			IF (del_active)
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
						key=24; 
				case 25: //copy
						goto CTRLC_MARK;
				case 26: //paste
						Paste();
						break;
				case 31...33: //sort
						IF(sort_num==1) DrawFilledBar(onLeft(192,168)/2+210,42,6,10);
						IF(sort_num==2) DrawFilledBar(onLeft(115,0),42,6,10);
						IF(sort_num==3) DrawFilledBar(onLeft(44,0),42,6,10);
						sort_num=id-30;
						Open_Dir(#path,WITH_REDRAW);
						break;
				case 78: //rescan devices
						Tip(56, "Devices", 78, "-");
						pause(10);
						GetIni(1);
						SystemDiscsGet();
						Open_Dir(#path,WITH_REDRAW);
						DrawLeftPanel();
						break;
				case 50...60: //Actions
						ActionsProcess(id-50);
						break;
				case 100...120:
					DEVICE_MARK:
						DrawRectangle3D(17,id-100*16+74,159,16, 0, 0); //auaaeaiea
						strcpy(#path, #disk_list[id-100].Item);
						files.first=files.current=0;
						Open_Dir(#path,1);
						pause(5);
						DrawRectangle3D(17,id-100*16+74,159,16, 0xFFFFFF, 0xFFFFFF);
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
			IF (edit2.flags<>64) && (key<>13) && (key<>27)
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
					case 003: //Ctrl+C
							CTRLC_MARK:
							IF (isdir) break;
							strcpy(#copy_file, #file_path);
							IF (key==24) cut_active=1; ELSE cut_active=0; 
							break;
					case 022: //Ctrl+V
							Paste();
							break;
					case 027: //Esc
							IF (rename_active==1) ReName(false);
							break;
					case 013: //Enter
							IF (rename_active==1) {ReName(true); break;}
							Open();
							break; 
					case 178: //up
							FileList_ReDraw(-1);
							break;
					case 177: //down
							FileList_ReDraw(1);
							break;
					case 180: //home
							files.first=0;
							files.current=0;
							List_ReDraw();
							break;
					case 181: //end
							files.first=files.count-files.visible;
							files.current=files.visible-1;
							List_ReDraw();
							break;
					case 183: //Page Down
							FileList_ReDraw(files.visible-1);
							break;
					case 184: //Page Up
							FileList_ReDraw(-files.visible+1);
							break;
					case 182: //del
							Del_Form();
							break;
					case 050...059: //F1-F10
							ActionsProcess(key-49);
							break; 
					default:    
							for (i=files.current+files.first+1; i<files.count; i++)
							{
								strcpy(#temp, file_mas[i]*304+buf+72);
								IF (temp[0]==key) || (temp[0]==key-32)
								{
									FileList_ReDraw(i-files.current-files.first);
									break;
								}
							}
			}                         
			break;
		case evReDraw:
			DefineAndDrawWindow(40,20,550,500,0x73,col_work,TITLE);
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window>2) break;
			files.SetSizes(192, 57, onLeft(192,27), onTop(57,6), disc_num*16+195,files.line_h);
			if (Form.height < files.min_h) MoveSize(OLD,OLD,OLD,files.min_h);
			if (Form.width<480) MoveSize(OLD,OLD,480,OLD);
			draw_window();
	}
}


inline fastcall void draw_window()
{	
	//toolbar
	PutPaletteImage(#toolbar,246,34,0,0,8,#toolbar_pal);
	for (j=0; j<3; j++) DefineButton(toolbar_buttons_x[j]+2,5+2,31-5,29-5,21+j+BT_HIDE,col_work);
	for (j=3; j<6; j++) DefineButton(toolbar_buttons_x[j],5,31,29,21+j+BT_HIDE,col_work);
	DrawBar(246,0,onLeft(246,60),12, col_work); //фон над полосой адреса
	DrawBar(246,29,onLeft(246,60),5,col_work);  //фон под полосой адреса
	DrawRectangle3D(246,12,onLeft(66,246),16,col_border,col_border);
	DefineButton(onLeft(34,0),6,27,28,51+BT_HIDE+BT_NOFRAME,col_work); //about
	PutPaletteImage(#goto_about,56,34,Form.width-65,0,8,#goto_about_pal);
	//main rectangles
	DrawRectangle3D(1,40,Form.cwidth-3,onTop(46,0),col_border,col_border);
	DrawRectangle3D(0,39,Form.cwidth-1,onTop(44,0),col_palette[4],col_palette[4]); //bg
	for (i=0; i<5; i++) DrawBar(0, 34+i, Form.cwidth, 1, col_palette[8-i]);	
	DrawLeftPanel();
	//ListBox
	DrawFlatButton(files.x,40,onLeft(files.x,168),16,31,col_work,"File");
	DrawFlatButton(onLeft(168,0),40,73,16,32,col_work,"Type");
	DrawFlatButton(onLeft(95,0),40,68,16,33,col_work,"Size");
	DrawBar(onLeft(27,0),57,1,onTop(22,57),col_border); //line to the left from the scroll
	DrawFlatButton(onLeft(27,0),40,16,16,0,col_work,"\x18");
	DrawFlatButton(onLeft(27,0),onTop(22,0),16,16,0,col_work,"\x19");
	Open_Dir(#path,ONLY_SHOW);
	if (del_active) Del_Form();
}


void KEdit()
{
	if (Form.width<480) return;
	PathShow.area_size_x = Form.cwidth-306;
	DrawBar(PathShow.start_x-3, PathShow.start_y-4, PathShow.area_size_x+2, 15, 0xFFFfff);
	PathShow_prepare stdcall(#PathShow);
	PathShow_draw stdcall(#PathShow);
}


void FileList_ReDraw(int cur)
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
	//если мы в конце списка файлов развернём окно появяться пустяе белые кнопки
	//это если выделение после схлопывания окна за кадром
	if (files.count-files.first<files.visible) || (files.current>files.visible-1)
	{ files.first=files.count-files.visible; files.current=files.visible-1; }

	for (j=0; j<files.visible; j++) if (files.current<>j) Line_ReDraw(0xFFFFFF, j); else Line_ReDraw(col_selec, files.current);
	//in the bottom
	paint_y = j * files.line_h + files.y;
	DrawBar(files.x,paint_y,files.w,onTop(paint_y,6),0xFFFFFF);
	DrawBar(onLeft(168,0),paint_y,1,onTop(paint_y,6),col_work);
	DrawBar(onLeft(95,0),paint_y,1,onTop(paint_y,6),col_work);
	Scroll();
}

void Line_ReDraw(dword color, filenum){
	dword text_col=0, name_len=0, y=filenum*files.line_h+57;
	IF (rename_active==1) ReName(false);
	DrawBar(192,y,3,files.line_h,color); 
	DrawBar(192+19,y,onLeft(46,192),files.line_h,color); DrawBar(195,y+17,16,1,color);
	if (files.line_h>18) DrawBar(195,y+18,16,files.line_h-18,color);
	if (files.line_h>15) DrawBar(195,y,16,files.line_h-15,color); 

	off=file_mas[filenum+files.first]*304 + buf+72;

	if (!TestBit(ESDWORD[off-40], 4)) //file or folder?
	{	
		strcpy(#temp, off);
		Put_icon(#temp+_strrchr(#temp,'.'), files.line_h/2-7+y, color);
		WriteText(7-strlen(ConvertMemSize(ESDWORD[off-8]))*6+onLeft(75,0),files.line_h-6/2+y,0x80,0,ConvertMemSize(ESDWORD[off-8])); //size
	}
	else
		if (!strcmp("..",off))
			Put_icon("..", files.line_h/2-7+y, color);
		else
			Put_icon("<DIR>", files.line_h/2-7+y, color);
	if (TestBit(ESDWORD[off-40],1)) || (TestBit(ESDWORD[off-40],2)) text_col=0xA6A6B7; //system or hiden?
	if (color<>0xFFFfff)
	{
		isdir=TestBit(ESDWORD[off-40], 4);		
		strcpy(#file_name, off);
		strcpy(#file_path, #path);
		strcat(#file_path, #file_name);
		if (text_col==0xA6A6B7) text_col=0xFFFFFF;
	}
	if (Form.width>=480)
	{
		FileShow.font_color = text_col;
		FileShow.area_size_x = Form.width - 380;
		FileShow.text_pointer = off;
		FileShow.start_y = files.line_h/2-3+y;
		PathShow_prepare stdcall(#FileShow);
		PathShow_draw stdcall(#FileShow);
	}
	DrawBar(onLeft(168,0),y,1,files.line_h,col_work); //gray line 1
	DrawBar(onLeft(95,0),y,1,files.line_h,col_work); //gray line 2
}


void Open_Dir(dword temp_, redraw){
	int errornum, count;
	char path_[4096],
	somelen=strlen(temp_)-1;

	if (redraw<>ONLY_SHOW)
	{
		strcpy(#path_, temp_);
		if (somelen) path_[somelen]=NULL;
		
		if (buf) free(buf);
    	buf = malloc(32);
		errornum=ReadDir(0, buf, #path_);
		if (errornum<>0)
		{
			HistoryPath(ADD_NEW_PATH);
			GoBack();
			Write_Error(errornum);
			return;
		}
    	files.count = ESDWORD[buf+8];
    	buf = realloc(buf, files.count * 304 + 32);
		ReadDir(files.count, buf, #path_);
		files.count=EBX;
		count = sizeof(file_mas)/sizeof(dword)-1;
		if (files.count>count) files.count=count;
		
	}
	if (files.count<>-1)
	{
		KEdit();
		HistoryPath(ADD_NEW_PATH);
		IF (!strcmp(".",buf+72)) {files.count--; memmov(buf,buf+304,files.count*304);} //oeeuo?oai yeaiaio "."
		files.visible=onTop(6,57)/files.line_h;
		IF (files.count<files.visible) files.visible=files.count;
		IF (sort_num==1) WriteText(Form.width+60/2,45,0x80,0x4E78AC,"\x19");
		IF (sort_num==2) WriteText(Form.width-115,45,0x80,0x4E78AC,"\x19");
		IF (sort_num==3) WriteText(Form.width-44,45,0x80,0x4E78AC,"\x19");
		IF (redraw<>ONLY_SHOW) Sorting();
		IF (redraw<>ONLY_OPEN) List_ReDraw();
	}
	IF (files.count==-1) && (redraw<>ONLY_OPEN) {files.visible=files.count=0; List_ReDraw();}
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
		strttl(off+40);
		if (TestBit(ESDWORD[off],4)) //isdir?
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
	IF (k>0) && (strcmp(file_mas[0]*304+buf+72,"..")<>0) FOR(k--; k>0; k--;) IF (!strcmp(file_mas[k]*304+buf+72,"..")) file_mas[k]><file_mas[0];
}


void Del_Form()
{
	int dform_x=Form.width/2-13;
	//oeia ieii
	if (!files.count) return;
	DrawFlatButton(dform_x,160,200,80,0,col_work, ""); //oi?ia
	WriteText(dform_x+19,175,0x80,0,"Do you really want to delete");
	IF (strlen(#file_name)<28) 
		{
			WriteText(strlen(#file_name)*6+dform_x+20,190,0x80,0,"?");
			WriteText(dform_x+20,190,0x80,0,#file_name); //ieoai eiy
		}
	ELSE
		{
			WriteText(164+dform_x,190,0x80,0,"...?");
			ESI = 24;
			WriteText(dform_x+20,190,0,0,#file_name); //ieoai eiy
		}
	//eiiii?ee
	DrawFlatButton(dform_x+20,208,70,20,301,0xFFB6B5,"Yes");
	DrawFlatButton(dform_x+111,208,70,20,302,0xC6DFC6,"No");
	del_active=1;
}

	
void Del_File(byte dodel)
{    
	int del_rezult;
	IF (dodel==true)
	{
		del_rezult = DeleteFile(#file_path);
		IF (del_rezult<>0)
		{
			Write_Error(del_rezult);
			IF ( isdir) ShowMessage("Error. Folder isn't empty.");
			IF (!isdir) ShowMessage("Error. Filesystem read-only.");
		}
 	}
	del_active=0;
	DeleteButton(301); DeleteButton(302);
	Open_Dir(#path,1);
}    

void Paste()
{
	char new_copy_path[4096];
	int copy_rezult;
	
	IF (!copy_file) return;
	strcpy(#new_copy_path, #path);
	strcat(#new_copy_path, #copy_file+strrchr(#copy_file,'/'));
	if (!strcmp(#copy_file,#new_copy_path))
	{
		strcpy(#new_copy_path, #path);
		strcat(#new_copy_path, "new_");
		strcat(#new_copy_path, #copy_file+strrchr(#copy_file,'/'));
	}
	copy_rezult = CopyFile(#copy_file,#new_copy_path);
	IF (copy_rezult!=0)
	{
		Write_Error(copy_rezult);
		return;
	}
	IF (cut_active)
	{
		strcpy(#file_path, #copy_file);
		Del_File(true);
		copy_file=NULL;
		cut_active=false;
	}
	SelectFile(#new_copy_path+strrchr(#new_copy_path,'/'));
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
		if (strcmp(#file_path,#temp)<>0) && (file_name)
		IF (isdir)
		{
			del_rezult = DeleteFile(#file_path);
			IF (del_rezult!=0)
			{
				Write_Error(del_rezult);
				ShowMessage("Error. Folder isn't empty.");
				return;
			}
			ELSE CreateDir(#temp);
			Open_Dir(#path,1);
		}
		ELSE
		{
			copy_rezult = CopyFile(#file_path,#temp);
			if (copy_rezult!=0) Write_Error(copy_rezult); else Del_File(true);
		}
		SelectFile(#edit_name);
	}
	Line_ReDraw(col_selec,files.current);
}


void SelectFile(dword that_file)
{
	files.first=files.current=0;
   	Open_Dir(#path,ONLY_OPEN);
	strttl(that_file);
	for (i=files.count-1; i>=0; i--;)
		if (!strcmp(file_mas[i]*304+buf+72,that_file)) break;
	FileList_ReDraw(i);
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
	if (!isdir)
	{
		GetIni(0);
	} 
	else
	{
		if (!strcmp(#file_name,"..")) { Dir_Up(); return; }
		strcpy(#path, #file_path);
		if (path[strlen(#path)-1]!='/') chrcat(#path, '/'); //need "/" in the end
		files.first=files.current=0;
		Open_Dir(#path,1);
	}
}


inline fastcall void GoBack()
{
	char cur_folder[4096];
	strcpy(#cur_folder, GetCurrentFolder());
	if (HistoryPath(GO_BACK)) SelectFile(#cur_folder);
}

void ActionsProcess(char N)
{
	switch(N)
	{
		case 1:
			SwitchToAnotherThread();
			CreateThread(#about_dialog,#stak1);
			break;
		case 2:
			if (!files.count) break;
			DeleteButton(files.current+201);
			edit2.flags=100000000000010b; //set active
			edit2.width=onLeft(24,217);
			edit2.top=files.current*files.line_h+59;
			edit2.size=edit2.pos=strlen(#file_name);
			edit_box_draw  stdcall (#edit2);
			DrawBar(213,files.current*files.line_h+58,edit2.width+1,1,0xFFFFCC); //bg
			rename_active=1;
			break;
		case 3:
			IF (!isdir) RunProgram("/sys/tinypad", #file_path);
			break;
		case 4:
			IF (!isdir) RunProgram("/sys/develop/heed", #file_path);
			break;
		case 5:
			Open_Dir(#path,1);
			break;
		case 6:
			strcpy(#temp, #path);
			strcat(#temp, "New folder");
			CreateDir(#temp);
			if (!EAX){
				SelectFile("New folder");
				ActionsProcess(2);
			}
			else
			{
				Write_Error(EAX);
				ShowMessage("Folder can not be created.");
			}
			break;
		case 7:
			strcpy(#temp, #path);
			strcat(#temp, "New file");
			WriteFile(0, 0, #temp);
			if (!EAX){
				SelectFile("New file");
				ActionsProcess(2);
			}
			else
			{
				Write_Error(EAX);
				ShowMessage("File can not be created.");
			}
			break;
		case 10: //F10
			RunProgram(EDITOR_PATH, abspath("Eolite.ini"));
			break;
	}
}

void copyf_Action() {
	return;
}

stop:

char stak1[512];
char stak2[512];