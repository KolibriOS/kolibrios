/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997, 1998  Sam Lantinga

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
    5635-34 Springhouse Dr.
    Pleasanton, CA 94588 (USA)
    slouken@devolution.com
*/

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id: SDL_systimer.h,v 1.2 2001/04/26 16:50:18 hercules Exp $";
#endif

/* The system dependent timer handling functions */

#include "SDL_timer.h"
#include "SDL_timer_c.h"


/* Initialize the system dependent timer subsystem */
extern int SDL_SYS_TimerInit(void);

/* Quit the system dependent timer subsystem */
extern void SDL_SYS_TimerQuit(void);

/* Start a timer set up by SDL_SetTimer() */
extern int SDL_SYS_StartTimer(void);

/* Stop a previously started timer */
extern void SDL_SYS_StopTimer(void);
