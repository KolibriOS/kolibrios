/*
playflic - play a FLIC file on the screen
Placed in the public domain by Andre de Leiradella on 24-fev-2003.

You'll need SDL and SDLmain to compile this program.
*/

#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_main.h>
#include "SDL_flic.h"

int main(int argc, char *argv[]) {
	SDL_RWops     *rwops;
	FLI_Animation *flic;
	int	      error;
	SDL_Surface   *screen;
	SDL_Rect      pos;
	SDL_Event     event;
	Uint32	      ticks, ticks2;

	if (argc != 2) {
		fprintf(stderr, "Usage: playflic <flicfile>\n");
		return 0;
	}
	/* Open the flic. */
	rwops = SDL_RWFromFile(argv[1], "rb");
	if (rwops == NULL) {
		fprintf(stderr, "FLIC file not found\n");
		return 0;
	}
	flic = FLI_Open(rwops, &error);
	if (error != FLI_OK)
		goto out;
	/* Init SDL. */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		goto out;
	}
	atexit(SDL_Quit);
	/* Set the video with the size of the flic. */
	screen = SDL_SetVideoMode(flic->width, flic->height, 0, SDL_SWSURFACE);
	if (screen == NULL) {
		fprintf(stderr, "Couldn't set %dx%d video mode: %s\n", flic->width, flic->height, SDL_GetError());
		goto out;
	}
	/* Play the flic. */
	pos.x = pos.y = 0;
	ticks = SDL_GetTicks();
	for (;;) {
		/* Render the next frame. */
		error = FLI_NextFrame(flic);
		if (error != FLI_OK)
			goto out;
		/* Blit it. */
		SDL_BlitSurface(flic->surface, NULL, screen, &pos);
		SDL_UpdateRect(screen, 0, 0, 0, 0);
		/* Exit program in case of any event. */
		while (SDL_PollEvent(&event) == 1) {
			switch (event.type) {
				case SDL_MOUSEBUTTONDOWN:
				case SDL_KEYDOWN:
				case SDL_QUIT:
					goto out;
			}
		}
		/* Delay between frames. */
		ticks = SDL_GetTicks() - ticks;
		if (ticks < flic->delay)
			SDL_Delay(flic->delay - ticks);
		ticks = SDL_GetTicks();
	}
	out:
	/* Describe the error. */
	switch (error) {
		case FLI_READERROR:
			fprintf(stderr, "Error while reading FLIC\n");
			break;
		case FLI_CORRUPTEDFILE:
			fprintf(stderr, "FLIC file corrupted\n");
			break;
		case FLI_SDLERROR:
			fprintf(stderr, "SDL error: %s\n", SDL_GetError());
			break;
		case FLI_OUTOFMEMORY:
			fprintf(stderr, "Out of memory\n");
			break;
	}
	FLI_Close(flic);
	return 0;
}
