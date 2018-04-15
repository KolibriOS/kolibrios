#ifndef INCLUDE_WINDOW_H
#define INCLUDE_WINDOW_H
#include "../lib/gui.h"


#define WINDOW_NORMAL 0x34
:struct window
{
	void create();
	dword left,top,width,height;
	dword caption,type,background;
	dword onbutton,onkey,ondraw;
	proc_info Form;
};

:void window::create()
{
	word id=0;
	IF(!caption)caption = "Window";
	IF(!width)width=350;
	IF(!height)height=300;
	IF(!type)type = WINDOW_NORMAL;
	IF(!background)background = 0xDED7CE;
	
	loop() switch(WaitEvent())
	{
		case evButton:
			id=GetButtonID();  
			IF(onbutton)onbutton(id);
			IF (id==1) ExitProcess();
			break;
	  
		case evKey:
			GetKeys();
			IF(onkey)onbutton(key_scancode);
			//if (key_scancode == SCAN_CODE_ESC ) ExitProcess();
			break;
		 
		case evReDraw:
			GetProcessInfo(#Form, SelfInfo);
			DefineAndDrawWindow(left,top,width,height,type,background,caption,0);
			
			IF(ondraw)ondraw();
			break;
	}
	
}

#endif