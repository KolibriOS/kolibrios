
/*
 * Backy_lib.h
 * Author: JohnXenox aka Aleksandr Igorevich.
 */

#ifndef __jxl_Date_get_h__
#define __jxl_Date_get_h__

#ifdef ___TINY_C___

typedef  unsigned int        uint32_t;
typedef  unsigned char       uint8_t;
typedef  unsigned short int  uint16_t;
typedef  unsigned long long  uint64_t;
typedef  char                int8_t;
typedef  short int           int16_t;
typedef  int                 int32_t;
typedef  long long           int64_t;

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

#else

#include <stdint.h>

static inline int32_t saveFile(uint32_t nbytes, uint8_t *data, uint32_t enc, uint8_t  *path)
{
    int32_t val;

    struct file_op_t
    {
        uint32_t    fn;
        uint32_t    reserved0;
        uint32_t    reserved1;
        uint32_t    number_of_bytes_to_write;
        void *      pointer_to_data;
        char        path[1];
    } *file_op = calloc(sizeof(*file_op) + strlen(path) + 2, 1); // FIXME: Not works on UTF16LE enc
    
    file_op->fn = 2;
    file_op->number_of_bytes_to_write = nbytes;
    file_op->pointer_to_data = data;
    if (enc != 0)
    {
        file_op->path[0] = enc;
        strcpy(file_op->path + 1, path);
    }
    else
    {
        strcpy(file_op->path, path);
    }
    
    asm volatile ("int $0x40":"=a"(val):"a"(80), "b"(file_op));
    return val;
}

#endif

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

#endif
