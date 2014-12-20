#include "paint.h"

typedef struct {
    char b;
    char g;
    char r;
} rgb;

struct {
    rect    area;
    rgb*    image;
} canvas = {0};

__u32 canvas_index(__u16 row, __u16 column) {
    return column + row * canvas.area.width;
}

point canvas_position(__u32 index) {
    point p = {
        .x = index % canvas.area.width,
        .y = index / canvas.area.width
    };
    return p;
}

void canvas_init(rect *r)
{
    canvas.area = *r;

    canvas_delete();
    canvas.image = malloc(r->width * r->height * sizeof(rgb));
}

void canvas_delete()
{
    if (canvas.image)
        free(canvas.image);
}

void canvas_fill(__u32 color)
{
    if (canvas.image)
    {
        __u32 i = 0;
        __u32 len = canvas.area.width * canvas.area.height;
        for (i = 0; i < len; i++)
        {
            canvas.image[i] = *((rgb*)(&color));
        }
    }
}

void canvas_draw_rect(rect *r, __u32 color)
{
    if (canvas.image)
    {
        __u32 row = 0;
        __u32 column = 0;
        rgb* c = (rgb*)(&color);

        for (row = 0; row < r->height; row++)
        {
            for (column = 0; column < (r->width); column++)
            {
                canvas.image[canvas_index(row + r->y - canvas.area.y,
                                          column + r->x - canvas.area.x)] = *c;
            }
        }
    }
}

void canvas_draw_text(rect* r, char* txt, __u32 len, __u32 color)
{
    __u32 w;
    __u32 h;
    char* mem;

    h = FONT_HEIGHT;
    w = len * FONT_WIDTH + len;
    mem = malloc(sizeof(__u32) * 2 + h * w * sizeof(__u32));

    memset(mem,0,sizeof(__u32) * 2 + h * w * sizeof(__u32));
    ((__u32*)mem)[0] = w;
    ((__u32*)mem)[1] = h;

     __asm__ __volatile__("int $0x40"::
                          "a"(4),
                          "b"(0),
                          "c"(color | (8 << 24)),
                          "d"((__u32)txt),
                          "S"(len),
                          "D"((__u32)(mem)));

    __u16 x = r->x + (r->width - (len * FONT_WIDTH + len) * 2) / 2 - canvas.area.x;
    __u16 y = r->y + (r->height - FONT_HEIGHT * 2) / 2 - canvas.area.y;

    int row = 0;
    int column = 0;
    __u32* __mem = (__u32*)(mem + 8);
    for (row = 0; row < h; row++)
    {
        for (column = 0; column < w; column++)
        {
            __u32 c = __mem[column + row * w];
            if (c & 0xFF000000)
            {
                canvas.image[canvas_index(row * 2 + y,column * 2 + x)] = *((rgb*)&c);
                canvas.image[canvas_index(row * 2 + y,column * 2 + 1 + x)] = *((rgb*)&c);

                canvas.image[canvas_index(row * 2 + 1 + y,column * 2 + x)] = *((rgb*)&c);
                canvas.image[canvas_index(row * 2 + 1 + y,column * 2 + 1 + x)] = *((rgb*)&c);
            }
        }
    }

    memset(mem,0,sizeof(__u32) * 2 + h * w * sizeof(__u32));
    free(mem);
}

void canvas_draw_value(rect* r, __u32 v, __u32 color)
{
    char buffer[16] = {0};
    __u32 length = strlen(itoa(v,buffer,10));
    canvas_draw_text(r,buffer,length,color);
}

void canvas_paint()
{
    if (canvas.image)
    {
        __menuet__putimage(canvas.area.x,
                           canvas.area.y,
                           canvas.area.width,
                           canvas.area.height,
                           (char*)canvas.image);
    }
}
