#ifndef __KOS_32_SYS_H__
#define __KOS_32_SYS_H__

#include <newlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

//#ifdef CONFIG_DEBUF
//  #define DBG(format,...) printf(format,##__VA_ARGS__)
//#else
//  #define DBG(format,...)
//#endif

#define TYPE_3_BORDER_WIDTH  5
#define WIN_STATE_MINIMIZED  0x02
#define WIN_STATE_ROLLED     0x04

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

typedef struct
{
  unsigned      handle;
  unsigned      io_code;
  void          *input;
  int           inp_size;
  void          *output;
  int           out_size;
}ioctl_t;

#pragma pack(push, 1)
struct proc_info
{
	unsigned long cpu_usage;
	unsigned short pos_in_stack;
	unsigned short slot;
	unsigned short reserved2;
	char name[12];
	unsigned long address;
	unsigned long memory_usage;
	unsigned long ID;
	unsigned long left,top;
	unsigned long width,height;
	unsigned short thread_state;
	unsigned short reserved3;
	unsigned long cleft, ctop, cwidth, cheight;
	unsigned char window_state;
	unsigned char reserved4[1024-71];
};
#pragma pack(pop)

struct kolibri_system_colors {
  color_t frame_area;
  color_t grab_bar;
  color_t grab_bar_button;
  color_t grab_button_text;
  color_t grab_text;
  color_t work_area;
  color_t work_button;
  color_t work_button_text;
  color_t work_text;
  color_t work_graph;
};

static inline void begin_draw(void)
{
    __asm__ __volatile__(
    "int $0x40" ::"a"(12),"b"(1));
};

static inline
void end_draw(void)
{
    __asm__ __volatile__(
    "int $0x40" ::"a"(12),"b"(2));
};

static inline void
put_image(uint16_t x_coord, uint16_t y_coord,
	  uint16_t size_x, uint16_t size_y, void *img)
{
    __asm__ __volatile__("int $0x40"
			 ::"a"(25),
			  "b"(img),
			  "c"(size_x<<16 | size_y),
			  "d"(x_coord<<16 | y_coord));
};


static inline
void sys_create_window(int x, int y, int w, int h, const char *name,
                       color_t workcolor, uint32_t style)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(0),
     "b"((x << 16) | ((w-1) & 0xFFFF)),
     "c"((y << 16) | ((h-1) & 0xFFFF)),
     "d"((style << 24) | (workcolor & 0xFFFFFF)),
     "D"(name),
     "S"(0) : "memory");
};

#define OLD -1
static inline
void sys_change_window(int new_x, int new_y, int new_w, int new_h)
{
    __asm__ __volatile__(
        "int $0x40"
        ::"a"(67), "b"(new_x), "c"(new_y), "d"(new_w),"S"(new_h)
    );
}

static inline
void define_button(uint32_t x_w, uint32_t y_h, uint32_t id, uint32_t color)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(8),
      "b"(x_w),
      "c"(y_h),
      "d"(id),
      "S"(color));
};

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
void draw_text_sys(const char *text, int x, int y, int len, color_t color)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(4),"d"(text),
      "b"((x << 16) | y),
      "S"(len),"c"(color)
     :"memory");
}

/*
void define_button_text(int x, int y, int w, int h, uint32_t id, uint32_t color, char* text)
{
    define_button(x * 65536 + w, y * 65536 + h, id, color);
    
    int tx = ((((-strlen(text))*8)+w)/2)+x;
	int ty = h/2-7+y;

	draw_text_sys(text, tx, ty, strlen(text), 0x90000000);
};
*/

static inline
uint32_t get_skin_height(void)
{
    uint32_t height;

    __asm__ __volatile__(
    "int $0x40 \n\t"
    :"=a"(height)
    :"a"(48),"b"(4));
    return height;
};

static inline void BeginDraw(void) __attribute__ ((alias ("begin_draw")));
static inline void EndDraw(void) __attribute__ ((alias ("end_draw")));
static inline void DrawWindow(int x, int y, int w, int h, const char *name,
                              color_t workcolor, uint32_t style)
                              __attribute__ ((alias ("sys_create_window")));
static inline void DefineButton(void) __attribute__ ((alias ("define_button")));
static inline void DrawLine(int xs, int ys, int xe, int ye, color_t color)
                            __attribute__ ((alias ("draw_line")));
static inline void DrawBar(int x, int y, int w, int h, color_t color)
                           __attribute__ ((alias ("draw_bar")));
static inline void DrawBitmap(void *bitmap, int x, int y, int w, int h)
                              __attribute__ ((alias ("draw_bitmap")));
static inline uint32_t GetSkinHeight(void) __attribute__ ((alias ("get_skin_height")));


#define POS_SCREEN 0
#define POS_WINDOW 1

static inline
pos_t get_mouse_pos(int origin)
{
    pos_t val;

    __asm__ __volatile__(
    "int $0x40 \n\t"
    "rol $16, %%eax"
    :"=a"(val)
    :"a"(37),"b"(origin));
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
    :"=a"(val)
    :"a"(37),"b"(7));
    return val;
};

static inline uint32_t load_cursor(void *path, uint32_t flags)
{
    uint32_t  val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(37), "b"(4), "c"(path), "d"(flags));
    return val;
}

static inline uint32_t  set_cursor(uint32_t  cursor)
{
    uint32_t  old;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(old)
    :"a"(37), "b"(5), "c"(cursor));
    return old;
};


#define EVM_REDRAW        1
#define EVM_KEY           2
#define EVM_BUTTON        4
#define EVM_EXIT          8
#define EVM_BACKGROUND    16
#define EVM_MOUSE         32
#define EVM_IPC           64
#define EVM_STACK         128
#define EVM_DEBUG         256
#define EVM_STACK2        512
#define EVM_MOUSE_FILTER  0x80000000
#define EVM_CURSOR_FILTER 0x40000000

static inline uint32_t set_wanted_events_mask(uint32_t event_mask)
{
  uint32_t  old_event_mask;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(old_event_mask)
    :"a"(40),"b"(event_mask));

    return old_event_mask;
};

static inline int destroy_cursor(uint32_t cursor)
{
    int ret;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(ret)
    :"a"(37), "b"(6), "c"(cursor)
    :"memory");
    return ret;
};

static inline pos_t GetMousePos(int origin) __attribute__ ((alias ("get_mouse_pos")));
static inline uint32_t GetMouseButtons(void) __attribute__ ((alias ("get_mouse_buttons")));
static inline uint32_t GetMouseWheels(void) __attribute__ ((alias ("get_mouse_wheels")));

static inline uint32_t  LoadCursor(void *path, uint32_t flags) __attribute__ ((alias ("load_cursor")));
static inline uint32_t SetCursor(uint32_t  cursor) __attribute__ ((alias ("set_cursor")));
static inline int DestroyCursor(uint32_t cursor) __attribute__ ((alias ("destroy_cursor")));

enum KOLIBRI_GUI_EVENTS {
    KOLIBRI_EVENT_NONE = 0,     /* Event queue is empty */
    KOLIBRI_EVENT_REDRAW = 1,   /* Window and window elements should be redrawn */
    KOLIBRI_EVENT_KEY = 2,      /* A key on the keyboard was pressed */
    KOLIBRI_EVENT_BUTTON = 3,   /* A button was clicked with the mouse */
    KOLIBRI_EVENT_DESKTOP = 5,  /* Desktop redraw finished */
    KOLIBRI_EVENT_MOUSE = 6,    /* Mouse activity (movement, button press) was detected */
    KOLIBRI_EVENT_IPC = 7,      /* Interprocess communication notify */
    KOLIBRI_EVENT_NETWORK = 8,  /* Network event */
    KOLIBRI_EVENT_DEBUG = 9,    /* Debug subsystem event */
    KOLIBRI_EVENT_IRQBEGIN = 16 /* 16..31 IRQ0..IRQ15 interrupt =IRQBEGIN+IRQn */
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

static inline uint32_t check_os_event(void)
{
    uint32_t val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(11));
    return val;
};

static inline uint32_t get_os_event(void)
{
    uint32_t val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(10));
    return val;
};
static inline uint32_t GetOsEvent(void) __attribute__ ((alias ("get_os_event")));

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
uint64_t get_ns_count(void)
{
    uint64_t val;
    __asm__ __volatile__(
    "int $0x40"
    :"=A"(val)
    :"a"(26), "b"(10));
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
uint32_t get_os_button(void)
{
    uint32_t val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(17));
    return val>>8;
};

static inline uint32_t
heap_init(void)
{
    uint32_t heapsize;

    __asm__ __volatile__(
                         "int $0x40"
                         :"=a"(heapsize)
                         :"a"(68),"b"(11)
                         );

    return heapsize;
}

static inline uint32_t get_service(char *name)
{
    uint32_t retval = 0;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(retval)
    :"a"(68),"b"(16),"c"(name)
    :"memory");

    return retval;
};

static inline int call_service(ioctl_t *io)
{
    int retval;

    __asm__ __volatile__(
    "int $0x40"
    :"=a"(retval)
    :"a"(68),"b"(17),"c"(io)
    :"memory","cc");

    return retval;
};


static inline void yield(void)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(68), "b"(1));
};

static inline void delay(uint32_t time)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(5), "b"(time)
    :"memory");
};

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
    :"a"(68),"b"(13),"c"(mem));
    return val;
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
};

static inline
int *user_unmap(void *base, size_t offset, size_t size)
{
    int  *val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(68),"b"(26),"c"(base),"d"(offset),"S"(size));
    return val;
};

static inline void *UserAlloc(size_t size) __attribute__ ((alias ("user_alloc")));
static inline int UserFree(void *mem) __attribute__ ((alias ("user_free")));
static inline void* UserRealloc(void *mem, size_t size) __attribute__ ((alias ("user_realloc")));
static inline int *UserUnmap(void *base, size_t offset, size_t size) __attribute__ ((alias ("user_unmap")));

typedef union
{
    struct
    {
        void   *data;
        size_t  size;
    };
    unsigned long long raw;
}ufile_t;

static inline ufile_t load_file(const char *path)
{
    ufile_t uf;

    __asm__ __volatile__ (
    "int $0x40"
    :"=A"(uf.raw)
    :"a" (68), "b"(27),"c"(path));

    return uf;
};
static inline ufile_t LoadFile(const char *path) __attribute__ ((alias ("load_file")));

static inline int GetScreenSize(void)
{
    int retval;

    __asm__ __volatile__(
    "int $0x40"
    :"=a"(retval)
    :"a"(61), "b"(1));
    return retval;
}

static inline
pos_t max_screen_size()
{
	pos_t size;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(size)
    :"a"(14));
    
      
     return size;
};

static inline void get_system_colors(struct kolibri_system_colors *color_table)
{
  __asm__ volatile ("int $0x40"
		    :
		    :"a"(48),"b"(3),"c"(color_table),"d"(40)
		    );
}

static inline void get_proc_info(char *info)
{
    __asm__ __volatile__(
    "int $0x40"
    :
    :"a"(9), "b"(info), "c"(-1)
    :"memory");
};
static inline void GetProcInfo(char *info) __attribute__ ((alias ("get_proc_info")));


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

    void *bitmap;
    int   stride;
};

static inline void Blit(void *bitmap, int dst_x, int dst_y,
                        int src_x, int src_y, int w, int h,
                        int src_w, int src_h, int stride)
{
    volatile struct blit_call bc;

    bc.dstx = dst_x;
    bc.dsty = dst_y;
    bc.w    = w;
    bc.h    = h;
    bc.srcx = src_x;
    bc.srcy = src_y;
    bc.srcw = src_w;
    bc.srch = src_h;
    bc.stride = stride;
    bc.bitmap = bitmap;

    __asm__ __volatile__(
    "int $0x40"
    ::"a"(73),"b"(0),"c"(&bc.dstx));
};

#define TLS_KEY_PID         0
#define TLS_KEY_TID         4
#define TLS_KEY_LOW_STACK   8
#define TLS_KEY_HIGH_STACK 12
#define TLS_KEY_LIBC       16

unsigned int tls_alloc(void);
int tls_free(unsigned int key);

static inline int tls_set(unsigned int key, void *val)
{
    int ret = -1;
    if(key < 4096)
    {
        __asm__ __volatile__(
        "movl %0, %%fs:(%1)"
        ::"r"(val),"r"(key));
        ret = 0;
    }
    return ret;
};

static inline void *tls_get(unsigned int key)
{
    void *val = (void*)-1;
    if(key < 4096)
    {
        __asm__ __volatile__(
        "movl %%fs:(%1), %0"
        :"=r"(val)
        :"r"(key));
    };
    return val;
}


int create_thread(int (*proc)(void *param), void *param, int stack_size);

void* load_library(const char *name);

void* get_proc_address(void *handle, const char *proc_name);

void enumerate_libraries(int (*callback)(void *handle, const char* name,
                                         uint32_t base, uint32_t size, void *user_data),
                         void *user_data);

#ifdef __cplusplus
}
#endif


#endif





