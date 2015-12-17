#define MEMSIZE 0x2EE80

#include "../lib/font.h"
#include "../lib/gui.h"

#define PANELH 30

void main()
{   
	proc_info Form;
	word i, y, btn;
	char line[256], title[4196];
	font.no_bg_copy = true;
	font.color = 0;
	font.bg_color = 0xFFFFFF;
	if (!param) strcpy(#param, DEFAULT_FONT);
	font.load(#param);
	strcpy(#title, "Font preview: ");
	strcat(#title, #param);
	loop()
	{
	  switch(WaitEvent())
      {
		case evButton:
			btn = GetButtonID();
			if (btn==1) ExitProcess();
			if (btn==2) font.bold ^=1;
			if (btn==3) font.italic ^=1;
			if (btn==4) font.smooth ^=1;
			goto _DRAW_WINDOW_CONTENT;
		case evReDraw:
			DefineAndDrawWindow(215,100,500,320,0x74,0xFFFFFF,#title);
			GetProcessInfo(#Form, SelfInfo);
			_DRAW_WINDOW_CONTENT:
			DrawBar(0, 0, Form.cwidth, PANELH, 0xCCCccc);
			CheckBox2(10, 8, 2, "Bold",  font.bold);
			CheckBox2(70, 8, 3, "Italic",  font.italic);
			CheckBox2(140, 8, 4, "Smooth",  font.smooth);
			font.buffer_size = free(font.buffer);
			if (!font.data)
			{
				DrawBar(0, PANELH, Form.cwidth, Form.cheight - PANELH, 0xFFFfff);
				WriteText(10, 50, 0x82, 0xFF00FF, "Font is not loaded.");
			}
			else for (i=10, y=5; i<22; i++, y+=font.height;) //not flexible, need to calculate font count and max line length
			{
				font.size.text = i;
				sprintf(#line,"Размер шрифта/size font %d пикселей.",i);
				font.prepare_buf(10,y,Form.cwidth,Form.cheight-PANELH, #line);
			}
			if (font.smooth) SmoothFont(font.buffer, font.size.width, font.size.height);
			font.left = 0;
			font.top = PANELH;
			font.show();
	  }
	}
}

void CheckBox2(dword x, y, id, text, byte value) {
	CheckBox(x, y, 14, 14, id, text, system.color.work_graph, system.color.work_text, value);
}