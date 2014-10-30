/*
GAME CENTER v1.5
*/

#define MEMSIZE 0x3E80
#include "..\lib\kolibri.h" 
#include "..\lib\strings.h" 
#include "..\lib\mem.h" 
#include "..\lib\file_system.h"
#include "..\lib\dll.h"
#include "..\lib\figures.h"

#include "..\lib\lib.obj\libio_lib.h"
#include "..\lib\lib.obj\libimg_lib.h"
#include "..\lib\lib.obj\libini.h"

system_colors sc;
proc_info Form;
int run_id, enum_i;

struct struct_skin {
	dword image, w, h;
	int Load();
} skin;

int struct_skin::Load()
{
	int i;
	dword image_data;
	skin.image = load_image("/sys/iconstrp.png");
	if (!skin.image) notify("'iconstrp.png not found' -E");
	skin.w = DSWORD[skin.image+4];
	skin.h = DSWORD[skin.image+8];
	image_data = DSDWORD[skin.image+24];
	sc.get();

	for (i=0; i<w*h*4; i+=4)
	{
		if (DSDWORD[image_data + i]==0) DSDWORD[image_data + i] = 0xF3F3F3;
	}
}


void main()
{   
	int id, key;
	mem_Init();
	if (load_dll2(libio, #libio_init,1)!=0) notify("Error: library doesn't exists - libio");
	if (load_dll2(libimg, #libimg_init,1)!=0) notify("Error: library doesn't exists - libimg");
	if (load_dll2(libini, #lib_init,1)!=0) notify("Error: library doesn't exists - libini");
	skin.Load();
	
	loop()
   {
      switch(WaitEvent())
      {
         case evButton:
            id=GetButtonID();               
            if (id==1) ExitProcess();
            if (id>=100)
            {
            	run_id = id - 100;
            	enum_i = 0;
            	ini_enum_keys stdcall ("/sys/settings/games.ini", "Games", #run_game);
            }
			break;
      
        case evKey:
			key = GetKey();
			break;
         
         case evReDraw:
			sc.get();
			DefineAndDrawWindow(215,100,568,390+60+GetSkinHeight(),0x74,sc.work," ");
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window>2) break;
			draw_window();
			break;
      }
   }
}

byte run_game(dword key_value, key_name, sec_name, f_name)
{
	if (run_id==enum_i)
	{
		ESBYTE[key_value + strchr(key_value, ',') - 1] = 0;
		RunProgram(key_value, '');
		return 0;
	}
	enum_i++;
	return 1;
}


int col_max, col_w, col_h, y;
int row, col;

byte key_process(dword key_value, key_name, sec_name, f_name)
{
	int tmp;
	int icon_n;


	if (col==col_max) {
		row++;
		col=0;
	}
	DefineButton(col*col_w+6,row*col_h+y,col_w,col_h-10,row*col_max+col+100+BT_HIDE,0);
	tmp = col_w/2;
	icon_n = atoi(key_value + strchr(key_value, ','));
	img_draw stdcall(skin.image, col*col_w+tmp-10, row*col_h+5+y, 32, 32, 0, icon_n*32);
	WriteTextCenter(col*col_w+7,row*col_h+47+y,col_w,0xD4D4d4,key_name);
	WriteTextCenter(col*col_w+6,row*col_h+46+y,col_w,0x000000,key_name);
	col++;
	return 1;
}

void draw_window()
{
	y = 25;
	DrawBar(0,0,Form.cwidth, y-1, sc.work);
	DrawBar(0,y-1, Form.cwidth, 1, sc.work_graph);
	DrawBar(0,y, Form.cwidth, Form.cheight-y, 0xF3F3F3);
	WriteTextB(Form.cwidth/2-70, 9, 0x90, sc.work_text, "KolibriOS Game Center");
	y += 2;
	col_max=8;
	col_w=68;
	col_h=70;
	col = row = 0;

	ini_enum_keys stdcall ("/sys/settings/games.ini", "Games", #key_process);
}



stop:
