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

/* General quit handling code for SDL */

#include <stdio.h>
#ifndef NO_SIGNAL_H
#include <signal.h>
#endif

#include "SDL_events.h"
#include "SDL_events_c.h"


#ifndef NO_SIGNAL_H
static void SDL_HandleSIG(int sig)
{
	/* Reset the signal handler */
	signal(sig, SDL_HandleSIG);

	/* Signal a quit interrupt */
	SDL_PrivateQuit();
}
#endif /* NO_SIGNAL_H */

/* Public functions */
int SDL_QuitInit(void)
{
#ifndef NO_SIGNAL_H
	void (*ohandler)(int);

	/* Both SIGINT and SIGTERM are translated into quit interrupts */
	ohandler = signal(SIGINT,  SDL_HandleSIG);
	if ( ohandler != SIG_DFL )
		signal(SIGINT, ohandler);
	ohandler = signal(SIGTERM, SDL_HandleSIG);
	if ( ohandler != SIG_DFL )
		signal(SIGTERM, ohandler);
#endif /* NO_SIGNAL_H */

	/* That's it! */
	return(0);
}

/* This function returns 1 if it's okay to close the application window */
int SDL_PrivateQuit(void)
{
	int posted;

	posted = 0;
	if ( SDL_ProcessEvents[SDL_QUIT] == SDL_ENABLE ) {
		SDL_Event event;
		event.type = SDL_QUIT;
		if ( (SDL_EventOK == NULL) || (*SDL_EventOK)(&event) ) {
			posted = 1;
			SDL_PushEvent(&event);
		}
	}
	return(posted);
}
