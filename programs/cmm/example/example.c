#define MEMSIZE 0x3E80

#include "../lib/io.h"

void main()
{   
	int id, key, i;
	dword file;
	io.dir.load(0,DIR_ONLYREAL);
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
				draw_window();
				
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
	int i;
	i=0;
	DefineAndDrawWindow(215,100,250,200,0x34,0xFFFFFF,"Window header");
	GetProcessInfo(#Form, SelfInfo);
	while(i<io.dir.count)
	{
		WriteText(5,i*8+3,0x80,0xFF00FF,io.dir.position(i));
		i++;
	}
	WriteText(10,110,0x80,0,#param);
}