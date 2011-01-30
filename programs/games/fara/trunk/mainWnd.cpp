#include "kosSyst.h"
#include "KosFile.h"
#include "gfxdef.h"
#include "mainWnd.h"
#include "lang.h"


//
RGB bmPMButton[] = {
	0xCCCCCC, 0xCCCCCC, 0x000000, 0x000000, 0xCCCCCC, 0xCCCCCC,
	0xCCCCCC, 0xCCCCCC, 0x000000, 0x000000, 0xCCCCCC, 0xCCCCCC,
	0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
	0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
	0xCCCCCC, 0xCCCCCC, 0x000000, 0x000000, 0xCCCCCC, 0xCCCCCC,
	0xCCCCCC, 0xCCCCCC, 0x000000, 0x000000, 0xCCCCCC, 0xCCCCCC,
};


//
#if LANG == RUS
char mainWndTitle[] = "PHARAON's CRYPT";
char mainWndCopyright[] = "(C) MMVI by Rabid Rabbit";
char mainWndMenuStart[] = "1. Н†з†вм ®£аг";
char mainWndMenuExit[] = "2. ВлеЃ§";
char mainWndMenuLevel[] = "Н†з†Ђм≠л© гаЃҐ•≠м - %U";
#else
char mainWndTitle[] = "PHARAON's CRYPT";
char mainWndCopyright[] = "(C) MMVI by Rabid Rabbit";
char mainWndMenuStart[] = "1. Start game";
char mainWndMenuExit[] = "2. Exit";
char mainWndMenuLevel[] = "Starting level - %U";
#endif
//
CKosBitmap mainWndFace;
//
Word mcx, mcy;

#define BT_SIZE_X_MINUS		4
#define BT_SIZE_X_PLUS		5

//
int MainWndLoop()
{
	Byte keyCode;
	Dword buttonID;
	int result;
	static bool firstTime = true;

	//
	startGameLevel = maxGameLevel;
	//
	if ( firstTime )
	{
		//
		mainWndFace.GetSize( mcx, mcy );
		//
		firstTime = false;
		//
		DrawMainWindow();
	}
	//
	kos_ChangeWindow( -1, -1, mcx + 1, mcy + 21 );
	//
	for ( result = MW_NONE; result == MW_NONE; )
	{
		switch( kos_WaitForEvent() )
		{
		case 1:
			DrawMainWindow();
			break;

		case 2:
			if ( kos_GetKey( keyCode ) )
			{
				//
				switch ( keyCode )
				{
				case '1':
					result = MW_START_GAME;
					break;

				case '2':
					result = MW_EXIT_APP;
					break;

				default:
					break;
				}
			}
			break;

		case 3:
			if ( kos_GetButtonID( buttonID ) )
			{
				//
				switch ( buttonID )
				{
				//
				case BT_SIZE_X_MINUS:
					if ( --startGameLevel < 1 )
						startGameLevel = 1;
					else
						DrawMainWindow();
					break;

				//
				case BT_SIZE_X_PLUS:
					if ( ++startGameLevel > maxGameLevel )
						startGameLevel = maxGameLevel;
					else
						DrawMainWindow();
					break;

				//
				default:
					break;
				}
			}

		default:
			break;
		}
	}
	// кнопки
	kos_DefineButton(
		0, 0,
		0, 0,
		BT_SIZE_X_MINUS + 0x80000000,
		0
		);
	//
	kos_DefineButton(
		0, 0,
		0, 0,
		BT_SIZE_X_PLUS + 0x80000000,
		0
		);
	//
	return result;
}


// полна€ отрисовка главного окна программы (1)
void DrawMainWindow()
{
	char line[64];

	//
	kos_WindowRedrawStatus( WRS_BEGIN );
	// окно
	kos_DefineAndDrawWindow(
		WNDLEFT, WNDTOP,
		mcx + 1, mcy + 21,
		0, 0x0,
		0, WNDHEADCOLOUR,
		WNDHEADCOLOUR
		);
	// заголовок окна
	kos_WriteTextToWindow(
		4, 7,
		0x10, WNDTITLECOLOUR,
		mainWndTitle, sizeof(mainWndTitle)-1
		);
	//
	mainWndFace.Draw( 1, 21 );
	// перва€ строка
	kos_WriteTextToWindow(
		8, 32,
		0, 0x0,
		mainWndMenuStart, sizeof(mainWndMenuStart)-1
		);
	// втора€ строка
	kos_WriteTextToWindow(
		8, 48,
		0, 0x0,
		mainWndMenuExit, sizeof(mainWndMenuExit)-1
		);
	// треть€ строка
	sprintf( line, mainWndMenuLevel, startGameLevel);
	kos_WriteTextToWindow(
		8, 64,
		0, 0x0,
		line, strlen( line )
		);
	// кнопки
	kos_DefineButton(
		mcx - 29, 64,
		12, 12,
		BT_SIZE_X_MINUS,
		0xCCCCCC
		);
	//
	kos_PutImage( bmPMButton + 12, 6, 2, mcx - 29 + 3, 69 );
	//
	kos_DefineButton(
		mcx - 16, 64,
		12, 12,
		BT_SIZE_X_PLUS,
		0xCCCCCC
		);
	//
	kos_PutImage( bmPMButton, 6, 6, mcx - 16 + 3, 67 );
	// копирайт
	kos_WriteTextToWindow(
		8, mcy - 16 + 21,
		0, 0x000066,
		mainWndCopyright, sizeof(mainWndCopyright)-1
		);
	//
	kos_WindowRedrawStatus( WRS_END );
}
