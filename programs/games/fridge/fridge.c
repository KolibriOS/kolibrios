#include <kos32sys.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "kolibri_libimg.h"

/* 
 EVENTS LOOK LIKE IN C--
*/

#define evReDraw 1
#define evKey 2
#define evButton 3



#define _size 4

#define bs 44


int field[_size][_size] = {
	{0, 0, 0, 1},
	{0, 0, 1, 0},
	{0, 0, 0, 1},
	{1, 0, 0, 0}
};

char* title = "Fridge v0.1";

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




// END OF PIC LOAD CODE


void redraw_buttons() {
	for (int j = 5, yy = 0; yy<_size; j+=bs, yy++)
			for (int i = 15, xx = 0; xx<_size; i+=bs, xx++)
			{
				define_button(65536 * i + (bs-1), 65536 * j + (bs-1), ((xx+1)*10)+yy+1, 0xbbbbbb);
				
				if (field[yy][xx]) draw_bitmap(VER, i, j, bs, bs);
				else draw_bitmap(HOR, i, j, bs, bs);
			}
}

void draw_window(){
        BeginDraw(); 
        DrawWindow(215,100,220, 220,title,0xbbbbbb,0x34);
        redraw_buttons();
        EndDraw();
}

void Button() {
	int id = get_os_button();
	if (id == 1) exit(0); else
	{
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

int main(int argc, char **argv)
{
    if (kolibri_libimg_init() == -1) 
    {
		printf("Can not load libimg.obj!\n");
		exit(1);
	}
    
    load_pict();
    
    draw_window();
    while(1)
    {
		switch(get_os_event())
		{
			case evButton:
				Button();
				
				break;
		  
			case evKey:
				get_key();
				break;
			 
			case evReDraw:
				draw_window();
				break;
		}
    }
}
