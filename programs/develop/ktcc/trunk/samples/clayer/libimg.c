#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <clayer/libimg.h>
#include <kos32sys1.h>

struct kolibri_system_colors sys_color_table;

char path[4096];
char* picture;
int x_size = 200, y_size = 150;

char* load_file_inmem(char* fname, int32_t* read_sz)
{
    FILE *f = fopen(fname, "rb");
    if (!f) {
        printf("Can't open file: %s\n", fname);
    }
    if (fseek(f, 0, SEEK_END)) {
        printf("Can't SEEK_END file: %s\n", fname);
    }
    int filesize = ftell(f);
    rewind(f);
    char* fdata = malloc(filesize);
    if(!fdata) {
        printf("No memory for file %s\n", fname);
    }
    *read_sz = fread(fdata, 1, filesize, f);
    if (ferror(f)) {
        printf("Error reading file %s\n", fname);
    }
    fclose(f);

    return fdata;
}

void draw_window()
{
    BeginDraw();
	DrawWindow(10, 40, x_size + 50, y_size + 50, "Libimg", sys_color_table.work_area, 0x34);
	
	// Draw Picture
	draw_bitmap(picture, 10, 10, x_size, y_size);
	
    EndDraw();
}

int main()
{
    if (kolibri_libimg_init() == -1)
    {
		printf("Error loading lib_img.obj\n");
	}

    // Load Image 
    const int icon_rgb_size = x_size * y_size;
    char *image_data,
         *filedata;
    
    strcpy(path, "kolibrios.jpg"); // Filename
    int32_t read_bytes;
    filedata = load_file_inmem(path, &read_bytes);
    picture = malloc(icon_rgb_size * 3);
    image_data = img_decode(filedata, read_bytes, 0); 
    img_to_rgb2(image_data, picture);
    img_destroy(image_data);
    free(filedata);
	// End Load Image

	get_system_colors(&sys_color_table);
    set_event_mask(0xC0000027);

    while (1) {
		switch(get_os_event())
		{
			case KOLIBRI_EVENT_REDRAW:
				draw_window();
				break;
			case KOLIBRI_EVENT_BUTTON:
				if (get_os_button() == 1) exit(0);
				break;
		}
	}
	return 0;
}
