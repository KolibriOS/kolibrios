#pragma once

#include <stdint.h>

#define ERROR     -1
#define ERROR_END -2

// set by convert(): 0 ok, ERROR bad char, ERROR_END empty/no number
extern int convert_error;

// parse a number (accepts '.' or ',' as the decimal point); *len gets the
// count of consumed chars when non-NULL
double convert(char *s, int *len);

// double -> freshly malloc'd string ("%f", 6 decimals)
char *ftoa(double d);

// case-insensitive compare of the first n bytes (ASCII); not in libc
int strnicmp(const char *a, const char *b, unsigned n);

// text with a KolibriOS fn4 font flag (0x10 small, 0x90 big)
void draw_text(int x, int y, unsigned font, uint32_t color, const char *s, int len);
// line; invert=1 draws an XOR (rubber-band) line
void draw_line(int x1, int y1, int x2, int y2, uint32_t color, int invert);
// rectangle outline
void draw_region(int x, int y, int w, int h, uint32_t color);
// text clipped to area_w pixels (small font)
void draw_cut_text(int x, int y, int area_w, uint32_t color, const char *s);
