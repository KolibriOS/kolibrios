#define MEMSIZE 0x2EE80

#include "../lib/font.h"
#include "../lib/gui.h"

#define PANELH 30

void main()
{   
	proc_info Form;
	FONT font_title = 0;
	FONT font_option = 0;
	word i, y, btn;
	char line[256], title[4196];
	font.no_bg_copy = true;
	font.color = 0;
	font.bg_color = 0xFFFFFF;
	font.left = 5;
	font.top = SKIN.height+PANELH;
	if (!param) strcpy(#param, "/sys/fonts/Tahoma.kf");
	font.load(#param);
	
	font_title.no_bg_copy = true;
	font_title.load("/sys/fonts/Tahoma.kf");
	font_title.size.text = 12;
	font_title.color = 0x444444;
	font_title.weight = 1;
	font_title.use_smooth = 1;
	font_title.bg_color = 0xE1E1E1;
	
	font_option.no_bg_copy = true;
	font_option.load("/sys/fonts/Tahoma.kf");
	font_option.size.text = 13;
	font_option.color = 0x222222;
	font_option.use_smooth = 1;
	font_option.bg_color = 0xDADADA;
	
	strcpy(#title, "Font preview: ");
	strcat(#title, #param);
	font_title.prepare(5, 4, #title);
	loop()
	{
	  switch(WaitEvent())
      {
		case evButton:
			btn = GetButtonID();
			if (btn==1) ExitProcess();
			if (btn==2) font.weight ^=1;
			if (btn==3) font.italic ^=1;
			if (btn==4) font.smooth ^=1;
			goto _DRAW_WINDOW_CONTENT;
		case evReDraw:
			DefineAndDrawWindow(215,100,500,320,0x04,0xFFFFFF,"");
			font_title.show();
			GetProcessInfo(#Form, SelfInfo);
			_DRAW_WINDOW_CONTENT:
			DrawBar(5, SKIN.height, Form.cwidth, PANELH, 0xDADADA);
			
			font_option.italic = font_option.smooth = 0;
			font_option.weight = 1;
			font_option.prepare(30, SKIN.height+7, "Bold");
			CheckBox2(10, SKIN.height+8, 2, "",  font.weight);
			font_option.show();
			
			font_option.weight = font_option.smooth = 0;
			font_option.italic = 1;
			font_option.prepare(90, SKIN.height+7, "Italic");
			CheckBox2(70, SKIN.height+8, 3, "",  font.italic);
			font_option.show();
			
			font_option.weight = font_option.italic = 0;
			font_option.smooth = 1;
			font_option.prepare(160, SKIN.height+7, "Smooth");
			CheckBox2(140, SKIN.height+8, 4, "Smooth",  font.smooth);
			font_option.show();
			
			IF(font.buffer)font.buffer_size = 0;
			
			if (!font.data)
			{
				DrawBar(5, SKIN.height+PANELH, Form.cwidth, Form.cheight - PANELH, 0xFFFfff);
				WriteText(15, 50, 0x82, 0xFF00FF, "Font is not loaded.");
			}
			else for (i=10, y=5; i<22; i++, y+=font.height;) //not flexible, need to calculate font count and max line length
			{
				font.size.text = i;
				sprintf(#line,"Размер шрифта/size font %d пикселей.",i);
				font.prepare_buf(15,SKIN.height+y,Form.cwidth,Form.cheight-PANELH, #line);
			}
			if (font.smooth) SmoothFont(font.buffer, font.size.width, font.size.height);
			font.show();
	  }
	}
}

void CheckBox2(dword x, y, id, text, byte value) {
	CheckBox(x, y, 14, 14, id, text, system.color.work_graph, system.color.work_text, value);
}