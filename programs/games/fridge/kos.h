/*
sys/kos.h
An attempt to create new C/C++ wrapper for syscalls
Based on kos32sys.h
KolibriOS Team
*/

#include <stdint.h>
#include <string.h>

/*********************** Types *************************/
typedef unsigned int color_t;

// struct for sysfn 70
#pragma pack(push,1)
typedef struct {
	unsigned p00;
	unsigned p04;
	char *p08;
	unsigned p12;
	unsigned p16;
	char p20;
	char *p21;
} kos_Struct70;
#pragma pack(pop)

// struct for blitter
struct blit_call {
	int dstx;
	int dsty;
	int w;
	int h;
	int srcx;
	int srcy;
	int srcw;
	int srch;
	void *bitmap;
	int stride;
};

// struct for sysfn 9
#pragma pack(push, 1)
struct proc_info {
	unsigned long cpu_usage;
	unsigned short pos_in_stack;
	unsigned short slot;
	unsigned short reserved;
	char name[12];
	unsigned long address;
	unsigned long memory_usage;
	unsigned long ID;
	unsigned long left,top;
	unsigned long width,height;
	unsigned short thread_state;
	unsigned short reserved2;
	unsigned long cleft, ctop, cwidth, cheight;
	unsigned char window_state;
	unsigned char reserved3[1024 - 71];
};
#pragma pack(pop)

// struct for sysfn 48
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

typedef union __attribute__((packed)) {
	uint32_t val;
	struct {
		short x;
		short y;
	};
} pos_t;


/*********************** Window Syscalls *************************/
static inline void kos_BeginDraw(void) {
	__asm__ __volatile__(
	"int $0x40" ::"a"(12),"b"(1));
};

static inline void kos_EndDraw(void) {
	__asm__ __volatile__(
	"int $0x40" ::"a"(12),"b"(2));
};

static inline void kos_DrawWindow(int x, int y, int w, int h, const char *title, color_t bgcolor, uint32_t style) {
	__asm__ __volatile__(
	"int $0x40"
	::"a"(0),
	 "b"((x << 16) | ((w-1) & 0xFFFF)),
	 "c"((y << 16) | ((h-1) & 0xFFFF)),
	 "d"((style << 24) | (bgcolor & 0xFFFFFF)),
	 "D"(title),
	 "S"(0) : "memory");
};

#define ZPOS_DESKTOP     -2
#define ZPOS_ALWAYS_BACK -1
#define ZPOS_NORMAL      0
#define ZPOS_ALWAYS_TOP  1
static inline void kos_SetWindowLayerBehaviour(int zpos) {
	__asm__ __volatile__(
	"int $0x40"
	::"a"(18),
	 "b"(25),
	 "c"(2),
	 "d"(-1),
	 "S"(zpos) : "memory");
};

#define OLD -1
static inline void kos_ChangeWindow(int new_x, int new_y, int new_w, int new_h) {
	__asm__ __volatile__(
		"int $0x40"
		::"a"(67), "b"(new_x), "c"(new_y), "d"(new_w),"S"(new_h)
	);
}

/*********************** Other GUI functions *************************/
static inline void kos_DrawText(int x, int y, const char *text, color_t color) {
	__asm__ __volatile__(
	"int $0x40"
	::"a"(4),"d"(text),
	  "b"((x << 16) | y),
	  "S"(strlen(text)),"c"(color)
	 :"memory");
}

static inline void kos_DrawButton(int x, int y, int w, int h, int id, color_t color) {
	__asm__ __volatile__(
	"int $0x40"
	::"a"(8),
	  "b"(x * 65536 + w),
	  "c"(y * 65536 + h),
	  "d"(id),
	  "S"(color));
};

static inline void kos_DrawButtonWithText(int x, int y, int w, int h, int id, color_t color, const char* text) {
	kos_DrawButton(x, y, w, h, id, color);

	int tx = ((((-strlen(text))*8)+w)/2)+x;
	int ty = h/2-7+y;

	kos_DrawText(tx, ty, text, 0x90000000);
};

static inline void kos_DrawLine(int x_start, int y_start, int x_end, int y_end, color_t color) {
	__asm__ __volatile__(
	"int $0x40"
	::"a"(38), "d"(color),
	  "b"((x_start << 16) | x_end),
	  "c"((y_start << 16) | y_end));
}

static inline void kos_DrawBar(int x, int y, int w, int h, color_t color) {
	__asm__ __volatile__(
	"int $0x40"
	::"a"(13), "d"(color),
	  "b"((x << 16) | w),
	  "c"((y << 16) | h));
}

static inline void kos_PutPixel(int x, int y, color_t color) {
	__asm__ __volatile__("int $0x40"
			 ::"a"(1),
			  "b"(x),
			  "c"(y),
			  "d"(color));
}

static inline void kos_DrawBitmap(void *bitmap, int x, int y, int w, int h) {
	__asm__ __volatile__(
	"int $0x40"
	::"a"(7), "b"(bitmap),
	  "c"((w << 16) | h),
	  "d"((x << 16) | y));
}

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

// Get screen part as image
static inline void kos_ScreenShot(char* image, int x, int y, int w, int h) {
	__asm__ __volatile__(
	"int $0x40"
	::"a"(36),
	 "b"(image),
	 "c"(w*65536+h),
	 "d"(x*65536+y) : "memory");
};

/*********************** Skin *************************/
// Get skin height
static inline uint32_t kos_SkinHeight(void) {
	uint32_t height;

	__asm__ __volatile__(
	"int $0x40 \n\t"
	:"=a"(height)
	:"a"(48),"b"(4));
	return height;
};

/*********************** Mouse *************************/
#define POS_SCREEN 0
#define POS_WINDOW 1

static inline pos_t kos_GetMousePos(int origin) {
	pos_t val;
	__asm__ __volatile__(
	"int $0x40 \n\t"
	"rol $16, %%eax"
	:"=a"(val)
	:"a"(37),"b"(origin));
	return val;
}

static inline uint32_t kos_GetMouseButtons(void) {
	uint32_t val;

	__asm__ __volatile__(
	"int $0x40"
	:"=a"(val)
	:"a"(37),"b"(2));
	return val;
};

static inline uint32_t kos_GetMouseWheels(void) {
	uint32_t val;

	__asm__ __volatile__(
	"int $0x40 \n\t"
	:"=a"(val)
	:"a"(37),"b"(7));
	return val;
};

static inline uint32_t kos_LoadCursor(void *path, uint32_t flags) {
	uint32_t  val;
	__asm__ __volatile__(
	"int $0x40"
	:"=a"(val)
	:"a"(37), "b"(4), "c"(path), "d"(flags));
	return val;
}

static inline uint32_t kos_SetCursor(uint32_t cursor) {
	uint32_t  old;
	__asm__ __volatile__(
	"int $0x40"
	:"=a"(old)
	:"a"(37), "b"(5), "c"(cursor));
	return old;
};

static inline int kos_DestroyCursor(uint32_t cursor) {
	int ret;
	__asm__ __volatile__(
	"int $0x40"
	:"=a"(ret)
	:"a"(37), "b"(6), "c"(cursor)
	:"memory");
	return ret;
};

/*********************** OS Events *************************/
#define evReDraw  1
#define evKey     2
#define evButton  3
#define evExit    4
#define evDesktop 5
#define evMouse   6
#define evIPC     7
#define evNetwork 8
#define evDebug   9

static inline uint32_t kos_WaitForEventTimeout(uint32_t time) {
	uint32_t val;
	__asm__ __volatile__(
	"int $0x40"
	:"=a"(val)
	:"a"(23), "b"(time));
	return val;
};

static inline uint32_t kos_CheckForEvent(void) {
	uint32_t val;
	__asm__ __volatile__(
	"int $0x40"
	:"=a"(val)
	:"a"(11));
	return val;
};

static inline uint32_t kos_WaitForEvent(void) {
	uint32_t val;
	__asm__ __volatile__(
	"int $0x40"
	:"=a"(val)
	:"a"(10));
	return val;
};

/*********************** Eventmask *************************/
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

static inline uint32_t kos_SetMaskForEvents(uint32_t event_mask) {
	uint32_t  old_event_mask;
	__asm__ __volatile__(
	"int $0x40"
	:"=a"(old_event_mask)
	:"a"(40),"b"(event_mask));

	return old_event_mask;
};

/*********************** Other *************************/
static inline int kos_GetKey() {
	unsigned short key;
	__asm__ __volatile__("int $0x40":"=a"(key):"0"(2));
	if(!(key & 0xFF)) return (key>>8)&0xFF; else return 0;
}

static inline uint32_t kos_GetButtonID(void) {
	uint32_t val;
	__asm__ __volatile__(
	"int $0x40"
	:"=a"(val)
	:"a"(17));
	return val>>8;
};

static inline void kos_Delay(uint32_t time) {
	__asm__ __volatile__(
	"int $0x40"
	::"a"(5), "b"(time)
	:"memory");
};

static inline pos_t kos_ScreenSize() {
	pos_t size;
	__asm__ __volatile__(
	"int $0x40"
	:"=a"(size)
	:"a"(14));

	return size;
};

static inline void kos_GetSystemColors(struct kolibri_system_colors *color_table) {
	__asm__ __volatile__ ("int $0x40"
	:
	:"a"(48),"b"(3),"c"(color_table),"d"(40)
	);
}

// sysfn 9
static inline void kos_ProcessInfo(char *info) {
	__asm__ __volatile__(
	"int $0x40"
	:
	:"a"(9), "b"(info), "c"(-1)
	:"memory");
};

static inline void kos_RunApp(char* app, char* param) {
	kos_Struct70 r;
	r.p00 = 7;
	r.p04 = 0;
	r.p08 = param;
	r.p12 = 0;
	r.p16 = 0;
	r.p20 = 0;
	r.p21 = app;
	__asm__ __volatile__ ("int $0x40"::"a"(70), "b"(&r));
}

