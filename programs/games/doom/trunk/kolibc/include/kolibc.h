

#ifndef kolibc_h
#define kolibc_h

#ifdef __cplusplus
extern "C"
{
#endif

typedef unsigned char byte;
typedef unsigned short int word;
typedef unsigned int dword;

typedef unsigned int fpos_t;
typedef unsigned int size_t;

#//define NULL (void*)0

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define FILE_OPEN_READ    0x01
#define FILE_OPEN_WRITE   0x02
#define FILE_OPEN_APPEND  0x04
#define FILE_OPEN_TEXT    0x08
#define FILE_OPEN_PLUS    0x10
#define EOF -1

typedef struct
{
  char   *buffer;
  char   *stream;
  size_t    strpos;
  size_t    remain;

  size_t filepos;
  
  size_t buffersize;
  size_t filesize;
  char*  filename;
  int    mode;
} FILE;

extern FILE* fopen(const char* filename, const char *mode);
extern int fclose(FILE* file);
extern int feof(FILE* file);
extern int fflush(FILE* file);
extern int fgetc(FILE* file);
extern int fgetpos(FILE* file,fpos_t* pos);
extern int fsetpos(FILE* file,const fpos_t* pos);
extern int fputc(int c,FILE* file);
extern int fread(void* buffer,size_t size,size_t count,FILE* file);
extern int fwrite(const void* buffer,size_t size,size_t count,FILE* file);
extern long ftell(FILE* file);
extern int fseek(FILE* file,long offset,int origin);
extern void rewind(FILE* file);
extern int fprintf(FILE* file, const char* format, ...);
extern int fscanf(FILE* file,const char* format, ...);
extern int ungetc(int c,FILE* file);

extern int sprintf(char *dest, const char *format,...);
extern int printf(const char *format,...);

typedef char *va_list;
#define _roundsize(n)    ( (sizeof(n) + 3) & ~3 )
#define va_start(ap,v) (ap = (va_list)&v+_roundsize(v))
#define va_arg(ap,t)    ( *(t *)((ap += _roundsize(t)) - _roundsize(t)) )
#define va_end(ap) (ap = (va_list)0)

/*
** All character classification functions except isascii().
** Integer argument (c) must be in ASCII range (0-127) for
** dependable answers.
*/

#define ALNUM     1
#define ALPHA     2
#define CNTRL     4
#define DIGIT     8
#define GRAPH    16
#define LOWER    32
#define PRINT    64
#define PUNCT   128
#define BLANK   256
#define UPPER   512
#define XDIGIT 1024

extern short int _is[128];

#define isalnum(c)(_is[c] & ALNUM ) /* 'a'-'z', 'A'-'Z', '0'-'9' */
#define isalpha(c)(_is[c] & ALPHA ) /* 'a'-'z', 'A'-'Z' */
#define iscntrl(c)(_is[c] & CNTRL ) /* 0-31, 127 */
#define isdigit(c)(_is[c] & DIGIT ) /* '0'-'9' */
#define isgraph(c)(_is[c] & GRAPH ) /* '!'-'~' */
#define islower(c)(_is[c] & LOWER ) /* 'a'-'z' */
#define isprint(c)(_is[c] & PRINT ) /* ' '-'~' */
#define ispunct(c)(_is[c] & PUNCT ) /* !alnum && !cntrl && !space */
#define isspace(c)(_is[c] & BLANK ) /* HT, LF, VT, FF, CR, ' ' */
#define isupper(c)(_is[c] & UPPER ) /* 'A'-'Z' */
#define isxdigit(c)(_is[c] & XDIGIT) /* '0'-'9', 'a'-'f', 'A'-'F' */


#define _LOWER  0x80
#define _UPPER  0x40
#define _DIGIT  0x20
#define _XDIGT  0x10
#define _PRINT  0x08
#define _PUNCT  0x04
#define _SPACE  0x02
#define _CNTRL  0x01

#define abs(i) (((i)<0)?(-(i)):(i))

#if 0
extern int atoib(char *s,int b);
extern int atoi(char *s);
extern char tolower(char c);
extern char toupper(char c);
extern void itoab(int n,char* s,int  b);
extern void itoa(int n,char* s);

extern char* strcat(char*,const char*);
extern char* strchr(const char*,int);
extern int strcmp(const char*,const char*);
extern int strcoll(const char*,const char*);
extern char* strcpy(char*,const char*);
extern int strcspn(const char*,const char*);
extern size_t strlen(const char*);
extern char* strncat(char*,const char*,int);
extern int strncmp(const char*,const char*,int);
extern char* strncpy(char*,const char*,int);
extern char* strpbrk(const char*,const char*);
extern char* strrchr(const char*,int);
extern int strspn(const char*,const char*);
extern char* strstr(const char*,const char*);
extern char* strtok(char*,const char*);
extern int strxfrm(char*,const char*,int);
extern char* strdup(const char*);
extern char toupper(char c);
#define isascii(char)   ( (unsigned)char < 0x80 )

extern void* memchr(const void*,int,int);
extern extern void* memchr(const void*,int,int);
extern int memcmp(const void*,const void*,int);
extern void* memcpy(void*,const void*,size_t);
void kmemset (void *dst, int val, size_t count);

extern void* memmove(void*,const void*,int);
extern void* memset(void*,int,int);
int memcmp(const void*,const void*,int);
extern void* memcpy(void*,const void*,size_t);
void kmemset (void *dst, int val, size_t count);

extern void* memmove(void*,const void*,int);
extern void* memset(void*,int,int);

#endif

void debug_out_str(char* str);

void* _cdecl dlmalloc(size_t size);
void* _cdecl dlrealloc(void* oldmem, size_t bytes);
void  _cdecl dlfree(void *mem);

//double pow_test(x,y);


#ifdef __cplusplus
extern "C"
}
#endif

#endif  //kolibc_h


