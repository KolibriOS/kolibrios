#include "kosSyst.h"
#include "mcarray.h"
#include "lang.h"


// битмап пустого места
RGB bmEmpty[] = {
	0x201010, 0x101020, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010,
	0x101010, 0x102010, 0x201010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010,
	0x101010, 0x101010, 0x101020, 0x102010, 0x101010, 0x101010, 0x101010, 0x101010,
	0x101010, 0x101010, 0x101010, 0x201010, 0x101020, 0x101010, 0x101010, 0x101010,
	0x101010, 0x101010, 0x101010, 0x101010, 0x102010, 0x201010, 0x101010, 0x101010,
	0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101020, 0x102010, 0x101010,
	0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x201010, 0x101020,
	0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x102010
};


// битмап игрока
RGB bmHero[] = {
	0x2020E0, 0x2020E0, 0x2020E0, 0x2020E0, 0x2020E0, 0x2020E0, 0x2020E0, 0x2020C0,
	0x2020E0, 0x2020E0, 0x2020E0, 0x2020E0, 0x2020E0, 0x2020E0, 0x2020C0, 0x2020A0,
	0x2020E0, 0x2020E0, 0x2020C0, 0x2020C0, 0x2020C0, 0x2020C0, 0x2020A0, 0x2020A0,
	0x2020E0, 0x2020E0, 0x2020C0, 0x2020C0, 0x2020C0, 0x2020C0, 0x2020A0, 0x2020A0,
	0x2020E0, 0x2020E0, 0x2020C0, 0x2020C0, 0x2020C0, 0x2020C0, 0x2020A0, 0x2020A0,
	0x2020E0, 0x2020E0, 0x2020C0, 0x2020C0, 0x2020C0, 0x2020C0, 0x2020A0, 0x2020A0,
	0x2020E0, 0x2020C0, 0x2020A0, 0x2020A0, 0x2020A0, 0x2020A0, 0x2020A0, 0x2020A0,
	0x2020C0, 0x2020A0, 0x2020A0, 0x2020A0, 0x2020A0, 0x2020A0, 0x2020A0, 0x2020A0
};


// битмап игрока
RGB bmSuperHero[] = {
	0x5720D0, 0x5720D0, 0x5720D0, 0x5720D0, 0x5720D0, 0x5720D0, 0x5720D0, 0x5720B0,
	0x5720D0, 0x5720D0, 0x5720D0, 0x5720D0, 0x5720D0, 0x5720D0, 0x5720B0, 0x572090,
	0x5720D0, 0x5720D0, 0x5720B0, 0x5720B0, 0x5720B0, 0x5720B0, 0x572090, 0x572090,
	0x5720D0, 0x5720D0, 0x5720B0, 0x5720B0, 0x5720B0, 0x5720B0, 0x572090, 0x572090,
	0x5720D0, 0x5720D0, 0x5720B0, 0x5720B0, 0x5720B0, 0x5720B0, 0x572090, 0x572090,
	0x5720D0, 0x5720D0, 0x5720B0, 0x5720B0, 0x5720B0, 0x5720B0, 0x572090, 0x572090,
	0x5720D0, 0x5720B0, 0x572090, 0x572090, 0x572090, 0x572090, 0x572090, 0x572090,
	0x5720B0, 0x572090, 0x572090, 0x572090, 0x572090, 0x572090, 0x572090, 0x572090
};


// битмап гада, бегающего по заполненной местности
RGB bmEnemy1[] = {
	0xE02020, 0xE02020, 0xE02020, 0xE02020, 0xE02020, 0xE02020, 0xE02020, 0xC02020,
	0xE02020, 0xE02020, 0xE02020, 0xE02020, 0xE02020, 0xE02020, 0xC02020, 0xA02020,
	0xE02020, 0xE02020, 0xC02020, 0xC02020, 0xC02020, 0xC02020, 0xA02020, 0xA02020,
	0xE02020, 0xE02020, 0xC02020, 0xC02020, 0xC02020, 0xC02020, 0xA02020, 0xA02020,
	0xE02020, 0xE02020, 0xC02020, 0xC02020, 0xC02020, 0xC02020, 0xA02020, 0xA02020,
	0xE02020, 0xE02020, 0xC02020, 0xC02020, 0xC02020, 0xC02020, 0xA02020, 0xA02020,
	0xE02020, 0xC02020, 0xA02020, 0xA02020, 0xA02020, 0xA02020, 0xA02020, 0xA02020,
	0xC02020, 0xA02020, 0xA02020, 0xA02020, 0xA02020, 0xA02020, 0xA02020, 0xA02020
};


// битмап гада, бегающего по пустому месту
RGB bmEnemy2[] = {
	0xE08020, 0xE08020, 0xE08020, 0xE08020, 0xE08020, 0xE08020, 0xE08020, 0xC08020,
	0xE08020, 0xE08020, 0xE08020, 0xE08020, 0xE08020, 0xE08020, 0xC08020, 0xA08020,
	0xE08020, 0xE08020, 0xC08020, 0xC08020, 0xC08020, 0xC08020, 0xA08020, 0xA08020,
	0xE08020, 0xE08020, 0xC08020, 0xC08020, 0xC08020, 0xC08020, 0xA08020, 0xA08020,
	0xE08020, 0xE08020, 0xC08020, 0xC08020, 0xC08020, 0xC08020, 0xA08020, 0xA08020,
	0xE08020, 0xE08020, 0xC08020, 0xC08020, 0xC08020, 0xC08020, 0xA08020, 0xA08020,
	0xE08020, 0xC08020, 0xA08020, 0xA08020, 0xA08020, 0xA08020, 0xA08020, 0xA08020,
	0xC08020, 0xA08020, 0xA08020, 0xA08020, 0xA08020, 0xA08020, 0xA08020, 0xA08020
};


// битмап заполнени€
RGB bmWall[] = {
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xCCCCCC,
	0xFFFFFF, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xAAAAAA,
	0xFFFFFF, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xAAAAAA,
	0xFFFFFF, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xAAAAAA,
	0xFFFFFF, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xAAAAAA,
	0xFFFFFF, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xAAAAAA,
	0xFFFFFF, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xCCCCCC, 0xAAAAAA,
	0xCCCCCC, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA
};


// след игрока
RGB bmTrack[] = {
	0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010,
	0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010,
	0x101010, 0x101010, 0x101010, 0x1010F0, 0x1010F0, 0x101010, 0x101010, 0x101010,
	0x101010, 0x101010, 0x1010F0, 0x1010F0, 0x1010F0, 0x1010F0, 0x101010, 0x101010,
	0x101010, 0x101010, 0x1010F0, 0x1010F0, 0x1010F0, 0x1010F0, 0x101010, 0x101010,
	0x101010, 0x101010, 0x101010, 0x1010F0, 0x1010F0, 0x101010, 0x101010, 0x101010,
	0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010,
	0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010
};


// след игрока
RGB bmSuperTrack[] = {
	0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010,
	0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010,
	0x101010, 0x101010, 0x101010, 0x5310D0, 0x5310D0, 0x101010, 0x101010, 0x101010,
	0x101010, 0x101010, 0x5310D0, 0x5310D0, 0x5310D0, 0x5310D0, 0x101010, 0x101010,
	0x101010, 0x101010, 0x5310D0, 0x5310D0, 0x5310D0, 0x5310D0, 0x101010, 0x101010,
	0x101010, 0x101010, 0x101010, 0x5310D0, 0x5310D0, 0x101010, 0x101010, 0x101010,
	0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010,
	0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010, 0x101010
};


// заполнение экрана дл€ смены уровн€
RGB bmFlip[] = {
	0x101010, 0xF0F0F0, 0x101010, 0xF0F0F0, 0x101010, 0xF0F0F0, 0x101010, 0xF0F0F0,
	0xF0F0F0, 0x101010, 0xF0F0F0, 0x101010, 0xF0F0F0, 0x101010, 0xF0F0F0, 0x101010,
	0x101010, 0xF0F0F0, 0x101010, 0xF0F0F0, 0x101010, 0xF0F0F0, 0x101010, 0xF0F0F0,
	0xF0F0F0, 0x101010, 0xF0F0F0, 0x101010, 0xF0F0F0, 0x101010, 0xF0F0F0, 0x101010,
	0x101010, 0xF0F0F0, 0x101010, 0xF0F0F0, 0x101010, 0xF0F0F0, 0x101010, 0xF0F0F0,
	0xF0F0F0, 0x101010, 0xF0F0F0, 0x101010, 0xF0F0F0, 0x101010, 0xF0F0F0, 0x101010,
	0x101010, 0xF0F0F0, 0x101010, 0xF0F0F0, 0x101010, 0xF0F0F0, 0x101010, 0xF0F0F0,
	0xF0F0F0, 0x101010, 0xF0F0F0, 0x101010, 0xF0F0F0, 0x101010, 0xF0F0F0, 0x101010
};


// бонус неу€звимости
RGB bmBonus1[] = {
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xCCCCCC,
	0xFFFFFF, 0xCCCCCC, 0xCCCCCC, 0x44AC44, 0x44AC44, 0xCCCCCC, 0xCCCCCC, 0xAAAAAA,
	0xFFFFFF, 0xCCCCCC, 0x44AC44, 0x44AC44, 0x44AC44, 0x44AC44, 0xCCCCCC, 0xAAAAAA,
	0xFFFFFF, 0x44AC44, 0x44AC44, 0x0C8C0C, 0x0C8C0C, 0x44AC44, 0x44AC44, 0xAAAAAA,
	0xFFFFFF, 0x44AC44, 0x44AC44, 0x0C8C0C, 0x0C8C0C, 0x44AC44, 0x44AC44, 0xAAAAAA,
	0xFFFFFF, 0xCCCCCC, 0x44AC44, 0x44AC44, 0x44AC44, 0x44AC44, 0xCCCCCC, 0xAAAAAA,
	0xFFFFFF, 0xCCCCCC, 0xCCCCCC, 0x44AC44, 0x44AC44, 0xCCCCCC, 0xCCCCCC, 0xAAAAAA,
	0xCCCCCC, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA
};


// бонус жизни
RGB bmBonus2[] = {
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xCCCCCC,
	0xFFFFFF, 0xCCCCCC, 0xD41414, 0xCCCCCC, 0xCCCCCC, 0xD41414, 0xCCCCCC, 0xAAAAAA,
	0xFFFFFF, 0xD41414, 0xD41414, 0xD41414, 0xD41414, 0xD41414, 0xD41414, 0xAAAAAA,
	0xFFFFFF, 0xD41414, 0xD41414, 0xD41414, 0xD41414, 0xD41414, 0xD41414, 0xAAAAAA,
	0xFFFFFF, 0xD41414, 0xD41414, 0xD41414, 0xD41414, 0xD41414, 0xD41414, 0xAAAAAA,
	0xFFFFFF, 0xCCCCCC, 0xD41414, 0xD41414, 0xD41414, 0xD41414, 0xCCCCCC, 0xAAAAAA,
	0xFFFFFF, 0xCCCCCC, 0xCCCCCC, 0xD41414, 0xD41414, 0xCCCCCC, 0xCCCCCC, 0xAAAAAA,
	0xCCCCCC, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA
};


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
void DrawAppWindow();
//
void DrawTop10Window();
//
void initWorldMap();
//
void drawWorldMap();
//
void clearWorldMap();
//
void drawWorldMapForFlip();
//
void drawWndTitleGo();
//
void ApplyMapDiffs( bool drawTitle = true );
//
int GetCompletePercents();


#define EAT_ENEMY_BONUS		100

#define BEFORE_START_LEVEL	100
#define BONUS1_LIFETIME		250
#define BONUS1_IND_HSIZE	6

#define MIN_LOOP_DELAY		1
#define MAX_LOOP_DELAY		20
#define DEFAULT_LOOP_DELAY	12

#define blockSize			8

#define ENTRY_CLIENT_SIZE_X	(400 - 2)
#define ENTRY_CLIENT_SIZE_Y	(144 - 2 - 20)

#define TOP10_CLIENT_SIZE_X	(176 - 2)
#define TOP10_CLIENT_SIZE_Y	(144 - 2 - 20)

#define MAX_X_SIZE			96
#define MIN_X_SIZE			48
#define MAX_Y_SIZE			56
#define MIN_Y_SIZE			28

#define flipMapSize			((mapSizeX * mapSizeY) / 4)
#define freeSpaceCount		((mapSizeX - 4) * (mapSizeY - 4))
//
#define	gmEmpty				0
#define gmHero				1
#define gmEnemy1			2
#define gmEnemy2			3
#define gmWall				4
#define gmTrack				5
#define gmFlip				6
#define gmBonus1			7
#define gmBonus2			8
#define gmSuperHero			9
#define gmSuperTrack		10
#define gmProbe				11

#define appStateEntry		0
#define appStateGo			1
#define appStateHideMap		2
#define appStateShowMap		3
#define appStatePause		4
#define appStateAfterDeath	5
#define appStateTop10		6

#define spacePerEnemy		30


#define BT_SIZE_X_PLUS		2
#define BT_SIZE_X_MINUS		3
#define BT_LOOP_PLUS		4
#define BT_LOOP_MINUS		5
#define BT_SIZE_Y_PLUS		6
#define BT_SIZE_Y_MINUS		7

#define TOP_TBL_SIZE		10


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
char top10FilePath[MAX_PATH];
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
		this->fi.offsetLow = this->fi.offsetHigh = 0;
		this->fi.dataCount = 0;
		this->fi.bufferPtr = this->block;
		this->fi.rwMode = 0;
		memcpy( this->fi.fileURL, top10FilePath, sizeof( top10FilePath ) );
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

sProcessInfo process_info;

// заголовок главного окна
char MainWindowTitle[] = "XONIX (C) MMVI by Rabid Rabbit";
char WindowTitle[128];
char Top10WndTitle[] = "Top 10";
#if LANG == RUS
char goWndTitle[] = "УаЃҐ•≠м %U, І†Ґ•аи•≠Ѓ %U%%, ¶®І≠•©: %U, бзсв: %U";
char goWndTitleSuperHero[] = "УаЃҐ•≠м %U, І†Ґ•аи•≠Ѓ %U%%, ¶®І≠•©: %U, бзсв: %U, бгѓ•а£•аЃ©: %U%%";
char menuStr1[] = "1. Н†з†вм ®£аг";
char menuStr2[] = "2. ВлеЃ§";
char menuStr3[] = "Уѓа†ҐЂ•≠®•: СТРЕЛКИ - ≠†ѓа†ҐЂ•≠®• §Ґ®¶•≠®п.";
char menuStr4[] = "SPACE - ѓ†гІ†, ESC - ҐлеЃ§ Ґ ђ•≠о.";
char thatsAllStr[] = "И£а† Ѓ™Ѓ≠з•≠†.";
char worldSizeStr[] = "Р†Іђ•а ѓЃЂп %U x %U нЂ•ђ•≠вЃҐ.";
char mainLoopDelayStr[] = "С™ЃаЃбвм %U";
char top10str1[] = "ENTER - ®ђп Ok.";
char top10str2[] = "ESC - ҐлеЃ§ Ґ ђ•≠о";
#else
char goWndTitle[] = "Level %U, completed %U%%, lives: %U, scores: %U";
char goWndTitleSuperHero[] = "Level %U, completed %U%%, lives: %U, scores: %U, superhero: %U%%";
char menuStr1[] = "1. Start game";
char menuStr2[] = "2. Exit";
char menuStr3[] = "Control: ARROWS - direction of movement.";
char menuStr4[] = "SPACE - pause, ESC - leave to menu.";
char thatsAllStr[] = "Game finished.";
char worldSizeStr[] = "Field size %U x %U cells.";
char mainLoopDelayStr[] = "Speed %U";
char top10str1[] = "Enter - name Ok.";
char top10str2[] = "ESC - leave to menu";
#endif
//
Byte beep1[] = { 0x90, 0x33, 0 };
//
Byte *heroPtr = NULL;
int heroDX = 0, heroDY = 0, lastMoveDirection = 0;
//
Byte * worldMap = NULL;
//
int	wndSizeX = 0;
int	wndSizeY = 0;
int mapSizeX = 64;
int mapSizeY = 32;
int loopDelay = DEFAULT_LOOP_DELAY;
int currentLevel = 1;
int appState = appStateEntry;
int levelFillEdge = 0;
int levelFillCount = 0;
int lifeCount = 0;
int flipMapCount = 0;
bool noBonus = false;
bool goToNextLevel = false;
bool bonus1Set = false;
bool bonus2Set = false;
int bonus1Count = 0;
int currentHero = gmHero;
int currentTrack = gmTrack;
Dword scoreCount = 0;
int enterName = -1;
int enterCharNdx = 0;
//
MCArray<Byte*> fillList;

//
struct flipMapEl
{
	Word x, y;
};

//
flipMapEl *flipMapPtr = NULL;


//
RGB *mapColours[] = {
	bmEmpty,
	bmHero,
	bmEnemy1,
	bmEnemy2,
	bmWall,
	bmTrack,
	bmFlip,
	bmBonus1,
	bmBonus2,
	bmSuperHero,
	bmSuperTrack,
	NULL
};


//
struct sMapDiff
{
	Byte *elPtr;
	Byte mapEl;
	//
	sMapDiff() {};
	//
	sMapDiff( Byte *sElPtr, Byte sMapEl )
	{
		this->elPtr = sElPtr;
		this->mapEl = sMapEl;
	};
};


//
class CMapDiff : public MCArray<sMapDiff>
{
public:
	virtual int Add( const sMapDiff &element )
	{
		element.elPtr[0] = element.mapEl;
		return MCArray<sMapDiff>::Add( element );
	}
};


//
CMapDiff mapDiffList;
MCArray<Byte*> sTrackList;


//
class CGenericEnemy
{
public:
	//
	Byte *ePtr;
	int dx, dy;
	//
	virtual bool Move(void) = 0;
};

class CWallEnemy : public CGenericEnemy
{
public:
	virtual bool Move(void);
};

class CSpaceEnemy : public CGenericEnemy
{
public:
	virtual bool Move(void);
};



//
bool CWallEnemy::Move()
{
	int ddx;
	Byte *nextPtr;
	Byte mapEl, dirMap;
	bool result, border;

	//
	result = false;
	border = false;
	//
	ddx = ( this->ePtr - worldMap ) % mapSizeX;
	//
	if ( ddx == 0 && this->dx < 0 )
	{
		border = true;
		this->dx = 0 - this->dx;
	}
	//
	if ( ddx == (mapSizeX - 1) && this->dx > 0 )
	{
		border = true;
		this->dx = 0 - this->dx;
	}
	//
	ddx = ( this->ePtr - worldMap ) / mapSizeX;
	//
	if ( ddx == 0 && this->dy < 0 )
	{
		border = true;
		this->dy = 0 - this->dy;
	}
	//
	if ( ddx == (mapSizeY - 1) && this->dy > 0 )
	{
		border = true;
		this->dy = 0 - this->dy;
	}
	// получим координаты места, в которое попадает объект
	nextPtr = this->ePtr + ( this->dx + this->dy );
	// получим элемент с карты
	mapEl = nextPtr[0];
	// 
	// в зависимости от элемента
	switch ( mapEl )
	{
	// напоролись на игрока
	case gmHero:
		if ( sTrackList.GetCount() <= 0 )
		{
			result = true;
			break;
		}

	// пустое место, след игрока или гады на поле - надо отскакивать
	case gmSuperHero:
	case gmSuperTrack:
	case gmTrack:
	case gmEnemy2:
	case gmEmpty:
		//
		dirMap = 0;
		// -dx +dy
		mapEl = this->ePtr[this->dy - this->dx];
		if ( mapEl == gmEmpty
			|| mapEl == gmTrack
			|| mapEl == gmEnemy2
			|| mapEl == gmSuperHero
			|| mapEl == gmSuperTrack
			) dirMap |= 1;
		// +dy
		mapEl = this->ePtr[this->dy];
		if ( mapEl == gmEmpty
			|| mapEl == gmTrack
			|| mapEl == gmEnemy2
			|| mapEl == gmSuperHero
			|| mapEl == gmSuperTrack
			) dirMap |= 2;
		// +dx
		mapEl = this->ePtr[this->dx];
		if ( mapEl == gmEmpty
			|| mapEl == gmTrack
			|| mapEl == gmEnemy2
			|| mapEl == gmSuperHero
			|| mapEl == gmSuperTrack
			) dirMap |= 4;
		// +dx -dy
		mapEl = this->ePtr[this->dx - this->dy];
		if ( mapEl == gmEmpty
			|| mapEl == gmTrack
			|| mapEl == gmEnemy2
			|| mapEl == gmSuperHero
			|| mapEl == gmSuperTrack
			) dirMap |= 8;
		//
		switch ( dirMap )
		{
		case 2:
		case 3:
			this->dy = 0 - this->dy;
			break;

		case 4:
		case 12:
			this->dx = 0 - this->dx;
			break;

		default:
			this->dx = 0 - this->dx;
			this->dy = 0 - this->dy;
			break;
		}
		//
		nextPtr = this->ePtr + ( this->dx + this->dy );
		// получим элемент с карты
		mapEl = nextPtr[0];
		//
		switch ( mapEl )
		{
		//
		case gmHero:
			if ( sTrackList.GetCount() <= 0 )
			{
				result = true;
			}

		//
		case gmSuperHero:
		case gmSuperTrack:
		case gmTrack:
		case gmEmpty:
		case gmEnemy2:
			break;

		//
		default:
			// стираем объект
			mapDiffList.Add( sMapDiff( this->ePtr, gmWall ) );
			// переместим объект
			this->ePtr = nextPtr;
			// рисуем объект по новым координатам
			mapDiffList.Add( sMapDiff( this->ePtr, gmEnemy1 ) );
			break;
		}
		//
		break;

	// летим
	default:
		// стираем объект
		mapDiffList.Add( sMapDiff( this->ePtr, gmWall ) );
		// переместим объект
		this->ePtr = nextPtr;
		// рисуем объект по новым координатам
		mapDiffList.Add( sMapDiff( this->ePtr, gmEnemy1 ) );
		//
		break;

	}
	//
	return result;
}

//
bool CSpaceEnemy::Move()
{
	Byte *nextPtr;
	Byte mapEl, dirMap;
	bool result, heroTrack;

	//
	result = false;
	//
	heroTrack = ( sTrackList.GetCount() > 0 );
	// получим координаты места, в которое попадает объект
	nextPtr = this->ePtr + ( this->dx + this->dy );
	// получим элемент с карты
	mapEl = nextPtr[0];
	// в зависимости от элемента
	switch ( mapEl )
	{
	// напоролись на игрока или его след
	case gmTrack:
		result = true;
		break;

	//
	case gmHero:
		if ( heroTrack )
		{
			result = true;
			break;
		}

	// надо отскакивать
	case gmSuperHero:
	case gmSuperTrack:
	case gmBonus1:
	case gmBonus2:
	case gmEnemy1:
	case gmWall:
		//
		dirMap = 0;
		// -dx +dy
		mapEl = this->ePtr[this->dy - this->dx];
		if ( mapEl == gmWall ||
			mapEl == gmEnemy1 ||
			mapEl == gmBonus1 ||
			mapEl == gmBonus2 ||
			mapEl == gmSuperHero ||
			mapEl == gmSuperTrack ||
			( mapEl == gmHero && !heroTrack )
			) dirMap |= 1;
		// +dy
		mapEl = this->ePtr[this->dy];
		if ( mapEl == gmWall ||
			mapEl == gmEnemy1 ||
			mapEl == gmBonus1 ||
			mapEl == gmBonus2 ||
			mapEl == gmSuperHero ||
			mapEl == gmSuperTrack ||
			( mapEl == gmHero && !heroTrack )
			) dirMap |= 2;
		// +dx
		mapEl = this->ePtr[this->dx];
		if ( mapEl == gmWall ||
			mapEl == gmEnemy1 ||
			mapEl == gmBonus1 ||
			mapEl == gmBonus2 ||
			mapEl == gmSuperHero ||
			mapEl == gmSuperTrack ||
			( mapEl == gmHero && !heroTrack )
			) dirMap |= 4;
		// +dx -dy
		mapEl = this->ePtr[this->dx - this->dy];
		if ( mapEl == gmWall ||
			mapEl == gmEnemy1 ||
			mapEl == gmBonus1 ||
			mapEl == gmBonus2 ||
			mapEl == gmSuperHero ||
			mapEl == gmSuperTrack ||
			( mapEl == gmHero && !heroTrack )
			) dirMap |= 8;
		//
		switch ( dirMap )
		{
		case 2:
		case 3:
			this->dy = 0 - this->dy;
			break;

		case 4:
		case 12:
			this->dx = 0 - this->dx;
			break;

		default:
			this->dx = 0 - this->dx;
			this->dy = 0 - this->dy;
			break;
		}
		//
		nextPtr = this->ePtr + ( this->dx + this->dy );
		// получим элемент с карты
		mapEl = nextPtr[0];
		//
		switch ( mapEl )
		{
		//
		case gmTrack:
			result = true;
			break;

		//
		case gmHero:
			if ( heroTrack )
			{
				result = true;
				break;
			}

		//
		case gmSuperHero:
		case gmSuperTrack:
		case gmBonus1:
		case gmBonus2:
		case gmWall:
		case gmEnemy1:
			break;

		//
		default:
			// стираем объект
			mapDiffList.Add( sMapDiff( this->ePtr, gmEmpty ) );
			// переместим объект
			this->ePtr = nextPtr;
			// рисуем объект по новым координатам
			mapDiffList.Add( sMapDiff( this->ePtr, gmEnemy2 ) );
			break;
		}
		//
		break;

	// летим
	default:
		// стираем объект
		mapDiffList.Add( sMapDiff( this->ePtr, gmEmpty ) );
		// переместим объект
		this->ePtr = nextPtr;
		// рисуем объект по новым координатам
		mapDiffList.Add( sMapDiff( this->ePtr, gmEnemy2 ) );
		//
		break;

	}
	//

	//
	return result;
}


//
MCArray<CGenericEnemy *> mapEnemies;


//
void xonixFree(void)
{
	clearWorldMap();
	if ( flipMapPtr != NULL )
	{
		delete flipMapPtr;
		flipMapPtr = NULL;
	}
}


//
void checkAndSetBonus2()
{
	Dword i;

	//
	if ( (!bonus2Set)
		&& rtlRand() < 0x40000000
		&& lifeCount < 3
		&& GetCompletePercents() > 50 )
	{
		//
		bonus2Set = true;
		//
		for ( i = rtlRand() % (mapSizeX * mapSizeY); worldMap[i] != gmWall; i = rtlRand() % (mapSizeX * mapSizeY) );
		//
		mapDiffList.Add( sMapDiff( worldMap + i, gmBonus2 ) );
	}
}


//
void ChangeHero()
{
	if ( bonus1Count < 1 )
	{
		currentHero = gmHero;
		currentTrack = gmTrack;
	}
	else
	{
		currentHero = gmSuperHero;
		currentTrack = gmSuperTrack;
	}
}


//
void checkAndSetBonus1()
{
	Dword i;

	//
	if ((!bonus1Set)
		&& rtlRand() > 0x80000000
		&& lifeCount < 2
		&& GetCompletePercents() > 75 )
		
	{
		//
		bonus1Set = true;
		//
		for ( i = rtlRand() % (mapSizeX * mapSizeY); worldMap[i] != gmWall; i = rtlRand() % (mapSizeX * mapSizeY) );
		//
		mapDiffList.Add( sMapDiff( worldMap + i, gmBonus1 ) );
	}
}


//
void CreateFlipMap(void)
{
	Word i, j;
	int ndx, ndx2, k;
	flipMapEl el;
	static int lastMapSizeX = 0, lastMapSizeY = 0;

	//
	if ( lastMapSizeX != mapSizeX || lastMapSizeY != mapSizeY )
	{
		//
		lastMapSizeX = mapSizeX;
		lastMapSizeY = mapSizeY;
		//
		if ( flipMapPtr != NULL )
		{
			delete flipMapPtr;
			flipMapPtr = NULL;
		}
	}
	//
	if ( flipMapPtr == NULL )
	{
		flipMapPtr = new flipMapEl[flipMapSize];
		//
		ndx = 0;
		//
		for ( i = 0; i < mapSizeY; i += 2 )
		{
			for ( j = 0; j < mapSizeX; j += 2 )
			{
				//
				flipMapPtr[ndx].x = j;
				flipMapPtr[ndx].y = i;
				//
				ndx++;
			}
		}
	}
	//
	for ( k = 0; k < flipMapSize; k++ )
	{
		//
		ndx = rtlRand() % flipMapSize;
		ndx2 = rtlRand() % flipMapSize;
		//
		el = flipMapPtr[ndx];
		flipMapPtr[ndx] = flipMapPtr[ndx2];
		flipMapPtr[ndx2] = el;
	}
}


//
bool ProcessEndTrack()
{
	int i, j, k, m;
	bool noFill;
	Byte *mPtr, *findPtr;

	//
	j = sTrackList.GetCount();
	//
	scoreCount += j;
	//
	for ( i = 0; i < j; i++ )
	{
		//
		mapDiffList.Add( sMapDiff( sTrackList[i], gmWall ) );
	}
	//
	levelFillCount -= j;
	//
	sTrackList.Clear();
	//
	heroPtr += heroDX + heroDY;
	mapDiffList.Add( sMapDiff( heroPtr, currentHero ) );
	//
	heroDX = 0;
	heroDY = 0;
	lastMoveDirection = 0;
	// заливка
	mPtr = worldMap;
	//
	for ( i = 0; i < mapSizeY; i++ )
	{
		for ( j = 0; j < mapSizeX; j++ )
		{
			//
			if ( mPtr[0] == gmEmpty )
			{
				//
				fillList.Clear();
				//
				noFill = false;
				//
				fillList.Add( mPtr );
				//
				mPtr[0] = gmProbe;
				//
				for ( k = 0; k < fillList.GetCount(); k++ )
				{
					// справа
					findPtr = fillList[k] + 1; 
					//
					switch ( findPtr[0] )
					{
					case gmEmpty:
						fillList.Add( findPtr );
						findPtr[0] = gmProbe;
						break;
					case gmEnemy2:
						noFill = true;
						break;
					default:
						break;
					}
					// слева
					findPtr = fillList[k] - 1; 
					//
					switch ( findPtr[0] )
					{
					case gmEmpty:
						fillList.Add( findPtr );
						findPtr[0] = gmProbe;
						break;
					case gmEnemy2:
						noFill = true;
						break;
					default:
						break;
					}
					// сверху
					findPtr = fillList[k] - mapSizeX; 
					//
					switch ( findPtr[0] )
					{
					case gmEmpty:
						fillList.Add( findPtr );
						findPtr[0] = gmProbe;
						break;
					case gmEnemy2:
						noFill = true;
						break;
					default:
						break;
					}
					// снизу
					findPtr = fillList[k] + mapSizeX; 
					//
					switch ( findPtr[0] )
					{
					case gmEmpty:
						fillList.Add( findPtr );
						findPtr[0] = gmProbe;
						break;
					case gmEnemy2:
						noFill = true;
						break;
					default:
						break;
					}
				}
				//
				if ( noFill )
				{
					//
					fillList.Clear();
				}
				else
				{
					//
					m = fillList.GetCount();
					//
					scoreCount += m;
					//
					for ( k = 0; k < m; k++ )
					{
						//
						mapDiffList.Add( sMapDiff( fillList[k], gmWall ) );
					}
					//
					levelFillCount -= m;
				}
			}
			else
			{
				mPtr++;
			}
		}
	}
	//
	mPtr = worldMap;
	//
	for ( i = 0; i < mapSizeY; i++ )
	{
		for ( j = 0; j < mapSizeX; j++ )
		{
			//
			if ( mPtr[0] == gmProbe ) mPtr[0] = gmEmpty;
			//
			mPtr++;
		}
	}
	//
	checkAndSetBonus1();
	checkAndSetBonus2();
	//
	ApplyMapDiffs();
	//
	return levelFillCount <= levelFillEdge;
}	


//
void EatEnemy( Byte *enemyPos )
{
	bool Eat = true;
	int i, j;

	//
	while ( Eat )
	{
		//
		Eat = false;
		//
		j = mapEnemies.GetCount();
		//
		for ( i = 0; i < j; i++ )
		{
			//
			if ( mapEnemies[i]->ePtr == enemyPos )
			{
				//
				delete mapEnemies[i];
				//
				mapEnemies.RemoveAt( i );
				//
				Eat = true;
				//
				scoreCount += EAT_ENEMY_BONUS;
				//
				break;
			}
		}
	}
}


//
bool MoveHero()
{
	int ddx;
	Byte *nextPtr;
	Byte mapEl;
	bool result;

	//
	if ( heroDX == 0 && heroDY == 0 ) return false;
	//
	result = false;
	//
	nextPtr = heroPtr + ( heroDX + heroDY );
	//
	ddx = ( ( heroPtr - worldMap ) % mapSizeX ) - ( ( nextPtr - worldMap ) % mapSizeX );
	//
	if ( ddx < -1 || ddx > 1 || nextPtr < worldMap || nextPtr >= ( worldMap + ( mapSizeX * mapSizeY ) ) )
	{
		heroDX = 0;
		heroDY = 0;
		return false;
	}


	//
	mapEl = nextPtr[0];
	//
	if ( sTrackList.GetCount() > 0 )
	{
		//
		switch ( mapEl )
		{
		//
		case gmEmpty:
			sTrackList.Add( nextPtr );
			break;
		//
		case gmBonus1:
			bonus1Count = BONUS1_LIFETIME;
			ChangeHero();
			goToNextLevel = ProcessEndTrack();
			return false;
			break;
		//
		case gmBonus2:
			lifeCount++;
			goToNextLevel = ProcessEndTrack();
			return false;
			break;
		//
		case gmWall:
			goToNextLevel = ProcessEndTrack();
			return false;
			break;
		//
		case gmEnemy1:
			if ( bonus1Count > 0 )
			{
				//
				EatEnemy( nextPtr );
				//
				goToNextLevel = ProcessEndTrack();
				//
				return false;
				break;
			}
			else
			{
				//
				return true;
			}
			break;
		//
		case gmEnemy2:
			if ( bonus1Count > 0 )
			{
				//
				EatEnemy( nextPtr );
				sTrackList.Add( nextPtr );
				break;
			}
			else
			{
				//
				return true;
			}
			break;
		//
		default:
			return true;
			break;
		}
	}
	else
	{
		//
		switch ( mapEl )
		{
		//
		case gmEmpty:
			sTrackList.Add( nextPtr );
			break;
		//
		case gmBonus1:
			bonus1Count = BONUS1_LIFETIME;
			break;
		//
		case gmBonus2:
			lifeCount++;
			break;
		//
		case gmWall:
			break;
		//
		case gmEnemy1:
			if ( bonus1Count > 0 )
			{
				EatEnemy( nextPtr );
			}
			else
			{
				result = true;
			}
			break;
		//
		case gmEnemy2:
			if ( bonus1Count > 0 )
			{
				EatEnemy( nextPtr );
				sTrackList.Add( nextPtr );
			}
			else
			{
				result = true;
			}
			break;
		//
		default:
			result = true;
			break;
		}
	}

	//
	mapDiffList.Add( sMapDiff( heroPtr, sTrackList.GetCount() <= 1 ? gmWall : currentTrack ) );
	heroPtr = nextPtr;
	mapDiffList.Add( sMapDiff( heroPtr, currentHero ) );

	return result;
}


//
bool MoveEnemies()
{
	bool result;
	int i, j, ir;

	//
	result = false;
	ir = 0;
	//
	j = mapEnemies.GetCount();
	//
	for ( i = 0; i < j; i++ )
	{
		ir += ( mapEnemies[i]->Move() ? 1 : 0 );
	}
	//
	result = ( ir != 0 );
	//
	return result;
}


//
void ApplyMapDiffs( bool drawTitle )
{
	int i, j;

	//
//	kos_WindowRedrawStatus( 1 );
	//
	if ( drawTitle ) drawWndTitleGo();
	//
	j = mapDiffList.GetCount();
	//
	for ( i = 0; i < j; i++ )
	{
		kos_PutImage(
			mapColours[mapDiffList[i].mapEl],
			blockSize,
			blockSize,
			( ( mapDiffList[i].elPtr - worldMap ) % mapSizeX ) * blockSize,
			( ( mapDiffList[i].elPtr - worldMap ) / mapSizeX ) * blockSize
			);
	}
	//
//	kos_WindowRedrawStatus( 2 );
	//
	mapDiffList.Clear();
}


//
void DeadHeroProcess()
{
	int i, j;
	Byte *mPtr = beep1;

	// beep
	__asm{
		mov eax, 55
		mov ebx, eax
		mov esi, mPtr
		push eax
		int 0x40
		pop eax
	}
	//
	j = sTrackList.GetCount();
	//
	for ( i = 0; i < j; i++ )
	{
		//
		mapDiffList.Add( sMapDiff( sTrackList[i], gmEmpty ) );
	}
	//
	mapDiffList.Add( sMapDiff( heroPtr, sTrackList.GetCount() > 0 ? gmEmpty : gmWall ) );
	//
	sTrackList.Clear();
	//
	heroPtr = worldMap;
	//
	while ( heroPtr[0] != gmWall ) heroPtr++;
	//
	mapDiffList.Add( sMapDiff( heroPtr, gmHero ) );
	//
	noBonus = true;
	//
	lifeCount--;
	//
	heroDX = 0;
	heroDY = 0;
	lastMoveDirection = 0;
}


//
bool CheckForNextLevel()
{
	//
	if ( goToNextLevel )
	{
		//
		CreateFlipMap();
		goToNextLevel = false;
		currentLevel++;
		appState = appStateHideMap;
		flipMapCount = 0;
		return true;
	}

	//
	return false;
}


//
void SetGameVars()
{
	//
	currentLevel = 1;
	lifeCount = 3;
	noBonus = true;
	bonus1Set = false;
	bonus2Set = false;
	bonus1Count = 0;
	goToNextLevel = false;
	currentHero = gmHero;
	currentTrack = gmTrack;
	scoreCount = 0;
	enterName = -1;
	//
	wndSizeX = ((mapSizeX*blockSize) + 5*2);
	wndSizeY = ((mapSizeY*blockSize) + kos_GetSkinHeight() + 5);
	//
	kos_ChangeWindow( -1, -1, wndSizeX-1, wndSizeY-1 );
}

//
void SetEntryVars()
{
	//
	wndSizeX = ENTRY_CLIENT_SIZE_X + 5*2;
	wndSizeY = ENTRY_CLIENT_SIZE_Y + kos_GetSkinHeight() + 5;
	//
	kos_SetWindowCaption(MainWindowTitle);
	kos_ChangeWindow( -1, -1, wndSizeX-1, wndSizeY-1 );
	kos_SetKeyboardDataMode( KM_SCANS );
}


//
void __cdecl ReleaseTop10()
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

	kos_SetWindowCaption(Top10WndTitle);
	//
	kos_ChangeWindow(-1, -1, TOP10_CLIENT_SIZE_X + 5 * 2 - 1, TOP10_CLIENT_SIZE_Y + kos_GetSkinHeight() + 5 - 1);
	//
	for ( i = 0; i < TOP_TBL_SIZE; i++ )
	{
		//
		if ( heroTbl[i].score < scoreCount )
		{
			//
			for ( j = TOP_TBL_SIZE - 1; j > i; j-- )
			{
				//
				heroTbl[j] = heroTbl[j-1];
			}
			//
			heroTbl[i].ClearName();
			heroTbl[i].score = scoreCount;
			//
			enterName = i;
			enterCharNdx = 0;
			//
			break;
		}
	}
}


//
// точка входа и функци€ обработки сообщений
//
void kos_Main()
{
	Dword buttonID;
	Byte keyCode;
	Byte *bPtr;
	bool workOn = true;
	char *cPtr;

	wndSizeX = ENTRY_CLIENT_SIZE_X + 5 * 2;
	wndSizeY = ENTRY_CLIENT_SIZE_Y + kos_GetSkinHeight() + 5;

	// отдел€ем им€ модул€ от пути
	cPtr = strrchr( kosExePath, '/' );
	// проверка ;)
	if ( cPtr == NULL )
	{
		//
		rtlDebugOutString( "xonix: Invalid path to executable." );
		//
		return;
	}
	//
	cPtr[1] = 0;
	//
	strcpy( top10FilePath, kosExePath );
	//
	strcpy( top10FilePath + ((cPtr - kosExePath) + 1), "xonix.t10" );

	// выполнение функций инициализации
	kos_SetKeyboardDataMode( KM_SCANS );
	//
	PrepareTop10();

	//
	while( workOn )
	{
		switch ( appState )
		{
		//
		case appStateEntry:
			switch ( kos_WaitForEvent() )
			{
			// перерисовка окна
			case 1:
				DrawAppWindow();
				break;

			//
			case 2:
				kos_GetKey( keyCode );
				switch ( keyCode )
				{
				//
				case 2:
					//
					appState = appStateGo;
					SetGameVars();
					initWorldMap();
//					DrawAppWindow();
					break;

				//
				case 3:
					xonixFree();
					workOn = false;
					break;
				}
				break;
			//
			case 3:
				//
				if ( ! kos_GetButtonID( buttonID ) ) break;
				//
				switch ( buttonID )
				{
				//
				case BT_SIZE_X_PLUS:
					mapSizeX += 2;
					if ( mapSizeX > MAX_X_SIZE ) mapSizeX = MAX_X_SIZE;
					break;
				//
				case BT_SIZE_X_MINUS:
					mapSizeX -= 2;
					if ( mapSizeX < MIN_X_SIZE ) mapSizeX = MIN_X_SIZE;
					break;
				//
				case BT_SIZE_Y_PLUS:
					mapSizeY += 2;
					if ( mapSizeY > MAX_Y_SIZE ) mapSizeY = MAX_Y_SIZE;
					break;
				//
				case BT_SIZE_Y_MINUS:
					mapSizeY -= 2;
					if ( mapSizeY < MIN_Y_SIZE ) mapSizeY = MIN_Y_SIZE;
					break;
				//
				case BT_LOOP_MINUS:
					loopDelay++;
					if ( loopDelay > MAX_LOOP_DELAY ) loopDelay = MAX_LOOP_DELAY;
					break;
				//
				case BT_LOOP_PLUS:
					loopDelay--;
					if ( loopDelay < MIN_LOOP_DELAY ) loopDelay = MIN_LOOP_DELAY;
					break;
				case 1:
					xonixFree();
					workOn = false;
					break;
				//
				default:
					break;
				}
				DrawAppWindow();
				break;
			//
			default:
				break;
			}
			break;
		//
		case appStateGo:
			//
			kos_Pause( loopDelay );
			//
			if ( bonus1Count > 0 ) bonus1Count--;
			//
			ChangeHero();
			//
			switch( kos_WaitForEvent( 1 ) )
			{
			//
			case 0:
				if ( MoveHero() )
				{
					//
					DeadHeroProcess();
				}
				else
				{
					//
					if ( CheckForNextLevel() )
					{
						break;
					}
				}
				if ( MoveEnemies() )
				{
					// сожрали игрока
					DeadHeroProcess();
				}
				ApplyMapDiffs();
				break;
			//
			case 1:
				DrawAppWindow();
				break;

			//
			case 2:
				do kos_GetKey( keyCode ); while ( keyCode & 0x80 );
				switch ( keyCode )
				{
				//
				case 0x1:
					SetEntryVars();
					appState = appStateEntry;
					clearWorldMap();
					DrawAppWindow();
					continue;

				//
				case 0x39:
					appState = appStatePause;
					break;

				//
				case 0x48:
					heroDX = 0;
					if ( lastMoveDirection == 0x50 )
					{
						heroDY = 0;
						lastMoveDirection = 0;
					}
					else
					{
						heroDY = -mapSizeX;
						lastMoveDirection = 0x48;
					}
					break;

				//
				case 0x50:
					heroDX = 0;
					if ( lastMoveDirection == 0x48 )
					{
						heroDY = 0;
						lastMoveDirection = 0;
					}
					else
					{
						heroDY = mapSizeX;
						lastMoveDirection = 0x50;
					}
					break;

				//
				case 0x4B:
					heroDY = 0;
					if ( lastMoveDirection == 0x4D )
					{
						heroDX = 0;
						lastMoveDirection = 0;
					}
					else
					{
						heroDX = -1;
						lastMoveDirection = 0x4B;
					}
					break;

				//
				case 0x4D:
					heroDY = 0;
					if ( lastMoveDirection == 0x4B )
					{
						heroDX = 0;
						lastMoveDirection = 0;
					}
					else
					{
						heroDX = 1;
						lastMoveDirection = 0x4D;
					}
					break;
				}
				//
				if ( MoveHero() )
				{
					//
					DeadHeroProcess();
				}
				else
				{
					//
					if ( CheckForNextLevel() )
					{
						break;
					}
				}
				if ( MoveEnemies() )
				{
					// сожрали игрока
					DeadHeroProcess();
				}
				ApplyMapDiffs();
				break;

			//
			case 3:
				kos_GetButtonID(buttonID);
				if (buttonID == 1)
				{
					xonixFree();
					workOn = false;
				}
				break;
			default:
				//
				if ( MoveHero() )
				{
					//
					DeadHeroProcess();
				}
				if ( MoveEnemies() )
				{
					// сожрали игрока
					DeadHeroProcess();
				}
				ApplyMapDiffs();
				break;
			}
			//
			if ( lifeCount <= 0 )
			{
				appState = appStateAfterDeath;
				DrawAppWindow();
			}
			//
			break;

		//
		case appStateAfterDeath:
			switch ( kos_WaitForEvent() )
			{
			//
			case 1:
				DrawAppWindow();
				break;
			//
			case 2:
				do kos_GetKey( keyCode ); while ( keyCode & 0x80 );
				if ( keyCode != 0 )
				{
					//
					appState = appStateTop10;
					SetUpTop10();
					DrawAppWindow();
				}
				break;
			//
			case 3:
				kos_GetButtonID(buttonID);
				if ( buttonID == 1 )
				{
					//
					appState = appStateTop10;
					SetUpTop10();
					DrawAppWindow();
				}
			//
			default:
				break;
			}
			break;

		//
		case appStateTop10:
			switch ( kos_WaitForEvent() )
			{
			//
			case 1:
				DrawAppWindow();
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
						SetEntryVars();
						clearWorldMap();
						appState = appStateEntry;
						DrawAppWindow();
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
					DrawAppWindow();
				}
				//
				break;
			//
			case 3:
				kos_GetButtonID(buttonID);
				if (buttonID == 1)
				{
					xonixFree();
					workOn = false;
				}
				break;
			default:
				break;
			}
			break;

		//
		case appStatePause:
			switch ( kos_WaitForEvent() )
			{
			case 1:
				DrawAppWindow();
				break;

			case 2:
				do kos_GetKey( keyCode ); while ( keyCode & 0x80 );
				if ( keyCode != 0 )
				{
					//
					appState = appStateGo;
				}
				break;
			case 3:
				kos_GetButtonID(buttonID);
				if (buttonID == 1)
				{
					xonixFree();
					workOn = false;
				}
				break;
			default:
				break;
			}
			break;

		//
		case appStateHideMap:
			//
			switch ( kos_WaitForEvent( 1 ) )
			{
			case 1:
				DrawAppWindow();
				break;
			case 2:
				while ( kos_GetKey( keyCode ) );
				break;
			default:
				bPtr = worldMap + (flipMapPtr[flipMapCount].x + (flipMapPtr[flipMapCount].y * mapSizeX));
				mapDiffList.Add( sMapDiff( bPtr, gmFlip ) );
				mapDiffList.Add( sMapDiff( bPtr + 1, gmFlip ) );
				mapDiffList.Add( sMapDiff( bPtr + mapSizeX, gmFlip ) );
				mapDiffList.Add( sMapDiff( bPtr + (mapSizeX + 1), gmFlip ) );
				ApplyMapDiffs( false );
				break;
			}
			//
			flipMapCount++;
			//
			if ( flipMapCount >= flipMapSize )
			{
				flipMapCount = 0;
				appState = appStateShowMap;
				DrawAppWindow();
			}
			break;
		//
		case appStateShowMap:
			//
			switch ( kos_WaitForEvent( 1 ) )
			{
			case 1:
				DrawAppWindow();
				break;
			default:
				break;
			}
			//
			flipMapCount++;
			//
			if ( flipMapCount >= BEFORE_START_LEVEL )
			{
				clearWorldMap();
				flipMapCount = 0;
				initWorldMap();
				appState = appStateGo;
				DrawAppWindow();
			}
			//
			break;
		}
	}
}


//
void DrawEntryScreen()
{
	PRINTK pr;
	char line[64];
	//
	kos_DefineAndDrawWindow(
		100, 100,
		wndSizeX-1, wndSizeY-1,
		0x34, 0,
		0, 0,
		MainWindowTitle
		);
	//
	if (process_info.processInfo.window_state & 0x06)
		return;

	/*
	kos_WriteTextToWindow(
		4, 4,
		0x10, 0x42D2E2,
		MainWindowTitle,
		sizeof( MainWindowTitle ) - 1
		);
		*/
	//
	kos_WriteTextToWindow(
		8-1, 32-21,
		0x10, 0x12FF12,
		menuStr1,
		sizeof( menuStr1 ) - 1
		);
	//
	kos_WriteTextToWindow(
		8-1, 48-21,
		0x10, 0x12FF12,
		menuStr2,
		sizeof( menuStr2 ) - 1
		);
	//
	kos_WriteTextToWindow(
		8-1, 80-21,
		0x10, 0xD0FF12,
		menuStr3,
		sizeof( menuStr3 ) - 1
		);
	//
	kos_WriteTextToWindow(
		8-1, 96-21,
		0x10, 0xD0FF12,
		menuStr4,
		sizeof( menuStr4 ) - 1
		);
	// размер пол€
	pr.fmtline = worldSizeStr;
	pr.args[0] = mapSizeX;
	pr.args[1] = mapSizeY;
	sprintk( line, &pr );
	//
	kos_WriteTextToWindow(
		8-1, 112-21,
		0x10, 0x12C0D0,
		line,
		strlen( line )
		);
	// кнопки X
	// xo oo
	//	  oo
	kos_DefineButton(
		ENTRY_CLIENT_SIZE_X - 62, 112-24,
		13, 13,
		BT_SIZE_X_MINUS,
		0xCCCCCC
		);
	//
	kos_PutImage( bmPMButton + 12, 6, 2, ENTRY_CLIENT_SIZE_X - (62-1) + 3, 117-23 );
	//
	// ox oo
	//	  oo
	kos_DefineButton(
		ENTRY_CLIENT_SIZE_X - 48, 112-24,
		13, 13,
		BT_SIZE_X_PLUS,
		0xCCCCCC
		);
	//
	kos_PutImage( bmPMButton, 6, 6, ENTRY_CLIENT_SIZE_X - (48-1) + 3, 115-23 );
	// кнопки Y
	// oo xo
	//	  oo
	kos_DefineButton(
		ENTRY_CLIENT_SIZE_X - 31, 112-24,
		13, 13,
		BT_SIZE_Y_MINUS,
		0xCCCCCC
		);
	//
	kos_PutImage( bmPMButton + 12, 6, 2, ENTRY_CLIENT_SIZE_X - (31-1) + 3, 117-23 );
	//
	// oo ox
	//	  oo
	kos_DefineButton(
		ENTRY_CLIENT_SIZE_X - 17, 112-24,
		13, 13,
		BT_SIZE_Y_PLUS,
		0xCCCCCC
		);
	//
	kos_PutImage( bmPMButton, 6, 6, ENTRY_CLIENT_SIZE_X - (17-1) + 3, 115-23 );
	//
	//задержка в цикле выборки сообщений
	pr.fmtline = mainLoopDelayStr;
	pr.args[0] = MAX_LOOP_DELAY + MIN_LOOP_DELAY - loopDelay;
	sprintk( line, &pr );
	//
	kos_WriteTextToWindow(
		8-1, 128-21,
		0x10, 0x12C0D0,
		line,
		strlen( line )
		);
	//
	kos_DefineButton(
		ENTRY_CLIENT_SIZE_X - 31, 128-23,
		13, 13,
		BT_LOOP_MINUS,
		0xCCCCCC
		);
	//
	kos_PutImage( bmPMButton + 12, 6, 2, ENTRY_CLIENT_SIZE_X - (31-1) + 3, 133-22 );
	//
	kos_DefineButton(
		ENTRY_CLIENT_SIZE_X - 17, 128-23,
		13, 13,
		BT_LOOP_PLUS,
		0xCCCCCC
		);
	//
	kos_PutImage( bmPMButton, 6, 6, ENTRY_CLIENT_SIZE_X - (17-1) + 3, 131-22 );
}


//
void DrawAppWindow()
{
	kos_WindowRedrawStatus( 1 );
	kos_ProcessInfo(&process_info, -1);

	switch (appState)
	{
		//
	case appStateTop10:
		DrawTop10Window();
		break;
		//
	case appStateEntry:
		//
		DrawEntryScreen();
		break;
		//
	case appStateGo:
	case appStateShowMap:
	case appStatePause:
		if (process_info.processInfo.window_state & 0x06)
		{
			appState = appStatePause;
		}
		drawWorldMap();
		break;
		//
	case appStateAfterDeath:
		//
		drawWorldMap();
		//
		kos_DefineButton(
			((wndSizeX - 10) / 2) - 64,
			((wndSizeY - kos_GetSkinHeight() - 5) / 2) - 16,
			128, 32,
			1,
			0x136793
			);
		//
		kos_WriteTextToWindow(
			((wndSizeX - 10) / 2) - (sizeof(thatsAllStr)* 4),
			((wndSizeY - kos_GetSkinHeight() - 5) / 2) - 4,
			0x10, 0xFFFFFF,
			thatsAllStr,
			sizeof (thatsAllStr)-1
			);
			//
		break;
		//
	case appStateHideMap:
		drawWorldMapForFlip();
		break;
	}
	//
	kos_WindowRedrawStatus( 2 );
}


//
void initWorldMap()
{
	int i, j, m, allocSize;
	CWallEnemy *we;
	CSpaceEnemy *se;

	//
	allocSize = mapSizeX * mapSizeY;
	worldMap = new Byte[allocSize];
	//
	__asm{
		mov edi, worldMap
		mov ecx, allocSize
		mov al, gmEmpty
		rep stosb
	}


	//
	levelFillEdge = ( ( currentLevel + 1 ) * spacePerEnemy ) + currentLevel;
	levelFillCount = freeSpaceCount;
	//
	if ( ! noBonus )
	{
		lifeCount++;
	}
	//
	noBonus = false;
	bonus1Set = false;
	bonus2Set = false;
	bonus1Count = 0;
	goToNextLevel = false;
	currentHero = gmHero;
	currentTrack = gmTrack;

	//
	for ( i = 0; i < mapSizeX; i++ )
	{
		//
		worldMap[i] = gmWall;
		worldMap[mapSizeX + i] = gmWall;
		//
		worldMap[((mapSizeY-2)*mapSizeX) + i] = gmWall;
		worldMap[((mapSizeY-1)*mapSizeX) + i] = gmWall;
	}
	//
	for ( i = 2; i < (mapSizeY-2); i++ )
	{
		//
		worldMap[(i*mapSizeX)] = gmWall;
		worldMap[(i*mapSizeX) + 1] = gmWall;
		worldMap[(i*mapSizeX) + mapSizeX - 2] = gmWall;
		worldMap[(i*mapSizeX) + mapSizeX - 1] = gmWall;
	}
	//
	heroPtr = worldMap + ( mapSizeX / 2 );
	heroPtr[0] = gmHero;
	heroDX = 0;
	heroDY = 0;
	//
	for ( i = 0; i < currentLevel; i++ )
	{
		//
		for (
			j = ( rtlRand() % (mapSizeX * (mapSizeY - 2)) ) + (mapSizeX * 2);
			worldMap[j] != gmWall;
			j = rtlRand() % (mapSizeX * mapSizeY)
			);
		//
		we = new CWallEnemy();
		//
		we->ePtr = worldMap + j;
		we->dx = rtlRand() & 1 ? 1 : -1;
		we->dy = rtlRand() & 1 ? mapSizeX : -mapSizeX;
		//
		mapEnemies.Add( we );
		//
		worldMap[j] = gmEnemy1;
	}
	//
	m = currentLevel + 1;
	//
	for ( i = 0; i < m; i++ )
	{
		//
		for (
			j = rtlRand() % (mapSizeX * mapSizeY);
			worldMap[j] != gmEmpty;
			j = rtlRand() % (mapSizeX * mapSizeY)
			);
		//
		se = new CSpaceEnemy();
		//
		se->ePtr = worldMap + j;
		se->dx = rtlRand() & 1 ? 1 : -1;
		se->dy = rtlRand() & 1 ? mapSizeX : -mapSizeX;
		//
		mapEnemies.Add( se );
		//
		worldMap[j] = gmEnemy2;
	}
}


//
void drawWorldMap()
{
	//
	kos_DefineAndDrawWindow(
		100, 100,
		wndSizeX-1, wndSizeY-1,
		0x74, 0,
		0, 0,
		MainWindowTitle
		);
	//
	drawWndTitleGo();
	//
	drawWorldMapForFlip();
}


//
int GetCompletePercents()
{
	int n1, n2;

	//
	n1 = freeSpaceCount - levelFillCount;
	n2 = freeSpaceCount - levelFillEdge;
	//
	return ( n1 >= n2 ) ? 100 : ( n1 * 100 ) / n2;
}


//
void drawWndTitleGo()
{
	PRINTK pr;
	static char prev_title[64] = {'\0'};

	//
	/*
	kos_DrawBar(
		1, 1,
		wndSizeX - 2, 18,
		0x2040A0
		);
		*/
	//
	//pr.fmtline = goWndTitle;
	pr.args[0] = currentLevel;
	pr.args[1] = GetCompletePercents();
	pr.args[2] = lifeCount;
	pr.args[3] = scoreCount;

	if (bonus1Count > 0)
	{
		pr.args[4] = 100*bonus1Count/BONUS1_LIFETIME;
		pr.fmtline = goWndTitleSuperHero;
	}
	else
	{
		pr.fmtline = goWndTitle;
		
	}
	sprintk(WindowTitle, &pr);
	//
	bool same = true;
	for (int i = 0; i < 64; i++)
	{
		if (WindowTitle[i] != prev_title[i])
		{
			same = false;
			break;
		}
		if ((WindowTitle[i] == '\0') || (prev_title[i] == '\0'))
		{
			break;
		}

	}

	if (!same)
	{
		kos_SetWindowCaption(WindowTitle);
		strcpy(prev_title, WindowTitle);
	}
	
		/*
	kos_WriteTextToWindow(
		4, 4,
		0x10, 0x42D2E2,
		line,
		strlen( line )
		);
		*/
	//
	/*
	if ( bonus1Count > 0 )
	{
		//
		kos_DrawBar(
			2, 22 - BONUS1_IND_HSIZE - 1,
			wndSizeX - 4, BONUS1_IND_HSIZE,
			0x2040A0
			);
		//
		kos_DrawBar(
			2, 22 - BONUS1_IND_HSIZE - 1,
			( bonus1Count * ( wndSizeX - 4 ) ) / BONUS1_LIFETIME, BONUS1_IND_HSIZE,
			0x5720B0
			);
			
	}*/
}

//
void drawWorldMapForFlip()
{
	int i, j;
	Byte *mPtr = worldMap;

	if (process_info.processInfo.window_state & 0x06)
		return;
	//
	for ( i = 0; i < mapSizeY; i++ )
	{
		//
		for ( j = 0; j < mapSizeX; j++ )
		{
			kos_PutImage(
				mapColours[*mPtr],
				blockSize,
				blockSize,
				j * blockSize,
				i * blockSize
				);
			//
			mPtr++;
		}
	}
}


//
void clearWorldMap()
{
	int i, j;

	//
	sTrackList.Clear();
	fillList.Clear();
	mapDiffList.Clear();
	//
	j = mapEnemies.GetCount();
	//
	for ( i = 0; i < j; i++ )
	{
		//
		delete mapEnemies[i];
	}
	//
	mapEnemies.Clear();
	//
	if ( worldMap != NULL )
	{
		delete worldMap;
		worldMap = NULL;
	}
}


//
void DrawTop10Window()
{
	int i;

	//
	kos_DefineAndDrawWindow(
		100, 100,
		0, 0,
		0x34, 0,
		0, 0,
		Top10WndTitle
		);
	//
	if (process_info.processInfo.window_state & 0x06)
		return;
	/*
	kos_WriteTextToWindow(
		4, 4,
		0x0, 0x42D2E2,
		Top10WndTitle,
		sizeof( Top10WndTitle ) - 1
		);
		*/
	//
	for ( i = 0; i < TOP_TBL_SIZE; i++ )
	{
		//
		kos_WriteTextToWindow(
			6-1, 2 + (i * 10),
			0x0, enterName != i ? 0xFFFFFF : 0x00FF00,
			heroTbl[i].name,
			sizeof( heroTbl[0].name )
			);
		//
		kos_DisplayNumberToWindow(
			heroTbl[i].score,
			8,
			112-1, 2 + (i * 10),
			0xFFFF55,
			nbDecimal,
			false
			);
	}
	//
	kos_WriteTextToWindow(
		6-1, 6 + (i * 10),
		0x10, 0x1060D0,
		enterName >= 0 ? top10str1 : top10str2,
		enterName >= 0 ? sizeof(top10str1) - 1 : sizeof(top10str2) - 1
		);
}


