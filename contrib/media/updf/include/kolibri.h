
#define NULL ((void*)0)
#define __attribute__(something) /* nothing */
#pragma pack(1)

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
unsigned	p40 __attribute__((packed));
} kol_struct_BDVK __attribute__((packed));

typedef struct
{
char	*name __attribute__((packed));
void	*data __attribute__((packed));
} kol_struct_import __attribute__((packed));


__declspec(noreturn) void __cdecl kol_exit();
void __cdecl kol_sleep(unsigned d);
void __cdecl kol_wnd_define(unsigned x, unsigned y, unsigned w, unsigned h, unsigned c);
void __cdecl kol_wnd_move(unsigned x, unsigned y);
void __cdecl kol_wnd_caption(char *s);
void __cdecl kol_event_mask(unsigned e);
unsigned __cdecl kol_event_wait();
unsigned __cdecl kol_event_wait_time(unsigned time);
unsigned __cdecl kol_event_check();
void __cdecl kol_paint_start();
void __cdecl kol_paint_end();
void __cdecl kol_paint_pixel(unsigned x, unsigned y, unsigned c);
void __cdecl kol_paint_bar(unsigned x, unsigned y, unsigned w, unsigned h, unsigned c);
void __cdecl kol_paint_line(unsigned x1, unsigned y1, unsigned x2, unsigned y2, unsigned c);
void __cdecl kol_paint_string(unsigned x, unsigned y, char *s, unsigned c);
void __cdecl kol_paint_image(unsigned x, unsigned y, unsigned w, unsigned h, char *d);
void __cdecl kol_paint_image_pal(unsigned x, unsigned y, unsigned w, unsigned h, char *d, unsigned *palette);
unsigned __cdecl kol_key_get();
unsigned __cdecl kol_key_control();
void __cdecl kol_key_lang_set(unsigned lang);
unsigned __cdecl kol_key_lang_get();
void __cdecl kol_key_mode_set(unsigned mode);
unsigned __cdecl kol_key_mode_get();
void __cdecl kol_btn_define(unsigned x, unsigned y, unsigned w, unsigned h, unsigned d, unsigned c);
unsigned __cdecl kol_btn_get();
void __cdecl kol_btn_type(unsigned t);
unsigned __cdecl kol_mouse_pos();
unsigned __cdecl kol_mouse_posw();
unsigned __cdecl kol_mouse_btn();
void __cdecl kol_board_putc(char c);
void __cdecl kol_board_puts(char *s);
void __cdecl kol_board_puti(int n);
int __declspec(noinline) __cdecl kol_file_70(kol_struct70 *k);
kol_struct_import* __fastcall kol_cofflib_load(char *name);
void* __cdecl kol_cofflib_procload (kol_struct_import *imp, char *name);
unsigned __cdecl kol_cofflib_procnum (kol_struct_import *imp);
void __cdecl kol_cofflib_procname (kol_struct_import *imp, char *name, unsigned n);
unsigned __fastcall kol_system_end(unsigned param);
unsigned __cdecl kol_system_cpufreq();
unsigned __fastcall kol_system_mem();
unsigned __fastcall kol_system_memfree();
unsigned __fastcall kol_system_time_get();
unsigned __fastcall kol_system_date_get();
void __cdecl kol_path_file2dir(char *dir, char *fname);
void __cdecl kol_path_full(char *full, char *fname);
void __cdecl kol_screen_wait_rr();
void __cdecl kol_screen_get_size(unsigned *w, unsigned *h);
unsigned __cdecl  kol_skin_height();
unsigned __cdecl kol_thread_start(unsigned start, unsigned stack);
unsigned __cdecl kol_time_tick();
unsigned __cdecl kol_sound_speaker(char data[]);
unsigned __fastcall kol_process_info(unsigned slot, char buf1k[]);
int __fastcall kol_process_kill_pid(unsigned process);
