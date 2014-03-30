#define MEMSIZE 0x3E80
#include "..\lib\kolibri.h" 
#include "..\lib\strings.h" 
#include "..\lib\mem.h" 
#include "..\lib\file_system.h"

void str_replace(dword buf_in, what_replace, to_what_replace) {
	dword start_pos=0;
	dword buf_from;

	buf_from = malloc(strlen(buf_in));
	loop() {
		strcpy(buf_from, buf_in);
		start_pos = strstr(buf_from, what_replace);
		if (start_pos == 0) break;
		strlcpy(buf_in, buf_from, start_pos-buf_from);
		strcat(buf_in, to_what_replace);
		start_pos += strlen(what_replace);
		strcat(buf_in, start_pos);
	}
	free(buf_from);
}

void main()
{   
	int id, key;
	strcpy(#param, " <html>lorem</html>");
	str_replace(#param, "<", "&lt");
	str_replace(#param, ">", "&gt");
	
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
	proc_info Form;
	DefineAndDrawWindow(215,100,250,200,0x34,0xFFFFFF,"Window header");
	GetProcessInfo(#Form, SelfInfo);
	WriteText(50,80,0x80,0,"Press Enter");
	WriteText(10,110,0x80,0,#param);
}


stop:
