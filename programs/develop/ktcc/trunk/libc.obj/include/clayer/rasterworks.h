#ifndef KOLIBRI_RASTERWORKS_H
#define KOLIBRI_RASTERWORKS_H

#include <stddef.h>

int kolibri_rasterworks_init(void);

extern void __stdcall (*drawText)(void *canvas, int x, int y, const char *string, int charQuantity, int fontColor, int params);
extern int  __stdcall (*countUTF8Z)(const char *string, int byteQuantity) __asm__("cntUTF-8");
extern int  __stdcall (*charsFit)(int areaWidth, int charHeight);
extern int  __stdcall (*strWidth)(int charQuantity, int charHeight);

#endif /* KOLIBRI_RASTERWORKS_H */