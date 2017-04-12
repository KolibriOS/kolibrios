#ifndef INCLUDE_CONSOLE_H
#define INCLUDE_CONSOLE_H

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

#ifndef INCLUDE_DLL_H
#include "../lib/dll.h"
#endif

dword libConsole = #alibConsole;
char alibConsole[] = "/sys/lib/console.obj";

dword con_start             = #a_con_start;
dword con_init              = #a_con_init;
dword con_write_asciiz      = #a_con_write_asciiz;
dword con_write_string      = #a_con_write_string;
dword con_printf            = #a_con_printf;
dword con_exit              = #a_con_exit;
dword con_get_flags         = #a_con_get_flags;
dword con_set_flags         = #a_con_set_flags;
dword con_kbhit             = #a_con_kbhit;
dword con_getch             = #a_con_getch;
dword con_getch2            = #a_con_getch2;
dword con_gets              = #a_con_gets;
dword con_gets2             = #a_con_gets2;
dword con_get_font_height   = #a_con_get_font_height;
dword con_get_cursor_height = #a_con_get_cursor_height;
dword con_set_cursor_height = #a_con_set_cursor_height;
dword con_cls               = #a_con_cls;
dword con_get_cursor_pos    = #a_con_get_cursor_pos;
dword con_set_cursor_pos    = #a_con_set_cursor_pos;
dword con_set_title         = #a_con_set_title;
$DD 2 dup 0

char a_con_start[]             = "START";
char a_con_init[]              = "con_init";
char a_con_write_asciiz[]      = "con_write_asciiz";
char a_con_write_string[]      = "con_write_string";
char a_con_printf[]            = "con_printf";
char a_con_exit[]              = "con_exit";
char a_con_get_flags[]         = "con_get_flags";
char a_con_set_flags[]         = "con_set_flags";
char a_con_kbhit[]             = "con_kbhit";
char a_con_getch[]             = "con_getch";
char a_con_getch2[]            = "con_getch2";
char a_con_gets[]              = "con_gets";
char a_con_gets2[]             = "con_gets2";
char a_con_get_font_height[]   = "con_get_font_height";
char a_con_get_cursor_height[] = "con_get_cursor_height";
char a_con_set_cursor_height[] = "con_set_cursor_height";
char a_con_cls[]               = "con_cls";
char a_con_get_cursor_pos[]    = "con_get_cursor_pos";
char a_con_set_cursor_pos[]    = "con_set_cursor_pos";
char a_con_set_title[]         = "con_set_title";

// text color
#define CON_COLOR_BLUE   0x01
#define CON_COLOR_GREEN  0x02
#define CON_COLOR_RED    0x04
#define CON_COLOR_BRIGHT 0x08

// background color
#define CON_BGR_BLUE   0x10
#define CON_BGR_GREEN  0x20
#define CON_BGR_RED    0x40
#define CON_BGR_BRIGHT 0x80

// special
#define CON_IGNORE_SPECIALS 0x100
#define CON_WINDOW_CLOSED   0x200

#endif