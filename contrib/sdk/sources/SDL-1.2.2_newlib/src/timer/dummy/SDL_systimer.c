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
#include <time.h>
#include <string.h>
#include <errno.h>

#include "SDL_error.h"
#include "SDL_timer.h"
#include "SDL_timer_c.h"
#include <sys/ksys.h>

#if _POSIX_THREAD_SYSCALL_SOFT
#include <pthread.h>
#endif

#if defined(DISABLE_THREADS) || defined(FORK_HACK)
#define USE_ITIMER
#endif

static unsigned starttime;

void SDL_StartTicks(void)
{
	starttime = _ksys_get_tick_count();
}


Uint32 SDL_GetTicks (void)
{
	unsigned curtime = _ksys_get_tick_count();
	return (curtime-starttime)*10;
}

void SDL_Delay(unsigned ms){
    _ksys_delay(ms/10+(ms%10>0));
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
