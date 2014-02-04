
#define LAY_HIDDEN 0
#define LAY_NUM 1
#define LAY_FOUND 2
#define LAY_OPENED 3

extern char	PATH[256];
extern char	PARAM[256];

int window_width, window_height;
int x_start, y_start;
int size;

char board[8][8][4];

int foxN;
int foxLeft;
int moves;

int result;
