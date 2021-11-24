typedef unsigned int size_t;
#define NULL ((void*)0)

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

int strcmp (const char* a, const char* b) {
    return (*a && *b && (*a == *b)) ? ((*(a+1) || *(b+1)) ? (strcmp(a+1, b+1)) : (0)) : ((*a > *b) ? (1) : (-1));
}

#include "cp866.h"
#include "cp1251.h"
#include "cp1252.h"
#include "koi8_ru.h"
#include "iso8859_5.h"
#include "utf8.h"

int encoding(const char *what) {
    if (!strcmp(what,"CP866")) return CP866;
    if (!strcmp(what,"CP1251")) return CP1251;
    if (!strcmp(what,"CP1252")) return CP1252;
    if (!strcmp(what,"KOI8-RU")) return KOI8_RU;
    if (!strcmp(what,"ISO8859-5")) return ISO8859_5;
    if (!strcmp(what,"UTF-8")) return UTF_8;
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

size_t iconv(iconv_t cd, const char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft) {
    int n, to, from, count1,count2;
    int pwc, converted,written;
    int (*mbtowc)(conv_t, ucs4_t*, const unsigned char*, int);
    int (*wctomb)(conv_t, unsigned char*, ucs4_t, int);

    char *str;
    str=*outbuf;

    from=cd>>16;
    to=cd&0xFFFF;

    switch (from)
    {
        case CP866: mbtowc=cp866_mbtowc; break;
        case CP1251: mbtowc=cp1251_mbtowc; break;
        case CP1252: mbtowc=cp1252_mbtowc; break;
        case ISO8859_5: mbtowc=iso8859_5_mbtowc; break;
        case KOI8_RU: mbtowc=koi8_ru_mbtowc; break;
        case UTF_8: mbtowc=utf8_mbtowc; break;
        default: return -2;
    }

    switch (to)
    {
        case CP866: wctomb=cp866_wctomb; break;
        case CP1251: wctomb=cp1251_wctomb; break;
        case CP1252: wctomb=cp1252_wctomb; break;
        case ISO8859_5: wctomb=iso8859_5_wctomb; break;
        case KOI8_RU: wctomb=koi8_ru_wctomb; break;
        case UTF_8: wctomb=utf8_wctomb; break;
        default: return -3;
    }

    count1=0;
    count2=0;

    while ( *inbytesleft>0 && *outbytesleft>1)
    {
        n=1;

        do {
        //converted= (utf8_mbtowc)(0,&pwc,((*inbuf)+count1),n);
        //    printf("%d\n",n);
        converted = (mbtowc)(0,&pwc,((*inbuf)+count1),n);

        n++;
        }    while (converted==RET_TOOFEW(0));

        if (converted<0) return -10; 
        //written=  (cp866_wctomb)(0,str+count2,pwc,1);
        written = (wctomb)(0,str+count2,pwc,1);
        if (written<0) written=0;//return -11; 

        //printf("Conv:%d Wri:%d In:%d Out:%d UTF:%x UCS:%x 866:%s\n",converted, written, *inbytesleft,*outbytesleft,*((*inbuf)+count1),pwc, str);

        (*inbytesleft)-=converted;
        (*outbytesleft)-=written;
        count1+=converted;
        count2+=written;
    }
    *(str+count2)='\0';

    if (*inbytesleft>0 && *outbytesleft==0) return -12;
    return 0;
}

typedef struct {
    char *name;
    void *func;
} export_t;

export_t EXPORTS[] = {
    {"START",      (void*)0x0},
    {"version",    (void*)0x00010001},
    {"iconv_open", iconv_open},
    {"iconv",      iconv},
    NULL
};
