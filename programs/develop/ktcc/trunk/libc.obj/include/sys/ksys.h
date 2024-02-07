#ifndef _KSYS_H_
#define _KSYS_H_

/* Copyright (C) KolibriOS team 2004-2021. All rights reserved. */
/* Distributed under terms of the GNU General Public License    */

/* This file contains basic wrappers over KolibriOS system calls. */
/* See sysfuncs.txt file for details. */

/*
 * This file was created with you in mind. Lest you reinvent the wheel.
 * If for some reason there is not enough wrapper add! I ask you to stick to the same style: snake_case.
 * Structure names must start with "ksys_" and end with "_t".
 * All wrappers must start with the "_ksys_" prefix.
 * I consider it mandatory to place the wrappers in the correct order in the official documentation.
 * Enjoy writing your code :)

 * Warning! The end of the file is the old definitions of function/structure names.
 * They are for compatibility... Better not to use them.
*/

#include <stddef.h>
#include <stdint.h>

#define KOSAPI     static inline
#define asm_inline __asm__ __volatile__

/*============== General structures ==============*/

#pragma pack(push, 1)

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} rgb_t;

typedef union {
    uint32_t val;
    struct {
        uint8_t hour;
        uint8_t min;
        uint8_t sec;
        uint8_t _zero;
    };
} ksys_time_t;

typedef union {
    uint32_t val;
    struct {
        uint8_t year;
        uint8_t month;
        uint8_t day;
        uint8_t _zero;
    };
} ksys_date_t;

typedef union {
    uint32_t val;
    struct {
        int16_t y;
        int16_t x;
    };
} ksys_pos_t;

typedef union ksys_oskey_t {
    uint32_t val;
    struct {
        uint8_t state;
        uint8_t code;
        uint8_t ctrl_key;
    };
} ksys_oskey_t;

typedef struct {
    void* data;
    size_t size;
} ksys_ufile_t;

typedef struct {
    uint32_t p00;
    union {
        uint64_t p04;
        struct {
            uint32_t p04dw;
            uint32_t p08dw;
        };
    };
    uint32_t p12;
    union {
        uint32_t p16;
        const char* new_name;
        void* bdfe;
        void* buf16;
        const void* cbuf16;
    };
    char p20;
    const char* p21;
} ksys70_t;

typedef struct {
    uint32_t status;
    uint32_t rw_bytes;
} ksys70_status_t;

typedef struct {
    uint32_t attributes;
    uint32_t name_cp;
    ksys_time_t creation_time;
    ksys_date_t creation_date;
    ksys_time_t last_access_time;
    ksys_date_t last_access_date;
    ksys_time_t last_modification_time;
    ksys_date_t last_modification_date;
    uint64_t size;
    char name[0];
} ksys_bdfe_t;

#define KSYS_THREAD_INFO_SIZE 1024

typedef union {
    struct {
        uint32_t cpu_usage;             // CPU usage (cycles per secondgoes)
        uint16_t pos_in_window_stack;   // position of the thread window in the window stack
        uint16_t slot_num_window_stack; // slot number in window stack
        uint16_t __reserved1;           // reserved
        char name[12];                  // process/thread name
        uint32_t memstart;              // process address in memory
        uint32_t memused;               // used memory size - 1
        int pid;                        // identifier (PID/TID)
        int winx_start;                 // window x-coordinate
        int winy_start;                 // window y-coordinate
        int winx_size;                  // x-axis flow window size
        int winy_size;                  // y-axis flow window size
        uint16_t slot_state;            // thread slot state
        uint16_t __reserved2;           // reserved
        int clientx;                    // client area start coordinate x-axis
        int clienty;                    // client area start coordinate y-axis
        int clientwidth;                // client area width
        int clientheight;               // client area height
        uint8_t window_state;           // window state - bitfield
        uint8_t event_mask;             // event mask
        uint8_t key_input_mode;         // keyboard input mode
    };
    uint8_t __reserved3[KSYS_THREAD_INFO_SIZE];
} ksys_thread_t;

typedef unsigned int ksys_color_t;

typedef struct {
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
} ksys_colors_table_t;

typedef struct {
    unsigned pid;     // PID of sending thread
    unsigned datalen; // data bytes
    char* data;       // data begin
} ksys_ipc_msg;

typedef struct {
    unsigned lock;      // nonzero is locked
    unsigned used;      // used bytes in buffer
    ksys_ipc_msg* data; // data begin
} ksys_ipc_buffer;

typedef struct {
    char* func_name;
    void* func_ptr;
} ksys_dll_t;

typedef unsigned ksys_drv_hand_t;

typedef struct {
    ksys_drv_hand_t handler;
    unsigned func_num;
    void* in_data_ptr;
    unsigned in_data_size;
    void* out_data_ptr;
    unsigned out_data_size;
} ksys_ioctl_t;

typedef struct {
    char key[64];
    char path[64];
} ksys_dir_key_t;

typedef union {
    uint8_t raw_data[24];
    struct {
        uint32_t id;
        uint8_t data[20];
    };
} ksys_signal_info_t;

typedef struct {
    int32_t dst_x;
    int32_t dst_y;
    uint32_t dst_w;
    uint32_t dst_h;
    int32_t src_x;
    int32_t src_y;
    uint32_t src_w;
    uint32_t src_h;
    uint32_t depth;
    uint32_t pitch;
} ksys_blitter_params_t;

#pragma pack(pop)

typedef rgb_t ksys_bitmap_t;

enum KSYS_FS_ERRORS {
    KSYS_FS_ERR_SUCCESS = 0, // Success
    KSYS_FS_ERR_1 = 1,       // Base and/or partition of a hard disk is not defined (fn21.7 & fn21.8)
    KSYS_FS_ERR_2 = 2,       // Function is not supported for the given file system
    KSYS_FS_ERR_3 = 3,       // Unknown file system
    KSYS_FS_ERR_4 = 4,       // Reserved, is never returned in the current implementation
    KSYS_FS_ERR_5 = 5,       // File not found
    KSYS_FS_ERR_EOF = 6,     // End of file, EOF
    KSYS_FS_ERR_7 = 7,       // Pointer lies outside of application memory
    KSYS_FS_ERR_8 = 8,       // Disk is full
    KSYS_FS_ERR_9 = 9,       // FAT table is destroyed
    KSYS_FS_ERR_10 = 10,     // Access denied
    KSYS_FS_ERR_11 = 11      // Device error
};

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

enum KSYS_FILE_ENCODING {
    KSYS_FILE_CP866 = 1,
    KSYS_FILE_UTF16LE = 2,
    KSYS_FILE_UTF8 = 3
};

enum KSYS_TITLE_ENCODING {
    KSYS_TITLE_CP866 = 1,
    KSYS_TITLE_UTF16LE = 2,
    KSYS_TITLE_UTF8 = 3
};

enum KSYS_SCANCODES {
    KSYS_SCANCODE_ANY = 0,
    KSYS_SCANCODE_ESC = 1,
    KSYS_SCANCODE_1 = 2,
    KSYS_SCANCODE_2 = 3,
    KSYS_SCANCODE_3 = 4,
    KSYS_SCANCODE_4 = 5,
    KSYS_SCANCODE_5 = 6,
    KSYS_SCANCODE_6 = 7,
    KSYS_SCANCODE_7 = 8,
    KSYS_SCANCODE_8 = 9,
    KSYS_SCANCODE_9 = 10,
    KSYS_SCANCODE_0 = 11,
    KSYS_SCANCODE_MINUS = 12,
    KSYS_SCANCODE_EQUAL = 13,
    KSYS_SCANCODE_BACKSPACE = 14,
    KSYS_SCANCODE_TAB = 15,
    KSYS_SCANCODE_Q = 16,
    KSYS_SCANCODE_W = 17,
    KSYS_SCANCODE_E = 18,
    KSYS_SCANCODE_R = 19,
    KSYS_SCANCODE_T = 20,
    KSYS_SCANCODE_Y = 21,
    KSYS_SCANCODE_U = 22,
    KSYS_SCANCODE_I = 23,
    KSYS_SCANCODE_O = 24,
    KSYS_SCANCODE_P = 25,
    KSYS_SCANCODE_LBRACE = 26,
    KSYS_SCANCODE_RBRACE = 27,
    KSYS_SCANCODE_ENTER = 28,
    KSYS_SCANCODE_EXT_NUMPAD_ENTER = 28,
    KSYS_SCANCODE_LCTRL = 29,
    KSYS_SCANCODE_EXT_RCTRL = 29,
    KSYS_SCANCODE_A = 30,
    KSYS_SCANCODE_S = 31,
    KSYS_SCANCODE_D = 32,
    KSYS_SCANCODE_F = 33,
    KSYS_SCANCODE_G = 34,
    KSYS_SCANCODE_H = 35,
    KSYS_SCANCODE_J = 36,
    KSYS_SCANCODE_K = 37,
    KSYS_SCANCODE_L = 38,
    KSYS_SCANCODE_SEMICOLON = 39,
    KSYS_SCANCODE_SQUOTE = 40,
    KSYS_SCANCODE_BQUOTE = 41,
    KSYS_SCANCODE_LSHIFT = 42,
    KSYS_SCANCODE_BACKSLASH = 43,
    KSYS_SCANCODE_Z = 44,
    KSYS_SCANCODE_X = 45,
    KSYS_SCANCODE_C = 46,
    KSYS_SCANCODE_V = 47,
    KSYS_SCANCODE_B = 48,
    KSYS_SCANCODE_N = 49,
    KSYS_SCANCODE_M = 50,
    KSYS_SCANCODE_COMMA = 51,
    KSYS_SCANCODE_POINT = 52,
    KSYS_SCANCODE_SLASH = 53,
    KSYS_SCANCODE_EXT_NUMPAD_DIV = 53,
    KSYS_SCANCODE_RSHIFT = 54,
    KSYS_SCANCODE_NUMPAD_MULT = 55,
    KSYS_SCANCODE_EXT_PRTSCR = 55,
    KSYS_SCANCODE_LALT = 56,
    KSYS_SCANCODE_EXT_RALT = 56,
    KSYS_SCANCODE_SPACE = 57,
    KSYS_SCANCODE_CAPSLOCK = 58,
    KSYS_SCANCODE_F1 = 59,
    KSYS_SCANCODE_F2 = 60,
    KSYS_SCANCODE_F3 = 61,
    KSYS_SCANCODE_F4 = 62,
    KSYS_SCANCODE_F5 = 63,
    KSYS_SCANCODE_F6 = 64,
    KSYS_SCANCODE_F7 = 65,
    KSYS_SCANCODE_F8 = 66,
    KSYS_SCANCODE_F9 = 67,
    KSYS_SCANCODE_F10 = 68,
    KSYS_SCANCODE_NUMLOCK = 69,
    KSYS_SCANCODE_SCRLOCK = 70,
    KSYS_SCANCODE_NUMPAD_7 = 71,
    KSYS_SCANCODE_EXT_HOME = 71,
    KSYS_SCANCODE_NUMPAD_8 = 72,
    KSYS_SCANCODE_EXT_UP = 72,
    KSYS_SCANCODE_NUMPAD_9 = 73,
    KSYS_SCANCODE_EXT_PGUP = 73,
    KSYS_SCANCODE_NUMPAD_MINUS = 74,
    KSYS_SCANCODE_NUMPAD_4 = 75,
    KSYS_SCANCODE_EXT_LEFT = 75,
    KSYS_SCANCODE_NUMPAD_5 = 76,
    KSYS_SCANCODE_NUMPAD_6 = 77,
    KSYS_SCANCODE_EXT_RIGHT = 77,
    KSYS_SCANCODE_NUMPAD_PLUS = 78,
    KSYS_SCANCODE_NUMPAD_1 = 79,
    KSYS_SCANCODE_EXT_END = 79,
    KSYS_SCANCODE_NUMPAD_2 = 80,
    KSYS_SCANCODE_EXT_DOWN = 80,
    KSYS_SCANCODE_NUMPAD_3 = 81,
    KSYS_SCANCODE_EXT_PGDOWN = 81,
    KSYS_SCANCODE_NUMPAD_0 = 82,
    KSYS_SCANCODE_EXT_INSERT = 82,
    KSYS_SCANCODE_NUMPAD_COMMA = 83,
    KSYS_SCANCODE_EXT_DELETE = 83,
    //84-86 doesn't exist
    KSYS_SCANCODE_F11 = 87,
    KSYS_SCANCODE_F12 = 88,
    //89,90 doesn't exist
    KSYS_SCANCODE_EXT_LWIN = 91,
    KSYS_SCANCODE_EXT_RWIN = 92,
    KSYS_SCANCODE_EXT_MENU = 93,
    KSYS_SCANCODE_EXT = 0xE0,

    KSYS_SCANCODE_UNK_M_UP = 250 // Is it needed?
};

KOSAPI int __strcmp(const char* s1, const char* s2)
{
    while ((*s1) && (*s1 == *s2)) {
        ++s1;
        ++s2;
    }
    return (*(unsigned char*)s1 - *(unsigned char*)s2);
}

/* ####################################################################### */
/* ############### С wrappers for system calls Kolibri OS ################ */
/* ####################################################################### */

/*=============== Function -1 - terminate thread/process ===============*/

KOSAPI void _ksys_exit(void)
{
    asm_inline("int $0x40" ::"a"(-1));
}

/*============== Function 0 - define and draw the window. ==============*/

KOSAPI void _ksys_create_window(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const char* name, ksys_color_t workcolor, uint32_t style)
{
    asm_inline(
        "int $0x40"
        :
        : "a"(0),
        "b"((x << 16) | ((w - 1) & 0xFFFF)),
        "c"((y << 16) | ((h - 1) & 0xFFFF)),
        "d"((style << 24) | (workcolor & 0xFFFFFF)),
        "D"(name),
        "S"(0)
        : "memory");
}

/*================ Function 1 - put pixel in the window. ===============*/

KOSAPI void _ksys_draw_pixel(uint32_t x, uint32_t y, ksys_color_t color)
{
    asm_inline("int $0x40" ::"a"(1), "b"(x), "c"(y), "d"(color));
}

/*============ Function 2 - get the code of the pressed key. ===========*/

KOSAPI ksys_oskey_t _ksys_get_key(void)
{
    ksys_oskey_t val;
    asm_inline(
        "int $0x40"
        : "=a"(val.val)
        : "a"(2));
    return val;
}

/*==================== Function 3 - get system time. ===================*/

KOSAPI ksys_time_t _ksys_get_time(void)
{
    ksys_time_t c_time;
    asm_inline(
        "int $0x40"
        : "=a"(c_time)
        : "a"(3)
        : "memory");
    return c_time;
}

/*=================== Function 4 - draw text string. ===================*/
KOSAPI void _ksys_draw_text(const char* text, uint32_t x, uint32_t y, uint32_t len, ksys_color_t color)
{
    asm_inline(
        "int $0x40"
        :
        : "a"(4), "d"(text),
        "b"((x << 16) | y),
        "S"(len), "c"(color)
        : "memory");
}

/*========================= Function 5 - delay. ========================*/

KOSAPI void _ksys_delay(uint32_t time)
{
    asm_inline(
        "int $0x40"
        :
        : "a"(5), "b"(time)
        : "memory");
}

/*=============== Function 7 - draw image in the window. ===============*/

KOSAPI void _ksys_draw_bitmap(void* bitmap, int x, int y, int w, int h)
{
    asm_inline(
        "int $0x40"
        :
        : "a"(7), "b"(bitmap),
        "c"((w << 16) | h),
        "d"((x << 16) | y)
        : "memory");
}

/*=============== Function 8 - define/delete the button. ===============*/

KOSAPI void _ksys_define_button(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t id, ksys_color_t color)
{
    asm_inline(
        "int $0x40"
        :
        : "a"(8),
        "b"((x << 16) + w),
        "c"((y << 16) + h),
        "d"(id),
        "S"(color));
};

KOSAPI void _ksys_delete_button(uint32_t id)
{
    asm_inline("int $0x40" ::"a"(8), "d"(id & 0x00FFFFFF | 0x80000000));
}

/*============ Function 9 - information on execution thread. ===========*/

#define KSYS_THIS_SLOT -1

enum KSYS_SLOT_STATES {
    KSYS_SLOT_STATE_RUNNING = 0,
    KSYS_SLOT_STATE_SUSPENDED = 1,
    KSYS_SLOT_STATE_SUSPENDED_WAIT_EVENT = 2,
    KSYS_SLOT_STATE_NORMAL_TERM = 3,
    KSYS_SLOT_STATE_EXCEPT_TERM = 4,
    KSYS_SLOT_STATE_WAIT_EVENT = 5,
    KSYS_SLOT_STATE_FREE = 9
};

KOSAPI int _ksys_thread_info(ksys_thread_t* table, int slot)
{
    int val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(9), "b"(table), "c"(slot)
        : "memory");
    return val;
}

/*==================== Function 10 - wait for event. ===================*/

KOSAPI uint32_t _ksys_wait_event(void)
{
    uint32_t val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(10));
    return val;
}

/*=============== Function 11 - check for event, no wait. ==============*/

KOSAPI uint32_t _ksys_check_event(void)
{
    uint32_t val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(11));
    return val;
}

/*=============== Function 12 - begin/end window redraw. ===============*/

KOSAPI void _ksys_start_draw(void)
{
    asm_inline("int $0x40" ::"a"(12), "b"(1));
}

KOSAPI void _ksys_end_draw(void)
{
    asm_inline("int $0x40" ::"a"(12), "b"(2));
}

/*============ Function 13 - draw a rectangle in the window. ===========*/

KOSAPI void _ksys_draw_bar(uint32_t x, uint32_t y, uint32_t w, uint32_t h, ksys_color_t color)
{
    asm_inline(
        "int $0x40"
        :
        : "a"(13), "d"(color),
        "b"((x << 16) | w),
        "c"((y << 16) | h));
}

/*=================== Function 14 - get screen size. ===================*/

KOSAPI ksys_pos_t _ksys_screen_size(void)
{
    ksys_pos_t size;
    asm_inline(
        "int $0x40"
        : "=a"(size)
        : "a"(14));
    return size;
}

/*== Function 15, subfunction 1 - set a size of the background image. ==*/

KOSAPI void _ksys_bg_set_size(uint32_t w, uint32_t h)
{
    asm_inline("int $0x40" ::"a"(15), "b"(1), "c"(w), "d"(h));
}

/*=== Function 15, subfunction 2 - put pixel on the background image. ==*/

KOSAPI void _ksys_bg_put_pixel(uint32_t x, uint32_t y, uint32_t w, ksys_color_t color)
{
    asm_inline("int $0x40" ::"a"(15), "b"(2), "c"((x + y * w) * 3), "d"(color));
}

/*=========== Function 15, subfunction 3 - redraw background. ==========*/

KOSAPI void _ksys_bg_redraw(void)
{
    asm_inline("int $0x40" ::"a"(15), "b"(3));
}

/*== Function 15, subfunction 4 - set drawing mode for the background. =*/

enum KSYS_BG_MODES {
    KSYS_BG_MODE_PAVE = 1,
    KSYS_BG_MODE_STRETCH = 2
};

KOSAPI void _ksys_bg_set_mode(uint32_t mode)
{
    asm_inline("int $0x40" ::"a"(15), "b"(4), "c"(mode));
}

/*===================== Function 15, subfunction 5 =====================*/
/*============ Put block of pixels on the background image. ============*/

KOSAPI void _ksys_bg_put_bitmap(ksys_bitmap_t* bitmap, size_t bitmap_size, uint32_t x, uint32_t y, uint32_t w)
{
    asm_inline(
        "int $0x40"
        :
        : "a"(15), "b"(5), "c"(bitmap), "d"((x + y * w) * 3), "S"(bitmap_size));
}

/*===================== Function 15, subfunction 6 =====================*/
/*======= Map background data to the address space of process. ==========*/

KOSAPI ksys_bitmap_t* _ksys_bg_get_map(void)
{
    ksys_bitmap_t* bitmap;
    asm_inline(
        "int $0x40"
        : "=a"(bitmap)
        : "a"(15), "b"(6));
    return bitmap;
}

/*===== Function 15, subfunction 7 - close mapped background data. =====*/

KOSAPI int _ksys_bg_close_map(ksys_bitmap_t* bitmap)
{
    int status; // 1 - OK, 0 - ERROR
    asm_inline(
        "int $0x40"
        : "=a"(status)
        : "a"(15), "b"(7), "c"(bitmap));
    return status;
}

/*===================== Function 15, subfunction 9 =====================*/
/*============= Redraws a rectangular part of the background ===========*/

KOSAPI void _ksys_bg_redraw_bar(ksys_pos_t angle1, ksys_pos_t angle2)
{
    asm_inline(
        "int $0x40"
        :
        : "a"(15), "b"(9),
        "c"(angle1.x * (1 << 16) + angle2.x),
        "d"(angle1.y * (1 << 16) + angle2.y));
}

/*=============== Function 16 - save ramdisk on a floppy. ==============*/

KOSAPI int _ksys_save_ramdisk_fd(uint32_t floppy_id)
{
    int status; // 0 - OK, 1 - ERROR
    asm_inline(
        "int $0x40"
        : "=a"(status)
        : "a"(16), "b"(floppy_id));
    return status;
}

/*======= Function 17 - get the identifier of the pressed button. ======*/

KOSAPI uint32_t _ksys_get_button(void)
{
    unsigned val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(17));
    return val >> 8;
}

/*===================== Function 18, subfunction 1 =====================*/
/*============= Make deactive the window of the given thread. ==========*/

KOSAPI void _ksys_unfocus_window(int slot)
{
    asm_inline("int $0x40" ::"a"(18), "b"(1), "c"(slot));
}

/*= Function 18, subfunction 2 - terminate process/thread by the slot. =*/

KOSAPI void _ksys_kill_by_slot(int slot)
{
    asm_inline(
        "int $0x40" ::"a"(18), "b"(2), "c"(slot));
}

/*===================== Function 18, subfunction 3 =====================*/
/*============= Make active the window of the given thread. ============*/

KOSAPI void _ksys_focus_window(int slot)
{
    asm_inline("int $0x40" ::"a"(18), "b"(3), "c"(slot));
}

/*===================== Function 18, subfunction 4 =====================*/
/*=========== Get counter of idle time units per one second. ===========*/

KOSAPI uint32_t _ksys_get_idle(void)
{
    uint32_t sec;
    asm_inline(
        "int $0x40"
        : "=a"(sec)
        : "a"(18), "b"(4));
    return sec;
}

/*========== Function 18, subfunction 5 - get CPU clock rate. ==========*/
/*================ modulo 2^32 clock ticks = 4GHz ======================*/

KOSAPI uint32_t _ksys_get_cpu_clock(void)
{
    uint32_t clock;
    asm_inline(
        "int $0x40"
        : "=a"(clock)
        : "a"(18), "b"(5));
    return clock;
}

/* Function 18, subfunction 6 - save ramdisk to the file on hard drive. */

KOSAPI uint32_t _ksys_save_ramdisk_hd(const char* ramdisk_path)
{
    uint32_t fs_err;
    asm_inline(
        "int $0x40"
        : "=a"(fs_err)
        : "a"(18), "b"(6), "c"(ramdisk_path));
    return fs_err;
}

/* Function 18, subfunction 9 - system shutdown with the parameter. */

enum KSYS_SHD_PARAM {
    KSYS_SHD_POWEROFF = 2,
    KSYS_SHD_REBOOT = 3,
    KSYS_SHD_RESTART_KRN = 4
};

KOSAPI void _ksys_shutdown(uint32_t shd_param)
{
    asm_inline("int $0x40" ::"a"(18), "b"(9), "c"(shd_param));
}

/*========= Function 18, subfunction 16 - get size of free RAM. ========*/

KOSAPI size_t _ksys_get_ram_size(void)
{
    size_t size;
    asm_inline(
        "int $0x40"
        : "=a"(size)
        : "a"(18), "b"(16));
    return size;
}

/*======== Function 18, subfunction 17 - get full amount of RAM. =======*/

KOSAPI size_t _ksys_get_full_ram(void)
{
    size_t size;
    asm_inline(
        "int $0x40"
        : "=a"(size)
        : "a"(18), "b"(17));
    return size;
}

/*===================== Function 18, subfunction 18 ====================*/
/*============= Terminate process/thread by the identifier. ============*/

KOSAPI void _ksys_kill_by_pid(uint32_t PID)
{
    asm_inline("int $0x40" ::"a"(18), "b"(18), "c"(PID));
}

/*========= Fuction 18, subfunction 19 - get/set mouse settings. ========*/

typedef enum KSYS_MOUSE_SETTINGS {
    KSYS_MOUSE_GET_SPEED = 0,              // Get mouse speed
    KSYS_MOUSE_SET_SPEED = 1,              // Set mouse speed
    KSYS_MOUSE_GET_SENS = 2,               // Get mouse sensitivity
    KSYS_MOUSE_SET_SENS = 3,               // Set mouse sensitivity
    KSYS_MOUSE_SET_POS = 4,                // Set the position of the mouse cursor
    KSYS_MOUSE_SIM_STATE = 5,              // Simulate the state of the mouse buttons
    KSYS_MOUSE_GET_DOUBLE_CLICK_DELAY = 6, // Get double click delay.
    KSYS_MOUSE_SET_DOUBLE_CLICK_DELAY = 7  // Set double click delay.
} ksys_mouse_settings_t;

KOSAPI uint32_t _ksys_set_mouse_settings(ksys_mouse_settings_t settings, uint32_t arg)
{
    uint32_t retval;
    asm_inline(
        "int $0x40"
        : "=a"(retval)
        : "a"(18), "b"(19), "c"(settings), "d"(arg)
        : "memory");
    return retval;
}

#define _ksys_set_mouse_pos(X, Y) _ksys_set_mouse_settings(KSYS_MOUSE_SET_POS, X * 65536 + Y)

/*===================== Function 18, subfunction 21 ====================*/
/*=====Get the slot number of the process / thread by identifier.. =====*/

KOSAPI int _ksys_get_thread_slot(int PID)
{
    int val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(18), "b"(21), "c"(PID));
    return val;
}

/*============= Function 23 - wait for event with timeout. =============*/

KOSAPI uint32_t _ksys_wait_event_timeout(uint32_t timeout)
{
    unsigned val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(23), "b"(timeout));
    return val;
}

/*=== Function 26, subfunction 9 - get the value of the time counter. ==*/

KOSAPI uint32_t _ksys_get_tick_count(void)
{
    unsigned val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(26), "b"(9));
    return val;
}

/*===================== Function 26, subfunction 10 ====================*/
/*========== Get the value of the high precision time counter. =========*/

KOSAPI uint64_t _ksys_get_ns_count(void)
{
    uint64_t val;
    asm_inline(
        "int $0x40"
        : "=A"(val)
        : "a"(26), "b"(10));
    return val;
}

/*=================== Function 29 - get system date. ===================*/

KOSAPI ksys_date_t _ksys_get_date(void)
{
    ksys_date_t val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(29));
    return val;
}

/*===========+ Function 30 - work with the current folder.==============*/
/*--------- Subfunction 1 - set current folder for the thread. ---------*/

KOSAPI void _ksys_setcwd(char* dir)
{
    asm_inline("int $0x40" ::"a"(30), "b"(1), "c"(dir));
}

/*--------- Subfunction 2 - get current folder for the thread. ---------*/

KOSAPI int _ksys_getcwd(char* buf, int bufsize)
{
    int val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(30), "b"(2), "c"(buf), "d"(bufsize));
    return val;
}

/* ---- Subfunction 3 - install the add.system directory for the kernel ------*/

KOSAPI int _ksys_set_kernel_dir(ksys_dir_key_t* table)
{
    int val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(30), "b"(3), "c"(table)
        : "memory");
    return val;
}

/*=================== Function 37 - work with mouse. ===================*/

enum KSYS_MOUSE_POS {
    KSYS_MOUSE_SCREEN_POS = 0,
    KSYS_MOUSE_WINDOW_POS = 1
};

KOSAPI ksys_pos_t _ksys_get_mouse_pos(int origin)
{
    ksys_pos_t pos;
    asm_inline(
        "int $0x40"
        : "=a"(pos)
        : "a"(37), "b"(origin));
    return pos;
}

enum KSYS_MOUSE_BUTTON {
    KSYS_MOUSE_LBUTTON_PRESSED = (1 << 0),
    KSYS_MOUSE_RBUTTON_PRESSED = (1 << 1),
    KSYS_MOUSE_MBUTTON_PRESSED = (1 << 2),
    KSYS_MOUSE_4BUTTON_PRESSED = (1 << 3),
    KSYS_MOUSE_5BUTTON_PRESSED = (1 << 4)
};

KOSAPI uint32_t _ksys_get_mouse_buttons(void) // subfunction 2 - states of the mouse buttons
{
    uint32_t val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(37), "b"(2));
    return val;
}

KOSAPI uint32_t _ksys_get_mouse_eventstate(void) // subfunction 3 - states and events of the mouse buttons
{
    uint32_t val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(37), "b"(3));
    return val;
}

enum KSYS_CURSOR_SRC {
    KSYS_CURSOR_FROM_FILE = 0,
    KSYS_CURSOR_FROM_MEM = 1,
    KSYS_CURSOR_INDIRECT = 2
};

KOSAPI void* _ksys_load_cursor(void* path, uint32_t flags) // subfunction 4 - load cursor
{
    void* val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(37), "b"(4), "c"(path), "d"(flags)
        : "memory");
    return val;
}

KOSAPI void* _ksys_set_cursor(void* cursor) // subfunction 5 - set cursor
{
    void* old;
    asm_inline(
        "int $0x40"
        : "=a"(old)
        : "a"(37), "b"(5), "c"(cursor));
    return old;
}

KOSAPI int _ksys_delete_cursor(void* cursor) // subfunction 6 - delete cursor
{
    int ret;
    asm_inline(
        "int $0x40"
        : "=a"(ret)
        : "a"(37), "b"(6), "c"(cursor)
        : "memory");
    return ret;
}

KOSAPI uint32_t _ksys_get_mouse_wheels(void) // subfunction 7 - get scroll data
{
    uint32_t val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(37), "b"(7));
    return val;
}

/*=========== Function 40 - set the mask for expected events. ==========*/

enum KSYS_EVENT_MASK {
    KSYS_EVM_REDRAW = 1,
    KSYS_EVM_KEY = 2,
    KSYS_EVM_BUTTON = 4,
    KSYS_EVM_EXIT = 8,
    KSYS_EVM_BACKGROUND = 16,
    KSYS_EVM_MOUSE = 32,
    KSYS_EVM_IPC = 64,
    KSYS_EVM_STACK = 128,
    KSYS_EVM_DEBUG = 256,
    KSYS_EVM_STACK2 = 512,
    KSYS_EVM_MOUSE_FILTER = 0x80000000,
    KSYS_EVM_CURSOR_FILTER = 0x40000000,
};

KOSAPI uint32_t _ksys_set_event_mask(uint32_t mask)
{
    unsigned val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(40), "b"(mask));
    return val;
}

/*====================== Function 38 - draw line. ======================*/

KOSAPI void _ksys_draw_line(int xs, int ys, int xe, int ye, ksys_color_t color)
{
    asm_inline(
        "int $0x40"
        :
        : "a"(38), "d"(color),
        "b"((xs << 16) | xe),
        "c"((ys << 16) | ye));
}

/*============= Function 47 - draw a number in the window. =============*/

KOSAPI void _ksys_draw_number(int number, int x, int y, int len, ksys_color_t color)
{
    unsigned fmt;
    fmt = len << 16 | 0x80000000; // no leading zeros + width
    asm_inline(
        "int $0x40"
        :
        : "a"(47), "b"(fmt), "c"(number), "d"((x << 16) | y), "S"(color));
}

KOSAPI void _ksys_draw_number_bg(unsigned number, int x, int y, int len, ksys_color_t color, ksys_color_t bg)
{
    unsigned fmt;
    fmt = len << 16 | 0x80000000; // no leading zeros + width
    asm_inline(
        "int $0x40"
        :
        : "a"(47), "b"(fmt), "c"(number), "d"((x << 16) | y), "S"(color), "D"(bg));
}

/*====== Function 48, subfunction 3 - get standard window colors. ======*/

KOSAPI void _ksys_get_system_colors(ksys_colors_table_t* color_table)
{
    asm_inline("int $0x40" ::"a"(48), "b"(3), "c"(color_table), "d"(40));
}

/*============ Function 48, subfunction 4 - get skin height. ===========*/

KOSAPI uint32_t _ksys_get_skin_height()
{
    unsigned height;
    asm_inline(
        "int $0x40"
        : "=a"(height)
        : "a"(48), "b"(4));
    return height;
}

/*==================== Function 51 - create thread. ====================*/

KOSAPI int _ksys_create_thread(void* thread_entry, void* stack_top)
{
    int val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(51), "b"(1), "c"(thread_entry), "d"(stack_top)
        : "memory");
    return val;
}

/*==================== Function 54, subfunction 0 ======================*/
/*============== Get the number of slots in the clipboard. =============*/

enum KSYS_CLIP_ENCODING {
    KSYS_CLIP_UTF8 = 0,
    KSYS_CLIP_CP866 = 1,
    KSYS_CLIP_CP1251 = 2
};

enum KSYS_CLIP_TYPES {
    KSYS_CLIP_TEXT = 0,
    KSYS_CLIP_IMAGE = 1,
    KSYS_CLIP_RAW = 2
};

KOSAPI int _ksys_clip_num(void)
{
    unsigned val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(54), "b"(0));
    return val;
}

/*==================== Function 54, subfunction 1 ======================*/
/*================= Read the data from the clipboard. ==================-*/

KOSAPI char* _ksys_clip_get(int n) // returned buffer must be freed by _ksys_free()
{
    char* val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(54), "b"(1), "c"(n));
    return val;
}

/*==================== Function 54, subfunction 2 ======================*/
/*================= Write the data to the clipboard. ===================*/

KOSAPI int _ksys_clip_set(int n, char* buffer)
{
    unsigned val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(54), "b"(2), "c"(n), "d"(buffer)
        : "memory");
    return val;
}

/*===================== Function 54, subfunction 3 =====================*/
/*================ Delete the last slot in the clipboard ===============*/

KOSAPI int _ksys_clip_pop()
{
    unsigned val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(54), "b"(3));
    return val;
}

/*===================== Function 54, subfunction 4 =====================*/
/*===================== Alarm reset the lock buffer ====================*/

KOSAPI int _ksys_clip_unlock()
{
    unsigned val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(54), "b"(4));
    return val;
}

/*============== Function 63 - work with the debug board. ==============*/

KOSAPI void _ksys_debug_putc(char c)
{
    asm_inline("int $0x40" ::"a"(63), "b"(1), "c"(c));
}

KOSAPI void _ksys_debug_puts(const char* s)
{
    unsigned i = 0;
    while (*(s + i)) {
        asm_inline("int $0x40" ::"a"(63), "b"(1), "c"(*(s + i)));
        i++;
    }
}

/*========= Function 65 - draw image with the palette in window. =============*/

KOSAPI void ksys_draw_bitmap_palette(void* bitmap, int x, int y, int w, int h, int bpp, void* palette, int offset)
{
    asm_inline(
        "pushl %%ebp,\n\t"            // save EBP register
        "movl 0x24(%%ebp), %%ebp\n\t" // 0x24 - "offset" param
        "int $0x40\n\t"
        "popl %%ebp" // restore EBP register
        :
        : "a"(65),
        "b"(bitmap),
        "c"((w << 16) + h),
        "d"((x << 16) + y),
        "S"(bpp),
        "D"(palette));
}

/*========= Function 66, subfunction 1 - set keyboard input mode. ==============*/

typedef enum KSYS_KEY_INPUT_MODE {
    KSYS_KEY_INPUT_MODE_ASCII = 0,
    KSYS_KEY_INPUT_MODE_SCANC = 1,
} ksys_key_input_mode_t;

KOSAPI void _ksys_set_key_input_mode(ksys_key_input_mode_t mode)
{
    asm_inline("int $0x40" ::"a"(66), "b"(1), "c"(mode));
}

/*========= Function 66, subfunction 3 - get the state of the control keys. ========*/

enum KSYS_CONTROL_KEYS {
    KSYS_CONTROL_LSHIFT = (1 << 0),
    KSYS_CONTROL_RSHIFT = (1 << 1),
    KSYS_CONTROL_LCTRL = (1 << 2),
    KSYS_CONTROL_RCTRL = (1 << 3),
    KSYS_CONTROL_LALT = (1 << 4),
    KSYS_CONTROL_RALT = (1 << 5),
    KSYS_CONTROL_CAPS = (1 << 6),
    KSYS_CONTROL_NUM_LOCK = (1 << 7),
    KSYS_CONTROL_SCROLL_LOCK = (1 << 8)
};

KOSAPI uint32_t _ksys_get_control_key_state(void)
{
    uint32_t key_state;
    asm_inline(
        "int $0x40"
        : "=a"(key_state)
        : "a"(66), "b"(3));
    return key_state;
}

/*========= Function 66, subfunction 4 - set system-wide hotkey. ========*/

enum KSYS_SYS_HOTKEY_CONTROL_KEY_STATES {
    KSYS_SYS_HOTKEY_SHIFT_NONE = 0x0,
    KSYS_SYS_HOTKEY_SHIFT_ONE = 0x1,
    KSYS_SYS_HOTKEY_SHIFT_BOTH = 0x2,
    KSYS_SYS_HOTKEY_SHIFT_LEFTONLY = 0x3,
    KSYS_SYS_HOTKEY_SHIFT_RIGHTONLY = 0x4,

    KSYS_SYS_HOTKEY_CTRL_NONE = 0x00,
    KSYS_SYS_HOTKEY_CTRL_ONE = 0x10,
    KSYS_SYS_HOTKEY_CTRL_BOTH = 0x20,
    KSYS_SYS_HOTKEY_CTRL_LEFTONLY = 0x30,
    KSYS_SYS_HOTKEY_CTRL_RIGHTONLY = 0x40,

    KSYS_SYS_HOTKEY_ALT_NONE = 0x000,
    KSYS_SYS_HOTKEY_ALT_ONE = 0x100,
    KSYS_SYS_HOTKEY_ALT_BOTH = 0x200,
    KSYS_SYS_HOTKEY_ALT_LEFTONLY = 0x300,
    KSYS_SYS_HOTKEY_ALT_RIGHTONLY = 0x400,
};

KOSAPI int _ksys_set_sys_hotkey(uint8_t scancode, uint16_t control_key_states)
{
    int res;
    asm_inline(
        "int $0x40"
        : "=a"(res)
        : "a"(66), "b"(4), "c"(scancode), "d"(control_key_states)
        : "memory");
    return res;
}

/*========= Function 66, subfunction 5 - delete installed hotkey. ========*/

KOSAPI int _ksys_del_sys_hotkey(uint8_t scancode, uint16_t control_key_states)
{
    int res;
    asm_inline(
        "int $0x40"
        : "=a"(res)
        : "a"(66), "b"(5), "c"(scancode), "d"(control_key_states)
        : "memory");
    return res;
}

/*========= Function 67 - change position/sizes of the window. =========*/

KOSAPI void _ksys_change_window(int new_x, int new_y, int new_w, int new_h)
{
    asm_inline("int $0x40" ::"a"(67), "b"(new_x), "c"(new_y), "d"(new_w), "S"(new_h));
}

/*===== Function 68, subfunction 1 - switch to the next thread of execution ====*/

KOSAPI void _ksys_thread_yield(void)
{
    asm_inline("int $0x40" ::"a"(68), "b"(1));
}

/*======== Function 68, subfunction 12 - allocate memory block. ========*/

KOSAPI void* _ksys_alloc(size_t size)
{
    void* val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(68), "b"(12), "c"(size));
    return val;
}

/*========== Function 68, subfunction 13 - free memory block. ==========*/

KOSAPI int _ksys_free(void* mem)
{
    int val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(68), "b"(13), "c"(mem));
    return val;
}

/*====== Function 68, subfunction 14 - Wait signal from other applications/drivers. =====*/

KOSAPI void _ksys_wait_signal(ksys_signal_info_t* signal)
{
    asm_inline(
        "int $0x40"
        :
        : "a"(68), "b"(14), "c"(signal)
        : "memory");
}

/*============= Function 68, subfunction 16 - load driver. =============*/

KOSAPI ksys_drv_hand_t _ksys_load_driver(char* driver_name)
{
    ksys_drv_hand_t driver_h;
    asm_inline(
        "int $0x40"
        : "=a"(driver_h)
        : "a"(68), "b"(16), "c"(driver_name));
    return driver_h;
}

/*============ Function 68, subfunction 17 - driver control. ===========*/

KOSAPI int _ksys_driver_control(ksys_ioctl_t* ioctl)
{
    int status;
    asm_inline(
        "int $0x40"
        : "=a"(status)
        : "a"(68), "b"(17), "c"(ioctl)
        : "memory");
    return status;
}

/*== Function 68, subfunction 18 - subfunction 19 - load DLL (MS COFF) ==*/

KOSAPI ksys_dll_t* _ksys_dlopen(const char* path)
{
    ksys_dll_t* table;
    asm_inline(
        "int $0x40"
        : "=a"(table)
        : "a"(68), "b"(19), "c"(path)
        : "memory");
    return table;
}

/* It is not a system call, it serves as an auxiliary tool*/

KOSAPI void* _ksys_dlsym(ksys_dll_t* table, const char* fun_name)
{
    unsigned i = 0;
    while (1) {
        if (!(table + i)->func_name) {
            break;
        } else {
            if (!__strcmp(fun_name, (table + i)->func_name)) {
                return (table + i)->func_ptr;
            }
        }
        i++;
    }
    return NULL;
}

/* Function 68, subfunction 20 - reallocate memory block.*/

KOSAPI void* _ksys_realloc(void* mem, size_t size)
{
    void* val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(68), "b"(20), "c"(size), "d"(mem)
        : "memory");
    return val;
}

/* Function 68, subfunction 21 - load driver by full name. */

KOSAPI ksys_drv_hand_t _ksys_load_driver_opt(char* driver_path, char* cmd_line)
{
    ksys_drv_hand_t driver_h;
    asm_inline(
        "int $0x40"
        : "=a"(driver_h)
        : "a"(68), "b"(21), "c"(driver_path), "d"(cmd_line));
    return driver_h;
}

/*======== Function 68, subfunction 22 - open named memory area. =======*/

enum KSYS_SHM_MODE {
    KSYS_SHM_OPEN = 0x00,
    KSYS_SHM_OPEN_ALWAYS = 0x04,
    KSYS_SHM_CREATE = 0x08,
    KSYS_SHM_READ = 0x00,
    KSYS_SHM_WRITE = 0x01,
};

KOSAPI int _ksys_shm_open(char* name, int mode, int size, char** new_shm)
{
    int error;
    asm_inline(
        "int $0x40"
        : "=a"(*new_shm), "=d"(error)
        : "a"(68), "b"(22), "c"(name), "d"(size), "S"(mode));
    return error;
}

/*======= Function 68, subfunction 23 - close named memory area. =======*/

KOSAPI void _ksys_shm_close(char* shm_name)
{
    asm_inline(
        "int $0x40"
        :
        : "a"(68), "b"(23), "c"(shm_name));
}

/*====== Function 68, subfunction 26 - release memory pages ============*/

KOSAPI int* _ksys_unmap(void* base, size_t offset, size_t size)
{
    int* val;
    asm_inline(
        "int $0x40"
        : "=a"(val)
        : "a"(68), "b"(26), "c"(base), "d"(offset), "S"(size));
    return val;
}

/*========== Function 68, subfunction 27 - load file ===================*/

KOSAPI ksys_ufile_t _ksys_load_file(const char* path)
{
    ksys_ufile_t uf;
    asm_inline(
        "int $0x40"
        : "=a"(uf.data), "=d"(uf.size)
        : "a"(68), "b"(27), "c"(path)
        : "memory");
    return uf;
}

/*==== Function 68, subfunction 28 - load file, specifying the encoding ===*/

KOSAPI ksys_ufile_t _ksys_load_file_enc(const char* path, unsigned file_encoding)
{
    ksys_ufile_t uf;
    asm_inline(
        "int $0x40"
        : "=a"(uf.data), "=d"(uf.size)
        : "a"(68), "b"(28), "c"(path), "d"(file_encoding)
        : "memory");
    return uf;
}

/*==== Function 70 - work with file system with long names support. ====*/

KOSAPI ksys70_status_t _ksys70(const ksys70_t* k)
{
    ksys70_status_t status;
    asm_inline(
        "int $0x40"
        : "=a"(status.status), "=b"(status.rw_bytes)
        : "a"(70), "b"(k)
        : "memory");
    return status;
}

/*====== Function 70, subfunction 0 - read file with long names support. ======*/

KOSAPI ksys70_status_t _ksys_file_read(const char* name, uint64_t offset, uint32_t size, void* buf)
{
    ksys70_t k;
    k.p00 = 0;
    k.p04 = offset;
    k.p12 = size;
    k.buf16 = buf;
    k.p20 = 0;
    k.p21 = name;
    return _ksys70(&k);
}

/*===================== Function 70, subfunction 2 =====================*/
/*============ Create/rewrite file with long names support. ============*/

KOSAPI int _ksys_file_create(const char* name)
{
    ksys70_t k;
    k.p00 = 2;
    k.p04dw = 0;
    k.p08dw = 0;
    k.p12 = 0;
    k.p21 = name;
    return _ksys70(&k).status;
}

/*===================== Function 70, subfunction 3 =====================*/
/*=========== Write to existing file with long names support. ==========*/

KOSAPI ksys70_status_t _ksys_file_write(const char* name, uint64_t offset, uint32_t size, const void* buf)
{
    ksys70_t k;
    k.p00 = 3;
    k.p04 = offset;
    k.p12 = size;
    k.cbuf16 = buf;
    k.p20 = 0;
    k.p21 = name;
    return _ksys70(&k);
}

/*========== Function 70, subfunction 5 - get information on file/folder. =====*/

KOSAPI int _ksys_file_info(const char* name, ksys_bdfe_t* bdfe)
{
    ksys70_t k;
    k.p00 = 5;
    k.p04dw = 0;
    k.p08dw = 0;
    k.p12 = 0;
    k.bdfe = bdfe;
    k.p20 = 0;
    k.p21 = name;
    return _ksys70(&k).status;
}

#define _ksys_dir_info _ksys_file_info

/*=========== Function 70, subfunction 7 - start application. ===========*/

KOSAPI int _ksys_exec(const char* app_name, char* args)
{
    ksys70_t file_opt;
    file_opt.p00 = 7;
    file_opt.p04dw = 0;
    file_opt.p08dw = (uint32_t)args;

    file_opt.p12 = 0;
    file_opt.p16 = 0;
    file_opt.p20 = 0;

    file_opt.p21 = app_name;
    return _ksys70(&file_opt).status;
}

/*========== Function 70, subfunction 8 - delete file/folder. ==========*/

KOSAPI int _ksys_file_delete(const char* name)
{
    ksys70_t k;
    k.p00 = 8;
    k.p20 = 0;
    k.p21 = name;
    return _ksys70(&k).status;
}

/*============= Function 70, subfunction 9 - create folder. =============*/

KOSAPI int _ksys_mkdir(const char* path)
{
    ksys70_t dir_opt;
    dir_opt.p00 = 9;
    dir_opt.p21 = path;
    return _ksys70(&dir_opt).status;
}

/*============= Function 70, subfunction 10 - rename/move. =============*/

KOSAPI int _ksys_file_rename(const char* name, const char* new_name)
{
    ksys70_t k;
    k.p00 = 10;
    k.new_name = new_name;
    k.p20 = 0;
    k.p21 = name;
    return _ksys70(&k).status;
}

#define _ksys_dir_rename _ksys_file_rename

/*============= Function 71, subfunction 1 - set window title =============*/

KOSAPI void _ksys_set_window_title(const char* title)
{
    asm_inline(
        "int $0x40"
        :
        : "a"(71), "b"(1), "c"(title)
        : "memory");
}

#define _ksys_clear_window_title() _ksys_set_window_title(NULL)

/*============= Function 73 - blitter =============*/

KOSAPI void _ksys_blitter(uint32_t flags, ksys_blitter_params_t *params)
{
    int res;
    asm_inline(
        "int $0x40"
        : "=a"(res)
        : "a"(73), "b"(flags), "c"(params)
        : "memory");
    (void)res;
}

/*============= Function 77, subfunction 0 - create futex object =============*/

KOSAPI uint32_t _ksys_futex_create(int* futex_ctrl)
{
    uint32_t futex_desc;
    asm_inline(
        "int $0x40"
        : "=a"(futex_desc)
        : "a"(77), "b"(0), "c"(futex_ctrl)
        : "memory");
    return futex_desc;
}

/*============= Function 77, subfunction 1 - destroy futex object =============*/

KOSAPI int _ksys_futex_destroy(uint32_t futex_desc)
{
    int res;
    asm_inline(
        "int $0x40"
        : "=a"(res)
        : "a"(77), "b"(1), "c"(futex_desc));
    return res;
}

/*============= Function 77, subfunction 2 - futex wait =============*/

KOSAPI int _ksys_futex_wait(uint32_t futex_desc, int ctrl_val, int timeout)
{
    int res;
    asm_inline(
        "int $0x40"
        : "=a"(res)
        : "a"(77), "b"(2), "c"(futex_desc), "d"(ctrl_val), "S"(timeout));
    return res;
}

/*============= Function 77, subfunction 3 - futex wake =============*/

KOSAPI int _ksys_futex_wake(uint32_t futex_desc, int max_wake_count)
{
    int count;
    asm_inline(
        "int $0x40"
        : "=a"(count)
        : "a"(77), "b"(3), "c"(futex_desc), "d"(max_wake_count));
    return count;
}

/*============= Function 77, subfunction 10 - read from file to buffer =============*/

KOSAPI int _ksys_posix_read(int pipefd, void* buff, int n)
{
    int count;
    asm_inline(
        "int $0x40"
        : "=a"(count)
        : "a"(77), "b"(10), "c"(pipefd), "d"(buff), "S"(n)
        : "memory");
    return count;
}

/*============= Function 77, subfunction 11 - write from buffer to file =============*/

KOSAPI int _ksys_posix_write(int pipefd, void* buff, int n)
{
    int count;
    asm_inline(
        "int $0x40"
        : "=a"(count)
        : "a"(77), "b"(11), "c"(pipefd), "d"(buff), "S"(n)
        : "memory");
    return count;
}

/*============= Function 77, subfunction 13 - create new pipe =============*/

KOSAPI int _ksys_posix_pipe2(int pipefd[2], int flags)
{
    int err;
    asm_inline(
        "int $0x40"
        : "=a"(err)
        : "a"(77), "b"(13), "c"(pipefd), "d"(flags)
        : "memory");
    return err;
}

/* ######### Old names of functions and structures. Do not use again! ##########*/

#define _ksys_get_event     _ksys_wait_event
#define _ksys_file_get_info _ksys_file_info

static inline int _ksys_file_read_file(const char* name, unsigned long long offset, unsigned size, void* buff, unsigned* bytes_read)
{
    ksys70_status_t res = _ksys_file_read(name, offset, size, buff);
    *bytes_read = res.rw_bytes;
    return res.status;
}

static inline int _ksys_file_write_file(const char* name, unsigned long long offset, unsigned size, const void* buff, unsigned* bytes_write)
{
    ksys70_status_t res = _ksys_file_write(name, offset, size, buff);
    *bytes_write = res.rw_bytes;
    return res.status;
}

#endif // _KSYS_H_
