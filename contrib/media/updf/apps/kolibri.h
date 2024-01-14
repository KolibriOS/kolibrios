
#ifndef NULL
#define NULL ((void*)0)
#endif

#define EVENT_REDRAW              0x00000001
#define EVENT_KEY                 0x00000002
#define EVENT_BUTTON              0x00000004
#define EVENT_END_REQUEST         0x00000008
#define EVENT_DESKTOP_BACK_DRAW   0x00000010
#define EVENT_MOUSE_CHANGE        0x00000020
#define EVENT_IPC		          0x00000040
#define EVENT_MOUSE_CURSOR_MASK   0x40000000
#define EVENT_MOUSE_WINDOW_MASK   0x80000000

#define SHM_OPEN		0
#define SHM_OPEN_ALWAYS	0x04
#define SHM_CREATE		0x08 
#define SHM_READ		0x00 
#define SHM_WRITE		0x01

#define E_NOTFOUND	5 
#define E_ACCESS	10 
#define E_NOMEM		30 
#define E_PARAM		33

#define FILENAME_MAX	1024

#define BT_DEL      0x80000000
#define BT_HIDE     0x40000000
#define BT_NOFRAME  0x20000000

#define evReDraw  1
#define evKey     2
#define evButton  3
#define evMouse   6
#define evNetwork 8

#define ASCII_KEY_LEFT  176
#define ASCII_KEY_RIGHT 179
#define ASCII_KEY_DOWN  177
#define ASCII_KEY_UP    178
#define ASCII_KEY_HOME  180
#define ASCII_KEY_END   181
#define ASCII_KEY_PGDN  183
#define ASCII_KEY_PGUP  184

#define ASCII_KEY_BS    8
#define ASCII_KEY_TAB   9
#define ASCII_KEY_ENTER 13
#define ASCII_KEY_ESC   27
#define ASCII_KEY_DEL   182
#define ASCII_KEY_INS   185
#define ASCII_KEY_SPACE 032

#pragma pack(push,1)
typedef struct 
{
unsigned	p00;
unsigned 	p04;
char	 	*p08;
unsigned	p12;
unsigned	p16;
char		p20;
char		*p21;
} kol_struct70;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct
{
unsigned	p00;
char		p04;
char		p05[3];
unsigned	p08;
unsigned	p12;
unsigned	p16;
unsigned	p20;
unsigned	p24;
unsigned	p28;
unsigned long long	p32;
unsigned	p40;
} kol_struct_BDVK;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct
{
char	*name;
void	*data;
} kol_struct_import;
#pragma pack(pop)

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

void kol_exit();
void kol_sleep(unsigned d);
void kol_wnd_define(unsigned x, unsigned y, unsigned w, unsigned h, unsigned cs, unsigned b, char *t);
void kol_wnd_caption(char *s);
void kol_wnd_change(int new_x, int new_y, int new_w, int new_h);
void kol_event_mask(unsigned e);
unsigned kol_event_wait();
unsigned kol_event_wait_time(unsigned time);
unsigned kol_event_check();
void kol_paint_start();
void kol_paint_end();
void kol_paint_pixel(unsigned x, unsigned y, unsigned c);
void kol_paint_bar(unsigned x, unsigned y, unsigned w, unsigned h, unsigned c);
void kol_paint_line(unsigned x1, unsigned y1, unsigned x2, unsigned y2, unsigned c);
void kol_paint_string(unsigned x, unsigned y, char *s, unsigned c);
void kol_paint_image(unsigned x, unsigned y, unsigned w, unsigned h, char *d);
void kol_paint_image_pal(unsigned x, unsigned y, unsigned w, unsigned h, char *buf, unsigned bits, unsigned palette);
unsigned kol_key_get();
unsigned kol_key_control();
void kol_key_lang_set(unsigned lang);
unsigned kol_key_lang_get();
void kol_key_mode_set(unsigned mode);
unsigned kol_key_mode_get();
void kol_paint_image_24(unsigned x, unsigned y, unsigned w, unsigned h, char *d);
unsigned kol_btn_get();
void kol_btn_type(unsigned t);
unsigned kol_mouse_pos();
unsigned kol_mouse_posw();
unsigned kol_mouse_btn();
void kol_board_putc(char c);
void kol_board_puts(char *s);
void kol_board_puti(int n);
int kol_file_70(kol_struct70 *k);
kol_struct_import* kol_cofflib_load(char *name);
void* kol_cofflib_procload (kol_struct_import *imp, char *name);
unsigned kol_cofflib_procnum (kol_struct_import *imp);
void kol_cofflib_procname (kol_struct_import *imp, char *name, unsigned n);
unsigned kol_system_end(unsigned param);
unsigned kol_system_cpufreq();
unsigned kol_system_mem();
unsigned kol_system_memfree();
unsigned kol_system_time_get();
unsigned kol_system_date_get();
void kol_path_file2dir(char *dir, char *fname);
void kol_path_full(char *full, char *fname);
void kol_screen_wait_rr();
void kol_screen_get_size(unsigned *w, unsigned *h);
unsigned kol_skin_height();
unsigned kol_thread_start(unsigned start, unsigned stack);
unsigned kol_time_tick();
unsigned kol_sound_speaker(char data[]);
unsigned kol_process_info(unsigned slot, char buf1k[]);
int kol_process_kill_pid(unsigned process);
void kol_get_kernel_ver(char buff16b[]);
int kol_kill_process(unsigned process);
int kol_buffer_open(char name[], int mode, int size, char **buf);
void  kol_buffer_close(char name[]);
int kol_clip_num();
char* kol_clip_get(int n);
int kol_clip_set(int n, char buffer[]);
void kol_btn_define(unsigned x, unsigned y, unsigned w, unsigned h, unsigned d, unsigned c);

void kos_blit(int dstx, int dsty, int w, int h, int srcx, 
	int srcy,int srcw, int srch, int stride, char *d);
int kos_random(int num);
int kos_get_mouse_wheels(void);
void kos_screen_max(int* x, int* y);
int kos_get_key();
void kos_text(int x, int y, int color, const char* text, int len);
