// OGG vorbis for KolibriOS in native shared COFF library format.

// https://github.com/nothings/stb/blob/master/stb_vorbis.c

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

double powerd (double x, int y)
{
    double temp;
    if (y == 0)
    return 1;
    temp = powerd (x, y / 2);
    if ((y % 2) == 0) {
        return temp * temp;
    } else {
        if (y > 0)
            return x * temp * temp;
        else
            return (temp * temp) / x;
    }
}

double ldexp(double Value, int Exponent)
{
    return((Value * powerd(2.0, Exponent)));
}

#include "qsort.c"

// For building with mingw compiler
__attribute__((externally_visible)) void __chkstk_ms(){
	return;
}

# define assert(expr)                ((void) (0))

// Actual stb_vorbis related stuff starts here

#define STB_VORBIS_NO_STDIO 1
#define STB_VORBIS_NO_CRT 1
//#define STB_VORBIS_NO_PULLDATA_API 1

#include "stb_vorbis.c"

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
char szPushDataOpen[]   	="pushdata_open";
char szPushDataDecode[] 	="pushdata_decode";
char szPushDataFlush[] 		="pushdata_flush";
//char szConvertSamplesShort[] = "convert_samples_short";
char szConvertSamplesShortInterleaved[] = "convert_channels_short_interleaved";

__attribute__((externally_visible))  export_t EXPORTS[] __asm__("EXPORTS") =
  {
	{ szLibInit,		libInit },
	{ szVersion,		(void*)0x00010001 },	  
    { szPushDataOpen,	stb_vorbis_open_pushdata },
    { szPushDataDecode, stb_vorbis_decode_frame_pushdata },
	{ szPushDataFlush, 	stb_vorbis_flush_pushdata},
	//{ szConvertSamplesShort, convert_samples_short},
	{ szConvertSamplesShortInterleaved, convert_channels_short_interleaved},

    { NULL, NULL },
  };

// End of file