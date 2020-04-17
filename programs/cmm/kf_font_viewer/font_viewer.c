#define MEMSIZE 0x2EE80

#include "../lib/kfont.h"
#include "../lib/gui.h"

#define PANELH 28
#define WIN_W 490
#define WIN_H 315
proc_info Form;

enum { 
	PHRASE_TAB=20, CHARS_TAB
};

_tabs tabs = { PHRASE_TAB };

block preview = { 0, PANELH, WIN_W, WIN_H - PANELH };
checkbox bold = { "Bold", false };
checkbox smooth = { "Smooth", true };
checkbox colored = { "Colored", true };

void main()
{   
	int btn;
	char title[4196];
	if (!param) strcpy(#param, DEFAULT_FONT);
	kfont.init(#param);
	strcpy(#title, "Font preview: ");
	strcat(#title, #param);
	loop() switch(WaitEvent())
	{
		case evButton:
			btn = GetButtonID();
			if (btn==1) ExitProcess();
			bold.click(btn); 
			smooth.click(btn);
			colored.click(btn);
			if (btn==PHRASE_TAB) || (btn==CHARS_TAB) tabs.click(btn);
			goto _DRAW_WINDOW_CONTENT;
		case evReDraw:
			sc.get();
			DefineAndDrawWindow(215,100,WIN_W+9,WIN_H+skin_height+5,0x74,0xFFFFFF,#title,0);
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window>2) break;
			_DRAW_WINDOW_CONTENT:

			kfont.bold = bold.checked;
			kfont.smooth = smooth.checked;

			DrawBar(0, 0, Form.cwidth, PANELH-1, sc.work);
			DrawBar(0, PANELH-1,Form.cwidth,1,sc.work_graph);
			bold.draw(10, 8);
			smooth.draw(83,8);
			colored.draw(170,8);

			tabs.draw_button(Form.cwidth-130, PHRASE_TAB, "Phrase");
			tabs.draw_button(Form.cwidth-60, CHARS_TAB, "Chars");

			if (!kfont.font)
			{
				DrawBar(preview.x, preview.y, preview.w, preview.h, 0xFFFfff);
				WriteText(10, 50, 0x82, 0xFF00FF, "Font is not loaded.");
				break;
			}
			if (tabs.active_tab==PHRASE_TAB) DrawPreviewPhrase();
			if (tabs.active_tab==CHARS_TAB) DrawPreviewChars();
	}
}

dword pal[] = { 0x4E4153, 0x57417C, 0x89633B, 0x819156, 0x00CCCC, 0x2AD266, 
	0xE000CC, 0x0498F9, 0xC3A9F5, 0xFFC200, 0xFF5836, 0xA086BA, 
	0,0,0,0,0 };

void DrawPreviewPhrase()
{
	dword i, y;
	dword c;
	char line[256];
	kfont.raw_size = free(kfont.raw);
	for (i=10, y=12; i<22; i++, y+=kfont.height+3;) //not flexible, need to calculate font count and max line length
	{
		if (colored.checked) c = pal[i-10]; else c=0;
		strcpy(#line, "Размер шрифта/forn size is ");
		strcat(#line, itoa(i));
		strcat(#line, " пикселей/px.");
		kfont.WriteIntoBuffer(14,y,Form.cwidth,Form.cheight-PANELH, 0xFFFFFF, c, i, #line);
	}
	if (kfont.smooth) kfont.ApplySmooth();
	kfont.ShowBuffer(preview.x, preview.y);
}

void DrawPreviewChars()
{
	dword i, x=20, y=0;
	char line[2];
	line[1]=NULL;
	kfont.raw_size = free(kfont.raw);
	for (i=0; i<255; i++) //not flexible, need to calculate font count and max line length
	{
		line[0]=i;
		kfont.WriteIntoBuffer(x,y,Form.cwidth,Form.cheight-PANELH, 0xFFFFFF, 0, 16, #line);
		x+= kfont.height+2;
		if (x>=preview.w-30) { 
			x=20;
			y+=kfont.height+2;
		}
	}
	if (kfont.smooth) kfont.ApplySmooth();
	kfont.ShowBuffer(preview.x, preview.y);
}
