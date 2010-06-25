
#define OS_BASE     0xE0000000
#define IMAGE_BASE  0xE0100000
#define LOAD_BASE   0x00100000


#define page_tabs       0xDD800000

#define master_tab      (page_tabs+(page_tabs>>10))

#define sel_tss         0x08

#define sel_os_stack    0x10
#define sel_os_code     0x18

#define sel_app_code    0x23
#define sel_app_data    0x2B

#define sel_srv_code    0x31
#define sel_srv_stack   0x39



#define __export __attribute__ ((dllexport))


void printf (const char *format, ...);

#define CALLER ((addr_t) __builtin_return_address(0))

extern void panic_printf(char *fmt, ...) __attribute__((noreturn));

#ifdef CONFIG_DEBUG

# define panic(format, ...) \
		panic_printf("Kernel panic in %s() at %s:%u: " format, __func__, \
		__FILE__, __LINE__, ##__VA_ARGS__);

#	define ASSERT(expr) \
		if (!(expr)) { \
			panic("assertion failed (%s), caller=%p\n", #expr, CALLER); \
		}

#define DBG(format,...) printf(format,##__VA_ARGS__)

#else

#	define panic(format, ...) \
		panic_printf("Kernel panic: " format, ##__VA_ARGS__);

# define ASSERT(expr)

# define DBG(format,...)

# define PANIC(expr)   \
      if (!(expr)) {   \
         panic_printf("Kernel panic in %s() at %s:%u: " \
                      "assertion failed (%s)",__func__ ,__FILE__,__LINE__, \
                       #expr); \
      };

#endif


static inline eflags_t safe_cli(void)
{
  eflags_t tmp;
	asm volatile (
    "pushfl\n\t"
    "popl %0\n\t"
		"cli\n"
    : "=r" (tmp));
  return tmp;
}

static inline void safe_sti(eflags_t efl)
{
	asm volatile (
    "pushl %0\n\t"
    "popfl\n"
    : : "r" (efl));
}

static inline index_t fnzb(u32_t arg)
{
  count_t n;
    asm volatile (
    "xorl %0, %0 \n\t"
                "bsr %1, %0"
    :"=&r"(n) :"r"(arg) );
	return n;
}

static inline index_t _bsf(u32_t arg)
{
  count_t n;
    asm volatile (
    "xorl %0, %0 \n\t"
                "bsf %1, %0"
    :"=&r" (n) :"r"(arg));
	return n;
}

static inline void _bts(u32_t *data, count_t val)
{
    asm volatile (
    "bts %0, %1 \n\t"
    ::"g"(data), "r"(val):"cc");
}

extern inline void _btr(u32_t *data, count_t val)
{
    asm volatile (
    "btr %0, %1 \n\t"
    ::"g"(data), "r"(val):"cc");
}

extern inline void* load_file(const char *path, size_t *size)
{
     void* retval;
     size_t tmp;

     __asm__ __volatile__ (
     "pushl %%eax           \n\t"
     "call _load_file@4     \n\t"
     :"=eax" (retval), "=ebx"(tmp)
     :"a" (path) );

     if(size)
        *size = tmp;
     return retval;
};


/*                            reemain part
  saved_box       BOX
  ipc_start       dd ?
  ipc_size    dd ?
  event_mask      dd ?
  debugger_slot   dd ?
          dd ?
  keyboard_mode   db ?
          db 3   dup(?)
  dir_table       dd ?
  dbg_event_mem   dd ?
  dbg_regs:
  dbg_regs.dr0    dd ?
  dbg_regs.dr1    dd ?
  dbg_regs.dr2    dd ?
  dbg_regs.dr3    dd ?
  dbg_regs.dr7    dd ?
  wnd_caption     dd ?
  wnd_clientbox   BOX
*/

//extern __fastcall void* load_file(const char *path, size_t *size);


typedef struct
{
    u32_t    edi;
    u32_t    esi;
    u32_t    ebp;
    u32_t    tmp;                    // esp
    u32_t    ebx;
    u32_t    edx;
    u32_t    ecx;
    u32_t    eax;
    addr_t   retaddr;
    addr_t   eip;
    u32_t    cs;
    u32_t    eflags;
    addr_t   esp;
    u32_t    ss;                    // 14*4

    u32_t    tid;                   // thread id
    u32_t    slot;                  // thread slot

    addr_t   pdir;                  //

    u32_t    thr_flags;             // process is runnable only if zero

    int      ticks_left;            // number of scheduling ticks left */
    int      quantum_size;          // quantum size in ticks */

    u32_t    user_time;             // user time in ticks
    u32_t    sys_time;              // sys time in ticks

}thr_t;

#define EFL_IF      0x0200
#define EFL_IOPL1   0x1000
#define EFL_IOPL2   0x2000
#define EFL_IOPL3   0x3000

typedef struct
{
  u32_t      handle;
  u32_t      io_code;
  void       *input;
  int        inp_size;
  void       *output;
  int        out_size;
}ioctl_t;


typedef struct __attribute__ ((packed))
{
    u32_t code;
    union
    {
        struct                          /* window event    */
        {
            u32_t   win;                /* window handle   */
            u32_t   val1;
            u32_t   val2;
            u16_t   x;                  /* cursor x        */
            u16_t   y;                  /* cursor y        */
            u32_t   unused;
        };

        struct                          /* realtime io     */
        {
            u32_t   sender;             /* service handler */
            u32_t   stream;             /* io stream id, if present  */
            addr_t  offset;
            size_t  size;
        };

        struct                          /* ipc event       */
        {
            u32_t   sender;
            u32_t   io_code;
            addr_t *input;
            size_t  inp_size;
            addr_t *output;
            size_t  out_size;
        };
    };
}event_t;


void __fastcall dump_file(addr_t addr, size_t size);



