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

#ifndef H_SYS
#define H_SYS

#define ESCAPE_KEY 27

#if defined(__linux__) && !defined(_KOS32)
    #define ENTER_KEY '\n'
#else
    #define ENTER_KEY 13
#endif

#ifdef _KOS32
#define kbhit kbhit // Prevent redeclaring kbhit() in pc.h
void CONSOLE_INIT(char* title);
void kol_exit();
extern void (* __attribute__((stdcall)) con_exit)(char bCloseWindow);
extern void (* __attribute__((cdecl))   printf2)(const char* format,...);
extern  int (* __attribute__((stdcall)) getch)();
extern  int (* __attribute__((stdcall)) kbhit)();
extern void (* __attribute__((stdcall)) cls)();

#else
void cls();
char getch();
int  kbhit ();
#endif

void initConsole();
void hideCursor();
void showCursor();
void consoleGoto(int line, int column);
void getWinWH(int& width, int &height);
#endif
