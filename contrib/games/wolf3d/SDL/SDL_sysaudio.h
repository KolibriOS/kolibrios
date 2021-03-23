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
 "@(#) $Id: SDL_sysaudio.h,v 1.8 2001/07/23 02:58:42 slouken Exp $";
#endif

#ifndef _SDL_sysaudio_h
#define _SDL_sysaudio_h

#include "SDL_mutex.h"
#include "SDL_thread.h"

/* The SDL audio driver */
typedef struct SDL_AudioDevice SDL_AudioDevice;

/* Define the SDL audio driver structure */
#define _THIS	SDL_AudioDevice *_this
#ifndef _STATUS
#define _STATUS	SDL_status *status
#endif
struct SDL_AudioDevice {
	/* * * */
	/* The name of this audio driver */
	const char *name;

	/* * * */
	/* The description of this audio driver */
	const char *desc;

	/* * * */
	/* Public driver functions */
	int  (*OpenAudio)(_THIS, SDL_AudioSpec *spec);
	void (*ThreadInit)(_THIS);	/* Called by audio thread at start */
	void (*WaitAudio)(_THIS);
	void (*PlayAudio)(_THIS);
	Uint8 *(*GetAudioBuf)(_THIS);
	void (*WaitDone)(_THIS);
	void (*CloseAudio)(_THIS);

	/* * * */
	/* Data common to all devices */

	/* The current audio specification (shared with audio thread) */
	SDL_AudioSpec spec;

	/* An audio conversion block for audio format emulation */
	SDL_AudioCVT convert;

	/* Current state flags */
	int enabled;
	int paused;
	int opened;

	/* Fake audio buffer for when the audio hardware is busy */
	Uint8 *fake_stream;

	/* A semaphore for locking the mixing buffers */
	SDL_mutex *mixer_lock;

	/* A thread to feed the audio device */
	SDL_Thread *thread;
	Uint32 threadid;

	/* * * */
	/* Data private to this driver */
	struct SDL_PrivateAudioData *hidden;

	/* * * */
	/* The function used to dispose of this structure */
	void (*free)(_THIS);
};
#undef _THIS

typedef struct AudioBootStrap {
	const char *name;
	const char *desc;
	int (*available)(void);
	SDL_AudioDevice *(*create)(int devindex);
} AudioBootStrap;

#ifdef OPENBSD_AUDIO_SUPPORT
extern AudioBootStrap OPENBSD_AUDIO_bootstrap;
#endif
#ifdef OSS_SUPPORT
extern AudioBootStrap DSP_bootstrap;
extern AudioBootStrap DMA_bootstrap;
#endif
#ifdef ALSA_SUPPORT
extern AudioBootStrap ALSA_bootstrap;
#endif
#if (defined(unix) && !defined(__CYGWIN32__)) && \
    !defined(OSS_SUPPORT) && !defined(ALSA_SUPPORT)
extern AudioBootStrap AUDIO_bootstrap;
#endif
#ifdef ARTSC_SUPPORT
extern AudioBootStrap ARTSC_bootstrap;
#endif
#ifdef ESD_SUPPORT
extern AudioBootStrap ESD_bootstrap;
#endif
#ifdef NAS_SUPPORT
extern AudioBootStrap NAS_bootstrap;
#endif
#ifdef ENABLE_DIRECTX
extern AudioBootStrap DSOUND_bootstrap;
#endif
#ifdef ENABLE_WINDIB
extern AudioBootStrap WAVEOUT_bootstrap;
#endif
#ifdef _AIX
extern AudioBootStrap Paud_bootstrap;
#endif
#ifdef __BEOS__
extern AudioBootStrap BAUDIO_bootstrap;
#endif
#if defined(macintosh) || TARGET_API_MAC_CARBON
extern AudioBootStrap SNDMGR_bootstrap;
#endif
#ifdef ENABLE_AHI
extern AudioBootStrap AHI_bootstrap;
#endif
#ifdef DISKAUD_SUPPORT
extern AudioBootStrap DISKAUD_bootstrap;
#endif

/* This is the current audio device */
extern SDL_AudioDevice *current_audio;

#endif /* _SDL_sysaudio_h */
