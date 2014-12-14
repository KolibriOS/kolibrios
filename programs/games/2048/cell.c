#include "cell.h"

__u8 tile_draw(tile* t)
{
    if (t->value)
    {
        __u32 bg_color = 0;
        __u32 txt_color = 0;
        switch (t->value)
        {
        case 0      : bg_color = CELL_COLOR;    break;
        case 2      : bg_color = 0xEEE4DA;     txt_color = 0x776E65;  break;
        case 4      : bg_color = 0xEDE0C8;     txt_color = 0x776E65;  break;
        case 8      : bg_color = 0xF2B179;     txt_color = 0xF9F6F2;  break;
        case 16     : bg_color = 0xF59563;     txt_color = 0xF9F6F2;  break;
        case 32     : bg_color = 0xF67C5F;     txt_color = 0xF9F6F2;  break;
        case 64     : bg_color = 0xF65E3B;     txt_color = 0xF9F6F2;  break;
        case 128    : bg_color = 0xEDCF72;     txt_color = 0xF9F6F2;  break;
        case 256    : bg_color = 0xEDCC61;     txt_color = 0xF9F6F2;  break;
        case 512    : bg_color = 0xEDC850;     txt_color = 0xF9F6F2;  break;
        case 1024   : bg_color = 0xEDC53F;     txt_color = 0xF9F6F2;  break;
        case 2048   : bg_color = 0xEDC22E;     txt_color = 0xF9F6F2;  break;
        default     : bg_color = 0x3C3A32;     txt_color = 0xF9F6F2;  break;
        }

        rect* begin = &t->cell;
        rect* end = &t->transition;

        if (rect_transform(begin,end,t->ani_step))
            t->animate = false;

        canvas_draw_rect(begin,bg_color);
        canvas_draw_value(begin,t->value,txt_color);

        if (t->merged)
        {
            if (rect_transform(&t->merged_rect,end,t->ani_step) &&
                    (t->animate == false))
            {
                t->animate = true;
                t->merged = false;
                t->value *= 2;
            }

            canvas_draw_rect(&t->merged_rect,bg_color);
            canvas_draw_value(&t->merged_rect,t->value,txt_color);
        }
    }

    return t->animate;
}

__u8 tile_mergeable(tile* from, tile* to)
{
    return (from && !from->merged &&
            to && !to->merged &&
            (from->value == to->value));
}
