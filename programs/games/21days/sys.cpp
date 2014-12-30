/******************************************************************
*   21 days: a game for programmers
*   Copyright (C) 2014 Maxim Grishin
*
*   This program is free software; you can redistribute it and/or
*   modify it under the terms of the GNU General Public License
*   as published by the Free Software Foundation; either version 2
*   of the License, or (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
*   MA  02110-1301, USA.
*******************************************************************/

char* windowTitle = "21 days by igevorse";

#ifdef _KOS32
#define kbhit kbhit // Prevent redeclaring kbhit() in pc.h
#include <string>
using std::string;

#pragma pack(push,1)
typedef struct {
    char	*name;
    void	*data;
} kol_struct_import;
#pragma pack(pop)

void kol_exit() {
    asm volatile ("int $0x40"::"a"(-1));
    }

void kol_screen_get_size(unsigned *w, unsigned *h) {
    unsigned size;
    asm volatile ("int $0x40":"=a"(size):"a"(14));
    *w = size / 65536;
    *h = size % 65536;
    }

void (* __attribute__((stdcall)) con_init)(unsigned w_w, unsigned w_h, unsigned s_w, unsigned s_h, const char* t);
void (* __attribute__((cdecl))   printf2)(const char* format,...);
void (* __attribute__((stdcall)) con_exit)(char bCloseWindow);
 int (* __attribute__((stdcall)) getch)();
 int (* __attribute__((stdcall)) kbhit)();
void (* __attribute__((stdcall)) con_set_cursor_pos) (int x, int y);
 int (* __attribute__((stdcall)) con_set_cursor_height)(int new_height);
void (* __attribute__((stdcall)) cls)();

int strcmp(const char* string1, const char* string2) {
	while (1) {
		if (*string1<*string2)
			return -1;
		if (*string1>*string2)
			return 1;

		if (*string1=='\0')
			return 0;
		string1++;
		string2++;
		}
	}

void* kol_cofflib_procload (kol_struct_import *imp, char *name) {
	for (int i=0;;i++)
		if (NULL == ((imp+i) -> name))
			break;
		else
			if (0 == strcmp(name, (imp+i)->name) )
				return (imp+i)->data;
	return NULL;
	}

kol_struct_import* kol_cofflib_load(char *name) {
	kol_struct_import* val;
	asm volatile ("int $0x40":"=a"(val):"a"(68), "b"(19), "c"(name));
	return val;
	}

unsigned _kWinWidth = 100;
unsigned _kWinHeight = 30;

void getWinWH(int& width, int& height) {
    width  = _kWinWidth;
    height = _kWinHeight-1;
    }

void CONSOLE_INIT(char* title) {
    kol_struct_import *imp = kol_cofflib_load("/sys/lib/console.obj");
	if (imp == 0)
      		kol_exit();

	con_init = ( __attribute__((stdcall))  void (*)(unsigned, unsigned, unsigned, unsigned, const char*))
		kol_cofflib_procload (imp, "con_init");
	if (con_init == NULL)
		kol_exit();

	printf2 = ( __attribute__((cdecl)) void (*)(const char*,...))
		kol_cofflib_procload (imp, "con_printf");
	if (printf2 == NULL)
		kol_exit();

	con_exit = ( __attribute__((stdcall)) void (*)(char))
		kol_cofflib_procload (imp, "con_exit");
	if (con_exit == NULL)
		kol_exit();

	getch = ( __attribute__((stdcall)) int (*)())
		kol_cofflib_procload (imp, "con_getch");
	if (getch == NULL)
		kol_exit();

	cls = ( __attribute__((stdcall)) void (*)())
		kol_cofflib_procload (imp, "con_cls");
	if (cls == NULL)
		kol_exit();

	kbhit = ( __attribute__((stdcall)) int (*)())
		kol_cofflib_procload (imp, "con_kbhit");
    	if (kbhit == NULL)
		kol_exit();

	con_set_cursor_pos =  ( __attribute__((stdcall)) void (*) (int, int))
    		kol_cofflib_procload (imp, "con_set_cursor_pos");
    	if (con_set_cursor_pos == NULL)
		kol_exit();

	con_set_cursor_height = ( __attribute__((stdcall)) int (*)(int))
		kol_cofflib_procload (imp, "con_set_cursor_height");
	if (con_set_cursor_height == NULL)
		kol_exit();

    unsigned sw, sh;

    kol_screen_get_size(&sw, &sh);
    // Change window w/h depending on screen w/h
    // Sorry for magic numbers
    if (_kWinWidth*8 >= sw)
        _kWinWidth = sw / 8;

    if (_kWinHeight*17 >= sh)
        _kWinHeight = sh / 17;

	con_init(_kWinWidth, _kWinHeight, _kWinWidth, _kWinHeight, title);
    }

void hideCursor() {
	con_set_cursor_height(0);
	}

void showCursor() {
	//nop;
	}

void consoleGoto(int line, int column) {
    con_set_cursor_pos(column-1, line-1);
    }

string itos(int n) {
    string s;
    int i, sign;

    if ((sign = n) < 0)
        n = -n;
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';

    int j,k;
    for (k = 0, j = i-1; k<j; k++, j--) {
        char c = s[k];
        s[k] = s[j];
        s[j] = c;
        }
    return s;
    }
// Ok, I don't want to use vsprintf from KolibriOS
string txt(string s, int d) {
    string t = s;
    unsigned i = t.find("%d");
    if (i != string::npos)
        t.replace(i, 2, itos(d).c_str());
    return t;
    }

string txt(string s, char c) {
    string t = s;
    unsigned i = t.find("%c");
    if (i != string::npos)
        t.replace(i, 2, c);
    return t;
    }

string txt(string s, string s2) {
    string t = s;
    unsigned i = t.find("%s");
    if (i != string::npos)
        t.replace(i, 2, s2.c_str());
    return t;
    }

string txt(string s, string s2, string s3) {
    return txt(txt(s, s2), s3);
    }
string txt(string s, char ch, string s2) {
    return txt(txt(s, ch), s2);
    }
string txt(string s, char ch, string s2, char ch2) {
    return txt(txt(txt(s, ch), s2), ch2);
    }
string txt(string s, int d1, int d2) {
    return txt(txt(s, d1), d2);
    }
string txt(string s, int d1, int d2, int d3) {
    return txt(txt(txt(s, d1), d2), d3);
    }
string txt(string s, int d1, int d2, int d3, int d4) {
    return txt(txt(txt(txt(s, d1), d2), d3), d4);
    }

void initConsole() {
    CONSOLE_INIT(windowTitle);
    }
#elif defined __linux__
#define printf2 printf
#include <cstdlib>      // system
#include <stdarg.h>
#include <stdio.h>  // printf, vsprintf
#include <sys/ioctl.h>  // get window width/height
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
static struct termios old, new1;

std::string txt(const char* s, ...) {
    va_list args;
    va_start(args,s);
    char x[100000];
    vsprintf(x, s, args);
    va_end(args);
    std::string str(x);
    return str;
    }

void cls() {
    system("clear");
    }

void hideCursor() {
    printf2("\033[?25l");
    }

void showCursor() {
    printf2("\033[?25h");
    }

void consoleGoto(int line, int column) {
    printf2("\033[%d;%dH",  line, column);
    }

void consoleSetColor(short text = 30, short background = 47) {
    printf2("\033[%d;%dm", text, background);
    }

/* Initialize new terminal i/o settings */
void initTermios() {
    tcgetattr(0, &old); /* grab old terminal i/o settings */
    new1 = old; /* make new settings same as old settings */
    new1.c_lflag &= ~ICANON; /* disable buffered i/o */
    new1.c_lflag &= ~ECHO; /* set echo mode */
    tcsetattr(0, TCSANOW, &new1); /* use these new terminal i/o settings now */
    }

/* Restore old terminal i/o settings */
void resetTermios() {
    tcsetattr(0, TCSANOW, &old);
    }

int kbhit () {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
        }
    return 0;
    }

/* Read 1 character without echo */
char getch() {
    char ch;
    initTermios();
    ch = getchar();
    resetTermios();
    return ch;
    }

void getWinWH(int& width, int& height) {
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    width  = w.ws_col;
    height = w.ws_row-1;
    }

void initConsole() {
    printf2("\033]0;%s\007", windowTitle);
    }
#elif defined _WIN32
#define printf2 printf
#include <stdio.h>  // printf, vsprintf
#include <windows.h>
#include <cstdlib> // system
#define _NO_OLDNAMES _NO_OLDNAMES
#include <conio.h> // kbhit(), getch(), getche()
#include <string>

char getch() {
  return _getch();
  }

int kbhit (void) {
  return _kbhit();
  }

std::string txt(const char* s, ...) {
    va_list args;
    va_start(args,s);
    char x[100000];
    vsprintf(x, s, args);
    va_end(args);
    std::string str(x);
    return str;
    }

void cls() {
    system("cls");
    }

void showCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.bVisible = TRUE;
    SetConsoleCursorInfo(consoleHandle, &info);
    }

void hideCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.bVisible = FALSE;
    info.dwSize = 1;
    SetConsoleCursorInfo(consoleHandle, &info);
    }

void consoleGoto(int line, int column) {
    COORD coord;
    coord.X = column;
    coord.Y = line;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    }

void consoleSetColor(short text = 30, short background = 47) {
    printf2("\033[%d;%dm", text, background);
    }

void getWinWH(int& width, int& height) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    width = csbi.srWindow.Right - csbi.srWindow.Left - 1;
    height = csbi.srWindow.Bottom - csbi.srWindow.Top - 1;
    }

void initConsole() {
    SetConsoleTitle(windowTitle);
    system("mode 100, 30");
    }
#endif
