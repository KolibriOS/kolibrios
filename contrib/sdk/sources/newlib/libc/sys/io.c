
#include <stddef.h>
#include <stdio.h>
#include "io.h"

extern __io_handle __io_tab[64];

static int fake_io(const char *path, const void *buff,
           size_t offset, size_t count, size_t *done)
{
    *done = 0;
    return 10;
};

//static int fake_io_read(const char *path, void *buff,
//           size_t offset, size_t count, size_t *done) __attribute__ ((alias("fake_io")));

static int fake_io_read(const char *path, void *buff,
           size_t offset, size_t count, size_t *done)
{
    printf("%s path:%s buf:%p offset:%d count:%d\n",
            __FUNCTION__, path, buff, offset, count);

}

static int fake_io_write(const char *path, const void *buff,
           size_t offset, size_t count, size_t *done) __attribute__ ((alias("fake_io")));

static inline void debug_out(const char val)
{
    __asm__ __volatile__(
    "int $0x40 \n\t"
    ::"a"(63), "b"(1),"c"(val));
}

static int debugwrite(const char *path, const void *buff,
                 size_t offset, size_t count, size_t *writes)
{
    int ret = count;
    const char *p = buff;

    while (count--)
    {
        debug_out(*p++);
    };
    *writes = ret;
    return ret;
};

void init_stdio()
{
    __io_handle *ioh;

    ioh = &__io_tab[__io_alloc()];
    ioh->mode  = _READ|_ISTTY;
    ioh->read  = &fake_io_read;
    ioh->write = &fake_io_write;

    ioh = &__io_tab[__io_alloc()];
    ioh->mode  = _WRITE|_ISTTY;
    ioh->read  = &fake_io_read;
    ioh->write = &debugwrite;

    ioh = &__io_tab[__io_alloc()];
    ioh->mode  = _WRITE|_ISTTY;
    ioh->read  = &fake_io_read;
    ioh->write = &debugwrite;

}
