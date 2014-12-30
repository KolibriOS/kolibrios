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

#ifndef H_INTERFACE
#define H_INTERFACE
#include <string>

char getAnswer(char a1, char a2);
char getAnswer(char a1, char a2, char a3);
char getAnswer(char a1, char a2, char a3, char a4);
char getKey();
void clearBuffer();

#ifdef _KOS32
using std::string;
string itos(int n);
string txt(string s, int d);
string txt(string s, string s2);
string txt(string s, string s2, string s3);

string txt(string s, char c);
string txt(string s, char ch, string s2);
string txt(string s, char ch, string s2, char ch2);

string txt(string s, int d1, int d2);
string txt(string s, int d1, int d2, int d3);
string txt(string s, int d1, int d2, int d3, int d4);

#else
std::string txt(const char* s, ...);
#endif

void wait(char a = ENTER_KEY, char b = ENTER_KEY);
void drawWindow(std::string content, const char* title = 0, std::string topline = "", std::string bottomline = "", bool usePagesForLongText = false);
void drawModalWindow(const char* content , const char* title = 0, const char* buttons = 0);
#endif
