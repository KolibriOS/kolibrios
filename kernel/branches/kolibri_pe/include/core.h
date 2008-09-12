
#define OS_BASE 0xE0000000


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

#endif


static inline eflags_t safe_cli(void)
{
  eflags_t tmp;
	asm volatile (
    "pushf\n\t"
    "pop %0\n\t"
		"cli\n"
    : "=r" (tmp)
	);
  return tmp;
}

static inline void safe_sti(eflags_t efl)
{
	asm volatile (
    "push %0\n\t"
		"popf\n"
    : : "r" (efl)
	);
}

static inline count_t fnzb(u32_t arg)
{
  count_t n;
  asm volatile ("xor %0, %0 \n\t"
                "bsr %0, %1"
                :"=&r" (n)
                :"r"(arg)
                );
	return n;
}

static inline count_t _bsf(u32_t arg)
{
  count_t n;
  asm volatile ("xor %0, %0 \n\t"
                "bsf %0, %1"
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

static inline void _btr(u32_t *data, count_t val)
{
  asm volatile ("btr %0, %1 \n\t"
                :
                :"g"(data), "r"(val)
                :"cc"
                );
}
