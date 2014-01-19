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
char mainWndTitle[] = "PHARAON's CRYPT\0";
char mainWndCopyright[] = "(C) MMVI by Rabid Rabbit";
char mainWndMenuStart[] = "Enter -Н†з†вм ®£аг";
char mainWndMenuLevel[] = "Н†з†Ђм≠л© гаЃҐ•≠м - %U";
#else
char mainWndTitle[] = "PHARAOH's CRYPT\0";
char mainWndCopyright[] = "(C) MMVI by Rabid Rabbit";
char mainWndMenuStart[] = "Enter - Start game";
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
				case 13:					// enter
					result = MW_START_GAME;
					break;

				case 27:					// escape
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
				case 1:
					result = MW_EXIT_APP;
					break;
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
		0x54, 0x0,				// Skinned fixed size window, dont fill working area, window has caption
		0, WNDHEADCOLOUR,
		mainWndTitle
		);
	// заголовок окна
	  kos_ChangeWindowCaption(mainWndTitle);
	mainWndFace.Draw( 1, 21 );
	// перва€ строка
	kos_WriteTextToWindow(
		8, 32,
		0, 0x0,
		mainWndMenuStart, sizeof(mainWndMenuStart)-1
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
