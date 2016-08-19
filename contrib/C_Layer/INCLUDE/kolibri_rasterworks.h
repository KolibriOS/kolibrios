#ifndef KOLIBRI_RASTERWORKS_H
#define KOLIBRI_RASTERWORKS_H

extern int kolibri_rasterworks_init(void); 

extern void (*drawText)(void *canvas, int x, int y, const char *string, int charQuantity, int fontColor, int params) __attribute__((__stdcall__));
extern int (*countUTF8Z)(const char *string, int byteQuantity) __attribute__((__stdcall__));
extern int (*charsFit)(int areaWidth, int charHeight) __attribute__((__stdcall__));
extern int (*strWidth)(int charQuantity, int charHeight) __attribute__((__stdcall__));

#endif /* KOLIBRI_RASTERWORKS_H */
