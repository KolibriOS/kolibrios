#ifndef SDL_kos
#define SDL_kos

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char       __u8;
typedef unsigned short      __u16;
typedef unsigned long       __u32;

struct process_table_entry
{
 __u32 cpu_usage;
 __u16 pos_in_windowing_stack;
 __u16 win_stack_val_at_ecx;
 __u16 reserved1;
 char name[12];
 __u32 memstart;
 __u32 memused;
 __u32 pid;
 __u32 winx_start,winy_start;
 __u32 winx_size,winy_size;
 __u16 thread_state;
 __u16 reserved2;
 __u32 client_left,client_top,client_width,client_height;
 __u8 window_state;
 __u8 reserved3[1024-71];
};

static inline 
void __kos__define_window(__u16 x1,__u16 y1,__u16 xsize,__u16 ysize,
     __u32 body_color,__u32 grab_color,__u32 frame_color)
{
 __u32 a,b;
 a=(x1<<16)|xsize;
 b=(y1<<16)|ysize;
 __asm__ __volatile__("int $0x40"::"a"(0),"b"(a),"c"(b),"d"(body_color),"S"(grab_color),
                      "D"(frame_color));
}

static inline void __kos__window_redraw(int status)
{
 __asm__ __volatile__("int $0x40"::"a"(12),"b"(status));
}

static inline
void __kos__draw_bitmap(void *bitmap, int x, int y, int w, int h)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(7), "b"(bitmap),
      "c"((w << 16) | h),
      "d"((x << 16) | y));
}

static inline
int __kos__getkey(void)
{
 __u16 __ret;
 __asm__ __volatile__("int $0x40":"=a"(__ret):"0"(2));
 if(!(__ret & 0xFF)) return (__ret>>8)&0xFF; else return 0;
}

static inline
int __kos__check_for_event(void)
{
 __u32 __ret;
 __asm__ __volatile__("int $0x40":"=a"(__ret):"0"(11));
 return __ret;
}

static inline
int __kos__set_events_mask(__u32 mask)
{
    register __u32 val;
    asm volatile ("int $0x40":"=a"(val):"a"(40), "b"(mask));
    return val;
}

static inline
void __kos__delay100(int time)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(5), "b"(time)
    :"memory");
};

static inline
int __kos__get_button_id()
{
    __u32 val;
    __asm__ __volatile__(
        "int $0x40"
        :"=a"(val)
        :"a"(17)
    );
    return val>>8;
}

#ifdef __cplusplus
}
#endif

#endif