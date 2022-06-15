// A little utility to convert SMDs to BINs

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#ifndef __MINGW32__
#include <signal.h>
#endif
#include "romload.h"

#undef main /* -Dmain=SDL_main */
int main(int argc, char *argv[])
{
	size_t size;
	FILE *out;
	uint8_t *rom;

#ifndef __MINGW32__
	signal(SIGPIPE, SIG_IGN);
#endif
	if (argc != 3) {
		fprintf(stderr, "Usage: %s {from.smd} {to.bin}\n", argv[0]);
		return EXIT_FAILURE;
	}
	rom = load_rom(&size, argv[1]);
	if (rom == NULL) {
		fprintf(stderr, "%s: `%s': Unable to load ROM\n", argv[0],
			argv[1]);
		return EXIT_FAILURE;
	}
	out = fopen(argv[2], "wb");
	if (out == NULL) {
		fprintf(stderr, "%s: `%s': %s\n", argv[0], argv[2],
			strerror(errno));
		unload_rom(rom);
		return EXIT_FAILURE;
	}
	size = fwrite(rom, size, 1, out);
	fclose(out);
	unload_rom(rom);
	if (size == 1)
		return EXIT_SUCCESS;
	fprintf(stderr, "%s: `%s': %s\n", argv[0], argv[2], strerror(errno));
	return EXIT_FAILURE;
}
