#define MEMSIZE 0xFFFFF
#include "../lib/kolibri.h" 
#include "../lib/strings.h" 
#include "../lib/mem.h" 
#include "../lib/gui.h" 

#ifndef AUTOBUILD
	#include "lang.h--"
#endif

/* === DATA === */

system_colors sc;
proc_info Form;


dword b_screen,
      s_screen;

int b_screen_width,
    b_screen_height,
    b_screen_length,
    s_screen_width,
    s_screen_height,
    s_screen_length;


/* === CODE === */


void main()
{	
	char id;
	mem_Init();
	b_screen_width  = GetScreenWidth()+1;
	b_screen_height = GetScreenHeight()+1;
	b_screen_length = b_screen_width*b_screen_height*3;
	s_screen_width  = b_screen_width / 2;
	s_screen_height = b_screen_height / 2;
	s_screen_length = b_screen_length / 2;	

	b_screen  = malloc(b_screen_length);
	s_screen = malloc(b_screen_length/2);

	loop()
	{
		switch(WaitEvent())
		{
		case evButton:
			id = GetButtonID();
			if (id==1) ExitProcess();
			if (id==10) TakeScreenshot();
			break;
         
		case evReDraw:
			sc.get();
			DefineAndDrawWindow(b_screen_width/4, b_screen_height/4, s_screen_width + 9, s_screen_height + GetSkinHeight() + 45,0x74, 0, "EasyShot v0.2",0);
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window>2) break;
			DrawBar(0, 0, Form.cwidth, 41, sc.work);
			DrawCaptButton(10, 10, 140, 20, 10, sc.work_button, sc.work_button_text, "Make screenshot");
			_PutImage(0, Form.cheight - s_screen_height,  s_screen_width, s_screen_height, s_screen);
			if (ESDWORD[s_screen]==0) 
				WriteTextB(Form.cwidth/2 - 60, Form.cheight/2+10, 0x90, 0xFFFfff, "There will be preview");
			else
			DrawCaptButton(160, 10, 80, 20, 11, sc.work_button, sc.work_button_text, "Save");
      }
   }
}

void TakeScreenshot() {
	MinimizeWindow();
	pause(20);
	CopyScreen(b_screen, 0, 0, b_screen_width, b_screen_height);
	ZoomImageTo50percent();
	ActivateWindow(GetProcessSlot(Form.ID));
	//_PutImage(0, Form.cheight - s_screen_height,  s_screen_width, s_screen_height, s_screen);
}

void ZoomImageTo50percent() {
	dword point_x,
	      line_h= b_screen_width * 3,
	      s_off = s_screen + 3,
	      b_off = b_screen + 6,
	      b_off_r,
	      b_off_g,
	      b_off_b,
	      rez_r, 
	      rez_g, 
	      rez_b;

	while( (s_off < s_screen + s_screen_length) && (b_off < b_screen + b_screen_length ) ) {
		
		if (b_off < b_screen + line_h) || (b_off > b_screen + b_screen_length - line_h)
		{
			ESBYTE[s_off]   = ESBYTE[b_off];
			ESBYTE[s_off+1] = ESBYTE[b_off+1];
			ESBYTE[s_off+2] = ESBYTE[b_off+2];
		}
		else
		{
			// line[x].R = (line[x+1].R + line[x].R + line[x-1].R + line1[x].R + line2[x].R) / 5;
			// line[x].G = (line[x+1].G + line[x].G + line[x-1].G + line1[x].G + line2[x].G) / 5;
			// line[x].B = (line[x+1].B + line[x].B + line[x-1].B + line1[x].B + line2[x].B) / 5
			b_off_r = b_off;
			b_off_g = b_off + 1;
			b_off_b = b_off + 2;
			rez_r = ESBYTE[b_off_r+3] + ESBYTE[b_off_r] + ESBYTE[b_off_r-3] + ESBYTE[b_off_r-line_h] + ESBYTE[b_off_r+line_h] / 5;
			rez_g = ESBYTE[b_off_g+3] + ESBYTE[b_off_g] + ESBYTE[b_off_g-3] + ESBYTE[b_off_g-line_h] + ESBYTE[b_off_g+line_h] / 5;
			rez_b = ESBYTE[b_off_b+3] + ESBYTE[b_off_b] + ESBYTE[b_off_b-3] + ESBYTE[b_off_b-line_h] + ESBYTE[b_off_b+line_h] / 5;
			ESBYTE[s_off] = rez_r;
			ESBYTE[s_off+1] = rez_g;
			ESBYTE[s_off+2] = rez_b;

		}
	
		s_off+=3;
		b_off+=6;

		point_x+=2;
		if (point_x >= b_screen_width) 
		{
			b_off += line_h;
			point_x = 0;
		}
	}
}


stop:
