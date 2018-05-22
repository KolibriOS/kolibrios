#ifndef INCLUDE_LIBRASTERWORKS_H
#define INCLUDE_LIBRASTERWORKS_H

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

#ifndef INCLUDE_DLL_H
#include "../lib/dll.h"
#endif

dword librasterworks = #alibrasterworks;
char alibrasterworks[] = "/sys/lib/rasterworks.obj";

dword rasterworks_drawText = #arasterworks_drawText;
dword rasterworks_cntUTF_8 = #arasterworks_cntUTF_8;
dword rasterworks_charsFit = #arasterworks_charsFit;
dword rasterworks_strWidth = #arasterworks_strWidth;
$DD 2 dup 0

char arasterworks_drawText[] = "drawText";
char arasterworks_cntUTF_8[] = "cntUTF-8";
char arasterworks_charsFit[] = "charsFit";
char arasterworks_strWidth[] = "strWidth";

//===================================================//
//                                                   //
//                    FUCTIONS                       //
//                                                   //
//===================================================//



#endif
