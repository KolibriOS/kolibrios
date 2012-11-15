#include "..\lib\mem.h" 
#include "..\lib\dll.h"
#include "..\lib\edit_box_lib.h"


#ifdef LANG_RUS
	char *but_text[]={
	"Добавить диск"w,
	"Удалить диск"w,
	"Помощь"w,
	0};
#else
	char *but_text[]={
	"Add disk",
	"Delete disk",
	"Help",
	0};
#endif

dword icons[14*56] = FROM "icons.raw";

struct path_string {
unsigned char Item[256];
};

path_string disk_list[40];
dword devbuf;
int disk_num;
int selected;

system_colors sc;
proc_info Form;

int mouse_dd;
unsigned char dsize[30];
edit_box edit1= {200,20,16,0xffffff,0x94AECE,0x94AECE,0x94AECE,0,248,#dsize,#mouse_dd,100000000000010b};



void Main_Window()
{
	unsigned int id;
	unsigned char key;
	unsigned int i, j, bbreak;
	
   	mem_Init();
	if (load_dll2(boxlib, #edit_box_draw,0)!=0)
	{
		notify("error: library doesn't exists /rd/1/lib/box_lib.obj");
		ExitProcess();
	}
	SetEventMask(0x27);
	loop()
	{
		switch(WaitEvent()) 
		{
		case evMouse:
			//edit_box_mouse stdcall (#edit1);
			break;
		case evButton:
            id=GetButtonID();               
			if (id==1) ExitProcess();
			if (id==10) //add
			{
				_ADD_DISK:
				if (disk_num>=10)
				{
					notify("Достигнут предел количества виртуальных дисков"w);
					break;
				}
				param[0]='a';
				//
				for (i=0; i<9; i++)
				{
					bbreak = true;
					for (j=0; j<=disk_num; j++)
					{
						if (i+48==disk_list[j].Item[3]) bbreak=false;
					}
					if (bbreak) break;
				}
				param[1]=i+48;
				Console_Work();
				Pause(5);
				GetDisks();
				DrawTmpDisks();
			}
			if (id==11) //del
			{
				_DEL_DISK:
				if (disk_num==0)
				{
					notify("Для начала добавьте хотя бы один диск"w);
					break;
				}
				param[0]='d';
				param[1]=disk_list[selected].Item[3];
				Console_Work();
				Pause(15);
				GetDisks();
				DrawTmpDisks();
			}
			if (id==12) //help
			{
				notify("Раздел в разработке"w);
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
			if (key==182) if (disk_num<>0) goto _DEL_DISK;
			if (key==14) goto _ADD_DISK;
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
			if (key==013)
			{
				OpenTmpDisk();
			}
			break;
         case evReDraw:
			Draw_Window();
		}
	}
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
unsigned int disk_pos_y[]={40,65,90,40,65,90,40,65,90,40,65,90};
void DrawTmpDisks()
{
	int i;
	DrawBar(0,31, Form.width-9,Form.height-GetSkinHeight()-5-30, 0xFFFFFF);
	if (disk_num==0)
	{
		WriteText(17,45,    0x90, 0x777777, "Здесь будет отображаться список"w, 0);
		WriteText(17,45+15, 0x90, 0x777777, "виртуальных дисков в системе."w, 0);
		WriteText(17,45+42, 0x90, 0x777777, "Попробуйте добавить один..."w, 0);
		return;
	};
	if (selected>=disk_num) selected=disk_num-1; //восстанавливает выделение - хорошая фича
	for (i=0; i<disk_num; i++)
	{
		DefineButton(disk_pos_x[i], disk_pos_y[i], 60, 20, 20+i, 0xFFFfff);
		WriteText(disk_pos_x[i]+25,disk_pos_y[i]+6, 0x90, 0, #disk_list[i].Item, 0);
		_PutImage(disk_pos_x[i]+5,disk_pos_y[i]+4, 14,14, 3*14*14*3+#icons);
		if (selected==i) DrawRegion(disk_pos_x[i], disk_pos_y[i], 60-1, 20-1, 0x00459A);
	}
}

void Draw_Window()
{	
	int i, x;
	
	sc.get();
	DefineAndDrawWindow(170,150,314,250,0x74,sc.work,"Virtual Disk Manager 0.35");
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return; //если свернуто в заголовок, ничего не рисуем
	
	//рисуем панель
	DrawBar(0,0,  Form.width-9,30, sc.work);
	DrawBar(0,30, Form.width-9,1,  sc.work_graph);
	x=6;
	for (i=0; i<3; i++)
	{
		DefineButton(x,5, strlen(but_text[i])*6+28,19, 10+i, sc.work_button);
		_PutImage(x+3,8,  14,14,   i*14*14*3+#icons);
		WriteText(x+22,11, 0x80, sc.work_button_text, but_text[i], 0);
		x+=strlen(but_text[i])*6+37; 
	}
	
	//получаем список доступных дисков
	GetDisks();
	//выводим доступные диски
	DrawTmpDisks();
	//дополнительная панель
}






