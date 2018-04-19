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

_tabs tabs = { 0,0, WIN_W, WIN_H, PHRASE_TAB};

block preview = { 0, PANELH, WIN_W, WIN_H - PANELH };
checkbox bold = { "Bold", false };
checkbox smooth = { "Smooth", true };

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
			if (bold.click(btn)) kfont.bold = bold.checked;
			if (smooth.click(btn)) kfont.smooth = smooth.checked;
			if (btn==PHRASE_TAB) || (btn==CHARS_TAB) tabs.click(btn);
			goto _DRAW_WINDOW_CONTENT;
		case evReDraw:
			system.color.get();
			DefineAndDrawWindow(215,100,WIN_W+9,WIN_H+skin_height+5,0x74,0xFFFFFF,#title,0);
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window>2) break;
			_DRAW_WINDOW_CONTENT:

			DrawBar(0, 0, Form.cwidth, PANELH-1, system.color.work);
			DrawBar(0, PANELH-1,Form.cwidth,1,system.color.work_graph);
			bold.draw(10, 8);
			smooth.draw(83,8);

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

void DrawPreviewPhrase()
{
	dword i, y;
	char line[256];
	kfont.raw_size = free(kfont.raw);
	for (i=10, y=5; i<22; i++, y+=kfont.height;) //not flexible, need to calculate font count and max line length
	{
		sprintf(#line,"Размер шрифта/size font %d пикселей.",i);
		kfont.WriteIntoBuffer(10,y,Form.cwidth,Form.cheight-PANELH, 0xFFFFFF, 0, i, #line);
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
