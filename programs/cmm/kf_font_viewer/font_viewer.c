#define MEMSIZE 0x2EE80

#include "../lib/font.h"
#include "../lib/gui.h"

#define PANELH 30

void main()
{   
	proc_info Form;
	int i, y, btn;
	char line[256], title[4196];
	if (!param) strcpy(#param, DEFAULT_FONT);
	label.init(#param);
	strcpy(#title, "Font preview: ");
	strcat(#title, #param);
	loop()
	{
	  switch(WaitEvent())
      {
		case evButton:
			btn = GetButtonID();
			if (btn==1) ExitProcess();
			if (btn==2) label.bold ^=1;
			if (btn==3) label.italic ^=1;
			if (btn==4) label.smooth ^=1;
			goto _DRAW_WINDOW_CONTENT;
		case evReDraw:
			system.color.get();
			DefineAndDrawWindow(215,100,500,320,0x74,0xFFFFFF,#title);
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window>2) break;
			_DRAW_WINDOW_CONTENT:
			DrawBar(0, 0, Form.cwidth, PANELH, system.color.work);
			CheckBox(10, 8, 2, "Bold",  label.bold);
			CheckBox(83, 8, 3, "Italic",  label.italic);
			CheckBox(170, 8, 4, "Smooth",  label.smooth);
			label.raw_size = free(label.raw);
			if (!label.font)
			{
				DrawBar(0, PANELH, Form.cwidth, Form.cheight - PANELH, 0xFFFfff);
				WriteText(10, 50, 0x82, 0xFF00FF, "Font is not loaded.");
			}
			else for (i=10, y=5; i<22; i++, y+=label.height;) //not flexible, need to calculate font count and max line length
			{
				sprintf(#line,"Размер шрифта/size font %d пикселей.",i);
				label.write_buf(10,y,Form.cwidth,Form.cheight-PANELH, 0xFFFFFF, 0, i, #line);
			}
			if (label.smooth) label.apply_smooth();
			label.show_buf(0, PANELH);
	  }
	}
}
