#include "..\lib\kolibri.h" 
#include "..\lib\file_system.h"


void main()
{   
	int id, key;
	
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
			IF (key==013){ //Enter
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
	DefineAndDrawWindow(215,100,250,200,0x34,0xFFFFFF,"Window header");
	WriteText(50,80,0x80,0,"Press Enter");
	
}


stop:
