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

#include "sys.h"

#ifndef _KOS32
#include <stdio.h>   // printf()
#define printf2 printf
#endif

#include <stdlib.h>  // srand()
#include <time.h>    // time()

int showMainMenu();

int main() {
    initConsole();
    hideCursor();
    srand(time(NULL));
    showMainMenu();
    showCursor();
#ifdef _KOS32
    // Close console window and kill the main thread
    con_exit(true);
    kol_exit();
#endif
    return 0;
    }
