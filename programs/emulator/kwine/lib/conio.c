
char* con_caption = "Console app";
//extern int __argc;
//extern char** __argv;
//extern char* __path;
uint32_t   *con_dll_ver;
int   __console_initdll_status = 0;
 
char* con_dllname = "/sys/lib/console.obj";

typedef int (stdcall * con_gets2_callback)(int keycode, char** pstr, int* pn, int* ppos);


void stdcall (*con_init)(uint32_t wnd_width, uint32_t wnd_height, uint32_t scr_width, uint32_t scr_height, const char* title) = 0;
void stdcall (*con_exit)(int bCloseWindow) = 0;
void stdcall (*con_set_title)(const char* title) = 0;
void stdcall (*con_write_asciiz)(const char* str) = 0;
void stdcall (*con_write_string)(const char* str, uint32_t length) = 0;
int cdecl (*con_printf)(const char* format, ...) = 0;
uint32_t stdcall (*con_get_flags)(void) = 0;
uint32_t stdcall (*con_set_flags)(uint32_t new_flags) = 0;
int stdcall (*con_get_font_height)(void) = 0;
int stdcall (*con_get_cursor_height)(void) = 0;
int stdcall (*con_set_cursor_height)(int new_height) = 0;
int stdcall (*con_getch)(void) = 0;
uint16_t stdcall (*con_getch2)(void) = 0;
int stdcall (*con_kbhit)(void) = 0;
char* stdcall (*con_gets)(char* str, int n) = 0;
char* stdcall (*con_gets2)(con_gets2_callback callback, char* str, int n) = 0;
void stdcall (*con_cls)() = 0;
void stdcall (*con_get_cursor_pos)(int* px, int* py) = 0;
void stdcall (*con_set_cursor_pos)(int x, int y) = 0;



void *load_library(char *name) {
	void *exports;
    asm volatile ("int $0x40":"=a"(exports):"a"(68), "b"(19), "c"(name));
    return exports;
}

void *getprocaddress(void *exports, char *name)
{
	if (exports == NULL) { return 0; }
	while (*(uint32_t*)exports != 0)
	{
		char *str1 = (char*)(*(uint32_t*)exports);
		if (strcmp(str1, name) == 0)
		{
            void *ptr = (void*)*(uint32_t*)(exports + 4);

            // important for debug
            /*debug_board_write_string(name);
            char otv[16];
            itoa(ptr, otv);
            debug_board_write_string(otv);
            debug_board_write_byte('\n');*/

			return ptr;
		}
		exports += 8;
	}
	return 0;
}


// don't change order in this! linked by index
char* con_imports[] = {
    "START", "version", "con_init", "con_write_asciiz", "con_write_string",
    "con_printf", "con_exit", "con_get_flags", "con_set_flags", "con_kbhit",
    "con_getch", "con_getch2", "con_gets", "con_gets2", "con_get_font_height",
    "con_get_cursor_height", "con_set_cursor_height",  "con_cls",
    "con_get_cursor_pos", "con_set_cursor_pos", "con_set_title",
    (char*)0
};

void con_lib_link(void *exp, char** imports)
{ 
    con_dll_ver             = getprocaddress(exp, imports[1]);
    con_init                = getprocaddress(exp, imports[2]);
    con_write_asciiz        = getprocaddress(exp, imports[3]);
    con_write_string        = getprocaddress(exp, imports[4]);
    con_printf              = getprocaddress(exp, imports[5]);
    con_exit                = getprocaddress(exp, imports[6]);
    con_get_flags           = getprocaddress(exp, imports[7]);
    con_set_flags           = getprocaddress(exp, imports[8]);
    con_kbhit               = getprocaddress(exp, imports[9]);
    con_getch               = getprocaddress(exp, imports[10]);
    con_getch2              = getprocaddress(exp, imports[11]);
    con_gets                = getprocaddress(exp, imports[12]);
    con_gets2               = getprocaddress(exp, imports[13]);
    con_get_font_height     = getprocaddress(exp, imports[14]);
    con_get_cursor_height   = getprocaddress(exp, imports[15]);
    con_set_cursor_height   = getprocaddress(exp, imports[16]);
    con_cls                 = getprocaddress(exp, imports[17]);
    con_get_cursor_pos      = getprocaddress(exp, imports[18]);
    con_set_cursor_pos      = getprocaddress(exp, imports[19]);
    con_set_title           = getprocaddress(exp, imports[20]);
}
 
 
int con_init_console_dll_param(uint32_t wnd_width, uint32_t wnd_height, uint32_t scr_width, uint32_t scr_height, const char* title)
/*works as con_init_console_dll, but call con_init with params*/
{
    void *hDll;
    if (__console_initdll_status == 1) return 0;
    if((hDll = load_library(con_dllname)) == 0)
    {
            debug_out_str("can't load lib\n");
            return 1;
    }
    //debug_board_write_byte('I');
    con_lib_link(hDll, con_imports);
    /*if (con_dll_ver != (uint32_t*)0x00020008)
    {
    	//debug_board_write_byte(48 + sizeof(KosExp));
    	//char otv[16];
    	//itoa(con_init, otv);
    	//debug_board_write_string(otv);

    	debug_board_write_string("con_dll_ver=");
        char otv[16];
        itoa(con_dll_ver, otv);
        debug_board_write_string(otv);
        debug_board_write_byte('\n');

        debug_board_write_string("con_init=");
        //char otv[16];
        itoa(con_init, otv);
        debug_board_write_string(otv);
        debug_board_write_byte('\n');

    	debug_board_write_string("(wtf)");
    }*/
    con_init(wnd_width, wnd_height, scr_width, scr_height, title);
    __console_initdll_status = 1;
    return 0;
}


int con_init_console_dll(void)
{
    return con_init_console_dll_param(-1, -1, -1, -1, con_caption);
}


// --------------------------------------------------------------------

int _getch()
{
	con_init_console_dll();
	return con_getch();
}

int _kbhit()
{
	con_init_console_dll();
	return con_kbhit();
}