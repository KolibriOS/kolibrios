
///===========================

#define CON_COLOR_BLUE		1
#define CON_COLOR_GREEN		2
#define CON_COLOR_RED		4
#define CON_COLOR_BRIGHT	8
/* цвет фона */
#define CON_BGR_BLUE		0x10
#define CON_BGR_GREEN		0x20
#define CON_BGR_RED			0x40
#define CON_BGR_BRIGHT		0x80

///===========================
char console_init_command = FALSE;
static inline void (* _stdcall con_init)(unsigned w_w, unsigned w_h, unsigned s_w, unsigned s_h, const char* t);
static inline void (* _cdecl _printf)(const char* format,...);
static inline void (* _stdcall _print)(const char* text);
static inline void (* _stdcall _exit)(char bCloseWindow);
static inline void (* __stdcall _gets)(char* str, int n);
static inline int (* __stdcall _getch)(void);
static inline int (* __stdcall con_get_font_height)(void);
static inline int (* __stdcall con_set_cursor_height)(int new_height);
unsigned (*__stdcall con_get_flags)(void);
unsigned (*__stdcall con_set_flags)(unsigned new_flags);
static inline void (*__stdcall con_cls)(void);

///===========================

static inline void print(const char *text)
{
	_print(text);
}

char _buf_gets[255];
static inline char *gets(void)
{
	CONSOLE_INIT("console");
	_gets(&_buf_gets,255);
	return &_buf_gets;
}
static inline char getch(void)
{
	CONSOLE_INIT("console");
	_getch();
}

static inline void CONSOLE_INIT(char title[])
{

if(console_init_command)return;

struct_import *imp;

imp = cofflib_load("/sys/lib/console.obj");
if (imp == NULL)
	exit();

con_init = ( _stdcall  void (*)(unsigned, unsigned, unsigned, unsigned, const char*)) 
		cofflib_procload (imp, "con_init");
if (con_init == NULL)
	exit();

_printf = ( _cdecl void (*)(const char*,...))
		cofflib_procload (imp, "con_printf");
if (_printf == NULL)
	exit();
	
_print = ( _cdecl void (*)(const char*))
		cofflib_procload (imp, "con_write_asciiz");
if (_printf == NULL)
	exit();

_exit = ( _stdcall void (*)(char))
		cofflib_procload (imp, "con_exit");
if (_exit == NULL)
	exit();

_gets = ( _stdcall void (*)(char*, int))
		cofflib_procload (imp, "con_gets");
if (_gets == NULL)
	exit();

_getch = ( _stdcall int (*)(void))
		cofflib_procload (imp, "con_getch2");
if (_getch == NULL)
	exit();

con_get_font_height = ( _stdcall int (*)(void))
		cofflib_procload (imp, "con_get_font_height");
if (con_get_font_height == NULL)
	exit();

con_set_cursor_height = ( _stdcall int (*)(int))
		cofflib_procload (imp, "con_set_cursor_height");
if (con_set_cursor_height == NULL)
	exit();

con_get_flags = ( _stdcall unsigned (*)(void))
		cofflib_procload (imp, "con_get_flags");
if (con_get_flags == NULL)
	exit();

con_set_flags = ( _stdcall unsigned (*)(unsigned))
		cofflib_procload (imp, "con_set_flags");
if (con_set_flags == NULL)
	exit();

con_cls = ( _stdcall void (*)(void))
		cofflib_procload (imp, "con_cls");
if (con_cls == NULL)
	exit();

con_init(-1, -1, -1, -1, title);

console_init_command = TRUE;
}
