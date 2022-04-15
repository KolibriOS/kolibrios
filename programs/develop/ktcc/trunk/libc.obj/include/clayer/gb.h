#ifndef KOLIBRI_GB_H
#define KOLIBRI_GB_H

#pragma pack(push, 1)
typedef struct
{
    int w;
    int h;
    char* bmp;
    char* alpha;
} GB_BMP;
#pragma pack(pop)

void gb_pixel_set(GB_BMP* b, int x, int y, unsigned c);
int gb_pixel_get(GB_BMP* b, int x, int y, unsigned* c);
void gb_line(GB_BMP* b, int x1, int y1, int x2, int y2, unsigned c);
void gb_rect(GB_BMP* b, int x, int y, int w, int h, unsigned c);
void gb_bar(GB_BMP* b, int x, int y, int w, int h, unsigned c);
void gb_circle(GB_BMP* b, int x, int y, int r, unsigned c);
void gb_image_set(GB_BMP* b_dest, int x_d, int y_d, GB_BMP* b_src, int x_s, int y_s, int w, int h);
void gb_image_set_t(GB_BMP* b_dest, int x_d, int y_d, GB_BMP* b_src, int x_s, int y_s, int w, int h, unsigned c);

#endif // KOLIBRI_GB_H