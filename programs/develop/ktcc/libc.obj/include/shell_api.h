#ifndef _SHELL_API_H_
#define _SHELL_API_H_

#include <sys/ksys.h>

#ifdef _BUILD_LIBC
#define __EXTERN
#else
#define __EXTERN extern
#endif

enum SHELL_API_COMMANDS {
    SHELL_OK = 0,
    SHELL_EXIT = 1,
    SHELL_PUTC = 2,
    SHELL_PUTS = 3,
    SHELL_GETC = 4,
    SHELL_GETS = 5,
    SHELL_CLS = 6,
    SHELL_PID = 7,
    SHELL_PING = 8
};

#define SHELL_SHM_MAX 1024 * 16

enum __SHELL_INIT_STATE {
    __SHELL_NOT_LOADED = 0, // not try init shell before
    __SHELL_LOADING = 1,    // in progress
    __SHELL_INIT_OK = 2,    // ok
    __SHELL_INIT_FAILED = 3 // fail init shell
};

#pragma pack(push, 1)
struct return_value {
    size_t written_bytes;
    char* str;
};

struct shell_shm_buffer {
    volatile uint8_t cmd;
    char data[SHELL_SHM_MAX - sizeof(((struct shell_shm_buffer*)NULL)->cmd) - sizeof(struct return_value) - sizeof(volatile int32_t)];
    struct return_value return_value;
    volatile int32_t shm_lock;                 /*0 free, 1 held*/
};
#pragma pack(pop)

static inline void __shell_lock(volatile int32_t * lock) {
    int32_t v;
    for (;;) {
        v = 1;
        asm volatile("xchgl %0, %1":"+r"(v),"+m"(*lock)::"memory");
        if (v==0) return;
        _ksys_thread_yield();
    }
}

static inline void __shell_unlock(volatile int32_t * lock) {
    asm volatile("movl $0, %0":"=m"(*lock)::"memory");
}

__EXTERN char __shell_shm_name[32];
__EXTERN struct shell_shm_buffer* __shell_shm;
__EXTERN enum __SHELL_INIT_STATE __shell_is_init;
__EXTERN void __shell_init();

#define __SHELL_WAIT()        \
    do {                      \
        _ksys_thread_yield(); \
    } while (__shell_shm->cmd);

__EXTERN int shell_ping();
__EXTERN unsigned shell_get_pid();
__EXTERN void shell_exit();

__EXTERN int shell_getc();
__EXTERN char* shell_gets(char* str, int n);

__EXTERN void shell_putc(char c);
__EXTERN size_t shell_puts(const char* str);
__EXTERN int shell_printf(const char* format, ...);

__EXTERN size_t shell_write_string(const char* s, size_t len);

__EXTERN void shell_cls();

#endif
