#include <stdio.h>
#include <string.h>
#include "7z.h"
#include "7zAlloc.h"
#include "7zBuf.h"
#include "7zCrc.h"
#include "7zFile.h"
#include "7zVersion.h"
#include "http.h"
#include "package.h"

#define PERIOD_4 (4 * 365 + 1)
#define PERIOD_100 (PERIOD_4 * 25 - 1)
#define PERIOD_400 (PERIOD_100 * 4 + 1)

#define _UTF8_START(n) (0x100 - (1 << (7 - (n))))

#define _UTF8_RANGE(n) (((UInt32)1) << ((n) * 5 + 6))

#define _UTF8_HEAD(n, val) ((Byte)(_UTF8_START(n) + (val >> (6 * (n)))))
#define _UTF8_CHAR(n, val) ((Byte)(0x80 + (((val) >> (6 * (n))) & 0x3F)))

#define MY_FILE_CODE_PAGE_PARAM

static ISzAlloc g_Alloc = { SzAlloc, SzFree };

static int Buf_EnsureSize(CBuf *dest, size_t size)
{
    if (dest->size >= size)
        return 1;
    Buf_Free(dest, &g_Alloc);
    return Buf_Create(dest, size, &g_Alloc);
}


int test_archive(const char *path)
{
    CFileInStream archiveStream;
    CLookToRead lookStream;
    CSzArEx db;
    SRes res;
    ISzAlloc allocImp;
    ISzAlloc allocTempImp;
    UInt16 *temp = NULL;

    allocImp.Alloc = SzAlloc;
    allocImp.Free = SzFree;

    allocTempImp.Alloc = SzAllocTemp;
    allocTempImp.Free = SzFreeTemp;

    if (InFile_Open(&archiveStream.file, path))
    {
        printf("can not open input file");
        return -1;
    }

    FileInStream_CreateVTable(&archiveStream);
    LookToRead_CreateVTable(&lookStream, False);

    lookStream.realStream = &archiveStream.s;
    LookToRead_Init(&lookStream);

    CrcGenerateTable();

    SzArEx_Init(&db);

    res = SzArEx_Open(&db, &lookStream.s, &allocImp, &allocTempImp);

    SzArEx_Free(&db, &allocImp);
    SzFree(NULL, temp);

    File_Close(&archiveStream.file);

    if (res == SZ_OK)
        return 0;
    else return -1;
};

static size_t Utf16_To_Utf8_Calc(const UInt16 *src, const UInt16 *srcLim)
{
    size_t size = 0;
    for (;;)
    {
        UInt32 val;
        if (src == srcLim)
            return size;

        size++;
        val = *src++;

        if (val < 0x80)
            continue;

        if (val < _UTF8_RANGE(1))
        {
            size++;
            continue;
        }

        if (val >= 0xD800 && val < 0xDC00 && src != srcLim)
        {
            UInt32 c2 = *src;
            if (c2 >= 0xDC00 && c2 < 0xE000)
            {
                src++;
                size += 3;
                continue;
            }
        }

        size += 2;
    }
}

static Byte *Utf16_To_Utf8(Byte *dest, const UInt16 *src, const UInt16 *srcLim)
{
    for (;;)
    {
        UInt32 val;
        if (src == srcLim)
            return dest;

        val = *src++;

        if (val < 0x80)
        {
            *dest++ = (char)val;
            continue;
        }

        if (val < _UTF8_RANGE(1))
        {
            dest[0] = _UTF8_HEAD(1, val);
            dest[1] = _UTF8_CHAR(0, val);
            dest += 2;
            continue;
        }

        if (val >= 0xD800 && val < 0xDC00 && src != srcLim)
        {
            UInt32 c2 = *src;
            if (c2 >= 0xDC00 && c2 < 0xE000)
            {
                src++;
                val = (((val - 0xD800) << 10) | (c2 - 0xDC00)) + 0x10000;
                dest[0] = _UTF8_HEAD(3, val);
                dest[1] = _UTF8_CHAR(2, val);
                dest[2] = _UTF8_CHAR(1, val);
                dest[3] = _UTF8_CHAR(0, val);
                dest += 4;
                continue;
            }
        }

        dest[0] = _UTF8_HEAD(2, val);
        dest[1] = _UTF8_CHAR(1, val);
        dest[2] = _UTF8_CHAR(0, val);
        dest += 3;
    }
}

static SRes Utf16_To_Utf8Buf(CBuf *dest, const UInt16 *src, size_t srcLen)
{
    size_t destLen = Utf16_To_Utf8_Calc(src, src + srcLen);
    destLen += 1;
    if (!Buf_EnsureSize(dest, destLen))
        return SZ_ERROR_MEM;
    *Utf16_To_Utf8(dest->data, src, src + srcLen) = 0;
    return SZ_OK;
}

static void GetAttribString(UInt32 wa, Bool isDir, char *s)
{
    s[0] = (char)(((wa & (1 << 4)) != 0 || isDir) ? 'D' : '.');
    s[1] = 0;
}

static void UInt64ToStr(UInt64 value, char *s)
{
    char temp[32];
    int pos = 0;
    do
    {
        temp[pos++] = (char)('0' + (unsigned)(value % 10));
        value /= 10;
    }
    while (value != 0);
    do
        *s++ = temp[--pos];
    while (pos);
    *s = '\0';
}

static SRes Utf16_To_Char(CBuf *buf, const UInt16 *s)
{
    unsigned len = 0;
    for (len = 0; s[len] != 0; len++);

    return Utf16_To_Utf8Buf(buf, s, len);
}

static char *UIntToStr(char *s, unsigned value, int numDigits)
{
    char temp[16];
    int pos = 0;
    do
        temp[pos++] = (char)('0' + (value % 10));
    while (value /= 10);
    for (numDigits -= pos; numDigits > 0; numDigits--)
        *s++ = '0';
    do
        *s++ = temp[--pos];
    while (pos);
    *s = '\0';
    return s;
}

static void UIntToStr_2(char *s, unsigned value)
{
    s[0] = (char)('0' + (value / 10));
    s[1] = (char)('0' + (value % 10));
}

static WRes OutFile_OpenUtf16(CSzFile *p, const UInt16 *name)
{
  CBuf buf;
  WRes res;
  Buf_Init(&buf);
  RINOK(Utf16_To_Char(&buf, name MY_FILE_CODE_PAGE_PARAM));
  printf("open file %s\n", (const char *)buf.data);
  res = OutFile_Open(p, (const char *)buf.data);
  Buf_Free(&buf, &g_Alloc);
  return res;
}

int create_dir(const char *path)
{
     int retval;
     __asm__ __volatile__ (
     "pushl $0 \n\t"
     "pushl $0 \n\t"
     "movl %1, 1(%%esp) \n\t"
     "pushl $0 \n\t"
     "pushl $0 \n\t"
     "pushl $0 \n\t"
     "pushl $0 \n\t"
     "pushl $9 \n\t"
     "movl %%esp, %%ebx \n\t"
     "movl $70, %%eax \n\t"
     "int $0x40 \n\t"
     "addl $28, %%esp \n\t"
     :"=a" (retval)
     :"r" (path)
     :"ebx");
  return retval;
};

static WRes MyCreateDir(const UInt16 *name)
{
  CBuf buf;
  WRes res;
  Buf_Init(&buf);
  RINOK(Utf16_To_Char(&buf, name MY_FILE_CODE_PAGE_PARAM));

  res = create_dir((const char *)buf.data) == 0 ? 0 : -1;
  Buf_Free(&buf, &g_Alloc);
  return res;
}

static SRes PrintString(const UInt16 *s)
{
    CBuf buf;
    SRes res;
    Buf_Init(&buf);
    res = Utf16_To_Char(&buf, s);
    if (res == SZ_OK)
        fputs((const char *)buf.data, stdout);
    Buf_Free(&buf, &g_Alloc);
    return res;
}

void PrintError(char *sz)
{
    printf("\nERROR: %s\n", sz);
}

static void ConvertFileTimeToString(const CNtfsFileTime *nt, char *s)
{
    unsigned year, mon, hour, min, sec;
    Byte ms[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    unsigned t;
    UInt32 v;
    UInt64 v64 = nt->Low | ((UInt64)nt->High << 32);
    v64 /= 10000000;
    sec = (unsigned)(v64 % 60); v64 /= 60;
    min = (unsigned)(v64 % 60); v64 /= 60;
    hour = (unsigned)(v64 % 24); v64 /= 24;

    v = (UInt32)v64;

    year = (unsigned)(1601 + v / PERIOD_400 * 400);
    v %= PERIOD_400;

    t = v / PERIOD_100; if (t ==  4) t =  3; year += t * 100; v -= t * PERIOD_100;
    t = v / PERIOD_4;   if (t == 25) t = 24; year += t * 4;   v -= t * PERIOD_4;
    t = v / 365;        if (t ==  4) t =  3; year += t;       v -= t * 365;

    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
        ms[1] = 29;
    for (mon = 0;; mon++)
    {
        unsigned s = ms[mon];
        if (v < s)
            break;
        v -= s;
    }
    s = UIntToStr(s, year, 4); *s++ = '-';
    UIntToStr_2(s, mon + 1); s[2] = '-'; s += 3;
    UIntToStr_2(s, (unsigned)v + 1); s[2] = ' '; s += 3;
    UIntToStr_2(s, hour); s[2] = ':'; s += 3;
    UIntToStr_2(s, min); s[2] = ':'; s += 3;
    UIntToStr_2(s, sec); s[2] = 0;
}


void do_7z_unpack(const char *srcpath)
{
    CFileInStream archiveStream;
    CLookToRead lookStream;

    CSzArEx db;
    SRes res;
    ISzAlloc allocImp;
    ISzAlloc allocTempImp;
    UInt16 *temp = NULL;
    size_t tempSize = 0;

    memset(&lookStream,0,sizeof(lookStream));

    allocImp.Alloc = SzAlloc;
    allocImp.Free = SzFree;

    allocTempImp.Alloc = SzAllocTemp;
    allocTempImp.Free = SzFreeTemp;

    if (InFile_Open(&archiveStream.file, srcpath))
        return;

    FileInStream_CreateVTable(&archiveStream);
    LookToRead_CreateVTable(&lookStream, False);

    lookStream.realStream = &archiveStream.s;
    LookToRead_Init(&lookStream);
    CrcGenerateTable();
    SzArEx_Init(&db);

    res = SzArEx_Open(&db, &lookStream.s, &allocImp, &allocTempImp);

    if (res == SZ_OK)
    {
        UInt32 i;
        UInt32 blockIndex    = 0xFFFFFFFF;  /* it can have any value before first call (if outBuffer = 0) */
        Byte *outBuffer      = 0;           /* it must be 0 before first call for each new archive. */
        size_t outBufferSize = 0;           /* it can have any value before first call (if outBuffer = 0) */

        for (i = 0; i < db.NumFiles; i++)
        {
            size_t offset = 0;
            size_t outSizeProcessed = 0;
            size_t len;
            unsigned isDir = SzArEx_IsDir(&db, i);

            if ( isDir )
                continue;

            len = SzArEx_GetFileNameUtf16(&db, i, NULL);

            if (len > tempSize)
            {
                SzFree(NULL, temp);
                tempSize = len;
                temp = (UInt16 *)SzAlloc(NULL, tempSize * sizeof(temp[0]));
                if (!temp)
                {
                    res = SZ_ERROR_MEM;
                    break;
                }
            }

            SzArEx_GetFileNameUtf16(&db, i, temp);
            res = PrintString(temp);
            if (res != SZ_OK)
                break;
            printf("\n");

            if (isDir)
                printf("/");
            else
            {
                res = SzArEx_Extract(&db, &lookStream.s, i,
                      &blockIndex, &outBuffer, &outBufferSize,
                      &offset, &outSizeProcessed,
                       &allocImp, &allocTempImp);
                if (res != SZ_OK)
                    break;
            }

            if (1)
            {
                CSzFile outFile;
                size_t processedSize;
                size_t j;
                UInt16 *name = (UInt16 *)temp;
                const UInt16 *destPath = (const UInt16 *)name;

                for (j = 0; name[j] != 0; j++)
                    if (name[j] == '/')
                {
                    name[j] = 0;
                    MyCreateDir(name);
                    name[j] = CHAR_PATH_SEPARATOR;
                }

                if (isDir)
                {
                    MyCreateDir(destPath);
                    printf("\n");
                    continue;
                }
                else if (OutFile_OpenUtf16(&outFile, destPath))
                {
                    PrintError("can not open output file");
                    res = SZ_ERROR_FAIL;
                    break;
                }

                processedSize = outSizeProcessed;

                if (File_Write(&outFile, outBuffer + offset, &processedSize) != 0 || processedSize != outSizeProcessed)
                {
                    PrintError("can not write output file\n");
                    res = SZ_ERROR_FAIL;
                    break;
                }

                if (File_Close(&outFile))
                {
                    PrintError("can not close output file\n");
                    res = SZ_ERROR_FAIL;
                    break;
                }
            };
        };
        IAlloc_Free(&allocImp, outBuffer);
    };
    SzArEx_Free(&db, &allocImp);
    SzFree(NULL, temp);

    File_Close(&archiveStream.file);
};

void do_install(list_t *install)
{
    package_t   *pkg, *tmp;
    char        *cache_path;

    list_for_each_entry_safe(pkg, tmp, install, list)
    {
        cache_path = make_cache_path(pkg->filename);

        sprintf(conbuf,"install package %s-%s\n", pkg->name, pkg->version);
        con_write_asciiz(conbuf);

        do_7z_unpack(cache_path);
        list_del_pkg(pkg);
    };
};

