#define MEMSIZE 0x7E80

#include "../lib/font.h"

byte id,key;

void main()
{   

	SetEventMask(1100111b);
	
	font.load("font/Verdana.kf");

	loop()
   {
      switch(WaitEvent())
      {
		case evMouse:
			mouse.get();
		
			
		break;
         case evButton:
            id=GetButtonID();               
            if (id==1) ExitProcess();
			break;
      
        case evKey:
			key = GetKey();
			if (key==013){ //Enter
				draw_window();
				
			}
			break;
         
         case evReDraw:
			draw_window();

			break;
      }
   }
}
char buf[40];
void draw_window()
{
	proc_info Form;
	
	int i =8;
	int ii = 0;
	DefineAndDrawWindow(215,100,450,500,0x33,0xFFFFFF,"Window header");
	GetProcessInfo(#Form, SelfInfo);
	while(i<=45)
	{
		sprintf(#buf,"Размер шрифта/size font %d пикселей.",i);
		font.text(0,ii,#buf,0,i);
		ii+=font.height;
		i++;
	}
}