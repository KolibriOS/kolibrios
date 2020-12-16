#ifndef __GLIBC__MENUET_CONSOLE_H
#define __GLIBC__MENUET_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include<menuet/os.h>

extern const unsigned char font_8x8[];
extern void __menuet__redraw_console(void);
extern void __menuet__show_text_cursor(void);
extern void __menuet__hide_text_cursor(void);
extern void __menuet__init_console(int x,int y);
extern void __menuet__clrscr(void);
extern void __menuet__putch(char c);
extern void __menuet__gotoxy(int x,int y);
extern void __menuet__outtextxy(int x,int y,char c);
extern void __menuet__getxy(int * x,int * y);

#ifdef __cplusplus
}
#endif

#endif
