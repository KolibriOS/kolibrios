
#include "all.h"

void init_board()
{
int i, x, y, z;
int x1, y1;

for (x = 0; x < 8; x++)
	for (y = 0; y < 8; y++)
		for (z = 0; z < 4; z++)
			board[x][y][z] = 0;

for (i = 0; i < 4; i++)
	board[rand()%8][rand()%8][LAY_HIDDEN] = 1;

foxN = 0;
for (x = 0; x < 8; x++)
	for (y = 0; y < 8; y++)
		if (board[x][y][LAY_HIDDEN] == 1)
			foxN++;

if (foxN < 4) // if some foxes are in one hole
	{
	init_board();
	return;
	}

// count of visible foxes	
for (x = 0; x < 8; x++)
	for (y = 0; y < 8; y++)
		{

		for (x1 = 0; x1 < 8; x1++) // horizontal
			if (board[x1][y][LAY_HIDDEN] == 1)
				board[x][y][LAY_NUM]++;

				
		for (y1 = 0; y1 < 8; y1++) // vertical
			if (board[x][y1][LAY_HIDDEN] == 1)
				board[x][y][LAY_NUM]++;


		if (x-y>0) // north-west to south-east
			x1 = x-y;
		else
			x1 = 0;

		if (y-x>0)
			y1 = y-x;
		else
			y1 = 0;
			
		do
			{
			if (board[x1][y1][LAY_HIDDEN] == 1)
				board[x][y][LAY_NUM]++;
			x1++;
			y1++;
			} while ((x1<8)&&(y1<8));

			
		if (x+y-7>0) // south-west to north-east
			x1 = x+y-7;
		else
			x1 = 0;

		if (x+y<8)
			y1 = x+y;
		else
			y1 = 7;
			
		do
			{
			if (board[x1][y1][LAY_HIDDEN] == 1)
				board[x][y][LAY_NUM]++;
			x1++;
			y1--;
			} while ((x1<8)&&(y1>-1));
			
			
		if (board[x][y][LAY_HIDDEN] == 1)
			board[x][y][LAY_NUM] -= 3;
			
		}

foxLeft = foxN;
moves = 0;
result = 0;
		
}

void init_grid_sizes()
{
size = 20;
x_start = 5;
y_start = 30;
window_width = 2*x_start + 8*size + 9;
window_height = y_start + x_start + 8*size + kol_skin_height() + 9 + 14;
}

void wnd_draw()
{

int i;
int x, y;
int x1, y1;
char tmp[64];
char tmp2[64];

kol_paint_start();

kol_wnd_define(100, 100, window_width, window_height, 0x34ddddff, 0x34ddddff, "FoxHunt v0.2 by Albom");

kol_btn_define(x_start, x_start, 40, 16, 2, 0xccccee);
kol_paint_string(x_start+20-12, x_start+5, "New", 0x902222ff);

strcpy(tmp, "F:");
itoa(foxLeft, tmp2);
strcat(tmp, tmp2);
strcat(tmp, "/4");
kol_paint_string(x_start+55, x_start+5, tmp, 0x902222ff);

strcpy(tmp, "M:");
itoa(moves, tmp2);
strcat(tmp, tmp2);
kol_paint_string(x_start+110, x_start+5, tmp, 0x902222ff);

for (i = 0; i <= 8; i++)
	{
	kol_paint_line(x_start+i*size, y_start, x_start+i*size, y_start+8*size, 0x33); // vertical
	kol_paint_line(x_start, y_start+i*size, x_start+8*size, y_start+i*size, 0x33); // horizontal
	}

// foxes found by user	
for (x1 = 0; x1 < 8; x1++)
	for (y1 = 0; y1 < 8; y1++)
		{
		if (board[x1][y1][LAY_FOUND]==1)
			{
			grid_to_pos(x1, y1, &x, &y);
			kol_paint_string(x+size/2-4, y+size/2-4, "F", 0x90227722);
			}
		}	

// opened cells
for (x1 = 0; x1 < 8; x1++)
	for (y1 = 0; y1 < 8; y1++)
	{
	if (board[x1][y1][LAY_OPENED] == 1)
		{
		itoa(board[x1][y1][LAY_NUM], tmp);
		grid_to_pos(x1, y1, &x, &y);
		kol_paint_string(x+size/2-4, y+size/2-4, tmp, 0x902222ff);
		}
	}		
		
if (result > 0)
	{
	char victory[]={"V I C T O R Y !"};
	kol_paint_string(window_width/2-7*8+4, window_height-14-kol_skin_height()-4, victory, 0x90227722);
	kol_paint_string(window_width/2-7*8+4-1, window_height-14-kol_skin_height()-4, victory, 0x90227722);
	kol_paint_string(window_width/2-7*8+4+1, window_height-14-kol_skin_height()-4, victory, 0x90227722);
	}

if (result < 0)
	{

	// draw real position of foxes
	for (x1 = 0; x1 < 8; x1++)
		for (y1 = 0; y1 < 8; y1++)
			{
			if (board[x1][y1][LAY_HIDDEN]==1)
				{
				grid_to_pos(x1, y1, &x, &y);
				kol_paint_string(x+size/2-2, y+size/2-2, "F", 0x90ff2222);
				}
			}		
			
	char game_over[]={"G A M E   O V E R !"};
	kol_paint_string(window_width/2-9*8+4, window_height-14-kol_skin_height()-4, game_over, 0x90ff2222);
	kol_paint_string(window_width/2-9*8+4-1, window_height-14-kol_skin_height()-4, game_over, 0x90ff2222);
	kol_paint_string(window_width/2-9*8+4+1, window_height-14-kol_skin_height()-4, game_over, 0x90ff2222);
	}	
		
kol_paint_end();



}


int check()
{
int x, y;
for (x = 0; x < 8; x++)
	for (y = 0; y < 8; y++)
		if (board[x][y][LAY_HIDDEN] != board[x][y][LAY_FOUND])
			return -1;
return 1;
}


void grid_to_pos(unsigned gx, unsigned gy, unsigned* x, unsigned* y)
{
*x = gx*size + x_start;
*y = gy*size + y_start;
}


int pos_to_grid(unsigned x, unsigned y, int* gx, int* gy)
{

*gx = (x - x_start)/size;
*gy = (y - y_start)/size;

if ((*gx < 0) || (*gx>7) || (*gy < 0) || (*gy>7) )
	return -1;
else
	return 1;
}


/// ===========================================================

void kol_main()
{

unsigned event;
unsigned key;
unsigned btn;
unsigned pos, x, y;
int gx, gy;


srand(kol_system_time_get());

kol_event_mask( 0xC0000027 ); // enable using of mouse 
init_grid_sizes();
init_board();
wnd_draw();


for(;;)
	{
	event = kol_event_wait();

	switch (event)
		{
		case 1:
			wnd_draw();
			break;

		case 2:
			key = (kol_key_get() & 0xff00)>>8;
			break;
			
		case 3:
			switch ((kol_btn_get() & 0xff00)>>8)
				{
				case 1: // close button
					kol_exit();
				case 2: // 'new' button 
					init_board();
					wnd_draw();
					break;
				}
			break;
			
		case 6:
			btn = kol_mouse_btn(); // read mouse button
			pos = kol_mouse_posw(); // read mouse position
			x = pos / 65536;
			y = pos % 65536;
			if (x > window_width)
				x=0;
			if (y > window_height)
				y=0;

			if ( pos_to_grid(x, y, &gx, &gy) > 0 )
				{
				switch (btn & 3)
					{
					case 1: // left button
						if (result == 0) // are we in game?
							{
							if (board[gx][gy][LAY_FOUND] == 0)
								{
								if ( board[gx][gy][LAY_OPENED] == 0 )
									board[gx][gy][LAY_OPENED] = 1;
								else
									board[gx][gy][LAY_OPENED] = 0;
								moves++;
								wnd_draw();
								}
							}
						break;
					case 2: // right button
						if (result == 0) // are we in game?
							{
							if (board[gx][gy][LAY_FOUND] == 0)
								{
								if ( board[gx][gy][LAY_OPENED] == 0 )
									{
									board[gx][gy][LAY_FOUND] = 1;
									foxLeft--;
									moves++;
									}
								}
							else
								{
								board[gx][gy][LAY_FOUND] = 0;
								foxLeft++;
								moves++;
								}
								
							if (foxLeft == 0) // all 4 foxes are marked
								result = check();
								
							wnd_draw();
							}
						break;
					}
				}
				break;
		}

	
	}

kol_exit();
}

/// ===========================================================
