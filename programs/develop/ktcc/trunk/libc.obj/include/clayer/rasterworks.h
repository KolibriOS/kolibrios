#ifndef KOLIBRI_RASTERWORKS_H
#define KOLIBRI_RASTERWORKS_H

#include <stddef.h>

DLLAPI void __stdcall drawText(void *canvas, int x, int y, const char *string, int charQuantity, int fontColor, int params);
DLLAPI int  __stdcall countUTF8Z(const char *string, int byteQuantity) __asm__("cntUTF-8");
DLLAPI int  __stdcall charsFit(int areaWidth, int charHeight);
DLLAPI int  __stdcall strWidth(int charQuantity, int charHeight);

#endif /* KOLIBRI_RASTERWORKS_H */