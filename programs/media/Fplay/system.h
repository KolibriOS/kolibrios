
#include <newlib.h>
#include <stdint.h>
#include <stddef.h>

#ifdef CONFIG_DEBUF
  #define DBG(format,...) printf(format,##__VA_ARGS__)
#else
  #define DBG(format,...)
#endif

typedef  unsigned int color_t;

typedef union __attribute__((packed))
{
    uint32_t val;
    struct
    {
        short  x;
        short  y;
    };
}pos_t;

typedef union __attribute__((packed))
{
    uint32_t val;
    struct
    {
        uint8_t   state;
        uint8_t   code;
        uint16_t  ctrl_key;
    };
}oskey_t;

static inline
void BeginDraw(void)
{
    __asm__ __volatile__(
    "int $0x40" ::"a"(12),"b"(1));
};

static inline
void EndDraw(void)
{
    __asm__ __volatile__(
    "int $0x40" ::"a"(12),"b"(2));
};

static inline void DrawWindow(int x, int y, int w, int h, char *name,
                       color_t workcolor, uint32_t style)
{

    __asm__ __volatile__(
    "int $0x40"
    ::"a"(0),
      "b"((x << 16) | (w & 0xFFFF)),
      "c"((y << 16) | (h & 0xFFFF)),
      "d"((style << 24) | (workcolor & 0xFFFFFF)),
      "D"(name));
};

static inline
pos_t get_mouse_pos(void)
{
    pos_t val;

    __asm__ __volatile__(
    "int $0x40 \n\t"
    "rol $16, %%eax"
    :"=a"(val)
    :"a"(37),"b"(1));
    return val;
}

static inline
pos_t get_cursor_pos(void)
{
    pos_t val;

    __asm__ __volatile__(
    "int $0x40 \n\t"
    "rol $16, %%eax"
    :"=a"(val)
    :"a"(37),"b"(0));
    return val;
}

static inline
uint32_t get_mouse_buttons(void)
{
    uint32_t val;

    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(37),"b"(2));
    return val;
};

static inline
uint32_t get_mouse_wheels(void)
{
    uint32_t val;

    __asm__ __volatile__(
    "int $0x40 \n\t"
    "rol $16, %%eax"
    :"=a"(val)
    :"a"(37),"b"(7));
    return val;
};

static inline
uint32_t wait_for_event(uint32_t time)
{
    uint32_t val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(23), "b"(time));
    return val;
};

static inline uint32_t check_os_event()
{
    uint32_t val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(11));
    return val;
};

static inline
uint32_t get_tick_count(void)
{
    uint32_t val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(26),"b"(9));
    return val;
};

static inline
oskey_t get_key(void)
{
    oskey_t val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(2));
    return val;
}

static inline
void draw_line(int xs, int ys, int xe, int ye, color_t color)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(38), "d"(color),
      "b"((xs << 16) | xe),
      "c"((ys << 16) | ye));
}



static inline
void draw_bar(int x, int y, int w, int h, color_t color)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(13), "d"(color),
      "b"((x << 16) | w),
      "c"((y << 16) | h));
}

static inline
void draw_bitmap(void *bitmap, int x, int y, int w, int h)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(7), "b"(bitmap),
      "c"((w << 16) | h),
      "d"((x << 16) | y));
}

static inline
void draw_text(const char *text, int x, int y, int len, color_t color)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(4),"d"(text),
      "b"((x << 16) | y),
      "S"(len),"c"(color));
}

static inline
void *user_alloc(size_t size)
{
    void  *val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(68),"b"(12),"c"(size));
    return val;
}

static inline
int user_free(void *mem)
{
    int  val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(68),"b"(12),"c"(mem));
    return val;
}

static inline
int *user_unmap(void *base, size_t offset, size_t size)
{
    void  *val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(68),"b"(26),"c"(base),"d"(offset),"S"(size));
    return val;
}

/*
extern inline
void    exit(int status)  __attribute__((noreturn)) ;

extern inline
void    exit(int status)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(-1));
    for(;;);
}
*/

static inline
uint32_t  load_cursor(void *path, uint32_t flags)
{
    uint32_t  val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(37), "b"(4), "c"(path), "d"(flags));
    return val;
}

static inline
uint32_t  set_cursor(uint32_t  cursor)
{
    uint32_t  old;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(old)
    :"a"(37), "b"(5), "c"(cursor));
    return old;
}

static inline
int destroy_cursor(uint32_t cursor)
{
    int ret;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(ret)
    :"a"(37), "b"(6), "c"(cursor)
    :"memory");
    return ret;
};

static inline void get_proc_info(char *info)
{
    __asm__ __volatile__(
    "int $0x40"
    :
    :"a"(9), "b"(info), "c"(-1));
}

static inline
void* user_realloc(void *mem, size_t size)
{
    void *val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(68),"b"(20),"c"(size),"d"(mem)
    :"memory");

    return val;
}

void *load_file(const char *path, size_t *len);

void *get_resource(void *data, uint32_t id);

struct blit_call
{
    int dstx;
    int dsty;
    int w;
    int h;

    int srcx;
    int srcy;
    int srcw;
    int srch;

    unsigned char *bitmap;
    int   stride;
};

void Blit(void *bitmap, int dst_x, int dst_y,
                        int src_x, int src_y, int w, int h,
                        int src_w, int src_h, int stride);






