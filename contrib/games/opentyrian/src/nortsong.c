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
#include "nortsong.h"

#include "file.h"
#include "joystick.h"
#include "keyboard.h"
#include "loudness.h"
#include "musmast.h"
#include "opentyr.h"
#include "params.h"
#include "sndmast.h"
#include "vga256d.h"

#include "SDL.h"

Uint32 target, target2;

JE_boolean notYetLoadedSound = true;

JE_word frameCount, frameCount2, frameCountMax;

JE_byte *digiFx[SAMPLE_COUNT] = { NULL }; /* [1..soundnum + 9] */
JE_word fxSize[SAMPLE_COUNT]; /* [1..soundnum + 9] */

JE_word tyrMusicVolume, fxVolume;
JE_word fxPlayVol;
JE_word tempVolume;

JE_word speed; /* JE: holds timer speed for 70Hz */

float jasondelay = 1000.0f / (1193180.0f / 0x4300);

void setdelay( JE_byte delay )
{
	target = (delay * 16) + SDL_GetTicks();
}

void setjasondelay( int delay )
{
	target = SDL_GetTicks() + delay * jasondelay;
}

void setjasondelay2( int delay )
{
	target2 = SDL_GetTicks() + delay * jasondelay;
}

int delaycount( void )
{
	return (SDL_GetTicks() < target ? target - SDL_GetTicks() : 0);
}

int delaycount2( void )
{
	return (SDL_GetTicks() < target2 ? target2 - SDL_GetTicks() : 0);
}

void wait_delay( void )
{
	Sint32 delay = target - SDL_GetTicks();
	if (delay > 0)
		SDL_Delay(delay);
}

void service_wait_delay( void )
{
	while (SDL_GetTicks() < target)
	{
		SDL_Delay(SDL_GetTicks() - target > SDL_POLL_INTERVAL ? SDL_POLL_INTERVAL : SDL_GetTicks() - target);
		service_SDL_events(false);
	}
}

void wait_delayorinput( JE_boolean keyboard, JE_boolean mouse, JE_boolean joystick )
{
	service_SDL_events(true);
	while (SDL_GetTicks() < target && !((keyboard && keydown) || (mouse && mousedown) || (joystick && joydown)))
	{
		SDL_Delay(SDL_GetTicks() - target > SDL_POLL_INTERVAL ? SDL_POLL_INTERVAL : SDL_GetTicks() - target);
		push_joysticks_as_keyboard();
		service_SDL_events(false);
	}
}

void JE_loadSndFile( const char *effects_sndfile, const char *voices_sndfile )
{
	JE_byte y, z;
	JE_word x;
	JE_longint templ;
	JE_longint sndPos[2][SAMPLE_COUNT + 1];
	JE_word sndNum;

	FILE *fi;
	
	/* SYN: Loading offsets into TYRIAN.SND */
	fi = dir_fopen_die(data_dir(), effects_sndfile, "rb");
	efread(&sndNum, sizeof(sndNum), 1, fi);

	for (x = 0; x < sndNum; x++)
	{
		efread(&sndPos[0][x], sizeof(sndPos[0][x]), 1, fi);
	}
	fseek(fi, 0, SEEK_END);
	sndPos[0][sndNum] = ftell(fi); /* Store file size */

	for (z = 0; z < sndNum; z++)
	{
		fseek(fi, sndPos[0][z], SEEK_SET);
		fxSize[z] = (sndPos[0][z+1] - sndPos[0][z]); /* Store sample sizes */
		free(digiFx[z]);
		digiFx[z] = malloc(fxSize[z]);
		efread(digiFx[z], 1, fxSize[z], fi); /* JE: Load sample to buffer */
	}

	fclose(fi);

	/* SYN: Loading offsets into VOICES.SND */
	fi = dir_fopen_die(data_dir(), voices_sndfile, "rb");
	
	efread(&sndNum, sizeof(sndNum), 1, fi);

	for (x = 0; x < sndNum; x++)
	{
		efread(&sndPos[1][x], sizeof(sndPos[1][x]), 1, fi);
	}
	fseek(fi, 0, SEEK_END);
	sndPos[1][sndNum] = ftell(fi); /* Store file size */

	z = SAMPLE_COUNT - 9;

	for (y = 0; y < sndNum; y++)
	{
		fseek(fi, sndPos[1][y], SEEK_SET);

		templ = (sndPos[1][y+1] - sndPos[1][y]) - 100; /* SYN: I'm not entirely sure what's going on here. */
		if (templ < 1) templ = 1;
		fxSize[z + y] = templ; /* Store sample sizes */
		digiFx[z + y] = malloc(fxSize[z + y]);
		efread(digiFx[z + y], 1, fxSize[z + y], fi); /* JE: Load sample to buffer */
	}

	fclose(fi);

	notYetLoadedSound = false;

}

void JE_playSampleNum( JE_byte samplenum )
{
	JE_multiSamplePlay(digiFx[samplenum-1], fxSize[samplenum-1], 0, fxPlayVol);
}

void JE_calcFXVol( void ) // TODO: not sure *exactly* what this does
{
	fxPlayVol = (fxVolume - 1) >> 5;
}

void JE_setTimerInt( void )
{
	jasondelay = 1000.0f / (1193180.0f / speed);
}

void JE_resetTimerInt( void )
{
	jasondelay = 1000.0f / (1193180.0f / 0x4300);
}

void JE_changeVolume( JE_word *music, int music_delta, JE_word *sample, int sample_delta )
{
	int music_temp = *music + music_delta,
	    sample_temp = *sample + sample_delta;
	
	if (music_delta)
	{
		if (music_temp > 255)
		{
			music_temp = 255;
			JE_playSampleNum(S_CLINK);
		}
		else if (music_temp < 0)
		{
			music_temp = 0;
			JE_playSampleNum(S_CLINK);
		}
	}
	
	if (sample_delta)
	{
		if (sample_temp > 255)
		{
			sample_temp = 255;
			JE_playSampleNum(S_CLINK);
		}
		else if (sample_temp < 0)
		{
			sample_temp = 0;
			JE_playSampleNum(S_CLINK);
		}
	}
	
	*music = music_temp;
	*sample = sample_temp;
	
	JE_calcFXVol();
	
	set_volume(*music, *sample);
}

