#ifndef PAINT_H
#define PAINT_H

#include "defines.h"
#include "rect.h"

void canvas_init(rect* r);
void canvas_delete();
void canvas_fill(__u32 color);

void canvas_draw_rect(rect* r, __u32 color);
void canvas_draw_text(rect* r, char* txt, __u32 len, __u32 color);
void canvas_draw_value(rect *r, __u32 v, __u32 color);

void canvas_paint();

#endif // PAINT_H
