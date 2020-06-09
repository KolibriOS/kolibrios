// minimp3 for KolibriOS in native shared COFF library format.

// Some functions to allow us building without any external libs

// memset - may be optimized
typedef unsigned int size_t;
static inline void*  memset(void *mem, int c, unsigned size) {   
	unsigned int i;
	for (i = 0; i < size; i++ )
		*((char *)mem+i) = (char) c;
	
	return mem;	
}

// memcpy - may be optimized
void* memcpy(void *dest, const void *src, size_t count) {  
	unsigned int i;
	for (i = 0; i < count; i++)
		*(char *)(dest+i) = *(char *)(src+i);
	
	return 0;
}

// For building with mingw compiler
void __chkstk_ms(){
	return;
}

// Actual minimp3 related stuff starts here

#define MINIMP3_ONLY_MP3					// No MP2
//#define MINIMP3_ONLY_SIMD					// No SSE2, some platforms might not have it
#define MINIMP3_NO_SIMD
//#define MINIMP3_NONSTANDARD_BUT_LOGICAL
//#define MINIMP3_FLOAT_OUTPUT
#define MINIMP3_IMPLEMENTATION				// Include the actual decoder
#include "minimp3.h"


// KolibriOS type EXPORTS header
 
int __stdcall start(){
      return 1;
}

int __stdcall version_major(){
      return 1;
}

int __stdcall version_minor(){
     return 0;
}

typedef struct{
  char *name;
  void *f;
}export_t;

char szStart[]		="START";
char szVersion[]	="version";
char szVersionM[]	="version_min";
char szInit[]   	="init";
char szDecode[] 	="decode";

__attribute__((externally_visible)) export_t EXPORTS[] __asm__("EXPORTS") =
  {
	{ szStart,		start },
	{ szVersion,	version_major },
	{ szVersionM,	version_minor },	  
    { szInit,	  	mp3dec_init },
    { szDecode, 	mp3dec_decode_frame },

    { NULL, NULL },
  };

// End of file