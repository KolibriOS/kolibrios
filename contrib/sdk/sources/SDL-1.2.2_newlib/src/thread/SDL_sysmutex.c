/* WARNING:  This file was automatically generated!
 * Original: ./src/thread/generic/SDL_sysmutex.c
 */
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


/*
 * KolibriOS mutex implementation.
 *
 * SDL here is built with -DDISABLE_THREADS, which would normally turn
 * SDL semaphores into stubs (returning NULL) and SDL mutexes into no-ops.
 * However, the KolibriOS audio backend (SDL_kolibri_audio.c) runs the SDL
 * audio callback in a REAL kernel thread created with _ksys_create_thread.
 * Applications such as ScummVM use SDL mutexes (via Common::Mutex) to guard
 * shared state (the audio mixer, software music drivers, ...) against
 * concurrent access from that audio thread and the main thread. With no-op
 * mutexes this synchronization silently disappears, leading to races and
 * crashes (e.g. a function pointer read while being overwritten -> a call
 * through a near-null pointer).
 *
 * So we provide a genuine, recursive mutex here, implemented as a small
 * spinlock that yields the CPU to other threads while waiting. It does not
 * rely on SDL semaphores (which stay stubbed out under DISABLE_THREADS).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/ksys.h>

#include "SDL_error.h"
#include "SDL_thread.h"
#include "SDL_systhread_c.h"


struct SDL_mutex {
	volatile int guard;       /* spinlock guarding the fields below   */
	volatile uint32_t owner;  /* tid of the owning thread, 0 if free  */
	volatile int recursive;   /* current lock/recursion depth         */
};

static uint32_t kos_current_tid(void)
{
	ksys_thread_t ti;
	_ksys_thread_info(&ti, KSYS_THIS_SLOT);
	return ti.pid;
}

/* Create a mutex */
SDL_mutex *SDL_CreateMutex(void)
{
	SDL_mutex *mutex = (SDL_mutex *)calloc(1, sizeof(*mutex));
	if ( ! mutex ) {
		SDL_OutOfMemory();
	}
	return mutex;
}

/* Free the mutex */
void SDL_DestroyMutex(SDL_mutex *mutex)
{
	if ( mutex ) {
		free(mutex);
	}
}

/* Lock the mutex (recursive) */
int SDL_mutexP(SDL_mutex *mutex)
{
	uint32_t me;

	/* Some SDL subsystems (e.g. the event queue) never allocate a mutex
	   under DISABLE_THREADS and pass NULL here. Those are only touched from
	   the main thread, so treat a NULL mutex as a silent no-op instead of
	   flooding errors. Real mutexes (the mixer's, ...) are non-NULL and are
	   genuinely locked below. */
	if ( mutex == NULL )
		return 0;

	me = kos_current_tid();
	for ( ;; ) {
		/* acquire the short internal guard */
		while ( __sync_lock_test_and_set(&mutex->guard, 1) )
			_ksys_thread_yield();

		if ( mutex->owner == 0 || mutex->owner == me ) {
			mutex->owner = me;
			++mutex->recursive;
			__sync_lock_release(&mutex->guard);
			return 0;
		}

		/* owned by another thread: drop the guard, yield and retry */
		__sync_lock_release(&mutex->guard);
		_ksys_thread_yield();
	}
}

/* Unlock the mutex */
int SDL_mutexV(SDL_mutex *mutex)
{
	if ( mutex == NULL )
		return 0;	/* no-op for NULL mutexes (see SDL_mutexP) */

	while ( __sync_lock_test_and_set(&mutex->guard, 1) )
		_ksys_thread_yield();

	if ( mutex->recursive > 0 ) {
		if ( --mutex->recursive == 0 )
			mutex->owner = 0;
	}

	__sync_lock_release(&mutex->guard);
	return 0;
}
