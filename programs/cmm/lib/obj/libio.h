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
char alibio[] = "/sys/lib/libio.obj"; //"libio.obj";

dword libio_init = #alibio_init;
dword file_size  = #afile_size;
dword file_open = #afile_open;
dword file_read  = #afile_read;
dword file_close = #afile_close;
$DD 2 dup 0


//import  libio                     , \
char alibio_init[] = "lib_init";
char afile_size[]  = "file_size";
char afile_open[] = "file_open";
char afile_read[]  = "file_read";
char afile_close[] = "file_close";

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