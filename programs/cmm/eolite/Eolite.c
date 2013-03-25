//Leency & Veliant 2008-2013
//GNU GPL licence.

//копировать через поток
//иконка действительно нужна другая для неизвесных устройств и папок в будующем.

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
//images
#include "imgs\toolbar.txt"
#include "imgs\left_p.txt"

int BUTTON_HEIGHT=18;

//for OpenDir
#define ONLY_SHOW	0
#define WITH_REDRAW	1
#define ONLY_OPEN	2

//переменные
#define TITLE "Eolite File Manager v1.64"
#define ABOUT_TITLE "Eolite v1.64"
dword col_work    = 0xE4DFE1;
dword col_border  = 0x819FC5;
dword col_padding = 0xC8C9C9;
dword col_selec   = 0x94AECE;
dword col_lpanel  = 0x00699C;


int  f_visible,
     count,
     za_kadrom,
     curbtn;
	 
	 
byte cut_active,
     rename_active,
     del_active;
byte show_actions=1,
	 show_dev_name=1,
     sort_num=2,
     isdir;
unsigned char
	path[4096],
	edit_path[4096],
	file_path[4096],
	file_name[256],
	copy_file[4096],
	temp[4096];
int scroll_size;

struct path_string {
char Item[4096];
};

int toolbar_buttons_x[7]={9,46,85,134,167,203};
char tmp_disk_del_param[3]="d0\0";

dword file_mas[6898];
int j, i, mouse_dd;
int scroll_used;

proc_info Form;
mouse m;
dword buf, off; //для текста и буфера

edit_box edit2= {250,213,80,0xFFFFCC,0x94AECE,0xFFFFCC,0xffffff,0,248,#file_name,#mouse_dd,64,6,6};
PathShow_data PathShow = {0, 17,250, 6, 250, 0, 0, 0x0, 0xFFFfff, #path, #edit_path, 0};
PathShow_data FileShow = {0, 56,215, 6, 100, 0, 0, 0x0, 0xFFFfff, #file_name, #temp, 0};

inline fastcall signed int _strrchr( ESI,BL)
{
	int jj=0, last=strlen(ESI);
	do{
		jj++;
		$lodsb
		IF(AL==BL) last=jj;
	} while(AL!=0);
	return last;
}

#include "include\some_code.h"
#include "include\about_dialog.h"
#include "include\sorting.h"
#include "include\icons_f.h"
#include "include\ini.h"
#include "include\left_panel.h"
#include "include\history.h"

void main() 
{
	word key, id, can_show; 
	int min_win_h;

	mem_Init();
	if (load_dll2(boxlib, #box_lib_init,0)!=0) notify("Error while loading library /rd/1/lib/box_lib.obj");
	SetEventMask(0x27);
	GetSystemDiscs();
	GetIni(1);
	
	if (param)
	{
		strcpy(#path, #param);
		if (strcmp(#path+strlen(#path)-1,"/")<>0) strcat(#path, "/"); //если нет, + "/"
	}
	else
		strcpy(#path, "/rd/1/");
		
	Open_Dir(#path,ONLY_OPEN);
	loop()	switch(WaitEvent())
	{
		case evMouse:
			IF (del_active) break;
			
			id=GetProcessSlot(Form.ID); 
			IF (id<>GetActiveProcess()) break; //если окно не активно на события мыши не реагируем
			
			IF (rename_active) edit_box_mouse stdcall(#edit2);
			
			m.get();

			/*if (m.pkm) && (m.x > 192) && (m.y > 57) can_show = 1;
			if (!m.pkm) && (can_show)
			{
				SwitchToAnotherThread();
				CreateThread(#FileMenu,#stak2);
				can_show = 0;
			}*/

			if (m.x>=Form.width-26) && (m.x<=Form.width-6) && (m.y>40) && (m.y<57)
			{
				IF (m.lkm==1) DrawRectangle3D(onLeft(26,0),41,14,14,0xC7C7C7,0xFFFFFF);
				WHILE (m.lkm==1) && (za_kadrom>0)
				{
					pause(10);
					za_kadrom--;
					List_ReDraw();
					m.get();
				}
				DrawRectangle3D(onLeft(26,0),41,14,14,0xFFFFFF,0xC7C7C7);
			}

			if (m.x>=Form.width-26) && (m.x<=Form.width-6) && (m.y>onTop(22,0)+1) && (m.y<onTop(22,0)+16)
			{
				IF (m.lkm==1) DrawRectangle3D(onLeft(26,0),onTop(21,0),14,14,0xC7C7C7,0xFFFFFF);
				while (m.lkm==1) && (za_kadrom<count-f_visible)
				{
					pause(10);
					za_kadrom++;
					List_ReDraw();
					m.get();
				}
				DrawRectangle3D(onLeft(26,0),onTop(21,0),14,14,0xFFFFFF,0xC7C7C7);
			}

			//колёсико мыши
			IF (m.vert==65535) && (za_kadrom>0)
			{
				if (za_kadrom>0) za_kadrom--;
				if (curbtn<f_visible-1) curbtn++;
				List_ReDraw();
				if (za_kadrom>0) za_kadrom--;
				if (curbtn<f_visible-1) curbtn++;
				List_ReDraw();
			}
			IF (m.vert==1) && (za_kadrom<count-f_visible)
			{
				if (za_kadrom<count-f_visible) za_kadrom++;
				if (curbtn>0) curbtn--;
				List_ReDraw();
				if (za_kadrom<count-f_visible) za_kadrom++;
				if (curbtn>0) curbtn--;
				List_ReDraw();
			}
			//скролл
			if (!m.lkm) && (scroll_used) { scroll_used=NULL; TVScroll(); }
			if (m.x>=Form.width-26) && (m.x<=Form.width-6) && (m.y>56) && (m.y<Form.height) && (m.lkm) && (!scroll_used) {scroll_used=1;TVScroll();}
			
			if (scroll_used)
			{
				IF (scroll_size/2+57>m.y) || (m.y<0) || (m.y>4000) m.y=scroll_size/2+57; //если курсор над окном
				id=za_kadrom; //сохраняем старое количество
				j= scroll_size/2;
				za_kadrom = m.y -j -57 * count;
				za_kadrom /= onTop(22,57);
				IF (f_visible+za_kadrom>count) za_kadrom=count-f_visible;
				IF (id<>za_kadrom) List_ReDraw(); //чтоб лишний раз не перерисовывать
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
							za_kadrom=curbtn=NULL; //вверх списка
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
						Open_Dir(#path,1);
						break; 
				case 30: //about
						SwitchToAnotherThread();
						CreateThread(#about_dialog,#stak1); 
						break;
				case 77:
						IF (show_actions==1) show_actions=0; ELSE show_actions=1;
						DrawLeftPanel();
						break;
				case 78: //rescan devices
						Tip(56, "Devices", 78, "-");
						pause(10);
						GetIni(1);
						GetSystemDiscs();
						Open_Dir(#path,WITH_REDRAW);
						DrawLeftPanel();
						break;
				case 80: //rename
						goto REN_MARK; 
				case 81: //Delete file
						Del_Form();
						break;
				case 82: //create folder
					NEW_FOLDER_MARK:
						strcpy(#temp, #path);
						strcat(#temp, "New folder");
						CreateDir(#temp);
						IF (!EAX){
							SelectFile("New folder");
							goto REN_MARK;
						}
						ELSE
						{
							Write_Error(EAX);
							ShowMessage("Folder can not be created.");
						}
						break;
				case 100...120:
					DEVICE_MARK:
						DrawRectangle3D(17,id-100*16+74,159,16, 0, 0); //выделение
						strcpy(#path, #disk_list[id-100].Item);
						za_kadrom=curbtn=0;
						Open_Dir(#path,1);
						pause(5);
						DrawRectangle3D(17,id-100*16+74,159,16, 0xFFFFFF, 0xFFFFFF);
						break;
				case 130...160:
						tmp_disk_del_param[1]=disk_list[id-130].Item[4];
						RunProgram("/sys/tmpdisk", #tmp_disk_del_param);
						pause(10);
						GetSystemDiscs();
						Open_Dir(#path,WITH_REDRAW);
						DrawLeftPanel();
						break;
				default:
						if (id<200) break; //кнопки из списка файлов
						if (curbtn!=id-201)	{FileList_ReDraw(id-201-curbtn); break;}
						else
						{
							OPEN_MARK:
							if (!isdir)
							{
								GetIni(0);
							} 
							else
							{
								if (!strcmp(#file_name,"..")) { Dir_Up(); break; }
								OPEN_DEV:
								strcpy(#path, #file_path);
								IF (strcmp(#path+strlen(#path)-1,"/")<>0) strcat(#path,"/"); //если нет, + "/"
								za_kadrom=curbtn=0;
								Open_Dir(#path,1);
							}
						}
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
					case   8: //Назад
							//GoBack();
							Dir_Up();
							break; 
					case 004: //Ctrl+D рис на раб стол растянуть
							strcpy(#temp, "\\S__");
							strcat(#temp, #file_path);
							RunProgram("/sys/media/kiv", #temp);
							break;
					case 014: //Ctrl+N новое окно
							IF (Form.left==98) MoveSize(Form.left-20,Form.top-20,OLD,OLD);
							RunProgram("/sys/File Managers/Eolite", #path);
							break; 
					case 024: //Ctrl+X
					case 003: //Ctrl+C
							CTRLC_MARK:
							IF (isdir) break; //папки пока что копировать не умеем
							strcpy(#copy_file, #file_path); //вычисляем какой файл копировать
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
							GOTO OPEN_MARK;
							break; 
					case 55: //F6 - new folder
							goto NEW_FOLDER_MARK;
					case 56:  //IF (rename_active==1) break;//up
					case 178: //up
							FileList_ReDraw(-1);
							break;
					case 177: //down
							FileList_ReDraw(1);
							break;
					case 180: //home
							za_kadrom=0;
							curbtn=0;
							List_ReDraw();
							break;
					case 181: //end
							za_kadrom=count-f_visible;
							curbtn=f_visible-1;
							List_ReDraw();
							break;
					case 183: //Page Down
							FileList_ReDraw(f_visible-1);
							break;
					case 184: //Page Up
							FileList_ReDraw(-f_visible+1);
							break;
					case 051: //Нажата F2
							REN_MARK:
							if (!count) break;
							DeleteButton(curbtn+201); //это чтоб можно было выделять мышью
							edit2.flags=66; //делаем компонент активным
							edit2.width=onLeft(24,217);
							edit2.top=curbtn*BUTTON_HEIGHT+59;
							edit2.size=edit2.pos=strlen(#file_name);
							edit_box_draw  stdcall (#edit2);
							DrawBar(213,curbtn*BUTTON_HEIGHT+58,edit2.width+1,1,0xFFFFCC); //полоса желтая сверху для одинаковости
							rename_active=1;
							break;
					case 052: //Нажата F3
							IF (!isdir) RunProgram("/sys/tinypad", #file_path);
							break;
					case 053: //Нажата F4
							IF (!isdir) RunProgram("/sys/develop/heed", #file_path);
							break;
					case 054: //F5
							Open_Dir(#path,1);
							break;
					case 182: //delete file
							Del_Form();
							break; 
					default:    
							for (i=curbtn+za_kadrom+1; i<count; i++)
							{
								strcpy(#temp, file_mas[i]*304+buf+72);
								IF (temp[0]==key) || (temp[0]==key-32)
								{
									FileList_ReDraw(i-curbtn-za_kadrom);
									break;
								}
							}
			}                         
			break;
		case evReDraw:
			DefineAndDrawWindow(40,20,550,500,0x73,col_work,TITLE);
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window>2) break;
			min_win_h = disc_num*16+195;
			if (Form.height < min_win_h) MoveSize(OLD,OLD,OLD,min_win_h);
			if (Form.width<480) MoveSize(OLD,OLD,480,OLD);
			draw_window();
	}
}


inline fastcall void draw_window()
{	
	//toolbar buttons
	PutPaletteImage(#toolbar,246,34,0,0,8,#toolbar_pal);
	for (j=0; j<3; j++) DefineButton(toolbar_buttons_x[j]+2,5+2,31-5,29-5,21+j+BT_HIDE,col_work);
	for (j=3; j<6; j++) DefineButton(toolbar_buttons_x[j],5,31,29,21+j+BT_HIDE,col_work);
	//полоса адреса
	DrawBar(246,0,onLeft(246,60),12, col_work); //фон над полосой адреса
	DrawBar(246,29,onLeft(246,60),5,col_work); //фон под полосой адреса
	DrawRectangle3D(246,12,onLeft(66,246),16,col_border,col_border);	//ободок
	DefineButton(onLeft(34,0),6,27,28,30+BT_HIDE+BT_NOFRAME,col_work); //about
	PutPaletteImage(#goto_about,56,34,Form.width-65,0,8,#goto_about_pal);
	//прямоугольники внутри
	DrawRectangle3D(1,40,Form.cwidth-3,onTop(46,0),col_border,col_border); //синий ободок
	DrawRectangle3D(0,39,Form.cwidth-1,onTop(44,0),col_palette[4],col_palette[4]); //фон
	for (i=0; i<5; i++) DrawBar(0, 34+i, Form.cwidth, 1, col_palette[8-i]);	
	DrawLeftPanel();
	//SortButtons
	DrawFlatButton(192,40,onLeft(192,168),16,31,col_work,"File");
	DrawFlatButton(onLeft(168,0),40,73,16,32,col_work,"Type");
	DrawFlatButton(onLeft(95,0),40,68,16,33,col_work,"Size");
	//Перерисовываем список
	Open_Dir(#path,ONLY_SHOW);
	//прокрутка
	DrawBar(onLeft(27,0),57,1,onTop(22,57),col_border); //линия слева от прокрутки 
	DrawFlatButton(onLeft(27,0),40,16,16,0,col_work,"\x18");		//прокрутка вверх
	DrawFlatButton(onLeft(27,0),onTop(22,0),16,16,0,col_work,"\x19");//прокрутка вниз
	if (del_active) Del_Form();
	DefineButton(onLeft(34,0),6,27,28,30+BT_HIDE+BT_NOFRAME,col_work); //about
}


void KEdit()
{
	PathShow.area_size_x = Form.cwidth-306;
	DrawBar(PathShow.start_x-3, PathShow.start_y-4, PathShow.area_size_x+2, 15, 0xFFFfff);
	PathShow_prepare stdcall(#PathShow);
	PathShow_draw stdcall(#PathShow);
}


void FileList_ReDraw(int curbtn_)
{
	if (curbtn_<=0) //вверх
	{
		IF (za_kadrom==0) && (curbtn<=0) return;
		IF (-curbtn_-1<curbtn)
		{
			Line_ReDraw(0xFFFFFF, curbtn); //белая полоса
			curbtn+=curbtn_;
			Line_ReDraw(col_selec, curbtn); //выделение
			return;
		}
		ELSE
		{
			IF (-curbtn_<za_kadrom) za_kadrom+=curbtn_; ELSE za_kadrom=0;
			curbtn=0;
			List_ReDraw();
			return;
		}
	}
	else  //вниз
	{
		IF (za_kadrom==count-f_visible) && (curbtn==f_visible-1) return;
		IF (f_visible-curbtn>curbtn_)
		{
			Line_ReDraw(0xFFFFFF, curbtn); //белая полоса
			curbtn+=curbtn_;
			Line_ReDraw(col_selec, curbtn); //выделение
			return;
		}
		else
		{
			IF(za_kadrom+curbtn+curbtn_>=count)
			{
				za_kadrom=count-f_visible;
				curbtn=curbtn_-za_kadrom+curbtn;
				}
			ELSE
			{
				za_kadrom+=curbtn_+curbtn-f_visible+1;
				curbtn=f_visible-1;
			}
			
			IF (curbtn<0) || (curbtn>f_visible)
			{
				curbtn=f_visible-1;
			}
			List_ReDraw();
		}
	}
}




void List_ReDraw()
{
	int paint_x=f_visible*BUTTON_HEIGHT+57;
	IF (count-za_kadrom<f_visible) || (curbtn>f_visible-1) //если мы в конце списка файлов развернём окно появяться пустяе белые кнопки
	{ za_kadrom=count-f_visible; curbtn=f_visible-1; } //это если выделение после схлопывания окна за кадром

	FOR (j=0; j<f_visible; j++) IF (curbtn<>j) Line_ReDraw(0xFFFFFF, j); ELSE Line_ReDraw(col_selec, curbtn);
		DrawBar(192,paint_x,onLeft(27,192),onTop(paint_x,6),0xFFFFFF); //заливка белым доконца
		DrawBar(onLeft(168,0),paint_x,1,onTop(paint_x,6),col_work); //полоса серая вертикальная 1
		DrawBar(onLeft(95,0),paint_x,1,onTop(paint_x,6),col_work); //полоса серая вертикальная 2
	TVScroll();
}

void Line_ReDraw(dword color, filenum){
	dword text_col=0, name_len=0, y=filenum*BUTTON_HEIGHT+57; //положение текста по Y
	IF (rename_active==1) ReName(false);
	DefineButton(192,y,onLeft(28,192),BUTTON_HEIGHT,201+filenum+BT_HIDE+BT_NOFRAME,color); //кнопа
	//да, я не спорю что изврат, но зато перерисовки не видно
	DrawBar(192,y,3,BUTTON_HEIGHT,color); 
	DrawBar(192+19,y,onLeft(46,192),BUTTON_HEIGHT,color); DrawBar(195,y+17,16,1,color);
	if (BUTTON_HEIGHT>18) DrawBar(195,y+18,16,BUTTON_HEIGHT-18,color);
	if (BUTTON_HEIGHT>15) DrawBar(195,y,16,BUTTON_HEIGHT-15,color); 

	off=file_mas[filenum+za_kadrom]*304 + buf+72;

	if (!TestBit(ESDWORD[off-40], 4)) //это ФАЙЛ или папка?
	{	
		strcpy(#temp, off);
		Put_icon(#temp+_strrchr(#temp,'.'), BUTTON_HEIGHT/2-7+y, color);
		WriteText(7-strlen(ConvertMemSize(ESDWORD[off-8]))*6+onLeft(75,0),BUTTON_HEIGHT-6/2+y,0x80,0,ConvertMemSize(ESDWORD[off-8])); //size
	}
	else
		if (!strcmp("..",off))
			Put_icon("..", BUTTON_HEIGHT/2-7+y, color);
		else
			Put_icon("<DIR>", BUTTON_HEIGHT/2-7+y, color);

	if (TestBit(ESDWORD[off-40],1)) || (TestBit(ESDWORD[off-40],2)) text_col=0xA6A6B7; //файл скрытый или системный?
	if (color<>0xFFFfff)
	{
		isdir=TestBit(ESDWORD[off-40], 4);		
		strcpy(#file_name, off);
		strcpy(#file_path, #path);
		strcat(#file_path, #file_name); //полный путь к файлу
		if (text_col==0xA6A6B7) text_col=0xFFFFFF;
	}
	FileShow.font_color = text_col;
	FileShow.area_size_x = Form.width - 380;
	FileShow.text_pointer = off;
	FileShow.start_y = BUTTON_HEIGHT/2-3+y;
	PathShow_prepare stdcall(#FileShow);
	PathShow_draw stdcall(#FileShow);

	DrawBar(onLeft(168,0),y,1,BUTTON_HEIGHT,col_work); //полоса серая вертикальная 1
	DrawBar(onLeft(95,0),y,1,BUTTON_HEIGHT,col_work); //полоса серая вертикальная 2
}


void Open_Dir(dword temp_, redraw){
	int errornum, max_count;
	char path_[4096],
	somelen=strlen(temp_)-1;
	
	if (redraw<>ONLY_SHOW)
	{
		strcpy(#path_, temp_);
		if (somelen) path_[somelen]=NULL;
		
		if (buf) free(buf);
    	buf = malloc(32);
		errornum=ReadDir(0, buf, #path_);
		if (errornum<>0) //ошибка при чтении папки
		{
			HistoryPath(ADD_NEW_PATH);
			GoBack();
			Write_Error(errornum);
			return;
		}
    	count = ESDWORD[buf+8];
    	buf = realloc(buf, count * 304 + 32);
		ReadDir(count, buf, #path_);
		count=EBX;
		max_count = sizeof(file_mas)/sizeof(dword)-1;
		if (count>max_count) count=max_count;
		
	}
	if (count<>-1)
	{
		KEdit();
		HistoryPath(ADD_NEW_PATH);
		IF (!strcmp(".",buf+72)) {count--; memmov(buf,buf+304,count*304);} //фильтруем элемент "."
		FOR (j=0;j<f_visible;j++) DeleteButton(201+j); //удаляем старые
		f_visible=onTop(6,57)/BUTTON_HEIGHT;
		IF (count<f_visible) f_visible=count;
		//стрелочка сортировки
		IF (sort_num==1) WriteText(Form.width+60/2,45,0x80,0x4E78AC,"\x19");
		IF (sort_num==2) WriteText(Form.width-115,45,0x80,0x4E78AC,"\x19");
		IF (sort_num==3) WriteText(Form.width-44,45,0x80,0x4E78AC,"\x19");
		IF (redraw<>ONLY_SHOW) Sorting(); //для больших папок при репеинте окна
		IF (redraw<>ONLY_OPEN) List_ReDraw();
	}
	IF (count==-1) && (redraw<>ONLY_OPEN) {f_visible=count=0; List_ReDraw();}
}



inline Sorting()
{
	dword k=0, l=1;
	int i;
	if (!strcmp(#path,"/")) //не сортировать папки и не менять регистр в "/"
	{
		FOR(k=1;k<count;k++;) file_mas[k]=k;
		return;
	}
	FOR (j=count-1, off=count-1*304+buf+32; j>=0; j--, off-=304;)  //папки вверх, файлы вниз
	{
		strttl(off+40);
		if (TestBit(ESDWORD[off],4)) //папка?
		{
			file_mas[k]=j;
			k++;
		}
		else
		{
			file_mas[count-l]=j;
			l++;
		}
	}
	//Собственно сортировка: вначале папки, потом файлы
	Sort_by_Name(0,k-1);
	IF (sort_num==1) Sort_by_Name(k,count-1);
	IF (sort_num==2) Sort_by_Type(k,count-1);
	IF (sort_num==3) Sort_by_Size(k,count-1);
	//если папка ".." не первая, ставим её туда
	IF (k>0) && (strcmp(file_mas[0]*304+buf+72,"..")<>0) FOR(k--; k>0; k--;) IF (!strcmp(file_mas[k]*304+buf+72,"..")) file_mas[k]><file_mas[0];
}


void Del_Form()
{
	int dform_x=Form.width/2-13;
	//типа окно
	if (!count) return;
	FOR (i=5;i<11;i++) DeleteButton(201+i); //удаляем кнопки под формой
	DrawFlatButton(dform_x,160,200,80,0,col_work, ""); //форма
	WriteText(dform_x+19,175,0x80,0,"Do you really want to delete");
	IF (strlen(#file_name)<28) 
		{
			WriteText(strlen(#file_name)*6+dform_x+20,190,0x80,0,"?");
			WriteText(dform_x+20,190,0x80,0,#file_name); //пишем имя
		}
	ELSE
		{
			WriteText(164+dform_x,190,0x80,0,"...?");
			ESI = 24;
			WriteText(dform_x+20,190,0,0,#file_name); //пишем имя
		}
	//кнопочки
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
	DeleteButton(301); DeleteButton(302); //удаляем кнопочки Yes/No
	Open_Dir(#path,1);
}    

void Paste()
{
	char new_copy_path[4096];
	int copy_rezult;
	
	IF (!copy_file) return; //отмена, еши ещё ничё не скопировали
	strcpy(#new_copy_path, #path);
	strcat(#new_copy_path, #copy_file+strrchr(#copy_file,'/'));
	if (!strcmp(#copy_file,#new_copy_path)) //если мы копируем и вставляем в одной и той же папке
	{
		strcpy(#new_copy_path, #path);
		strcat(#new_copy_path, "new_");
		strcat(#new_copy_path, #copy_file+strrchr(#copy_file,'/'));
	}
	copy_rezult = CopyFile(#copy_file,#new_copy_path);
	IF (copy_rezult!=0) //ошибка
	{
		Write_Error(copy_rezult);
		return;
	}
	IF (cut_active) //если мы выбрали вырезать
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
	DefineButton(192,curbtn*BUTTON_HEIGHT+57,onLeft(27,192),BUTTON_HEIGHT,curbtn+201+BT_HIDE+BT_NOFRAME,0xFFFFFF);
	if (rename==true)
	{
		strcpy(#temp, #path);
		strcpy(#edit_name, #file_name); //сохраняем новое имя файла, для того, чтобы его потом выделить
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
	Line_ReDraw(col_selec,curbtn);
}


void SelectFile(dword that_file)
{
	za_kadrom=curbtn=0; //вверх списка
   	Open_Dir(#path,ONLY_OPEN);
	strttl(that_file);
	for (i=count-1; i>=0; i--;)
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


inline fastcall void GoBack()   //вначале удаляем текущий путь, а потом копируем то, что осталось
{
	char cur_folder[4096];
	strcpy(#cur_folder, GetCurrentFolder());
	if (HistoryPath(GO_BACK)) SelectFile(#cur_folder);
}

stop:

char stak1[100];
char stak2[100];