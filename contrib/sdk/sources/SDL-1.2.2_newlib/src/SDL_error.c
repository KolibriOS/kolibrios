/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@devolution.com
*/

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id: SDL_error.c,v 1.2 2001/04/26 16:50:17 hercules Exp $";
#endif

/* Simple error handling in SDL */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "SDL_types.h"
#include "SDL_getenv.h"
#include "SDL_error.h"
#include "SDL_error_c.h"
#ifndef DISABLE_THREADS
#include "SDL_thread_c.h"
#endif

#ifdef DISABLE_THREADS
/* The default (non-thread-safe) global error variable */
static SDL_error SDL_global_error;

#define SDL_GetErrBuf()	(&SDL_global_error)
#endif /* DISABLE_THREADS */

#ifdef __CYGWIN__
#define DISABLE_STDIO
#endif

#define SDL_ERRBUFIZE	1024

/* Private functions */

static int vdprintf_help(unsigned c)
{ 
 int d0;
 if(c=='\n')
 {
  c='\r';
  __asm__ __volatile__("int $0x40":"=&a"(d0):"0"(63),"b"(1),"c"(c));
  c='\n';
  __asm__ __volatile__("int $0x40":"=&a"(d0):"0"(63),"b"(1),"c"(c));
  return 0;
 }
 __asm__ __volatile__("int $0x40":"=&a"(d0):"0"(63),"b"(1),"c"(c));
 return 0 ;
}

static void xputs(char * p)
{
 for(;*p;p++) vdprintf_help((*p)&0xff);
}

static char dbg_buf[1024];

void SDL_printf(const char * fmt,...)
{
 va_list ap;
 va_start(ap,fmt);
 vsprintf(dbg_buf,fmt,ap);
 va_end(ap);
 xputs(dbg_buf);
}

static void SDL_LookupString(const Uint8 *key, Uint16 *buf, int buflen)
{
	/* FIXME: Add code to lookup key in language string hash-table */

	/* Key not found in language string hash-table */
	while ( *key && (--buflen > 0) ) {
		*buf++ = *key++;
	}
	*buf = 0;	/* NULL terminate string */
}

/* Public functions */

void SDL_SetError (const char *fmt, ...)
{
	va_list ap;
	SDL_error *error;

	/* Copy in the key, mark error as valid */
	error = SDL_GetErrBuf();
	error->error = 1;
	strncpy((char *)error->key, fmt, sizeof(error->key));
	error->key[sizeof(error->key)-1] = '\0';

	va_start(ap, fmt);
	error->argc = 0;
	while ( *fmt ) {
		if ( *fmt++ == '%' ) {
			switch (*fmt++) {
			    case 0:  /* Malformed format string.. */
				--fmt;
				break;
#if 0	/* What is a character anyway?  (UNICODE issues) */
			    case 'c':
				error->args[error->argc++].value_c =
						va_arg(ap, unsigned char);
				break;
#endif
			    case 'd':
				error->args[error->argc++].value_i =
							va_arg(ap, int);
				break;
			    case 'f':
				error->args[error->argc++].value_f =
							va_arg(ap, double);
				break;
			    case 'p':
				error->args[error->argc++].value_ptr =
							va_arg(ap, void *);
				break;
			    case 's':
				{
				  int index = error->argc;
				  strncpy((char *)error->args[index].buf,
					va_arg(ap, char *), ERR_MAX_STRLEN);
				  error->args[index].buf[ERR_MAX_STRLEN-1] = 0;
				  error->argc++;
				}
				break;
			    default:
				break;
			}
			if ( error->argc >= ERR_MAX_ARGS ) {
				break;
			}
		}
	}
	va_end(ap);

	SDL_printf("SDL_SetError: %s\n", SDL_GetError());
}

/* Print out an integer value to a UNICODE buffer */
static int PrintInt(Uint16 *str, unsigned int maxlen, int value)
{
	char tmp[128];
	int len, i;

	sprintf(tmp, "%d", value);
	len = 0;
	if ( strlen(tmp) < maxlen ) {
		for ( i=0; tmp[i]; ++i ) {
			*str++ = tmp[i];
			++len;
		}
	}
	return(len);
}
/* Print out a double value to a UNICODE buffer */
static int PrintDouble(Uint16 *str, unsigned int maxlen, double value)
{
	char tmp[128];
	int len, i;

	sprintf(tmp, "%f", value);
	len = 0;
	if ( strlen(tmp) < maxlen ) {
		for ( i=0; tmp[i]; ++i ) {
			*str++ = tmp[i];
			++len;
		}
	}
	return(len);
}
/* Print out a pointer value to a UNICODE buffer */
static int PrintPointer(Uint16 *str, unsigned int maxlen, void *value)
{
	char tmp[128];
	int len, i;

	sprintf(tmp, "%p", value);
	len = 0;
	if ( strlen(tmp) < maxlen ) {
		for ( i=0; tmp[i]; ++i ) {
			*str++ = tmp[i];
			++len;
		}
	}
	return(len);
}

/* This function has a bit more overhead than most error functions
   so that it supports internationalization and thread-safe errors.
*/
Uint16 *SDL_GetErrorMsgUNICODE(Uint16 *errstr, unsigned int maxlen)
{
	SDL_error *error;

	/* Clear the error string */
	*errstr = 0; --maxlen;

	/* Get the thread-safe error, and print it out */
	error = SDL_GetErrBuf();
	if ( error->error ) {
		Uint16 translated[ERR_MAX_STRLEN], *fmt, *msg;
		int len;
		int argi;

		/* Print out the UNICODE error message */
		SDL_LookupString(error->key, translated, sizeof(translated));
		msg = errstr;
		argi = 0;
		for ( fmt=translated; *fmt && (maxlen > 0); ) {
			if ( *fmt == '%' ) {
				switch (fmt[1]) {
				    case 'S':	/* Special SKIP operand */
					argi += (fmt[2] - '0');
					++fmt;
					break;
				    case '%':
					*msg++ = '%';
					maxlen -= 1;
					break;
#if 0	/* What is a character anyway?  (UNICODE issues) */
				    case 'c':
                                        *msg++ = (unsigned char)
					         error->args[argi++].value_c;
					maxlen -= 1;
					break;
#endif
				    case 'd':
					len = PrintInt(msg, maxlen,
						error->args[argi++].value_i);
					msg += len;
					maxlen -= len;
					break;
				    case 'f':
					len = PrintDouble(msg, maxlen,
						error->args[argi++].value_f);
					msg += len;
					maxlen -= len;
					break;
				    case 'p':
					len = PrintPointer(msg, maxlen,
						error->args[argi++].value_ptr);
					msg += len;
					maxlen -= len;
					break;
				    case 's': /* UNICODE string */
					{ Uint16 buf[ERR_MAX_STRLEN], *str;
					  SDL_LookupString(error->args[argi++].buf, buf, sizeof(buf));
					  str = buf;
					  while ( *str && (maxlen > 0) ) {
						*msg++ = *str++;
						maxlen -= 1;
					  }
					}
					break;
				}
				fmt += 2;
			} else {
				*msg++ = *fmt++;
				maxlen -= 1;
			}
		}
		*msg = 0;	/* NULL terminate the string */
	}
	return(errstr);
}

Uint8 *SDL_GetErrorMsg(Uint8 *errstr, unsigned int maxlen)
{
	Uint16 *errstr16;
	unsigned int i;

	/* Allocate the UNICODE buffer */
	errstr16 = (Uint16 *)malloc(maxlen * (sizeof *errstr16));
	if ( ! errstr16 ) {
		strncpy((char *)errstr, "Out of memory", maxlen);
		errstr[maxlen-1] = '\0';
		return(errstr);
	}

	/* Get the error message */
	SDL_GetErrorMsgUNICODE(errstr16, maxlen);

	/* Convert from UNICODE to Latin1 encoding */
	for ( i=0; i<maxlen; ++i ) {
		errstr[i] = (Uint8)errstr16[i];
	}

	/* Free UNICODE buffer (if necessary) */
	free(errstr16);

	return(errstr);
}

/* Available for backwards compatibility */
char *SDL_GetError (void)
{
	static char errmsg[SDL_ERRBUFIZE];

	return((char *)SDL_GetErrorMsg((unsigned char *)errmsg, SDL_ERRBUFIZE));
}

void SDL_ClearError(void)
{
	SDL_error *error;

	error = SDL_GetErrBuf();
	error->error = 0;
}

/* Very common errors go here */
void SDL_Error(SDL_errorcode code)
{
	switch (code) {
		case SDL_ENOMEM:
			SDL_SetError("Out of memory");
			break;
		case SDL_EFREAD:
			SDL_SetError("Error reading from datastream");
			break;
		case SDL_EFWRITE:
			SDL_SetError("Error writing to datastream");
			break;
		case SDL_EFSEEK:
			SDL_SetError("Error seeking in datastream");
			break;
		default:
			SDL_SetError("Unknown SDL error");
			break;
	}
}
