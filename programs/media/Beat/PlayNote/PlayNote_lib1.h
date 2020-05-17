
/*
 * Author: JohnXenox aka Aleksandr Igorevich.
 *
 * PlayNote_lib1.h
*/

#ifndef __PlayNote_lib1_h__
#define __PlayNote_lib1_h__

static inline int initMemory(void)
{
    int size;

    __asm__ __volatile__("int $0x40":"=a"(size):"a"(68),"b"(11));
    return size;
}


static inline void makeDelay(int time)
{
    __asm__ __volatile__("int $0x40"::"a"(5), "b"(time):"memory");
}


static inline void *openFile(int *length, const char *path)
{
    int *fd;

    __asm__ __volatile__ ("int $0x40":"=a"(fd), "=d"(*length):"a" (68), "b"(27),"c"(path));

    return fd;
}



static inline void setCurrentPathToARawFile(char *dst_path, char *src_path, char* file_name)
{
    unsigned offset = 0;

    // cleans a dst path if not clean.
    if(dst_path[offset] != 0)
    {
        for(; dst_path[offset] != 0; offset++) dst_path[offset] = 0;
    }

    // copys current path into a buffer.
    strcpy(dst_path, src_path);

    offset = 0;

    // go to the end of a string.
    while(dst_path[offset] != 0) offset++;

    // clears all bytes to a character '/'.
    for(; dst_path[offset] != '/'; offset--) dst_path[offset] = 0;

    // increments a variable.
    offset++;

    // stores a name of a file in a buffer.
    strcpy(dst_path + offset, file_name);
}



void __attribute__ ((noinline)) printfOnADebugBoard(const char *format,...)
{
    va_list ap;
    char log_board[300];

    va_start (ap, format);
    tiny_vsnprintf(log_board, sizeof log_board, format, ap);
    va_end(ap);

    char *str = log_board;

    while(*str) __asm__ __volatile__("int $0x40"::"a"(63), "b"(1), "c"(*str++));
}










#endif







