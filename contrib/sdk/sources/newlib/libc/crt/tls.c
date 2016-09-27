#include <libsync.h>

static int tls_map[128/4];
static int *tls_scan_start = tls_map;
static mutex_t tls_mutex;

void tls_init()
{
    int i;

    mutex_init(&tls_mutex);

    tls_map[0] = 0xE0;

    for(i = 1; i < 128/4; i++)
        tls_map[i] = -1;
};

int tls_free(unsigned int key)
{
    int retval = -1;

    if(key < 4096)
    {
        mutex_lock(&tls_mutex);

        __asm__ volatile(
        "shrl $2, %0            \n\t"
        "btsl %0, (%1)          \n\t"
        ::"r"(key),"d"(tls_map)
        :"cc","memory");
        tls_scan_start = &tls_map[key>>5];
        mutex_unlock(&tls_mutex);
        retval = 0;
    }
    return retval;
};


unsigned int tls_alloc()
{
    unsigned int key;

    mutex_lock(&tls_mutex);

    __asm__ volatile(
    "1:                     \n\t"
    "bsfl (%1), %0          \n\t"
    "jnz 2f                 \n\t"
    "add $4, %1             \n\t"
    "cmpl $128+_tls_map, %1 \n\t"
    "jb 1b                  \n\t"
    "xorl %0, %0      \n\t"
    "notl %0             \n\t"
    "jmp 3f                 \n\t"
    "2:                     \n\t"
    "btrl %0, (%1)       \n\t"
    "subl $_tls_map, %1    \n\t"
    "leal (%0, %1, 8), %%eax \n\t"
    "shll $2, %0          \n\t"
    "3:"
    :"=r"(key),"=d"(tls_scan_start)
    :"d"(tls_scan_start)
    :"cc","memory");

    mutex_unlock(&tls_mutex);

    return key;
}
