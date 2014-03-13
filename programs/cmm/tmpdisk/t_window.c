#include "..\lib\figures.h"
#include "..\lib\mem.h" 
#include "..\lib\dll.h"
#include "..\lib\lib.obj\box_lib.h"

#ifdef LANG_RUS
	unsigned char *but_text[]={
	"Добавить диск [Ins]",
	"Удалить диск [Del]",
	"Добавить [Ctrl+Enter]",
	0};
	
	?define INTRO_TEXT_1 "Здесь будет отображаться список"
	?define INTRO_TEXT_2 "виртуальных дисков в системе."
	?define INTRO_TEXT_3 "Попробуйте добавить один..."
	?define INTRO_TEXT_4 "Размер:"

	?define NOTIFY_TEXT_NO_DISK    "Для начала добавьте хотя бы один диск"
	?define NOTIFY_TEXT_DISK_LIMIT "Достигнут предел количества виртуальных дисков"
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
	?define INTRO_TEXT_4 "Size:"

	?define NOTIFY_TEXT_NO_DISK    "You need to have at least one disk"
	?define NOTIFY_TEXT_DISK_LIMIT "Reached the limit of the number of virtual disks"
	?define FREE_RAM_TEXT "Free RAM size: "
#endif

struct path_string { unsigned char Item[256]; };
path_string disk_list[40];

dword devbuf;
int disk_num;
int selected;

system_colors sc;
proc_info Form;

unsigned char dsize[30];

unsigned char icons[14*56] = FROM "icons.raw";
#define TOPPANELH 50
#define BOTPANELH 20

int	mouse_dd;
char disk_size[30];
edit_box edit_disk_size= {50,0,5,0xffffff,0x94AECE,0x000000,0xffffff,0,4,#disk_size,#mouse_dd, 1000000000000010b};

void Main_Window()
{
	unsigned int id, key, err;
	int i, x;
	
   	mem_Init();
	if (load_dll2(boxlib, #box_lib_init,0)!=0) notify("Error while loading GUI library /sys/lib/boxlib.obj");
	GetSizeDisk();
	edit_disk_size.left = strlen(INTRO_TEXT_4)*6 + 10;
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
			if (id==1) ExitProcess();
			if (id==10) AddDisk();
			if (id==11) //del
			{
				_DEL_DISK:
				if (disk_num==0)
				{
					notify(NOTIFY_TEXT_NO_DISK);
					break;
				}
				param[0]='d';
				param[1]=disk_list[selected].Item[3];
				err = Console_Work();
				if ((err!=0) && (err<7)) notify(rezult_text[err]);
				pause(15);
				GetDisks();
				DrawTmpDisks();
			}
			if (id>=20)
			{
				if (selected==id-20) OpenTmpDisk();
				selected=id-20;
				DrawTmpDisks();
			}
            break;
        case evKey:
			key = GetKey();
			if (key==9)
			{
				if ( !asm test edit_disk_size.flags, 2) edit_disk_size.flags=1000000000000010b;
				else edit_disk_size.flags=1000000000000000b;
				edit_box_draw stdcall (#edit_disk_size);
			}				
			if ( asm test edit_disk_size.flags, 2)
			{
				if (key==13)
				{
					edit_disk_size.flags=1000000000000000b;
					edit_box_draw stdcall (#edit_disk_size);
				}
				EAX=key<<8;
				edit_box_key stdcall(#edit_disk_size);
				break;
			}
			if (key==182) if (disk_num<>0) goto _DEL_DISK;
			if (key==185) AddDisk();
			if (key==13) OpenTmpDisk();
			if (key==178)
			{
				if (selected==0) break;
				selected--;
				DrawTmpDisks();
			}
			if (key==177)
			{
				if (selected+2>disk_num) break;
				selected++;
				DrawTmpDisks();
			}
			if (key==176)
			{
				if (selected<3) break;
				selected-=3;
				DrawTmpDisks();
			}
			if (key==179)
			{
				if (selected+4>disk_num) break;
				selected+=3;
				DrawTmpDisks();
			}
			//EAX=key<<8;
			//edit_box_key stdcall(#edit_disk_size);
			break;
         case evReDraw:			
			sc.get();
			DefineAndDrawWindow(170,150,314,270,0x74,sc.work,"Virtual Disk Manager 0.5",0);
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window>2) break;

			DrawBar(0,0,  Form.cwidth,TOPPANELH, sc.work);
			DrawBar(0,TOPPANELH, Form.cwidth,1,  sc.work_graph);
			WriteText(6, 9, 0x80, sc.work_text, INTRO_TEXT_4);
			WriteText(edit_disk_size.left + edit_disk_size.width + 8, 9, 0x80, sc.work_text, "MB.");
			edit_box_draw stdcall (#edit_disk_size);
			for (i=0, x=6; i<2; i++, x+=strlen(but_text[i])*6+37)
			{
				DefineButton(x,25, strlen(but_text[i])*6+28,19, 10+i, sc.work_button);
				_PutImage(x+3,28,  14,14,   i*14*14*3+#icons);
				WriteText(x+22,31, 0x80, sc.work_button_text, but_text[i]);
			}			
			GetDisks();
			DrawTmpDisks();
		}
	}
}

void GetSizeDisk()
{
	int fr;
	fr = GetFreeRAM() / 5;
	fr = itoa(fr / 2048);
	strcpy(#disk_size, fr);
	edit_disk_size.size = edit_disk_size.pos = strlen(#disk_size);
	edit_box_draw stdcall (#edit_disk_size);
}

void OpenTmpDisk()
{
	unsigned char eol_param[256];
	if (!disk_num) return;
	strcpy(#eol_param, "/tmp#/1/");
	eol_param[4]=disk_list[selected].Item[3];
	RunProgram("/sys/File managers/Eolite", #eol_param);
}


void GetDisks()
{
	unsigned int j, fcount=30;
	unsigned char disk_name[256];
	
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


unsigned int disk_pos_x[]={13,13,13,83,83,83,153,153,153,223,223,223};
unsigned int disk_pos_y[]={60,85,110,60,85,110,60,85,110,60,85,110};

void DrawTmpDisks()
{
	char free_ram_text[60];
	int i,FreeRAM=GetFreeRAM()/1024;
	DrawBar(0,51, Form.cwidth,Form.cheight-TOPPANELH-BOTPANELH-2, 0xFFFFFF);
	DrawBar(0,Form.cheight-BOTPANELH-1, Form.cwidth,1, sc.work_graph);
	DrawBar(0,Form.cheight-BOTPANELH, Form.cwidth,BOTPANELH, sc.work);
	strcpy(#free_ram_text, FREE_RAM_TEXT);
	strcat(#free_ram_text, itoa(FreeRAM));
	strcat(#free_ram_text, " MB");
	WriteText(10, Form.cheight-13, 0x80, sc.work_text, #free_ram_text);
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
		DefineButton(disk_pos_x[i], disk_pos_y[i], 60, 20, 20+i, 0xFFFfff);
		WriteText(disk_pos_x[i]+25,disk_pos_y[i]+6, 0x90, 0, #disk_list[i].Item);
		_PutImage(disk_pos_x[i]+5,disk_pos_y[i]+4, 14,14, 3*14*14*3+#icons);
		if (selected==i) DrawRectangle(disk_pos_x[i], disk_pos_y[i], 60-1, 20-1, 0x00459A);
	}
}

void AddDisk()
{
	unsigned int i, j, err;
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
	strcat(#param, #disk_size); 
	err = Console_Work();
	if ((err!=0) && (err<7)) notify(rezult_text[err]);
	pause(5);
	GetDisks();
	DrawTmpDisks();
	GetSizeDisk();
}





