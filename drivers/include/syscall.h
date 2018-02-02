#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <ddk.h>

typedef u32 addr_t;
typedef u32 count_t;

typedef struct
{
  int width;
  int height;
  int bpp;
  int freq;
}videomode_t;

struct kos32_pdev
{
    struct kos32_pdev *prev;
    struct kos32_pdev *next;
    u32 devid;
    u32 class;
    u8  devfn;
    u8  bus;
    u8  reserved[2];
    u32 owner;
} __attribute__((packed));

///////////////////////////////////////////////////////////////////////////////

#define STDCALL  __attribute__ ((stdcall)) __attribute__ ((dllimport))
#define FASTCALL __attribute__ ((fastcall)) __attribute__ ((dllimport))

#define IMPORT   __attribute__ ((dllimport))

///////////////////////////////////////////////////////////////////////////////

#define SysMsgBoardStr  __SysMsgBoardStr
#define PciApi          __PciApi
#define CreateObject    __CreateObject
#define DestroyObject   __DestroyObject

#define _alloca(x) __builtin_alloca((x))

///////////////////////////////////////////////////////////////////////////////


void*  STDCALL AllocKernelSpace(size_t size)__asm__("AllocKernelSpace");
void   STDCALL FreeKernelSpace(void *mem)__asm__("FreeKernelSpace");
addr_t STDCALL MapIoMem(addr_t base, size_t size, u32 flags)__asm__("MapIoMem");
void*  STDCALL KernelAlloc(size_t size)__asm__("KernelAlloc");
void*  STDCALL KernelFree(const void *mem)__asm__("KernelFree");
void*  STDCALL UserAlloc(size_t size)__asm__("UserAlloc");
int    STDCALL UserFree(void *mem)__asm__("UserFree");

void*  STDCALL GetDisplay(void)__asm__("GetDisplay");

u32  IMPORT  GetTimerTicks(void)__asm__("GetTimerTicks");
u64  IMPORT  GetClockNs(void)__asm__("GetClockNs");

addr_t STDCALL AllocPage(void)__asm__("AllocPage");
addr_t STDCALL AllocPages(count_t count)__asm__("AllocPages");
void   IMPORT  __attribute__((regparm(1)))
               FreePage(addr_t page)__asm__("FreePage");
void   STDCALL MapPage(void *vaddr, addr_t paddr, u32 flags)__asm__("MapPage");


void* STDCALL CreateRingBuffer(size_t size, u32 map)__asm__("CreateRingBuffer");

u32 STDCALL RegService(char *name, srv_proc_t proc)__asm__("RegService");

int   STDCALL AttachIntHandler(int irq, void *handler, u32 access) __asm__("AttachIntHandler");

void  FASTCALL MutexInit(struct mutex*)__asm__("MutexInit");
void  FASTCALL MutexLock(struct mutex*)__asm__("MutexLock");
void  FASTCALL MutexUnlock(struct mutex*)__asm__("MutexUnlock");

void  FASTCALL InitRwsem(struct rw_semaphore *sem)__asm__("InitRwsem");
void  FASTCALL DownRead(struct rw_semaphore *sem)__asm__("DownRead");
void  FASTCALL DownWrite(struct rw_semaphore *sem)__asm__("DownWrite");
void  FASTCALL UpRead(struct rw_semaphore *sem)__asm__("UpRead");
void  FASTCALL UpWrite(struct rw_semaphore *sem)__asm__("UpWrite");

addr_t IMPORT  GetStackBase(void)__asm__("GetStackBase");
u32  IMPORT  GetPid(void)__asm__("GetPid");

u32 STDCALL TimerHS(u32 delay, u32 interval,
                    void *fn, void *data)asm("TimerHS");

void STDCALL CancelTimerHS(u32 handle)asm("CancelTimerHS");

u64 IMPORT GetCpuFreq()__asm__("GetCpuFreq");

///////////////////////////////////////////////////////////////////////////////

void   STDCALL SetMouseData(int btn, int x, int y,
                            int z, int h)__asm__("SetMouseData");

void   FASTCALL SetKeyboardData(u32 data)__asm__("SetKeyboardData");

struct kos32_pdev* IMPORT GetPCIList()__asm__("GetPCIList");

u8  STDCALL PciRead8 (u32 bus, u32 devfn, u32 reg)__asm__("PciRead8");
u16 STDCALL PciRead16(u32 bus, u32 devfn, u32 reg)__asm__("PciRead16");
u32 STDCALL PciRead32(u32 bus, u32 devfn, u32 reg)__asm__("PciRead32");

u32 STDCALL PciWrite8 (u32 bus, u32 devfn, u32 reg,u8 val) __asm__("PciWrite8");
u32 STDCALL PciWrite16(u32 bus, u32 devfn, u32 reg,u16 val)__asm__("PciWrite16");
u32 STDCALL PciWrite32(u32 bus, u32 devfn, u32 reg,u32 val)__asm__("PciWrite32");

#define pciReadByte(tag, reg) \
        PciRead8(PCI_BUS_FROM_TAG(tag),PCI_DFN_FROM_TAG(tag),(reg))

#define pciReadWord(tag, reg) \
        PciRead16(PCI_BUS_FROM_TAG(tag),PCI_DFN_FROM_TAG(tag),(reg))

#define pciReadLong(tag, reg) \
        PciRead32(PCI_BUS_FROM_TAG(tag),PCI_DFN_FROM_TAG(tag),(reg))

#define pciWriteByte(tag, reg, val) \
        PciWrite8(PCI_BUS_FROM_TAG(tag),PCI_DFN_FROM_TAG(tag),(reg),(val))

#define pciWriteWord(tag, reg, val) \
        PciWrite16(PCI_BUS_FROM_TAG(tag),PCI_DFN_FROM_TAG(tag),(reg),(val))

#define pciWriteLong(tag, reg, val) \
        PciWrite32(PCI_BUS_FROM_TAG(tag),PCI_DFN_FROM_TAG(tag),(reg),(val))


///////////////////////////////////////////////////////////////////////////////

int dbg_open(char *path);
int dbgprintf(const char* format, ...);

///////////////////////////////////////////////////////////////////////////////

static inline int CreateKernelThread(void *entry)
{
    int pid;
     __asm__ __volatile__ (
     "call *__imp__CreateThread"
     :"=a"(pid)
     :"b"(1),"c"(entry),"d"(0)
     :"memory");
     __asm__ __volatile__ ("":::"ebx","ecx","edx","esi","edi");
     return pid;
};


static inline evhandle_t CreateEvent(kevent_t *ev, u32 flags)
{
     evhandle_t evh;

     __asm__ __volatile__ (
     "call *__imp__CreateEvent"
     :"=A"(evh.raw)
     :"S" (ev), "c"(flags)
     :"memory");
     __asm__ __volatile__ ("":::"ebx","ecx","edx","esi", "edi");

     return evh;
};

static inline void RaiseEvent(evhandle_t evh, u32 flags, kevent_t *ev)
{
     __asm__ __volatile__ (
     "call *__imp__RaiseEvent"
     ::"a"(evh.handle),"b"(evh.euid),"d"(flags),"S" (ev)
     :"memory");
     __asm__ __volatile__ ("":::"ebx","ecx","edx","esi","edi");

};

static inline void WaitEvent(evhandle_t evh)
{
     __asm__ __volatile__ (
     "call *__imp__WaitEvent"
     ::"a"(evh.handle),"b"(evh.euid));
     __asm__ __volatile__ ("":::"ebx","ecx","edx","esi","edi");
};

static inline int WaitEventTimeout(evhandle_t evh, int timeout)
{
    int retval;
    __asm__ __volatile__ (
    "call *__imp__WaitEventTimeout"
    :"=a"(retval)
    :"a"(evh.handle),"b"(evh.euid), "c"(timeout));
    __asm__ __volatile__ ("":::"ebx","ecx","edx","esi","edi");
    return retval;
};

static inline void DestroyEvent(evhandle_t evh)
{
     __asm__ __volatile__ (
     "call *__imp__DestroyEvent"
     ::"a"(evh.handle),"b"(evh.euid));
     __asm__ __volatile__ ("":::"ebx","ecx","edx","esi","edi");
};

static inline u32 GetEvent(kevent_t *ev)
{
    u32  handle;

    __asm__ __volatile__ (
    "call *__imp__GetEvent"
    :"=a"(handle)
    :"D"(ev)
    :"memory");
    __asm__ __volatile__ ("":::"ebx","ecx","edx", "esi","edi");
     return handle;
};


static inline int GetScreenSize(void)
{
  int retval;

  asm("int $0x40"
      :"=a"(retval)
      :"a"(61), "b"(1));
  return retval;
}

static inline int GetScreenBpp(void)
{
  int retval;

  asm("int $0x40"
      :"=a"(retval)
      :"a"(61), "b"(2));
  return retval;
}

static inline int GetScreenPitch(void)
{
  int retval;

  asm("int $0x40"
      :"=a"(retval)
      :"a"(61), "b"(3));
  return retval;
}

static inline u32 GetPgAddr(void *mem)
{
     u32 retval;

     __asm__ __volatile__ (
     "call *__imp__GetPgAddr \n\t"
     :"=a" (retval)
     :"a" (mem) );
     return retval;
};

static inline void CommitPages(void *mem, u32 page, u32 size)
{
     size = (size+4095) & ~4095;
     __asm__ __volatile__ (
     "call *__imp__CommitPages"
     ::"a" (page), "b"(mem),"c"(size>>12)
     :"edx" );
     __asm__ __volatile__ ("":::"eax","ebx","ecx");
};

static inline void UnmapPages(void *mem, size_t size)
{
     size = (size+4095) & ~4095;
     __asm__ __volatile__ (
     "call *__imp__UnmapPages"
     ::"a" (mem), "c"(size>>12)
     :"edx");
     __asm__ __volatile__ ("":::"eax","ecx");
};

static inline void usleep(u32 delay)
{
     if( !delay )
        delay++;
     delay*= 500;

     while(delay--)
        __asm__ __volatile__(
        "xorl %%eax, %%eax \n\t"
        "cpuid \n\t"
        :::"eax","ebx","ecx","edx");
     };

static inline void udelay1(u32 delay)
{
    if(!delay) delay++;
    delay*= 100;

    while(delay--)
    {
        __asm__ __volatile__(
        "xorl %%eax, %%eax \n\t"
        "cpuid"
        :::"eax","ebx","ecx","edx" );
    }
}

static inline void msleep1(unsigned int msecs)
{
    msecs /= 10;
    if(!msecs) msecs = 1;

     __asm__ __volatile__ (
     "call *__imp__Delay"
     ::"b" (msecs));
     __asm__ __volatile__ (
     "":::"ebx");

};

static inline void mdelay1(u32 time)
{
    time /= 10;
    if(!time) time = 1;

     __asm__ __volatile__ (
     "call *__imp__Delay"
     ::"b" (time));
     __asm__ __volatile__ (
     "":::"ebx");

};

static inline u32 __PciApi(int cmd)
{
     u32 retval;

     __asm__ __volatile__ (
     "call *__imp__PciApi \n\t"
     "movzxb %%al, %%eax"
     :"=a" (retval)
     :"a" (cmd)
     :"cc");

     return retval;
};

static inline void* __CreateObject(u32 pid, size_t size)
{
     void *retval;

     __asm__ __volatile__ (
     "call *__imp__CreateObject \n\t"
     :"=a" (retval)
     :"a" (size),"b"(pid)
     :"esi","edi", "memory");
     return retval;
}

static inline void __DestroyObject(void *obj)
{
     __asm__ __volatile__ (
     "call *__imp__DestroyObject \n\t"
     :
     :"a" (obj));
     __asm__ __volatile__ (
     ""
     :::"eax","ebx","ecx","edx","esi","edi","cc","memory");
}

static inline u32 GetService(const char *name)
{
    u32 handle;

    __asm__ __volatile__
    (
     "pushl %%eax \n\t"
     "call *__imp__GetService"
     :"=a" (handle)
     :"a" (name)
     :"ebx","ecx","edx","esi", "edi"
  );
  return handle;
};

static inline u32 safe_cli(void)
{
     u32 ifl;
     __asm__ __volatile__ (
     "pushf\n\t"
     "popl %0\n\t"
     "cli\n"
     : "=r" (ifl));
    return ifl;
}

static inline void safe_sti(u32 efl)
{
     if (efl & (1<<9))
        __asm__ __volatile__ ("sti");
}

static inline u32 get_eflags(void)
{
    u32 val;
    asm volatile (
    "pushfl\n\t"
    "popl %0\n"
    : "=r" (val));
    return val;
}

static inline void __clear (void * dst, unsigned len)
{
     u32 tmp;
     __asm__ __volatile__ (
     "cld \n\t"
     "rep stosb \n"
     :"=c"(tmp),"=D"(tmp)
     :"a"(0),"c"(len),"D"(dst));
     __asm__ __volatile__ ("":::"ecx","edi");
};

static inline void out8(const u16 port, const u8 val)
{
    __asm__ __volatile__
    ("outb  %1, %0\n" : : "dN"(port), "a"(val));
}

static inline void out16(const u16 port, const u16 val)
{
    __asm__ __volatile__
    ("outw  %1, %0\n" : : "dN"(port), "a"(val));
}

static inline void out32(const u16 port, const u32 val)
{
    __asm__ __volatile__
    ("outl  %1, %0\n" : : "dN"(port), "a"(val));
}

static inline u8 in8(const u16 port)
{
    u8 tmp;
    __asm__ __volatile__
    ("inb %1, %0\n" : "=a"(tmp) : "dN"(port));
    return tmp;
};

static inline u16 in16(const u16 port)
{
    u16 tmp;
    __asm__ __volatile__
    ("inw %1, %0\n" : "=a"(tmp) : "dN"(port));
    return tmp;
};

static inline u32 in32(const u16 port)
{
    u32 tmp;
    __asm__ __volatile__
    ("inl %1, %0\n" : "=a"(tmp) : "dN"(port));
    return tmp;
};

static inline void delay(int time)
{
     __asm__ __volatile__ (
     "call *__imp__Delay"
     ::"b" (time));
     __asm__ __volatile__ (
     "":::"ebx");

}

static inline void change_task(void)
{
     __asm__ __volatile__ (
     "call *__imp__ChangeTask");
}

static inline void sysSetScreen(int width, int height, int pitch)
{
    __asm__ __volatile__
    (
        "call *__imp__SetScreen"
        :
        :"a" (width-1),"d"(height-1), "c"(pitch)
    );
    __asm__ __volatile__
    ("" :::"eax","ecx","edx");
}

void  FASTCALL sysSetFramebuffer(void *fb)__asm__("SetFramebuffer");


static inline void __SysMsgBoardStr(char *text)
{
    __asm__ __volatile__(
    "call *__imp__SysMsgBoardStr"
    ::"S" (text));
};

static inline void *vzalloc(unsigned long size)
{
    void *mem;

    mem = KernelAlloc(size);
    if(mem)
        memset(mem, 0, size);

   return mem;
};


static inline int power_supply_is_system_supplied(void) { return -1; };

#endif
