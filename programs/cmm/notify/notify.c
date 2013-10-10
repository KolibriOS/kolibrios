//notify 0.7
//SoUrcerer 2010, Leency 2012-2013, GNU GPLv2

#define MEMSIZE 0x2F00
#include "..\lib\kolibri.h" 
#include "..\lib\strings.h" 
#include "..\lib\mem.h" 
#include "..\lib\figures.h" 

?define PADDING 12;

int SCREEN_SIZE_X,
    SCREEN_SIZE_Y;

int WIN_X,
	WIN_Y,
    WIN_SIZE_X=256,
    WIN_SIZE_Y=28;

int delay = 400;

#include "lang.h--"
#ifdef LANG_RUS
	?define DEFAULT_TEXT "Эта программа показывает уведомления.";
#else
	?define DEFAULT_TEXT "This program shows notices. Open it with params.";
#endif

void PutText(dword x, y) {
	WriteText(x,y, 0x80, 0, #param);
}

inline fastcall int GetClientTop()
{
	$mov eax, 48
	$mov ebx, 5
	$int 0x40
    $mov eax, ebx
    $shr eax, 16
}

dword shadow_buf, lighter_pixel1, lighter_pixel2;
void GetBackground()
{
	CopyScreen(shadow_buf, WIN_X, WIN_Y, WIN_SIZE_X, WIN_SIZE_Y);
	CopyScreen(lighter_pixel1, WIN_X, WIN_Y, 1, 1);
	CopyScreen(lighter_pixel2, WIN_X, WIN_Y+WIN_SIZE_Y, 1, 1);
	ShadowImage(shadow_buf, WIN_SIZE_X, WIN_SIZE_Y, 6);
	ShadowImage(lighter_pixel1, 1, 1, 2);
	ShadowImage(lighter_pixel2, 1, 1, 2);
}


void main()
{   
	int TEXT_X=4,
	    TEXT_Y=12;
	char drawn;
	
	if (!param)	strcpy(#param, DEFAULT_TEXT);
	if (strlen(#param)*6>WIN_SIZE_X)
	{
		WIN_SIZE_X=strlen(#param)*6+PADDING;
		delay = strlen(#param)*10;
	}

	SCREEN_SIZE_X=GetScreenWidth()+1;	
	WIN_X = SCREEN_SIZE_X-WIN_SIZE_X-1;
	WIN_Y = GetClientTop();
	TEXT_X = -6*strlen(#param)+WIN_SIZE_X/2;
	TEXT_Y = WIN_SIZE_Y/2-3;
	
	//emulate multithread :)
	while (GetPixelColor(SCREEN_SIZE_X-1, SCREEN_SIZE_X, WIN_Y)==0x333333) WIN_Y+=WIN_SIZE_Y+17;

	mem_Init();
	shadow_buf = malloc(WIN_SIZE_X*WIN_SIZE_Y*3);
	lighter_pixel1 = malloc(3);
	lighter_pixel2 = malloc(3);
	GetBackground();

	loop()
	{
		WaitEventTimeout(delay);
		switch(EAX & 0xFF)
		{
		case evButton:
			ExitProcess();
			break;
         
		case evReDraw:
			DefineAndDrawWindow(WIN_X+1,WIN_Y,WIN_SIZE_X, WIN_SIZE_Y-1, 0x01, 0, 0, 0x01fffFFF);
			if (drawn==1)
			{
				drawn=2;
				MoveSize(0,0,-1,-1);
				pause(3);
				GetBackground();
				MoveSize(WIN_X,WIN_Y,-1,-1);
				drawn=1;
			} 
			DefineButton(0,0, WIN_SIZE_X, WIN_SIZE_Y, 1+BT_HIDE+BT_NOFRAME, 0);

			_PutImage(0,0,WIN_SIZE_X,WIN_SIZE_Y,shadow_buf);
			PutPixel(0,0,ESDWORD[lighter_pixel1]);
			PutPixel(0,WIN_SIZE_Y-1,ESDWORD[lighter_pixel2]);
			DrawBar(WIN_SIZE_X,0, 1, WIN_SIZE_Y, 0x333333);

			PutText(TEXT_X-1,TEXT_Y+1);
			PutText(TEXT_X-1,TEXT_Y-1);
			PutText(TEXT_X-1,TEXT_Y  );
			PutText(TEXT_X+1,TEXT_Y-1);
			PutText(TEXT_X+1,TEXT_Y  );
			PutText(TEXT_X+1,TEXT_Y+1);
			PutText(TEXT_X,  TEXT_Y+1);
			PutText(TEXT_X,  TEXT_Y-1);
			WriteText(TEXT_X,  TEXT_Y,   0x80, 0xFFFfff, #param);
			if (drawn==0) drawn=1;
			break;
		default:
			ExitProcess();
      }
   }
}


stop:
