#ifndef _ASM_GENERIC_BUG_H
#define _ASM_GENERIC_BUG_H

//extern __printf(3, 4)
//void warn_slowpath_fmt(const char *file, const int line,
//                       const char *fmt, ...);
//extern __printf(4, 5)
//void warn_slowpath_fmt_taint(const char *file, const int line, unsigned taint,
//                             const char *fmt, ...);

//extern void warn_slowpath_null(const char *file, const int line);

#define __WARN()                printf("\nWARNING: at %s:%d\n", __FILE__, __LINE__)
#define __WARN_printf(arg...)   printf("\nWARNING: at %s:%d\n", __FILE__, __LINE__)


#define WARN(condition, format...) ({                                   \
        int __ret_warn_on = !!(condition);                              \
        if (unlikely(__ret_warn_on))                                    \
            __WARN_printf(format);                                      \
        unlikely(__ret_warn_on);                                        \
})


#define WARN_ON(condition) ({                                           \
        int __ret_warn_on = !!(condition);                              \
        if (unlikely(__ret_warn_on))                                    \
                __WARN();                                               \
        unlikely(__ret_warn_on);                                        \
})


#define WARN_ONCE(condition, format...) ({                      \
        static bool __warned;                                   \
        int __ret_warn_once = !!(condition);                    \
                                                                \
        if (unlikely(__ret_warn_once))                          \
                if (WARN(!__warned, format))                    \
                        __warned = true;                        \
        unlikely(__ret_warn_once);                              \
})


#define WARN_ON_ONCE(condition) ({                              \
        static bool __warned;                                   \
        int __ret_warn_once = !!(condition);                    \
                                                                \
        if (unlikely(__ret_warn_once))                          \
                if (WARN_ON(!__warned))                         \
                        __warned = true;                        \
        unlikely(__ret_warn_once);                              \
})

#define BUG() do { \
         printk("BUG: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __FUNCTION__); \
         while(1){ delay(10); };                                                    \
 } while (0)

#define BUG_ON(condition) do { if (unlikely(condition)) BUG(); } while(0)

#define BUILD_BUG_ON_NOT_POWER_OF_2(n)                  \
        BUILD_BUG_ON((n) == 0 || (((n) & ((n) - 1)) != 0))


#define printk_once(fmt, ...)                   \
({                                              \
        static bool __print_once;               \
                                                \
        if (!__print_once) {                    \
                __print_once = true;            \
                printk(fmt, ##__VA_ARGS__);     \
        }                                       \
})


#define pr_warn_once(fmt, ...)                                  \
        printk_once(KERN_WARNING pr_fmt(fmt), ##__VA_ARGS__)

#endif
