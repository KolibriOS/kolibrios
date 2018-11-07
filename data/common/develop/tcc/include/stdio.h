#ifndef stdio_h
#define stdio_h

#include "kolibrisys.h"
#include <stdarg.h>
/* use stdarg.h
typedef char *va_list;
#define _roundsize(n)    ( (sizeof(n) + 3) & ~3 )
#define va_start(ap,v) (ap = (va_list)&v+_roundsize(v))
#define va_arg(ap,t)    ( *(t *)((ap += _roundsize(t)) - _roundsize(t)) )
#define va_end(ap) (ap = (va_list)0)
*/
#ifndef NULL
# define NULL ((void*)0)
#endif

typedef unsigned int fpos_t;  // 32bit is not enough! 4Gb limit
typedef unsigned int size_t;

int format_print(char *dest, size_t maxlen,const char *fmt0, va_list argp);

typedef struct {
  char*   buffer;
  dword   buffersize;
  dword   filesize;     // too small
  int     filepos;      // too small, may be -1
  char*   filename;
  int     mode;
  int	  ungetc_buf;
  dword   buffer_start;  // 1st byte position
  dword   buffer_end;    // points after last buffered data
} FILE;

#define stderr ((FILE*)3) /* works only for fprintf!!! */


#define FILE_OPEN_READ 0
#define FILE_OPEN_WRITE 1
#define FILE_OPEN_APPEND 2
#define FILE_OPEN_TEXT 4
#define FILE_OPEN_PLUS 8
#define EOF (-1)
#define BUFSIZ (4096)
#define FILENAME_MAX (0x400)

extern FILE* fopen(const char* filename, const char *mode);
extern int fclose(FILE* file);
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

#define getc(a) fgetc(a)
#define putc(a, b) fputc(a, b)
char * fgets (char * str, int num, FILE * stream);
int putchar (int ch);
int getchar (void);
int puts (const char * str);
char * gets (char * str);

typedef int (*virtual_getc)(void *sp, const void *obj);
typedef void (*virtual_ungetc)(void *sp, int c, const void *obj);
int format_scan(const void *src, const char *fmt, va_list argp, virtual_getc vgetc, virtual_ungetc vungetc);
int vscanf ( const char * format, va_list arg );
int scanf ( const char * format, ...);
int vsscanf ( const char * s, const char * format, va_list arg );
int sscanf ( const char * s, const char * format, ...);
int vfscanf ( FILE * stream, const char * format, va_list arg );
int fputs ( const char * str, FILE * file );
void clearerr ( FILE * stream );
int ferror ( FILE * stream );
void perror ( const char * str );
int vprintf ( const char * format, va_list arg );
int vsprintf (char * s, const char * format, va_list arg );
int vfprintf ( FILE * stream, const char * format, va_list arg );


int tiny_sprintf (char * s, const char * format, ... );
int tiny_snprintf (char * s, size_t n, const char * format, ... );
int tiny_vsnprintf (char * s, size_t n, const char * format, va_list args );
// support %c, %s, %d, %x, %u, %% for 32-bit values only. no width specs, left align
// always zero-ended

extern int errno;
/* errors codes from KOS, but minus */
#ifndef E_SUCCESS

# define E_SUCCESS (0)
# define E_UNSUPPORTED (-2)
# define E_UNKNOWNFS  (-3)
# define E_NOTFOUND (-5)
# define E_EOF  (-6)
# define E_INVALIDPTR (-7)
# define E_DISKFULL  (-8)
# define E_FSYSERROR  (-9)
# define E_ACCESS  (-10)
# define E_HARDWARE  (-11)
# define E_NOMEM  (-12)
/* conversion errors */
# define ERANGE (-20)
# define EINVAL (-21)
/* program run and pipe errors */
# define E_NOMEM2 (-30)
# define E_FILEFMT (-31)
# define E_TOOMANY (-32)
# define E_PARAM (-33)
#endif

#endif
