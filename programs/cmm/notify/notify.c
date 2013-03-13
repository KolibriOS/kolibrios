//notify 0.52
//SoUrcerer 2010, Leency 2012-2013, GNU GPLv2

#define MEMSIZE 0x3E80
#include "..\lib\kolibri.h" 
#include "..\lib\strings.h" 
#include "..\lib\mem.h" 
#include "..\lib\figures.h" 

int SCREEN_SIZE_X,
    SCREEN_SIZE_Y;

int WIN_Y=0,
    WIN_SIZE_X=300,
    WIN_SIZE_Y=28;

int delay = 500;
	
void main()
{   
	int TEXT_X=4,
	    TEXT_Y=12;
	dword shadow_buf, lighter_pixel1, lighter_pixel2;
	
	if (!param)
	{
		if (GetSystemLanguage()==4)
			strcpy(#param, "Эв† ѓаЃ£а†ђђ† ѓЃ™†ІлҐ†•в гҐ•§ЃђЂ•≠®п.");
		else
			strcpy(#param, "This program shows notices. Open it with params.");
	}
	
	SCREEN_SIZE_X=GetScreenWidth()+1;
	if (strlen(#param)*6>WIN_SIZE_X)
	{
		WIN_SIZE_X=strlen(#param)*6+8;
		delay = strlen(#param)*10;
	}
	
	TEXT_X=-6*strlen(#param)+WIN_SIZE_X/2;
	TEXT_Y=WIN_SIZE_Y/2-3;
	
	//из€€€щный костыль, реализующий многопоточность :)
	while (GetPixelColor(SCREEN_SIZE_X-1, SCREEN_SIZE_X, WIN_Y)==0x333333) WIN_Y+=WIN_SIZE_Y+17;

	#if !defined(AUTOBUILD)
	{
		mem_Init();
		shadow_buf = malloc(WIN_SIZE_X*WIN_SIZE_Y*3);
		lighter_pixel1 = malloc(3);
		lighter_pixel2 = malloc(3);
		CopyScreen(shadow_buf, SCREEN_SIZE_X-WIN_SIZE_X-1, WIN_Y, WIN_SIZE_X, WIN_SIZE_Y);
		CopyScreen(lighter_pixel1, SCREEN_SIZE_X-WIN_SIZE_X-1, WIN_Y, 1, 1);
		CopyScreen(lighter_pixel2, SCREEN_SIZE_X-WIN_SIZE_X-1, WIN_Y+WIN_SIZE_Y, 1, 1);
		ShadowImage(shadow_buf, WIN_SIZE_X, WIN_SIZE_Y, 4);
		ShadowImage(lighter_pixel1, 1, 1, 1);
		ShadowImage(lighter_pixel2, 1, 1, 1);
	}
	#endif

	loop()
	{
		WaitEventTimeout(delay);
		switch(EAX & 0xFF)
		{
		case evButton:
			if (GetButtonID()==1) ExitProcess();
			break;
			
		case evKey:
			break;
         
		case evReDraw:
			DefineAndDrawWindow(SCREEN_SIZE_X-WIN_SIZE_X,WIN_Y,WIN_SIZE_X, WIN_SIZE_Y-1, 0x01, 0, 0, 0x01fffFFF);
			DefineButton(0,0, WIN_SIZE_X, WIN_SIZE_Y, 1+BT_HIDE+BT_NOFRAME, 0);
			//draw_grid();
			//PutShadow(0,0,WIN_SIZE_X,WIN_SIZE_Y, 0, 4);
			#ifdef AUTOBUILD
				{ draw_grid(); }
			#else
			{
				_PutImage(0,0,WIN_SIZE_X,WIN_SIZE_Y,shadow_buf);
				PutPixel(0,0,ESDWORD[lighter_pixel1]);
				PutPixel(0,WIN_SIZE_Y-1,ESDWORD[lighter_pixel2]);
			}
			#endif
			DrawBar(WIN_SIZE_X,0, 1, WIN_SIZE_Y, 0x333333);
			WriteText(TEXT_X-1,TEXT_Y, 0x80, 0,#param); //тень
			WriteText(TEXT_X+1,TEXT_Y, 0x80, 0,#param);
			WriteText(TEXT_X,TEXT_Y-1, 0x80, 0,#param);
			WriteText(TEXT_X,TEXT_Y+1, 0x80, 0,#param);
			WriteText(TEXT_X-1,TEXT_Y-1, 0x80, 0,#param);
			WriteText(TEXT_X+1,TEXT_Y+1, 0x80, 0,#param);
			WriteText(TEXT_X-1,TEXT_Y+1, 0x80, 0,#param);
			WriteText(TEXT_X+1,TEXT_Y-1, 0x80, 0,#param);
			
			WriteText(TEXT_X,TEXT_Y, 0x80, 0xFFFfff,#param);
			break;
		default:
			ExitProcess();
      }
   }
}



:void draw_grid()
{
	int x, y; 
	
	for (y=0; y<=WIN_SIZE_Y; y++)
	{
		for (x=0; x<=WIN_SIZE_X; x++)	
		{
			if (! y&1) && (! x&1) PutPixel(x, y, 0);
			if (  y&1) && (  x&1) PutPixel(x, y, 0);
		}
	}
}


stop:
