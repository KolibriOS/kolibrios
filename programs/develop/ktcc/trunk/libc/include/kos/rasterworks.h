#ifndef KOLIBRI_RASTERWORKS_H
#define KOLIBRI_RASTERWORKS_H

//extern int kolibri_rasterworks_init(void);

extern void (*drawText __attribute__((__stdcall__)))(void *canvas, int x, int y, const char *string, int charQuantity, int fontColor, int params);
extern int (*countUTF8Z __attribute__((__stdcall__)))(const char *string, int byteQuantity);
extern int (*charsFit __attribute__((__stdcall__)))(int areaWidth, int charHeight);
extern int (*strWidth __attribute__((__stdcall__)))(int charQuantity, int charHeight);

#endif /* KOLIBRI_RASTERWORKS_H */
