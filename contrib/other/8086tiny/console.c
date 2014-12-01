#include<menuet/os.h>


///===========================

#define CON_COLOR_BLUE		1
#define CON_COLOR_GREEN		2
#define CON_COLOR_RED		4
#define CON_COLOR_BRIGHT	8
/* цвет фона */
#define CON_BGR_BLUE		0x10
#define CON_BGR_GREEN		0x20
#define CON_BGR_RED		0x40
#define CON_BGR_BRIGHT		0x80

///===========================
#define _stdcall __attribute__((stdcall))
#define __stdcall __attribute__((stdcall))
#define _cdecl __attribute__((cdecl))
#define __cdecl __attribute__((cdecl))





void (* _stdcall con_init)(unsigned w_w, unsigned w_h, unsigned s_w, unsigned s_h, const char* t);
void (* _cdecl printf)(const char* format,...);
void (* _stdcall _exit2)(char bCloseWindow);
void (* __stdcall gets)(char* str, int n);
 int (* __stdcall getch)(void);
 int (* __stdcall kbhit)(void);
 int (* __stdcall con_get_font_height)(void);
 int (* __stdcall con_set_cursor_height)(int new_height);
unsigned (*__stdcall con_get_flags)(void);
unsigned (*__stdcall con_set_flags)(unsigned new_flags);
void (*__stdcall con_cls)(void);

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
char	*name __attribute__((packed));
void	*data __attribute__((packed));
} kol_struct_import __attribute__((packed));

///===========================

void kol_exit()
{
__asm ("int $0x40"::"a"(-1));
}

int kol_file_70(kol_struct70 *k)
{
__asm ("int $0x40"::"a"(70), "b"(k));
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




void CONSOLE_INIT(char title[])
{
kol_struct_import *imp;

__menuet__debug_out ("console init");
imp = __kolibri__cofflib_load("/sys/lib/console.obj");
if (imp == NULL)
	{__menuet__debug_out ("imp NULL"); kol_exit();}

con_init = ( _stdcall  void (*)(unsigned, unsigned, unsigned, unsigned, const char*)) 
		kol_cofflib_procload (imp, "con_init");
if (con_init == NULL)
	{__menuet__debug_out ("con init failed"); kol_exit();}

printf = ( _cdecl void (*)(const char*,...))
		kol_cofflib_procload (imp, "con_printf");
if (printf == NULL)
	kol_exit();

_exit2 = ( _stdcall void (*)(char))
		kol_cofflib_procload (imp, "con_exit");
if (_exit2 == NULL)
	kol_exit();

gets = ( _stdcall void (*)(char*, int))
		kol_cofflib_procload (imp, "con_gets");
if (gets == NULL)
	kol_exit();

getch = ( _stdcall int (*)(void))
		kol_cofflib_procload (imp, "con_getch2");
if (getch == NULL)
	kol_exit();
    
kbhit = ( _stdcall int (*)(void))
		kol_cofflib_procload (imp, "con_kbhit");
if (getch == NULL)
	kol_exit();

con_get_font_height = ( _stdcall int (*)(void))
		kol_cofflib_procload (imp, "con_get_font_height");
if (con_get_font_height == NULL)
	kol_exit();

con_set_cursor_height = ( _stdcall int (*)(int))
		kol_cofflib_procload (imp, "con_set_cursor_height");
if (con_set_cursor_height == NULL)
	kol_exit();

con_get_flags = ( _stdcall unsigned (*)(void))
		kol_cofflib_procload (imp, "con_get_flags");
if (con_get_flags == NULL)
	kol_exit();

con_set_flags = ( _stdcall unsigned (*)(unsigned))
		kol_cofflib_procload (imp, "con_set_flags");
if (con_set_flags == NULL)
	kol_exit();

con_cls = ( _stdcall void (*)(void))
		kol_cofflib_procload (imp, "con_cls");
if (con_cls == NULL)
	kol_exit();

__menuet__debug_out ("success");
con_init(-1, -1, -1, -1, title);
}
