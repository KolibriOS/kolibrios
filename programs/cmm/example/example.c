#define MEMSIZE 0x3E80

#include "../lib/io.h"
#include "../lib/draw.h"

void main()
{   
	int id, key;
	mem_Init();
	io.set("/sys/RUN",ATR_HIDDEN);
	loop()
   {
      switch(WaitEvent())
      {
         case evButton:
            id=GetButtonID();               
            if (id==1) ExitProcess();
			break;
      
        case evKey:
			key = GetKey();
			if (key==013){ //Enter
				WriteText(50,90,0x80,0xFF00FF,"Pressed Enter");
			}
			break;
         
         case evReDraw:
			draw_window();
			break;
      }
   }
}
void draw_window()
{
	proc_info Form;
	dword pos;
	//float zz=0.944,ret;
	DefineAndDrawWindow(215,100,250,200,0x34,0xFFFFFF,"Window header");
	//draw.gradient(pos,10,10,0x0,20,20,0x0);
	GetProcessInfo(#Form, SelfInfo);
	draw.circle(60,60,5);

	WriteText(10,110,0x80,0,#param);
}

stop:
