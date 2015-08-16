#include <stdio.h>
#include <string.h>
#include "7z.h"
#include "7zAlloc.h"
#include "7zBuf.h"
#include "7zCrc.h"
#include "7zFile.h"
#include "7zVersion.h"

static ISzAlloc g_Alloc = { SzAlloc, SzFree };

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
