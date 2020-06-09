// FLAC decoder for KolibriOS in native shared COFF library format.

// https://github.com/mackron/dr_libs/blob/master/dr_flac.h

// Some functions to allow us building without any external libs

#include <stddef.h>

void* memset(void *base, int val, size_t len)
{
   int i;
   for (i=0; i < len; ++i) ((char *) base)[i] = val;
}

int memcmp(const void *p1, const void *p2, size_t len)
{
   unsigned char *q1 = (unsigned char *) p1;
   unsigned char *q2 = (unsigned char *) p2;
   int i;
   for (i=0; i < len; ++i)
      if (q1[i] < q2[i]) return -1;
      else if (q1[i] > q2[i]) return 1;
   return 0;
}

void* memcpy(void *dest, const void *src, size_t num)
{
   int i;
   for (i=0; i < num; ++i)
      ((char *)dest)[i] = ((char *) src)[i];
}

// For building with mingw compiler
__attribute__((externally_visible)) void __chkstk_ms(){
	return;
}

void* malloc(size_t size) {
	asm("int3");
	return 0;
}

void* realloc(void *ptr, size_t size) {
	asm("int3");
	return 0;
}

void free(void *ptr) {
	asm("int3");
	return;
}

#define DRFLAC_ASSERT(expr)					((void) (0))
#define DRFLAC_MALLOC(sz)                   malloc((sz))
#define DRFLAC_REALLOC(p, sz)               realloc((p), (sz))
#define DRFLAC_FREE(p)                      free((p))

// Actual dr_flac related stuff starts here

#define DRFLAC_IMPLEMENTATION

#define DR_FLAC_NO_STDIO
//#define DR_FLAC_NO_OGG
//#define DR_FLAC_BUFFER_SIZE 4096
//#define DR_FLAC_NO_CRC
#define DR_FLAC_NO_SIMD

#include "dr_flac.h"

// KolibriOS type EXPORTS header
 
int __stdcall libInit(){
      return 1;
}

typedef struct{
  char *name;
  void *f;
}export_t;

char szLibInit[]   			="lib_init";
char szVersion[]			="version";
char szClose[]   			="close";
char szOpen[]   			="open";
char szOpenRelaxed[]   		="open_relaxed";
char szReadPCMS16[]			="read_pcm_frames_s16";


__attribute__((externally_visible))  export_t EXPORTS[] __asm__("EXPORTS") =
  {
	{ szLibInit,		libInit },
	{ szVersion,		(void*)0x00010001 },	
	{ szClose,			drflac_close },
	{ szOpen,			drflac_open },
    { szOpenRelaxed,	drflac_open_relaxed },
	{ szReadPCMS16,		drflac_read_pcm_frames_s16},

    { NULL, NULL },
  };

// End of file