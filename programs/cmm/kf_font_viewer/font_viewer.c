#define MEMSIZE 1024*30

#include "../lib/kfont.h"
#include "../lib/gui.h"

#define PANELH 28
#define WIN_W 520
#define WIN_H 315
#define BASE_TAB_BUTTON_ID 97

_tabs tabs = { WIN_W-130, 0, NULL, BASE_TAB_BUTTON_ID };

block preview = { 0, PANELH, WIN_W, WIN_H - PANELH };
checkbox bold = { "Bold", false };
checkbox smooth = { "Smooth", true };
checkbox colored = { "Colored", true };

void main()
{   
	proc_info Form;
	char title[1024];
	int btn;

	if (!param) strcpy(#param, DEFAULT_FONT);
	kfont.init(#param);
	strcpy(#title, "Font preview: ");
	strcat(#title, #param);

	tabs.add("Phrase", #DrawPreviewPhrase);
	tabs.add("Chars", #DrawPreviewChars);

	loop() switch(@WaitEvent())
	{
		case evButton:
			btn = @GetButtonID();
			if (btn==1) @ExitProcess();
			bold.click(btn); 
			smooth.click(btn);
			colored.click(btn);
			tabs.click(btn);
			GOTO _DRAW_WINDOW_CONTENT;

		case evReDraw:
			sc.get();
			DefineAndDrawWindow(215,100,WIN_W+9,WIN_H+skin_height+5,0x74,0xFFFFFF,#title,0);
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window&ROLLED_UP) break;
			_DRAW_WINDOW_CONTENT:

			kfont.bold = bold.checked;
			kfont.smooth = smooth.checked;

			DrawBar(0, 0, WIN_W, PANELH-1, sc.work);
			DrawBar(0, PANELH-1,WIN_W,1,sc.work_graph);

			if (!kfont.font) {
				DrawBar(preview.x, preview.y, preview.w, preview.h, 0xFFFfff);
				WriteText(10, 50, 0x82, 0xFF00FF, "Font is not loaded.");
			} else {
				bold.draw(10, 8);
				smooth.draw(83,8);
				colored.draw(170,8);
				tabs.draw();
				tabs.draw_active_tab();
			}
	}
}

dword pal[] = { 0x4E4153, 0x57417C, 0x89633B, 0x819156, 0x00CCCC, 0x2AD266, 
	0xE000CC, 0x0498F9, 0xC3A9F5, 0xFFC200, 0xFF5836, 0xA086BA, 0 };

void DrawPreviewPhrase()
{
	dword i, y;
	dword c;
	char line[256];
	kfont.raw_size = free(kfont.raw);
	for (i=10, y=12; i<22; i++, y+=kfont.height+3;) //not flexible, need to calculate font count and max line length
	{
		if (colored.checked) c = pal[i-10]; else c=0;
		strcpy(#line, "Размер шрифта/font size is ");
		strcat(#line, itoa(i));
		strcat(#line, " пикселей/px.");
		kfont.WriteIntoBuffer(14,y,WIN_W,WIN_H-PANELH, 0xFFFFFF, c, i, #line);
	}
	if (kfont.smooth) kfont.ApplySmooth();
	kfont.ShowBuffer(preview.x, preview.y);
}

void DrawPreviewChars()
{
	dword i, x=20, y=0;
	char line[2]=0;
	kfont.raw_size = free(kfont.raw);
	for (i=0; i<255; i++) //not flexible, need to calculate font count and max line length
	{
		line[0]=i;
		kfont.WriteIntoBuffer(x,y,WIN_W,WIN_H-PANELH, 0xFFFFFF, 0, 16, #line);
		x+= kfont.height+2;
		if (x>=preview.w-30) { 
			x=20;
			y+=kfont.height+2;
		}
	}
	if (kfont.smooth) kfont.ApplySmooth();
	kfont.ShowBuffer(preview.x, preview.y);
}
