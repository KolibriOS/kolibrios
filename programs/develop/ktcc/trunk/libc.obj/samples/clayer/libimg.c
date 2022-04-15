/* Written by turbocat2001 (Logaev Maxim) */

#include <clayer/libimg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ksys.h>

#define NEW_IMG_H 128
#define NEW_IMG_W 128

#define IMG_H 256
#define IMG_W 256

Image* image_blend; // Create image struct

ksys_colors_table_t sys_color_table; // Create system colors table

void* load_img(char* fname, uint32_t* read_sz)
{ // Image file upload function
    FILE* f = fopen(fname, "rb");
    if (!f) {
        printf("Can't open file: %s\n", fname);
        return NULL;
    }
    if (fseek(f, 0, SEEK_END)) {
        printf("Can't SEEK_END file: %s\n", fname);
        return NULL;
    }
    int filesize = ftell(f);
    rewind(f);
    void* fdata = malloc(filesize);
    if (!fdata) {
        printf("No memory for file %s\n", fname);
        return NULL;
    }
    *read_sz = fread(fdata, 1, filesize, f);
    if (ferror(f)) {
        printf("Error reading file %s\n", fname);
        return NULL;
    }
    fclose(f);
    return fdata;
}

void draw_gui()
{
    _ksys_start_draw();
    _ksys_create_window(10, 40, (IMG_W + NEW_IMG_W) + 50, IMG_H + 50, "Libimg", sys_color_table.work_area, 0x34);
    img_draw(image_blend, 10, 10, IMG_W * 2, IMG_H, 0, 0); // Draw blended image to window
    _ksys_end_draw();
}

int main()
{
    _ksys_get_system_colors(&sys_color_table); // Get system colors theme
    _ksys_set_event_mask(0xC0000027);

    uint32_t img_size;
    void* file_data = load_img("logo.png", &img_size); // Get RAW data and size
    if (!file_data) {
        return 1;
    }

    Image* image = img_decode(file_data, img_size, 0); // Decode RAW data to Image data

    if (image->Type != IMAGE_BPP32) {
        image = img_convert(image, NULL, IMAGE_BPP32, 0, 0); // Convert image to format BPP32
        if (!image) {
            printf("Сonvert error!: \n");
            return 1;
        }
    }

    image_blend = img_create(IMG_W + NEW_IMG_W, IMG_H, IMAGE_BPP32);                  // Create an empty layer
    img_fill_color(image_blend, IMG_W + NEW_IMG_W, IMG_H, sys_color_table.work_area); // Fill the layer with one color
    img_blend(image_blend, image, 0, 0, 0, 0, IMG_W, IMG_H);                          // Blending images to display the alpha channel.
    /* Reduce image size from 256x256 to 128x128 */
    image = img_scale(image, 0, 0, IMG_W, IMG_H, NULL, LIBIMG_SCALE_STRETCH, LIBIMG_INTER_BILINEAR, NEW_IMG_W, NEW_IMG_H);
    img_blend(image_blend, image, 256, 0, 0, 0, NEW_IMG_W, NEW_IMG_H);
    img_destroy(image); // Destroy image structure
    free(file_data);    // Free allocated file_data buffer

    /* Main event loop */
    while (1) {
        switch (_ksys_get_event()) {
        case KSYS_EVENT_REDRAW:
            draw_gui();
            break;

        case KSYS_EVENT_BUTTON:
            if (_ksys_get_button() == 1) {
                return 0;
            }
            break;
        }
    }
    return 0;
}
