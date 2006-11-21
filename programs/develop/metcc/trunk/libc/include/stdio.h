#ifndef stdio_h
#define stdio_h
#include "mesys.h"
#define NULL (void*)0
typedef struct {
  char* buffer;
  int   buffersize;
  int   filesize;
  int   filepos;
  char* filename;
  int   mode;
} FILE;
#define FILE_OPEN_READ 0
#define FILE_OPEN_WRITE 1
#define FILE_OPEN_APPEND 2
#define FILE_OPEN_TEXT 4
#define FILE_OPEN_PLUS 8
#define EOF -1
extern FILE* fopen(const char* filename, const char *mode);
extern int fclose(FILE* file);
extern int feof(FILE* file);
extern int fflush(FILE* file);
extern int fgetc(FILE* file);
typedef int fpos_t;
extern int fgetpos(FILE* file,fpos_t* pos);
extern int fsetpos(FILE* file,const fpos_t* pos);
extern int fputc(int c,FILE* file);
extern int fread(void* buffer,int size,int count,FILE* file);
extern int fwrite(const void* buffer,int size,int count,FILE* file);
extern long ftell(FILE* file);
#define SEEK_CUR 0
#define SEEK_END 1
#define SEEK_SET 2
extern int fseek(FILE* file,long offset,int origin);
extern void rewind(FILE* file);
extern int fprintf(FILE* file, const char* format, ...);
extern int fscanf(FILE* file,const char* format, ...);
extern int ungetc(int c,FILE* file);
#endif
