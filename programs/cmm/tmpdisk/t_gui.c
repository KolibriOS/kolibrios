  /////////////////////////////////////////////////////////////////////////////////////////
 /////////////////////////                   GUI                      ////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

#include "..\lib\gui.h"
#include "..\lib\obj\box_lib.h"

//===================================================//
//                                                   //
//                   TRANSLATIONS                    //
//                                                   //
//===================================================//

#ifdef LANG_RUS
?define T_WIN_TITLE "TmpDisk 0.80: Управление дисками в ОЗУ"
?define T_ADD_DISK "Добавить диск [Ins]"	
?define T_DELETE_DISK "Удалить диск [Del]"
?define INTRO_TEXT_1 "Ни одного диска пока не добавлено."
?define INTRO_TEXT_2 "Пожалуйста, добавьте tmp0, он необходим."
?define NEW_DISK_SIZE "Новый диск:"
?define NEW_DISK_SIZE_W 11*8
?define NOTIFY_TEXT_NO_DISK    "'Для начала добавьте хотя бы один диск' -W"
?define NOTIFY_SYSTEM_DISK0    "'Диск с номером 0 является системным и удалять его не рекомендуется. 
В случае, если вы точно знаете, что делаете, удалить его можно с зажатой клавишей SHIFT.' -W"
?define NOTIFY_TEXT_DISK_LIMIT "'Достигнут предел количества виртуальных дисков' -W"
?define FREE_RAM_TEXT "Доступно %i MБ"
	
#else
#ifdef LANG_SPA
?define T_WIN_TITLE "TmpDisk 0.80: Gestor de discos RAM"
?define T_ADD_DISK "Anadir disco [Ins]"
?define T_DELETE_DISK "Eliminar disco [Del]"
?define INTRO_TEXT_1 "Aun no hay discos anadidos."
?define INTRO_TEXT_2 "Anada tmp0, es imprescindible."
?define NEW_DISK_SIZE "Disco nuevo:"
?define NEW_DISK_SIZE_W 12*8
?define NOTIFY_TEXT_NO_DISK    "'Necesitas tener al menos un disco' -W"
?define NOTIFY_SYSTEM_DISK0    "'El disco numero 0 es un disco del sistema. No se recomienda eliminarlo.
Si sabes lo que haces, puedes eliminarlo con la tecla SHIFT pulsada.' -W"
?define NOTIFY_TEXT_DISK_LIMIT "'Alcanzado el limite de discos virtuales' -W"
?define FREE_RAM_TEXT "RAM libre: %i MB"
#else
?define T_WIN_TITLE "TmpDisk 0.80: RAM Disk Manager"
?define T_ADD_DISK "Add disk [Ins]"
?define T_DELETE_DISK "Delete disk [Del]"
?define INTRO_TEXT_1 "No disks added yet."
?define INTRO_TEXT_2 "Please add tmp0 as it is crucial."
?define NEW_DISK_SIZE "New disk:"
?define NEW_DISK_SIZE_W 9*8
?define NOTIFY_TEXT_NO_DISK    "'You need to have at least one disk' -W"
?define NOTIFY_SYSTEM_DISK0    "'Disc number 0 is a system disk. It is not recommended to delete it.
In case when you know what you are doing you can delete it with the SHIFT key pressed.' -W"
?define NOTIFY_TEXT_DISK_LIMIT "'Reached the limit of the number of virtual disks' -W"
?define FREE_RAM_TEXT "Free RAM: %i MB"
#endif
#endif

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

struct path_string { unsigned char Item[10]; };
path_string disk_list[10];

dword devbuf;
char disk_num;
char selected;

#define WIN_W 410
#define WIN_H 268
#define BTN_W 190
#define PANEL_X 8
#define PANEL_Y 84
#define PANEL_W 393
#define PANEL_H 175

#define SELECTION_ACTIVE 0x0080FF
#define SELECTION_INACTIVE 0x757489

char new_disk_size[5];
edit_box edit_disk_size= {40,NEW_DISK_SIZE_W+14,13,0xffffff,0x94AECE,0xFFFfff,0xffffff,0x10000000,
	sizeof(new_disk_size)-1,#new_disk_size,0, ed_focus+ed_figure_only};

char tmp_path[8] = "/tmp#/1";
char disk_size_text[16];

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void Main_Window()
{
	word id;
	proc_info Form;

	load_dll(boxlib, #box_lib_init,0);
	GetNewSizeDisk();
	@SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
	loop()
	{
		switch(@WaitEvent())
		{
		case evMouse:
			edit_box_mouse stdcall (#edit_disk_size);
			break;
			
		case evButton:
			id = @GetButtonID();
			if (id==1) return;
			if (id==10) AddDisk();
			if (id==11) {
				GetKeys();
				DelDisk();
			}
			if (id>=20)
			{
				if (selected==id-20) OpenTmpDisk();
				selected=id-20;
				DrawTmpDisks();
			}
            break;
        case evKey:
			GetKeys();
			// PROCESS KEYS WHEN EDIT BOX INACTIVE
			if (!edit_disk_size.flags&ed_focus)	switch(key_scancode)
			{
				case SCAN_CODE_TAB:
					edit_disk_size.flags = ed_figure_only + ed_focus;
					edit_box_draw stdcall (#edit_disk_size);
					DrawTmpDisks();
					break;
				case SCAN_CODE_UP:
					if (selected==0) break;
					selected--;
					DrawTmpDisks();
					break;
				case SCAN_CODE_DOWN:
					if (selected+2>disk_num) break;
					selected++;
					DrawTmpDisks();
					break;
				case SCAN_CODE_LEFT:
					if (selected<3) break;
					selected-=3;
					DrawTmpDisks();
					break;
				case SCAN_CODE_RIGHT:
					if (selected+4>disk_num) break;
					selected+=3;
					DrawTmpDisks();
					break;
				case SCAN_CODE_INS:
					AddDisk();
					break;
				case SCAN_CODE_DEL:
					DelDisk();
					break;
				case SCAN_CODE_ENTER:
					OpenTmpDisk();
					break;
			}
			// PROCESS KEYS WHEN EDIT BOX ACTIVE
			else switch(key_scancode) 
			{
				case SCAN_CODE_TAB:
					edit_disk_size.flags = ed_figure_only;
					edit_box_draw stdcall (#edit_disk_size);
					DrawTmpDisks();
					break;
				case SCAN_CODE_ENTER:
				case SCAN_CODE_INS:
					AddDisk();
					break;
				default:
					EAX = key_editbox;
					edit_box_key stdcall(#edit_disk_size);
					break;
			}
			break;
         case evReDraw:
			sc.get();
			DefineAndDrawWindow(screen.w-WIN_W/2,screen.h-WIN_H/2,
				WIN_W+9, WIN_H+skin_h+4,0x74,sc.work,T_WIN_TITLE,0);
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window&ROLLED_UP) break;
			DrawWindowContent();
		}
	}
}

void DrawWindowContent()
{
	char free_ram_text[60];

	DrawBar(0,0, WIN_W,WIN_H, sc.work);
	WriteText(7, 16, 0x90, sc.work_text, NEW_DISK_SIZE);
	DrawRectangle(edit_disk_size.left-1, edit_disk_size.top-1, edit_disk_size.width+2, 23,sc.line);
	edit_box_draw stdcall (#edit_disk_size);
	DrawCaptButton(8, 48, BTN_W, 23, 10, 0x45975E, 0xFFFfff, T_ADD_DISK);
	DrawCaptButton(211, 48, BTN_W, 23, 11, 0xC75C54, 0xFFFfff, T_DELETE_DISK);
	miniprintf(#free_ram_text, FREE_RAM_TEXT, GetFreeRAM()/1024);
	WriteText(NEW_DISK_SIZE_W+62, 16, 0x90, sc.work_text, "MB");
	WriteText(210, 16, 0x90, sc.work_text, #free_ram_text);
	GetDisks();
	DrawTmpDisks();
}


void GetNewSizeDisk()
{
	int fr;

	fr = GetFreeRAM() / 5 * 2;
	fr = itoa(fr / 2048);
	edit_box_set_text stdcall (#edit_disk_size, fr);
}


void OpenTmpDisk()
{
	if (disk_num) {
		tmp_path[4] = disk_list[selected].Item[3];
		RunProgram("/sys/File managers/Eolite", #tmp_path);
	}
}


void GetDisks()
{
	unsigned int j, fcount=30;
	unsigned char disk_name[10];
	
	mem_Free(devbuf);
	devbuf=mem_Alloc(32);
	ReadDir(0, devbuf, "/");
	fcount=ESDWORD[devbuf+8];
	mem_Free(devbuf);
	devbuf=mem_Alloc(fcount+1*304+32);
	ReadDir(fcount, devbuf, "/");
	
	disk_num=0;
	for (j=0; j<fcount; j++;)
	{
		strcpy(#disk_name, j*304+devbuf+72);
		if (disk_name[0]!='t') continue;
		strcpy(#disk_list[disk_num].Item, #disk_name);
		disk_num++;
	}
	if (disk_num>12) disk_num=12;
}


unsigned int disk_pos_x[]={21,21,21,116,116,116,211,211,211,306,306,306};
unsigned int disk_pos_y[]={97,151,205, 97,151,205, 97,151,205, 97,151,205};

void DrawTmpDisks()
{
	dword selection_color, x, y;
	byte i;

	DrawBar(PANEL_X, PANEL_Y, PANEL_W, PANEL_H, sc.light);
	DrawRectangle(PANEL_X, PANEL_Y, PANEL_W, PANEL_H, sc.line);
	if (disk_num==0)
	{
		WriteText(PANEL_X+12, PANEL_Y+12, 0x90, sc.work_text, INTRO_TEXT_1);
		WriteText(PANEL_X+12, PANEL_Y+30, 0x90, 0xFF000d, INTRO_TEXT_2);
		return;
	};
	if (selected>=disk_num) selected=disk_num-1; //restore selected
	for (i=0; i<10; i++)
	{
		DeleteButton(20+i);
		if (i>=disk_num) continue;
		x = disk_pos_x[i]; y = disk_pos_y[i];
		DefineButton(x, y, 82, 41, 20+i, 0xFFFfff);
		WriteText(x+35,y+6,  0x90, 0x222222, #disk_list[i].Item);
		WriteText(x+37,y+24, 0x80, 0x555555, 
			GetDiskSizeInMb(disk_list[i].Item[3]));
		draw_icon_16(x+10,y+10, 51);
		if (selected==i) {
			if (edit_disk_size.flags & ed_focus)
				selection_color = SELECTION_INACTIVE;
			else
				selection_color = SELECTION_ACTIVE;
			DrawWideRectangle(x, y, 82, 41, 2, selection_color);
			PutPixel(x, y, sc.light);
		}
	}
}

:dword GetDiskSizeInMb(dword disk_n)
{
	BDVK bdvk;
	tmp_path[4]=disk_n;

	GetFileInfo(#tmp_path, #bdvk);
	miniprintf(#disk_size_text, "%i MB", bdvk.sizelo / 1048576);
	return #disk_size_text;
}

void AddDisk()
{
	byte i, j, err;

	if (disk_num>=10)
	{
		notify(NOTIFY_TEXT_DISK_LIMIT);
		return;
	}
	
	param[0]='a';
	for (i=0; i<9; i++)
	{
		for (j=0; j<=disk_num; j++)
		{
			if (i+48==disk_list[j].Item[3]) continue 1;
		}
		break;
	}
	
	param[1]=i+48;
	param[2]='s';
	param[3]='\0';
	strcat(#param, #new_disk_size); 
	err = Console_Work();
	if ((err!=0) && (err<7)) notify(rezult_text[err]);
	pause(5);
	GetNewSizeDisk();
	DrawWindowContent();
}


void DelDisk()
{
	byte err;

	if (disk_num==0)
	{
		notify(NOTIFY_TEXT_NO_DISK);
		return;
	}
	if (disk_list[selected].Item[3]=='0') && (! key_modifier & KEY_LSHIFT) && (! key_modifier & KEY_RSHIFT)
	{
		notify(NOTIFY_SYSTEM_DISK0);
		return;
	}
	param[0]='d';
	param[1]=disk_list[selected].Item[3];
	err = byte Console_Work();
	if ((err!=0) && (err<7)) notify(rezult_text[err]);
	pause(15);
	GetNewSizeDisk();
	DrawWindowContent();
}





