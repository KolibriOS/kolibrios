
#include "kolibri.h"
//#include "string.h"


extern char KOL_PATH[256];
extern char KOL_PARAM[256];
extern char KOL_DIR[256];


void kol_exit()
{
asm volatile ("int $0x40"::"a"(-1));
}


void kol_sleep(unsigned d)
{
asm volatile ("int $0x40"::"a"(5), "b"(d));
}


// define a window
// x, y - position; w, h - size; cs - color and style; c - caption; b - boder
void kol_wnd_define(unsigned x, unsigned y, unsigned w, unsigned h, unsigned cs, unsigned b, char *t)
{
asm volatile ("int $0x40"::"a"(0), "b"(x*65536+w), "c"(y*65536+h), "d"(cs), "D"(t), "S"(b) );
}


void kol_wnd_move(unsigned x, unsigned y)
{
asm volatile ("int $0x40"::"a"(67), "b"(x), "c"(y), "d"(-1), "S"(-1));
}


void kol_event_mask(unsigned e)
{
asm volatile ("int $0x40"::"a"(40), "b"(e));
}


unsigned kol_event_wait() 
{
asm volatile ("int $0x40"::"a"(10));
}


unsigned kol_event_wait_time(unsigned time)
{
asm volatile ("int $0x40"::"a"(23), "b"(time));
}


unsigned kol_event_check()
{
asm volatile ("int $0x40"::"a"(11));
}


void __attribute__((__always_inline__)) inline  kol_paint_start()
{
asm volatile ("int $0x40"::"a"(12), "b"(1));
}


void __attribute__((__always_inline__)) inline  kol_paint_end()
{
asm volatile ("int $0x40"::"a"(12), "b"(2));
}


void kol_paint_pixel(unsigned x, unsigned y, unsigned c)
{
asm volatile ("int $0x40"::"a"(1), "b"(x), "c"(y), "d"(c));
}


void kol_paint_bar(unsigned x, unsigned y, unsigned w, unsigned h, unsigned c)
{
asm volatile ("int $0x40"::"a"(13), "b"(x*65536+w), "c"(y*65536+h), "d"(c));
}


void kol_paint_line(unsigned x1, unsigned y1, unsigned x2, unsigned y2, unsigned c)
{
asm volatile ("int $0x40"::"a"(38), "b"(x1*65536+x2), "c"(y1*65536+y2), "d"(c));
}


void kol_paint_string(unsigned x, unsigned y, char *s, unsigned c)
{
asm volatile ("int $0x40"::"a"(4), "b"(x*65536+y), "c"(c), "d"(s));
}


void kol_paint_image(unsigned x, unsigned y, unsigned w, unsigned h, char *d)
{
asm volatile ("int $0x40"::"a"(7), "c"(w*65536+h), "d"(x*65536+y), "b"(d));
}


void kol_paint_image_pal(unsigned x, unsigned y, unsigned w, unsigned h, char *d, unsigned *palette)
{
asm volatile ("int $0x40"::"a"(65), "b"(d), "c"(w*65536+h), "d"(x*65536+y), "D"(palette), "S"(8));
}


unsigned kol_key_get()
{
asm volatile ("int $0x40"::"a"(2));
}


unsigned kol_key_control()
{
asm volatile ("int $0x40"::"a"(66), "b"(3));
}


void kol_key_lang_set(unsigned lang)
{
asm volatile ("int $0x40"::"a"(21), "b"(2), "c"(9), "d"(lang));
}


unsigned kol_key_lang_get()
{
asm volatile ("int $0x40"::"a"(26), "b"(2), "c"(9));
}


void kol_key_mode_set(unsigned mode)
{
asm volatile ("int $0x40"::"a"(66), "b"(1), "c"(mode));
}


unsigned kol_key_mode_get()
{
asm volatile ("int $0x40"::"a"(66), "b"(2));
}


unsigned kol_btn_get()
{
asm volatile ("int $0x40"::"a"(17));
}


void kol_btn_define(unsigned x, unsigned y, unsigned w, unsigned h, unsigned d, unsigned c)
{
asm volatile ("int $0x40"::"a"(8), "b"(x*65536+w), "c"(y*65536+h), "d"(d), "S"(c));
}


void kol_btn_type(unsigned t)
{
asm volatile ("int $0x40"::"a"(48), "b"(1), "c"(t));
}


void kol_wnd_caption(char *s)
{
asm volatile ("int $0x40"::"a"(71), "b"(1), "c"(s));
}


unsigned kol_mouse_pos()
{
asm volatile ("int $0x40"::"a"(37), "b"(0));
}


unsigned kol_mouse_posw()
{
asm volatile ("int $0x40"::"a"(37), "b"(1));
}


unsigned kol_mouse_btn()
{
asm volatile ("int $0x40"::"a"(37), "b"(2));
}


void kol_board_putc(char c)
{
asm volatile ("int $0x40"::"a"(63), "b"(1), "c"(c));
}


void kol_board_puts(char *s)
{
unsigned i;
i = 0;
while (*(s+i))
	{
	asm volatile ("int $0x40"::"a"(63), "b"(1), "c"(*(s+i)));
	i++;
	}
}


void kol_board_puti(int n)
{
char c;

if ( n > 1 )
	kol_board_puti(n / 10);

c = n % 10 + '0';
asm volatile ("int $0x40"::"a"(63), "b"(1), "c"(c));

}


int kol_file_70(kol_struct70 *k)
{
asm volatile ("int $0x40"::"a"(70), "b"(k));
}


kol_struct_import* kol_cofflib_load(char *name)
{
asm volatile ("int $0x40"::"a"(68), "b"(19), "c"(name));
}

/*
void* kol_cofflib_procload (kol_struct_import *imp, char *name)
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
*/

unsigned kol_cofflib_procnum (kol_struct_import *imp)
{
unsigned i, n;

for (i=n=0;;i++)
	if ( NULL == ((imp+i) -> name))
		break;
	else
		n++;

return n;
}

/*
void kol_cofflib_procname (kol_struct_import *imp, char *name, unsigned n)
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
*/


unsigned kol_system_cpufreq()
{
asm volatile ("int $0x40"::"a"(18), "b"(5));
}


unsigned kol_system_mem()
{
asm volatile ("int $0x40"::"a"(18), "b"(17));
}


unsigned kol_system_memfree()
{
asm volatile ("int $0x40"::"a"(18), "b"(16));
}


unsigned kol_system_time_get()
{
asm volatile ("int $0x40"::"a"(3));
}


unsigned kol_system_date_get()
{
asm volatile ("int $0x40"::"a"(29));
}


unsigned kol_system_end(unsigned param)
{
asm volatile ("int $0x40"::"a"(18), "b"(9), "c"(param));
}


/*
void kol_path_file2dir(char *dir, char *fname)
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


void kol_path_full(char *full, char *fname)
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
*/


void __attribute__((__always_inline__)) inline kol_screen_wait_rr()
{
asm volatile ("int $0x40"::"a"(18), "b"(14));
}



void kol_screen_get_size(unsigned *w, unsigned *h)
{
unsigned size;
asm volatile ("int $0x40":"=a"(size):"a"(14));
*w = size / 65536;
*h = size % 65536;
}



unsigned kol_skin_height()
{
asm volatile ("int $0x40"::"a"(48), "b"(4));
}


unsigned kol_thread_start(unsigned start, unsigned stack)
{
asm volatile ("int $0x40"::"a"(51), "b"(1), "c"(start), "d"(stack));
}


unsigned kol_time_tick()
{
asm volatile ("int $0x40"::"a"(26), "b"(9));
}


unsigned kol_sound_speaker(char data[])
{
asm volatile ("movl %0, %%esi"::"a"(data));
asm volatile ("int $0x40"::"a"(55), "b"(55));
}


unsigned kol_process_info(unsigned slot, char buf1k[])
{
asm volatile ("int $0x40"::"a"(9), "b"(buf1k), "c"(slot));
}


int kol_process_kill_pid(unsigned process)
{
asm volatile ("int $0x40"::"a"(18), "b"(18), "c"(process));
}

int kol_kill_process(unsigned process)
{
asm volatile ("int $0x40"::"a"(18), "b"(2), "c"(process));
}

void kol_get_kernel_ver(char buff16b[])
{
asm volatile ("int $0x40"::"a"(18), "b"(13), "c"(buff16b));
}

int kol_buffer_open(char name[], int mode, int size, char **buf)
{
int error;
asm volatile ("int $0x40":"=a"(*buf), "=d"(error):"a"(68), "b"(22), "c"(name), "d"(size), "S"(mode));
return error;
}

void kol_buffer_close(char name[])
{
asm volatile ("int $0x40"::"a"(68), "b"(23), "c"(name));
}

int kol_clip_num()
{
asm volatile ("int $0x40"::"a"(54), "b"(0));
}

char* kol_clip_get(int n)
{
asm volatile ("int $0x40"::"a"(54), "b"(1), "c"(n));
}

int kol_clip_set(int n, char buffer[])
{
asm volatile ("int $0x40"::"a"(54), "b"(2), "c"(n), "d"(buffer));
}
