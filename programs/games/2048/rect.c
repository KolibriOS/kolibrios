#include "rect.h"

void rect_draw(rect* r, __u32 color)
{
    __menuet__bar(r->x,r->y,r->width,r->height,color);
}

__u8 rect_transform(rect* from, rect* to, __u16 step)
{
    if (from->width < to->width)
    {
        from->width += (step << 1);
        if (from->width > to->width) from->width = to->width;
    }
    else if (from->width > to->width)
    {
        from->width -= (step << 1);
        if (from->width < to->width) from->width = to->width;
    }

    if (from->height < to->height)
    {
        from->height += (step << 1);
        if (from->height > to->height) from->height = to->height;
    }
    else if (from->height > to->height)
    {
        from->height -= (step << 1);
        if (from->height < to->height) from->height = to->height;
    }

    if (from->x < to->x)
    {
        from->x += step;
        if (from->x > to->x) from->x = to->x;
    }
    else if (from->x > to->x)
    {
        from->x -= step;
        if (from->x < to->x) from->x = to->x;
    }

    if (from->y < to->y)
    {
        from->y += step;
        if (from->y > to->y) from->y = to->y;
    }
    else if (from->y > to->y)
    {
        from->y -= step;
        if (from->y < to->y) from->y = to->y;
    }

    return (from->x == to->x)           &&
            (from->y == to->y)          &&
            (from->width == to->width)  &&
            (from->height == to->height);
}

void rect_draw_text(rect *r, char *txt, __u32 len, __u32 color, __u32 frame_color)
{
    // right down shadow
    __menuet__write_text(r->x + 1 + (r->width - text_length_px(len)) / 2,
                         r->y + 1 + (r->height - FONT_HEIGHT) / 2,
                         frame_color,txt,len);
    // right shadow
    __menuet__write_text(r->x + 1 + (r->width - text_length_px(len)) / 2,
                         r->y + (r->height - FONT_HEIGHT) / 2,
                         frame_color,txt,len);
    // down shadow
    __menuet__write_text(r->x + (r->width - text_length_px(len)) / 2,
                         r->y + 1 + (r->height - FONT_HEIGHT) / 2,
                         frame_color,txt,len);

    __menuet__write_text(r->x + (r->width - text_length_px(len)) / 2,
                         r->y + (r->height - FONT_HEIGHT) / 2,
                         color,txt,len);
}

void rect_draw_value(rect* r, __u32 v, __u32 color, __u32 frame_color)
{
    char buffer[16] = {0};
    __u32 length = strlen(itoa(v,buffer,10));
    rect_draw_text(r,buffer,length,color,frame_color);
}
