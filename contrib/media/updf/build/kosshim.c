/*
 * KolibriOS libc gap-fillers for the MuPDF 1.19 port.
 * The dynamic libc.dll does not export these, and MuPDF 1.19 (unlike 0.9)
 * pulls them in. Implemented self-contained, no extra dependencies.
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ---- process control -------------------------------------------------- */

void abort(void)
{
	exit(1);
}

int atexit(void (*fn)(void))
{
	(void)fn;      /* KolibriOS app exits via exit(); no handler chain */
	return 0;
}

/* ---- string/number ---------------------------------------------------- */

long long atoll(const char *s)
{
	long long v = 0;
	int neg = 0;
	while (*s == ' ' || *s == '\t') s++;
	if (*s == '-') { neg = 1; s++; }
	else if (*s == '+') s++;
	while (*s >= '0' && *s <= '9')
		v = v * 10 + (*s++ - '0');
	return neg ? -v : v;
}

/* ---- filesystem ------------------------------------------------------- */

/* KolibriOS paths are already absolute (/hd0/1/...); just copy through. */
char *realpath(const char *path, char *resolved)
{
	if (!resolved)
		return NULL;
	strcpy(resolved, path);
	return resolved;
}

/* ---- time ------------------------------------------------------------- */

/* Convert a broken-down UTC time to seconds since the Unix epoch.
   Used by MuPDF only to parse PDF /CreationDate style strings. */
time_t timegm(struct tm *tm)
{
	static const int mdays[12] =
		{ 0,31,59,90,120,151,181,212,243,273,304,334 };
	long year = tm->tm_year + 1900;
	long days;
	long leaps;

	leaps = (year - 1969) / 4 - (year - 1901) / 100 + (year - 1601) / 400;
	days = (year - 1970) * 365 + leaps + mdays[tm->tm_mon % 12] + (tm->tm_mday - 1);
	if (tm->tm_mon >= 2 &&
	    ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0))
		days += 1; /* this year's Feb 29 (leaps only counts through Jan 1) */

	return (time_t)(((days * 24 + tm->tm_hour) * 60 + tm->tm_min) * 60 + tm->tm_sec);
}

/* ---- thread-local storage (single-threaded stub) ---------------------- */
/* Pulled in via newlib's gthread reentrancy layer. We never spawn threads
   in the viewer, so a fixed key with no real storage is sufficient. */

unsigned int tls_alloc(void)
{
	return 0x80; /* any nonzero key */
}

int tls_free(unsigned int key)
{
	(void)key;
	return 0;
}
