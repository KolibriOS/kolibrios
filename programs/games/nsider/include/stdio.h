#ifndef stdio_h
#define stdio_h

#include "kolibrisys.h"

typedef char *va_list;
#define _roundsize(n)    ( (sizeof(n) + 3) & ~3 )
#define va_start(ap,v) (ap = (va_list)&v+_roundsize(v))
#define va_arg(ap,t)    ( *(t *)((ap += _roundsize(t)) - _roundsize(t)) )
#define va_end(ap) (ap = (va_list)0)

#define NULL ((void*)0)
//extern int stdcall format_print(char *dest, size_t maxlen, const char *fmt0, va_list argp);

typedef struct {
  char*   buffer;
  dword   buffersize;
  dword   filesize;
  dword   filepos;
  char*   filename;
  int     mode;
} FILE;

#define FILE_OPEN_READ 0
#define FILE_OPEN_WRITE 1
#define FILE_OPEN_APPEND 2
#define FILE_OPEN_TEXT 4
#define FILE_OPEN_PLUS 8
#define EOF -1

extern FILE* fopen(const char* filename, const char *mode);
extern void fclose(FILE* file);
extern int feof(FILE* file);
extern int fflush(FILE* file);
extern int fgetc(FILE* file);
extern int fgetpos(FILE* file,fpos_t* pos);
extern int fsetpos(FILE* file,const fpos_t* pos);
extern int fputc(int c,FILE* file);
extern int fread(void* buffer,int size,int count,FILE* file);
extern int fwrite(void *buffer,int size,int count,FILE* file);
extern long ftell(FILE* file);
#define SEEK_CUR 0
#define SEEK_END 1
#define SEEK_SET 2
extern int fseek(FILE* file,long offset,int origin);
extern void rewind(FILE* file);
extern int cdecl fprintf(FILE* file, const char* format,...);
extern int fscanf(FILE* file,const char* format,...);
extern int ungetc(int c,FILE* file);

extern int cdecl printf(const char *format,...);

extern int vsnprintf(char *dest, size_t size,const char *format,va_list ap);
extern int cdecl snprintf(char *dest, size_t size, const char *format,...);
extern int cdecl sprintf(char *dest,const char *format,...);

#endif
