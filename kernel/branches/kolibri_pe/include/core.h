
#define OS_BASE     0xE0000000
#define IMAGE_BASE  0xE0100000
#define LOAD_BASE   0x00100000

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
    : "=r" (tmp)
	);
  return tmp;
}

static inline void safe_sti(eflags_t efl)
{
	asm volatile (
    "pushl %0\n\t"
    "popfl\n"
    : : "r" (efl)
	);
}

static inline count_t fnzb(u32_t arg)
{
  count_t n;
  asm volatile ("xorl %0, %0 \n\t"
                "bsr %1, %0"
                :"=&r" (n)
                :"r"(arg)
                );
	return n;
}

static inline count_t _bsf(u32_t arg)
{
  count_t n;
  asm volatile ("xorl %0, %0 \n\t"
                "bsf %1, %0"
                :"=&r" (n)
                :"r"(arg)
                );
	return n;
}

static inline void _bts(u32_t *data, count_t val)
{
  asm volatile ("bts %0, %1 \n\t"
                :
                :"g"(data), "r"(val)
                :"cc"
                );
}

extern inline void _btr(u32_t *data, count_t val)
{
  asm volatile ("btr %0, %1 \n\t"
                :
                :"g"(data), "r"(val)
                :"cc"
                );
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


//extern __fastcall void* load_file(const char *path, size_t *size);
