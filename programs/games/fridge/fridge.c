// Includes //
#include "kos.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <kolibri_libimg.h>
// #include "mp3.h"

// Code //
#define button_color 0xbbbbbb
#define button_size 44

#define field_size 4

int field[field_size][field_size] = {
	{0, 0, 0, 1},
	{0, 0, 1, 0},
	{0, 0, 0, 1},
	{1, 0, 0, 0}
};

char* title = "Fridge";
#define BUTTON_RESTART 99

short victory = 0;

// Load pictures //
char temp_path[4096];
Image* HORIZONTAL_IMAGE;
Image* VERTICAL_IMAGE;

char* load_file_inmem(char* fname, int32_t* read_sz)
{
		FILE *f = fopen(fname, "rb");
		if (!f) {
			exit(1);
		}
		if (fseek(f, 0, SEEK_END)) {
			exit(1);
		}
		int filefield_size = ftell(f);
		rewind(f);
		char* fdata = malloc(filefield_size);
		if(!fdata) {
			exit(1);
		}
		*read_sz = fread(fdata, 1, filefield_size, f);
		if (ferror(f)) {
			exit(1);
		}
		fclose(f);

		return fdata;
}

void load_pictures() {
		const int icon_rgb_field_size = button_size*button_size;
		Image *image_data;
		char *filedata;
		
		strcpy(temp_path, "h.png");

		int32_t read_bytes;
		filedata = load_file_inmem(temp_path, &read_bytes);
		HORIZONTAL_IMAGE = malloc(icon_rgb_field_size * 3);
		
		image_data = img_decode(filedata, read_bytes, 0); 
		
		img_to_rgb2(image_data, HORIZONTAL_IMAGE);
		
		
		strcpy(temp_path, "v.png");

		filedata = load_file_inmem(temp_path, &read_bytes);
		VERTICAL_IMAGE = malloc(icon_rgb_field_size * 3);
		
		image_data = img_decode(filedata, read_bytes, 0); 
		
		img_to_rgb2(image_data, VERTICAL_IMAGE);
		
		img_destroy(image_data);
		free(filedata);
}


// GUI functions //
void RedrawButtons() {
		for (int j = 5, x = 0; x<field_size; j+=button_size, x++)
				for (int i = 15, y = 0; y<field_size; i+=button_size, y++)
				{
					// 0x50 mean button without drawing, but with border when press
					// ((y+1)*10)+x+1 mean button id
					kos_DrawButton(i, j, button_size, button_size, (0x50 << 24) | ((y+1)*10)+x+1, 0);
					
					if (field[x][y]) kos_DrawBitmap(VERTICAL_IMAGE, i, j, button_size, button_size);
					else kos_DrawBitmap(HORIZONTAL_IMAGE, i, j, button_size, button_size);
				}
}

void draw_game_window(){
		kos_BeginDraw(); 
		kos_DrawWindow(215, 100, 220, 220, title, button_color, 0x34);
		RedrawButtons();
		kos_EndDraw();
}


// Need refactoring:
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
		for (int y = 0; y<field_size; y++)
			for (int x = 0; x<field_size; x++)
			{
				field[x][y] = rand() % 2;
			}
}

// Need refactoring:
void draw_victory_window() {
		kos_BeginDraw(); 
		kos_DrawWindow(215,100,220, 220,title,button_color,0x34);
		
		draw_text_sysNEW("Ну вы, и", 10, 10, strlen("Ну вы, и"), 0xB1, 0x000000);
		draw_text_sysNEW("медвежатник,", 10, 50, strlen("Ну вы, и медвежатник,"), 0xB1, 0x000000);
		draw_text_sysNEW("Шеф!", 12, 90, strlen("Шеф!"), 0xB1, 0x000000);
		
		kos_DrawButton(((220/2)-(50)), 140, 140, 25+12, BUTTON_RESTART, 0x9A9A9A);
		draw_text_sysNEW("Заново", 80, 145, strlen("Заново"), 0xB1, 0x000000);
		kos_EndDraw();
}

void Button() {
		int id = kos_GetButtonID();
		if (id == 1) exit(0); else
		if (id == BUTTON_RESTART) {
			SetUp();
			victory = 0;
			draw_game_window();
		} else
		{
			// PlayMusic("./rotate.mp3");
			
			int x = (id/10)-1;
			int y = (id%10)-1;
			
			for (int i = 0; i<field_size; i++)
				if (field[i][x]) field[i][x] = 0; else field[i][x] = 1;
			
			for (int i = 0; i<field_size; i++)
				if (field[y][i]) field[y][i] = 0; else field[y][i] = 1;
				
			if (field[y][x]) field[y][x] = 0; else field[y][x] = 1;
			
			RedrawButtons();
		}
}


int FridgeOpened() {
		int fr_op = 0;
		for (int y = 0; y<field_size; y++)
				for (int x = 0; x<field_size; x++)
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
			printf("Can not load libimg!\n");
			exit(1);
		}
		
		load_pictures();
		
		while(1)
		{
			switch(kos_WaitForEvent())
			{
				case evButton:
					Button();
					if (FridgeOpened()) {
						victory = 1;
						draw_victory_window();
					}
					break;
				case evReDraw:
					if (!victory) draw_game_window();
					else draw_victory_window();
					break;
			}
		}
}
