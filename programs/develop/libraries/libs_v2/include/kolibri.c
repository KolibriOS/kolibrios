#ifndef KOLIBRI_INCLUDE
#define KOLIBRI_INCLUDE

#define NULL ((void*)0)

#define SHM_OPEN		0
#define SHM_OPEN_ALWAYS	0x04
#define SHM_CREATE		0x08 
#define SHM_READ		0x00 
#define SHM_WRITE		0x01

#define E_NOTFOUND	5 
#define E_ACCESS	10 
#define E_NOMEM		30 
#define E_PARAM		33

#define evReDraw  1
#define evKey     2
#define evButton  3
#define evDesktop 5
#define evMouse   6
#define evIPC     7
#define evNetwork 8
#define evDebug   9

#pragma pack(push,1)
typedef struct 
{
unsigned	p00;
unsigned	p04;
unsigned	p08;
unsigned	p12;
unsigned	p16;
char		p20;
char		*p21;
} struct70;
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
unsigned	p32[2];
unsigned	p40;
} struct_BDVK;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct
{
char	*name;
void	*data;
} struct_import;
#pragma pack(pop)

#include "string.c"


extern char PATH[256];
extern char PARAM[256];
extern char DIR[256];


static inline void exit()
{
	asm volatile ("int $0x40"::"a"(-1));
}

static inline dword get_pixel(unsigned x,unsigned y,unsigned size)
{
	asm volatile ("int $0x40"::"a"(35), "b"((y*size+x)));
}

static inline void sleep(unsigned d)
{
	asm volatile ("int $0x40"::"a"(5), "b"(d));
}

static inline void GetRGB(unsigned x,unsigned y, unsigned w, unsigned h,unsigned d)
{
	asm volatile ("int $0x40"::"a"(36), "b"(d), "c"((w<<16)|h), "d"((x<<16)|y));
}

// define a window
// x, y - position; w, h - size; cs - color and style; c - caption; b - boder
static inline void wnd_define(unsigned x, unsigned y, unsigned w, unsigned h)
{
	asm volatile ("int $0x40"::"a"(0), "b"(x*65536+w), "c"(y*65536+h), "d"(0x43000000));
}


static inline void wnd_move(unsigned x, unsigned y)
{
	asm volatile ("int $0x40"::"a"(67), "b"(x), "c"(y), "d"(-1), "S"(-1));
}


static inline void event_mask(unsigned e)
{
	asm volatile ("int $0x40"::"a"(40), "b"(e));
}


static inline unsigned event_wait() 
{
	asm volatile ("int $0x40"::"a"(10));
}


static inline unsigned event_wait_time(unsigned time)
{
	asm volatile ("int $0x40"::"a"(23), "b"(time));
}


static inline unsigned event_check()
{
	asm volatile ("int $0x40"::"a"(11));
}


static inline void __attribute__((__always_inline__)) paint_start()
{
	asm volatile ("int $0x40"::"a"(12), "b"(1));
}


static inline void __attribute__((__always_inline__)) paint_end()
{
	asm volatile ("int $0x40"::"a"(12), "b"(2));
}


static inline void paint_pixel(unsigned x, unsigned y, unsigned c)
{
	asm volatile ("int $0x40"::"a"(1), "b"(x), "c"(y), "d"(c));
}


static inline void paint_bar(unsigned x, unsigned y, unsigned w, unsigned h, unsigned c)
{
	asm volatile ("int $0x40"::"a"(13), "b"(x*65536+w), "c"(y*65536+h), "d"(c));
}


static inline void paint_line(unsigned x1, unsigned y1, unsigned x2, unsigned y2, unsigned c)
{
	asm volatile ("int $0x40"::"a"(38), "b"(x1*65536+x2), "c"(y1*65536+y2), "d"(c));
}


static inline void paint_string(unsigned x, unsigned y, char *s, unsigned c)
{
	asm volatile ("int $0x40"::"a"(4), "b"(x*65536+y), "c"(c), "d"(s));
}


static inline void paint_image(unsigned x, unsigned y, unsigned w, unsigned h, char *d)
{
	asm volatile ("int $0x40"::"a"(7), "c"(w*65536+h), "d"(x*65536+y), "b"(d));
}


static inline void paint_image_pal(unsigned x, unsigned y, unsigned w, unsigned h, char *d, unsigned *palette)
{
	asm volatile ("int $0x40"::"a"(65), "b"(d), "c"(w*65536+h), "d"(x*65536+y), "D"(palette), "S"(8));
}


static inline unsigned key_get()
{
	asm volatile ("int $0x40"::"a"(2));
}


static inline unsigned key_control()
{
	asm volatile ("int $0x40"::"a"(66), "b"(3));
}


static inline void key_lang_set(unsigned lang)
{
	asm volatile ("int $0x40"::"a"(21), "b"(2), "c"(9), "d"(lang));
}


static inline unsigned key_lang_get()
{
	asm volatile ("int $0x40"::"a"(26), "b"(2), "c"(9));
}


static inline void key_mode_set(unsigned mode)
{
	asm volatile ("int $0x40"::"a"(66), "b"(1), "c"(mode));
}


static inline unsigned key_mode_get()
{
	asm volatile ("int $0x40"::"a"(66), "b"(2));
}


static inline unsigned btn_get()
{
	asm volatile ("int $0x40"::"a"(17));
}


static inline void btn_define(unsigned x, unsigned y, unsigned w, unsigned h, unsigned d, unsigned c)
{
	asm volatile ("int $0x40"::"a"(8), "b"(x*65536+w), "c"(y*65536+h), "d"(d), "S"(c));
}


static inline void btn_type(unsigned t)
{
	asm volatile ("int $0x40"::"a"(48), "b"(1), "c"(t));
}


static inline void wnd_caption(char *s)
{
	asm volatile ("int $0x40"::"a"(71), "b"(1), "c"(s));
}


static inline unsigned mouse_pos()
{
	asm volatile ("int $0x40"::"a"(37), "b"(0));
}


static inline unsigned mouse_posw()
{
	asm volatile ("int $0x40"::"a"(37), "b"(1));
}


static inline unsigned mouse_btn()
{
	asm volatile ("int $0x40"::"a"(37), "b"(2));
}


static inline void board_putc(char c)
{
	asm volatile ("int $0x40"::"a"(63), "b"(1), "c"(c));
}


static inline void board_puts(char *s)
{
	unsigned i;
	i = 0;
	while (*(s+i))
	{
		asm volatile ("int $0x40"::"a"(63), "b"(1), "c"(*(s+i)));
	i++;
	}
}


static inline void board_puti(int n)
{
	char c;

	if ( n > 1 )
		board_puti(n / 10);

	c = n % 10 + '0';
	asm volatile ("int $0x40"::"a"(63), "b"(1), "c"(c));

}


static inline int file_70(struct70 *k)
{
	asm volatile ("int $0x40"::"a"(70), "b"(k));
}


static inline struct_import* cofflib_load(char *name)
{
	asm volatile ("int $0x40"::"a"(68), "b"(19), "c"(name));
}


static inline void* cofflib_procload (struct_import *imp, char *name)
{
	int i;
	for (i=0;;i++)
	if ( NULL == ((imp+i) -> name))
		break;
	else
		if ( 0 == strcmp(name, (imp+i)->name) )
			return (imp+i)->data;
	return NULL;
}


static inline unsigned cofflib_procnum (struct_import *imp)
{
	unsigned i, n;

	for (i=n=0;;i++)
		if ( NULL == ((imp+i) -> name))
			break;
		else
			n++;

	return n;
}


static inline void cofflib_procname (struct_import *imp, char *name, unsigned n)
{
	unsigned i;
	*name = 0;

	for (i=0;;i++)
		if ( NULL == ((imp+i) -> name))
			break;
		else
			if ( i == n )
				{
					strcpy(name, ((imp+i)->name));
					break;
				}

}


static inline unsigned system_cpufreq()
{
	asm volatile ("int $0x40"::"a"(18), "b"(5));
}


static inline unsigned system_mem()
{
	asm volatile ("int $0x40"::"a"(18), "b"(17));
}


static inline unsigned system_memfree()
{
	asm volatile ("int $0x40"::"a"(18), "b"(16));
}


static inline unsigned system_time_get()
{
	asm volatile ("int $0x40"::"a"(3));
}


static inline unsigned system_date_get()
{
	asm volatile ("int $0x40"::"a"(29));
}


static inline unsigned system_end(unsigned param)
{
	asm volatile ("int $0x40"::"a"(18), "b"(9), "c"(param));
}

static inline unsigned win_min()
{
	asm volatile ("int $0x40"::"a"(18), "b"(10));
}


static inline void path_file2dir(char *dir, char *fname)
{
	unsigned i;
	strcpy (dir, fname);
	for ( i = strlen(dir);; --i)
		if ( '/' == dir[i])
		{
			dir[i] = '\0';
			return;
		}
}


static inline void path_full(char *full, char *fname)
{
	char temp[256];

	switch (*fname)
		{

		case '/':
			strncpy(temp, fname+1, 2);
			temp[2]=0;
			if ( (!strcmp("rd", temp)) || (!strcmp("hd", temp)) || (!strcmp("cd", temp)) )
				strcpy (full, fname);
			break;

		case '.':
			break;

		default:
			break;

		};
}



static inline void __attribute__((__always_inline__)) screen_wait_rr()
{
	asm volatile ("int $0x40"::"a"(18), "b"(14));
}



static inline void screen_get_size(unsigned *w, unsigned *h)
{
	unsigned size;
	asm volatile ("int $0x40":"=a"(size):"a"(14));
	*h = size&0xFFFF;
	*w = size>>16;
}


static inline unsigned skin_height()
{
	asm volatile ("int $0x40"::"a"(48), "b"(4));
}


static inline unsigned thread_start(unsigned start, unsigned stack)
{
	asm volatile ("int $0x40"::"a"(51), "b"(1), "c"(start), "d"(stack));
}


static inline unsigned time_tick()
{
	asm volatile ("int $0x40"::"a"(26), "b"(9));
}


static inline unsigned sound_speaker(char data[])
{
	asm volatile ("movl %0, %%esi"::"a"(data));
	asm volatile ("int $0x40"::"a"(55), "b"(55));
}


static inline unsigned process_info(signed slot, char buf1k[])
{
	asm volatile ("int $0x40"::"a"(9), "b"(buf1k), "c"(slot));
}


static inline int process_kill_pid(unsigned process)
{
	asm volatile ("int $0x40"::"a"(18), "b"(18), "c"(process));
}

static inline int kill_process(unsigned process)
{
	asm volatile ("int $0x40"::"a"(18), "b"(2), "c"(process));
}

static inline void get_kernel_ver(char buff16b[])
{
	asm volatile ("int $0x40"::"a"(18), "b"(13), "c"(buff16b));
}

static inline long GetFreeRam(void)
{
	asm ("int $0x40"::"a"(18), "b"(16));
}

static inline long WaitBlanking(void)
{
	asm ("int $0x40"::"a"(18), "b"(14));
}

static inline int buffer_open(char name[], int mode, int size, char **buf)
{
	int error;
	asm volatile ("int $0x40":"=a"(*buf), "=d"(error):"a"(68), "b"(22), "c"(name), "d"(size), "S"(mode));
	return error;
}

static inline void buffer_close(char name[])
{
	asm volatile ("int $0x40"::"a"(68), "b"(23), "c"(name));
}

static inline int clip_num()
{
	asm volatile ("int $0x40"::"a"(54), "b"(0));
}

static inline char* clip_get(int n)
{
	asm volatile ("int $0x40"::"a"(54), "b"(1), "c"(n));
}

static inline int clip_set(int n, char buffer[])
{
	asm volatile ("int $0x40"::"a"(54), "b"(2), "c"(n), "d"(buffer));
}


#endif