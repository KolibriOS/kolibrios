/*
GAME CENTER v1.0
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

system_colors sc;
proc_info Form;

struct link {
	char *name;
	char *path;
	int icon;
};

struct link games[] = {
	"Bomber", "/kolibrios/games/bomber/bomber", 35,
	"DOOM1", "/kolibrios/games/doom1/doom", 43,
	"DOOM2", "/kolibrios/games/doom2/doom", 43,
	"Fara", "/kolibrios/games/fara/fara", 42,
	"JumpBump", "/kolibrios/games/jumpbump/jumpbump", 35,
	"Loderunner", "/kolibrios/games/LRL/LRL", 41,
	"BabyPainter", "/kolibrios/games/baby painter", 35,
	"Knight", "/kolibrios/games/knight", 35,
	"Pinton", "/kolibrios/games/piton", 32,

	"15", "/sys/games/15", 34,
	"Arcanii", "/sys/games/arcanii", 12,
	"Ataka", "/sys/games/ataka", 35,
	"C4", "/sys/games/c4", 35,
	"Checkers", "/sys/games/checkers", 20,
	"Clicks", "/sys/games/clicks", 18,
	"FNumbers", "/sys/games/FindNumbers", 35,
	"Flood-It", "/sys/games/flood-it", 27,
	"Freecell", "/sys/games/freecell", 35,
	"Gomoku", "/sys/games/gomoku", 24,
	"Invaders", "/kolibrios/games/invaders", 35,
	"Klavisha", "/sys/games/klavisha", 35,
	"Kosilka", "/sys/games/kosilka", 23,
	"Lines", "/sys/games/lines", 35,
	"MBlocks", "/sys/games/mblocks", 11,
	"Megamaze", "/sys/games/megamaze", 35,
	"Mine", "/sys/games/mine", 14,
	"Square", "/sys/games/msquare", 35,
	"Padenie", "/sys/games/padenie", 35,
	"Phenix", "/sys/games/phenix", 35,
	"Pipes", "/sys/games/pipes", 26,
	"Pong", "/sys/games/pong", 12,
	"Pong3", "/sys/games/pong3", 12,
	"Reversi", "/sys/games/reversi", 35,
	"Rforces", "/sys/games/rforces", 35,
	"Rsquare", "/sys/games/rsquare", 35,
	"Snake", "/sys/games/snake", 32,
	"Sq game", "/sys/games/sq_game", 35,
	"Sudoku", "/sys/games/sudoku", 25,
	"Sea War", "/sys/games/SW", 35,
	"Tanks", "/sys/games/tanks", 35,
	"Tetris", "/sys/games/tetris", 35,
	"Whowtbam", "/sys/games/whowtbam", 35,
	"Xonix", "/sys/games/xonix", 21,
	0
};

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
	skin.Load();
	
	loop()
   {
      switch(WaitEvent())
      {
         case evButton:
            id=GetButtonID();               
            if (id==1) ExitProcess();
            if (id>=100) RunProgram(games[id-100].path, "");
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


void draw_window()
{
	int row, col, col_max=8;
	int col_w=68, col_h=70;
	int tmp,y=25;
	DrawBar(0,0,Form.cwidth, y-1, sc.work);
	DrawBar(0,y-1, Form.cwidth, 1, sc.work_graph);
	DrawBar(0,y, Form.cwidth, Form.cheight-y, 0xF3F3F3);
	WriteTextB(Form.cwidth/2-70, 9, 0x90, sc.work_text, "KolibriOS Game Center");

	y += 7;
	for (col=0, row=0; games[row*col_max+col].name!=0; col++)
	{
		if (col==col_max) {
			row++;
			col=0;
		}
		DefineButton(col*col_w+6,row*col_h+y,col_w,col_h,row*col_max+col+100+BT_HIDE,0);
		tmp = col_w/2;
		img_draw stdcall(skin.image, col*col_w+tmp-10, row*col_h+5+y, 32, 32, 0, games[row*col_max+col].icon*32);
		WriteTextCenter(col*col_w+7,row*col_h+47+y,col_w,0xD4D4d4,games[row*col_max+col].name);
		WriteTextCenter(col*col_w+6,row*col_h+46+y,col_w,0x000000,games[row*col_max+col].name);
	}
}



stop:
