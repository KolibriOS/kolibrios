#ifndef __MGTK_BITMAP_H
#define __MGTK_BITMAP_H

#include<mgtk/widget.h>
#include<stdio.h>
extern "C" {
#include<libmgfx.h>
}

/* Image is 24-bit RGB bitmap */
class GImagePool: public GWidget
{
public:
    GImagePool(GRect *);
    virtual ~GImagePool();
    virtual void DrawWidget();
    virtual void Write(unsigned long,unsigned long,char *);
    virtual void Load(FILE *);
    virtual void Save(FILE *);
    virtual void PutPixel(int,int,unsigned long);
    GImagePool * Duplicate(GImagePool * Img);
private:
    char * image_data;
    unsigned long scan_size,image_size;
};

class GPicture: public GWidget
{
public:
 GPicture(GRect * r,char * filename);
 virtual ~GPicture();
 virtual void DrawWidget();
 virtual bool Load(char *);
private:
 mgfx_image_t * img;
};
    
#endif
