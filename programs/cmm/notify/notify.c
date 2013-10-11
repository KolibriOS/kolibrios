//notify 0.8
//SoUrcerer 2010, Leency 2012-2013, GNU GPLv2

#define MEMSIZE 0x2F00
#include "..\lib\kolibri.h" 
#include "..\lib\strings.h" 
#include "..\lib\mem.h" 
#include "..\lib\figures.h" 

dword shadow_buf_24, shadow_buf_32, lighter_pixel1, lighter_pixel2;

?define PADDING 15;

int SCREEN_SIZE_X,
    SCREEN_SIZE_Y;

int WIN_X,
	WIN_Y,
    WIN_SIZE_X=256,
    WIN_SIZE_Y=28;

int TEXT_X=PADDING,
	TEXT_Y;

int delay = 400;

#ifndef AUTOBUILD
	#include "lang.h--"
#endif

#ifdef LANG_RUS
	?define DEFAULT_TEXT "Эта программа показывает уведомления.";
#else
	?define DEFAULT_TEXT "This program shows notices. Open it with params.";
#endif

void PutText(dword x, y) {
	WriteBufText(x,y, 0x88, 0, #param, shadow_buf_32);
}

void Text()
{
	PutText(TEXT_X-1,TEXT_Y+1);
	PutText(TEXT_X-1,TEXT_Y-1);
	PutText(TEXT_X-1,TEXT_Y  );
	PutText(TEXT_X+1,TEXT_Y-1);
	PutText(TEXT_X+1,TEXT_Y  );
	PutText(TEXT_X+1,TEXT_Y+1);
	PutText(TEXT_X,  TEXT_Y+1);
	PutText(TEXT_X,  TEXT_Y-1);
	WriteBufText(TEXT_X, TEXT_Y, 0x88, 0xFFFfff, #param, shadow_buf_32);
}

inline fastcall int GetClientTop()
{
	$mov eax, 48
	$mov ebx, 5
	$int 0x40
    $mov eax, ebx
    $shr eax, 16
}

void from24to32(dword src, dst, Width, Height)
{
conv24to32:
	$mov esi, src
	$mov edi, dst
		
	$mov eax, Width
	//$mul ESDWORD[Height]
	$mul Height
	$mov ecx, eax
	$xor al, al 
_next:	
	$movsw
	$movsb
	$stosb 	
	$loop _next
}


void GetBackground()
{
	int i;

	for (i=1; i<=6; i++)
	{
		ShadowImage(shadow_buf_24, WIN_SIZE_X, WIN_SIZE_Y, 1);
		if (i%2 == 0) ShadowImage(lighter_pixel1, 1, 1, 1);
		if (i%2 == 0) ShadowImage(lighter_pixel2, 1, 1, 1);
		from24to32(shadow_buf_24, shadow_buf_32+8, WIN_SIZE_X, WIN_SIZE_Y);
		Text();
		PutPaletteImage(shadow_buf_32+8,WIN_SIZE_X,WIN_SIZE_Y,0,0,32,0);
		PutPixel(0,0,ESDWORD[lighter_pixel1]);
		PutPixel(0,WIN_SIZE_Y-1,ESDWORD[lighter_pixel2]);
		pause(5);
	}
}

void Exit()
{
	ExitProcess();
}


void main()
{	
	if (!param)	strcpy(#param, DEFAULT_TEXT);
	if (strlen(#param)*6>WIN_SIZE_X)
	{
		WIN_SIZE_X=strlen(#param)*6+PADDING;
		delay = strlen(#param)*10;
	}

	SCREEN_SIZE_X=GetScreenWidth()+1;	
	WIN_X = SCREEN_SIZE_X-WIN_SIZE_X-1;
	WIN_Y = GetClientTop();
	TEXT_X = -6*strlen(#param)+WIN_SIZE_X/2+1;
	TEXT_Y = WIN_SIZE_Y/2-4;
	
	//emulate multithread :)
	while (GetPixelColor(SCREEN_SIZE_X-1, SCREEN_SIZE_X, WIN_Y)==0x333333) WIN_Y+=WIN_SIZE_Y+17;

	mem_Init();
	shadow_buf_24 = malloc(WIN_SIZE_X*WIN_SIZE_Y*3);
	shadow_buf_32 = malloc(WIN_SIZE_X*WIN_SIZE_Y*4+8);
	lighter_pixel1 = malloc(3);
	lighter_pixel2 = malloc(3);
	CopyScreen(shadow_buf_24, WIN_X, WIN_Y, WIN_SIZE_X, WIN_SIZE_Y);
	CopyScreen(lighter_pixel1, WIN_X, WIN_Y, 1, 1);
	CopyScreen(lighter_pixel2, WIN_X, WIN_Y+WIN_SIZE_Y, 1, 1);
	ESDWORD[shadow_buf_32] = WIN_SIZE_X;
	ESDWORD[shadow_buf_32+4] = WIN_SIZE_Y;

	loop()
	{
		WaitEventTimeout(delay);
		switch(EAX & 0xFF)
		{
		case evButton:
			Exit();
			break;
         
		case evReDraw:
			DefineAndDrawWindow(WIN_X+1,WIN_Y,WIN_SIZE_X, WIN_SIZE_Y-1, 0x01, 0, 0, 0x01fffFFF);
			DrawBar(WIN_SIZE_X,0, 1, WIN_SIZE_Y, 0x333333);
			DefineButton(0,0, WIN_SIZE_X, WIN_SIZE_Y, 1+BT_HIDE+BT_NOFRAME, 0);
			GetBackground();
			break;

		default:
			Exit();
      }
   }
}


stop:
