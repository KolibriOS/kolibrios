//Asper
#ifndef INCLUDE_LIBIO_H
#define INCLUDE_LIBIO_H

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

#ifndef INCLUDE_DLL_H
#include "../lib/dll.h"
#endif

//library
dword libio = #alibio;
char alibio[21] = "/sys/lib/libio.obj\0"; //"libio.obj\0";
?define LIB_IO_LOAD_ERR "Error while loading library /rd/1/lib/libio.obj"

dword libio_init = #alibio_init;
dword file_size  = #afile_size;
dword file_open = #afile_open;
dword file_read  = #afile_read;
dword file_close = #afile_close;
$DD 2 dup 0


//import  libio                     , \
char alibio_init[9] = "lib_init\0";
char afile_size[11]  = "file_size\0";
char afile_open[12] = "file_open\0";
char afile_read[11]  = "file_read\0";
char afile_close[12] = "file_close\0";

//align 4
//dword fh=0;

#define O_BINARY  0
#define O_READ    1
#define O_WRITE   2
#define O_CREATE  4
#define O_SHARE   8
#define O_TEXT    16

#define SEEK_SET  0
#define SEEK_CUR  1
#define SEEK_END  2

#endif