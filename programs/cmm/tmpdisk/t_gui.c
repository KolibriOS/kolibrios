  /////////////////////////////////////////////////////////////////////////////////////////
 /////////////////////////                   GUI                      ////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

#include "..\lib\gui.h"
#include "..\lib\obj\box_lib.h"

#ifdef LANG_RUS
	unsigned char *but_text[]={
	"Добавить диск [Ins]",
	"Удалить диск [Del]",
	"Добавить [Ctrl+Enter]",
	0};
	
	?define INTRO_TEXT_1 "Здесь будет отображаться список"
	?define INTRO_TEXT_2 "виртуальных дисков в системе."
	?define INTRO_TEXT_3 "Попробуйте добавить один..."
	?define SIZE_TEXT "Размер:"
	?define NOTIFY_TEXT_NO_DISK    "'Для начала добавьте хотя бы один диск' -W"
	?define NOTIFY_TEXT_DISK_LIMIT "'Достигнут предел количества виртуальных дисков' -W"
	?define FREE_RAM_TEXT "Размер свободной оперативной памяти: "
	
#else
	unsigned char *but_text[]={
	"Add disk [Ins]",
	"Delete disk [Del]",
	"Add [Ctrl+Enter]",
	0};
	
	?define INTRO_TEXT_1 " There will be list of mounted"
	?define INTRO_TEXT_2 " virtual disks."
	?define INTRO_TEXT_3 " Try to add one..."
	?define SIZE_TEXT "Size:"
	?define NOTIFY_TEXT_NO_DISK    "'You need to have at least one disk' -W"
	?define NOTIFY_TEXT_DISK_LIMIT "'Reached the limit of the number of virtual disks' -W"
	?define FREE_RAM_TEXT "Free RAM size: "
#endif

struct path_string { unsigned char Item[10]; };
path_string disk_list[10];

dword devbuf;
char disk_num;
char selected;

proc_info Form;

unsigned char icons[] = FROM "icons.raw";
#define TOPPANELH 54
#define BOTPANELH 20

int	mouse_dd;
char new_disk_size[5];
edit_box edit_disk_size= {50,0,7,0xffffff,0x94AECE,0xFFFfff,0xffffff,0,4,#new_disk_size,#mouse_dd, 1000000000000010b};

void Main_Window()
{
	word id;
	int i, x;
	
   	mem_Init();
	load_dll(boxlib, #box_lib_init,0);
	GetSizeDisk();
	edit_disk_size.left = strlen(SIZE_TEXT)*9 + 10;
	SetEventMask(0x27);
	loop()
	{
		switch(WaitEvent()) 
		{
		case evMouse:
			if (!CheckActiveProcess(Form.ID)) break;
			edit_box_mouse stdcall (#edit_disk_size);
			break;
			
		case evButton:
			id=GetButtonID();               
			if (id==1) return;
			if (id==10) AddDisk();
			if (id==11) DelDisk();
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
			if ( !asm test edit_disk_size.flags, 2)	switch(key_scancode) 
			{
				case SCAN_CODE_TAB:
					edit_disk_size.flags=1000000000000010b;
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
					if (disk_num<>0) DelDisk();
					break;
				case SCAN_CODE_ENTER:
					OpenTmpDisk();
					break;
			}
			// PROCESS KEYS WHEN EDIT BOX ACTIVE		
			else switch(key_scancode) 
			{
				case SCAN_CODE_TAB:
					edit_disk_size.flags=1000000000000000b;
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
			system.color.get();
			DefineAndDrawWindow(170,150,314,270,0x74,system.color.work,"Virtual Disk Manager 0.65",0);
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window>2) break;

			DrawBar(0,0,  Form.cwidth,TOPPANELH, system.color.work);
			DrawBar(0,TOPPANELH, Form.cwidth,1,  system.color.work_graph);
			WriteText(6, 6, 0x90, system.color.work_text, SIZE_TEXT);
			WriteText(edit_disk_size.left + edit_disk_size.width + 12, 6, 0x90, system.color.work_text, "MB.");
			DrawEditBox(#edit_disk_size);
			for (i=0, x=6; i<2; i++, x+=strlen(but_text[i])*6+37)
			{
				DefineButton(x,29, strlen(but_text[i])*6+28,19, 10+i, system.color.work_button);
				_PutImage(x+3,32,  14,14,   i*14*14*3+#icons);
				WriteText(x+22,35, 0x80, system.color.work_button_text, but_text[i]);
			}		
			GetDisks();
			DrawTmpDisks();
		}
	}
}


void GetSizeDisk()
{
	int fr;
	fr = GetFreeRAM() / 5 * 2;
	fr = itoa(fr / 2048);
	strcpy(#new_disk_size, fr);
	edit_disk_size.size = edit_disk_size.pos = strlen(#new_disk_size);
	edit_box_draw stdcall (#edit_disk_size);
}


void OpenTmpDisk()
{
	unsigned char eol_param[10];
	if (!disk_num) return;
	strcpy(#eol_param, "/tmp#/1/");
	eol_param[4] = disk_list[selected].Item[3];
	RunProgram("/sys/File managers/Eolite", #eol_param);
}


void GetDisks()
{
	unsigned int j, fcount=30;
	unsigned char disk_name[10];
	
	mem_Free(devbuf);
	devbuf= mem_Alloc(32);
	ReadDir(0, devbuf, "/");
	fcount=ESDWORD[devbuf+8];
	mem_Free(devbuf);
	devbuf = mem_Alloc(fcount+1*304+32);
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


unsigned int disk_pos_x[]={13,13,13,85,85,85,157,157,157,229,229,229};
unsigned int disk_pos_y[]={60,95,130, 60, 95, 130, 60, 95,130, 60, 85,130};

void DrawTmpDisks()
{
	dword selection_color;
	dword selection_active = 0x0080FF;
	dword selection_inactive = 0x757489;
	char free_ram_text[60];
	byte i, real_id;
	int FreeRAM=GetFreeRAM()/1024;

	DrawBar(0,TOPPANELH+1, Form.cwidth,Form.cheight-TOPPANELH-BOTPANELH-2, 0xFFFFFF);
	DrawBar(0,Form.cheight-BOTPANELH-1, Form.cwidth,1, system.color.work_graph);
	DrawBar(0,Form.cheight-BOTPANELH, Form.cwidth,BOTPANELH, system.color.work);
	strcpy(#free_ram_text, FREE_RAM_TEXT);
	strcat(#free_ram_text, itoa(FreeRAM));
	strcat(#free_ram_text, " MB");
	WriteText(10, Form.cheight-13, 0x80, system.color.work_text, #free_ram_text);
	if (disk_num==0)
	{
		WriteText(17,65,    0x90, 0x777777, INTRO_TEXT_1);
		WriteText(17,65+15, 0x90, 0x777777, INTRO_TEXT_2);
		WriteText(17,65+42, 0x90, 0x777777, INTRO_TEXT_3);
		return;
	};
	if (selected>=disk_num) selected=disk_num-1; //restore selected
	for (i=0; i<10; i++) DeleteButton(20+i);
	for (i=0; i<disk_num; i++)
	{
		DefineButton(disk_pos_x[i], disk_pos_y[i], 65, 30, 20+i, 0xFFFfff);
		WriteText(disk_pos_x[i]+25,disk_pos_y[i]+2,  10110000b, 0, #disk_list[i].Item);
		real_id = disk_list[i].Item[3] - '0';
		WriteText(disk_pos_x[i]+25,disk_pos_y[i]+19, 0x80, 0x888888, ConvertSize(disk_sizes[real_id]));
		_PutImage(disk_pos_x[i]+5,disk_pos_y[i]+4, 14,14, 2*14*14*3+#icons);
		if (selected==i) {
			if ( !asm test edit_disk_size.flags, 2) selection_color = selection_active; else selection_color = selection_inactive;
			DrawWideRectangle(disk_pos_x[i], disk_pos_y[i], 65, 30, 2, selection_color);
			PutPixel(disk_pos_x[i], disk_pos_y[i], 0xFFFfff);
		}
	}
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
	GetDisks();
	DrawTmpDisks();
	GetSizeDisk();
}


void DelDisk()
{
	byte err;
	if (disk_num==0)
	{
		notify(NOTIFY_TEXT_NO_DISK);
		return;
	}
	param[0]='d';
	param[1]=disk_list[selected].Item[3];
	err = byte Console_Work();
	if ((err!=0) && (err<7)) notify(rezult_text[err]);
	pause(15);
	GetDisks();
	DrawTmpDisks();
}





