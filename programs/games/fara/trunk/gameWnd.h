// gameWnd.h

extern CKosBitmap gameFace;
extern CKosBitmap gameBlocks;
extern CKosBitmap gameNumbers;
extern CKosBitmap gameBlocksZ[4];
extern CFishka *fishki[blocksNum];
extern Dword playerScore;

#define GM_NONE		0
#define GM_TOP10	1
#define GM_ABORT	2

#define NUM_WIDTH		12
#define NUM_HEIGHT		20

#define SCORE_LEFT		(9+2)
#define SCORE_TOP		(57+22)
#define SCORE_WIDTH		72

#define LEVEL_LEFT		(66+2)
#define LEVEL_TOP		(89+22)
#define LEVEL_WIDTH		24

//
void DrawGameWindow();
//
int GameLoop();

