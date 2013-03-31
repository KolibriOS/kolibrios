#define MEMSIZE 0xA1000
#include "..\lib\kolibri.h"
#include "..\lib\strings.h"
#include "..\lib\figures.h"
#include "..\lib\encoding.h"
#include "..\lib\file_system.h"
#include "..\lib\mem.h"
#include "..\lib\dll.h"
#include "..\lib\copyf.h"
//*.obj libraries
#include "..\lib\lib.obj\box_lib.h"
#include "..\lib\lib.obj\libio_lib.h"
#include "..\lib\lib.obj\libimg_lib.h"
#include "..\lib\lib.obj\truetype.h"


#define LOGOW 16
#define LOGOH 16
#define BLACK_H 40
#define TEXTX 20
#define WIN_W 500
#define WIN_H 350

unsigned char logo[LOGOW*LOGOH*3]= FROM "img\logo.raw";

proc_info Form;
system_colors sc;
char dialog;
enum {
	HALLO,
	INSTALL,
	END
};

int DefineWindow(dword wtitle, wbutton)
{
	sc.get();
	DefineAndDrawWindow(GetScreenWidth()-WIN_W/2,GetScreenHeight()-WIN_H/2-30, WIN_W+9, WIN_H+GetSkinHeight()+4, 0x74,0xFFFfff); 
	DrawTitle("KolibriN 8.2a Setup");
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return 0; //rolled_up

	DrawBar(0, 0, Form.cwidth, BLACK_H, 0);
	_PutImage(BLACK_H-LOGOW/2, BLACK_H-LOGOH/2, LOGOW,LOGOH, #logo);
	WriteTextB(BLACK_H-LOGOW + LOGOW, BLACK_H-6/2, 0x90, 0xFFFfff, wtitle);
	DrawBar(0, BLACK_H, Form.cwidth, Form.cheight-BLACK_H, 0xFFFfff);
	DrawCaptButton(Form.cwidth-107, Form.cheight-40, 90, 24, 10, sc.work_button, sc.work_button_text,wbutton);
	return 1;
}

#include "tmp_add.c"
#include "hallo.c";
#include "installation.c";

void main()
{
	mem_Init();
	program_path[strrchr(#program_path, '/')] = '\0';
	HalloLoop();
}


stop: