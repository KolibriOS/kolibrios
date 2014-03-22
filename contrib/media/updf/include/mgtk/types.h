#ifndef __MGTK_TYPES_H
#define __MGTK_TYPES_H

typedef struct {
    int x,y;
} GPoint;

typedef struct {
    int x,y;
    int w,h;
    inline void Assign(int _x,int _y,int _w,int _h)
    {
     x=_x;
     y=_y;
     w=_w;
     h=_h;
    }
} GRect;

#define RGB(r,g,b)	(((b)&0xff)|(((g)&0x00ff)<<8)|(((r)&0x0000ff)<<16))

#endif
