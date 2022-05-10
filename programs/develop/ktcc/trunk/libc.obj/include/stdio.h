///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2019, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// \brief Tiny printf, sprintf and snprintf implementation, optimized for speed on
//        embedded systems with a very limited resources.
//        Use this instead of bloated standard/newlib printf.
//        These routines are thread safe and reentrant.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _STDIO_H_
#define _STDIO_H_

#include <stdarg.h>
#include <stddef.h>

DLLAPI int puts(const char* str);
DLLAPI int printf(const char* format, ...);
DLLAPI int sprintf(char* buffer, const char* format, ...);
DLLAPI int snprintf(char* buffer, size_t count, const char* format, ...);
DLLAPI int vsnprintf(char* buffer, size_t count, const char* format, va_list va);
DLLAPI int vsprintf(char* buffer, const char* format, va_list va);
DLLAPI int vprintf(const char* format, va_list va);

DLLAPI void debug_printf(const char* format, ...);

typedef size_t fpos_t;

#define _FILEMODE_R 1 << 0    // Read
#define _FILEMODE_W 1 << 1    // Write
#define _FILEMODE_A 1 << 2    // Append
#define _FILEMODE_PLUS 1 << 3 // Plus

typedef struct FILE_s {
    char* name;
    fpos_t position;
    int error;
    int eof;
    int mode;              // flags _FILEMODE_*
    int __ungetc_emu_buff; // Uses __ungetc_emu (temporary solution!)
} FILE;

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

#define BUFSIZ 1024

#define EOF -1

#define FOPEN_MAX 0xffffffff

#define FILENAME_MAX 255

#define L_tmpnam FILENAME_MAX

#define SEEK_CUR 0
#define SEEK_END 1
#define SEEK_SET 2

#define TMP_MAX FOPEN_MAX

#define stderr (FILE*)3
#define stdin (FILE*)1
#define stdout (FILE*)2

DLLAPI int fgetc(FILE*);
DLLAPI char* fgets(char* restrict, int, FILE* restrict);
DLLAPI int fprintf(FILE* restrict, const char* restrict, ...);
DLLAPI int fputc(int, FILE*);
DLLAPI int fputs(const char* restrict, FILE* restrict);
DLLAPI size_t fread(void* restrict, size_t size, size_t count, FILE* restrict);
DLLAPI int fscanf(FILE* restrict, const char* restrict, ...);
DLLAPI size_t fwrite(const void* restrict, size_t size, size_t count, FILE* restrict);
DLLAPI int getc(FILE*);
#define getc() fgetc(stdin)
DLLAPI int getchar(void);
DLLAPI int printf(const char* restrict, ...);
#define putc(ch) fputc(ch, stdout)
DLLAPI int puts(const char*);
DLLAPI int scanf(const char* restrict, ...);
DLLAPI char* gets(char* str);
DLLAPI int ungetc(int, FILE*);
DLLAPI int vfprintf(FILE* restrict, const char* restrict, va_list);
DLLAPI int vfscanf(FILE* restrict, const char* restrict, va_list);
DLLAPI int vprintf(const char* restrict, va_list);
DLLAPI int vscanf(const char* restrict, va_list);
DLLAPI int sscanf(const char*, const char* restrict, ...);
DLLAPI int vsscanf(const char*, const char*, va_list);

DLLAPI int remove(const char*);
DLLAPI int rename(const char*, const char*);
DLLAPI FILE* tmpfile(void);
DLLAPI char* tmpnam(char*);

DLLAPI int fclose(FILE*);
DLLAPI int fflush(FILE*);
DLLAPI FILE* fopen(const char* restrict, const char* restrict);
DLLAPI FILE* freopen(const char* restrict, const char* restrict, FILE* restrict);
DLLAPI void setbuf(FILE* restrict, char* restrict);
DLLAPI int setvbuf(FILE* restrict, char* restrict, int, size_t);

DLLAPI int fgetpos(FILE* restrict, fpos_t* restrict);
DLLAPI int fseek(FILE*, long, int);
DLLAPI int fsetpos(FILE*, const fpos_t*);
DLLAPI long ftell(FILE*);
DLLAPI void rewind(FILE*);

DLLAPI void clearerr(FILE*);
DLLAPI int feof(FILE*);
DLLAPI int ferror(FILE*);
DLLAPI void perror(const char*);

DLLAPI size_t fread(void* restrict, size_t, size_t, FILE* restrict);

DLLAPI int getchar(void);

#endif // _STDIO_H_
