#ifndef RECT_H
#define RECT_H

#include "defines.h"

typedef struct {
    short x;
    short y;
    short width;
    short height;
} rect;

typedef struct {
    short x;
    short y;
} point;

// Draw rect filled with color
void rect_draw(rect* r, __u32 color);

// Make transformation step
// Rect 'from' will be changed
// Return 'true' if transformation ends ('from' == 'to')
__u8 rect_transform(rect* from, rect* to, __u16 step);

// Draw text at the rect center
void rect_draw_text(rect* r, char* txt, __u32 len, __u32 color, __u32 frame_color);

// Draw value as text at the rect center
void rect_draw_value(rect* r, __u32 v, __u32 color, __u32 frame_color);

#endif // RECT_H
