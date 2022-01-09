//Generated with Lev's C-- pack for HiAsm, www.hiasm.com

#define MEMSIZE 4096*400

#include "../lib/kolibri.h"
#include "../lib/gui.h"

/*========================================================
=                                                        =
=                          DATA                          =
=                                                        =
========================================================*/

_rgb image[256*256];

#define fw 256+167
#define fh 290

enum { 
	DRAW1_BTN=10, 
	DRAW2_BTN, 
	SETBG_BTN 
};

/*========================================================
=                                                        =
=                       MAIN CYCLE                       =
=                                                        =
========================================================*/

void main()
int id;
{
  loop() switch(WaitEvent())
  {
	case evButton:
		id=GetButtonID();
		if (id==1) ExitProcess();
		if (id==DRAW1_BTN) EventDraw1();
		if (id==DRAW2_BTN) EventDraw2();
		if (id==SETBG_BTN) EventSetBackground(#image,256,256);
		break;
	case evKey:
		if (GetKeys()==27) ExitProcess();
		break;
	case evReDraw:
		sc.get();
		DefineAndDrawWindow(screen.w-fw/2,screen.h-fh/2,fw,fh+skin_h,0x33,0xE0DFE3,"Rainbow (rgb test)",0);
		PutImage(0,0,256,256,#image);
		DrawCaptButton(280,20, 110,30,DRAW1_BTN,sc.button,sc.button_text,"Draw 1");
		DrawCaptButton(280,60, 110,30,DRAW2_BTN,sc.button,sc.button_text,"Draw 2");
		DrawCaptButton(280,100,110,30,SETBG_BTN,sc.button,sc.button_text,"Background");
  }
}

/*========================================================
=                                                        =
=              Background system functions               =
=                                                        =
========================================================*/

inline fastcall void BG_PaintMode( ECX)
{
	$mov eax, 15
	$mov ebx, 4
	$int  0x40
}

inline fastcall void BG_SetImageSize( ECX, EDX)
{
	$mov eax, 15
	$mov ebx, 1
	$int  0x40
}

inline fastcall void BG_PutPixels( ECX, EDX, ESI)
{
	$mov eax, 15
	$mov ebx, 5
	$int  0x40
}

inline fastcall void BG_Redraw()
{
	$mov eax, 15
	$mov ebx, 3
	$int  0x40
}

/*========================================================
=                                                        =
=                       EVENTS                           =
=                                                        =
========================================================*/

void EventDraw1()
{
int i1;
int i4;
int pos;
  for(i4 = 0; i4 < 256; i4++) {
	for(i1 = 0; i1 < 256; i1++) {
	  pos = i1 * 256 + i4;
	  image[pos].b = i1;
	  image[pos].r = 0;
	  image[pos].g = 0;
	}
  }
  PutImage(0,0,256,256,#image);
}

void EventDraw2()
{
int i1;
int i4;
int t2;
  for(i4 = 0; i4 < 256; i4++) {
	for(i1 = 0; i1 < 256; i1++) {
	  t2 = i1 * 256 + i4;
	  image[t2].r = i4;
	  image[t2].g = i1;
	  image[t2].b = 0;
	}
  }
  PutImage(0,0,256,256,#image);
}

void EventSetBackground(dword img_pointer, w,h)
{
	BG_PaintMode(2); //1-tile,2-stratch
	BG_SetImageSize(w,h);
	BG_PutPixels(img_pointer, 0, 3*w*h);
	BG_Redraw();	
}

stop:
