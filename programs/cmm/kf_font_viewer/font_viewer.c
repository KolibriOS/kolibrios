#define MEMSIZE 0x2EE80

#include "../lib/font.h"
#include "../lib/gui.h"

#define PANELH 30
proc_info Form;

enum { 
	STRONG_BTN=10, ITALIC_BTN, SMOOTH_BTN, 
	PHRASE_TAB=20, CHARS_TAB
};


void main()
{   
	int btn;
	char title[4196];
	if (!param) strcpy(#param, DEFAULT_FONT);
	label.init(#param);
	tabs.active_tab=PHRASE_TAB;
	strcpy(#title, "Font preview: ");
	strcat(#title, #param);
	loop() switch(WaitEvent())
	{
		case evButton:
			btn = GetButtonID();
			if (btn==1) ExitProcess();
			if (btn==STRONG_BTN) label.bold ^=1;
			if (btn==ITALIC_BTN) label.italic ^=1;
			if (btn==SMOOTH_BTN) label.smooth ^=1;
			if (btn==PHRASE_TAB) || (btn==CHARS_TAB) tabs.click(btn);
			goto _DRAW_WINDOW_CONTENT;
		case evReDraw:
			system.color.get();
			DefineAndDrawWindow(215,100,500,320+skin_height,0x74,0xFFFFFF,#title);
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window>2) break;
			_DRAW_WINDOW_CONTENT:
			DrawBar(0, 0, Form.cwidth, PANELH-1, system.color.work);
			CheckBox(10, 8, STRONG_BTN, "Bold",  label.bold);
			CheckBox(83, 8, ITALIC_BTN, "Italic",  label.italic);
			CheckBox(170,8, SMOOTH_BTN, "Smooth",  label.smooth);
			tabs.draw(Form.cwidth-150, PANELH, PHRASE_TAB, "Phrase");
			tabs.draw(Form.cwidth-70, PANELH, CHARS_TAB, "Chars");
			DrawBar(0, PANELH-1,Form.cwidth,1,system.color.work_graph);
			if (!label.font)
			{
				DrawBar(0, PANELH, Form.cwidth, Form.cheight - PANELH, 0xFFFfff);
				WriteText(10, 50, 0x82, 0xFF00FF, "Font is not loaded.");
				break;
			}
			if (tabs.active_tab==PHRASE_TAB) DrawPreviewPhrase();
			if (tabs.active_tab==CHARS_TAB) DrawPreviewChars();
	}
}

void DrawPreviewPhrase()
{
	dword i, y;
	char line[256];
	label.raw_size = free(label.raw);
	for (i=10, y=5; i<22; i++, y+=label.height;) //not flexible, need to calculate font count and max line length
	{
		sprintf(#line,"Размер шрифта/size font %d пикселей.",i);
		label.write_buf(10,y,Form.cwidth,Form.cheight-PANELH, 0xFFFFFF, 0, i, #line);
	}
	if (label.smooth) label.apply_smooth();
	label.show_buf(0, PANELH);
}

void DrawPreviewChars()
{
	dword i, x=20, y=0;
	char line[2];
	line[1]=NULL;
	label.raw_size = free(label.raw);
	for (i=0; i<255; i++) //not flexible, need to calculate font count and max line length
	{
		line[0]=i;
		label.write_buf(x,y,Form.cwidth,Form.cheight-PANELH, 0xFFFFFF, 0, 16, #line);
		x+= label.height+2;
		if (x>=Form.cwidth-30) { 
			x=20;
			y+=label.height+2;
		}
	}
	if (label.smooth) label.apply_smooth();
	label.show_buf(0, PANELH);
}
