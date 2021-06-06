#ifndef __KOS_32_SYS_H__
#define __KOS_32_SYS_H__

#include <stddef.h>
#include <stdarg.h>
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned long long uint64_t;

#ifdef __cplusplus
extern "C" {
#endif

#define TYPE_3_BORDER_WIDTH  5
#define WIN_STATE_MINIMIZED  0x02
#define WIN_STATE_ROLLED     0x04
#define POS_SCREEN 0
#define POS_WINDOW 1

#define IPC_NOBUFFER 1
#define IPC_LOCKED 2
#define IPC_OVERFLOW 3
#define IPC_NOPID 4

#define SHM_OPEN        0x00
#define SHM_OPEN_ALWAYS 0x04
#define SHM_CREATE      0x08
#define SHM_READ        0x00
#define SHM_WRITE       0x01
   
// for clipboard funtions    
#define UTF 0
#define CP866 1
#define CP1251 2
#define TEXT 0
#define IMAGE 1
#define RAW 2

//Read/Write data as type (int char, etc.) at address "addr" with offset "offset". eg DATA(int, buff, 8);
#define DATA(type, addr, offset) *((type*)((uint8_t*)addr+offset))

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
}RGB;
    
typedef  unsigned int color_t;

typedef union __attribute__((packed)) pos_t
{
    uint32_t val;
    struct
    {
        short  x;
        short  y;
    };
} pos_t;


typedef union __attribute__((packed)) oskey_t
{
    uint32_t val;
    struct
    {
        uint8_t   state;
        uint8_t   code;
        uint16_t  ctrl_key;
    };
} oskey_t;

typedef struct
{
  unsigned      handle;
  unsigned      io_code;
  void          *input;
  int           inp_size;
  void          *output;
  int           out_size;
}ioctl_t;

typedef union
{
    struct
    {
        void   *data;
        size_t  size;
    } x;
    unsigned long long raw;
}ufile_t;

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

struct ipc_message
{
    uint32_t    pid;        // PID of sending thread
    uint32_t    datalen;    // data bytes
    char        data[0];    // data begin
};

struct ipc_buffer
{
    uint32_t    lock;   // nonzero is locked
    uint32_t    used;   // used bytes in buffer
    struct ipc_message  data[0];    // data begin
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
static inline
void draw_text_sys_bg(const char *text, int x, int y, int len, color_t color, color_t bg)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(4),"d"(text),
      "b"((x << 16) | y),
      "S"(len),"c"(color), "D"(bg)
     :"memory");
}


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

static inline uint32_t get_os_event()
{
    uint32_t val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(10));
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
uint64_t get_ns_count(void)
{
    uint64_t val;
    __asm__ __volatile__(
    "int $0x40"
    :"=A"(val)
    :"a"(26), "b"(10));
    return val;
};

static inline oskey_t get_key(void)
{
    oskey_t val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(2));
    return val;
}

static inline
uint32_t get_os_button()
{
    uint32_t val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(17));
    return val>>8;
};

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

static inline ufile_t load_file(const char *path)
{
    ufile_t uf;

    __asm__ __volatile__ (
    "int $0x40"
    :"=A"(uf.raw)
    :"a" (68), "b"(27),"c"(path));

    return uf;
};

static inline int GetScreenSize()
{
    int retval;

    __asm__ __volatile__(
    "int $0x40"
    :"=a"(retval)
    :"a"(61), "b"(1));
    return retval;
}


static inline void get_proc_info(char *info)
{
    __asm__ __volatile__(
    "int $0x40"
    :
    :"a"(9), "b"(info), "c"(-1)
    :"memory");
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


// newlib exclusive
#ifndef __TINYC__
int create_thread(int (*proc)(void *param), void *param, int stack_size);

void* load_library(const char *name);

void* get_proc_address(void *handle, const char *proc_name);

void enumerate_libraries(int (*callback)(void *handle, const char* name,
                                         uint32_t base, uint32_t size, void *user_data),
                         void *user_data);
#endif

// May be next section need to be added in newlibc

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


// copied from /programs/system/shell/system/kolibri.c
// fn's returned -1 as syserror, 1 as error, 0 as OK
static inline
int kol_clip_num()
{
    register uint32_t val;
    asm volatile ("int $0x40":"=a"(val):"a"(54), "b"(0));
    return val;
}

static inline
char* kol_clip_get(int n)
// returned buffer must be freed by user_free()
{
    register char* val;
    asm volatile ("int $0x40":"=a"(val):"a"(54), "b"(1), "c"(n));
    return val;
}

static inline
int kol_clip_set(int n, char buffer[])
{
    register uint32_t val;
    asm volatile ("int $0x40":"=a"(val):"a"(54), "b"(2), "c"(n), "d"(buffer));
    return val;
}

static inline
int kol_clip_pop()
{
    register uint32_t val;
    asm volatile ("int $0x40":"=a"(val):"a"(54), "b"(3));
    return val;
}

static inline
int kol_clip_unlock()
{
    register uint32_t val;
    asm volatile ("int $0x40":"=a"(val):"a"(54), "b"(4));
    return val;
}

static inline void get_system_colors(struct kolibri_system_colors *color_table)
{
  __asm__ volatile ("int $0x40"
		    :
		    :"a"(48),"b"(3),"c"(color_table),"d"(40)
		    );

  /* color_table should point to the system color table */
}

static inline void debug_board_write_byte(const char ch){
    __asm__ __volatile__(
    "int $0x40"
    :
    :"a"(63), "b"(1), "c"(ch));
}


static inline void draw_number_sys(int32_t number, int x, int y, int len, color_t color){
    register uint32_t fmt;
    fmt = len << 16 | 0x80000000; // no leading zeros + width
//    fmt = len << 16 | 0x00000000; //  leading zeros + width
    __asm__ __volatile__(
    "int $0x40"
    :
    :"a"(47), "b"(fmt), "c"(number), "d"((x << 16) | y), "S"(color));
}

static inline void draw_number_sys_bg(int32_t number, int x, int y, int len, color_t color, color_t bg){
    register uint32_t fmt;
    fmt = len << 16 | 0x80000000; // no leading zeros + width
//    fmt = len << 16 | 0x00000000; //  leading zeros + width
    __asm__ __volatile__(
    "int $0x40"
    :
    :"a"(47), "b"(fmt), "c"(number), "d"((x << 16) | y), "S"(color), "D"(bg));
}

static inline
uint32_t get_mouse_eventstate(void)
{
    uint32_t val;

    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(37),"b"(3));
    return val;
};

static inline
uint32_t set_event_mask(uint32_t mask)
{
    register uint32_t val;
    asm volatile ("int $0x40":"=a"(val):"a"(40), "b"(mask));
    return val;
}

typedef void (*thread_proc)(void*);

static inline
int start_thread(thread_proc proc, char* stack_top)
{
    register int val;
    asm volatile ("int $0x40":"=a"(val):"a"(51), "b"(1), "c"(proc), "d"(stack_top));
    return val;
}

static inline
void kos_exit()
{
    asm volatile ("int $0x40"::"a"(-1));
}

static inline void focus_window(int slot){
    asm volatile ("int $0x40"::"a"(18), "b"(3), "c"(slot));
}

static inline int get_thread_slot(int tid){
    register int val;
    asm volatile ("int $0x40":"=a"(val):"a"(18), "b"(21), "c"(tid));
    return val;
}

static inline void set_current_folder(char* dir){
    asm volatile ("int $0x40"::"a"(30), "b"(1), "c"(dir));
}

static inline int get_current_folder(char* buf, int bufsize){
    register int val;
    asm volatile ("int $0x40":"=a"(val):"a"(30), "b"(2), "c"(buf), "d"(bufsize));
    return val;
}

static inline
void ipc_set_area(void* buf, int bufsize){
    asm volatile ("int $0x40"::"a"(60), "b"(1), "c"(buf), "d"(bufsize));
}

static inline
int ipc_send_message(int pid_reciever, void *data, int datalen) {
    register int val;
    asm volatile ("int $0x40":"=a"(val):"a"(60), "b"(2), "c"(pid_reciever), "d"(data), "S"(datalen));
    return val;
}

static inline
void* shm_open(char *shm_name, int msize, int flags, int *retsz){
    register int val, cod;
    asm volatile ("int $0x40":"=a"(val),"=d"(cod):"a"(68), "b"(22), "c"(shm_name), "d"(msize), "S"(flags));

    if(retsz) *retsz = cod;  // errcode if NULL or memsize when open
    return (void*)val;
}

static inline
void shm_close(char *shm_name){
    asm volatile ("int $0x40"::"a"(68), "b"(23), "c"(shm_name));
}

static inline
int start_app(char *app_name, char *args){
    register int val;
    struct file_op_t
    {
        uint32_t    fn;
        uint32_t    flags;
        char*       args;
        uint32_t    res1, res2;
        char        zero;
        char*       app_name  __attribute__((packed));
    } file_op;
    memset(&file_op, 0, sizeof(file_op));
    file_op.fn = 7;
    file_op.args = args;
    file_op.app_name = app_name;

    
    asm volatile ("int $0x40":"=a"(val):"a"(70), "b"(&file_op));

    return val;
}

//added nonstatic inline because incomfortabre stepping in in debugger
void __attribute__ ((noinline)) debug_board_write_str(const char* str);
void __attribute__ ((noinline)) debug_board_printf(const char *format,...);

/* copy body to only one project file
void __attribute__ ((noinline)) debug_board_write_str(const char* str){
  while(*str)
    debug_board_write_byte(*str++);
}

void __attribute__ ((noinline)) debug_board_printf(const char *format,...)
{
        va_list ap;
        char log_board[300];

        va_start (ap, format);
        vsnprintf(log_board, sizeof log_board, format, ap);
        va_end(ap);
        debug_board_write_str(log_board);
}
*/

// TinyC don't support aliasing of static inline funcs, but support #define :)
#ifndef __TINYC__
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
static inline pos_t GetMousePos(int origin) __attribute__ ((alias ("get_mouse_pos")));
static inline uint32_t GetMouseButtons(void) __attribute__ ((alias ("get_mouse_buttons")));
static inline uint32_t GetMouseWheels(void) __attribute__ ((alias ("get_mouse_wheels")));
static inline uint32_t  LoadCursor(void *path, uint32_t flags) __attribute__ ((alias ("load_cursor")));
static inline uint32_t SetCursor(uint32_t  cursor) __attribute__ ((alias ("set_cursor")));
static inline int DestroyCursor(uint32_t cursor) __attribute__ ((alias ("destroy_cursor")));
static inline uint32_t GetOsEvent(void) __attribute__ ((alias ("get_os_event")));
static inline void *UserAlloc(size_t size) __attribute__ ((alias ("user_alloc")));
static inline int UserFree(void *mem) __attribute__ ((alias ("user_free")));
static inline void* UserRealloc(void *mem, size_t size) __attribute__ ((alias ("user_realloc")));
static inline int *UserUnmap(void *base, size_t offset, size_t size) __attribute__ ((alias ("user_unmap")));
static inline ufile_t LoadFile(const char *path) __attribute__ ((alias ("load_file")));
static inline void GetProcInfo(char *info) __attribute__ ((alias ("get_proc_info")));
#else
	#define BeginDraw begin_draw
	#define EndDraw end_draw
	#define DrawWindow sys_create_window
	#define DefineButton define_button
	#define DrawLine draw_line
	#define DrawBar draw_bar
	#define DrawBitmap draw_bitmap
	#define GetSkinHeight get_skin_height
	#define GetMousePos get_mouse_pos
	#define GetMouseButtons get_mouse_buttons
	#define GetMouseWheels get_mouse_wheels
	#define LoadCursor load_cursor
	#define SetCursor set_cursor
	#define DestroyCursor destroy_cursor
	#define GetOsEvent get_os_event
	#define UserAlloc user_alloc
	#define UserFree user_free
	#define UserRealloc user_realloc
	#define UserUnmap user_unmap
	#define LoadFile load_file
	#define GetProcInfo get_proc_info
#endif

#ifdef __cplusplus
}
#endif


#endif





