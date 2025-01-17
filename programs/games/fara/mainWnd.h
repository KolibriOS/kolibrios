// mainWnd.h

#define MW_NONE			0
#define MW_EXIT_APP		1
#define MW_START_GAME	2


//
int MainWndLoop();
// полная отрисовка главного окна программы (1)
void DrawMainWindow();
//
extern CKosBitmap mainWndFace;
