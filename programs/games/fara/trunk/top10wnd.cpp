// top10wnd.cpp

#include "kosSyst.h"
#include "KosFile.h"
#include "gfxdef.h"
#include "gameWnd.h"
#include "mcarray.h"
#include "top10wnd.h"
#include "lang.h"

//
char top10FilePath[MAX_PATH];

//
struct hiScoreHero
{
	char name[12];
	Dword score;
	//
	hiScoreHero()
	{
		//
		this->ClearName();
		this->score = 0;
	};
	//
	void ClearName()
	{
		memset( (Byte *)(this->name), '.', sizeof(this->name) );
	};
};

//
hiScoreHero heroTbl[TOP_TBL_SIZE];

//
struct hiScoreFile
{
	Byte block[512];
	kosFileInfo fi;
	//
	hiScoreFile()
	{
		int i;

		//
		this->fi.OffsetLow = this->fi.OffsetHigh = 0;
		this->fi.dataCount = 0;
		this->fi.bufferPtr = this->block;
		this->fi.rwMode = 0;
		memcpy( this->fi.fileURL, top10FilePath, strlen( top10FilePath ) + 1);
		//
		for ( i = 0; i < ( sizeof( this->block ) / sizeof( Dword ) ); i++ )
		{
			//
			((Dword *)(this->block))[i] = rtlRand();
		}
	};
	//
	virtual ~hiScoreFile()
	{}
	//
	bool LoadFromDisk()
	{
		bool result;
		int i;
		Dword j, k;
		Byte *bPtr;

		//
		this->fi.rwMode = FO_READ;
		this->fi.OffsetLow = this->fi.OffsetHigh = 0;
		this->fi.dataCount = 512;
		result = kos_FileSystemAccess( &(this->fi) ) == 0;
		//
		if ( result )
		{
			// декодируем
			rtlSrand( ((Dword *)(this->block))[(sizeof(this->block) / sizeof(Dword)) - 1] );
			//
			for ( i = 0; i < (sizeof( heroTbl ) * 5); i++ )
			{
				// не трогаем последний Dword
				j = rtlRand() % (sizeof(this->block) - 7);
				k = ( rtlRand() % 31 ) + 1;
				//
				bPtr = this->block + j;
				//
				__asm{
					mov edx, bPtr
					mov ecx, k
					mov eax, [edx]
					bswap eax
					ror eax, cl
					mov [edx], eax
				}
			}
			//
			rtlSrand( kos_GetSystemClock() );
		}
		//
		return result;
	};
	//
	bool SaveToDisk()
	{
		int i;
		Dword *rndList;
		Byte *bPtr;
		Dword k, keyLock;

		//
		rndList = new Dword[(sizeof( heroTbl ) * 5) * 2];
		//
		keyLock = rtlRand();
		//
		for ( i = 0; i < (sizeof( heroTbl ) * 5); i++ )
		{
			//
			rndList[i * 2] = rtlRand() % (sizeof(this->block) - 7);
			rndList[(i * 2) + 1] = ( rtlRand() % 31 ) + 1;
		}
		//
		for ( i = (sizeof( heroTbl ) * 5) - 1; i >= 0; i-- )
		{
			//
			bPtr = this->block + rndList[i * 2];
			k = rndList[(i * 2) + 1];
			//
			__asm{
				mov edx, bPtr
				mov ecx, k
				mov eax, [edx]
				rol eax, cl
				bswap eax
				mov [edx], eax
			}
		}
		//
		delete rndList;
		//
		((Dword *)(this->block))[(sizeof(this->block) / sizeof(Dword)) - 1] = keyLock;
		//
		this->fi.rwMode = FO_WRITE;
		this->fi.dataCount = 512;
		return kos_FileSystemAccess( &( this->fi) ) == 0;
	};
};


///
hiScoreFile *top10Heroes = NULL;

//
#if LANG == RUS
char Top10WndTitle[] = "Top 10";
char top10str1[] = "ENTER - Ё¬п Ok.";
char top10str2[] = "ESC - ўле®¤ ў ¬Ґ­о";
#else
char Top10WndTitle[] = "Top 10";
char top10str1[] = "Enter - name Ok.";
char top10str2[] = "Esc - leave to menu";
#endif
int enterName = -1;
int enterCharNdx = 0;


//
void ReleaseTop10()
{
	//
	if ( top10Heroes != NULL )
	{
		//
		memcpy( top10Heroes->block, heroTbl, sizeof(heroTbl) );
		//
		top10Heroes->SaveToDisk();
		//
		delete top10Heroes;
	}
}


//
void PrepareTop10()
{
	//
	top10Heroes = new hiScoreFile;
	//
	atexit( ReleaseTop10 );
	//
	if ( top10Heroes->LoadFromDisk() )
	{
		//
		memcpy( heroTbl, top10Heroes->block, sizeof(heroTbl) );
	}
}


//
void SetUpTop10()
{
	int i, j;
	Byte keyCode;

	//
	while ( kos_CheckForEvent() == 2 ) kos_GetKey( keyCode );
	//
	kos_SetKeyboardDataMode( KM_CHARS );
	//
	kos_ChangeWindow( -1, -1, TOP10_WND_SIZE_X, TOP10_WND_SIZE_Y );
	//
	for ( i = 0; i < TOP_TBL_SIZE; i++ )
	{
		//
		if ( heroTbl[i].score < playerScore )
		{
			//
			for ( j = TOP_TBL_SIZE - 1; j > i; j-- )
			{
				//
				heroTbl[j] = heroTbl[j-1];
			}
			//
			heroTbl[i].ClearName();
			heroTbl[i].score = playerScore;
			//
			enterName = i;
			enterCharNdx = 0;
			//
			break;
		}
	}
}


//
void DrawTop10Window()
{
	int i;

	//
	kos_DefineAndDrawWindow(
		100, 100,
		TOP10_WND_SIZE_X, TOP10_WND_SIZE_Y,
		0, 0,
		0, 0x2040A0,
		0x2040A0
		);
	//
	kos_WriteTextToWindow(
		4, 4,
		0x0, 0x42D2E2,
		Top10WndTitle,
		sizeof( Top10WndTitle ) - 1
		);
	//
	for ( i = 0; i < TOP_TBL_SIZE; i++ )
	{
		//
		kos_WriteTextToWindow(
			6, 21 + 2 + (i * 10),
			0x0, enterName != i ? 0xFFFFFF : 0x00FF00,
			heroTbl[i].name,
			sizeof( heroTbl[0].name )
			);
		//
		kos_DisplayNumberToWindow(
			heroTbl[i].score,
			8,
			112, 21 + 2 + (i * 10),
			0xFFFF55,
			nbDecimal,
			false
			);
	}
	//
	kos_WriteTextToWindow(
		6, 21 + 6 + (i * 10),
		0x10, 0x1060D0,
		enterName >= 0 ? top10str1 : top10str2,
		enterName >= 0 ? sizeof(top10str1) - 1 : sizeof(top10str2) - 1
		);
}


// игровой процесс
void Top10Loop()
{
	Byte keyCode;

	//
	SetUpTop10();
	//
	while ( true )
	{
		switch ( kos_WaitForEvent() )
		{
		//
		case 1:
			DrawTop10Window();
			break;
		//
		case 2:
			//
			kos_GetKey( keyCode );
			//
			if ( enterName < 0 )
			{
				//
				if ( keyCode == 0x1b )
				{
					//
					return;
				}
			}
			else
			{
				//
				switch ( keyCode )
				{
				//
				case 13:
					//
					enterName = -1;
					break;
				//
				case 8:
					//
					if ( enterCharNdx > 0 )
					{
						//
						heroTbl[enterName].name[--enterCharNdx] = '.';
					}
					break;
				//
				default:
					if ( keyCode >= 0x20 )
					{
						//
						heroTbl[enterName].name[enterCharNdx++] = keyCode;
						//
						if ( enterCharNdx >= sizeof(heroTbl[0].name) )
						{
							//
							enterName = -1;
						}
					}
					break;
				}
				//
				DrawTop10Window();
			}
			//
			break;
		//
		default:
			break;
		}
	}
}