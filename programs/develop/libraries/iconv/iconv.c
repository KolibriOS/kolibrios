#include <string.h>
#include <stdio.h>
#include <errno.h>

typedef int conv_t;
typedef unsigned int ucs4_t;
typedef int iconv_t;

/* Return code if invalid input after a shift sequence of n bytes was read.
   (xxx_mbtowc) */
#define RET_SHIFT_ILSEQ(n)  (-1-2*(n))
/* Return code if invalid. (xxx_mbtowc) */
#define RET_ILSEQ           RET_SHIFT_ILSEQ(0)
/* Return code if only a shift sequence of n bytes was read. (xxx_mbtowc) */
#define RET_TOOFEW(n)       (-2-2*(n))

/* Return code if invalid. (xxx_wctomb) */
#define RET_ILUNI      -1
/* Return code if output buffer is too small. (xxx_wctomb, xxx_reset) */
#define RET_TOOSMALL   -2

#define CP866 0
#define CP1251 1
#define CP1252 2
#define KOI8_RU 3
#define ISO8859_5 4
#define UTF_8 5
#define KOI8_R 6
#define ISO8859_1 7

#include "cp866.h"
#include "cp1251.h"
#include "cp1252.h"
#include "koi8_r.h"
#include "koi8_ru.h"
#include "iso8859_1.h"
#include "iso8859_5.h"
#include "utf8.h"

int encoding(const char *someencoding) {

	char *what = strdup(someencoding);
	/* Ignore //TRANSLIT or //IGNORE for now. */
	int i;
	for(i = 0; i < strlen(what); i++) {
	  if(what[i] == '/') {
		what[i] = '\0';
		break;
	  }
	}

	if (!strcasecmp(what,"CP866")) return CP866;
	if (!strcasecmp(what,"CP1251")) return CP1251;
	if (!strcasecmp(what,"windows-1251")) return CP1251;
	if (!strcasecmp(what,"CP1252")) return CP1252;
	if (!strcasecmp(what,"windows-1252")) return CP1252;	
	if (!strcasecmp(what,"KOI8-R")) return KOI8_R;	
	if (!strcasecmp(what,"KOI8-RU")) return KOI8_RU;
	if (!strcasecmp(what,"ISO8859-1")) return ISO8859_1;	
	if (!strcasecmp(what,"ISO8859-5")) return ISO8859_5;
	if (!strcasecmp(what,"UTF-8")) return UTF_8;
	return -1;
}

iconv_t iconv_open(const char *tocode, const char *fromcode) {
	int to, from;

	if ((to=encoding(tocode))==-1) return -1;
	if ((from=encoding(fromcode))==-1) return -1;

	to=to<<16&0xFFFF0000;
	from=from&0xFFFF;

	return to+from;
}

int iconv_close(iconv_t icd)
{
  return 0;
}

size_t iconv(iconv_t cd, const char **inbuf, size_t *inbytesleft,
	char **outbuf, size_t *outbytesleft)
{
	int n, to, from;
	size_t count1,count2;
	unsigned int pwc;
	int converted,written;
	int (*mbtowc)(conv_t, ucs4_t *, const unsigned char *, int);
	int (*wctomb)(conv_t, unsigned char *, ucs4_t, int);

	to=cd>>16;
	from=cd&0xFFFF;

	switch (from)
	{
		case CP866: mbtowc=cp866_mbtowc; break;
		case CP1251: mbtowc=cp1251_mbtowc; break;
		case CP1252: mbtowc=cp1252_mbtowc; break;
		case ISO8859_1: mbtowc=iso8859_1_mbtowc; break;		
		case ISO8859_5: mbtowc=iso8859_5_mbtowc; break;
		case KOI8_R: mbtowc=koi8_r_mbtowc; break;		
		case KOI8_RU: mbtowc=koi8_ru_mbtowc; break;
		case UTF_8: mbtowc=utf8_mbtowc; break;
		default: return (size_t)-1;
	}

	switch (to)
	{
		case CP866: wctomb=cp866_wctomb; break;
		case CP1251: wctomb=cp1251_wctomb; break;
		case CP1252: wctomb=cp1252_wctomb; break;
		case ISO8859_1: wctomb=iso8859_1_wctomb; break;		
		case ISO8859_5: wctomb=iso8859_5_wctomb; break;
		case KOI8_R: wctomb=koi8_r_wctomb; break;		
		case KOI8_RU: wctomb=koi8_ru_wctomb; break;
		case UTF_8: wctomb=utf8_wctomb; break;
		default: return (size_t)-1;
	}

	count1=0;
	count2=0;

    /* Convert input multibyte char to wide character by using calls to mbtowc */
    /* Convert wide character to multibyte by calls to wctomb */
    /* Handle errors as we go on converting to be as standard compliant as possible */
    while(count1 < *inbytesleft) {
      unsigned char mbholder[] = { 0,0,0,0,0,0 };

      int numbytes = (mbtowc)(0, &pwc,((*inbuf)+count1), *inbytesleft - count1);
      if(numbytes < 0) {
        /* errno = EILSEQ if invalid multibyte sequence encountered in input */
        /* errno = EINVAL if input ends in the middle of a multibyte sequence */

        switch(numbytes) {
          case RET_TOOFEW(0):
            errno = EINVAL;
            break;

          case RET_ILSEQ:
            errno = EILSEQ;
            break;
        }

        *inbytesleft -= count1;
        *outbytesleft -= count2;
        *inbuf += count1;
        *outbuf += count2;
        return (size_t) -1;
      }

      /* Convert from wide to multibyte storing result in mbholder and num converted in numbytes2 */
      /* Pass the minimum amount of space we have, one from mbholder and one from remaining in outbuf */
      int minspace = sizeof(mbholder) <= (*outbytesleft - count2) ? sizeof(mbholder) : (*outbytesleft - count2);

      int numbytes2 = (wctomb)(0, &mbholder[0], pwc, minspace);
      if(numbytes2 < 0) {
        switch(numbytes2) {
          case RET_ILUNI:
            errno = EILSEQ;
            break;
          case RET_TOOSMALL:
            errno = E2BIG;
            break;
        }

        *inbytesleft -= count1;
        *outbytesleft -= count2;
        *inbuf += count1;
        *outbuf += count2;

        return (size_t) -1;
      }

      int i;
      for(i = 0; i < numbytes2; i++) {
        *(*outbuf + count2 + i) = mbholder[i];
      }

      count1+=numbytes;
      count2+=numbytes2;
    }

    /* Successfully converted everything, update the variables and return number of bytes converted */
    *inbytesleft -= count1;
    *outbytesleft -= count2;
    *inbuf += count1;
    *outbuf += count2;

    return count1;
}

/* int main() */
/* { */
/* 	char *s;// ="вертолет"; */
/* 	char *z; */
/* 	//unsigned int pwc; */
/* 	iconv_t cd; */
/* 	size_t in, out; */

/*     FILE *infile; */
/*     char *fname = "file3.txt"; */

/*     size_t testmax = 100; */
/*     size_t test = 0; */

/*     infile = fopen(fname,"r"); */

/* 	fseek(infile, 0, SEEK_END); */
/* 	size_t file_size = ftell(infile); */
/* 	rewind(infile); */

/* 	char *buffer = (char*)malloc(file_size * sizeof(char)); */
/* 	if (buffer == NULL) */
/* 	{ */
/* 		fclose(infile); */
/* 		printf("Error allocating %d bytes.\n", file_size * sizeof(char)); */
/* 		return -1; */
/* 	} */
/* 	size_t bytes_read = fread(buffer, sizeof(char), file_size, infile); */
/* 	if (bytes_read != file_size) */
/* 	{ */
/* 		/\* printf("Have read only %d bytes of %d.\n", bytes_read, file_size); *\/ */
/* 		free(buffer); */
/* 		fclose(infile); */
/* 		return -1; */
/* 	} */

/* 	/\* in=strlen(buffer); *\/ */
/*     in = bytes_read; */
/* 	z=malloc(in+12000); */

/* 	out=in-1000; */
/* 	cd=iconv_open("UTF-8","UTF-8"); */
/* //	printf("%x\n",cd); */
/* 	int t; */
/*     char *zor = z; */

/*     /\* for(t = 0; t < 27400; t++) *\/ */
/*     /\*    printf("0x%x,", buffer[t]); *\/ */

/* 	t=iconv(cd, &buffer, &in, &z, &out); */
/* 	/\* printf("\nResult after iconv(): %d", t); *\/ */

/*     /\* for(t = 0; t < 24259; t++) *\/ */
/*     /\*   printf("%c", zor[t]); *\/ */

/*  	//for (;s<s+strlen(s);s++) {cp866_mbtowc (0,  &pwc, s, 1);printf("%c=%u\n",*s,pwc);} */
/* } */

/* typedef struct */
/* { */
/* 	char *name; */
/* 	void *f; */
/* } export_t; */

/* char szStart[]           = "START"; */
/* char szVersion[]         = "version"; */
/* char sziconv_open[]    = "iconv_open"; */
/* char sziconv[]   = "iconv"; */

/* export_t EXPORTS[] __asm__("EXPORTS") = */
/* { */
/* 	{ szStart,       (void*)0x0 }, */
/* 	{ szVersion,     (void*)0x00010001 }, */
/* 	{ sziconv_open,  iconv_open    }, */
/* 	{ sziconv,       iconv   }, */
/* 	{ NULL,          NULL }, */
/* }; */
