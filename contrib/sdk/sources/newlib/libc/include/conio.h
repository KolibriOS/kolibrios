#ifndef _CONIO_H_
#define _CONIO_H_

#include <stdint.h>

#define CON_WINDOW_CLOSED 0x200
#define CON_COLOR_BLUE 0x01
#define CON_COLOR_GREEN 0x02
#define CON_COLOR_RED 0x04
#define CON_COLOR_BRIGHT 0x08
/* background color */

#define CON_BGR_BLUE 0x10
#define CON_BGR_GREEN 0x20
#define CON_BGR_RED 0x40
#define CON_BGR_BRIGHT 0x80
/* output controls */

#define CON_IGNORE_SPECIALS 0x100
 
#define __con_api __attribute__((stdcall)) __attribute__((dllimport))

#ifdef __cplusplus
extern "C"
{
#endif

int   __con_api con_init(uint32_t wnd_width, uint32_t wnd_height, uint32_t scr_width, uint32_t scr_height, const char* title);
void  __con_api con_exit(int bCloseWindow);
void  __con_api  con_set_title(const char* title);
void  __con_api con_write_asciiz(const char* str);
void  __con_api con_write_string(const char* str, uint32_t length);
uint32_t __con_api con_get_flags(void);
uint32_t __con_api con_set_flags(uint32_t new_flags);
int   __con_api con_get_font_height(void);
int   __con_api con_get_cursor_height(void);
int   __con_api con_set_cursor_height(int new_height);
int   __con_api con_getch(void);
uint16_t  __con_api con_getch2(void);
int   __con_api con_kbhit(void);
char* __con_api con_gets(char* str, int n);
typedef int __con_api (*con_gets2_callback)(int keycode, char** pstr, int* pn, int* ppos);
char*  __con_api con_gets2(con_gets2_callback callback, char* str, int n);
void  __con_api con_cls(void);
void  __con_api con_get_cursor_pos(int* px, int* py);
void  __con_api con_set_cursor_pos(int x, int y);

#ifdef __cplusplus
};
#endif

#endif
