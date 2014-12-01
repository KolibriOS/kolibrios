#include "kolibri.h"
#include "string.h"


extern char KOL_PATH[256];
extern char KOL_PARAM[256];
extern char KOL_DIR[256];


void kol_exit()
{
asm ("int $0x40"::"a"(-1));
}


void kol_sleep(unsigned d)
{
asm ("int $0x40"::"a"(5), "b"(d));
}


void kol_wnd_define(unsigned x, unsigned y, unsigned w, unsigned h, unsigned c)
{
asm ("nop"::"a"(0), "b"(x*65536+w), "c"(y*65536+h), "d"(c));
asm ("movl $0xffffff, %esi \n int $0x40");
}


void kol_wnd_move(unsigned x, unsigned y)
{
asm ("nop"::"a"(67), "b"(x), "c"(y));
asm ("movl $-1, %edx \n movl $-1, %esi \n int $0x40");
}


void kol_event_mask(unsigned e)
{
asm ("int $0x40"::"a"(40), "b"(e));
}


unsigned kol_event_wait()
{
asm ("int $0x40"::"a"(10));
}


unsigned kol_event_wait_time(unsigned time)
{
asm ("int $0x40"::"a"(23), "b"(time));
}


unsigned kol_event_check()
{
asm ("int $0x40"::"a"(11));
}


void kol_paint_start()
{
asm ("int $0x40"::"a"(12), "b"(1));
}


void kol_paint_end()
{
asm ("int $0x40"::"a"(12), "b"(2));
}


void kol_paint_pixel(unsigned x, unsigned y, unsigned c)
{
asm ("int $0x40"::"a"(1), "b"(x), "c"(y), "d"(c));
}


void kol_paint_bar(unsigned x, unsigned y, unsigned w, unsigned h, unsigned c)
{
asm ("int $0x40"::"a"(13), "b"(x*65536+w), "c"(y*65536+h), "d"(c));
}


void kol_paint_line(unsigned x1, unsigned y1, unsigned x2, unsigned y2, unsigned c)
{
asm ("int $0x40"::"a"(38), "b"(x1*65536+x2), "c"(y1*65536+y2), "d"(c));
}


void kol_paint_string(unsigned x, unsigned y,  unsigned c, char *s, unsigned l)
{
asm ("int $0x40"::"a"(4), "b"(x*65536+y), "c"(c), "d"(s), "S"(l));
}


void kol_paint_image(unsigned x, unsigned y, unsigned w, unsigned h, char *d)
{
asm ("int $0x40"::"a"(7), "c"(w*65536+h), "d"(x*65536+y), "b"(d));
}


void kol_paint_image_pal(unsigned x, unsigned y, unsigned w, unsigned h, char *d, unsigned *palette)
{
asm ("nop"::"c"(w*65536+h), "d"(x*65536+y), "b"(d));
asm ("nop"::"a"(palette));
asm ("movl %eax, %edi");
asm ("xor %eax, %eax");
asm ("movl %eax, %ebp");
asm ("pushl $8");
asm ("popl %esi");
asm ("int $0x40"::"a"(65));
}


unsigned kol_key_get()
{
 unsigned __ret;
 asm ("int $0x40":"=a"(__ret):"0"(2));
 if(!(__ret & 0xFF)) return (__ret>>8)&0xFF; else return 0;
}


unsigned kol_key_control()
{
asm ("int $0x40"::"a"(66), "b"(3));
}


void kol_key_lang_set(unsigned lang)
{
asm ("int $0x40"::"a"(21), "b"(2), "c"(9), "d"(lang));
}


unsigned kol_key_lang_get()
{
asm ("int $0x40"::"a"(26), "b"(2), "c"(9));
}


void kol_key_mode_set(unsigned mode)
{
asm ("int $0x40"::"a"(66), "b"(1), "c"(mode));
}


unsigned kol_key_mode_get()
{
asm ("int $0x40"::"a"(66), "b"(2));
}


unsigned kol_btn_get()
{
unsigned __ret;
asm ("int $0x40":"=a"(__ret):"0"(17));
 if((__ret & 0xFF)==0) return (__ret>>8)&0xFF; else return -1;
}


void kol_btn_define(unsigned x, unsigned y, unsigned w, unsigned h, unsigned d, unsigned c)
{
asm ("nop"::"b"(x*65536+w), "c"(y*65536+h), "d"(d));
asm ("nop"::"a"(c));
asm ("movl %eax, %esi");
asm ("int $0x40"::"a"(8));
}


void kol_btn_type(unsigned t)
{
asm ("int $0x40"::"a"(48), "b"(1), "c"(t));
}


void kol_wnd_caption(char *s)
{
asm ("int $0x40"::"a"(71), "b"(1), "c"(s));
}


unsigned kol_mouse_pos()
{
asm ("int $0x40"::"a"(37), "b"(0));
}


unsigned kol_mouse_posw()
{
asm ("int $0x40"::"a"(37), "b"(1));
}


unsigned kol_mouse_btn()
{
asm ("int $0x40"::"a"(37), "b"(2));
}


void kol_board_putc(char c)
{
asm ("int $0x40"::"a"(63), "b"(1), "c"(c));
}


void kol_board_puts(char *s)
{
unsigned i;
i = 0;
while (*(s+i))
	{
	asm ("int $0x40"::"a"(63), "b"(1), "c"(*(s+i)));
	i++;
	}
}


void kol_board_puti(int n)
{
char c;
int i = 0;
do 
	{
	c = n % 10 + '0';
	asm ("int $0x40"::"a"(63), "b"(1), "c"(c));
	i++;
	}
	while ((n /= 10) > 0);
}


int kol_file_70(kol_struct70 *k)
{
asm ("int $0x40"::"a"(70), "b"(k));
}


kol_struct_import* kol_cofflib_load(char *name)
{
asm ("int $0x40"::"a"(68), "b"(19), "c"(name));
}


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


unsigned kol_system_cpufreq()
{
asm ("int $0x40"::"a"(18), "b"(5));
}


unsigned kol_system_mem()
{
asm ("int $0x40"::"a"(18), "b"(17));
}


unsigned kol_system_memfree()
{
asm ("int $0x40"::"a"(18), "b"(16));
}


unsigned kol_system_time_get()
{
asm ("int $0x40"::"a"(3));
}


unsigned kol_system_date_get()
{
asm ("int $0x40"::"a"(29));
}


unsigned kol_system_end(unsigned param)
{
asm ("int $0x40"::"a"(18), "b"(9), "c"(param));
}


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



void kol_screen_wait_rr()
{
asm ("int $0x40"::"a"(18), "b"(14));
}



void kol_screen_get_size(unsigned *w, unsigned *h)
{
unsigned size;
asm ("int $0x40":"=a"(size):"a"(14));
*w = size / 65536;
*h = size % 65536;
}



unsigned kol_skin_height()
{
asm ("int $0x40"::"a"(48), "b"(4));
}


unsigned kol_thread_start(unsigned start, unsigned stack)
{
asm ("int $0x40"::"a"(51), "b"(1), "c"(start), "d"(stack));
}


unsigned kol_time_tick()
{
asm ("int $0x40"::"a"(26), "b"(9));
}


unsigned kol_sound_speaker(char data[])
{
asm ("movl %0, %%esi"::"a"(data));
asm ("int $0x40"::"a"(55), "b"(55));
}


unsigned kol_process_info(unsigned slot, char buf1k[])
{
asm ("int $0x40"::"a"(9), "b"(buf1k), "c"(slot));
}


int kol_process_kill_pid(unsigned process)
{
asm ("int $0x40"::"a"(18), "b"(18), "c"(process));
}
