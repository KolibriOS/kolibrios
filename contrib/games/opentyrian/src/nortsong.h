/* 
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2009  The OpenTyrian Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef NORTSONG_H
#define NORTSONG_H

#include "opentyr.h"

#include "musmast.h"
#include "sndmast.h"

#include "SDL.h"

extern Uint32 target, target2;

extern JE_word frameCount, frameCount2, frameCountMax;

extern JE_byte *digiFx[SAMPLE_COUNT];
extern JE_word fxSize[SAMPLE_COUNT];

extern JE_word tyrMusicVolume, fxVolume;
extern JE_word fxPlayVol;
extern JE_word tempVolume;

extern JE_word speed;

extern float jasondelay;

void setdelay( JE_byte delay );
void setjasondelay( int delay );
void setjasondelay2( int delay );
int delaycount( void );
int delaycount2( void );

void wait_delay( void );
void service_wait_delay( void );
void wait_delayorinput( JE_boolean keyboard, JE_boolean mouse, JE_boolean joystick );

void JE_resetTimerInt( void );
void JE_setTimerInt( void );

void JE_calcFXVol( void );
void JE_changeVolume( JE_word *music, int music_delta, JE_word *sample, int sample_delta );

void JE_loadSndFile( const char *effects_sndfile, const char *voices_sndfile );
void JE_playSampleNum( JE_byte samplenum );

#endif /* NORTSONG_H */

