#define MEMSIZE 1024*30
#define ENTRY_POINT #main

#include "../lib/kfont.h"
#include "../lib/gui.h"

#define BARH 28
#define WINW 528
#define WINH 315

char active_tab = 0;
char colored = true;
dword checkbox_flag;

void main()
{   
	proc_info Form;

	mem_init();
	checkbox_flag = memopen("CHECKBOX", NULL, SHM_READ);

	if (!param) strcpy(#param, DEFAULT_FONT);
	kfont.init(#param);
	strcpy(#title, "Font preview: ");
	strcat(#title, #param);

	loop() switch(@WaitEvent())
	{
		case evKey:
			@GetKey();
			EAX >>= 16;
			if (AL == SCAN_CODE_ESC) ExitProcess();
			if (AL == SCAN_CODE_TAB) {
				active_tab^=1;
				goto _DRAW_WINDOW_CONTENT;
			}
			break;

		case evButton:
			@GetButtonID();
			if (EAX==1) ExitProcess();
			if (EAX==2) kfont.bold^=1;
			if (EAX==3) kfont.smooth^=1;
			if (EAX==4) colored^=1;
			if (EAX==5) active_tab=0;
			if (EAX==6) active_tab=1;
			GOTO _DRAW_WINDOW_CONTENT;

		case evReDraw:
			sc.get();
			DefineAndDrawWindow(215,100,WINW+9,WINH+GetSkinHeight()+4,0x74,0xFFFFFF,#title,0);
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window&ROLLED_UP) break;
			_DRAW_WINDOW_CONTENT:

			DrawBar(0, 0, WINW, BARH-1, sc.work);
			DrawBar(0, BARH-1,WINW,1,sc.line);

			if (!kfont.font) {
				DrawBar(0, BARH, WINW, WINH - BARH, 0xFFFfff);
				WriteText(10, 50, 0x82, 0xFF00FF, "Font is not loaded.");
			} else {
				draw_checkbox(9, kfont.bold);
				draw_checkbox(81, kfont.smooth);
				draw_checkbox(169, colored);
				WriteText(30, 8, 0x90, sc.work_text, "Bold     Smooth     Colored            Phrase   Chars");
				UnsafeDefineButton(0, 3, 70, 24, 2+BT_HIDE+BT_NOFRAME, ESI);
				$inc edx //11
				$add ebx, 80 << 16
				$int 64
				$inc edx //12
				$add ebx, 88 << 16
				$int 64
				$inc edx //13
				$add ebx, 162 << 16
				$int 64
				$inc edx //14
				$add ebx, 70 << 16
				$int 64
				DrawBar(active_tab*70+335, BARH-4, 60, 4, 0xE44C9C);
				if (!active_tab) DrawPreviewPhrase(); else DrawPreviewChars();
			}
	}
}

dword pal[] = { 0x4E4153, 0x57417C, 0x89633B, 0x819156, 0x00CCCC, 0x2AD266, 
	0xE000CC, 0x0498F9, 0xC3A9F5, 0xFFC200, 0xFF5836, 0xA086BA, 0 };

void DrawPreviewPhrase()
{
	dword i, y=12;
	dword line[64];
	kfont.raw_size = free(kfont.raw);
	for (i=10; i<22; i++) //not flexible, need to calculate font count and max line length
	{
		strcpy(#line, "Размер шрифта/font size is ");
		itoa_(#line+27, i);
		strcat(#line, " пикселей/px.");
		kfont.WriteIntoBuffer(14,y,WINW,WINH-BARH, 0xFFFFFF, pal[i-10]*colored, i, #line);
		y+=kfont.height+3;
	}
	DrawBuf();
}

void DrawPreviewChars()
{
	dword i, x=20, y=0;
	char line[2];
	kfont.raw_size = free(kfont.raw);
	for (i=0; i<255; i++)
	{
		line[0]=i;
		kfont.WriteIntoBuffer(x,y,WINW,WINH-BARH, 0xFFFFFF, 0, 16, #line);
		x+= kfont.height+2;
		if (x>=WINW-30) { 
			x=20;
			y+=kfont.height+2;
		}
	}
	DrawBuf();
}

void DrawBuf()
{
	if (kfont.smooth) kfont.ApplySmooth();
	kfont.ShowBuffer(0, BARH);
}

void draw_checkbox(dword _x, _checked)
{
	#define SIZE 14
	#define CHBOXY 7
	DrawRectangle3D(_x, CHBOXY, SIZE, SIZE, sc.line, sc.line);
	if (_checked == false)
	{
		DrawRectangle3D(_x+1, CHBOXY+1, SIZE-2, SIZE-2, 0xDDDddd, 0xffffff);
		DrawBar(_x+2, CHBOXY+2, SIZE-3, SIZE-3, 0xffffff);
	} else {
		if (checkbox_flag) PutImage(_x+1, CHBOXY+1, 13, 13, checkbox_flag);
	}
	DrawRectangle3D(_x-1,CHBOXY-1,SIZE+2,SIZE+2,sc.dark,sc.light);
}

char title[PATHLEN];