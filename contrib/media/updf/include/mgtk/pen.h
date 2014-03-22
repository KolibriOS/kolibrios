#ifndef __MGTK_PEN_H
#define __MGTK_PEN_H

#include<mgtk/widget.h>

struct GImageData {
    int w,h;
    unsigned char * data;
};

class GPen
{
public:
    GPen(GWidget *);
    virtual ~GPen();
    void PutPixel(int,int,unsigned long);
    void Line(int,int,int,int,unsigned long);
    void FillRect(int,int,int,int,unsigned long);
    void DrawLines(GPoint *,int nr_point,unsigned long color);
    void WriteText(int,int,char *,int,unsigned long);
    void Rect(int,int,int,int,unsigned long);
    void Rect3d(int x,int y,int w,int h,unsigned long color);
    void FillRect2(int,int,int,int,unsigned long);
    void Line2(int,int,int,int,unsigned long);
    void PutImage(int,int,int,int,char *);
    void PutImage(int,int,struct GImageData *);
private:
    int _dx,_dy;
    int _lx,_ly;
};

#endif
