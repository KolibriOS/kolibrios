#define MEMSIZE 4096*10

#include "../lib/io.h"
#include "../lib/gui.h"

void main()
{
	word id;
	dword file;
	io.dir.load(0,DIR_ONLYREAL);
	loop() switch(WaitEvent())
	{
		case evButton:
			id=GetButtonID();               
			if (id==1) ExitProcess();
			break;
	  
		case evKey:
			GetKeys();
			if (key_scancode == SCAN_CODE_ESC ) ExitProcess();
			break;
		 
		case evReDraw:
			draw_window();
			break;
	}
}
void draw_window()
{
	proc_info Form;
	int i;
	DefineAndDrawWindow(215,100,350,300,0x34,0xFFFFFF,"Window header");
	GetProcessInfo(#Form, SelfInfo);
	for (i=0; i<io.dir.count; i++)
	{
		WriteText(5,i*8+3,0x80,0xFF00FF,io.dir.position(i));
	}
	DrawCaptButton(100, 10, 100, 22, 22, 0xCCCccc, 0x000000, "Button");
	WriteText(100,50,0x80,0,"Textline small");
	WriteText(100,70,0x90,0,"Textline big");
	DrawBar(100, 110, 100, 100, 0x66AF86);
}