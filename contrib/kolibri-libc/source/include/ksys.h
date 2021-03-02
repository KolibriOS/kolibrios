#ifndef _KSYS_H_
#define _KSYS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#define asm_inline __asm__ __volatile__
#define not_optimized __attribute__((optimize("O0")))

#define KSYS_FS_ERR_SUCCESS 0  // Success
#define KSYS_FS_ERR_1       1  // Base and/or partition of a hard disk is not defined (fn21.7 & fn21.8)
#define KSYS_FS_ERR_2       2  // Function is not supported for the given file system
#define KSYS_FS_ERR_3       3  // Unknown file system
#define KSYS_FS_ERR_4       4  // Reserved, is never returned in the current implementation
#define KSYS_FS_ERR_5       5  // File not found
#define KSYS_FS_ERR_EOF     6  // End of file, EOF
#define KSYS_FS_ERR_7       7  // Pointer lies outside of application memory
#define KSYS_FS_ERR_8       8  // Disk is full
#define KSYS_FS_ERR_9       9  // FAT table is destroyed
#define KSYS_FS_ERR_10      10 // Access denied
#define KSYS_FS_ERR_11      11 // Device error

typedef struct {
    unsigned char blue;
    unsigned char green;
    unsigned char red;
}rgb_t;
 
#pragma pack(push,1)
typedef union{
    unsigned val;
    struct{
        short  x;
        short  y;
    };
}ksys_pos_t;

typedef union ksys_oskey_t{
    unsigned val;
    struct{
        unsigned char state;
        unsigned char code;
        unsigned char ctrl_key;
    };
}ksys_oskey_t;

typedef struct{
  unsigned     handle;
  unsigned     io_code;
  unsigned     *input;
  int          inp_size;
  void         *output;
  int          out_size;
}ksys_ioctl_t;

typedef struct{
    void *data;
    size_t size;
}ksys_ufile_t;


typedef struct{
    unsigned            p00;
    union{
        uint64_t        p04; 
        struct {
            unsigned    p04dw;
            unsigned    p08dw;
        };
    };
    unsigned            p12;
    union {
        unsigned        p16;
        const char     *new_name;
        void           *bdfe;
        void           *buf16;
        const void     *cbuf16;
    };
    char                p20;
    const char         *p21;
}ksys70_t;


typedef struct {
    unsigned attributes;
    unsigned name_cp;
    char creation_time[4];
    char creation_date[4];
    char last_access_time[4];
    char last_access_date[4];
    char last_modification_time[4];
    char last_modification_date[4];
    unsigned long long size;
    char name[0];
}ksys_bdfe_t;

typedef struct {
  int cpu_usage;             //+0
  int window_pos_info;       //+4
  short int reserved1;       //+8
  char name[12];             //+10
  int memstart;              //+22
  int memused;               //+26
  int pid;                   //+30
  int winx_start;            //+34
  int winy_start;            //+38
  int winx_size;             //+42
  int winy_size;             //+46
  short int slot_info;       //+50
  short int reserved2;       //+52
  int clientx;               //+54
  int clienty;               //+58
  int clientwidth;           //+62
  int clientheight;          //+66
  unsigned char window_state;//+70
  char reserved3[1024-71];   //+71
}ksys_proc_table_t;

typedef unsigned int ksys_color_t;

typedef struct{
    ksys_color_t frame_area;
    ksys_color_t grab_bar;
    ksys_color_t grab_bar_button; 
    ksys_color_t grab_button_text;
    ksys_color_t grab_text;
    ksys_color_t work_area;
    ksys_color_t work_button;
    ksys_color_t work_button_text;
    ksys_color_t work_text;
    ksys_color_t work_graph;
}ksys_colors_table_t;

typedef struct{
    unsigned pid;      // PID of sending thread
    unsigned datalen;  // data bytes
    char     *data;    // data begin
}ksys_ipc_msg;
 
typedef struct{
    unsigned lock;              // nonzero is locked
    unsigned used;              // used bytes in buffer
    ksys_ipc_msg *data;         // data begin
}ksys_ipc_buffer;

typedef struct {
    char* func_name;
    void* func_ptr;
}ksys_coff_etable_t;

#pragma pack(pop)

enum KSYS_EVENTS {
    KSYS_EVENT_NONE = 0,     /* Event queue is empty */
    KSYS_EVENT_REDRAW = 1,   /* Window and window elements should be redrawn */
    KSYS_EVENT_KEY = 2,      /* A key on the keyboard was pressed */
    KSYS_EVENT_BUTTON = 3,   /* A button was clicked with the mouse */
    KSYS_EVENT_DESKTOP = 5,  /* Desktop redraw finished */
    KSYS_EVENT_MOUSE = 6,    /* Mouse activity (movement, button press) was detected */
    KSYS_EVENT_IPC = 7,      /* Interprocess communication notify */
    KSYS_EVENT_NETWORK = 8,  /* Network event */
    KSYS_EVENT_DEBUG = 9,    /* Debug subsystem event */
    KSYS_EVENT_IRQBEGIN = 16 /* 16..31 IRQ0..IRQ15 interrupt =IRQBEGIN+IRQn */
};

enum KSYS_FILE_ENCODING{
    KSYS_FILE_CP866 =1,
    KSYS_FILE_UTF16LE = 2,
    KSYS_FILE_UTF8 = 3
};

enum KSYS_CLIP_ENCODING{
    KSYS_CLIP_UTF8 = 0,
    KSYS_CLIP_CP866 = 1,
    KSYS_CLIP_CP1251 = 2
};

enum KSYS_CLIP_TYPES{
    KSYS_CLIP_TEXT = 0,
    KSYS_CLIP_IMAGE = 1,
    KSYS_CLIP_RAW = 2
};

enum KSYS_MOUSE_POS{
    KSYS_MOUSE_SCREEN_POS = 0,
    KSYS_MOUSE_WINDOW_POS = 1
};

static inline 
int _ksys_strcmp(const char * s1, const char * s2 )
{
    while ((*s1) && (*s1 == *s2)){
        ++s1;
        ++s2;
    }

    return ( *( unsigned char * )s1 - * ( unsigned char * )s2 );
}

// Functions for working with the graphical interface

static inline 
void _ksys_start_draw()
{
   asm_inline("int $0x40"::"a"(12),"b"(1));
}

static inline 
void _ksys_end_draw()
{
    asm_inline("int $0x40" ::"a"(12),"b"(2));
}

static inline 
void _ksys_create_window(int x, int y, int w, int h, const char *name, ksys_color_t workcolor, unsigned style)
{
    asm_inline(
        "int $0x40"
        ::"a"(0),
        "b"((x << 16) | ((w-1) & 0xFFFF)),
        "c"((y << 16) | ((h-1) & 0xFFFF)),
        "d"((style << 24) | (workcolor & 0xFFFFFF)),
        "D"(name),
        "S"(0) : "memory"
     );
};

static inline 
void _ksys_change_window(int new_x, int new_y, int new_w, int new_h)
{
    asm_inline(
        "int $0x40"
        ::"a"(67), "b"(new_x), "c"(new_y), "d"(new_w),"S"(new_h)
    );
}
 
static inline
void _ksys_define_button(unsigned x, unsigned y, unsigned w, unsigned h, unsigned id, ksys_color_t color)
{
   asm_inline(
        "int $0x40"
        ::"a"(8),
        "b"((x<<16)+w),
        "c"((y<<16)+h),
        "d"(id),
        "S"(color)
    );
};

static inline
void _ksys_draw_line(int xs, int ys, int xe, int ye, ksys_color_t color)
{
    asm_inline(
        "int $0x40"
        ::"a"(38), "d"(color),
        "b"((xs << 16) | xe),
        "c"((ys << 16) | ye)
    );
}

static inline
void _ksys_draw_bar(int x, int y, int w, int h, ksys_color_t color)
{
    asm_inline(
        "int $0x40"
        ::"a"(13), "d"(color),
        "b"((x << 16) | w),
        "c"((y << 16) | h)
    );
}

static inline
void _ksys_draw_bitmap(void *bitmap, int x, int y, int w, int h)
{
    asm_inline(
        "int $0x40"
        ::"a"(7), "b"(bitmap),
        "c"((w << 16) | h),
        "d"((x << 16) | y)
    );
}

static inline
void _ksys_draw_text(const char *text, int x, int y, int len, ksys_color_t color)
{
   asm_inline(
        "int $0x40"
        ::"a"(4),"d"(text),
        "b"((x << 16) | y),
        "S"(len),"c"(color)
        :"memory"
    );
}

static inline
void _ksys_draw_text_bg(const char *text, int x, int y, int len, ksys_color_t color, ksys_color_t bg)
{
    asm_inline(
        "int $0x40"
        ::"a"(4),"d"(text),
        "b"((x << 16) | y),
        "S"(len),"c"(color), "D"(bg)
        :"memory"
    );
}

static inline 
void _ksys_draw_number(int number, int x, int y, int len, ksys_color_t color){
    unsigned fmt;
    fmt = len << 16 | 0x80000000; // no leading zeros + width
    asm_inline(
        "int $0x40"
        ::"a"(47), "b"(fmt), "c"(number), "d"((x << 16) | y), "S"(color)
    );
}
 
static inline 
void _ksys_draw_number_bg(unsigned number, int x, int y, int len, ksys_color_t color, ksys_color_t bg){
    unsigned fmt;
    fmt = len << 16 | 0x80000000; // no leading zeros + width
    asm_inline(
        "int $0x40"
        ::"a"(47), "b"(fmt), "c"(number), "d"((x << 16) | y), "S"(color), "D"(bg)
    );
}

static inline
unsigned _ksys_get_skin_height()
{
    unsigned height;
    asm_inline(
        "int $0x40 \n\t"
        :"=a"(height)
        :"a"(48),"b"(4)
    );
    return height;
}

static inline 
void _ksys_get_colors(ksys_colors_table_t *color_table)
{
    asm_inline(
       "int $0x40"
        ::"a"(48),"b"(3),"c"(color_table),"d"(40)
    );
}


/* Functions for working with a mouse and cursors. */

static inline
ksys_pos_t _ksys_get_mouse_pos(int origin)
{
    ksys_pos_t val;
    asm_inline(
        "int $0x40 \n\t"
        "rol $16, %%eax"
        :"=a"(val)
        :"a"(37),"b"(origin)
    );
    return val;
}
 
static inline
unsigned _ksys_get_mouse_buttons()
{
    unsigned val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(37),"b"(2)
    );
    return val;
}
 
static inline
unsigned _ksys_get_mouse_wheels()
{
    unsigned val;
    asm_inline(
        "int $0x40 \n\t"
        :"=a"(val)
        :"a"(37),"b"(7)
    );
    return val;
}
 
static inline 
unsigned _ksys_load_cursor(void *path, unsigned flags)
{
    unsigned val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(37), "b"(4), "c"(path), "d"(flags)
    );
    return val;
}
 
static inline 
unsigned _ksys_set_cursor(unsigned  cursor)
{
    unsigned old;
    asm_inline(
        "int $0x40"
        :"=a"(old)
        :"a"(37), "b"(5), "c"(cursor)
    );
    return old;
}
 
static inline 
int _ksys_destroy_cursor(unsigned cursor)
{
    int ret;
    asm_inline(
        "int $0x40"
        :"=a"(ret)
        :"a"(37), "b"(6), "c"(cursor)
        :"memory"
    );
    return ret;
}

static inline
unsigned _ksys_get_mouse_eventstate()
{
    unsigned val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(37),"b"(3)
    );
    return val;
}


/* Functions for working with events and buttons. */

static inline
unsigned _ksys_set_event_mask(unsigned mask)
{
    unsigned val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(40), "b"(mask)
    );
    return val;
}

static inline
unsigned _ksys_wait_event(unsigned time)
{
    unsigned val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(23), "b"(time)
    );
    return val;
}
 
static inline 
unsigned _ksys_check_event()
{
    unsigned val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(11)
    );
    return val;
}
 
static inline 
unsigned _ksys_get_event()
{
    unsigned val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(10)
    );
    return val;
}

static inline
unsigned _ksys_get_button()
{
    unsigned val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(17)
    );
    return val>>8;
}
 
static inline 
ksys_oskey_t _ksys_get_key(void)
{
    ksys_oskey_t val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(2)
    );
    return val;
}

/* Functions for working with the clipboard */

static inline
int _ksys_clip_num()
{
    unsigned val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(54), "b"(0)
    );
    return val;
}
 
static inline
char* _ksys_clip_get(int n) // returned buffer must be freed by _ksys_free()
{
    char* val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(54), "b"(1), "c"(n)
    );
    return val;
}
 
static inline
int _ksys_clip_set(int n, char *buffer)
{
    unsigned val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(54), "b"(2), "c"(n), "d"(buffer)
    );
    return val;
}
 
static inline
int _ksys_clip_pop()
{
    unsigned val;
    asm_inline (
        "int $0x40"
        :"=a"(val)
        :"a"(54), "b"(3)
    );
    return val;
}
 
static inline
int _ksys_clip_unlock()
{
    unsigned val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(54), "b"(4)
    );
    return val;
}


/* Working with time */

static inline
unsigned _ksys_get_tick_count()
{
    unsigned val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(26),"b"(9)
    );
    return val;
}
 
static inline
uint64_t  _ksys_get_ns_count()
{
    uint64_t val;
    asm_inline(
        "int $0x40"
        :"=A"(val)
        :"a"(26), "b"(10)
    );
    return val;
}

static inline 
void _ksys_delay(unsigned time)
{
    asm_inline(
        "int $0x40"
        ::"a"(5), "b"(time)
        :"memory"
    );
}

static inline
unsigned _ksys_get_date()
{
    unsigned val;
    asm_inline("int $0x40":"=a"(val):"a"(29));
    return val;
}

static inline
unsigned _ksys_get_clock()
{
    unsigned val;
    asm_inline("int $0x40":"=a"(val):"a"(3));
    return val;
}


/* Working with memory allocation */ 

static inline
void* _ksys_alloc(size_t size){
    void  *val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(68),"b"(12),"c"(size)
    );
    return val;
}
 
static inline
int _ksys_free(void *mem)
{
    int val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(68),"b"(13),"c"(mem)
    );
    return val;
}

static inline
void* _ksys_realloc(void *mem, size_t size)
{
    void *val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(68),"b"(20),"c"(size),"d"(mem)
        :"memory"
    );
    return val;
}
 
static inline
int* _ksys_unmap(void *base, size_t offset, size_t size)
{
    int  *val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(68),"b"(26),"c"(base),"d"(offset),"S"(size)
    );
    return val;
}


/* Loading the dynamic coff library */

static inline
ksys_coff_etable_t* not_optimized _ksys_cofflib_load(const char* path)
{
    ksys_coff_etable_t *table;
    asm_inline(
        "int $0x40"
        :"=a"(table)
        :"a"(68),"b"(19), "c"(path)
    );
    return table;
}

static inline
void* not_optimized _ksys_cofflib_getproc(ksys_coff_etable_t *table, const char* fun_name)
{
    unsigned i=0;
    while (1){
        if (NULL == (table+i)->func_name){
            break;
        }else{
            if (!_ksys_strcmp(fun_name, (table+i)->func_name)){
                return (table+i)->func_ptr;
            }
        }
        i++;
    }
    return NULL;
}


/* Debug board functions */ 

static inline
void _ksys_debug_putc(char c)
{
    asm_inline("int $0x40"::"a"(63), "b"(1), "c"(c));
}
 
static inline
void _ksys_debug_puts(char *s)
{
    unsigned i=0;
    while (*(s+i)){
        asm_inline ("int $0x40"::"a"(63), "b"(1), "c"(*(s+i)));
        i++;
    }
}
 

/* Working with threads and process */

static inline
int _ksys_start_thread(void* proc, char* stack_top)
{
    int val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(51), "b"(1), "c"(proc), "d"(stack_top)
    );
    return val;
}
 
static inline 
void _ksys_focus_window(int slot){
    asm_inline(
        "int $0x40"
        ::"a"(18), "b"(3), "c"(slot)
    );
}
 
static inline 
int _ksys_get_thread_slot(int tid){
    int val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(18), "b"(21), "c"(tid)
    );
    return val;
}

static inline 
int not_optimized _ksys_process_info(ksys_proc_table_t* table, int pid)
{
    int val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(9), "b"(table), "c"(pid)
    );
    return val;
}

static inline
void _ksys_exit()
{
    asm_inline("int $0x40"::"a"(-1));
}


/* Working with files and directories */

static inline 
void _ksys_setcwd(char* dir){
    asm_inline(
        "int $0x40"
        ::"a"(30), "b"(1), "c"(dir)
    );
}
 
static inline 
int _ksys_getcwd(char* buf, int bufsize){
    register int val;
    asm_inline(
        "int $0x40"
        :"=a"(val):"a"(30), "b"(2), "c"(buf), "d"(bufsize)
    );
    return val;
}

static inline 
ksys_ufile_t _ksys_load_file(const char *path)
{
    ksys_ufile_t uf;
    asm_inline(
        "int $0x40"
        :"=a"(uf.data), "=d"(uf.size)
        :"a"(68), "b"(27),"c"(path)
    );
    return uf;
}

static inline 
ksys_ufile_t _ksys_load_file_enc(const char *path, unsigned file_encoding)
{
    ksys_ufile_t uf;
    asm_inline(
        "int $0x40"
        :"=a"(uf.data), "=d"(uf.size)
        :"a"(68), "b"(28),"c"(path), "d"(file_encoding)
    );
    return uf;
}

static inline
int not_optimized _ksys_work_files(const ksys70_t *k)
{
    int status;
    asm_inline(
        "int $0x40"
        :"=a"(status)
        :"a"(70), "b"(k)
    );
    return status;
}

static inline
int not_optimized _ksys_file_read_file(const char *name, unsigned long long offset, unsigned size, void *buf, unsigned *bytes_read)
{
    ksys70_t k;
    k.p00 = 0;
    k.p04 = offset;
    k.p12 = size;
    k.buf16 = buf;
    k.p20 = 0;
    k.p21 = name;
    int status;
    unsigned bytes_read_v;
    asm_inline(
        "int $0x40"
        :"=a"(status), "=b"(bytes_read_v)
        :"a"(70), "b"(&k)
    );
    if (!status) {
        *bytes_read = bytes_read_v;
    }
    return status;
}

static inline
int not_optimized _ksys_file_write_file(const char *name, unsigned long long offset, unsigned size, const void *buf, unsigned *bytes_written)
{
    ksys70_t k;
    k.p00 = 3;
    k.p04 = offset;
    k.p12 = size;
    k.cbuf16 = buf;
    k.p20 = 0;
    k.p21 = name;
    int status;
    unsigned bytes_written_v;
    asm_inline(
        "int $0x40"
        :"=a"(status), "=b"(bytes_written_v)
        :"a"(70), "b"(&k)
    );
    if (!status) {
        *bytes_written = bytes_written_v;
    }
    return status;
}

static inline
int not_optimized _ksys_file_get_info(const char *name, ksys_bdfe_t *bdfe)
{
    ksys70_t k;
    k.p00 = 5;
    k.bdfe = bdfe;
    k.p20 = 0;
    k.p21 = name;
    return _ksys_work_files(&k);
}

static inline
int not_optimized _ksys_file_delete(const char *name)
{
    ksys70_t k;
    k.p00 = 8;
    k.p20 = 0;
    k.p21 = name;
    return _ksys_work_files(&k);
}

static inline
int not_optimized _ksys_file_rename(const char *name, const char *new_name)
{
    ksys70_t k;
    k.p00 = 10;
    k.new_name = new_name;
    k.p20 = 0;
    k.p21 = name;
    return _ksys_work_files(&k);
}


static inline
int not_optimized _ksys_exec(char *app_name, char *args){
    ksys70_t file_op;
    file_op.p00 = 7;
    file_op.p04dw = 0;
    file_op.p08dw = (unsigned)args;
    file_op.p21 = app_name;
    register int val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(70), "b"(&file_op)
    );
    return val;
}

#endif // _KSYS_H_