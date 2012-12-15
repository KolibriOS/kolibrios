//notify 0.5

#include "..\lib\kolibri.h" 
#include "..\lib\strings.h" 


int SCREEN_SIZE_X,
    SCREEN_SIZE_Y;

int WIN_Y=0,
    WIN_SIZE_X=300,
    WIN_SIZE_Y=28;

	
void main()
{   
	int TEXT_X=4,
	    TEXT_Y=12;
	
	if (!param)
	{
		if (GetSystemLanguage()==4)
			strcpy(#param, "Эв† ѓаЃ£а†ђђ† ѓЃ™†ІлҐ†•в гҐ•§ЃђЂ•≠®п.");
		else
			strcpy(#param, "This program shows notices. Open it with params.");
	}
	
	SCREEN_SIZE_X=GetScreenWidth()+1;
	if (strlen(#param)*6>WIN_SIZE_X) WIN_SIZE_X=strlen(#param)*6+8;
	
	TEXT_X=-6*strlen(#param)+WIN_SIZE_X/2;
	TEXT_Y=WIN_SIZE_Y/2-3;
	
	//из€€€щный костыль, реализующий многопоточность :)
	while (!GetPixelColor(SCREEN_SIZE_X-1, SCREEN_SIZE_X, WIN_Y)) WIN_Y+=45;
	

	loop()
   {
		WaitEventTimeout(500);
		switch(EAX & 0xFF)
		{
		case evButton:
			if (GetButtonID()==1) ExitProcess();
			break;
			
		case evKey:
			break;
         
		case evReDraw:
			DefineAndDrawWindow(SCREEN_SIZE_X-WIN_SIZE_X,WIN_Y,WIN_SIZE_X, WIN_SIZE_Y, 0x01, 0, 0, 0x01fffFFF);
			DefineButton(0,0, WIN_SIZE_X, WIN_SIZE_Y, 1+BT_HIDE+BT_NOFRAME, 0);
			draw_grid();
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



void draw_grid()
{
	int x, y; 
	
	for (y=0; y<=WIN_SIZE_Y; y++)
	{
		for (x=0; x<=WIN_SIZE_X; x++)	
		{
			if (! y&1) && (! x&1) PutPixel(x, y, 0);
			if (  y&1) && (  x&1) PutPixel(x, y, 0);
			//PutPixel(x, y, GetPixelColor(SCREEN_SIZE_X-WIN_SIZE_X+x, SCREEN_SIZE_X, y));
		}
	}
}





stop:
