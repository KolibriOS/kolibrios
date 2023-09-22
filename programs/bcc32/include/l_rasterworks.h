#ifndef __L_RASTERWORKS_INCLUDED_
#define __L_RASTERWORKS_INCLUDED_
//
// rasterworks - import table
//
#define import_rasterworks drawText

void (__stdcall* drawText)(void *canvas, long x, long y, const char *string, long charQuantity, long fontColor, long params) = (void (__stdcall*)(void*, long, long, const char*, long, long, long))&"drawText";
long (__stdcall* countUTF8Z)(const char *string, long byteQuantity) = (long (__stdcall*)(const char*, long))&"countUTF8Z";
long (__stdcall* charsFit)(long areaWidth, long charHeight) = (long (__stdcall*)(long, long))&"charsFit";
long (__stdcall* strWidth)(long charQuantity, long charHeight) = (long (__stdcall*)(long, long))&"strWidth";

asm{
	dd 0,0
}
#endif /* __L_RASTERWORKS_INCLUDED_ */

