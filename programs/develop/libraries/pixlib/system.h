

typedef struct
{
  unsigned      handle;
  unsigned      io_code;
  void          *input;
  int           inp_size;
  void          *output;
  int           out_size;
}ioctl_t;


#define ERR_OK       0
#define ERR_PARAM   -1
#define ERR_NOMEM   -2


///////////////////////////////////////////////////////////////////////////////

#ifdef KOLIBRI_PE
  #define  LFB_BASE   0xDF000000
#else
  #define  LFB_BASE   0xFE000000
#endif


void usleep(u32_t delay);

static int __attribute__ ((always_inline))
abs (int i)
{
  return i < 0 ? -i : i;
};


extern inline u32_t get_service(char *name)
{
  u32_t retval;
  asm("int $0x40"
      :"=a"(retval)
      :"a"(68),"b"(16),"c"(name));

  return retval;
};

extern inline u32_t load_service(char *name)
{
  u32_t retval;
  asm("int $0x40"
      :"=a"(retval)
      :"a"(68),"b"(21),"c"(name));

  return retval;
};

extern inline int call_service(ioctl_t *io)
{
  int retval;

  asm("int $0x40"
      :"=a"(retval)
      :"a"(68),"b"(17),"c"(io)
      :"memory");

  return retval;
};

extern inline void* UserAlloc(size_t size)
{
  void *retval;

  asm("int $0x40"
      :"=a"(retval)
      :"a"(68),"b"(12),"c"(size)
      :"memory");
  return retval;
}

extern inline void UserFree(void *mem)
{
     __asm__ __volatile__(
     "int $0x40"
     ::"a"(68),"b"(13),"c"(mem)
     :"memory");
}

extern inline int GetScreenSize()
{
     int retval;

     __asm__ __volatile__(
     "int $0x40"
     :"=a"(retval)
     :"a"(61), "b"(1));
     return retval;
}

extern inline int GetScreenBpp()
{
     int retval;

     __asm__ __volatile__(
     "int $0x40"
     :"=a"(retval)
     :"a"(61), "b"(2));
     return retval;
}

extern inline int GetScreenPitch()
{
     int retval;

     __asm__ __volatile__(
     "int $0x40"
     :"=a"(retval)
     :"a"(61), "b"(3));
     return retval;
}

extern inline int test_mmx()
{
     int retval;

     __asm__ __volatile__(
     "cpuid\n\t"
     "testl $23, %%edx\n\t"
     "setnzb %%al\n\t"
     "movzx %%al, %%eax"
     :"=a"(retval)
     :"a"(0)
     :"ebx","ecx","edx");

     return retval;
}
