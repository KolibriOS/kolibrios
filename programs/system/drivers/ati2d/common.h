
#define OS_BASE   0x80000000

#include "xmd.h"

#define NULL (void*)(0)

#define FALSE   0
#define TRUE    1

typedef void *pointer;

typedef unsigned int   Bool;

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef unsigned int memType;
typedef unsigned int size_t;

typedef struct { float hi, lo; } range;

typedef struct
{
  unsigned      handle;
  unsigned      io_code;
  void          *input;
  int           inp_size;
  void          *output;
  int           out_size;
}ioctl_t;

typedef int (_stdcall *srv_proc_t)(ioctl_t *);

u32 __stdcall drvEntry(int)__asm__("_drvEntry");

///////////////////////////////////////////////////////////////////////////////

#define STDCALL __attribute__ ((stdcall)) __attribute__ ((dllimport))
#define IMPORT __attribute__ ((dllimport))

///////////////////////////////////////////////////////////////////////////////

#define PG_SW       0x003
#define PG_NOCACHE  0x018

CARD32 STDCALL AllocKernelSpace(unsigned size)__asm__("AllocKernelSpace");
void*  STDCALL KernelAlloc(unsigned size)__asm__("KernelAlloc");
int KernelFree(void *);

void* STDCALL CreateRingBuffer(size_t size, u32 map)__asm__("CreateRingBuffer");

u32 STDCALL RegService(char *name, srv_proc_t proc)__asm__("RegService");

//void *CreateObject(u32 pid, size_t size);
//void *DestroyObject(void *obj);

CARD32 STDCALL MapIoMem(CARD32 Base,CARD32 size,CARD32 flags)__asm__("MapIoMem");

static inline u32 GetPgAddr(void *mem)
{
  u32 retval;

	asm volatile (
    "call *__imp__GetPgAddr \n\t"
    :"=eax" (retval)
    :"a" (mem)
	);
  return retval;
}

///////////////////////////////////////////////////////////////////////////////

u32 PciApi(int cmd);

u8  STDCALL PciRead8 (u32 bus, u32 devfn, u32 reg)__asm__("PciRead8");
u16 STDCALL PciRead16(u32 bus, u32 devfn, u32 reg)__asm__("PciRead16");
u32 STDCALL PciRead32(u32 bus, u32 devfn, u32 reg)__asm__("PciRead32");

u32 STDCALL PciWrite8 (u32 bus, u32 devfn, u32 reg,u8 val) __asm__("PciWrite8");
u32 STDCALL PciWrite16(u32 bus, u32 devfn, u32 reg,u16 val)__asm__("PciWrite16");
u32 STDCALL PciWrite32(u32 bus, u32 devfn, u32 reg,u32 val)__asm__("PciWrite32");

///////////////////////////////////////////////////////////////////////////////

#define SysMsgBoardStr  __SysMsgBoardStr
#define PciApi          __PciApi
//#define RegService      __RegService
#define CreateObject    __CreateObject
#define DestroyObject   __DestroyObject

///////////////////////////////////////////////////////////////////////////////

void *malloc(size_t);
void *calloc( size_t num, size_t size );
void *realloc(void*, size_t);
void free(void*);

#define kmalloc malloc
#define kfree   free

///////////////////////////////////////////////////////////////////////////////

int memcmp(const void *s1, const void *s2, size_t n);
void * memcpy(void * _dest, const void *_src, size_t _n);
char * strcpy(char *to, const char *from);
char * strcat(char *s, const char *append);
int strcmp(const char *s1, const char *s2);
size_t strlen(const char *str);
char * strdup(const char *_s);
char * strchr(const char *s, int c);

///////////////////////////////////////////////////////////////////////////////

int snprintf(char *s, size_t n, const char *format, ...);
int printf(const char* format, ...);
int dbg_open(char *path);
int dbgprintf(const char* format, ...);

///////////////////////////////////////////////////////////////////////////////


void usleep(u32 delay);

static int __attribute__ ((always_inline))
abs (int i)
{
  return i < 0 ? -i : i;
};

static void __attribute__ ((always_inline))
__clear (void * dst, unsigned len)
{ u32 tmp;
  asm __volatile__
  (
    "xorl %%eax, %%eax \n\t"
    "cld \n\t"
    "rep stosb \n"
    :"=c"(tmp),"=D"(tmp)
    :"c"(len),"D"(dst)
    :"memory","eax","cc"
  );
};


static inline u32 safe_cli(void)
{
  u32 tmp;
	asm volatile (
		"pushf\n\t"
		"popl %0\n\t"
		"cli\n"
    : "=r" (tmp)
	);
  return tmp;
}

static inline void safe_sti(u32 ipl)
{
	asm volatile (
		"pushl %0\n\t"
		"popf\n"
		: : "r" (ipl)
	);
}

///////////////////////////////////////////////////////////////////////////////

int _stdcall srv_cursor(ioctl_t *io);
int _stdcall srv_2d(ioctl_t *io);


