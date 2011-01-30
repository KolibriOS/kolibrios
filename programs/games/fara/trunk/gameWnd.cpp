//

#include "kosSyst.h"
#include "KosFile.h"
#include "gfxdef.h"
#include "gameWnd.h"
#include "mcarray.h"
#include "lang.h"

//
#define EMPTY_PLACE				0

#define LEVEL_PROGRESS_LEFT		(132+1)
#define LEVEL_PROGRESS_TOP		(388+21)
#define LEVEL_PROGRESS_HEIGHT	4
#define LEVEL_PROGRESS_WIDTH	329
#define LEVEL_PROGRESS_COLOUR	0xDF2933

#define BONUS_SOMEBLOCK_TOP		(120+21)
#define BONUS_SOMEBLOCK_LEFT	(46+1)
#define BONUS_FREEBLOCK_TOP		(163+21)
#define BONUS_FREEBLOCK_LEFT	(9+1)
#define BONUS_DIAGBLOCK_TOP		(213+21)
#define BONUS_DIAGBLOCK_LEFT	(48+1)
#define BONUS_PROGRESS_HEIGHT	37
#define BONUS_PROGRESS_WIDTH	4

#define LEVEL_SCORE_BASE		50

//
CKosBitmap gameFace;
CKosBitmap gameBlocks;
CKosBitmap gameNumbers;
CKosBitmap gameBlocksZ[4];
//
CFishka *fishki[blocksNum];

//
char *gameWndTitle;
#if LANG == RUS
char gameWndTitle1[] = "” а ®­ ¦¤св вҐЎп :)";
char gameWndTitle2[] = "Ќг ў®в...";
char gameOverText[] = "„ «миҐ ¤®а®ЈЁ ­Ґв!";
#else
char gameWndTitle1[] = "Pharaoh waits for you :)";
char gameWndTitle2[] = "Well...";
char gameOverText[] = "No further way!";
#endif
//
Word gcx, gcy;
//
#define mapSize			8
//
//
#define blocksLeft		(134+2)
#define blocksTop		(43+22)
//
#define blockTypesNum	10
//
Byte gameMap[mapSize * mapSize];
// уровень
int gameLevel;
//
int maxGameLevel;
int startGameLevel;
// счёт
Dword playerScore;
// счётчик добавленных блоков
int insertedBlocksCount;
// количество блоков для перехода на следующий уровень
int levelBlocksLimit;
// номер выделенного блока
int selectedBlock;
// прибавление шага для индикатора прохождения уровня * 1024
int levelProgressStep;
// занчение индикатора прохождения уровня
int levelProgressCount;
// блоки для удаления
struct sRemoveBlock
{
	short int ndx;
	short int value;
	sRemoveBlock()
	{
		this->ndx = 0;
		this->value = 0;
	};
	sRemoveBlock( int pNdx, int pValue )
	{
		this->ndx = pNdx;
		this->value = pValue;
	};
};
bool operator == ( const sRemoveBlock &b1, const sRemoveBlock &b2 )
{
	return b1.ndx == b2.ndx;
};
MCArray<sRemoveBlock> toRemoveList;
// падающие блоки
MCArray<int> toFallList;
//
bool mouseButtonDisabled;
//
bool freezeBonuses = false;
//
int bonusSomeBlockProgressStep;
int bonusSomeBlockProgressCount;
//
int bonusFreeBlockProgressStep;
int bonusFreeBlockProgressCount;
//
int bonusDiagBlockProgressStep;
int bonusDiagBlockProgressCount;



#define SELECTED_BLOCK_NONE			-1

// бонус включен
int bonusSomeBlock;
// счётчик блоков для бонуса
int bonusSomeBlockCount;
// количество блоков, которое надо убрать, чтобы получить бонус
int bonusSomeBlockEdge;
#define BONUS_SOMEBLOCK_SELECTED	-2
//
bool bonusFreeBlock;
int bonusFreeBlockCount;
int bonusFreeBlockEdge;
#define BONUS_FREEBLOCK_SELECTED	-3
//
int bonusDiagBlock;
int bonusDiagBlockCount;
int bonusDiagBlockEdge;
#define BONUS_DIAGBLOCK_SELECTED	-4
//
void drawScore();
// отобразить блоки бонусов
void drawBonuses();
//
void drawLevelNum();
//
void fadeOutBlocks();
//
void fallDownBlocks();


void ClearGameMeters()
{
	// индикатор прохождение уровня
	kos_DrawBar(
		LEVEL_PROGRESS_LEFT,
		LEVEL_PROGRESS_TOP,
		LEVEL_PROGRESS_WIDTH,
		LEVEL_PROGRESS_HEIGHT,
		0
		);
	// индикаторы бонусов
	// some
	kos_DrawBar(
		33+1,
		118+21,
		BONUS_PROGRESS_WIDTH,
		BONUS_PROGRESS_HEIGHT,
		0
		);
	// free
	kos_DrawBar(
		58+1,
		166+21,
		BONUS_PROGRESS_WIDTH,
		BONUS_PROGRESS_HEIGHT,
		0
		);
	// diag
	kos_DrawBar(
		37+1,
		213+21,
		BONUS_PROGRESS_WIDTH,
		BONUS_PROGRESS_HEIGHT,
		0
		);
}


//
void drawGameMeters()
{
	//
	ClearGameMeters();
	// индикатор прохождение уровня
	kos_DrawBar(
		LEVEL_PROGRESS_LEFT,
		LEVEL_PROGRESS_TOP,
		levelProgressCount,
		LEVEL_PROGRESS_HEIGHT,
		LEVEL_PROGRESS_COLOUR
		);
	//
	if ( bonusSomeBlockProgressCount > 0 )
	// some
	kos_DrawBar(
		33+1,
		118+21+BONUS_PROGRESS_HEIGHT-bonusSomeBlockProgressCount,
		BONUS_PROGRESS_WIDTH,
		bonusSomeBlockProgressCount,
		LEVEL_PROGRESS_COLOUR
		);
	//
	if ( bonusFreeBlockProgressCount > 0 )
	// free
	kos_DrawBar(
		58+1,
		166+21+BONUS_PROGRESS_HEIGHT-bonusFreeBlockProgressCount,
		BONUS_PROGRESS_WIDTH,
		bonusFreeBlockProgressCount,
		LEVEL_PROGRESS_COLOUR
		);
	//
	if ( bonusDiagBlockProgressCount > 0 )
	// diag
	kos_DrawBar(
		37+1,
		213+21+BONUS_PROGRESS_HEIGHT-bonusDiagBlockProgressCount,
		BONUS_PROGRESS_WIDTH,
		bonusDiagBlockProgressCount,
		LEVEL_PROGRESS_COLOUR
		);
}


////////////////////////////////////////////////////////////////////////////////
void SetLevelVariables()
{
	selectedBlock = -1;
	levelBlocksLimit = ( gameLevel > 5 ) ? LEVEL_SCORE_BASE * ( gameLevel + 1 ) : LEVEL_SCORE_BASE * ( 11 - gameLevel );
	insertedBlocksCount = 0;
	//
	levelProgressCount = 0;
	levelProgressStep = ( LEVEL_PROGRESS_WIDTH * 1024 ) / levelBlocksLimit;
	//
	bonusSomeBlockEdge = levelBlocksLimit / 4;
	bonusFreeBlockEdge = levelBlocksLimit / 3;
	bonusDiagBlockEdge = levelBlocksLimit / 2;
	//
	bonusSomeBlockProgressStep = ( BONUS_PROGRESS_HEIGHT * 1024 ) / bonusSomeBlockEdge;
	bonusSomeBlockProgressCount = ( ( ( bonusSomeBlockCount > bonusSomeBlockEdge ) ? bonusSomeBlockEdge : bonusSomeBlockCount ) * bonusSomeBlockProgressStep ) / 1024;
	//
	bonusFreeBlockProgressStep = ( BONUS_PROGRESS_HEIGHT * 1024 ) / bonusFreeBlockEdge;
	bonusFreeBlockProgressCount = ( ( ( bonusFreeBlockCount > bonusFreeBlockEdge ) ? bonusFreeBlockEdge : bonusFreeBlockCount ) * bonusFreeBlockProgressStep ) / 1024;
	//
	bonusDiagBlockProgressStep = ( BONUS_PROGRESS_HEIGHT * 1024 ) / bonusDiagBlockEdge;
	bonusDiagBlockProgressCount = ( ( ( bonusDiagBlockCount > bonusDiagBlockEdge ) ? bonusDiagBlockEdge : bonusDiagBlockCount ) * bonusDiagBlockProgressStep ) / 1024;
}


//
int GetScoreByBlocks( int blocksCount )
{
	int limit, update, i, j, acc;

	//
	if ( blocksCount < 3 ) return 0;
	//
	limit = 20 * blocksNum * gameLevel;
	//
	update = gameLevel;
	acc = 1;
	//
	j = blocksCount - 3;
	//
	for ( i = 0; i < j; i++ )
	{
		//
		acc *= gameLevel + 1;
		//
		if ( ( update + acc ) > limit ) return limit;
	}
	//
	return update + acc;
}



//
int insertNewBlocks()
{
	int i, j, k, ndx, result, btn;

	//
	toFallList.Clear();
	//
	result = 0;
	//
	btn = gameLevel > 5 ? blockTypesNum : 5 + gameLevel;
	//
	ndx = ( mapSize * mapSize ) - mapSize;
	//
	for ( i = 0; i < mapSize; i++ )
	{
		for ( j = 0; j < mapSize; j++ )
		{
			//
			k = ndx + i - ( j * mapSize );
			//
			if ( gameMap[k] == EMPTY_PLACE )
			{
				//
				for ( ; j < (mapSize-1); j++ )
				{
					//
					gameMap[k] = gameMap[k-mapSize];
					toFallList.AddExclusive( k );
					k -= mapSize;
				}
				//
				gameMap[i] = ( rtlRand() % btn ) + 1;
				toFallList.AddExclusive( i );
				//
				result++;
				//
				break;
			}
		}
	}
	//
	insertedBlocksCount += result;
	//
	return result;
}


// поиск цепочек блоков для удаления, удаление блоков
// возвращает приращение счёта игрока
int findBlockLines()
{
	int x, y, ndx;
	int removeCount = 0;

	// очистим список для записи результатов
	toRemoveList.Clear();
	//
	for ( y = 0; y < mapSize; y++ )
	{
		//
		for ( x = 0; x < mapSize; x++ )
		{
			//
			ndx = x + ( y * mapSize );
			//
			if ( gameMap[ndx] == EMPTY_PLACE ) continue;
			// проверяем горизонтальную цепочку
			if ( x < (mapSize - 2) )
			{
				//
				if (
					( gameMap[ndx] == gameMap[ndx+1]  &&  gameMap[ndx] == gameMap[ndx+2] )
					|| ( BONUS_FREE_BLOCK == gameMap[ndx]  &&  gameMap[ndx+1] == gameMap[ndx+2] )
					|| ( gameMap[ndx] == gameMap[ndx+1]  &&  BONUS_FREE_BLOCK == gameMap[ndx+2] )
					|| ( gameMap[ndx] == gameMap[ndx+2]  &&  BONUS_FREE_BLOCK == gameMap[ndx+1] )
					)
				{
					// нашли цепочку, запомним
					toRemoveList.AddExclusive( sRemoveBlock( ndx, gameMap[ndx] ) );
					toRemoveList.AddExclusive( sRemoveBlock( ndx+1, gameMap[ndx+1] ) );
					toRemoveList.AddExclusive( sRemoveBlock( ndx+2, gameMap[ndx+2] ) );
				}
			}
			// проверяем вертикальную цепочку
			if ( y < (mapSize - 2) )
			{
				//
				if (
					( gameMap[ndx] == gameMap[ndx+mapSize]  &&  gameMap[ndx] == gameMap[ndx+mapSize+mapSize] )
					|| ( BONUS_FREE_BLOCK == gameMap[ndx]  &&  gameMap[ndx+mapSize] == gameMap[ndx+mapSize+mapSize] )
					|| ( gameMap[ndx] == gameMap[ndx+mapSize]  &&  BONUS_FREE_BLOCK == gameMap[ndx+mapSize+mapSize] )
					|| ( gameMap[ndx] == gameMap[ndx+mapSize+mapSize]  &&  BONUS_FREE_BLOCK == gameMap[ndx+mapSize] )
					)
				{
					// нашли цепочку, запомним
					toRemoveList.AddExclusive( sRemoveBlock( ndx, gameMap[ndx] ) );
					toRemoveList.AddExclusive( sRemoveBlock( ndx+mapSize, gameMap[ndx+mapSize] ) );
					toRemoveList.AddExclusive( sRemoveBlock( ndx+mapSize+mapSize, gameMap[ndx+mapSize+mapSize] ) );
				}
			}
		}
	}
	//
	return toRemoveList.GetCount();
}


// проверка на невозможность собрать какую-нибудь линию
bool checkGameOut()
{
	int x, y, ndx;

	//
	ndx = 0;
	//
	for ( y = 0; y < mapSize; y++ )
	{
		//
		for ( x = 0; x < mapSize; x++ )
		{
			// проверяем горизонтальные цепочки из двух символов
			if ( x < (mapSize - 1) )
			{
				//
				if ( gameMap[ndx] == gameMap[ndx+1] )
				{
					// нашли цепочку из двух блоков
					// проверка бонусов
					if ( bonusSomeBlock == gameMap[ndx] ) return false;
					if ( bonusFreeBlock ) return false;
					// проверка обычных перестановок
					if ( y > 0 )
					{
						//
						if ( x > 0 )
						{
							//
							if ( gameMap[ndx-mapSize-1] == gameMap[ndx] ) return false;
						}
						//
						if ( x < (mapSize - 2) )
						{
							//
							if ( gameMap[ndx-mapSize+2] == gameMap[ndx] ) return false;
						}
					}
					//
					if ( x > 1 )
					{
						//
						if ( gameMap[ndx-2] == gameMap[ndx] ) return false;
					}
					//
					if ( x < (mapSize - 3) )
					{
						//
						if ( gameMap[ndx+3] == gameMap[ndx] ) return false;
					}
					//
					if ( y < (mapSize - 1) )
					{
						//
						if ( x > 0 )
						{
							//
							if ( gameMap[ndx+mapSize-1] == gameMap[ndx] ) return false;
						}
						//
						if ( x < (mapSize - 2) )
						{
							//
							if ( gameMap[ndx+mapSize+2] == gameMap[ndx] ) return false;
						}
					}
					// проверка диагональных перестановок
					if ( bonusDiagBlock > 0 )
					{
						//
						if ( y > 0 )
						{
							//
							if ( x > 1 )
							{
								//
								if ( gameMap[ndx-mapSize-2] == gameMap[ndx] ) return false;
							}
							//
							if ( gameMap[ndx-mapSize] == gameMap[ndx] ) return false;
							//
							if ( x < (mapSize - 2) )
							{
								//
								if ( gameMap[ndx-mapSize+1] == gameMap[ndx] ) return false;
							}
							//
							if ( x < (mapSize - 3) )
							{
								//
								if ( gameMap[ndx-mapSize+3] == gameMap[ndx] ) return false;
							}
						}
						//
						if ( y < (mapSize - 1) )
						{
							//
							if ( x > 1 )
							{
								//
								if ( gameMap[ndx+mapSize-2] == gameMap[ndx] ) return false;
							}
							//
							if ( gameMap[ndx+mapSize] == gameMap[ndx] ) return false;
							//
							if ( x < (mapSize - 2) )
							{
								//
								if ( gameMap[ndx+mapSize+1] == gameMap[ndx] ) return false;
							}
							//
							if ( x < (mapSize - 3) )
							{
								//
								if ( gameMap[ndx+mapSize+3] == gameMap[ndx] ) return false;
							}
						}
					} // закончена проверка диагональных перестановок
				}
			}
			// проверяем горизонтальные цепочки из двух блоков с промежутком
			if ( x < (mapSize - 2) )
			{
				//
				if ( gameMap[ndx] == gameMap[ndx+2] )
				{
					// нашли два блока с промежутком
					// проверка бонусов
					if ( bonusSomeBlock == gameMap[ndx] ) return false;
					if ( bonusFreeBlock ) return false;
					//
					if ( y > 0 )
					{
						//
						if ( gameMap[ndx-mapSize+1] == gameMap[ndx] ) return false;
					}
					//
					if ( y < (mapSize-1) )
					{
						//
						if ( gameMap[ndx+mapSize+1] == gameMap[ndx] ) return false;
					}
					// проверка диагональных перестановок
					if ( bonusDiagBlock > 0 )
					{
						//
						if ( y > 0 )
						{
							//
							if ( gameMap[ndx-mapSize] == gameMap[ndx] ) return false;
							//
							if ( gameMap[ndx-mapSize+2] == gameMap[ndx] ) return false;
						}
						//
						if ( y < (mapSize-1) )
						{
							//
							if ( gameMap[ndx+mapSize] == gameMap[ndx] ) return false;
							//
							if ( gameMap[ndx+mapSize+2] == gameMap[ndx] ) return false;
						}
					}
				}
			}
			// проверяем вертикальные цепочки из двух символов
			if ( y < (mapSize - 1) )
			{
				//
				if ( gameMap[ndx] == gameMap[ndx+mapSize] )
				{
					// нашли цепочку из двух блоков
					// проверка бонусов
					if ( bonusSomeBlock == gameMap[ndx] ) return false;
					if ( bonusFreeBlock ) return false;
					//
					if ( x > 0 )
					{
						//
						if ( y > 0 )
						{
							//
							if ( gameMap[ndx-1-mapSize] == gameMap[ndx] ) return false;
						}
						//
						if ( y < (mapSize - 2) )
						{
							//
							if ( gameMap[ndx-1+(2*mapSize)] == gameMap[ndx] ) return false;
						}
					}
					//
					if ( y > 1 )
					{
						//
						if ( gameMap[ndx-(2*mapSize)] == gameMap[ndx] ) return false;
					}
					//
					if ( y < (mapSize - 3) )
					{
						//
						if ( gameMap[ndx+(3*mapSize)] == gameMap[ndx] ) return false;
					}
					//
					if ( x < (mapSize - 1) )
					{
						//
						if ( y > 0 )
						{
							//
							if ( gameMap[ndx+1-mapSize] == gameMap[ndx] ) return false;
						}
						//
						if ( y < (mapSize - 2) )
						{
							//
							if ( gameMap[ndx+1+(2*mapSize)] == gameMap[ndx] ) return false;
						}
					}
					// проверка диагональных перестановок
					if ( bonusDiagBlock > 0 )
					{
						//
						if ( x > 0 )
						{
							//
							if ( y > 1 )
							{
								//
								if ( gameMap[ndx-1-(2*mapSize)] == gameMap[ndx] ) return false;
							}
							//
							if ( gameMap[ndx-1] == gameMap[ndx] ) return false;
							//
							if ( y < (mapSize - 2) )
							{
								//
								if ( gameMap[ndx-1+mapSize] == gameMap[ndx] ) return false;
							}
							//
							if ( y < (mapSize - 3) )
							{
								//
								if ( gameMap[ndx-1+(3*mapSize)] == gameMap[ndx] ) return false;
							}
						}
						//
						if ( x < (mapSize - 1) )
						{
							//
							if ( y > 1 )
							{
								//
								if ( gameMap[ndx+1-(2*mapSize)] == gameMap[ndx] ) return false;
							}
							//
							if ( gameMap[ndx+1] == gameMap[ndx] ) return false;
							//
							if ( y < (mapSize - 2) )
							{
								//
								if ( gameMap[ndx+1+mapSize] == gameMap[ndx] ) return false;
							}
							//
							if ( y < (mapSize - 3) )
							{
								//
								if ( gameMap[ndx+1+(3*mapSize)] == gameMap[ndx] ) return false;
							}
						}
					} // закончена проверка диагональных перестановок
				}
			}
			// проверяем вертикальные цепочки из двух блоков с промежутком
			if ( y < (mapSize - 2) )
			{
				//
				if ( gameMap[ndx] == gameMap[ndx+(2*mapSize)] )
				{
					// нашли два блока с промежутком
					// проверка бонусов
					if ( bonusSomeBlock == gameMap[ndx] ) return false;
					if ( bonusFreeBlock ) return false;
					//
					if ( x > 0 )
					{
						//
						if ( gameMap[ndx-1+mapSize] == gameMap[ndx] ) return false;
					}
					//
					if ( x < (mapSize-1) )
					{
						//
						if ( gameMap[ndx+1+mapSize] == gameMap[ndx] ) return false;
					}
					// проверка диагональных перестановок
					if ( bonusDiagBlock > 0 )
					{
						//
						if ( x > 0 )
						{
							//
							if ( gameMap[ndx-1] == gameMap[ndx] ) return false;
							//
							if ( gameMap[ndx-1+(2*mapSize)] == gameMap[ndx] ) return false;
						}
						//
						if ( x < (mapSize-1) )
						{
							//
							if ( gameMap[ndx+1] == gameMap[ndx] ) return false;
							//
							if ( gameMap[ndx+1+(2*mapSize)] == gameMap[ndx] ) return false;
						}
					}
				}
			}
			//
			ndx++;
		}
	}
	//
	gameWndTitle = gameWndTitle2;
	//
	return true;
}


//
bool exterminateLines()
{
	int deletedBlocks, btn, i, j;
	bool needRedrawBonus = false;

	//
	btn = gameLevel > 5 ? blockTypesNum : 5 + gameLevel;
	//
	playerScore += GetScoreByBlocks( deletedBlocks = findBlockLines() );
	//
	if ( ! freezeBonuses )
	{
		//
		if ( gameLevel >= 2 && bonusSomeBlock <= 0 ) bonusSomeBlockCount += deletedBlocks;
		if ( gameLevel >= 3 && !bonusFreeBlock ) bonusFreeBlockCount += deletedBlocks;
		if ( gameLevel >= 4 && bonusDiagBlock <= 0 ) bonusDiagBlockCount += deletedBlocks;
		// первый бонус
		if (
			( bonusSomeBlockCount >= bonusSomeBlockEdge || ( gameLevel >= 2 && deletedBlocks >= 4 ) )
			&& bonusSomeBlock <= 0
			)
		{
			bonusSomeBlock = ( rtlRand() % btn ) + 1;
			needRedrawBonus = true;
			if ( bonusSomeBlockCount >= bonusSomeBlockEdge ) bonusSomeBlockCount = 0;
		}
		if ( bonusSomeBlockCount >= bonusSomeBlockEdge ) bonusSomeBlockCount = 0;
		// второй бонус
		if ( bonusFreeBlockCount >= bonusFreeBlockEdge || ( gameLevel >=3 && deletedBlocks >= 5 ) )
		{
			bonusFreeBlock = true;
			needRedrawBonus = true;
			if ( bonusFreeBlockCount >= bonusFreeBlockEdge ) bonusFreeBlockCount = 0;
		}
		if ( bonusFreeBlockCount >= bonusFreeBlockEdge ) bonusFreeBlockCount = 0;
		// третий бонус
		if ( bonusDiagBlockCount >= bonusDiagBlockEdge || ( gameLevel >= 4 && deletedBlocks >= 6 ) )
		{
			bonusDiagBlock = 3;
			needRedrawBonus = true;
			if ( bonusDiagBlockCount >= bonusDiagBlockEdge ) bonusDiagBlockCount = 0;
		}
		if ( bonusDiagBlockCount >= bonusDiagBlockEdge ) bonusDiagBlockCount = 0;
		//
		bonusSomeBlockProgressCount = ( ( ( bonusSomeBlockCount > bonusSomeBlockEdge ) ? bonusSomeBlockEdge : bonusSomeBlockCount ) * bonusSomeBlockProgressStep ) / 1024;
		//
		bonusFreeBlockProgressCount = ( ( ( bonusFreeBlockCount > bonusFreeBlockEdge ) ? bonusFreeBlockEdge : bonusFreeBlockCount ) * bonusFreeBlockProgressStep ) / 1024;
		//
		bonusDiagBlockProgressCount = ( ( ( bonusDiagBlockCount > bonusDiagBlockEdge ) ? bonusDiagBlockEdge : bonusDiagBlockCount ) * bonusDiagBlockProgressStep ) / 1024;
		//
		if ( needRedrawBonus ) drawBonuses();
	}
	//
	j = toRemoveList.GetCount();
	//
	for ( i = 0; i < j; i++ )
	{
		gameMap[toRemoveList[i].ndx] = EMPTY_PLACE;
	}
	//
	return toRemoveList.GetCount() > 0;
}


// заполнение игрового поля случайной комбинацией блоков
void initGameMap()
{
	int i, localScore, localInserted, btn;

	//
	btn = gameLevel > 5 ? blockTypesNum : 5 + gameLevel;
	//
	for ( i = 0; i < (mapSize * mapSize); i++ )
	{
		gameMap[i] = ( rtlRand() % btn ) + 1;
	}
	//
	localScore = playerScore;
	localInserted = insertedBlocksCount;
	//
	freezeBonuses = true;
	//
	while ( exterminateLines() )
	{
		while ( insertNewBlocks() > 0 );
	}
	//
	freezeBonuses = false;
	//
	playerScore = localScore;
	insertedBlocksCount = localInserted;
}


// отобразить блоки бонусов
void drawBonuses()
{
	//
	kos_PutImage(
		selectedBlock != BONUS_SOMEBLOCK_SELECTED ?
			fishki[bonusSomeBlock]->GetBits() :
			fishki[bonusSomeBlock]->GetHighlightedBits(),
		blockSize, blockSize,
		BONUS_SOMEBLOCK_LEFT, BONUS_SOMEBLOCK_TOP
		);
	//
	kos_PutImage(
		bonusFreeBlock ?
		(
			selectedBlock != BONUS_FREEBLOCK_SELECTED ?
			fishki[BONUS_FREE_BLOCK]->GetBits() :
			fishki[BONUS_FREE_BLOCK]->GetHighlightedBits()
		) :
		fishki[0]->GetBits(),
		blockSize, blockSize,
		BONUS_FREEBLOCK_LEFT, BONUS_FREEBLOCK_TOP
		);
	//
	kos_PutImage(
		bonusDiagBlock > 0 ?
			fishki[bonusDiagBlock+BONUS_DIAG_BLOCK-1]->GetBits() :
			fishki[0]->GetBits(),
		blockSize, blockSize,
		BONUS_DIAGBLOCK_LEFT, BONUS_DIAGBLOCK_TOP
		);
}


// отобразить игровое поле
void drawGameMap()
{
	int i, j, ndx;

	//
	for ( i = 0; i < mapSize; i++ )
	{
		//
		for ( j = 0; j < mapSize; j++ )
		{
			//
			ndx = (i*mapSize) + j;
			//
			kos_PutImage(
				ndx != selectedBlock ?
					fishki[gameMap[ndx]]->GetBits() :
					fishki[gameMap[ndx]]->GetHighlightedBits(),
				blockSize, blockSize,
				(j * blockSize) + blocksLeft,
				(i * blockSize) + blocksTop
				);
		}
	}
}


// координаты курсора "мыши"
int mX, mY;

// проверка на нажатие левой кнопки "мыши"
bool mouseLButtonDown()
{
	static bool isDown = false;
	Dword buttons;

	//
	kos_GetMouseState( buttons, mX, mY );
	//
	if ( mouseButtonDisabled ) return false;
	//
	if ( ( buttons & 1 ) )
	{
		if ( isDown )
			return false;
		else
		{
			isDown = true;
			return true;
		}
	}
	else
	{
		if ( isDown )
		{
			isDown = false;
		}
		return false;
	}
}

//
void flipTwoBlocks( int ndx1, int ndx2 )
{
	Word blX, blY, selX, selY;
	Byte fishCode;

	//
	blX = ( ndx1 % mapSize ) * blockSize + blocksLeft;
	blY = ( ndx1 / mapSize ) * blockSize + blocksTop;
	selX = ( ndx2 % mapSize ) * blockSize + blocksLeft;
	selY = ( ndx2 / mapSize ) * blockSize + blocksTop;
	// переставим блоки местами
	fishCode = gameMap[ndx1];
	gameMap[ndx1] = gameMap[ndx2];
	gameMap[ndx2] = fishCode;
	// изображение блока
	kos_PutImage(
		fishki[gameMap[ndx1]]->GetBits(),
		blockSize, blockSize,
		blX,
		blY
		);
	// изображение блока
	kos_PutImage(
		fishki[gameMap[ndx2]]->GetBits(),
		blockSize, blockSize,
		selX,
		selY
		);
}


// игровой процесс
int GameLoop()
{
	int result, ndx, blX, blY, selX, selY, ddX, ddY, nSel;
	Byte keyCode, mCode;
	bool needDecBonus;
	Dword buttonID;

	//
	gameWndTitle = gameWndTitle1;
	gameFace.GetSize( gcx, gcy );
	gameLevel = startGameLevel;
	playerScore = 0;
	bonusSomeBlock = 0;
	bonusFreeBlock = false;
	bonusDiagBlock = 0;
	bonusSomeBlockCount = 0;
	bonusFreeBlockCount = 0;
	bonusDiagBlockCount = 0;
	bonusSomeBlockProgressCount = 0;
	bonusFreeBlockProgressCount = 0;
	bonusDiagBlockProgressCount = 0;
	SetLevelVariables();
	mouseButtonDisabled = false;
	initGameMap();
	//
	kos_ChangeWindow( -1, -1, gcx + 1, gcy + 21 );
	//
	for ( result = GM_NONE; result == GM_NONE; )
	{
		switch( kos_WaitForEvent() )
		{
		// надо полностью перерисовать окно
		case 1:
			DrawGameWindow();
			break;

		// клавиатура
		case 2:
			if ( kos_GetKey( keyCode ) )
			{
				//
				switch ( keyCode )
				{
				case 0x1B:
					result = GM_ABORT;
					break;

				default:
					break;
				}
			}
			break;

		// кнопки
		case 3:
			if ( kos_GetButtonID( buttonID ) )
			{
				switch ( buttonID )
				{
				case 0xA:
					result = GM_ABORT;
					break;

				default:
					break;
				}
			}

		// событие от мыши
		case 6:
			// нажатие левой кнопки?
			if ( mouseLButtonDown() )
			{
				// считаем координаты относително игрового поля
				blX = mX - blocksLeft;
				blY = mY - blocksTop;
				// попало в игровое поле?
				if ( blX >= 0 && blX < (mapSize * blockSize)
					&& blY >= 0 && blY < (mapSize * blockSize) )
				{
					// получаем координаты в блоках
					blX /= blockSize;
					blY /= blockSize;
					// получаем номер блока на карте
					ndx = blX + ( blY * mapSize );
					// ещё одна проверка, чтобы не вылезти за пределы карты
					if ( ndx >= 0 && ndx < (mapSize * mapSize) )
					{
						// начинаем перерисовку
						kos_WindowRedrawStatus( WRS_BEGIN );
						// если не было выбранного блока
						if ( selectedBlock == SELECTED_BLOCK_NONE )
						{
							// запомним выделенный блок
							selectedBlock = ndx;
							// отметим блок на игровом поле
							kos_PutImage(
								fishki[gameMap[selectedBlock]]->GetHighlightedBits(),
								blockSize, blockSize,
								( selectedBlock % mapSize ) * blockSize + blocksLeft,
								( selectedBlock / mapSize ) * blockSize + blocksTop
								);
						}
						else // помеченный блок уже есть
						{
							if ( selectedBlock >= 0 )
							{
								// координаты помеченного блока
								selX = selectedBlock % mapSize;
								selY = selectedBlock / mapSize;
								// был выбран другой блок?
								if ( ndx != selectedBlock )
								{
									// считаем разность координат двух блоков
									ddX = selX - blX;
									ddY = selY - blY;
									needDecBonus = ( bonusDiagBlock > 0 && abs(ddX) == 1 && abs(ddY) == 1 );
									// если это соседний блок
									if (
										( abs(ddX) == 1 && ddY == 0 )
										|| ( abs(ddY) == 1 && ddX == 0 )
										|| needDecBonus
										)
									{
										// переставим блоки местами
										flipTwoBlocks( ndx, selectedBlock );
										//
										kos_Pause( 16 );
										//
										if ( findBlockLines() > 0 )
										{
											//
											if ( needDecBonus )
											{
												//
												--bonusDiagBlock;
												//
												drawBonuses();
											}
											// снимаем пометку с блока
											selectedBlock = SELECTED_BLOCK_NONE;
											//
											while ( exterminateLines() )
											{
												//
												fadeOutBlocks();
												//
												//drawGameMap();
												//drawScore();
												//
												//kos_Pause( 25 );
												//
												while ( insertNewBlocks() > 0 )
												{
													//
													fallDownBlocks();
													//
													//drawGameMap();
													//kos_Pause( 30 );
												}
											}
											//
											drawScore();
											//
											levelProgressCount = ( ( ( insertedBlocksCount > levelBlocksLimit ) ? levelBlocksLimit : insertedBlocksCount ) * levelProgressStep ) / 1024;
											//
											drawGameMeters();
											//
											if ( insertedBlocksCount > levelBlocksLimit )
											{
												kos_Pause( 50 );
												gameLevel++;
												SetLevelVariables();
												//
												//initGameMap();
												//
												//DrawGameWindow();
												//
												drawGameMeters();
												drawLevelNum();
											}
											else
												//
												if ( mouseButtonDisabled = checkGameOut() )
												{
													//
													DrawGameWindow();
												}
										}
										else
										{
											// не получается линии, блоки ставим на место
											flipTwoBlocks( ndx, selectedBlock );
											// снимаем пометку с блока
											selectedBlock = SELECTED_BLOCK_NONE;
										}
									}
									else // выбран несоседний блок
									{
										// перестим маркер
										kos_PutImage(
											fishki[gameMap[selectedBlock]]->GetBits(),
											blockSize, blockSize,
											selX * blockSize + blocksLeft,
											selY * blockSize + blocksTop
											);
										// пометим выбранный блок
										selectedBlock = ndx;
										// на игровом поле
										kos_PutImage(
											fishki[gameMap[selectedBlock]]->GetHighlightedBits(),
											blockSize, blockSize,
											blX * blockSize + blocksLeft,
											blY * blockSize + blocksTop
											);
									}
								}
								else // выбран тот же блок
								{
									// снимаем пометку
									kos_PutImage(
										fishki[gameMap[selectedBlock]]->GetBits(),
										blockSize, blockSize,
										selX * blockSize + blocksLeft,
										selY * blockSize + blocksTop
										);
									//
									selectedBlock = SELECTED_BLOCK_NONE;
								}
							}
							else
							// ткнули в блок при выбранном бонусе
							{
								//
								mCode = gameMap[ndx];
								//
								switch ( selectedBlock )
								{
								case BONUS_SOMEBLOCK_SELECTED:
									gameMap[ndx] = bonusSomeBlock;
									break;

								case BONUS_FREEBLOCK_SELECTED:
									gameMap[ndx] = BONUS_FREE_BLOCK;
									break;

								default:
									break;
								}
								//
								if ( findBlockLines() > 0 )
								{
									// убираем использованный бонус
									switch ( selectedBlock )
									{
									case BONUS_SOMEBLOCK_SELECTED:
										bonusSomeBlock = 0;
										break;

									case BONUS_FREEBLOCK_SELECTED:
										bonusFreeBlock = false;
										break;

									default:
										break;
									}
									// на экране тоже
									drawBonuses();
									drawGameMap();
									kos_Pause( 16 );
									// убираем блоки
									// снимаем пометку с блока
									selectedBlock = SELECTED_BLOCK_NONE;
									//
									while ( exterminateLines() )
									{
										//
										fadeOutBlocks();
										//
										//drawGameMap();
										//drawScore();
										//
										//kos_Pause( 25 );
										//
										while ( insertNewBlocks() > 0 )
										{
											//
											fallDownBlocks();
											//
											//drawGameMap();
											//kos_Pause( 30 );
										}
									}
									//
									drawScore();
									//
									levelProgressCount = ( ( ( insertedBlocksCount > levelBlocksLimit ) ? levelBlocksLimit : insertedBlocksCount ) * levelProgressStep ) / 1024;
									//
									drawGameMeters();
									//
									if ( insertedBlocksCount > levelBlocksLimit )
									{
										kos_Pause( 50 );
										gameLevel++;
										SetLevelVariables();
										//
										//initGameMap();
										//
										//DrawGameWindow();
										//
										drawGameMeters();
										drawLevelNum();
									}
									else
										//
										mouseButtonDisabled = checkGameOut();
								}
								else
								// бонус здесь неприменим
								{
									// вернём взад
									gameMap[ndx] = mCode;
									// пометим блок
									selectedBlock = ndx;
									// на игровом поле
									kos_PutImage(
										fishki[gameMap[selectedBlock]]->GetHighlightedBits(),
										blockSize, blockSize,
										blX * blockSize + blocksLeft,
										blY * blockSize + blocksTop
										);
									// уберём пометку с бонуса
									drawBonuses();
								}
							}
						}
						// определим кнопку
						kos_DefineButton(
							444+1, 2+21,
							54, 20,
							0x6000000A,
							0
							);
						// завершаем перерисовку
						kos_WindowRedrawStatus( WRS_END );
					}
				}
				else
				// проверим попадание в бонусы
				{
					nSel = 0;
					//
					if ( mX >= BONUS_SOMEBLOCK_LEFT && (mX - BONUS_SOMEBLOCK_LEFT) <= blockSize
						&& mY >= BONUS_SOMEBLOCK_TOP && (mY - BONUS_SOMEBLOCK_TOP ) <= blockSize )
					{
						//
						nSel = BONUS_SOMEBLOCK_SELECTED;
					}
					//
					if ( mX >= BONUS_FREEBLOCK_LEFT && (mX - BONUS_FREEBLOCK_LEFT) <= blockSize
						&& mY >= BONUS_FREEBLOCK_TOP && (mY - BONUS_FREEBLOCK_TOP ) <= blockSize )
					{
						//
						nSel = BONUS_FREEBLOCK_SELECTED;
					}
					//
					if ( mX >= BONUS_DIAGBLOCK_LEFT && (mX - BONUS_DIAGBLOCK_LEFT) <= blockSize
						&& mY >= BONUS_DIAGBLOCK_TOP && (mY - BONUS_DIAGBLOCK_TOP ) <= blockSize )
					{
						//
						nSel = BONUS_DIAGBLOCK_SELECTED;
					}
					//
					if ( nSel != 0 )
					{
						//
						if ( selectedBlock > 0 )
						{
							// снимаем пометку
							kos_PutImage(
								fishki[gameMap[selectedBlock]]->GetBits(),
								blockSize, blockSize,
								(selectedBlock % mapSize) * blockSize + blocksLeft,
								(selectedBlock / mapSize) * blockSize + blocksTop
								);
						}
						//
						if ( selectedBlock != nSel )
							selectedBlock = nSel;
						else
							selectedBlock = SELECTED_BLOCK_NONE;
						//
						drawBonuses();
					}
				}

			}
			//
			break;

		default:
			break;
		}
	}
	// отменим кнопку
	kos_DefineButton(
		0, 0,
		0, 0,
		0x8000000A,
		0
		);
	//
	if ( gameLevel > maxGameLevel ) maxGameLevel = gameLevel;
	//
	return result;
}

//
void drawLevelNum()
{
	Word startX;

	//
	if ( gameLevel > 9 )
	{
		//
		startX = LEVEL_LEFT;
		//
		kos_PutImage(
			gameNumbers.GetBits() + ( ( gameLevel / 10 ) * NUM_WIDTH * NUM_HEIGHT ),
			NUM_WIDTH, NUM_HEIGHT,
			startX, LEVEL_TOP
			);
		//
		startX += NUM_WIDTH;
	}
	else
	{
		//
		startX = LEVEL_LEFT + ( (LEVEL_WIDTH - NUM_WIDTH) / 2 );
	}
	//
	kos_PutImage(
		gameNumbers.GetBits() + ( ( gameLevel % 10 ) * NUM_WIDTH * NUM_HEIGHT ),
		NUM_WIDTH, NUM_HEIGHT,
		startX, LEVEL_TOP
		);
}

//
void drawScore()
{
	Word startX;
	int i, j;
	char strNum[8];

	// число в текстовом виде
	sprintf( strNum, "%U", playerScore );
	//
	j = strlen( strNum );
	//
	startX = SCORE_LEFT + ( ( SCORE_WIDTH - ( NUM_WIDTH * j ) ) / 2 );
	//
	for ( i = 0; i < j; i++ )
	{
		//
		kos_PutImage(
			gameNumbers.GetBits() + ( ( strNum[i] - '0' ) * NUM_WIDTH * NUM_HEIGHT ),
			NUM_WIDTH, NUM_HEIGHT,
			startX, SCORE_TOP
			);
		//
		startX += NUM_WIDTH;
	}
}


//
void DrawGameWindow()
{
	//
	kos_WindowRedrawStatus( WRS_BEGIN );
	// окно
	kos_DefineAndDrawWindow(
		WNDLEFT, WNDTOP,
		gcx + 1, gcy + 21,
		0, 0x0,
		0, WNDHEADCOLOUR,
		WNDHEADCOLOUR
		);
	// заголовок окна
	kos_WriteTextToWindow(
		4, 7,
		0x10, WNDTITLECOLOUR,
		gameWndTitle, strlen(gameWndTitle)
		);
	//
	gameFace.Draw( 1, 21 );
	drawGameMap();
	drawGameMeters();
	drawBonuses();
	// номер уровня
	drawLevelNum();
	drawScore();
	//
	if ( mouseButtonDisabled )
	{
		//
		kos_DrawBar(
			( gcx + 1 - 160 ) / 2, ( gcy + 21 - 32 ) / 2,
			160, 32,
			0x2383B3
			);
		//
		kos_WriteTextToWindow(
			( gcx + 1 - sizeof( gameOverText ) * 6 ) / 2, ( gcy + 21 - 9 ) / 2,
			0x0, 0xF7F7F7,
			gameOverText,
			sizeof( gameOverText )
			);
	}
	// определим кнопку
	kos_DefineButton(
		444+1, 2+21,
		54, 20,
		0x6000000A,
		0
		);
	//
	kos_WindowRedrawStatus( WRS_END );
}


//
void fadeOutBlocks()
{
	int i, j, k, ndx, x, y;

	//
	j = toRemoveList.GetCount();
	//
	for ( k = 0; k < 4; k++ )
	{
		//
		__asm{
				push	eax
				push	ebx
				mov		eax, 18
				mov		ebx, 14
				int		0x40
				pop		eax
				pop		ebx
		}
		//
		for ( i = 0; i < j; i++ )
		{
			//
			ndx = toRemoveList[i].ndx;
			y = ndx / mapSize;
			x = ndx % mapSize;
			//
			kos_PutImage(
				gameBlocksZ[k].GetBits() + ( (toRemoveList[i].value - 1) * blockSize * blockSize ),
				blockSize, blockSize,
				(x * blockSize) + blocksLeft,
				(y * blockSize) + blocksTop
				);
		}
		//
		kos_Pause( 3 );
	}
	//clear
	for ( i = 0; i < j; i++ )
	{
		//
		ndx = toRemoveList[i].ndx;
		y = ndx / mapSize;
		x = ndx % mapSize;
		//
		kos_DrawBar(
			(x * blockSize) + blocksLeft,
			(y * blockSize) + blocksTop,
			blockSize, blockSize, 0
			);
	}
	//
	kos_Pause( 16 );
}


//
void fallDownBlocks()
{
	int i, j, k, ndx, x, y, height, offset;

	//
	j = toFallList.GetCount();
	//
	for ( k = 1; k <= blockSize; k += 2 )
	{
		//
		__asm{
				push	eax
				push	ebx
				mov		eax, 18
				mov		ebx, 14
				int		0x40
				pop		eax
				pop		ebx
		}
		//
		for ( i = 0; i < j; i++ )
		{
			//
			ndx = toFallList[i];
			//
			y = ( ( ( ndx / mapSize ) - 1 ) * blockSize ) + k;
			if ( y < 0 )
			{
				y = 0;
				height = k;
				offset = blockSize - k;
			}
			else
			{
				offset = 0;
				height = blockSize;
			}
			//
			x = ( ndx % mapSize ) * blockSize;
			//
			kos_PutImage(
				fishki[gameMap[ndx]]->GetBits() + ( offset * blockSize ),
				blockSize, height,
				x + blocksLeft,
				y + blocksTop
				);
		}
		//
		kos_Pause( 1 );
	}
}
