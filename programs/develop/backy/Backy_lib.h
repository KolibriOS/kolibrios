
/*
 * Backy_lib.h
 * Author: JohnXenox aka Aleksandr Igorevich.
 */

#ifndef __Backy_lib_h__
#define __Backy_lib_h__

typedef  unsigned int        uint32_t;
typedef  unsigned char       uint8_t;
typedef  unsigned short int  uint16_t;
typedef  unsigned long long  uint64_t;
typedef  char                int8_t;
typedef  short int           int16_t;
typedef  int                 int32_t;
typedef  long long           int64_t;

static inline uint32_t getDate(void)
{
    uint32_t date;
    __asm__ __volatile__("int $0x40":"=a"(date):"a"(29));
    return date;
}



static inline uint32_t getTime(void)
{
    uint32_t time;
    __asm__ __volatile__("int $0x40":"=a"(time):"a"(3));
    return time;
}



static inline void *openFile(uint32_t *length, const uint8_t *path)
{
    uint8_t *fd;

    __asm__ __volatile__ ("int $0x40":"=a"(fd), "=d"(*length):"a" (68), "b"(27),"c"(path));

    return fd;
}



static inline int32_t saveFile(uint32_t nbytes, uint8_t *data, uint32_t enc, uint8_t  *path)
{
    int32_t val;

    uint8_t dt[28];  // basic information structure.

    (uint32_t)  dt[0]  = 2;       // subfunction number.
    (uint32_t)  dt[4]  = 0;       // reserved.
    (uint32_t)  dt[8]  = 0;       // reserved.
    (uint32_t)  dt[12] = nbytes;  // number of bytes to write.
    (uint8_t *) dt[16] = data;    // pointer to data.
    (uint32_t)  dt[20] = enc;     // string encoding (0 = default, 1 = cp866, 2 = UTF-16LE, 3 = UTF-8).
    (uint8_t *) dt[24] = path;    // pointer to path.

    __asm__ __volatile__("int $0x40":"=a"(val):"a"(80), "b"(&dt));

    return val;
}





int checkStateOnSave(int state)
{
    #if defined (lang_en)

        switch(state)
        {
            case 2:  con_printf("\nThe function isn't supported for the given file system!\n");
                     return 2;

            case 3:  con_printf("\nUnknown file system!\n");
                     return 3;

            case 5:  con_printf("\nFile isn't found!\n");
                     return 5;

            case 6:  con_printf("\nEnd of a file, EOF!\n");
                     return 6;

            case 7:  con_printf("\nPointer lies outside of application memory!\n");
                     return 7;

            case 8:  con_printf("\nDisk is full!\n");
                     return 8;

            case 9:  con_printf("\nFile system error!\n");
                     return 9;

            case 10: con_printf("\nAccess denied!\n");
                     return 10;

            case 11: con_printf("\nDevice error!\n");
                     return 11;

            case 12: con_printf("\nFile system requires more memory!\n");
                     return 12;
        }

    #elif defined (lang_ru)

        switch(state)
        {
            case 2:  con_printf("\nФункция не поддерживается для данной файловой системы!\n");
                     return 2;

            case 3:  con_printf("\nНеизвестная файловая система!\n");
                     return 3;

            case 5:  con_printf("\nФайл не найден!\n");
                     return 5;

            case 6:  con_printf("\nФайл закончился!\n");
                     return 6;

            case 7:  con_printf("\nУказатель вне памяти приложения!\n");
                     return 7;

            case 8:  con_printf("\nДиск заполнен!\n");
                     return 8;

            case 9:  con_printf("\nОшибка файловой системы!\n");
                     return 9;

            case 10: con_printf("\nДоступ запрещён!\n");
                     return 10;

            case 11: con_printf("\nОшибка устройства!\n");
                     return 11;

            case 12: con_printf("\nФайловой системе недостаточно оперативной памяти!\n");
                     return 12;
        }

    #endif
}







#endif
