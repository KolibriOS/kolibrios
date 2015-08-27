#define MEMSIZE 0xEE80

#include "../lib/font.h"
#include "../lib/gui.h"

void main()
{   
	word i, y, btn;
	char line[256], title[4196];
	font.no_bg_copy = true;
	font.color = 0;
	font.bg_color = 0xFFFFFF;
	if (!param) strcpy(#param, "/sys/fonts/Tahoma.kf");
	font.load(#param);
	strcpy(#title, "Kolibri font preview: ");
	strcat(#title, #param);
	loop()
	{
	  switch(WaitEvent())
      {
		case evButton:
			btn = GetButtonID();
			if (btn==1) ExitProcess();
			if (btn==2) font.weight ^=1;
			if (btn==3) font.italic ^=1;
			if (btn==4) font.no_bg_copy ^=1;
		case evReDraw:
			DefineAndDrawWindow(215,100,500,320,0x33,0xFFFFFF,#title);
			DrawBar(0, 0, 500-9, 30, 0xCCCccc);
			CheckBox2(10, 8, 2, "Bold",  font.weight);
			CheckBox2(70, 8, 3, "Italic",  font.italic);
			CheckBox2(140, 8, 4, "Smooth",  font.no_bg_copy);
			if (!font.data)
			{
				WriteText(10, 50, 0x82, 0xFF00FF, "Font is not loaded.");
			} 
			else for (i=10, y=40; i<22; i++, y+=font.height;)
			{
				font.size.text = i;
				sprintf(#line,"Размер шрифта/size font %d пикселей.",i);
				font.prepare(10,y,#line);
				font.show(10, y);
			}
	  }
	}
}

void CheckBox2(dword x, y, id, text, byte value) {
	CheckBox(x, y, 14, 14, id, text, system.color.work_graph, system.color.work_text, value);
}