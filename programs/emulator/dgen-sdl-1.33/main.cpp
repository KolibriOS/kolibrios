// DGen/SDL 1.17
// by Joe Groff <joe@pknet.com>
// Read LICENSE for copyright etc., but if you've seen one BSDish license,
// you've seen them all ;)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <errno.h>

#ifdef __MINGW32__
#include <windows.h>
#include <wincon.h>
#endif

#define IS_MAIN_CPP
#include "system.h"
#include "md.h"
#include "pd.h"
#include "pd-defs.h"
#include "rc.h"
#include "rc-vars.h"

#ifdef __BEOS__
#include <OS.h>
#endif

#ifdef __MINGW32__
static long dgen_mingw_detach = 1;
#endif

// Defined in ras.cpp, and set to true if the Genesis palette's changed.
extern int pal_dirty;

FILE *debug_log = NULL;

// Do a demo frame, if active
enum demo_status {
	DEMO_OFF,
	DEMO_RECORD,
	DEMO_PLAY
};

static inline void do_demo(md& megad, FILE* demo, enum demo_status* status)
{
	uint32_t pad[2];

	switch (*status) {
	case DEMO_OFF:
		break;
	case DEMO_RECORD:
		pad[0] = h2be32(megad.pad[0]);
		pad[1] = h2be32(megad.pad[1]);
		fwrite(&pad, sizeof(pad), 1, demo);
		break;
	case DEMO_PLAY:
		if (fread(&pad, sizeof(pad), 1, demo) == 1) {
			megad.pad[0] = be2h32(pad[0]);
			megad.pad[1] = be2h32(pad[1]);
		}
		else {
			if (feof(demo))
				pd_message("Demo finished.");
			else
				pd_message("Demo finished (read error).");
			*status = DEMO_OFF;
		}
		break;
	}
}

// Temporary garbage can string :)
static char temp[65536] = "";

// Show help and exit with code 2
static void help()
{
  printf(
  "DGen/SDL v"VER"\n"
  "Usage: dgen [options] [romname [...]]\n\n"
  "Where options are:\n"
  "    -v              Print version number and exit.\n"
  "    -r RCFILE       Read in the file RCFILE after parsing\n"
  "                    $HOME/.dgen/dgenrc.\n"
  "    -n USEC         Causes DGen to sleep USEC microseconds per frame, to\n"
  "                    be nice to other processes.\n"
  "    -p CODE,CODE... Takes a comma-delimited list of Game Genie (ABCD-EFGH)\n"
  "                    or Hex (123456:ABCD) codes to patch the ROM with.\n"
  "    -R (J|X|U|E| )  Force emulator region. Affects vertical resolution,\n"
  "                    frame rate and ROM operation.\n"
  "                    J: Japan (NTSC), X: Japan (PAL), U: America (NTSC)\n"
  "                    E: Europe (PAL), ' ': auto (default).\n"
  "    -N              Use NTSC mode (60Hz).\n"
  "    -P              Use PAL mode (50Hz).\n"
  "    -H HZ           Use a custom frame rate.\n"
  "    -d DEMONAME     Record a demo of the game you are playing.\n"
  "    -D DEMONAME     Play back a previously recorded demo.\n"
  "    -s SLOT         Load the saved state from the given slot at startup.\n"
#ifdef __MINGW32__
  "    -m              Do not detach from console.\n"
#endif
  );
  // Display platform-specific options
  pd_help();
  exit(2);
}

// Save/load states
// It is externed from your implementation to change the current slot
// (I know this is a hack :)
int slot = 0;
void md_save(md& megad)
{
	FILE *save;
	char file[64];

	if (!megad.plugged) {
		pd_message("Cannot save state when no ROM is loaded.");
		return;
	}
	if (((size_t)snprintf(file,
			      sizeof(file),
			      "%s.gs%d",
			      megad.romname,
			      slot) >= sizeof(file)) ||
	    ((save = dgen_fopen("saves", file, DGEN_WRITE)) == NULL)) {
		snprintf(temp, sizeof(temp),
			 "Couldn't save state to slot %d!", slot);
		pd_message(temp);
		return;
	}
	megad.export_gst(save);
	fclose(save);
	snprintf(temp, sizeof(temp), "Saved state to slot %d.", slot);
	pd_message(temp);
}

void md_load(md& megad)
{
	FILE *load;
	char file[64];

	if (!megad.plugged) {
		pd_message("Cannot restore state when no ROM is loaded.");
		return;
	}
	if (((size_t)snprintf(file,
			      sizeof(file),
			      "%s.gs%d",
			      megad.romname,
			      slot) >= sizeof(file)) ||
	    ((load = dgen_fopen("saves", file, DGEN_READ)) == NULL)) {
		snprintf(temp, sizeof(temp),
			 "Couldn't load state from slot %d!", slot);
		pd_message(temp);
		return;
	}
	megad.import_gst(load);
	fclose(load);
	snprintf(temp, sizeof(temp), "Loaded state from slot %d.", slot);
	pd_message(temp);
}

// Load/save states from file
void ram_save(md& megad)
{
	FILE *save;
	int ret;

	if (!megad.has_save_ram())
		return;
	save = dgen_fopen("ram", megad.romname, DGEN_WRITE);
	if (save == NULL)
		goto fail;
	ret = megad.put_save_ram(save);
	fclose(save);
	if (ret == 0)
		return;
fail:
	fprintf(stderr, "Couldn't save battery RAM to `%s'\n", megad.romname);
}

void ram_load(md& megad)
{
	FILE *load;
	int ret;

	if (!megad.has_save_ram())
		return;
	load = dgen_fopen("ram", megad.romname, DGEN_READ);
	if (load == NULL)
		goto fail;
	ret = megad.get_save_ram(load);
	fclose(load);
	if (ret == 0)
		return;
fail:
	fprintf(stderr, "Couldn't load battery RAM from `%s'\n",
		megad.romname);
}

int main(int argc, char *argv[])
{
  int c = 0, stop = 0, usec = 0, start_slot = -1;
  unsigned long frames, frames_old, fps;
  char *patches = NULL, *rom = NULL;
  unsigned long oldclk, newclk, startclk, fpsclk;
  FILE *file = NULL;
  enum demo_status demo_status = DEMO_OFF;
  unsigned int samples;
  class md *megad;
  bool first = true;
  bool forced_hz = false;
  bool forced_pal = false;

	// Parse the RC file
	if ((dgen_autoconf) &&
	    ((file = dgen_fopen_autorc(DGEN_READ)) != NULL)) {
		parse_rc(file, DGEN_AUTORC);
		fclose(file);
	}
	if ((file = dgen_fopen_rc(DGEN_READ)) != NULL) {
		parse_rc(file, DGEN_RC);
		fclose(file);
		file = NULL;
		pd_rc();
	}
	else if (errno == ENOENT) {
		if ((file = dgen_fopen_rc(DGEN_APPEND)) != NULL) {
			fprintf(file,
				"# DGen " VER " configuration file.\n"
				"# See dgenrc(5) for more information.\n");
			fclose(file);
			file = NULL;
		}
	}
	else
		fprintf(stderr, "rc: %s: %s\n", DGEN_RC, strerror(errno));

  // Check all our options
  snprintf(temp, sizeof(temp), "%s%s",
#ifdef __MINGW32__
	   "m"
#endif
	   "s:hvr:n:p:R:NPH:d:D:",
	   pd_options);
  while((c = getopt(argc, argv, temp)) != EOF)
    {
      switch(c)
	{
	case 'v':
	  // Show version and exit
	  printf("DGen/SDL version "VER"\n");
	  return 0;
	case 'r':
	  // Parse another RC file or stdin
	  if ((strcmp(optarg, "-") == 0) ||
	      ((file = dgen_fopen(NULL, optarg,
				  (DGEN_READ | DGEN_CURRENT))) != NULL)) {
	    if (file == NULL)
	      parse_rc(stdin, "(stdin)");
	    else {
	      parse_rc(file, optarg);
	      fclose(file);
	      file = NULL;
	    }
	    pd_rc();
	  }
	  else
	    fprintf(stderr, "rc: %s: %s\n", optarg, strerror(errno));
	  break;
	case 'n':
	  // Sleep for n microseconds
	  dgen_nice = atoi(optarg);
	  break;
	case 'p':
	  // Game Genie patches
	  patches = optarg;
	  break;
	case 'R':
		// Region selection
		if (strlen(optarg) != 1)
			goto bad_region;
		switch (optarg[0] | 0x20) {
		case 'j':
		case 'x':
		case 'u':
		case 'e':
		case ' ':
			break;
		default:
		bad_region:
			fprintf(stderr, "main: invalid region `%s'.\n",
				optarg);
			return EXIT_FAILURE;
		}
		dgen_region = (optarg[0] & ~(0x20));
		// Override PAL and Hz settings if region is specified.
		if (dgen_region) {
			int hz;
			int pal;

			md::region_info(dgen_region, &pal, &hz, 0, 0, 0);
			dgen_hz = hz;
			dgen_pal = pal;
		}
		forced_pal = false;
		forced_hz = false;
		break;
	case 'N':
		// NTSC mode
		dgen_hz = NTSC_HZ;
		dgen_pal = 0;
		forced_pal = true;
		break;
	case 'P':
		// PAL mode
		dgen_hz = PAL_HZ;
		dgen_pal = 1;
		forced_pal = true;
		break;
	case 'H':
		// Custom frame rate
		dgen_hz = atoi(optarg);
		if ((dgen_hz <= 0) || (dgen_hz > 1000)) {
			fprintf(stderr, "main: invalid frame rate (%ld).\n",
				(long)dgen_hz);
			dgen_hz = (dgen_pal ? 50 : 60);
			forced_hz = false;
		}
		else
			forced_hz = true;
		break;
#ifdef __MINGW32__
	case 'm':
		dgen_mingw_detach = 0;
		break;
#endif
	case 'd':
	  // Record demo
	  if(file)
	    {
	      fprintf(stderr,"main: Can't record and play at the same time!\n");
	      break;
	    }
	  if(!(file = dgen_fopen("demos", optarg, DGEN_WRITE)))
	    {
	      fprintf(stderr, "main: Can't record demo file %s!\n", optarg);
	      break;
	    }
	  demo_status = DEMO_RECORD;
	  break;
	case 'D':
	  // Play demo
	  if(file)
	    {
	      fprintf(stderr,"main: Can't record and play at the same time!\n");
	      break;
	    }
	  if(!(file = dgen_fopen("demos", optarg, (DGEN_READ | DGEN_CURRENT))))
	    {
	      fprintf(stderr, "main: Can't play demo file %s!\n", optarg);
	      break;
	    }
	  demo_status = DEMO_PLAY;
	  break;
	case '?': // Bad option!
	case 'h': // A cry for help :)
	  help();
        case 's':
          // Pick a savestate to autoload
          start_slot = atoi(optarg);
          break;
	default:
	  // Pass it on to platform-dependent stuff
	  pd_option(c, optarg);
	  break;
	}
    }

#ifdef __BEOS__
  // BeOS snooze() sleeps in milliseconds, not microseconds
  dgen_nice /= 1000;
#endif

#ifdef __MINGW32__
	if (dgen_mingw_detach) {
		FILE *cons;

		fprintf(stderr,
			"main: Detaching from console, use -m to prevent"
			" this.\n");
		// Console isn't needed anymore. Redirect output to log file.
		cons = dgen_fopen(NULL, "log.txt", (DGEN_WRITE | DGEN_TEXT));
		if (cons != NULL) {
			fflush(stdout);
			fflush(stderr);
			dup2(fileno(cons), fileno(stdout));
			dup2(fileno(cons), fileno(stderr));
			fclose(cons);
			setvbuf(stdout, NULL, _IONBF, 0);
			setvbuf(stderr, NULL, _IONBF, 0);
			cons = NULL;
		}
		FreeConsole();
	}
#endif

  // Initialize the platform-dependent stuff.
  if (!pd_graphics_init(dgen_sound, dgen_pal, dgen_hz))
    {
      fprintf(stderr, "main: Couldn't initialize graphics!\n");
      return 1;
    }
  if(dgen_sound)
    {
      long rate = dgen_soundrate;

      if (dgen_soundsegs < 0)
	      dgen_soundsegs = 0;
      samples = (dgen_soundsegs * (rate / dgen_hz));
      pd_sound_init(rate, samples);
    }

	rom = argv[optind];
	// Create the megadrive object.
	megad = new md(dgen_pal, dgen_region);
	if ((megad == NULL) || (!megad->okay())) {
		fprintf(stderr, "main: Mega Drive initialization failed.\n");
		goto clean_up;
	}
next_rom:
	// Load the requested ROM.
	if (rom != NULL) {
		if (megad->load(rom)) {
			pd_message("Unable to load \"%s\".", rom);
			if ((first) && ((optind + 1) == argc))
				goto clean_up;
		}
		else
			pd_message("Loaded \"%s\".", rom);
	}
	else
		pd_message("No cartridge.");
	first = false;
	// Set untouched pads.
	megad->pad[0] = MD_PAD_UNTOUCHED;
	megad->pad[1] = MD_PAD_UNTOUCHED;
#ifdef WITH_JOYSTICK
	if (dgen_joystick)
		megad->init_joysticks();
#endif
	// Load patches, if given.
	if (patches) {
		printf("main: Using patch codes \"%s\".\n", patches);
		megad->patch(patches, NULL, NULL, NULL);
		// Use them only once.
		patches = NULL;
	}
	// Reset
	megad->reset();

	// Automatic region settings from ROM header.
	if (!dgen_region) {
		uint8_t c = megad->region_guess();
		int hz;
		int pal;

		md::region_info(c, &pal, &hz, 0, 0, 0);
		if (forced_hz)
			hz = dgen_hz;
		if (forced_pal)
			pal = dgen_pal;
		if ((hz != dgen_hz) || (pal != dgen_pal) ||
		    (c != megad->region)) {
			megad->region = c;
			dgen_hz = hz;
			dgen_pal = pal;
			printf("main: reconfiguring for region \"%c\": "
			       "%dHz (%s)\n", c, hz, (pal ? "PAL" : "NTSC"));
			pd_graphics_reinit(dgen_sound, dgen_pal, dgen_hz);
			if (dgen_sound) {
				long rate = dgen_soundrate;

				pd_sound_deinit();
				samples = (dgen_soundsegs * (rate / dgen_hz));
				pd_sound_init(rate, samples);
			}
			megad->pal = pal;
			megad->init_pal();
			megad->init_sound();
		}
	}

	// Load up save RAM
	ram_load(*megad);
	// If -s option was given, load the requested slot
	if (start_slot >= 0) {
		slot = start_slot;
		md_load(*megad);
	}
	// If autoload is on, load save state 0
	else if (dgen_autoload) {
		slot = 0;
		md_load(*megad);
	}

	// Start the timing refs
	startclk = pd_usecs();
	oldclk = startclk;
	fpsclk = startclk;

	// Show cartridge header
	if (dgen_show_carthead)
		pd_show_carthead(*megad);

	// Go around, and around, and around, and around... ;)
	frames = 0;
	frames_old = 0;
	fps = 0;
	while (!stop) {
		const unsigned int usec_frame = (1000000 / dgen_hz);
		unsigned long tmp;
		int frames_todo;

		newclk = pd_usecs();

		if (pd_stopped()) {
			// Fix FPS count.
			tmp = (newclk - oldclk);
			startclk += tmp;
			fpsclk += tmp;
			oldclk = newclk;
		}

		// Update FPS count.
		tmp = ((newclk - fpsclk) & 0x3fffff);
		if (tmp >= 1000000) {
			fpsclk = newclk;
			if (frames_old > frames)
				fps = (frames_old - frames);
			else
				fps = (frames - frames_old);
			frames_old = frames;
		}

		if (dgen_frameskip == 0) {
			// Check whether megad->one_frame() must be called.
			if (pd_freeze)
				goto frozen;
			goto do_not_skip;
		}

		// Measure how many frames to do this round.
		usec += ((newclk - oldclk) & 0x3fffff); // no more than 4 secs
		frames_todo = (usec / usec_frame);
		usec %= usec_frame;
		oldclk = newclk;

		if (frames_todo == 0) {
			// No frame to do yet, relax the CPU until next one.
			tmp = (usec_frame - usec);
			if (tmp > 1000) {
				// Never sleep for longer than the 50Hz value
				// so events are checked often enough.
				if (tmp > (1000000 / 50))
					tmp = (1000000 / 50);
				tmp -= 1000;
#ifdef __BEOS__
				snooze(tmp / 1000);
#else
				usleep(tmp);
#endif
			}
		}
		else {
			// Check whether megad->one_frame() must be called.
			if (pd_freeze)
				goto frozen;

			// Draw frames.
			while (frames_todo != 1) {
				do_demo(*megad, file, &demo_status);
				if (dgen_sound) {
					// Skip this frame, keep sound going.
					megad->one_frame(NULL, NULL, &sndi);
					pd_sound_write();
				}
				else
					megad->one_frame(NULL, NULL, NULL);
				--frames_todo;
				stop |= (pd_handle_events(*megad) ^ 1);
			}
			--frames_todo;
		do_not_skip:
			do_demo(*megad, file, &demo_status);
			if (dgen_sound) {
				megad->one_frame(&mdscr, mdpal, &sndi);
				pd_sound_write();
			}
			else
				megad->one_frame(&mdscr, mdpal, NULL);
		frozen:
			if ((mdpal) && (pal_dirty)) {
				pd_graphics_palette_update();
				pal_dirty = 0;
			}
			pd_graphics_update(megad->plugged);
			++frames;
		}

		stop |= (pd_handle_events(*megad) ^ 1);

		if (dgen_nice) {
#ifdef __BEOS__
			snooze(dgen_nice);
#else
			usleep(dgen_nice);
#endif
		}
	}

#ifdef WITH_JOYSTICK
	if (dgen_joystick)
		megad->deinit_joysticks();
#endif

	// Print fps
	fpsclk = ((pd_usecs() - startclk) / 1000000);
	if (fpsclk == 0)
		fpsclk = 1;
#ifdef WITH_DEBUGGER
	megad->debug_leave();
#endif
	printf("%lu frames per second (average %lu, optimal %ld)\n",
	       fps, (frames / fpsclk), (long)dgen_hz);

	ram_save(*megad);
	if (dgen_autosave) {
		slot = 0;
		md_save(*megad);
	}
	megad->unplug();
	if (file) {
		fclose(file);
		file = NULL;
	}
	if ((++optind) < argc) {
		rom = argv[optind];
		stop = 0;
		goto next_rom;
	}
clean_up:
	// Cleanup
	delete megad;
	pd_sound_deinit();
	pd_quit();
	// Save configuration.
	if (dgen_autoconf) {
		if ((file = dgen_fopen_autorc(DGEN_WRITE)) == NULL)
			fputs("main: can't write " DGEN_AUTORC ".\n", stderr);
		else {
			fprintf(file,
				"# DGen/SDL v" VER "\n"
				"# This file is automatically overwritten.\n"
				"\n");
			dump_rc(file);
			fclose(file);
			file = NULL;
		}
	}
	// Come back anytime :)
	return 0;
}
