#ifndef _CONIO_H
#define _CONIO_H

int _getch();
int _kbhit();

int con_init_console_dll(void);
int con_init_console_dll_param(uint32_t wnd_width, uint32_t wnd_height, uint32_t scr_width, uint32_t scr_height, const char* title);
void con_lib_link(void *exp, char** imports);

#endif