
#define NULL ((void*)0)

typedef struct 
{
unsigned	p00 __attribute__((packed));
unsigned	p04 __attribute__((packed));
unsigned	p08 __attribute__((packed));
unsigned	p12 __attribute__((packed));
unsigned	p16 __attribute__((packed));
char		p20 __attribute__((packed));
char		*p21 __attribute__((packed));
} kol_struct70 __attribute__((packed));


typedef struct
{
unsigned	p00 __attribute__((packed));
char		p04 __attribute__((packed));
char		p05[3] __attribute__((packed));
unsigned	p08 __attribute__((packed));
unsigned	p12 __attribute__((packed));
unsigned	p16 __attribute__((packed));
unsigned	p20 __attribute__((packed));
unsigned	p24 __attribute__((packed));
unsigned	p28 __attribute__((packed));
unsigned	p32[2] __attribute__((packed));
} kol_struct_BDVK __attribute__((packed));

typedef struct
{
char	*name __attribute__((packed));
void	*data __attribute__((packed));
} kol_struct_import __attribute__((packed));


void kol_exit();
void kol_sleep(unsigned d);
void kol_wnd_define(unsigned x, unsigned y, unsigned w, unsigned h, unsigned c);
void kol_wnd_caption(char *s);
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
unsigned kol_key_get();
void kol_btn_define(unsigned x, unsigned y, unsigned w, unsigned h, unsigned d, unsigned c);
unsigned kol_btn_get();
void kol_btn_type(unsigned t);
unsigned kol_mouse_pos();
unsigned kol_mouse_posw();
unsigned kol_mouse_btn();
void kol_board_putc(char c);
void kol_board_puts(char *s);
unsigned kol_file_70(kol_struct70 *k);
kol_struct_import* kol_cofflib_load(char *name);
void* kol_cofflib_procload (kol_struct_import *imp, char *name);
unsigned kol_cofflib_procnum (kol_struct_import *imp);
void kol_cofflib_procname (kol_struct_import *imp, char *name, unsigned n);
unsigned kol_system_cpufreq();
unsigned kol_system_mem();
unsigned kol_system_memfree();
unsigned kol_system_time_get();
void kol_path_file2dir(char *dir, char *fname);
void kol_path_full(char *full, char *fname);
void kol_screen_wait_rr();
unsigned kol_skin_height();
