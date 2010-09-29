
#include <ddk.h>

#ifndef __SYSCALL_H__
#define __SYSCALL_H__

///////////////////////////////////////////////////////////////////////////////

#define STDCALL  __attribute__ ((stdcall)) __attribute__ ((dllimport))
#define FASTCALL __attribute__ ((fastcall)) __attribute__ ((dllimport))

#define IMPORT   __attribute__ ((dllimport))

///////////////////////////////////////////////////////////////////////////////

#define SysMsgBoardStr  __SysMsgBoardStr
#define PciApi          __PciApi
#define CreateObject    __CreateObject
#define DestroyObject   __DestroyObject

///////////////////////////////////////////////////////////////////////////////


void*  STDCALL AllocKernelSpace(size_t size)__asm__("AllocKernelSpace");
void   STDCALL FreeKernelSpace(void *mem)__asm__("FreeKernelSpace");
addr_t STDCALL MapIoMem(addr_t base, size_t size, u32_t flags)__asm__("MapIoMem");
void*  STDCALL KernelAlloc(size_t size)__asm__("KernelAlloc");
void*  STDCALL KernelFree(void *mem)__asm__("KernelFree");
void*  STDCALL UserAlloc(size_t size)__asm__("UserAlloc");
int    STDCALL UserFree(void *mem)__asm__("UserFree");

void*  STDCALL GetDisplay(void)__asm__("GetDisplay");

u32_t  IMPORT  GetTimerTicks(void)__asm__("GetTimerTicks");

addr_t STDCALL AllocPage(void)__asm__("AllocPage");
addr_t STDCALL AllocPages(count_t count)__asm__("AllocPages");

void* STDCALL CreateRingBuffer(size_t size, u32_t map)__asm__("CreateRingBuffer");

u32_t STDCALL RegService(char *name, srv_proc_t proc)__asm__("RegService");

int   STDCALL AttachIntHandler(int irq, void *handler, u32_t access) __asm__("AttachIntHandler");

void  FASTCALL MutexInit(struct mutex*)__asm__("MutexInit");
void  FASTCALL MutexLock(struct mutex*)__asm__("MutexLock");
void  FASTCALL MutexUnlock(struct mutex*)__asm__("MutexUnlock");

///////////////////////////////////////////////////////////////////////////////

void   STDCALL SetMouseData(int btn, int x, int y,
                            int z, int h)__asm__("SetMouseData");

u8_t  STDCALL PciRead8 (u32_t bus, u32_t devfn, u32_t reg)__asm__("PciRead8");
u16_t STDCALL PciRead16(u32_t bus, u32_t devfn, u32_t reg)__asm__("PciRead16");
u32_t STDCALL PciRead32(u32_t bus, u32_t devfn, u32_t reg)__asm__("PciRead32");

u32_t STDCALL PciWrite8 (u32_t bus, u32_t devfn, u32_t reg,u8_t val) __asm__("PciWrite8");
u32_t STDCALL PciWrite16(u32_t bus, u32_t devfn, u32_t reg,u16_t val)__asm__("PciWrite16");
u32_t STDCALL PciWrite32(u32_t bus, u32_t devfn, u32_t reg,u32_t val)__asm__("PciWrite32");

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

static inline evhandle_t CreateEvent(kevent_t *ev, u32_t flags)
{
     evhandle_t evh;

     __asm__ __volatile__ (
     "call *__imp__CreateEvent"
     :"=A"(evh.raw)
     :"S" (ev), "c"(flags)
     :"memory");
     __asm__ __volatile__ ("":::"ebx","ecx", "esi", "edi");

     return evh;
};

static inline void RaiseEvent(evhandle_t evh, u32_t flags, kevent_t *ev)
{
     __asm__ __volatile__ (
     "call *__imp__RaiseEvent"
     ::"a"(evh.handle),"b"(evh.euid),"d"(flags),"S" (ev)
     :"memory");
     __asm__ __volatile__ ("":::"ebx","ecx", "esi", "edi");

};

static inline void WaitEvent(u32_t handle, u32_t euid)
{
     __asm__ __volatile__ (
     "call *__imp__WaitEvent"
     ::"a"(handle),"b"(euid));
     __asm__ __volatile__ ("":::"ecx","edx", "esi");
};

static inline u32_t GetEvent(kevent_t *ev)
{
    u32_t  handle;

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

static inline u32_t GetPgAddr(void *mem)
{
     u32_t retval;

     __asm__ __volatile__ (
     "call *__imp__GetPgAddr \n\t"
     :"=eax" (retval)
     :"a" (mem) );
     return retval;
};

static inline void CommitPages(void *mem, u32_t page, u32_t size)
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

static inline void usleep(u32_t delay)
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

static inline void udelay(u32_t delay)
{
    if(!delay) delay++;
    delay*= 500;

    while(delay--)
    {
        __asm__ __volatile__(
        "xorl %%eax, %%eax \n\t"
        "cpuid"
        :::"eax","ebx","ecx","edx" );
    }
}

static inline void mdelay(u32_t time)
{
    time /= 10;
    if(!time) time = 1;

     __asm__ __volatile__ (
     "call *__imp__Delay"
     ::"b" (time));
     __asm__ __volatile__ (
     "":::"ebx");

};


static inline u32_t __PciApi(int cmd)
{
     u32_t retval;

     __asm__ __volatile__ (
     "call *__imp__PciApi \n\t"
     "movzxb %%al, %%eax"
     :"=a" (retval)
     :"a" (cmd)
     :"cc");

     return retval;
};

static inline void* __CreateObject(u32_t pid, size_t size)
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

static inline u32_t GetService(const char *name)
{
    u32_t handle;

    __asm__ __volatile__
    (
     "pushl %%eax \n\t"
     "call *__imp__GetService"
     :"=eax" (handle)
     :"a" (name)
     :"ebx","ecx","edx","esi", "edi"
  );
  return handle;
};

static inline u32_t safe_cli(void)
{
     u32_t ifl;
     __asm__ __volatile__ (
     "pushf\n\t"
     "popl %0\n\t"
     "cli\n"
     : "=r" (ifl));
    return ifl;
}

static inline void safe_sti(u32_t efl)
{
     if (efl & (1<<9))
        __asm__ __volatile__ ("sti");
}

static inline u32_t get_eflags(void)
{
    u32_t val;
    asm volatile (
    "pushfl\n\t"
    "popl %0\n"
    : "=r" (val));
    return val;
}

static inline void __clear (void * dst, unsigned len)
{
     u32_t tmp;
     __asm__ __volatile__ (
     "cld \n\t"
     "rep stosb \n"
     :"=c"(tmp),"=D"(tmp)
     :"a"(0),"c"(len),"D"(dst));
     __asm__ __volatile__ ("":::"ecx","edi");
};

static inline void out8(const u16_t port, const u8_t val)
{
    __asm__ __volatile__
    ("outb  %1, %0\n" : : "dN"(port), "a"(val));
}

static inline void out16(const u16_t port, const u16_t val)
{
    __asm__ __volatile__
    ("outw  %1, %0\n" : : "dN"(port), "a"(val));
}

static inline void out32(const u16_t port, const u32_t val)
{
    __asm__ __volatile__
    ("outl  %1, %0\n" : : "dN"(port), "a"(val));
}

static inline u8_t in8(const u16_t port)
{
    u8_t tmp;
    __asm__ __volatile__
    ("inb %1, %0\n" : "=a"(tmp) : "dN"(port));
    return tmp;
};

static inline u16_t in16(const u16_t port)
{
    u16_t tmp;
    __asm__ __volatile__
    ("inw %1, %0\n" : "=a"(tmp) : "dN"(port));
    return tmp;
};

static inline u32_t in32(const u16_t port)
{
    u32_t tmp;
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

int drm_order(unsigned long size);

static inline void __iomem *ioremap(uint32_t offset, size_t size)
{
    return (void __iomem*) MapIoMem(offset, size, 3);
}

static inline void iounmap(void *addr)
{
    FreeKernelSpace(addr);
}

static inline void __SysMsgBoardStr(char *text)
{
    __asm__ __volatile__(
    "call *__imp__SysMsgBoardStr"
    ::"S" (text));
};

#endif
