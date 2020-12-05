#include <kos32sys.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <kolibri_libimg.h>
/*
#include "mp3.h"
*/
//EVENTS LOOK LIKE IN C--
#define evReDraw  1
#define evKey     2
#define evButton  3
#define evExit    4
#define evDesktop 5
#define evMouse   6
#define evIPC     7
#define evNetwork 8
#define evDebug   9


#define RESTART 99

#define b_color 0xbbbbbb
#define _size 4
#define bs 44

int field[_size][_size] = {
	{0, 0, 0, 1},
	{0, 0, 1, 0},
	{0, 0, 0, 1},
	{1, 0, 0, 0}
};
short vict = 0;
short debug_pr = 0;

char* title = "Fridge";


// PICTURES LOADING CODE
char temp_path[4096];
char* HOR;
char* VER;


char*   load_file_inmem(char* fname, int32_t* read_sz)
{
		FILE *f = fopen(fname, "rb");
		if (!f) {
			exit(1);
		}
		if (fseek(f, 0, SEEK_END)) {
			exit(1);
		}
		int filesize = ftell(f);
		rewind(f);
		char* fdata = malloc(filesize);
		if(!fdata) {
			exit(1);
		}
		*read_sz = fread(fdata, 1, filesize, f);
		if (ferror(f)) {
			exit(1);
		}
		fclose(f);

		return fdata;
}


void load_pict() {
		const int icon_rgb_size = bs*bs;
		char *image_data,
			 *filedata;
		
		strcpy(temp_path, "h.png");

		int32_t read_bytes;
		filedata = load_file_inmem(temp_path, &read_bytes);
		HOR = malloc(icon_rgb_size * 3);
		
		image_data = img_decode(filedata, read_bytes, 0); 
		
		img_to_rgb2(image_data, HOR);
		
		
		strcpy(temp_path, "v.png");

		filedata = load_file_inmem(temp_path, &read_bytes);
		VER = malloc(icon_rgb_size * 3);
		
		image_data = img_decode(filedata, read_bytes, 0); 
		
		img_to_rgb2(image_data, VER);
		
		img_destroy(image_data);
		free(filedata);
}
// END OF PICTURES LOAD CODE



void redraw_buttons() {
		for (int j = 5, yy = 0; yy<_size; j+=bs, yy++)
				for (int i = 15, xx = 0; xx<_size; i+=bs, xx++)
				{
					define_button(65536 * i + (bs-1), 65536 * j + (bs-1), ((xx+1)*10)+yy+1, b_color);
					
					if (field[yy][xx]) draw_bitmap(VER, i, j, bs, bs);
					else draw_bitmap(HOR, i, j, bs, bs);
				}
}

void draw_game_window(){
		BeginDraw(); 
		DrawWindow(215,100,220, 220,title,b_color,0x34);
		redraw_buttons();
		EndDraw();
}



static inline
void draw_text_sysNEW(const char *text, int x, int y, int len, int fontType, color_t color)
{
		__asm__ __volatile__(
		"int $0x40"
		::"a"(4),"d"(text),
		  "b"((x << 16) | y),
		  "S"(len),"c"(fontType<<24+color)
		 :"memory");
}

void SetUp() {
		for (int y = 0; y<_size; y++)
					for (int x = 0; x<_size; x++)
					{
						field[x][y] = rand() % 2;
					}
}

void draw_victory_window() {
		BeginDraw(); 
		DrawWindow(215,100,220, 220,title,b_color,0x34);
		
		draw_text_sysNEW("Ну вы, и", 10, 10, strlen("Ну вы, и"), 0xB1, 0x000000);
		draw_text_sysNEW("медвежатник,", 10, 50, strlen("Ну вы, и медвежатник,"), 0xB1, 0x000000);
		draw_text_sysNEW("Шеф!", 12, 90, strlen("Шеф!"), 0xB1, 0x000000);
		
		if (debug_pr) {
			printf("Fridge: Very great!\n");
			debug_pr = 0;
		}
		
		define_button(65536 * ((220/2)-(50)) + 140, 65536 * 140 + 25+12, RESTART, 0x9A9A9A);
		draw_text_sysNEW("Заново", 80, 145, strlen("Заново"), 0xB1, 0x000000);
		EndDraw();
}



void Button() {
		int id = get_os_button();
		if (id == 1) exit(0); else
		if (id == RESTART) {
			SetUp();
			vict = 0;
			draw_game_window();
		} else
		{
			// PlayMusic("./rotate.mp3");
			
			int x = (id/10)-1;
			int y = (id%10)-1;
			
			for (int i = 0; i<_size; i++)
				if (field[i][x]) field[i][x] = 0; else field[i][x] = 1;
			
			for (int i = 0; i<_size; i++)
				if (field[y][i]) field[y][i] = 0; else field[y][i] = 1;
				
			if (field[y][x]) field[y][x] = 0; else field[y][x] = 1;
			
			redraw_buttons();
		}
}


int fridge_opened() {
		int fr_op = 0;
		for (int y = 0; y<_size; y++)
				for (int x = 0; x<_size; x++)
				{
					fr_op += field[x][y];
				}
		if (fr_op == 0) return 1;
		return 0;
}


int main()
{
		srand(time(0));
		
		if (kolibri_libimg_init() == -1) 
		{
			printf("Can not load libimg.obj!\n");
			exit(1);
		}
		load_pict();
		
		draw_game_window();
		while(1)
		{
			switch(get_os_event())
			{
				case evButton:
					Button();
					if (fridge_opened()) {
						vict = 1;
						debug_pr = 1;
						draw_victory_window();
					}
					break;
			  
				case evKey:
					get_key();
					break;
				 
				case evReDraw:
					if (!vict) draw_game_window();
					else draw_victory_window();
					break;
			}
		}
}
