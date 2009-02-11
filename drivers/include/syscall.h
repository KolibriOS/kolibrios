
#define OS_BASE   0x80000000

typedef struct
{
  u32_t      handle;
  u32_t      io_code;
  void       *input;
  int        inp_size;
  void       *output;
  int        out_size;
}ioctl_t;

typedef int (__stdcall *srv_proc_t)(ioctl_t *);

#define ERR_OK       0
#define ERR_PARAM   -1


u32_t __stdcall drvEntry(int)__asm__("_drvEntry");

///////////////////////////////////////////////////////////////////////////////

#define STDCALL __attribute__ ((stdcall)) __attribute__ ((dllimport))
#define IMPORT __attribute__ ((dllimport))

///////////////////////////////////////////////////////////////////////////////

#define SysMsgBoardStr  __SysMsgBoardStr
#define PciApi          __PciApi
//#define RegService      __RegService
#define CreateObject    __CreateObject
#define DestroyObject   __DestroyObject

///////////////////////////////////////////////////////////////////////////////

#define PG_SW       0x003
#define PG_NOCACHE  0x018

void*  STDCALL AllocKernelSpace(size_t size)__asm__("AllocKernelSpace");
void*  STDCALL KernelAlloc(size_t size)__asm__("KernelAlloc");
void*  STDCALL KernelFree(void *mem)__asm__("KernelFree");
void*  STDCALL UserAlloc(size_t size)__asm__("UserAlloc");
int    STDCALL UserFree(void *mem)__asm__("UserFree");

addr_t STDCALL AllocPages(count_t count)__asm__("AllocPages");

void* STDCALL CreateRingBuffer(size_t size, u32_t map)__asm__("CreateRingBuffer");

u32_t STDCALL RegService(char *name, srv_proc_t proc)__asm__("RegService");

int   STDCALL AttachIntHandler(int irq, void *handler, u32_t access) __asm__("AttachIntHandler");


//void *CreateObject(u32 pid, size_t size);
//void *DestroyObject(void *obj);

addr_t STDCALL MapIoMem(addr_t base, size_t size, u32_t flags)__asm__("MapIoMem");

///////////////////////////////////////////////////////////////////////////////

void   STDCALL SetMouseData(int btn, int x, int y,
                            int z, int h)__asm__("SetMouseData");

static u32_t PciApi(int cmd);

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

extern inline int GetScreenSize()
{
  int retval;

  asm("int $0x40"
      :"=a"(retval)
      :"a"(61), "b"(1));
  return retval;
}

extern inline int GetScreenBpp()
{
  int retval;

  asm("int $0x40"
      :"=a"(retval)
      :"a"(61), "b"(2));
  return retval;
}

extern inline int GetScreenPitch()
{
  int retval;

  asm("int $0x40"
      :"=a"(retval)
      :"a"(61), "b"(3));
  return retval;
}

extern inline u32_t GetPgAddr(void *mem)
{
     u32_t retval;

     __asm__ __volatile__ (
     "call *__imp__GetPgAddr \n\t"
     :"=eax" (retval)
     :"a" (mem) );
     return retval;
};

extern inline void CommitPages(void *mem, u32_t page, u32_t size)
{
     size = (size+4095) & ~4095;
     __asm__ __volatile__ (
     "call *__imp__CommitPages"
     ::"a" (page), "b"(mem),"c"(size>>12)
     :"edx" );
     __asm__ __volatile__ ("":::"eax","ebx","ecx");
};

extern inline void UnmapPages(void *mem, size_t size)
{
     size = (size+4095) & ~4095;
     __asm__ __volatile__ (
     "call *__imp__UnmapPages"
     ::"a" (mem), "c"(size>>12)
     :"edx");
     __asm__ __volatile__ ("":::"eax","ecx");
};

extern inline void usleep(u32_t delay)
{
     if( !delay )
        delay++;
     delay*=1000;

     while(delay--)
     __asm__ __volatile__ (
     "xorl %%eax, %%eax \n\t"
     "cpuid \n\t"
        :::"eax","ebx","ecx","edx");
};

extern inline u32_t __PciApi(int cmd)
{
     u32_t retval;

     __asm__ __volatile__ (
     "call *__imp__PciApi"
     :"=a" (retval)
     :"a" (cmd)
     :"memory");
     return retval;
};

extern inline void* __CreateObject(u32_t pid, size_t size)
{
     void *retval;

     __asm__ __volatile__ (
     "call *__imp__CreateObject \n\t"
     :"=a" (retval)
     :"a" (size),"b"(pid)
     :"esi","edi", "memory");
     return retval;
}

extern inline void *__DestroyObject(void *obj)
{
     __asm__ __volatile__ (
     "call *__imp__DestroyObject"
     :
     :"a" (obj)
     :"ebx","edx","esi","edi", "memory");
}


/*
u32 __RegService(char *name, srv_proc_t proc)
{
  u32 retval;

  asm __volatile__
  (
    "pushl %%eax \n\t"
    "pushl %%ebx \n\t"
    "call *__imp__RegService \n\t"
    :"=eax" (retval)
    :"a" (proc), "b" (name)
    :"memory"
  );
  return retval;
};
*/

extern inline u32_t safe_cli(void)
{
     u32_t ifl;
     __asm__ __volatile__ (
     "pushf\n\t"
     "popl %0\n\t"
     "cli\n"
     : "=r" (ifl));
    return ifl;
}

extern inline void safe_sti(u32_t ifl)
{
     __asm__ __volatile__ (
     "pushl %0\n\t"
     "popf\n"
     : : "r" (ifl)
	);
}

extern inline void __clear (void * dst, unsigned len)
{
     u32_t tmp;
     __asm__ __volatile__ (
//     "xorl %%eax, %%eax \n\t"
     "cld \n\t"
     "rep stosb \n"
     :"=c"(tmp),"=D"(tmp)
     :"a"(0),"c"(len),"D"(dst));
     __asm__ __volatile__ ("":::"ecx","edi");
};

extern inline void out8(const u16_t port, const u8_t val)
{
    __asm__ __volatile__
    ("outb  %1, %0\n" : : "dN"(port), "a"(val));
}

extern inline void out16(const u16_t port, const u16_t val)
{
    __asm__ __volatile__
    ("outw  %1, %0\n" : : "dN"(port), "a"(val));
}

extern inline void out32(const u16_t port, const u32_t val)
{
    __asm__ __volatile__
    ("outl  %1, %0\n" : : "dN"(port), "a"(val));
}

extern inline u8_t in8(const u16_t port)
{
    u8_t tmp;
    __asm__ __volatile__
    ("inb %1, %0\n" : "=a"(tmp) : "dN"(port));
    return tmp;
};

extern inline u16_t in16(const u16_t port)
{
    u16_t tmp;
    __asm__ __volatile__
    ("inw %1, %0\n" : "=a"(tmp) : "dN"(port));
    return tmp;
};

extern inline u32_t in32(const u16_t port)
{
    u32_t tmp;
    __asm__ __volatile__
    ("inl %1, %0\n" : "=a"(tmp) : "dN"(port));
    return tmp;
};

extern inline void delay(int time)
{
     __asm__ __volatile__ (
     "call *__imp__Delay"
     ::"b" (time));
     __asm__ __volatile__ (
     "":::"ebx");

}

extern inline void change_task()
{
     __asm__ __volatile__ (
     "call *__imp__ChangeTask");
}

