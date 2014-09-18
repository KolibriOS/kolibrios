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
 "@(#) $Id: SDL_systimer.c,v 1.2 2001/04/26 16:50:18 hercules Exp $";
#endif

#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "SDL_error.h"
#include "SDL_timer.h"
#include "SDL_timer_c.h"

#if _POSIX_THREAD_SYSCALL_SOFT
#include <pthread.h>
#endif

#if defined(DISABLE_THREADS) || defined(FORK_HACK)
#define USE_ITIMER
#endif


/* The first ticks value of the application */
//static struct timeval start;
//static unsigned startlo,starthi;
//static unsigned clockrate;
static unsigned starttime;

void SDL_StartTicks(void)
{
// gettimeofday(&start, NULL);
//  __asm__ ("int $0x40" : "=a"(clockrate) : "a"(18),"b"(5));
//  __asm__ ("rdtsc" : "=a"(startlo),"=d"(starthi));
	__asm__ ("int $0x40" : "=a"(starttime) : "a"(26),"b"(9));
}


Uint32 SDL_GetTicks (void)
{
/* struct timeval now;
 Uint32 ticks;
 gettimeofday(&now, NULL);
 ticks=(now.tv_sec-start.tv_sec)*1000+(now.tv_usec-start.tv_usec)/1000;
 return(ticks);*/
	/*int res;
	__asm__ ("rdtsc\n\t"
		"sub (_startlo),%%eax\n\t"
		"sbb (_starthi),%%edx\n\t"
		"push %%eax\n\t"
		"mov %%edx,%%eax\n\t"
		"mov $1000,%%ecx\n\t"
		"mul %%ecx\n\t"
		"xchg (%%esp),%%eax\n\t"
		"mul %%ecx\n\t"
		"add %%edx,(%%esp)\n\t"
		"pop %%edx\n\t"
		"divl (_clockrate)\n\t" : "=a"(res));
	return res;*/
	unsigned curtime;
	__asm__ ("int $0x40" : "=a"(curtime) : "a"(26),"b"(9));
	return (curtime-starttime)*10;
}

void SDL_Delay (Uint32 ms)
{
 __menuet__delay100(ms);
/*  Uint32 start = SDL_GetTicks();
  do
    __asm__("int $0x40" :: "a"(68),"b"(1));
  while (SDL_GetTicks()-start < ms);*/
}

int SDL_SYS_TimerInit(void)
{
	return(0);
}

void SDL_SYS_TimerQuit(void)
{
}

int SDL_SYS_StartTimer(void)
{
	return(0);
}

void SDL_SYS_StopTimer(void)
{
}
