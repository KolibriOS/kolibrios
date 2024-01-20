/**
 * SDL interface
 */

#ifdef __MINGW32__
#undef __STRICT_ANSI__
#endif

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <SDL.h>
#include <SDL_audio.h>

#ifdef WITH_OPENGL
# include <SDL_opengl.h>
#endif

#ifdef WITH_THREADS
#include <SDL_thread.h>
#endif

#ifdef HAVE_MEMCPY_H
#include "memcpy.h"
#endif
#include "md.h"
#include "rc.h"
#include "rc-vars.h"
#include "pd.h"
#include "pd-defs.h"
#include "font.h"
#include "system.h"
#include "prompt.h"
#include "romload.h"
#include "splash.h"

#ifdef WITH_HQX
#define HQX_NO_UINT24
#include "hqx.h"
#endif

#ifdef WITH_SCALE2X
extern "C" {
#include "scalebit.h"
}
#endif

#ifdef _KOLIBRI
#include <sys/ksys.h>
#endif

/// Number of microseconds to sustain messages
#define MESSAGE_LIFE 3000000

static void pd_message_process(void);
static size_t pd_message_write(const char *msg, size_t len, unsigned int mark);
static size_t pd_message_display(const char *msg, size_t len,
				 unsigned int mark, bool update);
static void pd_message_postpone(const char *msg);
static void pd_message_cursor(unsigned int mark, const char *msg, ...);

/// Generic type for supported colour depths.
typedef union {
	uint8_t *u8;
	uint32_t *u32;
	uint24_t *u24;
	uint16_t *u16;
	uint16_t *u15;
} bpp_t;

#ifdef WITH_OPENGL

/// Framebuffer texture.
struct texture {
	unsigned int width; ///< texture width
	unsigned int height; ///< texture height
	unsigned int vis_width; ///< visible width
	unsigned int vis_height; ///< visible height
	GLuint id; ///< texture identifier
	GLuint dlist; ///< display list
	unsigned int u32:1; ///< texture is 32-bit
	unsigned int linear:1; ///< linear filtering is enabled
	union {
		uint16_t *u16;
		uint32_t *u32;
	} buf; ///< 16 or 32-bit buffer
};

static void release_texture(struct texture&);
static int init_texture(struct screen *);
static void update_texture(struct texture&);

#endif // WITH_OPENGL

struct screen {
	unsigned int window_width; ///< window width
	unsigned int window_height; ///< window height
	unsigned int width; ///< buffer width
	unsigned int height; ///< buffer height
	unsigned int bpp; ///< bits per pixel
	unsigned int Bpp; ///< bytes per pixel
	unsigned int x_scale; ///< horizontal scale factor
	unsigned int y_scale; ///< vertical scale factor
	unsigned int info_height; ///< message bar height (included in height)
	bpp_t buf; ///< generic pointer to pixel data
	unsigned int pitch; ///< number of bytes per line in buf
	SDL_Surface *surface; ///< SDL surface
	unsigned int want_fullscreen:1; ///< want fullscreen
	unsigned int is_fullscreen:1; ///< fullscreen enabled
#ifdef WITH_OPENGL
	struct texture texture; ///< OpenGL texture data
	unsigned int want_opengl:1; ///< want OpenGL
	unsigned int is_opengl:1; ///< OpenGL enabled
#endif
#ifdef WITH_THREADS
	unsigned int want_thread:1; ///< want updates from a separate thread
	unsigned int is_thread:1; ///< thread is present
	SDL_Thread *thread; ///< thread itself
	SDL_mutex *lock; ///< lock for updates
	SDL_cond *cond; ///< condition variable to signal updates
#endif
	SDL_Color color[64]; ///< SDL colors for 8bpp modes
};

static struct screen screen;

static struct {
	const unsigned int width; ///< 320
	unsigned int height; ///< 224 or 240 (NTSC_VBLANK or PAL_VBLANK)
	unsigned int hz; ///< refresh rate
	unsigned int is_pal: 1; ///< PAL enabled
	uint8_t palette[256]; ///< palette for 8bpp modes (mdpal)
} video = {
	320, ///< width is always 320
	NTSC_VBLANK, ///< NTSC height by default
	NTSC_HZ, ///< 60Hz
	0, ///< NTSC is enabled
	{ 0 }
};

/**
 * Call this before accessing screen.buf.
 * No syscalls allowed before screen_unlock().
 */
static int screen_lock()
{
#ifdef WITH_THREADS
	if (screen.is_thread) {
		assert(screen.lock != NULL);
		SDL_LockMutex(screen.lock);
	}
#endif
#ifdef WITH_OPENGL
	if (screen.is_opengl)
		return 0;
#endif
	if (SDL_MUSTLOCK(screen.surface) == 0)
		return 0;
	return SDL_LockSurface(screen.surface);
}

/**
 * Call this after accessing screen.buf.
 */
static void screen_unlock()
{
#ifdef WITH_THREADS
	if (screen.is_thread) {
		assert(screen.lock != NULL);
		SDL_UnlockMutex(screen.lock);
	}
#endif
#ifdef WITH_OPENGL
	if (screen.is_opengl)
		return;
#endif
	if (SDL_MUSTLOCK(screen.surface) == 0)
		return;
	SDL_UnlockSurface(screen.surface);
}

/**
 * Do not call this directly, use screen_update() instead.
 */
static void screen_update_once()
{
#ifdef WITH_OPENGL
	if (screen.is_opengl) {
		update_texture(screen.texture);
		return;
	}
#endif
	SDL_Flip(screen.surface);
}

#ifdef WITH_THREADS

static int screen_update_thread(void *)
{
	assert(screen.lock != NULL);
	assert(screen.cond != NULL);
	SDL_LockMutex(screen.lock);
	while (screen.want_thread) {
		SDL_CondWait(screen.cond, screen.lock);
		screen_update_once();
	}
	SDL_UnlockMutex(screen.lock);
	return 0;
}

static void screen_update_thread_start()
{
	DEBUG(("starting thread..."));
	assert(screen.want_thread);
	assert(screen.lock == NULL);
	assert(screen.cond == NULL);
	assert(screen.thread == NULL);
#ifdef WITH_OPENGL
	if (screen.is_opengl) {
		DEBUG(("this is not supported when OpenGL is enabled"));
		return;
	}
#endif
	if ((screen.lock = SDL_CreateMutex()) == NULL) {
		DEBUG(("unable to create lock"));
		goto error;
	}

	if ((screen.cond = SDL_CreateCond()) == NULL) {
		DEBUG(("unable to create condition variable"));
		goto error;
	}
	screen.thread = SDL_CreateThread(screen_update_thread, NULL);
	if (screen.thread == NULL) {
		DEBUG(("unable to start thread"));
		goto error;
	}
	screen.is_thread = 1;
	DEBUG(("thread started"));
	return;
error:
	if (screen.cond != NULL) {
		SDL_DestroyCond(screen.cond);
		screen.cond = NULL;
	}
	if (screen.lock != NULL) {
		SDL_DestroyMutex(screen.lock);
		screen.lock = NULL;
	}
}

static void screen_update_thread_stop()
{
	if (!screen.is_thread) {
		assert(screen.thread == NULL);
		return;
	}
	DEBUG(("stopping thread..."));
	assert(screen.thread != NULL);
	screen.want_thread = 0;
	SDL_CondSignal(screen.cond);
	SDL_WaitThread(screen.thread, NULL);
	screen.thread = NULL;
	SDL_DestroyCond(screen.cond);
	screen.cond = NULL;
	SDL_DestroyMutex(screen.lock);
	screen.lock = NULL;
	screen.is_thread = 0;
	DEBUG(("thread stopped"));
}

#endif // WITH_THREADS

/**
 * Call this after writing into screen.buf.
 */
static void screen_update()
{
#ifdef WITH_THREADS
	if (screen.is_thread)
		SDL_CondSignal(screen.cond);
	else
#endif // WITH_THREADS
		screen_update_once();
}

/**
 * Clear screen.
 */
static void screen_clear()
{
	if ((screen.buf.u8 == NULL) || (screen_lock()))
		return;
	memset(screen.buf.u8, 0, (screen.pitch * screen.height));
	screen_unlock();
}

// Bad hack- extern slot etc. from main.cpp so we can save/load states
extern int slot;
void md_save(md &megad);
void md_load(md &megad);

// Define externed variables
struct bmap mdscr;
unsigned char *mdpal = NULL;
struct sndinfo sndi;
const char *pd_options =
#ifdef WITH_OPENGL
	"g:"
#endif
	"fX:Y:S:G:";

static void mdscr_splash();

/// Circular buffer and related functions.
typedef struct {
	size_t i; ///< data start index
	size_t s; ///< data size
	size_t size; ///< buffer size
	union {
		uint8_t *u8;
		int16_t *i16;
	} data; ///< storage
} cbuf_t;

/**
 * Write/copy data into a circular buffer.
 * @param[in,out] cbuf Destination buffer.
 * @param[in] src Buffer to copy from.
 * @param size Size of src.
 * @return Number of bytes copied.
 */
size_t cbuf_write(cbuf_t *cbuf, uint8_t *src, size_t size)
{
	size_t j;
	size_t k;

	if (size > cbuf->size) {
		src += (size - cbuf->size);
		size = cbuf->size;
	}
	k = (cbuf->size - cbuf->s);
	j = ((cbuf->i + cbuf->s) % cbuf->size);
	if (size > k) {
		cbuf->i = ((cbuf->i + (size - k)) % cbuf->size);
		cbuf->s = cbuf->size;
	}
	else
		cbuf->s += size;
	k = (cbuf->size - j);
	if (k >= size) {
		memcpy(&cbuf->data.u8[j], src, size);
	}
	else {
		memcpy(&cbuf->data.u8[j], src, k);
		memcpy(&cbuf->data.u8[0], &src[k], (size - k));
	}
	return size;
}

/**
 * Read bytes out of a circular buffer.
 * @param[out] dst Destination buffer.
 * @param[in,out] cbuf Circular buffer to read from.
 * @param size Maximum number of bytes to copy to dst.
 * @return Number of bytes copied.
 */
size_t cbuf_read(uint8_t *dst, cbuf_t *cbuf, size_t size)
{
	if (size > cbuf->s)
		size = cbuf->s;
	if ((cbuf->i + size) > cbuf->size) {
		size_t k = (cbuf->size - cbuf->i);

		memcpy(&dst[0], &cbuf->data.u8[(cbuf->i)], k);
		memcpy(&dst[k], &cbuf->data.u8[0], (size - k));
	}
	else
		memcpy(&dst[0], &cbuf->data.u8[(cbuf->i)], size);
	cbuf->i = ((cbuf->i + size) % cbuf->size);
	cbuf->s -= size;
	return size;
}

/// Sound
static struct {
	unsigned int rate; ///< samples rate
	unsigned int samples; ///< number of samples required by the callback
	cbuf_t cbuf; ///< circular buffer
} sound;

/// Messages
static struct {
	unsigned int displayed:1; ///< whether message is currently displayed
	unsigned long since; ///< since this number of microseconds
	size_t length; ///< remaining length to display
	char message[2048]; ///< message
} info;

/// Prompt
static struct {
	struct prompt status; ///< prompt status
	char** complete; ///< completion results array
	unsigned int skip; ///< number of entries to skip in the array
	unsigned int common; ///< common length of all entries
} prompt;

/// Prompt return values
#define PROMPT_RET_CONT 0x01 ///< waiting for more input
#define PROMPT_RET_EXIT 0x02 ///< leave prompt normally
#define PROMPT_RET_ERROR 0x04 ///< leave prompt with error
#define PROMPT_RET_ENTER 0x10 ///< previous line entered
#define PROMPT_RET_MSG 0x80 ///< pd_message() has been used

struct prompt_command {
	const char* name;
	/// command function pointer
        int (*cmd)(class md&, unsigned int, const char**);
	/// completion function shoud complete the last entry in the array
	char* (*cmpl)(class md&, unsigned int, const char**, unsigned int);
};

// Extra commands usable from prompt.
static int prompt_cmd_exit(class md&, unsigned int, const char**);
static int prompt_cmd_load(class md&, unsigned int, const char**);
static char* prompt_cmpl_load(class md&, unsigned int, const char**,
			      unsigned int);
static int prompt_cmd_unload(class md&, unsigned int, const char**);
static int prompt_cmd_reset(class md&, unsigned int, const char**);
static int prompt_cmd_unbind(class md&, unsigned int, const char**);
static char* prompt_cmpl_unbind(class md&, unsigned int, const char**,
				unsigned int);
#ifdef WITH_CTV
static int prompt_cmd_filter_push(class md&, unsigned int, const char**);
static char* prompt_cmpl_filter_push(class md&, unsigned int, const char**,
				     unsigned int);
static int prompt_cmd_filter_pop(class md&, unsigned int, const char**);
static int prompt_cmd_filter_none(class md&, unsigned int, const char**);
#endif
static int prompt_cmd_calibrate(class md&, unsigned int, const char**);
static int prompt_cmd_config_load(class md&, unsigned int, const char**);
static int prompt_cmd_config_save(class md&, unsigned int, const char**);
static char* prompt_cmpl_config_file(class md&, unsigned int, const char**,
				     unsigned int);
#ifdef WITH_VGMDUMP
static char* prompt_cmpl_vgmdump(class md&, unsigned int, const char**,
				 unsigned int);
static int prompt_cmd_vgmdump(class md&, unsigned int, const char**);
#endif

/**
 * List of commands to auto complete.
 */
static const struct prompt_command prompt_command[] = {
	{ "quit", prompt_cmd_exit, NULL },
	{ "q", prompt_cmd_exit, NULL },
	{ "exit", prompt_cmd_exit, NULL },
	{ "load", prompt_cmd_load, prompt_cmpl_load },
	{ "open", prompt_cmd_load, prompt_cmpl_load },
	{ "o", prompt_cmd_load, prompt_cmpl_load },
	{ "plug", prompt_cmd_load, prompt_cmpl_load },
	{ "unload", prompt_cmd_unload, NULL },
	{ "close", prompt_cmd_unload, NULL },
	{ "unplug", prompt_cmd_unload, NULL },
	{ "reset", prompt_cmd_reset, NULL },
	{ "unbind", prompt_cmd_unbind, prompt_cmpl_unbind },
#ifdef WITH_CTV
	{ "ctv_push", prompt_cmd_filter_push, prompt_cmpl_filter_push },
	{ "ctv_pop", prompt_cmd_filter_pop, NULL },
	{ "ctv_none", prompt_cmd_filter_none, NULL },
#endif
	{ "calibrate", prompt_cmd_calibrate, NULL },
	{ "calibrate_js", prompt_cmd_calibrate, NULL }, // deprecated name
	{ "config_load", prompt_cmd_config_load, prompt_cmpl_config_file },
	{ "config_save", prompt_cmd_config_save, prompt_cmpl_config_file },
#ifdef WITH_VGMDUMP
	{ "vgmdump", prompt_cmd_vgmdump, prompt_cmpl_vgmdump },
#endif
	{ NULL, NULL, NULL }
};

/// Extra commands return values.
#define CMD_OK 0x00 ///< command successful
#define CMD_EINVAL 0x01 ///< invalid argument
#define CMD_FAIL 0x02 ///< command failed
#define CMD_ERROR 0x03 ///< fatal error, DGen should exit
#define CMD_MSG 0x80 ///< pd_message() has been used

/// Stopped flag used by pd_stopped()
static int stopped = 0;

/// Events handling status.
static enum events {
	STARTED,
	STOPPED,
	STOPPED_PROMPT,
	STOPPED_GAME_GENIE,
	PROMPT,
	GAME_GENIE
} events = STARTED;

static int stop_events(md& megad, enum events status);
static void restart_events(md &megad);

/// Messages shown whenever events are stopped.
static const char stopped_str[] = "STOPPED.";
static const char prompt_str[] = ":";
static const char game_genie_str[] = "Enter Game Genie/Hex code: ";

/// Enable emulation by default.
bool pd_freeze = false;
static unsigned int pd_freeze_ref = 0;

static void freeze(bool toggle)
{
	if (toggle == true) {
		if (!pd_freeze_ref) {
			assert(pd_freeze == false);
			pd_freeze = true;
		}
		pd_freeze_ref++;
	}
	else {
		if (pd_freeze_ref) {
			assert(pd_freeze == true);
			pd_freeze_ref--;
			if (!pd_freeze_ref)
				pd_freeze = false;
		}
		else
			assert(pd_freeze == false);
	}
}

/**
 * Elapsed time in microseconds.
 * @return Microseconds.
 */
unsigned long pd_usecs(void)
{
#ifndef _KOLIBRI
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (unsigned long)((tv.tv_sec * 1000000) + tv.tv_usec);
#else
	return _ksys_get_tick_count()*10000;
#endif
}

/**
 * Prompt "exit" command handler.
 * @return Error status to make DGen exit.
 */
static int prompt_cmd_exit(class md&, unsigned int, const char**)
{
	return (CMD_ERROR | CMD_MSG);
}

/**
 * Prompt "load" command handler.
 * @param md Context.
 * @param ac Number of arguments in av.
 * @param av Arguments.
 * @return Status code.
 */
static int prompt_cmd_load(class md& md, unsigned int ac, const char** av)
{
	extern int slot;
	extern void ram_save(class md&);
	extern void ram_load(class md&);
	char *s;

	if (ac != 2)
		return CMD_EINVAL;
	s = backslashify((const uint8_t *)av[1], strlen(av[1]), 0, NULL);
	if (s == NULL)
		return CMD_FAIL;
	ram_save(md);
	if (dgen_autosave) {
		slot = 0;
		md_save(md);
	}
	md.unplug();
	pd_message("");
	if (md.load(av[1])) {
		mdscr_splash();
		pd_message("Unable to load \"%s\"", s);
		free(s);
		return (CMD_FAIL | CMD_MSG);
	}
	pd_message("Loaded \"%s\"", s);
	free(s);
	if (dgen_show_carthead)
		pd_show_carthead(md);
	// Initialize like main() does.
	md.reset();

	if (!dgen_region) {
		uint8_t c = md.region_guess();
		int hz;
		int pal;

		md::region_info(c, &pal, &hz, 0, 0, 0);
		if ((hz != dgen_hz) || (pal != dgen_pal) || (c != md.region)) {
			md.region = c;
			dgen_hz = hz;
			dgen_pal = pal;
			printf("sdl: reconfiguring for region \"%c\": "
			       "%dHz (%s)\n", c, hz, (pal ? "PAL" : "NTSC"));
			pd_graphics_reinit(dgen_sound, dgen_pal, dgen_hz);
			if (dgen_sound) {
				long rate = dgen_soundrate;
				unsigned int samples;

				pd_sound_deinit();
				samples = (dgen_soundsegs * (rate / dgen_hz));
				pd_sound_init(rate, samples);
			}
			md.pal = pal;
			md.init_pal();
			md.init_sound();
		}
	}

	ram_load(md);
	if (dgen_autoload) {
		slot = 0;
		md_load(md);
	}
	return (CMD_OK | CMD_MSG);
}

static void rehash_prompt_complete_common()
{
	size_t i;
	unsigned int common;

	prompt.common = 0;
	if ((prompt.complete == NULL) || (prompt.complete[0] == NULL))
		return;
	common = strlen(prompt.complete[0]);
	for (i = 1; (prompt.complete[i] != NULL); ++i) {
		unsigned int tmp;

		tmp = strcommon(prompt.complete[i], prompt.complete[(i - 1)]);
		if (tmp < common)
			common = tmp;
		if (common == 0)
			break;
	}
	prompt.common = common;
}

static char* prompt_cmpl_load(class md& md, unsigned int ac, const char** av,
			      unsigned int len)
{
	const char *prefix;
	size_t i;
	unsigned int skip;

	(void)md;
	assert(ac != 0);
	if ((ac == 1) || (len == ~0u) || (av[(ac - 1)] == NULL)) {
		prefix = "";
		len = 0;
	}
	else
		prefix = av[(ac - 1)];
	if (prompt.complete == NULL) {
		// Rebuild cache.
		prompt.skip = 0;
		prompt.complete = complete_path(prefix, len,
						dgen_rom_path.val);
		if (prompt.complete == NULL)
			return NULL;
		rehash_prompt_complete_common();
	}
retry:
	skip = prompt.skip;
	for (i = 0; (prompt.complete[i] != NULL); ++i) {
		if (skip == 0)
			break;
		--skip;
	}
	if (prompt.complete[i] == NULL) {
		if (prompt.skip != 0) {
			prompt.skip = 0;
			goto retry;
		}
		return NULL;
	}
	++prompt.skip;
	return strdup(prompt.complete[i]);
}

static int prompt_cmd_unload(class md& md, unsigned int, const char**)
{
	extern int slot;
	extern void ram_save(class md&);

	info.length = 0; // clear postponed messages
	pd_message("No cartridge.");
	ram_save(md);
	if (dgen_autosave) {
		slot = 0;
		md_save(md);
	}
	if (md.unplug())
		return (CMD_FAIL | CMD_MSG);
	mdscr_splash();
	return (CMD_OK | CMD_MSG);
}

static char* prompt_cmpl_config_file(class md& md, unsigned int ac,
				     const char** av, unsigned int len)
{
	const char *prefix;
	size_t i;
	unsigned int skip;

	(void)md;
	assert(ac != 0);
	if ((ac == 1) || (len == ~0u) || (av[(ac - 1)] == NULL)) {
		prefix = "";
		len = 0;
	}
	else
		prefix = av[(ac - 1)];
	if (prompt.complete == NULL) {
		// Rebuild cache.
		prompt.skip = 0;
		prompt.complete = complete_path(prefix, len, "config");
		if (prompt.complete == NULL)
			return NULL;
		rehash_prompt_complete_common();
	}
retry:
	skip = prompt.skip;
	for (i = 0; (prompt.complete[i] != NULL); ++i) {
		if (skip == 0)
			break;
		--skip;
	}
	if (prompt.complete[i] == NULL) {
		if (prompt.skip != 0) {
			prompt.skip = 0;
			goto retry;
		}
		return NULL;
	}
	++prompt.skip;
	return strdup(prompt.complete[i]);
}

static int prompt_rehash_rc_field(const struct rc_field*, md&);

/**
 * Prompt "config_load" command handler.
 * @param md Context.
 * @param ac Number of arguments in av.
 * @param av Arguments.
 * @return Status code.
 */
static int prompt_cmd_config_load(class md& md, unsigned int ac,
				  const char** av)
{
	FILE *f;
	char *s;
	unsigned int i;

	if (ac != 2)
		return CMD_EINVAL;
	s = backslashify((const uint8_t *)av[1], strlen(av[1]), 0, NULL);
	if (s == NULL)
		return CMD_FAIL;
	f = dgen_fopen("config", av[1], (DGEN_READ | DGEN_CURRENT));
	if (f == NULL) {
		pd_message("Cannot load configuration \"%s\": %s.",
			   s, strerror(errno));
		free(s);
		return (CMD_FAIL | CMD_MSG);
	}
	parse_rc(f, av[1]);
	fclose(f);
	for (i = 0; (rc_fields[i].fieldname != NULL); ++i)
		prompt_rehash_rc_field(&rc_fields[i], md);
	pd_message("Loaded configuration \"%s\".", s);
	free(s);
	return (CMD_OK | CMD_MSG);
}

/**
 * Prompt "config_save" command handler.
 * @param md Context.
 * @param ac Number of arguments in av.
 * @param av Arguments.
 * @return Status code.
 */
static int prompt_cmd_config_save(class md& md, unsigned int ac,
				  const char** av)
{
	FILE *f;
	char *s;

	(void)md;
	if (ac != 2)
		return CMD_EINVAL;
	s = backslashify((const uint8_t *)av[1], strlen(av[1]), 0, NULL);
	if (s == NULL)
		return CMD_FAIL;
	f = dgen_fopen("config", av[1], (DGEN_WRITE | DGEN_TEXT));
	if (f == NULL) {
		pd_message("Cannot save configuration \"%s\": %s",
			   s, strerror(errno));
		free(s);
		return (CMD_FAIL | CMD_MSG);
	}
	dump_rc(f);
	fclose(f);
	pd_message("Saved configuration \"%s\"", s);
	free(s);
	return (CMD_OK | CMD_MSG);
}

static int prompt_cmd_reset(class md& md, unsigned int, const char**)
{
	md.reset();
	return CMD_OK;
}

static int prompt_cmd_unbind(class md&, unsigned int ac, const char** av)
{
	unsigned int i;
	int ret;

	if (ac < 2)
		return CMD_EINVAL;
	ret = CMD_FAIL;
	for (i = 1; (i != ac); ++i) {
		struct rc_field *rcf = rc_fields;

		while (rcf->fieldname != NULL) {
			if ((rcf->parser == rc_bind) &&
			    (!strcasecmp(av[i], rcf->fieldname))) {
				rc_binding_del(rcf);
				ret = CMD_OK;
			}
			else
				++rcf;
		}
	}
	return ret;
}

static char* prompt_cmpl_unbind(class md&, unsigned int ac,
				const char** av, unsigned int len)
{
	const struct rc_binding *rcb;
	const char *prefix;
	unsigned int skip;

	assert(ac != 0);
	if ((ac == 1) || (len == ~0u) || (av[(ac - 1)] == NULL)) {
		prefix = "";
		len = 0;
	}
	else
		prefix = av[(ac - 1)];
	skip = prompt.skip;
retry:
	for (rcb = rc_binding_head.next;
	     (rcb != &rc_binding_head);
	     rcb = rcb->next) {
		if (strncasecmp(prefix, rcb->rc, len))
			continue;
		if (skip == 0)
			break;
		--skip;
	}
	if (rcb == &rc_binding_head) {
		if (prompt.skip != 0) {
			prompt.skip = 0;
			goto retry;
		}
		return NULL;
	}
	++prompt.skip;
	return strdup(rcb->rc);
}

#ifdef WITH_VGMDUMP

static char* prompt_cmpl_vgmdump(class md& md, unsigned int ac,
				 const char** av, unsigned int len)
{
	const char *prefix;
	size_t i;
	unsigned int skip;

	(void)md;
	assert(ac != 0);
	if ((ac == 1) || (len == ~0u) || (av[(ac - 1)] == NULL)) {
		prefix = "";
		len = 0;
	}
	else
		prefix = av[(ac - 1)];
	if (prompt.complete == NULL) {
		// Rebuild cache.
		prompt.skip = 0;
		prompt.complete = complete_path(prefix, len, "vgm");
		if (prompt.complete == NULL)
			return NULL;
		rehash_prompt_complete_common();
	}
retry:
	skip = prompt.skip;
	for (i = 0; (prompt.complete[i] != NULL); ++i) {
		if (skip == 0)
			break;
		--skip;
	}
	if (prompt.complete[i] == NULL) {
		if (prompt.skip != 0) {
			prompt.skip = 0;
			goto retry;
		}
		return NULL;
	}
	++prompt.skip;
	return strdup(prompt.complete[i]);
}

static int prompt_cmd_vgmdump(class md& md, unsigned int ac, const char** av)
{
	char *s;

	if (ac < 2)
		return CMD_EINVAL;
	if (!strcasecmp(av[1], "stop")) {
		if (md.vgm_dump == false)
			pd_message("VGM dumping already stopped.");
		else {
			md.vgm_dump_stop();
			pd_message("Stopped VGM dumping.");
		}
		return (CMD_OK | CMD_MSG);
	}
	if (strcasecmp(av[1], "start"))
		return CMD_EINVAL;
	if (ac < 3) {
		pd_message("VGM file name required.");
		return (CMD_EINVAL | CMD_MSG);
	}
	s = backslashify((const uint8_t *)av[2], strlen(av[2]), 0, NULL);
	if (s == NULL)
		return CMD_FAIL;
	if (md.vgm_dump_start(av[2])) {
		pd_message("Cannot dump VGM to \"%s\": %s",
			   s, strerror(errno));
		free(s);
		return (CMD_FAIL | CMD_MSG);
	}
	pd_message("Started VGM dumping to \"%s\"", s);
	free(s);
	return (CMD_OK | CMD_MSG);
}

#endif

struct filter_data {
	bpp_t buf; ///< Input or output buffer.
	unsigned int width; ///< Buffer width.
	unsigned int height; ///< Buffer height.
	unsigned int pitch; ///< Number of bytes per line in buffer.
	void *data; ///< Filter-specific data.
	bool updated:1; ///< Filter updated data to match its output.
	bool failed:1; ///< Filter failed.
};

typedef void filter_func_t(const struct filter_data *in,
			   struct filter_data *out);

struct filter {
	const char *name; ///< Filter name.
	filter_func_t *func; ///< Filtering function.
	bool safe:1; ///< Output buffer can be the same as input.
	bool ctv:1; ///< Part of the CTV filters set.
	bool resize:1; ///< Filter resizes input.
};

static filter_func_t filter_scale;
static filter_func_t filter_off;
static filter_func_t filter_stretch;
#ifdef WITH_SCALE2X
static filter_func_t filter_scale2x;
#endif
#ifdef WITH_HQX
static filter_func_t filter_hqx;
#endif
#ifdef WITH_CTV
static filter_func_t filter_blur;
static filter_func_t filter_scanline;
static filter_func_t filter_interlace;
static filter_func_t filter_swab;

static void set_swab();
#endif

static const struct filter filters_available[] = {
	{ "stretch", filter_stretch, false, false, true },
	{ "scale", filter_scale, false, false, true },
#ifdef WITH_SCALE2X
	{ "scale2x", filter_scale2x, false, false, true },
#endif
#ifdef WITH_HQX
	{ "hqx", filter_hqx, false, false, true },
#endif
	{ "none", filter_off, true, false, true },
#ifdef WITH_CTV
	// These filters must match ctv_names in rc.cpp.
	{ "off", filter_off, true, true, false },
	{ "blur", filter_blur, true, true, false },
	{ "scanline", filter_scanline, true, true, false },
	{ "interlace", filter_interlace, true, true, false },
	{ "swab", filter_swab, true, true, false },
#endif
};

static unsigned int filters_stack_size;
static bool filters_stack_default;
static const struct filter *filters_stack[64];
static bpp_t filters_stack_data_buf[2];
static struct filter_data filters_stack_data[1 + elemof(filters_stack)];

/**
 * Return filter structure associated with name.
 * @param name Name of filter.
 * @return Pointer to filter or NULL if not found.
 */
static const struct filter *filters_find(const char *name)
{
	size_t i;

	for (i = 0; (i != elemof(filters_available)); ++i)
		if (!strcasecmp(name, filters_available[i].name))
			return &filters_available[i];
	return NULL;
}

/**
 * Update filters data, reallocate extra buffers if necessary.
 */
static void filters_stack_update()
{
	size_t i;
	const struct filter *f;
	struct filter_data *fd;
	unsigned int buffers;
	bpp_t buf[2];
	struct filter_data in_fd = {
		// Use the same formula as draw_scanline() in ras.cpp to avoid
		// the messy border for any supported depth.
		{ ((uint8_t *)mdscr.data + (mdscr.pitch * 8) + 16) },
		video.width,
		video.height,
		(unsigned int)mdscr.pitch,
		NULL,
		false,
		false,
	};
	struct filter_data out_fd = {
		{ screen.buf.u8 },
		screen.width,
		(screen.height - screen.info_height),
		screen.pitch,
		NULL,
		false,
		false,
	};
	struct filter_data *prev_fd;

	DEBUG(("updating filters data"));
retry:
	assert(filters_stack_size <= elemof(filters_stack));
	buffers = 0;
	buf[0].u8 = filters_stack_data_buf[0].u8;
	buf[1].u8 = filters_stack_data_buf[1].u8;
	// Get the number of defined filters and count how many of them cannot
	// use the same buffer for both input and output.
	// Unless they are on top, "unsafe" filters require extra buffers.
	assert(filters_stack_data[0].data == NULL);
	for (i = 0; (i != elemof(filters_stack)); ++i) {
		if (i == filters_stack_size)
			break;
		f = filters_stack[i];
		assert(f != NULL);
		if ((f->safe == false) && (i != (filters_stack_size - 1)))
			++buffers;
		// Clear filters stack output data.
		free(filters_stack_data[i + 1].data);
	}
	memset(filters_stack_data, 0, sizeof(filters_stack_data));
	// Add a valid default filter if stack is empty.
	if (i == 0) {
		assert(filters_stack_size == 0);
		filters_stack[0] = &filters_available[0];
		++filters_stack_size;
		filters_stack_default = true;
		goto retry;
	}
	// Remove default filter if there is one and stack is not empty.
	else if ((i > 1) && (filters_stack_default == true)) {
		assert(filters_stack_size > 1);
		--filters_stack_size;
		memmove(&filters_stack[0], &filters_stack[1],
			(sizeof(filters_stack[0]) * filters_stack_size));
		filters_stack_default = false;
		goto retry;
	}
	// Check if extra buffers are required.
	if (buffers) {
		if (buffers > 2)
			buffers = 2;
		else {
			// Remove unnecessary buffer.
			free(buf[1].u8);
			buf[1].u8 = NULL;
			filters_stack_data_buf[1].u8 = NULL;
		}
		DEBUG(("requiring %u extra buffer(s)", buffers));
		// Reallocate them.
		for (i = 0; (i != buffers); ++i) {
			size_t size = (screen.pitch * screen.height);

			DEBUG(("temporary buffer %u size: %zu", i, size));
			buf[i].u8 =
				(uint8_t *)realloc((void *)buf[i].u8, size);
			if (size == 0) {
				assert(buf[i].u8 == NULL);
				DEBUG(("freed zero-sized buffer"));
				filters_stack_data_buf[i].u8 = NULL;
				continue;
			}
			if (buf[i].u8 == NULL) {
				// Not good, remove one of the filters that
				// require an extra buffer and try again.
				free(filters_stack_data_buf[i].u8);
				filters_stack_data_buf[i].u8 = NULL;
				for (i = 0;
				     (i < filters_stack_size);
				     ++i) {
					if (filters_stack[i]->safe == true)
						continue;
					--filters_stack_size;
					memmove(&filters_stack[i],
						&filters_stack[i + 1],
						(sizeof(filters_stack[i]) *
						 (filters_stack_size - i)));
					break;
				}
				goto retry;
			}
			filters_stack_data_buf[i].u8 = buf[i].u8;
		}
	}
	else {
		// No extra buffer required, deallocate them.
		DEBUG(("removing temporary buffers"));
		for (i = 0; (i != elemof(buf)); ++i) {
			free(buf[i].u8);
			buf[i].u8 = NULL;
			filters_stack_data_buf[i].u8 = NULL;
		}
	}
	// Update I/O buffers.
	buffers = 0;
	prev_fd = &filters_stack_data[0];
	memcpy(prev_fd, &in_fd, sizeof(*prev_fd));
	for (i = 0; (i != elemof(filters_stack)); ++i) {
		if (i == filters_stack_size)
			break;
		f = filters_stack[i];
		fd = &filters_stack_data[i + 1];
		// The last filter uses screen output.
		if (i == (filters_stack_size - 1))
			memcpy(fd, &out_fd, sizeof(*fd));
		// Safe filters have the same input as their output.
		else if (f->safe == true)
			memcpy(fd, prev_fd, sizeof(*fd));
		// Other filters output to a temporary buffer.
		else {
			fd->buf.u8 = buf[buffers].u8;
			fd->width = screen.width;
			fd->height = (screen.height - screen.info_height);
			fd->pitch = screen.pitch;
			fd->data = NULL;
			fd->updated = false;
			fd->failed = false;
			buffers ^= 1;
		}
		prev_fd = fd;
	}
#ifndef NDEBUG
	DEBUG(("filters stack:"));
	for (i = 0; (i != filters_stack_size); ++i)
		DEBUG(("- %s (input: %p output: %p)",
		       filters_stack[i]->name,
		       (void *)filters_stack_data[i].buf.u8,
		       (void *)filters_stack_data[i + 1].buf.u8));
#endif
	screen_clear();
}

/**
 * Add filter to stack.
 * @param f Filter to add.
 */
static void filters_push(const struct filter *f)
{
	assert(filters_stack_size <= elemof(filters_stack));
	if ((f == NULL) || (filters_stack_size == elemof(filters_stack)))
		return;
	DEBUG(("%s", f->name));
	filters_stack[filters_stack_size] = f;
	filters_stack_data[filters_stack_size + 1].data = NULL;
	++filters_stack_size;
	filters_stack_update();
}

/**
 * Insert filter at the bottom of the stack.
 * @param f Filter to insert.
 */
static void filters_insert(const struct filter *f)
{
	assert(filters_stack_size <= elemof(filters_stack));
	if ((f == NULL) ||
	    (filters_stack_size == elemof(filters_stack)))
		return;
	DEBUG(("%s", f->name));
	memmove(&filters_stack[1], &filters_stack[0],
		(filters_stack_size * sizeof(filters_stack[0])));
	filters_stack[0] = f;
	filters_stack_data[0 + 1].data = NULL;
	++filters_stack_size;
	filters_stack_update();
}

// Currently unused.
#if 0

/**
 * Add filter to stack if not already in it.
 * @param f Filter to add.
 */
static void filters_push_once(const struct filter *f)
{
	size_t i;

	assert(filters_stack_size <= elemof(filters_stack));
	if (f == NULL)
		return;
	DEBUG(("%s", f->name));
	for (i = 0; (i != filters_stack_size); ++i)
		if (filters_stack[i] == f)
			return;
	filters_push(f);
}

#endif

#ifdef WITH_CTV

/**
 * Remove last filter from stack.
 */
static void filters_pop()
{
	assert(filters_stack_size <= elemof(filters_stack));
	if (filters_stack_size) {
		--filters_stack_size;
		DEBUG(("%s", filters_stack[filters_stack_size]->name));
		free(filters_stack_data[filters_stack_size + 1].data);
#ifndef NDEBUG
		memset(&filters_stack[filters_stack_size], 0xf0,
		       sizeof(filters_stack[filters_stack_size]));
		memset(&filters_stack_data[filters_stack_size + 1], 0xf1,
		       sizeof(filters_stack_data[filters_stack_size + 1]));
#endif
	}
	filters_stack_update();
}

#endif

/**
 * Remove a filter from anywhere in the stack.
 * @param index Filters stack index.
 */
static void filters_remove(unsigned int index)
{
	assert(filters_stack_size <= elemof(filters_stack));
	if (index >= filters_stack_size)
		return;
	--filters_stack_size;
	DEBUG(("%s", filters_stack[index]->name));
	free(filters_stack_data[index + 1].data);
#ifndef NDEBUG
	memset(&filters_stack[index], 0xf2, sizeof(filters_stack[index]));
	memset(&filters_stack_data[index + 1], 0xf3,
	       sizeof(filters_stack_data[index + 1]));
#endif
	memmove(&filters_stack[index], &filters_stack[index + 1],
		(sizeof(filters_stack[index]) * (filters_stack_size - index)));
	memmove(&filters_stack_data[index + 1], &filters_stack_data[index + 2],
		(sizeof(filters_stack_data[index + 1]) *
		 (filters_stack_size - index)));
	filters_stack_update();
}

/**
 * Remove all occurences of filter from the stack.
 * @param f Filter to remove.
 */
static void filters_pluck(const struct filter *f)
{
	size_t i;

	assert(filters_stack_size <= elemof(filters_stack));
	if (f == NULL)
		return;
	DEBUG(("%s", f->name));
	for (i = 0; (i < filters_stack_size); ++i) {
		if (filters_stack[i] != f)
			continue;
		--filters_stack_size;
		DEBUG(("%s", filters_stack[i]->name));
		free(filters_stack_data[i + 1].data);
#ifndef NDEBUG
		memset(&filters_stack[i], 0xf4, sizeof(filters_stack[i]));
		memset(&filters_stack_data[i + 1], 0xf5,
		       sizeof(filters_stack_data[i + 1]));
#endif
		memmove(&filters_stack[i], &filters_stack[i + 1],
			(sizeof(filters_stack[i]) * (filters_stack_size - i)));
		memmove(&filters_stack_data[i + 1], &filters_stack_data[i + 2],
			(sizeof(filters_stack_data[i + 1]) *
			 (filters_stack_size - i)));
		--i;
	}
	filters_stack_update();
}

#ifdef WITH_CTV

/**
 * Remove all occurences of CTV filters from the stack.
 */
static void filters_pluck_ctv()
{
	size_t i;

	assert(filters_stack_size <= elemof(filters_stack));
	for (i = 0; (i < filters_stack_size); ++i) {
		if (filters_stack[i]->ctv == false)
			continue;
		--filters_stack_size;
		DEBUG(("%s", filters_stack[i]->name));
		free(filters_stack_data[i + 1].data);
#ifndef NDEBUG
		memset(&filters_stack[i], 0xf6, sizeof(filters_stack[i]));
		memset(&filters_stack_data[i + 1], 0xf6,
		       sizeof(filters_stack_data[i + 1]));
#endif
		memmove(&filters_stack[i], &filters_stack[i + 1],
			(sizeof(filters_stack[i]) * (filters_stack_size - i)));
		memmove(&filters_stack_data[i + 1], &filters_stack_data[i + 2],
			(sizeof(filters_stack_data[i + 1]) *
			 (filters_stack_size - i)));
		--i;
	}
	filters_stack_update();
}

#endif

#ifdef WITH_CTV

/**
 * Empty filters stack.
 */
static void filters_empty()
{
	size_t i;

	assert(filters_stack_size <= elemof(filters_stack));
	DEBUG(("stack size was %u", filters_stack_size));
	for (i = 0; (i < filters_stack_size); ++i)
		free(filters_stack_data[i + 1].data);
	filters_stack_size = 0;
#ifndef NDEBUG
	memset(filters_stack, 0xb0, sizeof(filters_stack));
	memset(filters_stack_data, 0xb0, sizeof(filters_stack_data));
	filters_stack_data[0].data = NULL;
#endif
	filters_stack_update();
}

#endif

/**
 * Take a screenshot.
 */
static void do_screenshot(md& megad)
{
	static unsigned int n = 0;
	static char romname_old[sizeof(megad.romname)];
	FILE *fp;
#ifdef HAVE_FTELLO
	off_t pos;
#else
	long pos;
#endif
	bpp_t line;
	unsigned int width;
	unsigned int height;
	unsigned int pitch;
	unsigned int bpp = mdscr.bpp;
	uint8_t (*out)[3]; // 24 bpp
	char name[(sizeof(megad.romname) + 32)];

	if (dgen_raw_screenshots) {
		width = video.width;
		height = video.height;
		pitch = mdscr.pitch;
		line.u8 = ((uint8_t *)mdscr.data + (pitch * 8) + 16);
	}
	else {
		width = screen.width;
		height = screen.height;
		pitch = screen.pitch;
		line = screen.buf;
	}
	switch (bpp) {
	case 15:
	case 16:
	case 24:
	case 32:
		break;
	default:
		pd_message("Screenshots unsupported in %d bpp.", bpp);
		return;
	}
	// Make take a long time, let the main loop know about it.
	stopped = 1;
	// If megad.romname is different from last time, reset n.
	if (memcmp(romname_old, megad.romname, sizeof(romname_old))) {
		memcpy(romname_old, megad.romname, sizeof(romname_old));
		n = 0;
	}
retry:
	snprintf(name, sizeof(name), "%s-%06u.tga",
		 ((megad.romname[0] == '\0') ? "unknown" : megad.romname), n);
	fp = dgen_fopen("screenshots", name, DGEN_APPEND);
	if (fp == NULL) {
		pd_message("Can't open %s.", name);
		return;
	}
	fseek(fp, 0, SEEK_END);
#ifdef HAVE_FTELLO
	pos = ftello(fp);
#else
	pos = ftell(fp);
#endif
	if (((off_t)pos == (off_t)-1) || ((off_t)pos != (off_t)0)) {
		fclose(fp);
		n = ((n + 1) % 1000000);
		goto retry;
	}
	// Allocate line buffer.
	if ((out = (uint8_t (*)[3])malloc(sizeof(*out) * width)) == NULL)
		goto error;
	// Header
	{
		uint8_t tmp[(3 + 5)] = {
			0x00, // length of the image ID field
			0x00, // whether a color map is included
			0x02 // image type: uncompressed, true-color image
			// 5 bytes of color map specification
		};

		if (!fwrite(tmp, sizeof(tmp), 1, fp))
			goto error;
	}
	{
		uint16_t tmp[4] = {
			0, // x-origin
			0, // y-origin
			h2le16(width), // width
			h2le16(height) // height
		};

		if (!fwrite(tmp, sizeof(tmp), 1, fp))
			goto error;
	}
	{
		uint8_t tmp[2] = {
			24, // always output 24 bits per pixel
			(1 << 5) // top-left origin
		};

		if (!fwrite(tmp, sizeof(tmp), 1, fp))
			goto error;
	}
	// Data
	switch (bpp) {
		unsigned int y;
		unsigned int x;

	case 15:
		for (y = 0; (y < height); ++y) {
			if (screen_lock())
				goto error;
			for (x = 0; (x < width); ++x) {
				uint16_t v = line.u16[x];

				out[x][0] = ((v << 3) & 0xf8);
				out[x][1] = ((v >> 2) & 0xf8);
				out[x][2] = ((v >> 7) & 0xf8);
			}
			screen_unlock();
			if (!fwrite(out, (sizeof(*out) * width), 1, fp))
				goto error;
			line.u8 += pitch;
		}
		break;
	case 16:
		for (y = 0; (y < height); ++y) {
			if (screen_lock())
				goto error;
			for (x = 0; (x < width); ++x) {
				uint16_t v = line.u16[x];

				out[x][0] = ((v << 3) & 0xf8);
				out[x][1] = ((v >> 3) & 0xfc);
				out[x][2] = ((v >> 8) & 0xf8);
			}
			screen_unlock();
			if (!fwrite(out, (sizeof(*out) * width), 1, fp))
				goto error;
			line.u8 += pitch;
		}
		break;
	case 24:
		for (y = 0; (y < height); ++y) {
			if (screen_lock())
				goto error;
#ifdef WORDS_BIGENDIAN
			for (x = 0; (x < width); ++x) {
				out[x][0] = line.u24[x][2];
				out[x][1] = line.u24[x][1];
				out[x][2] = line.u24[x][0];
			}
#else
			memcpy(out, line.u24, (sizeof(*out) * width));
#endif
			screen_unlock();
			if (!fwrite(out, (sizeof(*out) * width), 1, fp))
				goto error;
			line.u8 += pitch;
		}
		break;
	case 32:
		for (y = 0; (y < height); ++y) {
			if (screen_lock())
				goto error;
			for (x = 0; (x < width); ++x) {
#ifdef WORDS_BIGENDIAN
				uint32_t rgb = h2le32(line.u32[x]);

				memcpy(&(out[x]), &rgb, 3);
#else
				memcpy(&(out[x]), &(line.u32[x]), 3);
#endif
			}
			screen_unlock();
			if (!fwrite(out, (sizeof(*out) * width), 1, fp))
				goto error;
			line.u8 += pitch;
		}
		break;
	}
	pd_message("Screenshot written to %s.", name);
	free(out);
	fclose(fp);
	return;
error:
	pd_message("Error while generating screenshot %s.", name);
	free(out);
	fclose(fp);
}

/**
 * SDL flags help.
 */
void pd_help()
{
  printf(
#ifdef WITH_OPENGL
  "    -g (1|0)        Enable/disable OpenGL.\n"
#endif
  "    -f              Attempt to run fullscreen.\n"
  "    -X scale        Scale the screen in the X direction.\n"
  "    -Y scale        Scale the screen in the Y direction.\n"
  "    -S scale        Scale the screen by the same amount in both directions.\n"
  "    -G WxH          Desired window size.\n"
  );
}

/**
 * Handle rc variables
 */
void pd_rc()
{
	// Set stuff up from the rcfile first, so we can override it with
	// command-line options
	if (dgen_scale >= 1) {
		dgen_x_scale = dgen_scale;
		dgen_y_scale = dgen_scale;
	}
#ifdef WITH_CTV
	set_swab();
#endif
}

/**
 * Handle the switches.
 * @param c Switch's value.
 */
void pd_option(char c, const char *)
{
	int xs, ys;

	switch (c) {
#ifdef WITH_OPENGL
	case 'g':
		dgen_opengl = atoi(optarg);
		break;
#endif
	case 'f':
		dgen_fullscreen = 1;
		break;
	case 'X':
		if ((xs = atoi(optarg)) <= 0)
			break;
		dgen_x_scale = xs;
		break;
	case 'Y':
		if ((ys = atoi(optarg)) <= 0)
			break;
		dgen_y_scale = ys;
		break;
	case 'S':
		if ((xs = atoi(optarg)) <= 0)
			break;
		dgen_x_scale = xs;
		dgen_y_scale = xs;
		break;
	case 'G':
		if ((sscanf(optarg, " %d x %d ", &xs, &ys) != 2) ||
		    (xs < 0) || (ys < 0))
			break;
		dgen_width = xs;
		dgen_height = ys;
		break;
	}
}

#ifdef WITH_OPENGL

#ifdef WORDS_BIGENDIAN
#define TEXTURE_16_TYPE GL_UNSIGNED_SHORT_5_6_5
#define TEXTURE_32_TYPE GL_UNSIGNED_INT_8_8_8_8_REV
#else
#define TEXTURE_16_TYPE GL_UNSIGNED_SHORT_5_6_5
#define TEXTURE_32_TYPE GL_UNSIGNED_BYTE
#endif

static void texture_init_id(struct texture& texture)
{
	GLint param;

	if (texture.linear)
		param = GL_LINEAR;
	else
		param = GL_NEAREST;
	glBindTexture(GL_TEXTURE_2D, texture.id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param);
	if (texture.u32 == 0)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
			     texture.width, texture.height,
			     0, GL_RGB, TEXTURE_16_TYPE,
			     texture.buf.u16);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
			     texture.width, texture.height,
			     0, GL_BGRA, TEXTURE_32_TYPE,
			     texture.buf.u32);
}

static void texture_init_dlist(struct texture& texture)
{
	glNewList(texture.dlist, GL_COMPILE);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, texture.vis_width, texture.vis_height, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glBindTexture(GL_TEXTURE_2D, texture.id);
	glBegin(GL_QUADS);
	glTexCoord2i(0, 1);
	glVertex2i(0, texture.height); // lower left
	glTexCoord2i(0, 0);
	glVertex2i(0, 0); // upper left
	glTexCoord2i(1, 0);
	glVertex2i(texture.width, 0); // upper right
	glTexCoord2i(1, 1);
	glVertex2i(texture.width, texture.height); // lower right
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glEndList();
}

/**
 * Round a value up to nearest power of two.
 * @param v Value.
 * @return Rounded value.
 */
static uint32_t roundup2(uint32_t v)
{
	--v;
	v |= (v >> 1);
	v |= (v >> 2);
	v |= (v >> 4);
	v |= (v >> 8);
	v |= (v >> 16);
	++v;
	return v;
}

static void release_texture(struct texture& texture)
{
	if ((texture.dlist != 0) && (glIsList(texture.dlist))) {
		glDeleteTextures(1, &texture.id);
		glDeleteLists(texture.dlist, 1);
		texture.dlist = 0;
	}
	free(texture.buf.u32);
	texture.buf.u32 = NULL;
}

static int init_texture(struct screen *screen)
{
	struct texture& texture = screen->texture;
	unsigned int vis_width;
	unsigned int vis_height;
	unsigned int x;
	unsigned int y;
	unsigned int w;
	unsigned int h;
	void *tmp;
	size_t i;
	GLenum error;

	// When bool_opengl_stretch is enabled, width and height are redefined
	// using X and Y scale factors with additional room for the info bar.
	if (dgen_opengl_stretch) {
		vis_width = (video.width *
			     (screen->x_scale ? screen->x_scale : 1));
		vis_height = (video.height *
			      (screen->y_scale ? screen->y_scale : 1));
		vis_height += screen->info_height;
		if (dgen_aspect) {
			// Keep scaled aspect ratio.
			w = ((screen->height * vis_width) / vis_height);
			h = ((screen->width * vis_height) / vis_width);
			if (w >= screen->width) {
				w = screen->width;
				if (h == 0)
					++h;
			}
			else {
				h = screen->height;
				if (w == 0)
					++w;
			}
		}
		else {
			// Aspect ratio won't be kept.
			w = screen->width;
			h = screen->height;
		}
	}
	else {
		w = vis_width = screen->width;
		h = vis_height = screen->height;
	}
	if (screen->width > w)
		x = ((screen->width - w) / 2);
	else
		x = 0;
	if (screen->height > h)
		y = ((screen->height - h) / 2);
	else
		y = 0;
	DEBUG(("initializing for width=%u height=%u", vis_width, vis_height));
	// Set viewport.
	DEBUG(("glViewport(%u, %u, %u, %u)", x, y, w, h));
	glViewport(x, y, w, h);
	// Disable dithering
	glDisable(GL_DITHER);
	// Disable anti-aliasing
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_POINT_SMOOTH);
	// Disable depth buffer
	glDisable(GL_DEPTH_TEST);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_FLAT);

	// Initialize and allocate texture.
	texture.u32 = (!!dgen_opengl_32bit);
	texture.linear = (!!dgen_opengl_linear);
	texture.width = roundup2(vis_width);
	texture.height = roundup2(vis_height);
	if (dgen_opengl_square) {
		// Texture must be square.
		if (texture.width < texture.height)
			texture.width = texture.height;
		else
			texture.height = texture.width;
	}
	texture.vis_width = vis_width;
	texture.vis_height = vis_height;
	DEBUG(("texture width=%u height=%u", texture.width, texture.height));
	if ((texture.width == 0) || (texture.height == 0))
		goto fail;
	i = ((texture.width * texture.height) * (2 << texture.u32));
	DEBUG(("texture size=%lu (%u Bpp)",
	       (unsigned long)i, (2 << texture.u32)));
	if ((tmp = realloc(texture.buf.u32, i)) == NULL)
		goto fail;
	memset(tmp, 0, i);
	texture.buf.u32 = (uint32_t *)tmp;
	if ((texture.dlist != 0) && (glIsList(texture.dlist))) {
		glDeleteTextures(1, &texture.id);
		glDeleteLists(texture.dlist, 1);
	}
	DEBUG(("texture buf=%p", (void *)texture.buf.u32));
	if ((texture.dlist = glGenLists(1)) == 0)
		goto fail;
	if ((glGenTextures(1, &texture.id), error = glGetError()) ||
	    (texture_init_id(texture), error = glGetError()) ||
	    (texture_init_dlist(texture), error = glGetError())) {
		// Do something with "error".
		goto fail;
	}
	DEBUG(("texture initialization OK"));
	return 0;
fail:
	release_texture(texture);
	DEBUG(("texture initialization failed"));
	return -1;
}

static void update_texture(struct texture& texture)
{
	glBindTexture(GL_TEXTURE_2D, texture.id);
	if (texture.u32 == 0)
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
				texture.vis_width, texture.vis_height,
				GL_RGB, TEXTURE_16_TYPE,
				texture.buf.u16);
	else
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
				texture.vis_width, texture.vis_height,
				GL_BGRA, TEXTURE_32_TYPE,
				texture.buf.u32);
	glCallList(texture.dlist);
	SDL_GL_SwapBuffers();
}

#endif // WITH_OPENGL

/**
 * This filter passes input to output unchanged, only centered or truncated
 * if necessary. Doesn't have any fallback, thus cannot fail.
 * @param in Input buffer data.
 * @param out Output buffer data.
 */
static void filter_off(const struct filter_data *in, struct filter_data *out)
{
	unsigned int line;
	unsigned int height;
	uint8_t *in_buf;
	uint8_t *out_buf;

	// Check if copying is necessary.
	if (in->buf.u8 == out->buf.u8)
		return;
	// Copy line by line and center.
	if (in->height > out->height)
		height = out->height;
	else
		height = in->height;
	if (out->updated == false) {
		if (in->width <= out->width) {
			unsigned int x_off = ((out->width - in->width) / 2);
			unsigned int y_off = ((out->height - height) / 2);

			out->buf.u8 += (x_off * screen.Bpp);
			out->buf.u8 += (out->pitch * y_off);
			out->width = in->width;
		}
		out->height = height;
		out->updated = true;
	}
	in_buf = in->buf.u8;
	out_buf = out->buf.u8;
	for (line = 0; (line != height); ++line) {
		memcpy(out_buf, in_buf, (out->width * screen.Bpp));
		in_buf += in->pitch;
		out_buf += out->pitch;
	}
}

// Copy/rescale functions.

struct filter_scale_data {
	unsigned int x_scale;
	unsigned int y_scale;
	filter_func_t *filter;
};

template <typename uintX_t>
static void filter_scale_X(const struct filter_data *in,
			   struct filter_data *out)
{
	struct filter_scale_data *data =
		(struct filter_scale_data *)out->data;
	uintX_t *dst = (uintX_t *)out->buf.u32;
	unsigned int dst_pitch = out->pitch;
	uintX_t *src = (uintX_t *)in->buf.u32;
	unsigned int src_pitch = in->pitch;
	unsigned int width = in->width;
	unsigned int x_scale = data->x_scale;
	unsigned int y_scale = data->y_scale;
	unsigned int height = in->height;
	unsigned int y;

	for (y = 0; (y != height); ++y) {
		uintX_t *out = dst;
		unsigned int i;
		unsigned int x;

		for (x = 0; (x != width); ++x) {
			uintX_t tmp = src[x];

			for (i = 0; (i != x_scale); ++i)
				*(out++) = tmp;
		}
		out = dst;
		dst = (uintX_t *)((uint8_t *)dst + dst_pitch);
		for (i = 1; (i < y_scale); ++i) {
			memcpy(dst, out, (width * sizeof(*dst) * x_scale));
			out = dst;
			dst = (uintX_t *)((uint8_t *)dst + dst_pitch);
		}
		src = (uintX_t *)((uint8_t *)src + src_pitch);
	}
}

static void filter_scale_3(const struct filter_data *in,
			   struct filter_data *out)
{
	struct filter_scale_data *data =
		(struct filter_scale_data *)out->data;
	uint24_t *dst = out->buf.u24;
	unsigned int dst_pitch = out->pitch;
	uint24_t *src = in->buf.u24;
	unsigned int src_pitch = in->pitch;
	unsigned int width = in->width;
	unsigned int x_scale = data->x_scale;
	unsigned int y_scale = data->y_scale;
	unsigned int height = in->height;
	unsigned int y;

	for (y = 0; (y != height); ++y) {
		uint24_t *out = dst;
		unsigned int i;
		unsigned int x;

		for (x = 0; (x != width); ++x) {
			uint24_t tmp;

			u24cpy(&tmp, &src[x]);
			for (i = 0; (i != x_scale); ++i)
				u24cpy((out++), &tmp);
		}
		out = dst;
		dst = (uint24_t *)((uint8_t *)dst + dst_pitch);
		for (i = 1; (i < y_scale); ++i) {
			memcpy(dst, out, (width * sizeof(*dst) * x_scale));
			out = dst;
			dst = (uint24_t *)((uint8_t *)dst + dst_pitch);
		}
		src = (uint24_t *)((uint8_t *)src + src_pitch);
	}
}

/**
 * This filter attempts to rescale according to screen X/Y factors.
 * @param in Input buffer data.
 * @param out Output buffer data.
 */
static void filter_scale(const struct filter_data *in,
			 struct filter_data *out)
{
	static const struct {
		unsigned int Bpp;
		filter_func_t *func;
	} scale_mode[] = {
		{ 1, filter_scale_X<uint8_t> },
		{ 2, filter_scale_X<uint16_t> },
		{ 3, filter_scale_3 },
		{ 4, filter_scale_X<uint32_t> },
	};
	struct filter_scale_data *data;
	unsigned int width;
	unsigned int height;
	unsigned int x_off;
	unsigned int y_off;
	unsigned int x_scale;
	unsigned int y_scale;
	filter_func_t *filter;
	unsigned int i;

	if (out->failed == true) {
	failed:
		filter_off(in, out);
		return;
	}
	if (out->updated == true) {
		data = (struct filter_scale_data *)out->data;
		filter = data->filter;
	process:
		// Feed this to the basic scaler.
		(*filter)(in, out);
		return;
	}
	// Initialize filter.
	assert(out->data == NULL);
	x_scale = screen.x_scale;
	y_scale = screen.y_scale;
	while ((width = (in->width * x_scale)) > out->width)
		--x_scale;
	while ((height = (in->height * y_scale)) > out->height)
		--y_scale;
	// Check whether output is large enough.
	if ((x_scale == 0) || (y_scale == 0)) {
		DEBUG(("cannot rescale by %ux%u", x_scale, y_scale));
		out->failed = true;
		goto failed;
	}
	// Not rescaling is faster through filter_off().
	if ((x_scale == 1) && (y_scale == 1)) {
		DEBUG(("using faster fallback for %ux%u", x_scale, y_scale));
		out->failed = true;
		goto failed;
	}
	// Find a suitable filter.
	for (i = 0; (i != elemof(scale_mode)); ++i)
		if (scale_mode[i].Bpp == screen.Bpp)
			break;
	if (i == elemof(scale_mode)) {
		DEBUG(("%u Bpp depth is not supported", screen.Bpp));
		out->failed = true;
		goto failed;
	}
	DEBUG(("using %u Bpp function to scale by %ux%u",
	       screen.Bpp, x_scale, y_scale));
	data = (struct filter_scale_data *)malloc(sizeof(*data));
	if (data == NULL) {
		DEBUG(("allocation failure"));
		out->failed = true;
		goto failed;
	}
	filter = scale_mode[i].func;
	data->filter = filter;
	data->x_scale = x_scale;
	data->y_scale = y_scale;
	// Center output.
	x_off = ((out->width - width) / 2);
	y_off = ((out->height - height) / 2);
	out->buf.u8 += (x_off * screen.Bpp);
	out->buf.u8 += (out->pitch * y_off);
	out->width = width;
	out->height = height;
	out->data = (void *)data;
	out->updated = true;
	goto process;
}

struct filter_stretch_data {
	uint8_t *h_table;
	uint8_t *v_table;
	filter_func_t *filter;
};

template <typename uintX_t>
static void filter_stretch_X(const struct filter_data *in,
			     struct filter_data *out)
{
	struct filter_stretch_data *data =
		(struct filter_stretch_data *)out->data;
	uint8_t *h_table = data->h_table;
	uint8_t *v_table = data->v_table;
	uintX_t *dst = (uintX_t *)out->buf.u8;
	unsigned int dst_pitch = out->pitch;
	unsigned int dst_w = out->width;
	uintX_t *src = (uintX_t *)in->buf.u8;
	unsigned int src_pitch = in->pitch;
	unsigned int src_w = in->width;
	unsigned int src_h = in->height;
	unsigned int src_y;

	dst_pitch /= sizeof(*dst);
	src_pitch /= sizeof(*src);
	for (src_y = 0; (src_y != src_h); ++src_y) {
		uint8_t v_repeat = v_table[src_y];
		unsigned int src_x;
		unsigned int dst_x;

		if (!v_repeat) {
			src += src_pitch;
			continue;
		}
		for (src_x = 0, dst_x = 0; (src_x != src_w); ++src_x) {
			uint8_t h_repeat = h_table[src_x];

			if (!h_repeat)
				continue;
			while (h_repeat--)
				dst[dst_x++] = src[src_x];
		}
		dst += dst_pitch;
		while (--v_repeat) {
			memcpy(dst, (dst - dst_pitch), (dst_w * sizeof(*dst)));
			dst += dst_pitch;
		}
		src += src_pitch;
	}
}

static void filter_stretch_3(const struct filter_data *in,
			     struct filter_data *out)
{
	struct filter_stretch_data *data =
		(struct filter_stretch_data *)out->data;
	uint8_t *h_table = data->h_table;
	uint8_t *v_table = data->v_table;
	uint24_t *dst = out->buf.u24;
	unsigned int dst_pitch = out->pitch;
	unsigned int dst_w = out->width;
	uint24_t *src = in->buf.u24;
	unsigned int src_pitch = in->pitch;
	unsigned int src_w = in->width;
	unsigned int src_h = in->height;
	unsigned int src_y;

	dst_pitch /= sizeof(*dst);
	src_pitch /= sizeof(*src);
	for (src_y = 0; (src_y != src_h); ++src_y) {
		uint8_t v_repeat = v_table[src_y];
		unsigned int src_x;
		unsigned int dst_x;

		if (!v_repeat) {
			src += src_pitch;
			continue;
		}
		for (src_x = 0, dst_x = 0; (src_x != src_w); ++src_x) {
			uint8_t h_repeat = h_table[src_x];

			if (!h_repeat)
				continue;
			while (h_repeat--)
				u24cpy(&dst[dst_x++], &src[src_x]);
		}
		dst += dst_pitch;
		while (--v_repeat) {
			memcpy(dst, (dst - dst_pitch), (dst_w * sizeof(*dst)));
			dst += dst_pitch;
		}
		src += src_pitch;
	}
}

/**
 * This filter stretches the input buffer to fill the entire output.
 * @param in Input buffer data.
 * @param out Output buffer data.
 */
static void filter_stretch(const struct filter_data *in,
			   struct filter_data *out)
{
	static const struct {
		unsigned int Bpp;
		filter_func_t *func;
	} stretch_mode[] = {
		{ 1, filter_stretch_X<uint8_t> },
		{ 2, filter_stretch_X<uint16_t> },
		{ 3, filter_stretch_3 },
		{ 4, filter_stretch_X<uint32_t> },
	};
	struct filter_stretch_data *data;
	unsigned int dst_w;
	unsigned int dst_h;
	unsigned int src_w;
	unsigned int src_h;
	unsigned int h_ratio;
	unsigned int v_ratio;
	unsigned int dst_x;
	unsigned int dst_y;
	unsigned int src_x;
	unsigned int src_y;
	filter_func_t *filter;
	unsigned int i;

	if (out->failed == true) {
	failed:
		filter_off(in, out);
		return;
	}
	if (out->updated == true) {
		data = (struct filter_stretch_data *)out->data;
		filter = data->filter;
	process:
		(*filter)(in, out);
		return;
	}
	// Initialize filter.
	assert(out->data == NULL);
	dst_w = out->width;
	dst_h = out->height;
	src_w = in->width;
	src_h = in->height;
	if ((src_h == 0) || (src_w == 0)) {
		DEBUG(("invalid input size: %ux%u", src_h, src_w));
		out->failed = true;
		goto failed;
	}
	// Make sure input and output pitches are multiples of pixel size
	// at the current depth.
	if ((in->pitch % screen.Bpp) || (out->pitch % screen.Bpp)) {
		DEBUG(("Bpp: %u, in->pitch: %u, out->pitch: %u",
		       screen.Bpp, in->pitch, out->pitch));
		out->failed = true;
		goto failed;
	}
	// Find a suitable filter.
	for (i = 0; (i != elemof(stretch_mode)); ++i)
		if (stretch_mode[i].Bpp == screen.Bpp)
			break;
	if (i == elemof(stretch_mode)) {
		DEBUG(("%u Bpp depth is not supported", screen.Bpp));
		out->failed = true;
		goto failed;
	}
	filter = stretch_mode[i].func;
	// Fix output if original aspect ratio must be kept.
	if (dgen_aspect) {
		unsigned int w = ((dst_h * src_w) / src_h);
		unsigned int h = ((dst_w * src_h) / src_w);

		if (w >= dst_w) {
			w = dst_w;
			if (h == 0)
				++h;
		}
		else {
			h = dst_h;
			if (w == 0)
				++w;
		}
		dst_w = w;
		dst_h = h;
	}
	// Precompute H and V pixel ratios.
	h_ratio = ((dst_w << 10) / src_w);
	v_ratio = ((dst_h << 10) / src_h);
	data = (struct filter_stretch_data *)
		calloc(1, sizeof(*data) + src_w + src_h);
	if (data == NULL) {
		DEBUG(("allocation failure"));
		out->failed = true;
		goto failed;
	}
	DEBUG(("stretching %ux%u to %ux%u/%ux%u (aspect ratio %s)",
	       src_w, src_h, dst_w, dst_h, out->width, out->height,
	       (dgen_aspect ? "must be kept" : "is free")));
	data->h_table = (uint8_t *)(data + 1);
	data->v_table = (data->h_table + src_w);
	data->filter = filter;
	for (dst_x = 0; (dst_x != dst_w); ++dst_x) {
		src_x = ((dst_x << 10) / h_ratio);
		if (src_x < src_w)
			++data->h_table[src_x];
	}
	for (dst_y = 0; (dst_y != dst_h); ++dst_y) {
		src_y = ((dst_y << 10) / v_ratio);
		if (src_y < src_h)
			++data->v_table[src_y];
	}
	// Center output.
	dst_x = ((out->width - dst_w) / 2);
	dst_y = ((out->height - dst_h) / 2);
	out->buf.u8 += (dst_x * screen.Bpp);
	out->buf.u8 += (out->pitch * dst_y);
	out->width = dst_w;
	out->height = dst_h;
	out->data = (void *)data;
	out->updated = true;
	goto process;
}

#ifdef WITH_HQX

/**
 * This filter attempts to rescale with HQX.
 * @param in Input buffer data.
 * @param out Output buffer data.
 */
static void filter_hqx(const struct filter_data *in, struct filter_data *out)
{
	typedef void hqx_func_t(void *src, uint32_t src_pitch,
				void *dst, uint32_t dst_pitch,
				int width, int height);

	static const struct {
		unsigned int Bpp;
		unsigned int scale;
		hqx_func_t *func;
	} hqx_mode[] = {
		{ 2, 2, (hqx_func_t *)hq2x_16_rb },
		{ 2, 3, (hqx_func_t *)hq3x_16_rb },
		{ 2, 4, (hqx_func_t *)hq4x_16_rb },
		{ 3, 2, (hqx_func_t *)hq2x_24_rb },
		{ 3, 3, (hqx_func_t *)hq3x_24_rb },
		{ 3, 4, (hqx_func_t *)hq4x_24_rb },
		{ 4, 2, (hqx_func_t *)hq2x_32_rb },
		{ 4, 3, (hqx_func_t *)hq3x_32_rb },
		{ 4, 4, (hqx_func_t *)hq4x_32_rb },
	};
	static bool hqx_initialized = false;
	unsigned int width;
	unsigned int height;
	unsigned int x_off;
	unsigned int y_off;
	unsigned int x_scale;
	unsigned int y_scale;
	hqx_func_t *hqx;
	unsigned int i;

	if (out->failed == true) {
	failed:
		filter_off(in, out);
		return;
	}
	if (out->updated == true) {
		hqx = *(hqx_func_t **)out->data;
	process:
		// Feed this to HQX.
		(*hqx)((void *)in->buf.u32, in->pitch,
		       (void *)out->buf.u32, out->pitch,
		       in->width, in->height);
		return;
	}
	// Initialize filter.
	assert(out->data == NULL);
	x_scale = screen.x_scale;
	y_scale = screen.y_scale;
retry:
	while ((width = (in->width * x_scale)) > out->width)
		--x_scale;
	while ((height = (in->height * y_scale)) > out->height)
		--y_scale;
	// Check whether output is large enough.
	if ((x_scale == 0) || (y_scale == 0)) {
		DEBUG(("cannot rescale by %ux%u", x_scale, y_scale));
		out->failed = true;
		goto failed;
	}
	// Find a suitable combination.
	for (i = 0; (i != elemof(hqx_mode)); ++i)
		if ((hqx_mode[i].Bpp == screen.Bpp) &&
		    (hqx_mode[i].scale == x_scale) &&
		    (hqx_mode[i].scale == y_scale))
			break;
	if (i == elemof(hqx_mode)) {
		// Nothing matches, find something that fits.
		DEBUG(("%ux%u @%u Bpp scale factor not supported, trying"
		       " another",
		       x_scale, y_scale, screen.Bpp));
		// Must be square.
		if (x_scale > y_scale)
			x_scale = y_scale;
		else if (y_scale > x_scale)
			y_scale = x_scale;
		assert(x_scale == y_scale);
		(void)y_scale;
		do {
			--i;
			if ((hqx_mode[i].Bpp == screen.Bpp) &&
			    (hqx_mode[i].scale <= x_scale)) {
				x_scale = hqx_mode[i].scale;
				y_scale = hqx_mode[i].scale;
				goto retry;
			}
		}
		while (i != 0);
		DEBUG(("failed to use %ux%u @%u Bpp scale factor",
		       x_scale, y_scale, screen.Bpp));
		out->failed = true;
		goto failed;
	}
	DEBUG(("using %ux%u @%u Bpp scale factor",
	       x_scale, y_scale, screen.Bpp));
	hqx = hqx_mode[i].func;
	out->data = malloc(sizeof(hqx));
	if (out->data == NULL) {
		DEBUG(("allocation failure"));
		out->failed = true;
		goto failed;
	}
	*(hqx_func_t **)out->data = hqx;
	// Center output.
	x_off = ((out->width - width) / 2);
	y_off = ((out->height - height) / 2);
	out->buf.u8 += (x_off * screen.Bpp);
	out->buf.u8 += (out->pitch * y_off);
	out->width = width;
	out->height = height;
	out->updated = true;
	// Initialize HQX if necessary.
	if (hqx_initialized == false) {
		pd_message_cursor(~0u, "Initializing hqx...");
		stopped = 1;
		hqxInit();
		pd_message_cursor(~0u, "");
		hqx_initialized = true;
	}
	goto process;
}

#endif // WITH_HQX

#ifdef WITH_SCALE2X

/**
 * This filter attempts to rescale with Scale2x.
 * @param in Input buffer data.
 * @param out Output buffer data.
 */
static void filter_scale2x(const struct filter_data *in,
			   struct filter_data *out)
{
	static const struct {
		unsigned int x_scale;
		unsigned int y_scale;
		unsigned int mode;
	} scale2x_mode[] = {
		{ 2, 2, 2 },
		{ 2, 3, 203 },
		{ 2, 4, 204 },
		{ 3, 3, 3 },
		{ 4, 4, 4 }
	};
	unsigned int width;
	unsigned int height;
	unsigned int x_off;
	unsigned int y_off;
	unsigned int x_scale;
	unsigned int y_scale;
	unsigned int mode;
	unsigned int i;

	if (out->failed == true) {
	failed:
		filter_off(in, out);
		return;
	}
	if (out->updated == true) {
		mode = *(unsigned int *)out->data;
	process:
		// Feed this to scale2x.
		scale(mode, out->buf.u32, out->pitch, in->buf.u32, in->pitch,
		      screen.Bpp, in->width, in->height);
		return;
	}
	// Initialize filter.
	assert(out->data == NULL);
	x_scale = screen.x_scale;
	y_scale = screen.y_scale;
retry:
	while ((width = (in->width * x_scale)) > out->width)
		--x_scale;
	while ((height = (in->height * y_scale)) > out->height)
		--y_scale;
	// Check whether output is large enough.
	if ((x_scale == 0) || (y_scale == 0)) {
		DEBUG(("cannot rescale by %ux%u", x_scale, y_scale));
		out->failed = true;
		goto failed;
	}
	// Check whether depth is supported by filter.
	if ((screen.Bpp != 4) && (screen.Bpp != 2)) {
		DEBUG(("unsupported depth %u", screen.bpp));
		out->failed = true;
		goto failed;
	}
	// Find a suitable combination.
	for (i = 0; (i != elemof(scale2x_mode)); ++i)
		if ((scale2x_mode[i].x_scale == x_scale) &&
		    (scale2x_mode[i].y_scale == y_scale))
			break;
	if (i == elemof(scale2x_mode)) {
		// Nothing matches, find something that fits.
		DEBUG(("%ux%u scale factor not supported, trying another",
		       x_scale, y_scale));
		do {
			--i;
			if ((scale2x_mode[i].x_scale <= x_scale) &&
			    (scale2x_mode[i].y_scale <= y_scale)) {
				x_scale = scale2x_mode[i].x_scale;
				y_scale = scale2x_mode[i].y_scale;
				goto retry;
			}
		}
		while (i != 0);
		DEBUG(("failed to use %ux%u scale factor", x_scale, y_scale));
		out->failed = true;
		goto failed;
	}
	DEBUG(("using %ux%u scale factor", x_scale, y_scale));
	mode = scale2x_mode[i].mode;
	out->data = malloc(sizeof(mode));
	if (out->data == NULL) {
		DEBUG(("allocation failure"));
		out->failed = true;
		goto failed;
	}
	*(unsigned int *)out->data = mode;
	// Center output.
	x_off = ((out->width - width) / 2);
	y_off = ((out->height - height) / 2);
	out->buf.u8 += (x_off * screen.Bpp);
	out->buf.u8 += (out->pitch * y_off);
	out->width = width;
	out->height = height;
	out->updated = true;
	goto process;
}

#endif // WITH_SCALE2X

#ifdef WITH_CTV

// "Blur" CTV filters.

static void filter_blur_32(const struct filter_data *in,
			   struct filter_data *out)
{
	bpp_t in_buf = in->buf;
	bpp_t out_buf = out->buf;
	unsigned int xsize = out->width;
	unsigned int ysize = out->height;
	unsigned int y;

	for (y = 0; (y < ysize); ++y) {
		uint32_t old = *in_buf.u32;
		unsigned int x;

		for (x = 0; (x < xsize); ++x) {
			uint32_t tmp = in_buf.u32[x];

			tmp = (((((tmp & 0x00ff00ff) +
				  (old & 0x00ff00ff)) >> 1) & 0x00ff00ff) |
			       ((((tmp & 0xff00ff00) +
				  (old & 0xff00ff00)) >> 1) & 0xff00ff00));
			old = in_buf.u32[x];
			out_buf.u32[x] = tmp;
		}
		in_buf.u8 += in->pitch;
		out_buf.u8 += out->pitch;
	}
}

static void filter_blur_24(const struct filter_data *in,
			   struct filter_data *out)
{
	bpp_t in_buf = in->buf;
	bpp_t out_buf = out->buf;
	unsigned int xsize = out->width;
	unsigned int ysize = out->height;
	unsigned int y;

	for (y = 0; (y < ysize); ++y) {
		uint24_t old;
		unsigned int x;

		u24cpy(&old, in_buf.u24);
		for (x = 0; (x < xsize); ++x) {
			uint24_t tmp;

			u24cpy(&tmp, &in_buf.u24[x]);
			out_buf.u24[x][0] = ((tmp[0] + old[0]) >> 1);
			out_buf.u24[x][1] = ((tmp[1] + old[1]) >> 1);
			out_buf.u24[x][2] = ((tmp[2] + old[2]) >> 1);
			u24cpy(&old, &tmp);
		}
		in_buf.u8 += in->pitch;
		out_buf.u8 += out->pitch;
	}
}

static void filter_blur_16(const struct filter_data *in,
			   struct filter_data *out)
{
	bpp_t in_buf = in->buf;
	bpp_t out_buf = out->buf;
	unsigned int xsize = out->width;
	unsigned int ysize = out->height;
	unsigned int y;

#ifdef WITH_X86_CTV
	if (in_buf.u16 == out_buf.u16) {
		for (y = 0; (y < ysize); ++y) {
			// Blur, by Dave
			blur_bitmap_16((uint8_t *)out_buf.u16, (xsize - 1));
			out_buf.u8 += out->pitch;
		}
		return;
	}
#endif
	for (y = 0; (y < ysize); ++y) {
		uint16_t old = *in_buf.u16;
		unsigned int x;

		for (x = 0; (x < xsize); ++x) {
			uint16_t tmp = in_buf.u16[x];

			tmp = (((((tmp & 0xf81f) +
				  (old & 0xf81f)) >> 1) & 0xf81f) |
			       ((((tmp & 0x07e0) +
				  (old & 0x07e0)) >> 1) & 0x07e0));
			old = in_buf.u16[x];
			out_buf.u16[x] = tmp;
		}
		in_buf.u8 += in->pitch;
		out_buf.u8 += out->pitch;
	}
}

static void filter_blur_15(const struct filter_data *in,
			   struct filter_data *out)
{
	bpp_t in_buf = in->buf;
	bpp_t out_buf = out->buf;
	unsigned int xsize = out->width;
	unsigned int ysize = out->height;
	unsigned int y;

#ifdef WITH_X86_CTV
	if (in_buf.u15 == out_buf.u15) {
		for (y = 0; (y < ysize); ++y) {
			// Blur, by Dave
			blur_bitmap_15((uint8_t *)out_buf.u15, (xsize - 1));
			out_buf.u8 += out->pitch;
		}
		return;
	}
#endif
	for (y = 0; (y < ysize); ++y) {
		uint16_t old = *in_buf.u15;
		unsigned int x;

		for (x = 0; (x < xsize); ++x) {
			uint16_t tmp = in_buf.u15[x];

			tmp = (((((tmp & 0x7c1f) +
				  (old & 0x7c1f)) >> 1) & 0x7c1f) |
			       ((((tmp & 0x03e0) +
				  (old & 0x03e0)) >> 1) & 0x03e0));
			old = in_buf.u15[x];
			out_buf.u15[x] = tmp;
		}
		in_buf.u8 += in->pitch;
		out_buf.u8 += out->pitch;
	}
}

static void filter_blur(const struct filter_data *in,
			struct filter_data *out)
{
	static const struct {
		unsigned int bpp;
		filter_func_t *filter;
	} blur_mode[] = {
		{ 32, filter_blur_32 },
		{ 24, filter_blur_24 },
		{ 16, filter_blur_16 },
		{ 15, filter_blur_15 },
	};
	filter_func_t *blur;

	if (out->failed == true) {
	failed:
		filter_off(in, out);
		return;
	}
	if (out->updated == false) {
		unsigned int i;

		for (i = 0; (i != elemof(blur_mode)); ++i)
			if (blur_mode[i].bpp == screen.bpp)
				break;
		if (i == elemof(blur_mode)) {
			DEBUG(("%u bpp depth is not supported", screen.bpp));
			out->failed = true;
			goto failed;
		}
		blur = blur_mode[i].filter;
		out->data = malloc(sizeof(filter));
		if (out->data == NULL) {
			DEBUG(("allocation failure"));
			out->failed = true;
			goto failed;
		}
		if (in->width <= out->width) {
			unsigned int x_off = ((out->width - in->width) / 2);
			unsigned int y_off = ((out->height - in->height) / 2);

			out->buf.u8 += (x_off * screen.Bpp);
			out->buf.u8 += (out->pitch * y_off);
			out->width = in->width;
		}
		if (in->height <= out->height)
			out->height = in->height;
		*((filter_func_t **)out->data) = blur;
		out->updated = true;
	}
	else
		blur = *(filter_func_t **)out->data;
	(*blur)(in, out);
}

// Scanline/Interlace CTV filters.

static void filter_scanline_frame(const struct filter_data *in,
				  struct filter_data *out)
{
	unsigned int frame = ((unsigned int *)out->data)[0];
	unsigned int bpp = ((unsigned int *)out->data)[1];
	bpp_t in_buf = in->buf;
	bpp_t out_buf = out->buf;
	unsigned int xsize = out->width;
	unsigned int ysize = out->height;

	out_buf.u8 += (out->pitch * !!frame);
	switch (bpp) {
		unsigned int x;
		unsigned int y;

	case 32:
		for (y = frame; (y < ysize); y += 2) {
			for (x = 0; (x < xsize); ++x)
				out_buf.u32[x] =
					((in_buf.u32[x] >> 1) & 0x7f7f7f7f);
			in_buf.u8 += (in->pitch * 2);
			out_buf.u8 += (out->pitch * 2);
		}
		break;
	case 24:
		for (y = frame; (y < ysize); y += 2) {
			for (x = 0; (x < xsize); ++x) {
				out_buf.u24[x][0] = (in_buf.u24[x][0] >> 1);
				out_buf.u24[x][1] = (in_buf.u24[x][1] >> 1);
				out_buf.u24[x][2] = (in_buf.u24[x][2] >> 1);
			}
			in_buf.u8 += (in->pitch * 2);
			out_buf.u8 += (out->pitch * 2);
		}
		break;
	case 16:
		for (y = frame; (y < ysize); y += 2) {
#ifdef WITH_X86_CTV
			if (in_buf.u16 == out_buf.u16) {
				// Scanline, by Phil
				test_ctv((uint8_t *)out_buf.u16, xsize);
			}
			else
#endif
			for (x = 0; (x < xsize); ++x)
				out_buf.u16[x] =
					((in_buf.u16[x] >> 1) & 0x7bef);
			in_buf.u8 += (in->pitch * 2);
			out_buf.u8 += (out->pitch * 2);
		}
		break;
	case 15:
		for (y = frame; (y < ysize); y += 2) {
#ifdef WITH_X86_CTV
			if (in_buf.u15 == out_buf.u15) {
				// Scanline, by Phil
				test_ctv((uint8_t *)out_buf.u16, xsize);
			}
			else
#endif
			for (x = 0; (x < xsize); ++x)
				out_buf.u15[x] =
					((in_buf.u15[x] >> 1) & 0x3def);
			in_buf.u8 += (in->pitch * 2);
			out_buf.u8 += (out->pitch * 2);
		}
		break;
	}
}

static void filter_scanline(const struct filter_data *in,
			    struct filter_data *out)
{
	if (out->failed == true) {
	failed:
		filter_off(in, out);
		return;
	}
	if (out->updated == false) {
		if ((screen.bpp != 32) &&
		    (screen.bpp != 24) &&
		    (screen.bpp != 16) &&
		    (screen.bpp != 15)) {
			DEBUG(("%u bpp depth is not supported", screen.bpp));
			out->failed = true;
			goto failed;
		}
		out->data = malloc(sizeof(unsigned int [2]));
		if (out->data == NULL) {
			DEBUG(("allocation failure"));
			out->failed = true;
			goto failed;
		}
		if (in->width <= out->width) {
			unsigned int x_off = ((out->width - in->width) / 2);
			unsigned int y_off = ((out->height - in->height) / 2);

			out->buf.u8 += (x_off * screen.Bpp);
			out->buf.u8 += (out->pitch * y_off);
			out->width = in->width;
		}
		if (in->height <= out->height)
			out->height = in->height;
		((unsigned int *)out->data)[0] = 0;
		((unsigned int *)out->data)[1] = screen.bpp;
		out->updated = true;
	}
	filter_scanline_frame(in, out);
}

static void filter_interlace(const struct filter_data *in,
			     struct filter_data *out)
{
	if (out->failed == true) {
	failed:
		filter_off(in, out);
		return;
	}
	if (out->updated == false) {
		if ((screen.bpp != 32) &&
		    (screen.bpp != 24) &&
		    (screen.bpp != 16) &&
		    (screen.bpp != 15)) {
			DEBUG(("%u bpp depth is not supported", screen.bpp));
			out->failed = true;
			goto failed;
		}
		out->data = malloc(sizeof(unsigned int [2]));
		if (out->data == NULL) {
			DEBUG(("allocation failure"));
			out->failed = true;
			goto failed;
		}
		if (in->width <= out->width) {
			unsigned int x_off = ((out->width - in->width) / 2);
			unsigned int y_off = ((out->height - in->height) / 2);

			out->buf.u8 += (x_off * screen.Bpp);
			out->buf.u8 += (out->pitch * y_off);
			out->width = in->width;
		}
		if (in->height <= out->height)
			out->height = in->height;
		((unsigned int *)out->data)[0] = 0;
		((unsigned int *)out->data)[1] = screen.bpp;
		out->updated = true;
	}
	filter_scanline_frame(in, out);
	((unsigned int *)out->data)[0] ^= 1;
}

// Byte swap filter.
static void filter_swab(const struct filter_data *in,
			struct filter_data *out)
{
	bpp_t in_buf;
	bpp_t out_buf;
	unsigned int xsize;
	unsigned int ysize;

	if (out->failed == true) {
	failed:
		filter_off(in, out);
		return;
	}
	if (out->updated == false) {
		if ((screen.Bpp != 4) &&
		    (screen.Bpp != 3) &&
		    (screen.Bpp != 2)) {
			DEBUG(("%u Bpp depth is not supported", screen.Bpp));
			out->failed = true;
			goto failed;
		}
		if (in->width <= out->width) {
			unsigned int x_off = ((out->width - in->width) / 2);
			unsigned int y_off = ((out->height - in->height) / 2);

			out->buf.u8 += (x_off * screen.Bpp);
			out->buf.u8 += (out->pitch * y_off);
			out->width = in->width;
		}
		if (in->height <= out->height)
			out->height = in->height;
		out->updated = true;
	}
	in_buf = in->buf;
	out_buf = out->buf;
	ysize = out->height;
	xsize = out->width;
	switch (screen.Bpp) {
		unsigned int x;
		unsigned int y;

	case 4:
		for (y = 0; (y < ysize); ++y) {
			for (x = 0; (x < xsize); ++x) {
				union {
					uint32_t u32;
					uint8_t u8[4];
				} tmp[2];

				tmp[0].u32 = in_buf.u32[x];
				tmp[1].u8[0] = tmp[0].u8[3];
				tmp[1].u8[1] = tmp[0].u8[2];
				tmp[1].u8[2] = tmp[0].u8[1];
				tmp[1].u8[3] = tmp[0].u8[0];
				out_buf.u32[x] = tmp[1].u32;
			}
			in_buf.u8 += in->pitch;
			out_buf.u8 += out->pitch;
		}
		break;
	case 3:
		for (y = 0; (y < ysize); ++y) {
			for (x = 0; (x < xsize); ++x) {
				uint24_t tmp = {
					in_buf.u24[x][2],
					in_buf.u24[x][1],
					in_buf.u24[x][0]
				};

				u24cpy(&out_buf.u24[x], &tmp);
			}
			in_buf.u8 += in->pitch;
			out_buf.u8 += out->pitch;
		}
		break;
	case 2:
		for (y = 0; (y < ysize); ++y) {
			for (x = 0; (x < xsize); ++x)
				out_buf.u16[x] = ((in_buf.u16[x] << 8) |
						  (in_buf.u16[x] >> 8));
			in_buf.u8 += in->pitch;
			out_buf.u8 += out->pitch;
		}
		break;
	}
}

#endif // WITH_CTV

/**
 * Special characters interpreted by filter_text().
 * FILTER_TEXT_BG_NONE  transparent background.
 * FILTER_TEXT_BG_BLACK black background.
 * FILTER_TEXT_7X6      use 7x6 font.
 * FILTER_TEXT_8X13     use 8x13 font.
 * FILTER_TEXT_16X26    use 16x26 font.
 * FILTER_TEXT_CENTER   center justify.
 * FILTER_TEXT_LEFT     left justify.
 * FILTER_TEXT_RIGHT    right justify.
 */
#define FILTER_TEXT_ESCAPE "\033"
#define FILTER_TEXT_BG_NONE FILTER_TEXT_ESCAPE "\x01\x01"
#define FILTER_TEXT_BG_BLACK FILTER_TEXT_ESCAPE "\x01\x02"
#define FILTER_TEXT_7X6 FILTER_TEXT_ESCAPE "\x02\x01"
#define FILTER_TEXT_8X13 FILTER_TEXT_ESCAPE "\x02\x02"
#define FILTER_TEXT_16X26 FILTER_TEXT_ESCAPE "\x02\x03"
#define FILTER_TEXT_CENTER FILTER_TEXT_ESCAPE "\x03\x01"
#define FILTER_TEXT_LEFT FILTER_TEXT_ESCAPE "\x03\x02"
#define FILTER_TEXT_RIGHT FILTER_TEXT_ESCAPE "\x03\x03"

static char filter_text_str[2048];

/**
 * Append message to filter_text_str[].
 */
static void filter_text_msg(const char *fmt, ...)
{
	size_t off;
	size_t len = sizeof(filter_text_str);
	va_list vl;

	assert(filter_text_str[(len - 1)] == '\0');
	off = strlen(filter_text_str);
	len -= off;
	if (len == 0)
		return;
	va_start(vl, fmt);
	vsnprintf(&filter_text_str[off], len, fmt, vl);
	va_end(vl);
}

/**
 * Text overlay filter.
 * @param in Input buffer data.
 * @param out Output buffer data.
 */
static void filter_text(const struct filter_data *in,
			struct filter_data *out)
{
	bpp_t buf = out->buf;
	unsigned int buf_pitch = out->pitch;
	unsigned int xsize = out->width;
	unsigned int ysize = out->height;
	unsigned int bpp = screen.bpp;
	unsigned int Bpp = ((bpp + 1) / 8);
	const char *str = filter_text_str;
	const char *next = str;
	bool clear = false;
	bool flush = false;
	enum { LEFT, CENTER, RIGHT } justify = LEFT;
	const struct {
		enum font_type type;
		unsigned int width;
		unsigned int height;
	} font_data[] = {
		{ FONT_TYPE_7X5, 7, (5 + 1) }, // +1 for vertical spacing.
		{ FONT_TYPE_8X13, 8, 13 },
		{ FONT_TYPE_16X26, 16, 26 }
	}, *font = &font_data[0], *old_font = font;
	unsigned int line_length = 0;
	unsigned int line_off = 0;
	unsigned int line_width = 0;
	unsigned int line_height = font->height;

	// Input is unused.
	(void)in;
	assert(filter_text_str[(sizeof(filter_text_str) - 1)] == '\0');
	while (1) {
		unsigned int len;
		unsigned int width;

		if ((*next == '\0') || (*next == '\n')) {
		trunc:
			if (flush == false) {
				next = str;
				assert(line_width <= xsize);
				switch (justify) {
				case LEFT:
					line_off = 0;
					break;
				case CENTER:
					line_off = ((xsize - line_width) / 2);
					break;
				case RIGHT:
					line_off = (xsize - line_width);
					break;
				}
				if (clear)
					memset(buf.u8, 0,
					       (buf_pitch * line_height));
				font = old_font;
				flush = true;
			}
			else if (*next == '\0')
				break;
			else {
				if (*next == '\n')
					++next;
				str = next;
				old_font = font;
				line_length = 0;
				line_off = 0;
				line_width = 0;
				buf.u8 += (buf_pitch * line_height);
				ysize -= line_height;
				line_height = font->height;
				// Still enough vertical pixels for this line?
				if (ysize < line_height)
					break;
				flush = false;
			}
		}
		else if (*next == *FILTER_TEXT_ESCAPE) {
			const char *tmp;
			size_t sz;

#define FILTER_TEXT_IS(f)				\
			(tmp = (f), sz = strlen(f),	\
			 !strncmp(tmp, next, sz))

			if (FILTER_TEXT_IS(FILTER_TEXT_BG_NONE))
				clear = false;
			else if (FILTER_TEXT_IS(FILTER_TEXT_BG_BLACK))
				clear = true;
			else if (FILTER_TEXT_IS(FILTER_TEXT_CENTER))
				justify = CENTER;
			else if (FILTER_TEXT_IS(FILTER_TEXT_LEFT))
				justify = LEFT;
			else if (FILTER_TEXT_IS(FILTER_TEXT_RIGHT))
				justify = RIGHT;
			else if (FILTER_TEXT_IS(FILTER_TEXT_7X6))
				font = &font_data[0];
			else if (FILTER_TEXT_IS(FILTER_TEXT_8X13))
				font = &font_data[1];
			else if (FILTER_TEXT_IS(FILTER_TEXT_16X26))
				font = &font_data[2];
			next += sz;
		}
		else if ((line_width + font->width) <= xsize) {
			++line_length;
			line_width += font->width;
			if (line_height < font->height) {
				line_height = font->height;
				// Still enough vertical pixels for this line?
				if (ysize < line_height)
					break;
			}
			++next;
		}
		else // Truncate line.
			goto trunc;
		if (flush == false)
			continue;
		// Compute number of characters and width.
		len = 0;
		width = 0;
		while ((len != line_length) &&
		       (next[len] != '\0') &&
		       (next[len] != '\n') &&
		       (next[len] != *FILTER_TEXT_ESCAPE)) {
			width += font->width;
			++len;
		}
		// Display.
		len = font_text((buf.u8 +
				 // Horizontal offset.
				 (line_off * Bpp) +
				 // Vertical offset.
				 ((line_height - font->height) * buf_pitch)),
				(xsize - line_off),
				line_height, Bpp, buf_pitch, next, len, ~0u,
				font->type);
		line_off += width;
		next += len;
	}
}

static const struct filter filter_text_def = {
	"text", filter_text, true, false, false
};

#ifdef WITH_CTV

static void set_swab()
{
	const struct filter *f = filters_find("swab");

	if (f == NULL)
		return;
	filters_pluck(f);
	if (dgen_swab)
		filters_insert(f);
}

static int prompt_cmd_filter_push(class md&, unsigned int ac, const char** av)
{
	unsigned int i;

	if (ac < 2)
		return CMD_EINVAL;
	for (i = 1; (i != ac); ++i) {
		const struct filter *f = filters_find(av[i]);

		if (f == NULL)
			return CMD_EINVAL;
		filters_push(f);
	}
	return CMD_OK;
}

static char* prompt_cmpl_filter_push(class md&, unsigned int ac,
				     const char** av, unsigned int len)
{
	const struct filter *f;
	const char *prefix;
	unsigned int skip;
	unsigned int i;

	assert(ac != 0);
	if ((ac == 1) || (len == ~0u) || (av[(ac - 1)] == NULL)) {
		prefix = "";
		len = 0;
	}
	else
		prefix = av[(ac - 1)];
	skip = prompt.skip;
retry:
	for (i = 0; (i != elemof(filters_available)); ++i) {
		f = &filters_available[i];
		if (strncasecmp(prefix, f->name, len))
			continue;
		if (skip == 0)
			break;
		--skip;
	}
	if (i == elemof(filters_available)) {
		if (prompt.skip != 0) {
			prompt.skip = 0;
			goto retry;
		}
		return NULL;
	}
	++prompt.skip;
	return strdup(f->name);
}

static int prompt_cmd_filter_pop(class md&, unsigned int ac, const char**)
{
	if (ac != 1)
		return CMD_EINVAL;
	filters_pop();
	return CMD_OK;
}

static int prompt_cmd_filter_none(class md&, unsigned int ac, const char**)
{
	if (ac != 1)
		return CMD_EINVAL;
	filters_empty();
	return CMD_OK;
}

#endif // WITH_CTV

static bool calibrating = false; //< True during calibration.
static unsigned int calibrating_controller; ///< Controller being calibrated.

static void manage_calibration(enum rc_binding_type type, intptr_t code);

/**
 * Interactively calibrate a controller.
 * If n_args == 1, controller 0 will be configured.
 * If n_args == 2, configure controller in string args[1].
 * @param n_args Number of arguments.
 * @param[in] args List of arguments.
 * @return Status code.
 */
static int
prompt_cmd_calibrate(class md&, unsigned int n_args, const char** args)
{
	/* check args first */
	if (n_args == 1)
		calibrating_controller = 0;
	else if (n_args == 2) {
		calibrating_controller = (atoi(args[1]) - 1);
		if (calibrating_controller > 1)
			return CMD_EINVAL;
	}
	else
		return CMD_EINVAL;
	manage_calibration(RCB_NUM, -1);
	return (CMD_OK | CMD_MSG);
}

static int set_scaling(const char *name)
{
	unsigned int i = filters_stack_size;

	assert(i <= elemof(filters_stack));
	// Replace all current scalers with these.
	while (i != 0) {
		--i;
		if (filters_stack[i]->resize == true)
			filters_remove(i);
	}
	while (name += strspn(name, " \t\n"), name[0] != '\0') {
		const struct filter *f;
		int len = strcspn(name, " \t\n");
		char token[64];

		snprintf(token, sizeof(token), "%.*s", len, name);
		name += len;
		if (((f = filters_find(token)) == NULL) ||
		    (filters_stack_size == elemof(filters_stack)))
			return -1;
		filters_push(f);
	}
	return 0;
}

/**
 * Display splash screen.
 */
static void mdscr_splash()
{
	unsigned int x;
	unsigned int y;
	bpp_t src;
	unsigned int src_pitch = (dgen_splash_data.width *
				  dgen_splash_data.bytes_per_pixel);
	unsigned int sw = dgen_splash_data.width;
	unsigned int sh = dgen_splash_data.height;
	bpp_t dst;
	unsigned int dst_pitch = mdscr.pitch;
	unsigned int dw = video.width;
	unsigned int dh = video.height;

	if ((dgen_splash_data.bytes_per_pixel != 3) || (sw != dw))
		return;
	src.u8 = (uint8_t *)dgen_splash_data.pixel_data;
	dst.u8 = ((uint8_t *)mdscr.data + (dst_pitch * 8) + 16);
	// Center it.
	if (sh < dh) {
		unsigned int off = ((dh - sh) / 2);

		memset(dst.u8, 0x00, (dst_pitch * off));
		memset(&dst.u8[(dst_pitch * (off + sh))], 0x00,
		       (dst_pitch * (dh - (off + sh))));
		dst.u8 += (dst_pitch * off);
	}
	switch (mdscr.bpp) {
	case 32:
		for (y = 0; ((y != dh) && (y != sh)); ++y) {
			for (x = 0; ((x != dw) && (x != sw)); ++x) {
				dst.u32[x] = ((src.u24[x][0] << 16) |
					      (src.u24[x][1] << 8) |
					      (src.u24[x][2] << 0));
			}
			src.u8 += src_pitch;
			dst.u8 += dst_pitch;
		}
		break;
	case 24:
		for (y = 0; ((y != dh) && (y != sh)); ++y) {
			for (x = 0; ((x != dw) && (x != sw)); ++x) {
				dst.u24[x][0] = src.u24[x][2];
				dst.u24[x][1] = src.u24[x][1];
				dst.u24[x][2] = src.u24[x][0];
			}
			src.u8 += src_pitch;
			dst.u8 += dst_pitch;
		}
		break;
	case 16:
		for (y = 0; ((y != dh) && (y != sh)); ++y) {
			for (x = 0; ((x != dw) && (x != sw)); ++x) {
				dst.u16[x] = (((src.u24[x][0] & 0xf8) << 8) |
					      ((src.u24[x][1] & 0xfc) << 3) |
					      ((src.u24[x][2] & 0xf8) >> 3));
			}
			src.u8 += src_pitch;
			dst.u8 += dst_pitch;
		}
		break;
	case 15:
		for (y = 0; ((y != dh) && (y != sh)); ++y) {
			for (x = 0; ((x != dw) && (x != sw)); ++x) {
				dst.u16[x] = (((src.u24[x][0] & 0xf8) << 7) |
					      ((src.u24[x][1] & 0xf8) << 2) |
					      ((src.u24[x][2] & 0xf8) >> 3));
			}
			src.u8 += src_pitch;
			dst.u8 += dst_pitch;
		}
		break;
	case 8:
		break;
	}
}

/**
 * Initialize screen.
 *
 * @param width Width of display.
 * @param height Height of display.
 * @return 0 on success, -1 if screen could not be initialized with current
 * options but remains in its previous state, -2 if screen is unusable.
 */
static int screen_init(unsigned int width, unsigned int height)
{
	static bool once = true;
	uint32_t flags = (SDL_RESIZABLE | SDL_ANYFORMAT | SDL_HWPALETTE |
			SDL_HWSURFACE);
	struct screen scrtmp;
	const struct dgen_font *font;

#ifdef WITH_THREADS
	screen_update_thread_stop();
#endif
	DEBUG(("want width=%u height=%u", width, height));
	stopped = 1;
	// Copy current screen data.
	memcpy(&scrtmp, &screen, sizeof(scrtmp));
	if (once) {
		unsigned int info_height = dgen_font[FONT_TYPE_8X13].h;
		// Force defaults once.
		scrtmp.window_width = 0;
		scrtmp.window_height = 0;
		scrtmp.width = (video.width * 2);
		scrtmp.height = ((video.height * 2) + info_height);
		scrtmp.x_scale = (scrtmp.width / video.width);
		scrtmp.y_scale = (scrtmp.height / video.height);
		scrtmp.bpp = 0;
		scrtmp.Bpp = 0;
		scrtmp.info_height = info_height;
		scrtmp.buf.u8 = 0;
		scrtmp.pitch = 0;
		scrtmp.surface = 0;
		scrtmp.want_fullscreen = 0;
		scrtmp.is_fullscreen = 0;
#ifdef WITH_OPENGL
		scrtmp.want_opengl = 0;
		scrtmp.is_opengl = 0;
#endif
#ifdef WITH_THREADS
		scrtmp.want_thread = 0;
		scrtmp.is_thread = 0;
		scrtmp.thread = 0;
		scrtmp.lock = 0;
		scrtmp.cond = 0;
#endif
		memset(scrtmp.color, 0, sizeof(scrtmp.color));
		once = false;
	}
	// Use configuration data.
	if (width != 0)
		scrtmp.width = width;
	if (dgen_width >= 1)
		scrtmp.width = dgen_width;
	if (height != 0)
		scrtmp.height = height;
	if (dgen_height >= 1)
		scrtmp.height = dgen_height;
	if (dgen_depth >= 0) {
		scrtmp.bpp = dgen_depth;
		scrtmp.Bpp = 0;
	}
	// scrtmp.x_scale, scrtmp.y_scale and scrtmp.info_height cannot be
	// determined yet.
	scrtmp.want_fullscreen = !!dgen_fullscreen;
#ifdef WITH_OPENGL
opengl_failed:
	scrtmp.want_opengl = !!dgen_opengl;
#endif
#ifdef WITH_THREADS
	scrtmp.want_thread = !!dgen_screen_thread;
#endif
	// Configure SDL_SetVideoMode().
	if (scrtmp.want_fullscreen)
		flags |= SDL_FULLSCREEN;
#ifdef WITH_OPENGL
	if (scrtmp.want_opengl) {
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, !!dgen_doublebuffer);
		flags |= SDL_OPENGL;
	}
	else
#endif
		flags |= ((dgen_doublebuffer ? SDL_DOUBLEBUF : 0) |
			  SDL_ASYNCBLIT);
	if (scrtmp.want_fullscreen) {
		SDL_Rect **modes;

		// Check if we're going to be bound to a particular resolution.
		modes = SDL_ListModes(NULL, (flags | SDL_FULLSCREEN));
		if ((modes != NULL) && (modes != (SDL_Rect **)-1)) {
			unsigned int i;
			struct {
				unsigned int i;
				unsigned int w;
				unsigned int h;
			} best = { 0, (unsigned int)-1, (unsigned int)-1 };

			// Find the best resolution available.
			for (i = 0; (modes[i] != NULL); ++i) {
				unsigned int w, h;

				DEBUG(("checking mode %dx%d",
				       modes[i]->w, modes[i]->h));
				if ((modes[i]->w < scrtmp.width) ||
				    (modes[i]->h < scrtmp.height))
					continue;
				w = (modes[i]->w - scrtmp.width);
				h = (modes[i]->h - scrtmp.height);
				if ((w <= best.w) && (h <= best.h)) {
					best.i = i;
					best.w = w;
					best.h = h;
				}
			}
			if ((best.w == (unsigned int)-1) ||
			    (best.h == (unsigned int)-1))
				DEBUG(("no mode looks good"));
			else {
				scrtmp.width = modes[best.i]->w;
				scrtmp.height = modes[best.i]->h;
				DEBUG(("mode %ux%u looks okay",
				       scrtmp.width, scrtmp.height));
			}
		}
		DEBUG(("adjusted fullscreen resolution to %ux%u",
		       scrtmp.width, scrtmp.height));
	}
	// Set video mode.
	DEBUG(("SDL_SetVideoMode(%u, %u, %d, 0x%08x)",
	       scrtmp.width, scrtmp.height, scrtmp.bpp, flags));
	scrtmp.surface = SDL_SetVideoMode(scrtmp.width, scrtmp.height,
					  scrtmp.bpp, flags);
	if (scrtmp.surface == NULL) {
#ifdef WITH_OPENGL
		// Try again without OpenGL.
		if (flags & SDL_OPENGL) {
			assert(scrtmp.want_opengl);
			DEBUG(("OpenGL initialization failed, retrying"
			       " without it."));
			dgen_opengl = 0;
			flags &= ~SDL_OPENGL;
			goto opengl_failed;
		}
#endif
		return -1;
	}
	DEBUG(("SDL_SetVideoMode succeeded"));
	// Update with current values.
	scrtmp.window_width = scrtmp.surface->w;
	scrtmp.window_height = scrtmp.surface->h;
	scrtmp.width = scrtmp.window_width;
	scrtmp.height = scrtmp.window_height;
	// By default, using 5% of the vertical resolution for info bar ought
	// to be good enough for anybody. Pick something close.
	if (dgen_info_height < 0)
		scrtmp.info_height = ((scrtmp.height * 5) / 100);
	else
		scrtmp.info_height = dgen_info_height;
	if (scrtmp.info_height > scrtmp.height)
		scrtmp.info_height = scrtmp.height;
	font = font_select(scrtmp.width, scrtmp.info_height, FONT_TYPE_AUTO);
	if (font == NULL)
		scrtmp.info_height = 0;
	else
		scrtmp.info_height = font->h;
	assert(scrtmp.info_height <= scrtmp.height); // Do not forget.
	// Determine default X and Y scale values from what remains.
	if (dgen_x_scale >= 0)
		scrtmp.x_scale = dgen_x_scale;
	else
		scrtmp.x_scale = (scrtmp.width / video.width);
	if (dgen_y_scale >= 0)
		scrtmp.y_scale = dgen_y_scale;
	else
		scrtmp.y_scale = ((scrtmp.height - scrtmp.info_height) /
				  video.height);
	if (dgen_aspect) {
		if (scrtmp.x_scale >= scrtmp.y_scale)
			scrtmp.x_scale = scrtmp.y_scale;
		else
			scrtmp.y_scale = scrtmp.x_scale;
	}
	// Fix bpp.
	assert(scrtmp.surface->format != NULL);
	scrtmp.bpp = scrtmp.surface->format->BitsPerPixel;
	// 15 bpp has be forced if it was required. SDL does not return the
	// right value.
	if ((dgen_depth == 15) && (scrtmp.bpp == 16))
		scrtmp.bpp = 15;
	scrtmp.Bpp = scrtmp.surface->format->BytesPerPixel;
	scrtmp.buf.u8 = (uint8_t *)scrtmp.surface->pixels;
	scrtmp.pitch = scrtmp.surface->pitch;
	scrtmp.is_fullscreen = scrtmp.want_fullscreen;
	DEBUG(("video configuration: x_scale=%u y_scale=%u",
	       scrtmp.x_scale, scrtmp.y_scale));
	DEBUG(("screen configuration: width=%u height=%u bpp=%u Bpp=%u"
	       " info_height=%u"
	       " buf.u8=%p pitch=%u surface=%p want_fullscreen=%u"
	       " is_fullscreen=%u",
	       scrtmp.width, scrtmp.height, scrtmp.bpp, scrtmp.Bpp,
	       scrtmp.info_height,
	       (void *)scrtmp.buf.u8, scrtmp.pitch, (void *)scrtmp.surface,
	       scrtmp.want_fullscreen, scrtmp.is_fullscreen));
#ifdef WITH_OPENGL
	if (scrtmp.want_opengl) {
		if (init_texture(&scrtmp)) {
			DEBUG(("OpenGL initialization failed, retrying"
			       " without it."));
			dgen_opengl = 0;
			flags &= ~SDL_OPENGL;
			goto opengl_failed;
		}
		// Update using texture info.
		scrtmp.Bpp = (2 << scrtmp.texture.u32);
		scrtmp.bpp = (scrtmp.Bpp * 8);
		scrtmp.buf.u32 = scrtmp.texture.buf.u32;
		scrtmp.width = scrtmp.texture.vis_width;
		scrtmp.height = scrtmp.texture.vis_height;
		scrtmp.pitch = (scrtmp.texture.vis_width <<
				(1 << scrtmp.texture.u32));
	}
	scrtmp.is_opengl = scrtmp.want_opengl;
	DEBUG(("OpenGL screen configuration: is_opengl=%u buf.u32=%p pitch=%u",
	       scrtmp.is_opengl, (void *)scrtmp.buf.u32, scrtmp.pitch));
#endif
	// Screen is now initialized, update data.
	screen = scrtmp;
#ifdef WITH_OPENGL
	if (!screen.is_opengl) {
		// Free OpenGL resources.
		DEBUG(("releasing OpenGL resources"));
		release_texture(screen.texture);
	}
#endif
	// Set up the Mega Drive screen.
	// Could not be done earlier because bpp was unknown.
	if ((mdscr.data == NULL) ||
	    ((unsigned int)mdscr.bpp != screen.bpp) ||
	    ((unsigned int)mdscr.w != (video.width + 16)) ||
	    ((unsigned int)mdscr.h != (video.height + 16))) {
		mdscr.w = (video.width + 16);
		mdscr.h = (video.height + 16);
		mdscr.pitch = (mdscr.w * screen.Bpp);
		mdscr.bpp = screen.bpp;
		free(mdscr.data);
		mdscr.data = (uint8_t *)calloc(mdscr.h, mdscr.pitch);
		if (mdscr.data == NULL) {
			// Cannot recover. Clean up and bail out.
			memset(&mdscr, 0, sizeof(mdscr));
			return -2;
		}
		mdscr_splash();
	}
	DEBUG(("md screen configuration: w=%d h=%d bpp=%d pitch=%d data=%p",
	       mdscr.w, mdscr.h, mdscr.bpp, mdscr.pitch, (void *)mdscr.data));
	// If we're in 8 bit mode, set color 0xff to white for the text,
	// and make a palette buffer.
	if (screen.bpp == 8) {
		SDL_Color color = { 0xff, 0xff, 0xff, 0x00 };
		SDL_SetColors(screen.surface, &color, 0xff, 1);
		memset(video.palette, 0x00, sizeof(video.palette));
		mdpal = video.palette;
	}
	else
		mdpal = NULL;
#ifdef WITH_THREADS
	if (screen.want_thread)
		screen_update_thread_start();
#endif
	// Rehash filters.
	filters_stack_update();
	// Update screen.
	pd_graphics_update(true);
	return 0;
}

/**
 * Set fullscreen mode.
 * @param toggle Nonzero to enable fullscreen, otherwise disable it.
 * @return 0 on success.
 */
static int set_fullscreen(int toggle)
{
	unsigned int w;
	unsigned int h;

	if (((!toggle) && (!screen.is_fullscreen)) ||
	    ((toggle) && (screen.is_fullscreen))) {
		// Already in the desired mode.
		DEBUG(("already %s fullscreen mode, ret=-1",
		       (toggle ? "in" : "not in")));
		return -1;
	}
#ifdef HAVE_SDL_WM_TOGGLEFULLSCREEN
	// Try this first.
	DEBUG(("trying SDL_WM_ToggleFullScreen(%p)", (void *)screen.surface));
	if (SDL_WM_ToggleFullScreen(screen.surface))
		return 0;
	DEBUG(("falling back to screen_init()"));
#endif
	dgen_fullscreen = toggle;
	if (screen.surface != NULL) {
		// Try to keep the current mode.
		w = screen.surface->w;
		h = screen.surface->h;
	}
	else if ((dgen_width > 0) && (dgen_height > 0)) {
		// Use configured mode.
		w = dgen_width;
		h = dgen_height;
	}
	else {
		// Try to make a guess.
		w = (video.width * screen.x_scale);
		h = (video.height * screen.y_scale);
	}
	DEBUG(("reinitializing screen with want_fullscreen=%u,"
	       " screen_init(%u, %u)",
	       screen.want_fullscreen, w, h));
	return screen_init(w, h);
}

/**
 * Initialize SDL, and the graphics.
 * @param want_sound Nonzero if we want sound.
 * @param want_pal Nonzero for PAL mode.
 * @param hz Requested frame rate (between 0 and 1000).
 * @return Nonzero if successful.
 */
int pd_graphics_init(int want_sound, int want_pal, int hz)
{
	SDL_Event event;

	prompt_init(&prompt.status);
	if ((hz <= 0) || (hz > 1000)) {
		// You may as well disable bool_frameskip.
		fprintf(stderr, "sdl: invalid frame rate (%d)\n", hz);
		return 0;
	}
	video.hz = hz;
	if (want_pal) {
		// PAL
		video.is_pal = 1;
		video.height = 240;
	}
	else {
		// NTSC
		video.is_pal = 0;
		video.height = 224;
	}
#if !defined __MINGW32__ && !defined _KOLIBRI 
	// [fbcon workaround]
	// Disable SDL_FBACCEL (if unset) before calling SDL_Init() in case
	// fbcon is to be used. Prevents SDL_FillRect() and SDL_SetVideoMode()
	// from hanging when hardware acceleration is available.
	setenv("SDL_FBACCEL", "0", 0);
	// [fbcon workaround]
	// A mouse is never required.
	setenv("SDL_NOMOUSE", "1", 0);
#endif
	if (SDL_Init(SDL_INIT_VIDEO | (want_sound ? SDL_INIT_AUDIO : 0))) {
		fprintf(stderr, "sdl: can't init SDL: %s\n", SDL_GetError());
		return 0;
	}
#ifndef __MINGW32__
	{
		char buf[32];

		// [fbcon workaround]
		// Double buffering usually makes screen blink during refresh.
		if ((SDL_VideoDriverName(buf, sizeof(buf))) &&
		    (!strcmp(buf, "fbcon")))
			dgen_doublebuffer = 0;
	}
#endif
	// Required for text input.
	SDL_EnableUNICODE(1);
	// Set the titlebar.
	SDL_WM_SetCaption("DGen/SDL " VER, "DGen/SDL " VER);
	// Hide the cursor.
	SDL_ShowCursor(0);
	// Initialize screen.
	if (screen_init(0, 0))
		goto fail;
	// Initialize scaling.
	set_scaling(scaling_names[dgen_scaling % NUM_SCALING]);
	DEBUG(("using scaling filter \"%s\"",
	       scaling_names[dgen_scaling % NUM_SCALING]));
	DEBUG(("screen initialized"));
#if !defined __MINGW32__  && !defined _KOLIBRI
	// We don't need setuid privileges anymore
	if (getuid() != geteuid())
		setuid(getuid());
	DEBUG(("setuid privileges dropped"));
#endif
#ifdef WITH_CTV
	filters_pluck_ctv();
	{
		const struct filter *f;

		f = filters_find(ctv_names[dgen_craptv % NUM_CTV]);
		if ((f != NULL) && (f->func != filter_off))
			filters_insert(f);
	}
#endif // WITH_CTV
	DEBUG(("ret=1"));
	fprintf(stderr, "video: %dx%d, %u bpp (%u Bpp), %uHz\n",
		screen.surface->w, screen.surface->h, screen.bpp,
		screen.Bpp, video.hz);
#ifdef WITH_OPENGL
	if (screen.is_opengl) {
		DEBUG(("GL_VENDOR=\"%s\" GL_RENDERER=\"%s\""
		       " GL_VERSION=\"%s\"",
		       glGetString(GL_VENDOR), glGetString(GL_RENDERER),
		       glGetString(GL_VERSION)));
		fprintf(stderr,
			"video: OpenGL texture %ux%ux%u (%ux%u)\n",
			screen.texture.width,
			screen.texture.height,
			(2 << screen.texture.u32),
			screen.texture.vis_width,
			screen.texture.vis_height);
	}
#endif
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_VIDEORESIZE:
			if (screen_init(event.resize.w, event.resize.h))
				goto fail;
			break;
		}
	}
	return 1;
fail:
	fprintf(stderr, "sdl: can't initialize graphics.\n");
	return 0;
}

/**
 * Reinitialize graphics.
 * @param want_pal Nonzero for PAL mode.
 * @param hz Requested frame rate (between 0 and 1000).
 * @return Nonzero if successful.
 */
int pd_graphics_reinit(int, int want_pal, int hz)
{
	if ((hz <= 0) || (hz > 1000)) {
		// You may as well disable bool_frameskip.
		fprintf(stderr, "sdl: invalid frame rate (%d)\n", hz);
		return 0;
	}
	video.hz = hz;
	if (want_pal) {
		// PAL
		video.is_pal = 1;
		video.height = 240;
	}
	else {
		// NTSC
		video.is_pal = 0;
		video.height = 224;
	}
	// Reinitialize screen.
	if (screen_init(screen.window_width, screen.window_height))
		goto fail;
	
	DEBUG(("screen reinitialized"));
	return 1;
fail:
	fprintf(stderr, "sdl: can't reinitialize graphics.\n");
	return 0;
}

/**
 * Update palette.
 */
void pd_graphics_palette_update()
{
	unsigned int i;

	for (i = 0; (i < 64); ++i) {
		screen.color[i].r = mdpal[(i << 2)];
		screen.color[i].g = mdpal[((i << 2) + 1)];
		screen.color[i].b = mdpal[((i << 2) + 2)];
	}
#ifdef WITH_OPENGL
	if (!screen.is_opengl)
#endif
		SDL_SetColors(screen.surface, screen.color, 0, 64);
}

/**
 * Display screen.
 * @param update False if screen buffer is garbage and must be updated first.
 */
void pd_graphics_update(bool update)
{
	static unsigned long fps_since = 0;
	static unsigned long frames_old = 0;
	static unsigned long frames = 0;
	unsigned long usecs = pd_usecs();
	const struct filter *f;
	struct filter_data *fd;
	size_t i;

	// Check whether the message must be processed.
	if ((events == STARTED) &&
	    ((info.displayed) || (info.length))  &&
	    ((usecs - info.since) >= MESSAGE_LIFE))
		pd_message_process();
	else if (dgen_fps) {
		unsigned long tmp = ((usecs - fps_since) & 0x3fffff);

		++frames;
		if (tmp >= 1000000) {
			unsigned long fps;

			fps_since = usecs;
			if (frames_old > frames)
				fps = (frames_old - frames);
			else
				fps = (frames - frames_old);
			frames_old = frames;
			if (!info.displayed) {
				char buf[16];

				snprintf(buf, sizeof(buf), "%lu FPS", fps);
				pd_message_write(buf, strlen(buf), ~0u);
			}
		}
	}
	if (update == false)
		mdscr_splash();
	// Process output through filters.
	for (i = 0; (i != elemof(filters_stack)); ++i) {
		f = filters_stack[i];
		fd = &filters_stack_data[i];
		if ((filters_stack_size == 0) ||
		    (i == (filters_stack_size - 1)))
			break;
		f->func(fd, (fd + 1));
	}
	// Lock screen.
	if (screen_lock())
		return;
	// Generate screen output with the last filter.
	f->func(fd, (fd + 1));
	// Unlock screen.
	screen_unlock();
	// Update the screen.
	screen_update();
}

/**
 * Callback for sound.
 * @param stream Sound destination buffer.
 * @param len Length of destination buffer.
 */
static void snd_callback(void *, Uint8 *stream, int len)
{
	size_t wrote;

	// Slurp off the play buffer
	wrote = cbuf_read(stream, &sound.cbuf, len);
	if (wrote == (size_t)len)
		return;
	// Not enough data, fill remaining space with silence.
	memset(&stream[wrote], 0, ((size_t)len - wrote));
}

/**
 * Initialize the sound.
 * @param freq Sound samples rate.
 * @param[in,out] samples Minimum buffer size in samples.
 * @return Nonzero on success.
 */
int pd_sound_init(long &freq, unsigned int &samples)
{
	SDL_AudioSpec wanted;
	SDL_AudioSpec spec;

	// Clean up first.
	pd_sound_deinit();

	// Set the desired format
	wanted.freq = freq;
#ifdef WORDS_BIGENDIAN
	wanted.format = AUDIO_S16MSB;
#else
	wanted.format = AUDIO_S16LSB;
#endif
	wanted.channels = 2;
	wanted.samples = dgen_soundsamples;
	wanted.callback = snd_callback;
	wanted.userdata = NULL;

	if (SDL_InitSubSystem(SDL_INIT_AUDIO)) {
		fprintf(stderr, "sdl: unable to initialize audio\n");
		return 0;
	}

	// Open audio, and get the real spec
	if (SDL_OpenAudio(&wanted, &spec) < 0) {
		fprintf(stderr,
			"sdl: couldn't open audio: %s\n",
			SDL_GetError());
		return 0;
	}

	// Check everything
	if (spec.channels != 2) {
		fprintf(stderr, "sdl: couldn't get stereo audio format.\n");
		goto snd_error;
	}
	if (spec.format != wanted.format) {
		fprintf(stderr, "sdl: unable to get 16-bit audio.\n");
		goto snd_error;
	}

	// Set things as they really are
	sound.rate = freq = spec.freq;
	sndi.len = (spec.freq / video.hz);
	sound.samples = spec.samples;
	samples += sound.samples;

	// Calculate buffer size (sample size = (channels * (bits / 8))).
	sound.cbuf.size = (samples * (2 * (16 / 8)));
	sound.cbuf.i = 0;
	sound.cbuf.s = 0;

	fprintf(stderr, "sound: %uHz, %d samples, buffer: %u bytes\n",
		sound.rate, spec.samples, (unsigned int)sound.cbuf.size);

	// Allocate zero-filled play buffer.
	sndi.lr = (int16_t *)calloc(2, (sndi.len * sizeof(sndi.lr[0])));

	sound.cbuf.data.i16 = (int16_t *)calloc(1, sound.cbuf.size);
	if ((sndi.lr == NULL) || (sound.cbuf.data.i16 == NULL)) {
		fprintf(stderr, "sdl: couldn't allocate sound buffers.\n");
		goto snd_error;
	}

	// Start sound output.
	SDL_PauseAudio(0);

	// It's all good!
	return 1;

snd_error:
	// Oops! Something bad happened, cleanup.
	SDL_CloseAudio();
	free((void *)sndi.lr);
	sndi.lr = NULL;
	sndi.len = 0;
	free((void *)sound.cbuf.data.i16);
	sound.cbuf.data.i16 = NULL;
	memset(&sound, 0, sizeof(sound));
	return 0;
}

/**
 * Deinitialize sound subsystem.
 */
void pd_sound_deinit()
{
	if (sound.cbuf.data.i16 != NULL) {
		SDL_PauseAudio(1);
		SDL_CloseAudio();
		free((void *)sound.cbuf.data.i16);
	}
	memset(&sound, 0, sizeof(sound));
	free((void*)sndi.lr);
	sndi.lr = NULL;
}

/**
 * Return samples read/write indices in the buffer.
 */
unsigned int pd_sound_rp()
{
	unsigned int ret;

	if (!sound.cbuf.size)
		return 0;
	SDL_LockAudio();
	ret = sound.cbuf.i;
	SDL_UnlockAudio();
	return (ret >> 2);
}

unsigned int pd_sound_wp()
{
	unsigned int ret;

	if (!sound.cbuf.size)
		return 0;
	SDL_LockAudio();
	ret = ((sound.cbuf.i + sound.cbuf.s) % sound.cbuf.size);
	SDL_UnlockAudio();
	return (ret >> 2);
}

/**
 * Write contents of sndi to sound.cbuf.
 */
void pd_sound_write()
{
	if (!sound.cbuf.size)
		return;
	SDL_LockAudio();
	cbuf_write(&sound.cbuf, (uint8_t *)sndi.lr, (sndi.len * 4));
	SDL_UnlockAudio();
}

/**
 * Tells whether DGen stopped intentionally so emulation can resume without
 * skipping frames.
 */
int pd_stopped()
{
	int ret = stopped;

	stopped = 0;
	return ret;
}

/**
 * Keyboard input.
 */
typedef struct {
	char *buf;
	size_t pos;
	size_t size;
} kb_input_t;

/**
 * Keyboard input results.
 */
enum kb_input {
	KB_INPUT_ABORTED,
	KB_INPUT_ENTERED,
	KB_INPUT_CONSUMED,
	KB_INPUT_IGNORED
};

/**
 * Manage text input with some rudimentary history.
 * @param input Input buffer.
 * @param ksym Keyboard symbol.
 * @param ksym_uni Unicode translation for keyboard symbol.
 * @return Input result.
 */
static enum kb_input kb_input(kb_input_t *input, uint32_t ksym,
			      uint16_t ksym_uni)
{
#define HISTORY_LEN 32
	static char history[HISTORY_LEN][64];
	static int history_pos = -1;
	static int history_len = 0;
	char c;

	if (ksym & KEYSYM_MOD_CTRL)
		return KB_INPUT_IGNORED;
	if (isprint((c = ksym_uni))) {
		if (input->pos >= (input->size - 1))
			return KB_INPUT_CONSUMED;
		if (input->buf[input->pos] == '\0')
			input->buf[(input->pos + 1)] = '\0';
		input->buf[input->pos] = c;
		++input->pos;
		return KB_INPUT_CONSUMED;
	}
	else if (ksym == SDLK_DELETE) {
		size_t tail;

		if (input->buf[input->pos] == '\0')
			return KB_INPUT_CONSUMED;
		tail = ((input->size - input->pos) + 1);
		memmove(&input->buf[input->pos],
			&input->buf[(input->pos + 1)],
			tail);
		return KB_INPUT_CONSUMED;
	}
	else if (ksym == SDLK_BACKSPACE) {
		size_t tail;

		if (input->pos == 0)
			return KB_INPUT_CONSUMED;
		--input->pos;
		tail = ((input->size - input->pos) + 1);
		memmove(&input->buf[input->pos],
			&input->buf[(input->pos + 1)],
			tail);
		return KB_INPUT_CONSUMED;
	}
	else if (ksym == SDLK_LEFT) {
		if (input->pos != 0)
			--input->pos;
		return KB_INPUT_CONSUMED;
	}
	else if (ksym == SDLK_RIGHT) {
		if (input->buf[input->pos] != '\0')
			++input->pos;
		return KB_INPUT_CONSUMED;
	}
	else if ((ksym == SDLK_RETURN) || (ksym == SDLK_KP_ENTER)) {
		history_pos = -1;
		if (input->pos == 0)
			return KB_INPUT_ABORTED;
		if (history_len < HISTORY_LEN)
			++history_len;
		memmove(&history[1], &history[0],
			((history_len - 1) * sizeof(history[0])));
		strncpy(history[0], input->buf, sizeof(history[0]));
		return KB_INPUT_ENTERED;
	}
	else if (ksym == SDLK_ESCAPE) {
		history_pos = 0;
		return KB_INPUT_ABORTED;
	}
	else if (ksym == SDLK_UP) {
		if (input->size == 0)
			return KB_INPUT_CONSUMED;
		if (history_pos < (history_len - 1))
			++history_pos;
		strncpy(input->buf, history[history_pos], input->size);
		input->buf[(input->size - 1)] = '\0';
		input->pos = strlen(input->buf);
		return KB_INPUT_CONSUMED;
	}
	else if (ksym == SDLK_DOWN) {
		if ((input->size == 0) || (history_pos < 0))
			return KB_INPUT_CONSUMED;
		if (history_pos > 0)
			--history_pos;
		strncpy(input->buf, history[history_pos], input->size);
		input->buf[(input->size - 1)] = '\0';
		input->pos = strlen(input->buf);
		return KB_INPUT_CONSUMED;
	}
	return KB_INPUT_IGNORED;
}

/**
 * Write a message to the status bar while displaying a cursor and without
 * buffering.
 */
static void pd_message_cursor(unsigned int mark, const char *msg, ...)
{
	va_list vl;
	char buf[1024];
	size_t len;
	size_t disp_len;

	va_start(vl, msg);
	len = (size_t)vsnprintf(buf, sizeof(buf), msg, vl);
	va_end(vl);
	buf[(sizeof(buf) - 1)] = '\0';
	disp_len = font_text_max_len(screen.width, screen.info_height,
				     FONT_TYPE_AUTO);
	if (mark > len) {
		if (len <= disp_len)
			pd_message_display(buf, len, ~0u, true);
		else
			pd_message_display(&(buf[(len - disp_len)]), disp_len,
					   ~0u, true);
		return;
	}
	if (len <= disp_len)
		pd_message_display(buf, len, mark, true);
	else if (len == mark)
		pd_message_display(&buf[((len - disp_len) + 1)],
				   disp_len, (disp_len - 1), true);
	else if ((len - mark) < disp_len)
		pd_message_display(&buf[(len - disp_len)], disp_len,
				   (mark - (len - disp_len)), true);
	else if (mark != ~0u)
		pd_message_display(&buf[mark], disp_len, 0, true);
}

/**
 * Rehash rc vars that require special handling (see "SH" in rc.cpp).
 */
static int prompt_rehash_rc_field(const struct rc_field *rc, md& megad)
{
	bool fail = false;
	bool init_video = false;
	bool init_sound = false;
	bool init_joystick = false;

	if (rc->variable == &dgen_craptv) {
#ifdef WITH_CTV
		filters_pluck_ctv();
		filters_insert(filters_find(ctv_names[dgen_craptv % NUM_CTV]));
#else
		fail = true;
#endif
	}
	else if (rc->variable == &dgen_scaling) {
		if (set_scaling(scaling_names[dgen_scaling]) == 0)
			fail = true;
	}
	else if (rc->variable == &dgen_emu_z80) {
		megad.z80_state_dump();
		// Z80: 0 = none, 1 = CZ80, 2 = MZ80, 3 = DRZ80
		switch (dgen_emu_z80) {
#ifdef WITH_MZ80
		case 1:
			megad.z80_core = md::Z80_CORE_MZ80;
			break;
#endif
#ifdef WITH_CZ80
		case 2:
			megad.z80_core = md::Z80_CORE_CZ80;
			break;
#endif
#ifdef WITH_DRZ80
		case 3:
			megad.z80_core = md::Z80_CORE_DRZ80;
			break;
#endif
		default:
			megad.z80_core = md::Z80_CORE_NONE;
			break;
		}
		megad.z80_state_restore();
	}
	else if (rc->variable == &dgen_emu_m68k) {
		megad.m68k_state_dump();
		// M68K: 0 = none, 1 = StarScream, 2 = Musashi, 3 = Cyclone
		switch (dgen_emu_m68k) {
#ifdef WITH_STAR
		case 1:
			megad.cpu_emu = md::CPU_EMU_STAR;
			break;
#endif
#ifdef WITH_MUSA
		case 2:
			megad.cpu_emu = md::CPU_EMU_MUSA;
			break;
#endif
#ifdef WITH_CYCLONE
		case 3:
			megad.cpu_emu = md::CPU_EMU_CYCLONE;
			break;
#endif
		default:
			megad.cpu_emu = md::CPU_EMU_NONE;
			break;
		}
		megad.m68k_state_restore();
	}
	else if ((rc->variable == &dgen_sound) ||
		 (rc->variable == &dgen_soundrate) ||
		 (rc->variable == &dgen_soundsegs) ||
		 (rc->variable == &dgen_soundsamples) ||
		 (rc->variable == &dgen_mjazz))
		init_sound = true;
	else if (rc->variable == &dgen_fullscreen) {
		if (screen.want_fullscreen != (!!dgen_fullscreen)) {
			init_video = true;
		}
	}
	else if ((rc->variable == &dgen_info_height) ||
		 (rc->variable == &dgen_width) ||
		 (rc->variable == &dgen_height) ||
		 (rc->variable == &dgen_x_scale) ||
		 (rc->variable == &dgen_y_scale) ||
		 (rc->variable == &dgen_depth) ||
		 (rc->variable == &dgen_doublebuffer) ||
		 (rc->variable == &dgen_screen_thread))
		init_video = true;
	else if (rc->variable == &dgen_swab) {
#ifdef WITH_CTV
		set_swab();
#else
		fail = true;
#endif
	}
	else if ((rc->variable == &dgen_scale) ||
		 (rc->variable == &dgen_aspect)) {
		dgen_x_scale = dgen_scale;
		dgen_y_scale = dgen_scale;
		init_video = true;
	}
	else if ((rc->variable == &dgen_opengl) ||
		 (rc->variable == &dgen_opengl_stretch) ||
		 (rc->variable == &dgen_opengl_linear) ||
		 (rc->variable == &dgen_opengl_32bit) ||
		 (rc->variable == &dgen_opengl_square)) {
#ifdef WITH_OPENGL
		init_video = true;
#else
		(void)0;
#endif
	}
	else if (rc->variable == &dgen_joystick)
		init_joystick = true;
	else if (rc->variable == &dgen_hz) {
		// See md::md().
		if (dgen_hz <= 0)
			dgen_hz = 1;
		else if (dgen_hz > 1000)
			dgen_hz = 1000;
		if (((unsigned int)dgen_hz != video.hz) ||
		    ((unsigned int)dgen_hz != megad.vhz)) {
			video.hz = dgen_hz;
			init_video = true;
			init_sound = true;
		}
	}
	else if (rc->variable == &dgen_pal) {
		// See md::md().
		if ((dgen_pal) &&
		    ((video.is_pal == false) ||
		     (megad.pal == 0) ||
		     (video.height != PAL_VBLANK))) {
			megad.pal = 1;
			megad.init_pal();
			video.is_pal = true;
			video.height = PAL_VBLANK;
			init_video = true;
		}
		else if ((!dgen_pal) &&
			 ((video.is_pal == true) ||
			  (megad.pal == 1) ||
			  (video.height != NTSC_VBLANK))) {
			megad.pal = 0;
			megad.init_pal();
			video.is_pal = false;
			video.height = NTSC_VBLANK;
			init_video = true;
		}
	}
	else if (rc->variable == &dgen_region) {
		uint8_t c;
		int hz;
		int pal;
		int vblank;

		if (dgen_region)
			c = dgen_region;
		else
			c = megad.region_guess();
		md::region_info(c, &pal, &hz, &vblank, 0, 0);
		if ((hz != dgen_hz) || (pal != dgen_pal) ||
		    (c != megad.region)) {
			megad.region = c;
			dgen_hz = hz;
			dgen_pal = pal;
			megad.pal = pal;
			megad.init_pal();
			video.is_pal = pal;
			video.height = vblank;
			video.hz = hz;
			init_video = true;
			init_sound = true;
			fprintf(stderr,
				"sdl: reconfiguring for region \"%c\": "
				"%dHz (%s)\n", megad.region, hz,
				(pal ? "PAL" : "NTSC"));
		}
	}
	else if (rc->variable == (intptr_t *)((void *)&dgen_rom_path))
		set_rom_path(dgen_rom_path.val);
	if (init_video) {
		// This is essentially what pd_graphics_init() does.
		memset(megad.vdp.dirt, 0xff, 0x35);
		switch (screen_init(screen.window_width,
				    screen.window_height)) {
		case 0:
			break;
		case -1:
			goto video_warn;
		default:
			goto video_fail;
		}
	}
	if (init_sound) {
		if (video.hz == 0)
			fail = true;
		else if (dgen_sound == 0)
			pd_sound_deinit();
		else {
			uint8_t ym2612_buf[512];
			uint8_t sn76496_buf[16];
			unsigned int samples;
			long rate = dgen_soundrate;

			pd_sound_deinit();
			samples = (dgen_soundsegs * (rate / video.hz));
			if (!pd_sound_init(rate, samples))
				fail = true;
			YM2612_dump(0, ym2612_buf);
			SN76496_dump(0, sn76496_buf);
			megad.init_sound();
			SN76496_restore(0, sn76496_buf);
			YM2612_restore(0, ym2612_buf);
		}
	}
	if (init_joystick) {
#ifdef WITH_JOYSTICK
		megad.deinit_joysticks();
		if (dgen_joystick)
			megad.init_joysticks();
#else
		fail = true;
#endif
	}
	if (fail) {
		pd_message("Failed to rehash value.");
		return (PROMPT_RET_EXIT | PROMPT_RET_MSG);
	}
	return PROMPT_RET_CONT;
video_warn:
	pd_message("Failed to reinitialize video.");
	return (PROMPT_RET_EXIT | PROMPT_RET_MSG);
video_fail:
	fprintf(stderr, "sdl: fatal error while trying to change screen"
		" resolution.\n");
	return (PROMPT_RET_ERROR | PROMPT_RET_MSG);
}

static void prompt_show_rc_field(const struct rc_field *rc)
{
	size_t i;
	intptr_t val = *rc->variable;

	if ((rc->parser == rc_number) ||
	    (rc->parser == rc_soundrate))
		pd_message("%s is %ld", rc->fieldname, val);
	else if (rc->parser == rc_keysym) {
		char *ks = dump_keysym(val);

		if ((ks == NULL) || (ks[0] == '\0'))
			pd_message("%s isn't bound", rc->fieldname);
		else
			pd_message("%s is bound to \"%s\"", rc->fieldname, ks);
		free(ks);
	}
	else if (rc->parser == rc_boolean)
		pd_message("%s is %s", rc->fieldname,
			   ((val) ? "true" : "false"));
	else if (rc->parser == rc_joypad) {
		char *js = dump_joypad(val);

		if ((js == NULL) || (js[0] == '\0'))
			pd_message("%s isn't bound", rc->fieldname);
		else
			pd_message("%s is bound to \"%s\"", rc->fieldname, js);
		free(js);
	}
	else if (rc->parser == rc_mouse) {
		char *mo = dump_mouse(val);

		if ((mo == NULL) || (mo[0] == '\0'))
			pd_message("%s isn't bound", rc->fieldname);
		else
			pd_message("%s is bound to \"%s\"", rc->fieldname, mo);
		free(mo);
	}
	else if (rc->parser == rc_ctv) {
		i = val;
		if (i >= NUM_CTV)
			pd_message("%s is undefined", rc->fieldname);
		else
			pd_message("%s is \"%s\"", rc->fieldname,
				   ctv_names[i]);
	}
	else if (rc->parser == rc_scaling) {
		i = val;
		if (i >= NUM_SCALING)
			pd_message("%s is undefined", rc->fieldname);
		else
			pd_message("%s is \"%s\"", rc->fieldname,
				   scaling_names[i]);
	}
	else if (rc->parser == rc_emu_z80) {
		for (i = 0; (emu_z80_names[i] != NULL); ++i)
			if (i == (size_t)val)
				break;
		if (emu_z80_names[i] == NULL)
			pd_message("%s is undefined", rc->fieldname);
		else
			pd_message("%s is \"%s\"", rc->fieldname,
				   emu_z80_names[i]);
	}
	else if (rc->parser == rc_emu_m68k) {
		for (i = 0; (emu_m68k_names[i] != NULL); ++i)
			if (i == (size_t)val)
				break;
		if (emu_m68k_names[i] == NULL)
			pd_message("%s is undefined", rc->fieldname);
		else
			pd_message("%s is \"%s\"", rc->fieldname,
				   emu_m68k_names[i]);
	}
	else if (rc->parser == rc_region) {
		const char *s;

		if (val == 'U')
			s = "America (NTSC)";
		else if (val == 'E')
			s = "Europe (PAL)";
		else if (val == 'J')
			s = "Japan (NTSC)";
		else if (val == 'X')
			s = "Japan (PAL)";
		else
			s = "Auto";
		pd_message("%s is \"%c\" (%s)", rc->fieldname,
			   (val ? (char)val : (char)' '), s);
	}
	else if ((rc->parser == rc_string) ||
		 (rc->parser == rc_rom_path)) {
		struct rc_str *rs = (struct rc_str *)rc->variable;
		char *s;

		if (rs->val == NULL)
			pd_message("%s has no value", rc->fieldname);
		else if ((s = backslashify((const uint8_t *)rs->val,
					   strlen(rs->val), 0,
					   NULL)) != NULL) {
			pd_message("%s is \"%s\"", rc->fieldname, s);
			free(s);
		}
		else
			pd_message("%s can't be displayed", rc->fieldname);
	}
	else if (rc->parser == rc_bind) {
		char *f = backslashify((uint8_t *)rc->fieldname,
				       strlen(rc->fieldname), 0, NULL);
		char *s = *(char **)rc->variable;

		assert(s != NULL);
		assert((intptr_t)s != -1);
		s = backslashify((uint8_t *)s, strlen(s), 0, NULL);
		if ((f == NULL) || (s == NULL))
			pd_message("%s can't be displayed", rc->fieldname);
		else
			pd_message("%s is bound to \"%s\"", f, s);
		free(f);
		free(s);
	}
	else
		pd_message("%s: can't display value", rc->fieldname);
}

static int handle_prompt_enter(class md& md)
{
	struct prompt_parse pp;
	struct prompt *p = &prompt.status;
	size_t i;
	int ret;
	bool binding_tried = false;

	if (prompt_parse(p, &pp) == NULL)
		return PROMPT_RET_ERROR;
	if (pp.argc == 0) {
		ret = PROMPT_RET_EXIT;
		goto end;
	}
	ret = 0;
	// Look for a command with that name.
	for (i = 0; (prompt_command[i].name != NULL); ++i) {
		int cret;

		if (strcasecmp(prompt_command[i].name, (char *)pp.argv[0]))
			continue;
		cret = prompt_command[i].cmd(md, pp.argc,
					     (const char **)pp.argv);
		if ((cret & ~CMD_MSG) == CMD_ERROR)
			ret |= PROMPT_RET_ERROR;
		if (cret & CMD_MSG)
			ret |= PROMPT_RET_MSG;
		else if (cret & CMD_FAIL) {
			pd_message("%s: command failed", (char *)pp.argv[0]);
			ret |= PROMPT_RET_MSG;
		}
		else if (cret & CMD_EINVAL) {
			pd_message("%s: invalid argument", (char *)pp.argv[0]);
			ret |= PROMPT_RET_MSG;
		}
		goto end;
	}
binding_retry:
	// Look for a variable with that name.
	for (i = 0; (rc_fields[i].fieldname != NULL); ++i) {
		intptr_t potential;

		if (strcasecmp(rc_fields[i].fieldname, (char *)pp.argv[0]))
			continue;
		// Display current value?
		if (pp.argv[1] == NULL) {
			prompt_show_rc_field(&rc_fields[i]);
			ret |= PROMPT_RET_MSG;
			break;
		}
		// Parse and set value.
		potential = rc_fields[i].parser((char *)pp.argv[1],
						rc_fields[i].variable);
		if ((rc_fields[i].parser != rc_number) && (potential == -1)) {
			pd_message("%s: invalid value",	(char *)pp.argv[0]);
			ret |= PROMPT_RET_MSG;
			break;
		}
		if ((rc_fields[i].parser == rc_string) ||
		    (rc_fields[i].parser == rc_rom_path)) {
			struct rc_str *rs;

			rs = (struct rc_str *)rc_fields[i].variable;
			if (rc_str_list == NULL) {
				atexit(rc_str_cleanup);
				rc_str_list = rs;
			}
			else if (rs->alloc == NULL) {
				rs->next = rc_str_list;
				rc_str_list = rs;
			}
			else
				free(rs->alloc);
			rs->alloc = (char *)potential;
			rs->val = rs->alloc;
		}
		else
			*(rc_fields[i].variable) = potential;
		ret |= prompt_rehash_rc_field(&rc_fields[i], md);
		break;
	}
	if (rc_fields[i].fieldname == NULL) {
		if ((binding_tried == false) &&
		    (rc_binding_add((char *)pp.argv[0], "") != NULL)) {
			binding_tried = true;
			goto binding_retry;
		}
		pd_message("%s: unknown command", (char *)pp.argv[0]);
		ret |= PROMPT_RET_MSG;
	}
end:
	prompt_parse_clean(&pp);
	prompt_push(p);
	ret |= PROMPT_RET_ENTER;
	return ret;
}

static void handle_prompt_complete_clear()
{
	complete_path_free(prompt.complete);
	prompt.complete = NULL;
	prompt.skip = 0;
	prompt.common = 0;
}

static int handle_prompt_complete(class md& md, bool rwd)
{
	struct prompt_parse pp;
	struct prompt *p = &prompt.status;
	size_t prompt_common = 0; // escaped version of prompt.common
	unsigned int skip;
	size_t i;
	const char *arg;
	unsigned int alen;
	char *s = NULL;

	if (prompt_parse(p, &pp) == NULL)
		return PROMPT_RET_ERROR;
	if (rwd)
		prompt.skip -= 2;
	if (pp.index == 0) {
		const char *cs = NULL;
		const char *cm = NULL;
		size_t common;
		unsigned int tmp;

		assert(prompt.complete == NULL);
		// The first argument needs to be completed. This is either
		// a command or a variable name.
		arg = (const char *)pp.argv[0];
		alen = pp.cursor;
		if ((arg == NULL) || (alen == ~0u)) {
			arg = "";
			alen = 0;
		}
		common = ~0u;
	complete_cmd_var:
		skip = prompt.skip;
		for (i = 0; (prompt_command[i].name != NULL); ++i) {
			if (strncasecmp(prompt_command[i].name, arg, alen))
				continue;
			if (cm == NULL)
				tmp = strlen(prompt_command[i].name);
			else
				tmp = strcommon(prompt_command[i].name, cm);
			cm = prompt_command[i].name;
			if (tmp < common)
				common = tmp;
			if (skip != 0) {
				--skip;
				continue;
			}
			if (cs == NULL)
				cs = prompt_command[i].name;
			if (common == 0)
				goto complete_cmd_found;
		}
		// Variables.
		for (i = 0; (rc_fields[i].fieldname != NULL); ++i) {
			if (strncasecmp(rc_fields[i].fieldname, arg, alen))
				continue;
			if (cm == NULL)
				tmp = strlen(rc_fields[i].fieldname);
			else
				tmp = strcommon(rc_fields[i].fieldname, cm);
			cm = rc_fields[i].fieldname;
			if (tmp < common)
				common = tmp;
			if (skip != 0) {
				--skip;
				continue;
			}
			if (cs == NULL)
				cs = rc_fields[i].fieldname;
			if (common == 0)
				break;
		}
		if (cs == NULL) {
			// Nothing matched, try again if possible.
			if (prompt.skip) {
				prompt.skip = 0;
				goto complete_cmd_var;
			}
			goto end;
		}
	complete_cmd_found:
		++prompt.skip;
		s = backslashify((const uint8_t *)cs, strlen(cs), 0, &common);
		if (s == NULL)
			goto end;
		if (common != ~0u) {
			prompt.common = common;
			prompt_common = common;
		}
		goto replace;
	}
	// Complete function arguments.
	for (i = 0; (prompt_command[i].name != NULL); ++i) {
		char *t;

		if (strcasecmp(prompt_command[i].name,
			       (const char *)pp.argv[0]))
			continue;
		if (prompt_command[i].cmpl == NULL)
			goto end;
		t = prompt_command[i].cmpl(md, pp.argc, (const char **)pp.argv,
					   pp.cursor);
		if (t == NULL)
			goto end;
		prompt_common = prompt.common;
		s = backslashify((const uint8_t *)t, strlen(t), 0,
				 &prompt_common);
		free(t);
		if (s == NULL)
			goto end;
		goto replace;
	}
	// Variable value completion.
	arg = (const char *)pp.argv[pp.index];
	alen = pp.cursor;
	if ((arg == NULL) || (alen == ~0u)) {
		arg = "";
		alen = 0;
	}
	for (i = 0; (rc_fields[i].fieldname != NULL); ++i) {
		struct rc_field *rc = &rc_fields[i];
		const char **names;

		if (strcasecmp(rc->fieldname, (const char *)pp.argv[0]))
			continue;
		// Boolean values.
		if (rc->parser == rc_boolean)
			s = strdup((prompt.skip & 1) ? "true" : "false");
		// ROM path.
		else if (rc->parser == rc_rom_path) {
			if (prompt.complete == NULL) {
				prompt.complete =
					complete_path(arg, alen, NULL);
				prompt.skip = 0;
				rehash_prompt_complete_common();
			}
			if (prompt.complete != NULL) {
				char **ret = prompt.complete;

			rc_rom_path_retry:
				skip = prompt.skip;
				for (i = 0; (ret[i] != NULL); ++i) {
					if (skip == 0)
						break;
					--skip;
				}
				if (ret[i] == NULL) {
					if (prompt.skip != 0) {
						prompt.skip = 0;
						goto rc_rom_path_retry;
					}
				}
				else {
					prompt_common = prompt.common;
					s = backslashify
						((const uint8_t *)ret[i],
						 strlen(ret[i]), 0,
						 &prompt_common);
				}
			}
		}
		// Numbers.
		else if ((rc->parser == rc_number) ||
			 (rc->parser == rc_soundrate)) {
			char buf[10];

		rc_number_retry:
			if (snprintf(buf, sizeof(buf), "%d",
				     (int)prompt.skip) >= (int)sizeof(buf)) {
				prompt.skip = 0;
				goto rc_number_retry;
			}
			s = strdup(buf);
		}
		// CTV filters, scaling algorithms, Z80, M68K.
		else if ((names = ctv_names, rc->parser == rc_ctv) ||
			 (names = scaling_names, rc->parser == rc_scaling) ||
			 (names = emu_z80_names, rc->parser == rc_emu_z80) ||
			 (names = emu_m68k_names, rc->parser == rc_emu_m68k)) {
		rc_names_retry:
			skip = prompt.skip;
			for (i = 0; (names[i] != NULL); ++i) {
				if (skip == 0)
					break;
				--skip;
			}
			if (names[i] == NULL) {
				if (prompt.skip != 0) {
					prompt.skip = 0;
					goto rc_names_retry;
				}
			}
			else
				s = strdup(names[i]);
		}
		if (s == NULL)
			break;
		++prompt.skip;
		goto replace;
	}
	goto end;
replace:
	prompt_replace(p, pp.argo[pp.index].pos, pp.argo[pp.index].len,
		       (const uint8_t *)s, strlen(s));
	if (prompt_common) {
		unsigned int cursor;

		cursor = (pp.argo[pp.index].pos + prompt_common);
		if (cursor > p->history[(p->current)].length)
			cursor = p->history[(p->current)].length;
		if (cursor != p->cursor) {
			p->cursor = cursor;
			handle_prompt_complete_clear();
		}
	}
end:
	free(s);
	prompt_parse_clean(&pp);
	return 0;
}

static int handle_prompt(uint32_t ksym, uint16_t ksym_uni, md& megad)
{
	struct prompt::prompt_history *ph;
	size_t sz;
	uint8_t c[6];
	char *s;
	int ret = PROMPT_RET_CONT;
	struct prompt *p = &prompt.status;
	struct prompt_parse pp;

	if (ksym == 0)
		goto end;
	switch (ksym & ~KEYSYM_MOD_MASK) {
	case SDLK_UP:
		handle_prompt_complete_clear();
		prompt_older(p);
		break;
	case SDLK_DOWN:
		handle_prompt_complete_clear();
		prompt_newer(p);
		break;
	case SDLK_LEFT:
		handle_prompt_complete_clear();
		prompt_left(p);
		break;
	case SDLK_RIGHT:
		handle_prompt_complete_clear();
		prompt_right(p);
		break;
	case SDLK_HOME:
		handle_prompt_complete_clear();
		prompt_begin(p);
		break;
	case SDLK_END:
		handle_prompt_complete_clear();
		prompt_end(p);
		break;
	case SDLK_BACKSPACE:
		handle_prompt_complete_clear();
		prompt_backspace(p);
		break;
	case SDLK_DELETE:
		handle_prompt_complete_clear();
		prompt_delete(p);
		break;
	case SDLK_a:
		// ^A
		if ((ksym & KEYSYM_MOD_CTRL) == 0)
			goto other;
		handle_prompt_complete_clear();
		prompt_begin(p);
		break;
	case SDLK_e:
		// ^E
		if ((ksym & KEYSYM_MOD_CTRL) == 0)
			goto other;
		handle_prompt_complete_clear();
		prompt_end(p);
		break;
	case SDLK_u:
		// ^U
		if ((ksym & KEYSYM_MOD_CTRL) == 0)
			goto other;
		handle_prompt_complete_clear();
		prompt_clear(p);
		break;
	case SDLK_k:
		// ^K
		if ((ksym & KEYSYM_MOD_CTRL) == 0)
			goto other;
		handle_prompt_complete_clear();
		prompt_replace(p, p->cursor, ~0u, NULL, 0);
		break;
	case SDLK_w:
		// ^W
		if ((ksym & KEYSYM_MOD_CTRL) == 0)
			goto other;
		if (prompt_parse(p, &pp) == NULL)
			break;
		if (pp.argv[pp.index] == NULL) {
			if (pp.index == 0) {
				prompt_parse_clean(&pp);
				break;
			}
			--pp.index;
		}
		handle_prompt_complete_clear();
		if (pp.argv[(pp.index + 1)] != NULL)
			prompt_replace(p, pp.argo[pp.index].pos,
				       (pp.argo[(pp.index + 1)].pos -
					pp.argo[pp.index].pos),
				       NULL, 0);
		else
			prompt_replace(p, pp.argo[pp.index].pos, ~0u, NULL, 0);
		p->cursor = pp.argo[pp.index].pos;
		prompt_parse_clean(&pp);
		break;
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
		handle_prompt_complete_clear();
		ret |= handle_prompt_enter(megad);
		break;
	case SDLK_ESCAPE:
		handle_prompt_complete_clear();
		ret |= PROMPT_RET_EXIT;
		break;
	case SDLK_TAB:
		if (ksym & KEYSYM_MOD_SHIFT)
			ret |= handle_prompt_complete(megad, true);
		else
			ret |= handle_prompt_complete(megad, false);
		break;
	default:
	other:
		if (ksym_uni == 0)
			break;
		handle_prompt_complete_clear();
		sz = utf32u8(c, ksym_uni);
		if ((sz != 0) &&
		    ((s = backslashify(c, sz, BACKSLASHIFY_NOQUOTES,
				       NULL)) != NULL)) {
			size_t i;

			for (i = 0; (i != strlen(s)); ++i)
				prompt_put(p, s[i]);
			free(s);
		}
		break;
	}
end:
	if ((ret & ~(PROMPT_RET_CONT | PROMPT_RET_ENTER)) == 0) {
		ph = &p->history[(p->current)];
		pd_message_cursor((p->cursor + 1),
				  "%s%.*s", prompt_str, ph->length, ph->line);
	}
	return ret;
}

// Controls enum. You must add new entries at the end. Do not change the order.
enum ctl_e {
	CTL_PAD1_UP,
	CTL_PAD1_DOWN,
	CTL_PAD1_LEFT,
	CTL_PAD1_RIGHT,
	CTL_PAD1_A,
	CTL_PAD1_B,
	CTL_PAD1_C,
	CTL_PAD1_X,
	CTL_PAD1_Y,
	CTL_PAD1_Z,
	CTL_PAD1_MODE,
	CTL_PAD1_START,
	CTL_PAD2_UP,
	CTL_PAD2_DOWN,
	CTL_PAD2_LEFT,
	CTL_PAD2_RIGHT,
	CTL_PAD2_A,
	CTL_PAD2_B,
	CTL_PAD2_C,
	CTL_PAD2_X,
	CTL_PAD2_Y,
	CTL_PAD2_Z,
	CTL_PAD2_MODE,
	CTL_PAD2_START,
#ifdef WITH_PICO
	CTL_PICO_PEN_UP,
	CTL_PICO_PEN_DOWN,
	CTL_PICO_PEN_LEFT,
	CTL_PICO_PEN_RIGHT,
	CTL_PICO_PEN_BUTTON,
#endif
	CTL_DGEN_QUIT,
	CTL_DGEN_CRAPTV_TOGGLE,
	CTL_DGEN_SCALING_TOGGLE,
	CTL_DGEN_RESET,
	CTL_DGEN_SLOT0,
	CTL_DGEN_SLOT1,
	CTL_DGEN_SLOT2,
	CTL_DGEN_SLOT3,
	CTL_DGEN_SLOT4,
	CTL_DGEN_SLOT5,
	CTL_DGEN_SLOT6,
	CTL_DGEN_SLOT7,
	CTL_DGEN_SLOT8,
	CTL_DGEN_SLOT9,
	CTL_DGEN_SLOT_NEXT,
	CTL_DGEN_SLOT_PREV,
	CTL_DGEN_SAVE,
	CTL_DGEN_LOAD,
	CTL_DGEN_Z80_TOGGLE,
	CTL_DGEN_CPU_TOGGLE,
	CTL_DGEN_STOP,
	CTL_DGEN_PROMPT,
	CTL_DGEN_GAME_GENIE,
	CTL_DGEN_VOLUME_INC,
	CTL_DGEN_VOLUME_DEC,
	CTL_DGEN_FULLSCREEN_TOGGLE,
	CTL_DGEN_FIX_CHECKSUM,
	CTL_DGEN_SCREENSHOT,
	CTL_DGEN_DEBUG_ENTER,
	CTL_
};

// Controls definitions.
struct ctl {
	const enum ctl_e type;
	intptr_t (*const rc)[RCB_NUM];
	int (*const press)(struct ctl&, md&);
	int (*const release)(struct ctl&, md&);
#define DEF 0, 0, 0, 0
	unsigned int pressed:1;
	unsigned int coord:1;
	unsigned int x:10;
	unsigned int y:10;
};

static int ctl_pad1(struct ctl& ctl, md& megad)
{
	switch (ctl.type) {
	case CTL_PAD1_UP:
		megad.pad[0] &= ~MD_UP_MASK;
		break;
	case CTL_PAD1_DOWN:
		megad.pad[0] &= ~MD_DOWN_MASK;
		break;
	case CTL_PAD1_LEFT:
		megad.pad[0] &= ~MD_LEFT_MASK;
		break;
	case CTL_PAD1_RIGHT:
		megad.pad[0] &= ~MD_RIGHT_MASK;
		break;
	case CTL_PAD1_A:
		megad.pad[0] &= ~MD_A_MASK;
		break;
	case CTL_PAD1_B:
		megad.pad[0] &= ~MD_B_MASK;
		break;
	case CTL_PAD1_C:
		megad.pad[0] &= ~MD_C_MASK;
		break;
	case CTL_PAD1_X:
		megad.pad[0] &= ~MD_X_MASK;
		break;
	case CTL_PAD1_Y:
		megad.pad[0] &= ~MD_Y_MASK;
		break;
	case CTL_PAD1_Z:
		megad.pad[0] &= ~MD_Z_MASK;
		break;
	case CTL_PAD1_MODE:
		megad.pad[0] &= ~MD_MODE_MASK;
		break;
	case CTL_PAD1_START:
		megad.pad[0] &= ~MD_START_MASK;
		break;
	default:
		break;
	}
	return 1;
}

static int ctl_pad1_release(struct ctl& ctl, md& megad)
{
	switch (ctl.type) {
	case CTL_PAD1_UP:
		megad.pad[0] |= MD_UP_MASK;
		break;
	case CTL_PAD1_DOWN:
		megad.pad[0] |= MD_DOWN_MASK;
		break;
	case CTL_PAD1_LEFT:
		megad.pad[0] |= MD_LEFT_MASK;
		break;
	case CTL_PAD1_RIGHT:
		megad.pad[0] |= MD_RIGHT_MASK;
		break;
	case CTL_PAD1_A:
		megad.pad[0] |= MD_A_MASK;
		break;
	case CTL_PAD1_B:
		megad.pad[0] |= MD_B_MASK;
		break;
	case CTL_PAD1_C:
		megad.pad[0] |= MD_C_MASK;
		break;
	case CTL_PAD1_X:
		megad.pad[0] |= MD_X_MASK;
		break;
	case CTL_PAD1_Y:
		megad.pad[0] |= MD_Y_MASK;
		break;
	case CTL_PAD1_Z:
		megad.pad[0] |= MD_Z_MASK;
		break;
	case CTL_PAD1_MODE:
		megad.pad[0] |= MD_MODE_MASK;
		break;
	case CTL_PAD1_START:
		megad.pad[0] |= MD_START_MASK;
		break;
	default:
		break;
	}
	return 1;
}

static int ctl_pad2(struct ctl& ctl, md& megad)
{
	switch (ctl.type) {
	case CTL_PAD2_UP:
		megad.pad[1] &= ~MD_UP_MASK;
		break;
	case CTL_PAD2_DOWN:
		megad.pad[1] &= ~MD_DOWN_MASK;
		break;
	case CTL_PAD2_LEFT:
		megad.pad[1] &= ~MD_LEFT_MASK;
		break;
	case CTL_PAD2_RIGHT:
		megad.pad[1] &= ~MD_RIGHT_MASK;
		break;
	case CTL_PAD2_A:
		megad.pad[1] &= ~MD_A_MASK;
		break;
	case CTL_PAD2_B:
		megad.pad[1] &= ~MD_B_MASK;
		break;
	case CTL_PAD2_C:
		megad.pad[1] &= ~MD_C_MASK;
		break;
	case CTL_PAD2_X:
		megad.pad[1] &= ~MD_X_MASK;
		break;
	case CTL_PAD2_Y:
		megad.pad[1] &= ~MD_Y_MASK;
		break;
	case CTL_PAD2_Z:
		megad.pad[1] &= ~MD_Z_MASK;
		break;
	case CTL_PAD2_MODE:
		megad.pad[1] &= ~MD_MODE_MASK;
		break;
	case CTL_PAD2_START:
		megad.pad[1] &= ~MD_START_MASK;
		break;
	default:
		break;
	}
	return 1;
}

static int ctl_pad2_release(struct ctl& ctl, md& megad)
{
	switch (ctl.type) {
	case CTL_PAD2_UP:
		megad.pad[1] |= MD_UP_MASK;
		break;
	case CTL_PAD2_DOWN:
		megad.pad[1] |= MD_DOWN_MASK;
		break;
	case CTL_PAD2_LEFT:
		megad.pad[1] |= MD_LEFT_MASK;
		break;
	case CTL_PAD2_RIGHT:
		megad.pad[1] |= MD_RIGHT_MASK;
		break;
	case CTL_PAD2_A:
		megad.pad[1] |= MD_A_MASK;
		break;
	case CTL_PAD2_B:
		megad.pad[1] |= MD_B_MASK;
		break;
	case CTL_PAD2_C:
		megad.pad[1] |= MD_C_MASK;
		break;
	case CTL_PAD2_X:
		megad.pad[1] |= MD_X_MASK;
		break;
	case CTL_PAD2_Y:
		megad.pad[1] |= MD_Y_MASK;
		break;
	case CTL_PAD2_Z:
		megad.pad[1] |= MD_Z_MASK;
		break;
	case CTL_PAD2_MODE:
		megad.pad[1] |= MD_MODE_MASK;
		break;
	case CTL_PAD2_START:
		megad.pad[1] |= MD_START_MASK;
		break;
	default:
		break;
	}
	return 1;
}

#ifdef WITH_PICO

static int ctl_pico_pen(struct ctl& ctl, md& megad)
{
	static unsigned int min_y = 0x1fc;
	static unsigned int max_y = 0x2f7;
	static unsigned int min_x = 0x3c;
	static unsigned int max_x = 0x17c;
	static const struct {
		enum ctl_e type;
		unsigned int coords:1;
		unsigned int dir:1;
		unsigned int lim[2];
	} motion[] = {
		{ CTL_PICO_PEN_UP, 1, 0, { min_y, max_y } },
		{ CTL_PICO_PEN_DOWN, 1, 1, { min_y, max_y } },
		{ CTL_PICO_PEN_LEFT, 0, 0, { min_x, max_x } },
		{ CTL_PICO_PEN_RIGHT, 0, 1, { min_x, max_x } }
	};
	unsigned int i;

	if (ctl.type == CTL_PICO_PEN_BUTTON) {
		megad.pad[0] &= ~MD_PICO_PENBTN_MASK;
		return 1;
	}
	// Use coordinates if available.
	if ((ctl.coord) &&
	    (screen.window_width != 0) &&
	    (screen.window_height != 0)) {
		megad.pico_pen_coords[1] =
			(min_y + ((ctl.y * (max_y - min_y)) /
				  screen.window_height));
		megad.pico_pen_coords[0] =
			(min_x + ((ctl.x * (max_x - min_x)) /
				  screen.window_width));
		return 1;
	}
	for (i = 0; (i != elemof(motion)); ++i) {
		unsigned int coords;

		if (motion[i].type != ctl.type)
			continue;
		coords = motion[i].coords;
		if (motion[i].dir)
			megad.pico_pen_coords[coords] += pico_pen_stride;
		else
			megad.pico_pen_coords[coords] -= pico_pen_stride;
		if ((megad.pico_pen_coords[coords] < motion[i].lim[0]) ||
		    (megad.pico_pen_coords[coords] > motion[i].lim[1]))
			megad.pico_pen_coords[coords] =
				motion[i].lim[motion[i].dir];
		break;
	}
	return 1;
}

static int ctl_pico_pen_release(struct ctl& ctl, md& megad)
{
	if (ctl.type == CTL_PICO_PEN_BUTTON)
		megad.pad[0] |= MD_PICO_PENBTN_MASK;
	return 1;
}

#endif

static int ctl_dgen_quit(struct ctl&, md&)
{
	return 0;
}

static int ctl_dgen_craptv_toggle(struct ctl&, md&)
{
#ifdef WITH_CTV
	dgen_craptv = ((dgen_craptv + 1) % NUM_CTV);
	filters_pluck_ctv();
	filters_insert(filters_find(ctv_names[dgen_craptv]));
	pd_message("Crap TV mode \"%s\".", ctv_names[dgen_craptv]);
#endif // WITH_CTV
	return 1;
}

static int ctl_dgen_scaling_toggle(struct ctl&, md&)
{
	dgen_scaling = ((dgen_scaling + 1) % NUM_SCALING);
	if (set_scaling(scaling_names[dgen_scaling]))
		pd_message("Scaling algorithm \"%s\" unavailable.",
			   scaling_names[dgen_scaling]);
	else
		pd_message("Using scaling algorithm \"%s\".",
			   scaling_names[dgen_scaling]);
	return 1;
}

static int ctl_dgen_reset(struct ctl&, md& megad)
{
	megad.reset();
	pd_message("Genesis reset.");
	return 1;
}

static int ctl_dgen_slot(struct ctl& ctl, md&)
{
	slot = ((int)ctl.type - CTL_DGEN_SLOT0);
	pd_message("Selected save slot %d.", slot);
	return 1;
}

static int ctl_dgen_slot_next(struct ctl&, md&)
{
	if (slot == 9)
		slot = 0;
	else
		slot++;
	pd_message("Selected next save slot (%d).", slot);
	return 1;
}

static int ctl_dgen_slot_prev(struct ctl&, md&)
{
	if (slot == 0)
		slot = 9;
	else
		slot--;
	pd_message("Selected previous save slot (%d).", slot);
	return 1;
}

static int ctl_dgen_save(struct ctl&, md& megad)
{
	md_save(megad);
	return 1;
}

static int ctl_dgen_load(struct ctl&, md& megad)
{
	md_load(megad);
	return 1;
}

// Cycle Z80 core.
static int ctl_dgen_z80_toggle(struct ctl&, md& megad)
{
	const char *msg;

	megad.cycle_z80();
	switch (megad.z80_core) {
#ifdef WITH_CZ80
	case md::Z80_CORE_CZ80:
		msg = "CZ80 core activated.";
		break;
#endif
#ifdef WITH_MZ80
	case md::Z80_CORE_MZ80:
		msg = "MZ80 core activated.";
		break;
#endif
#ifdef WITH_DRZ80
	case md::Z80_CORE_DRZ80:
		msg = "DrZ80 core activated.";
		break;
#endif
	default:
		msg = "Z80 core disabled.";
		break;
	}
	pd_message(msg);
	return 1;
}

// Added this CPU core hot swap.  Compile both Musashi and StarScream
// in, and swap on the fly like DirectX DGen. [PKH]
static int ctl_dgen_cpu_toggle(struct ctl&, md& megad)
{
	const char *msg;

	megad.cycle_cpu();
	switch (megad.cpu_emu) {
#ifdef WITH_STAR
	case md::CPU_EMU_STAR:
		msg = "StarScream CPU core activated.";
		break;
#endif
#ifdef WITH_MUSA
	case md::CPU_EMU_MUSA:
		msg = "Musashi CPU core activated.";
		break;
#endif
#ifdef WITH_CYCLONE
	case md::CPU_EMU_CYCLONE:
		msg = "Cyclone CPU core activated.";
		break;
#endif
	default:
		msg = "CPU core disabled.";
		break;
	}
	pd_message(msg);
	return 1;
}

static int ctl_dgen_stop(struct ctl&, md& megad)
{
	pd_message(stopped_str);
	if (stop_events(megad, STOPPED) != 0)
		return 0;
	return 1;
}

static int ctl_dgen_prompt(struct ctl&, md& megad)
{
	pd_message_cursor(strlen(prompt_str), prompt_str);
	if (stop_events(megad, PROMPT) != 0)
		return 0;
	return 1;
}

static int ctl_dgen_game_genie(struct ctl&, md& megad)
{
	pd_message_cursor(strlen(game_genie_str), game_genie_str);
	if (stop_events(megad, GAME_GENIE) != 0)
		return 0;
	return 1;
}

static int ctl_dgen_volume(struct ctl& ctl, md&)
{
	if (ctl.type == CTL_DGEN_VOLUME_INC)
		++dgen_volume;
	else
		--dgen_volume;
	if (dgen_volume < 0)
		dgen_volume = 0;
	else if (dgen_volume > 100)
		dgen_volume = 100;
	pd_message("Volume %d%%.", (int)dgen_volume);
	return 1;
}

static int ctl_dgen_fullscreen_toggle(struct ctl&, md&)
{
	switch (set_fullscreen(!screen.is_fullscreen)) {
	case -2:
		fprintf(stderr,
			"sdl: fatal error while trying to change screen"
			" resolution.\n");
		return 0;
	case -1:
		pd_message("Failed to toggle fullscreen mode.");
		break;
	default:
		pd_message("Fullscreen mode toggled.");
	}
	return 1;
}

static int ctl_dgen_fix_checksum(struct ctl&, md& megad)
{
	pd_message("Checksum fixed.");
	megad.fix_rom_checksum();
	return 1;
}

static int ctl_dgen_screenshot(struct ctl&, md& megad)
{
	do_screenshot(megad);
	return 1;
}

static int ctl_dgen_debug_enter(struct ctl&, md& megad)
{
#ifdef WITH_DEBUGGER
	stopped = 1;
	if (megad.debug_trap == false)
		megad.debug_enter();
	else
		megad.debug_leave();
#else
	(void)megad;
	pd_message("Debugger support not built in.");
#endif
	return 1;
}

static struct ctl control[] = {
	// Array indices and control[].type must match enum ctl_e's order.
	{ CTL_PAD1_UP, &pad1_up, ctl_pad1, ctl_pad1_release, DEF },
	{ CTL_PAD1_DOWN, &pad1_down, ctl_pad1, ctl_pad1_release, DEF },
	{ CTL_PAD1_LEFT, &pad1_left, ctl_pad1, ctl_pad1_release, DEF },
	{ CTL_PAD1_RIGHT, &pad1_right, ctl_pad1, ctl_pad1_release, DEF },
	{ CTL_PAD1_A, &pad1_a, ctl_pad1, ctl_pad1_release, DEF },
	{ CTL_PAD1_B, &pad1_b, ctl_pad1, ctl_pad1_release, DEF },
	{ CTL_PAD1_C, &pad1_c, ctl_pad1, ctl_pad1_release, DEF },
	{ CTL_PAD1_X, &pad1_x, ctl_pad1, ctl_pad1_release, DEF },
	{ CTL_PAD1_Y, &pad1_y, ctl_pad1, ctl_pad1_release, DEF },
	{ CTL_PAD1_Z, &pad1_z, ctl_pad1, ctl_pad1_release, DEF },
	{ CTL_PAD1_MODE, &pad1_mode, ctl_pad1, ctl_pad1_release, DEF },
	{ CTL_PAD1_START, &pad1_start, ctl_pad1, ctl_pad1_release, DEF },
	{ CTL_PAD2_UP, &pad2_up, ctl_pad2, ctl_pad2_release, DEF },
	{ CTL_PAD2_DOWN, &pad2_down, ctl_pad2, ctl_pad2_release, DEF },
	{ CTL_PAD2_LEFT, &pad2_left, ctl_pad2, ctl_pad2_release, DEF },
	{ CTL_PAD2_RIGHT, &pad2_right, ctl_pad2, ctl_pad2_release, DEF },
	{ CTL_PAD2_A, &pad2_a, ctl_pad2, ctl_pad2_release, DEF },
	{ CTL_PAD2_B, &pad2_b, ctl_pad2, ctl_pad2_release, DEF },
	{ CTL_PAD2_C, &pad2_c, ctl_pad2, ctl_pad2_release, DEF },
	{ CTL_PAD2_X, &pad2_x, ctl_pad2, ctl_pad2_release, DEF },
	{ CTL_PAD2_Y, &pad2_y, ctl_pad2, ctl_pad2_release, DEF },
	{ CTL_PAD2_Z, &pad2_z, ctl_pad2, ctl_pad2_release, DEF },
	{ CTL_PAD2_MODE, &pad2_mode, ctl_pad2, ctl_pad2_release, DEF },
	{ CTL_PAD2_START, &pad2_start, ctl_pad2, ctl_pad2_release, DEF },
#ifdef WITH_PICO
	{ CTL_PICO_PEN_UP,
	  &pico_pen_up, ctl_pico_pen, ctl_pico_pen_release, DEF },
	{ CTL_PICO_PEN_DOWN,
	  &pico_pen_down, ctl_pico_pen, ctl_pico_pen_release, DEF },
	{ CTL_PICO_PEN_LEFT,
	  &pico_pen_left, ctl_pico_pen, ctl_pico_pen_release, DEF },
	{ CTL_PICO_PEN_RIGHT,
	  &pico_pen_right, ctl_pico_pen, ctl_pico_pen_release, DEF },
	{ CTL_PICO_PEN_BUTTON,
	  &pico_pen_button, ctl_pico_pen, ctl_pico_pen_release, DEF },
#endif
	{ CTL_DGEN_QUIT, &dgen_quit, ctl_dgen_quit, NULL, DEF },
	{ CTL_DGEN_CRAPTV_TOGGLE,
	  &dgen_craptv_toggle, ctl_dgen_craptv_toggle, NULL, DEF },
	{ CTL_DGEN_SCALING_TOGGLE,
	  &dgen_scaling_toggle, ctl_dgen_scaling_toggle, NULL, DEF },
	{ CTL_DGEN_RESET, &dgen_reset, ctl_dgen_reset, NULL, DEF },
	{ CTL_DGEN_SLOT0, &dgen_slot_0, ctl_dgen_slot, NULL, DEF },
	{ CTL_DGEN_SLOT1, &dgen_slot_1, ctl_dgen_slot, NULL, DEF },
	{ CTL_DGEN_SLOT2, &dgen_slot_2, ctl_dgen_slot, NULL, DEF },
	{ CTL_DGEN_SLOT3, &dgen_slot_3, ctl_dgen_slot, NULL, DEF },
	{ CTL_DGEN_SLOT4, &dgen_slot_4, ctl_dgen_slot, NULL, DEF },
	{ CTL_DGEN_SLOT5, &dgen_slot_5, ctl_dgen_slot, NULL, DEF },
	{ CTL_DGEN_SLOT6, &dgen_slot_6, ctl_dgen_slot, NULL, DEF },
	{ CTL_DGEN_SLOT7, &dgen_slot_7, ctl_dgen_slot, NULL, DEF },
	{ CTL_DGEN_SLOT8, &dgen_slot_8, ctl_dgen_slot, NULL, DEF },
	{ CTL_DGEN_SLOT9, &dgen_slot_9, ctl_dgen_slot, NULL, DEF },
	{ CTL_DGEN_SLOT_NEXT, &dgen_slot_next, ctl_dgen_slot_next, NULL, DEF },
	{ CTL_DGEN_SLOT_PREV, &dgen_slot_prev, ctl_dgen_slot_prev, NULL, DEF },
	{ CTL_DGEN_SAVE, &dgen_save, ctl_dgen_save, NULL, DEF },
	{ CTL_DGEN_LOAD, &dgen_load, ctl_dgen_load, NULL, DEF },
	{ CTL_DGEN_Z80_TOGGLE,
	  &dgen_z80_toggle, ctl_dgen_z80_toggle, NULL, DEF },
	{ CTL_DGEN_CPU_TOGGLE,
	  &dgen_cpu_toggle, ctl_dgen_cpu_toggle, NULL, DEF },
	{ CTL_DGEN_STOP, &dgen_stop, ctl_dgen_stop, NULL, DEF },
	{ CTL_DGEN_PROMPT, &dgen_prompt, ctl_dgen_prompt, NULL, DEF },
	{ CTL_DGEN_GAME_GENIE,
	  &dgen_game_genie, ctl_dgen_game_genie, NULL, DEF },
	{ CTL_DGEN_VOLUME_INC,
	  &dgen_volume_inc, ctl_dgen_volume, NULL, DEF },
	{ CTL_DGEN_VOLUME_DEC,
	  &dgen_volume_dec, ctl_dgen_volume, NULL, DEF },
	{ CTL_DGEN_FULLSCREEN_TOGGLE,
	  &dgen_fullscreen_toggle, ctl_dgen_fullscreen_toggle, NULL, DEF },
	{ CTL_DGEN_FIX_CHECKSUM,
	  &dgen_fix_checksum, ctl_dgen_fix_checksum, NULL, DEF },
	{ CTL_DGEN_SCREENSHOT,
	  &dgen_screenshot, ctl_dgen_screenshot, NULL, DEF },
	{ CTL_DGEN_DEBUG_ENTER,
	  &dgen_debug_enter, ctl_dgen_debug_enter, NULL, DEF },
	{ CTL_, NULL, NULL, NULL, DEF }
};

static struct {
	char const* name; ///< Controller button name.
	enum ctl_e const id[2]; ///< Controls indices in control[].
	bool once; ///< If button has been pressed once.
	bool twice; ///< If button has been pressed twice.
	enum rc_binding_type type; ///< Type of code.
	intptr_t code; ///< Temporary code.
} calibration_steps[] = {
	{ "START", { CTL_PAD1_START, CTL_PAD2_START },
	  false, false, RCB_NUM, -1 },
	{ "MODE", { CTL_PAD1_MODE, CTL_PAD2_MODE },
	  false, false, RCB_NUM, -1 },
	{ "A", { CTL_PAD1_A, CTL_PAD2_A },
	  false, false, RCB_NUM, -1 },
	{ "B", { CTL_PAD1_B, CTL_PAD2_B },
	  false, false, RCB_NUM, -1 },
	{ "C", { CTL_PAD1_C, CTL_PAD2_C },
	  false, false, RCB_NUM, -1 },
	{ "X", { CTL_PAD1_X, CTL_PAD2_X },
	  false, false, RCB_NUM, -1 },
	{ "Y", { CTL_PAD1_Y, CTL_PAD2_Y },
	  false, false, RCB_NUM, -1 },
	{ "Z", { CTL_PAD1_Z, CTL_PAD2_Z },
	  false, false, RCB_NUM, -1 },
	{ "UP", { CTL_PAD1_UP, CTL_PAD2_UP },
	  false, false, RCB_NUM, -1 },
	{ "DOWN", { CTL_PAD1_DOWN, CTL_PAD2_DOWN },
	  false, false, RCB_NUM, -1 },
	{ "LEFT", { CTL_PAD1_LEFT, CTL_PAD2_LEFT },
	  false, false, RCB_NUM, -1 },
	{ "RIGHT", { CTL_PAD1_RIGHT, CTL_PAD2_RIGHT },
	  false, false, RCB_NUM, -1 },
	{ NULL, { CTL_, CTL_ },
	  false, false, RCB_NUM, -1 }
};

/**
 * Handle input during calibration process.
 * @param type Type of code.
 * @param code Code to process.
 */
static void manage_calibration(enum rc_binding_type type, intptr_t code)
{
	unsigned int step = 0;

	assert(calibrating_controller < 2);
	if (!calibrating) {
		// Stop emulation, enter calibration mode.
		freeze(true);
		calibrating = true;
		filter_text_str[0] = '\0';
		filter_text_msg(FILTER_TEXT_BG_BLACK
				FILTER_TEXT_CENTER
				FILTER_TEXT_8X13
				"CONTROLLER %u CALIBRATION\n"
				"\n"
				FILTER_TEXT_7X6
				FILTER_TEXT_LEFT
				"Press each button twice,\n"
				"or two different buttons to skip them.\n"
				"\n",
				(calibrating_controller + 1));
		filters_pluck(&filter_text_def);
		filters_insert(&filter_text_def);
		goto ask;
	}
	while (step != elemof(calibration_steps))
		if ((calibration_steps[step].once == true) &&
		    (calibration_steps[step].twice == true))
			++step;
		else
			break;
	if (step == elemof(calibration_steps)) {
		// Reset everything.
		for (step = 0; (step != elemof(calibration_steps)); ++step) {
			calibration_steps[step].once = false;
			calibration_steps[step].twice = false;
			calibration_steps[step].type = RCB_NUM;
			calibration_steps[step].code = -1;
		}
		// Restart emulation.
		freeze(false);
		calibrating = false;
		filters_pluck(&filter_text_def);
		return;
	}
	if (calibration_steps[step].once == false) {
		char *dump;

		if (type == RCBK)
			dump = dump_keysym(code);
		else if (type == RCBJ)
			dump = dump_joypad(code);
		else if (type == RCBM)
			dump = dump_mouse(code);
		else
			dump = NULL;
		assert(calibration_steps[step].twice == false);
		calibration_steps[step].once = true;
		calibration_steps[step].type = type;
		calibration_steps[step].code = code;
		filter_text_msg("\"%s\", confirm: ", (dump ? dump : ""));
		free(dump);
	}
	else if (calibration_steps[step].twice == false) {
		calibration_steps[step].twice = true;
		if ((calibration_steps[step].type == type) &&
		    (calibration_steps[step].code == code))
			filter_text_msg("OK\n");
		else {
			calibration_steps[step].type = RCB_NUM;
			calibration_steps[step].code = -1;
			filter_text_msg("none\n");
		}
	}
	if ((calibration_steps[step].once != true) ||
	    (calibration_steps[step].twice != true))
		return;
	++step;
ask:
	if (step == elemof(calibration_steps)) {
		code = calibration_steps[(elemof(calibration_steps) - 1)].code;
		if (code == -1)
			filter_text_msg("\n"
					"Aborted.");
		else {
			unsigned int i;

			for (i = 0; (i != elemof(calibration_steps)); ++i) {
				enum ctl_e id;

				id = calibration_steps[i].id
					[calibrating_controller];
				type = calibration_steps[i].type;
				code = calibration_steps[i].code;
				assert((size_t)id < elemof(control));
				assert(control[id].type == id);
				if ((id != CTL_) && (type != RCB_NUM))
					(*control[id].rc)[type] = code;
			}
			filter_text_msg("\n"
					"Applied.");
		}
	}
	else if (calibration_steps[step].name != NULL)
		filter_text_msg("%s: ", calibration_steps[step].name);
	else
		filter_text_msg("\n"
				"Press any button twice to apply settings:\n"
				"");
}

static struct rc_binding_item combos[64];

static void manage_combos(md& md, bool pressed, enum rc_binding_type type,
			  intptr_t code)
{
	unsigned int i;

	(void)md;
	for (i = 0; (i != elemof(combos)); ++i) {
		if (!combos[i].assigned) {
			if (!pressed)
				return; // Not in the list, nothing to do.
			// Not found, add it to the list.
			combos[i].assigned = true;
			combos[i].type = type;
			combos[i].code = code;
			return;
		}
		if ((combos[i].type != type) || (combos[i].code != code))
			continue; // Does not match.
		if (pressed)
			return; // Already pressed.
		// Release entry.
		memmove(&combos[i], &combos[i + 1],
			((elemof(combos) - (i + 1)) * sizeof(combos[i])));
		break;
	}
}

static bool check_combos(md& md, struct rc_binding_item item[],
			 unsigned int num)
{
	unsigned int i;
	unsigned int found = 0;

	(void)md;
	for (i = 0; (i != num); ++i) {
		unsigned int j;

		if (!item[i].assigned) {
			num = i;
			break;
		}
		for (j = 0; (j != elemof(combos)); ++j) {
			if (!combos[j].assigned)
				break;
			if ((combos[j].type != item[i].type) ||
			    (combos[j].code != item[i].code))
				continue;
			++found;
			break;
		}
	}
	if (num == 0)
		return false;
	return (found == num);
}

static int manage_bindings(md& md, bool pressed, enum rc_binding_type type,
			   intptr_t code)
{
	struct rc_binding *rcb = rc_binding_head.next;
	size_t pos = 0;
	size_t seek = 0;

	if ((dgen_buttons) && (pressed)) {
		char *dump;

		if (type == RCBK)
			dump = dump_keysym(code);
		else if (type == RCBJ)
			dump = dump_joypad(code);
		else if (type == RCBM)
			dump = dump_mouse(code);
		else
			dump = NULL;
		if (dump != NULL) {
			pd_message("Pressed \"%s\".", dump);
			free(dump);
		}
	}
	while (rcb != &rc_binding_head) {
		if ((pos < seek) ||
		    (!check_combos(md, rcb->item, elemof(rcb->item)))) {
			++pos;
			rcb = rcb->next;
			continue;
		}
		assert(rcb->to != NULL);
		assert((intptr_t)rcb->to != -1);
		// For keyboard and joystick bindings, perform related action.
		if ((type = RCBK, !strncasecmp("key_", rcb->to, 4)) ||
		    (type = RCBJ, !strncasecmp("joy_", rcb->to, 4)) ||
		    (type = RCBM, !strncasecmp("mou_", rcb->to, 4))) {
			struct rc_field *rcf = rc_fields;

			while (rcf->fieldname != NULL) {
				struct ctl *ctl = control;

				if (strcasecmp(rcb->to, rcf->fieldname)) {
					++rcf;
					continue;
				}
				while (ctl->rc != NULL) {
					if (&(*ctl->rc)[type] !=
					    rcf->variable) {
						++ctl;
						continue;
					}
					// Got it, finally.
					if (pressed) {
						assert(ctl->press != NULL);
						if (!ctl->press(*ctl, md))
							return 0;
					}
					else if (ctl->release != NULL) {
						if (!ctl->release(*ctl, md))
							return 0;
					}
					break;
				}
				break;
			}
		}
		// Otherwise, pass it to the prompt.
		else if (pressed) {
			handle_prompt_complete_clear();
			prompt_replace(&prompt.status, 0, 0,
				       (uint8_t *)rcb->to, strlen(rcb->to));
			if (handle_prompt_enter(md) & PROMPT_RET_ERROR)
				return 0;
		}
		// In case the current (or any other binding) has been
		// removed, rewind and seek to the next position.
		rcb = rc_binding_head.next;
		seek = (pos + 1);
		pos = 0;
	}
	return 1;
}

static int manage_game_genie(md& megad, intptr_t ksym, intptr_t ksym_uni)
{
	static char buf[12];
	static kb_input_t input = { buf, 0, sizeof(buf) };
	unsigned int len = strlen(game_genie_str);

	switch (kb_input(&input, ksym, ksym_uni)) {
		unsigned int errors;
		unsigned int applied;
		unsigned int reverted;

	case KB_INPUT_ENTERED:
		megad.patch(input.buf, &errors, &applied, &reverted);
		if (errors)
			pd_message("Invalid code.");
		else if (reverted)
			pd_message("Reverted.");
		else if (applied)
			pd_message("Applied.");
		else {
		case KB_INPUT_ABORTED:
			pd_message("Aborted.");
		}
		goto over;
	case KB_INPUT_CONSUMED:
		pd_message_cursor((len + input.pos), "%s%.*s", game_genie_str,
				  (int)input.pos, buf);
		break;
	case KB_INPUT_IGNORED:
		break;
	}
	return 0;
over:
	input.buf = buf;
	input.pos = 0;
	input.size = sizeof(buf);
	memset(buf, 0, sizeof(buf));
	return 1;
}

#ifdef WITH_PICO

static void manage_pico_pen(md& megad)
{
	static unsigned long pico_pen_last_update;
	unsigned long pico_pen_now;

	if (!megad.pico_enabled)
		return;
	// Repeat pen motion as long as buttons are not released.
	// This is not necessary when pen is managed by direct coordinates.
	if ((((control[CTL_PICO_PEN_UP].pressed) &&
	      (!control[CTL_PICO_PEN_UP].coord)) ||
	     ((control[CTL_PICO_PEN_DOWN].pressed) &&
	      (!control[CTL_PICO_PEN_DOWN].coord)) ||
	     ((control[CTL_PICO_PEN_LEFT].pressed) &&
	      (!control[CTL_PICO_PEN_LEFT].coord)) ||
	     ((control[CTL_PICO_PEN_RIGHT].pressed) &&
	      (!control[CTL_PICO_PEN_RIGHT].coord))) &&
	    (pico_pen_now = pd_usecs(),
	     ((pico_pen_now - pico_pen_last_update) >=
	      ((unsigned long)pico_pen_delay * 1000)))) {
		if (control[CTL_PICO_PEN_UP].pressed)
			ctl_pico_pen
				(control[CTL_PICO_PEN_UP], megad);
		if (control[CTL_PICO_PEN_DOWN].pressed)
			ctl_pico_pen
				(control[CTL_PICO_PEN_DOWN], megad);
		if (control[CTL_PICO_PEN_LEFT].pressed)
			ctl_pico_pen
				(control[CTL_PICO_PEN_LEFT], megad);
		if (control[CTL_PICO_PEN_RIGHT].pressed)
			ctl_pico_pen
				(control[CTL_PICO_PEN_RIGHT], megad);
		pico_pen_last_update = pico_pen_now;
	}
}

#endif

static bool mouse_is_grabbed()
{
	return (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_ON);
}

static void mouse_grab(bool grab)
{
	SDL_GrabMode mode = SDL_WM_GrabInput(SDL_GRAB_QUERY);

	if ((grab) && (!pd_freeze) && (mode == SDL_GRAB_OFF)) {
		// Hide the cursor.
		SDL_ShowCursor(0);
		pd_message("Mouse trapped. Stop emulation to release.");
		SDL_WM_GrabInput(SDL_GRAB_ON);
	}
	else if ((!grab) && (mode == SDL_GRAB_ON)) {
		SDL_ShowCursor(1);
		SDL_WM_GrabInput(SDL_GRAB_OFF);
	}
}

static int stop_events(md& megad, enum events status)
{
	struct ctl* ctl;

	stopped = 1;
	freeze(true);
	events = status;
	// Release controls.
	for (ctl = control; (ctl->rc != NULL); ++ctl) {
		if (ctl->pressed == false)
			continue;
		ctl->pressed = false;
		ctl->coord = false;
		if ((ctl->release != NULL) &&
		    (ctl->release(*ctl, megad) == 0))
			return -1; // XXX do something about this.
	}
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
			    SDL_DEFAULT_REPEAT_INTERVAL);
	// Switch out of fullscreen mode (assuming this is supported)
	if (screen.is_fullscreen) {
		if (set_fullscreen(0) < -1)
			return -1;
		pd_graphics_update(true);
	}
	mouse_grab(false);
	return 0;
}

static void restart_events(md& megad)
{
	(void)megad;
	stopped = 1;
	freeze(false);
	handle_prompt_complete_clear();
	SDL_EnableKeyRepeat(0, 0);
	events = STARTED;
}

static struct {
	unsigned long when[0x100];
	uint8_t enabled[0x100 / 8];
	unsigned int count;
} mouse_motion_release;

#define MOUSE_MOTION_RELEASE_IS_ENABLED(which) \
	(mouse_motion_release.enabled[(which) / 8] & (1 << ((which) % 8)))
#define MOUSE_MOTION_RELEASE_DISABLE(which) \
	(mouse_motion_release.enabled[(which) / 8] &= ~(1 << ((which) % 8)))
#define MOUSE_MOTION_RELEASE_ENABLE(which) \
	(mouse_motion_release.enabled[(which) / 8] |= (1 << ((which) % 8)))

static void mouse_motion_delay_release(unsigned int which, bool enable)
{
	if (which >= elemof(mouse_motion_release.when)) {
		DEBUG(("mouse index too high (%u)", which));
		return;
	}
	if (!enable) {
		if (!MOUSE_MOTION_RELEASE_IS_ENABLED(which))
			return;
		MOUSE_MOTION_RELEASE_DISABLE(which);
		assert(mouse_motion_release.count != 0);
		--mouse_motion_release.count;
		return;
	}
	if (!MOUSE_MOTION_RELEASE_IS_ENABLED(which)) {
		MOUSE_MOTION_RELEASE_ENABLE(which);
		++mouse_motion_release.count;
		assert(mouse_motion_release.count <=
		       elemof(mouse_motion_release.when));
	}
	mouse_motion_release.when[which] =
		(pd_usecs() + (dgen_mouse_delay * 1000));
}

static bool mouse_motion_released(SDL_Event *event)
{
	unsigned int i;
	unsigned long now;

	if (mouse_motion_release.count == 0)
		return false;
	now = pd_usecs();
	for (i = 0; (i != mouse_motion_release.count); ++i) {
		unsigned long diff;

		if (!MOUSE_MOTION_RELEASE_IS_ENABLED(i))
			continue;
		diff = (mouse_motion_release.when[i] - now);
		if (diff < (unsigned long)(dgen_mouse_delay * 1000))
			continue;
		event->motion.type = SDL_MOUSEMOTION;
		event->motion.which = i;
		event->motion.xrel = 0;
		event->motion.yrel = 0;
		MOUSE_MOTION_RELEASE_DISABLE(i);
		--mouse_motion_release.count;
		return true;
	}
	return false;
}

#define MOUSE_SHOW_USECS (unsigned long)(2 * 1000000)

// The massive event handler!
// I know this is an ugly beast, but please don't be discouraged. If you need
// help, don't be afraid to ask me how something works. Basically, just handle
// all the event keys, or even ignore a few if they don't make sense for your
// interface.
int pd_handle_events(md &megad)
{
	static uint16_t kpress[0x100];
#ifdef WITH_DEBUGGER
	static bool debug_trap;
#endif
	static unsigned long hide_mouse_when;
	static bool hide_mouse;
#ifdef WITH_JOYSTICK
	static uint32_t const axis_value[][3] = {
		// { pressed, [implicitly released ...] }
		{ JS_AXIS_NEGATIVE, JS_AXIS_BETWEEN, JS_AXIS_POSITIVE },
		{ JS_AXIS_POSITIVE, JS_AXIS_BETWEEN, JS_AXIS_NEGATIVE },
		{ JS_AXIS_BETWEEN, JS_AXIS_POSITIVE, JS_AXIS_NEGATIVE }
	};
	static uint32_t const hat_value[][2] = {
		// { SDL value, pressed }
		{ SDL_HAT_UP, JS_HAT_UP },
		{ SDL_HAT_RIGHT, JS_HAT_RIGHT },
		{ SDL_HAT_DOWN, JS_HAT_DOWN },
		{ SDL_HAT_LEFT, JS_HAT_LEFT }
	};
	unsigned int hat_value_map;
	intptr_t joypad;
#endif
	bool pressed;
	uint32_t plist[8];
	uint32_t rlist[8];
	unsigned int i, pi, ri;
	SDL_Event event;
	uint16_t ksym_uni;
	intptr_t ksym;
	intptr_t mouse;
	unsigned int which;

#ifdef WITH_DEBUGGER
	if ((megad.debug_trap) && (megad.debug_enter() < 0))
		return 0;
	if (debug_trap != megad.debug_trap) {
		debug_trap = megad.debug_trap;
		if (debug_trap)
			mouse_grab(false);
		if (sound.cbuf.size)
			SDL_PauseAudio(debug_trap == true);
	}
#endif
	if ((hide_mouse) &&
	    ((hide_mouse_when - pd_usecs()) >= MOUSE_SHOW_USECS)) {
		if (!mouse_is_grabbed())
			SDL_ShowCursor(0);
		hide_mouse = false;
	}
next_event:
	if (mouse_motion_released(&event))
		goto mouse_motion;
	if (!SDL_PollEvent(&event)) {
#ifdef WITH_PICO
		manage_pico_pen(megad);
#endif
		return 1;
	}
	switch (event.type) {
#ifdef WITH_JOYSTICK
	case SDL_JOYAXISMOTION:
		if (event.jaxis.value <= -16384)
			i = 0;
		else if (event.jaxis.value >= 16384)
			i = 1;
		else
			i = 2;
		plist[0] = JS_AXIS(event.jaxis.which,
				   event.jaxis.axis,
				   axis_value[i][0]);
		rlist[0] = JS_AXIS(event.jaxis.which,
				   event.jaxis.axis,
				   axis_value[i][1]);
		rlist[1] = JS_AXIS(event.jaxis.which,
				   event.jaxis.axis,
				   axis_value[i][2]);
		// "between" causes problems during calibration, ignore it.
		if (axis_value[i][0] == JS_AXIS_BETWEEN)
			pi = 0;
		else
			pi = 1;
		ri = 2;
		goto joypad_axis;
	case SDL_JOYHATMOTION:
		pi = 0;
		ri = 0;
		hat_value_map = 0;
		for (i = 0; (i != elemof(hat_value)); ++i)
			if (event.jhat.value & hat_value[i][0]) {
				plist[pi++] = JS_HAT(event.jhat.which,
						     event.jhat.hat,
						     hat_value[i][1]);
				hat_value_map |= (1 << i);
			}
		for (i = 0; (i != elemof(hat_value)); ++i)
			if ((hat_value_map & (1 << i)) == 0)
				rlist[ri++] = JS_HAT(event.jhat.which,
						     event.jhat.hat,
						     hat_value[i][1]);
	joypad_axis:
		for (i = 0; (i != ri); ++i)
			manage_combos(megad, false, RCBJ, rlist[i]);
		for (i = 0; (i != pi); ++i)
			manage_combos(megad, true, RCBJ, plist[i]);
		if (events != STARTED)
			break;
		if (calibrating) {
			for (i = 0; ((calibrating) && (i != pi)); ++i)
				manage_calibration(RCBJ, plist[i]);
			break;
		}
		for (struct ctl* ctl = control; (ctl->rc != NULL); ++ctl) {
			// Release buttons first.
			for (i = 0; (i != ri); ++i) {
				if ((ctl->pressed == false) ||
				    ((uint32_t)(*ctl->rc)[RCBJ] != rlist[i]))
					continue;
				ctl->pressed = false;
				ctl->coord = false;
				if ((ctl->release != NULL) &&
				    (ctl->release(*ctl, megad) == 0))
					return 0;
			}
			for (i = 0; (i != pi); ++i) {
				if ((uint32_t)(*ctl->rc)[RCBJ] == plist[i]) {
					assert(ctl->press != NULL);
					ctl->pressed = true;
					ctl->coord = false;
					if (ctl->press(*ctl, megad) == 0)
						return 0;
				}
			}
		}
		for (i = 0; (i != ri); ++i)
			if (!manage_bindings(megad, false, RCBJ, rlist[i]))
				return 0;
		for (i = 0; (i != pi); ++i)
			if (!manage_bindings(megad, true, RCBJ, plist[i]))
				return 0;
		break;
	case SDL_JOYBUTTONDOWN:
		assert(event.jbutton.state == SDL_PRESSED);
		pressed = true;
		goto joypad_button;
	case SDL_JOYBUTTONUP:
		assert(event.jbutton.state == SDL_RELEASED);
		pressed = false;
	joypad_button:
		joypad = JS_BUTTON(event.jbutton.which, event.jbutton.button);
		manage_combos(megad, pressed, RCBJ, joypad);
		if (events != STARTED)
			break;
		if (calibrating) {
			if (pressed)
				manage_calibration(RCBJ, joypad);
			break;
		}
		for (struct ctl* ctl = control; (ctl->rc != NULL); ++ctl) {
			if ((*ctl->rc)[RCBJ] != joypad)
				continue;
			ctl->pressed = pressed;
			ctl->coord = false;
			if (pressed == false) {
				if ((ctl->release != NULL) &&
				    (ctl->release(*ctl, megad) == 0))
					return 0;
			}
			else {
				assert(ctl->press != NULL);
				if (ctl->press(*ctl, megad) == 0)
					return 0;
			}
		}
		if (manage_bindings(megad, pressed, RCBJ, joypad) == 0)
			return 0;
		break;
#endif // WITH_JOYSTICK
	case SDL_KEYDOWN:
		ksym = event.key.keysym.sym;
		ksym_uni = event.key.keysym.unicode;
		if ((ksym_uni < 0x20) ||
		    ((ksym >= SDLK_KP0) && (ksym <= SDLK_KP_EQUALS)))
			ksym_uni = 0;
		kpress[(ksym & 0xff)] = ksym_uni;
		if (ksym_uni)
			ksym = ksym_uni;
		else if (event.key.keysym.mod & KMOD_SHIFT)
			ksym |= KEYSYM_MOD_SHIFT;

		// Check for modifiers
		if (event.key.keysym.mod & KMOD_CTRL)
			ksym |= KEYSYM_MOD_CTRL;
		if (event.key.keysym.mod & KMOD_ALT)
			ksym |= KEYSYM_MOD_ALT;
		if (event.key.keysym.mod & KMOD_META)
			ksym |= KEYSYM_MOD_META;

		manage_combos(megad, true, RCBK, ksym);

		if (calibrating) {
			manage_calibration(RCBK, ksym);
			break;
		}

		switch (events) {
			int ret;

		case STARTED:
			break;
		case PROMPT:
		case STOPPED_PROMPT:
			ret = handle_prompt(ksym, ksym_uni, megad);
			if (ret & PROMPT_RET_ERROR) {
				restart_events(megad);
				return 0;
			}
			if (ret & PROMPT_RET_EXIT) {
				if (events == STOPPED_PROMPT) {
					// Return to stopped mode.
					pd_message(stopped_str);
					events = STOPPED;
					goto next_event;
				}
				if ((ret & PROMPT_RET_MSG) == 0)
					pd_message("RUNNING.");
				restart_events(megad);
				goto next_event;
			}
			if (ret & PROMPT_RET_ENTER) {
				// Back to the prompt only in stopped mode.
				if (events == STOPPED_PROMPT)
					goto next_event;
				if ((ret & PROMPT_RET_MSG) == 0)
					pd_message("");
				restart_events(megad);
				goto next_event;
			}
			// PROMPT_RET_CONT
			goto next_event;
		case GAME_GENIE:
		case STOPPED_GAME_GENIE:
			if (manage_game_genie(megad, ksym, ksym_uni) == 0)
				goto next_event;
			if (events == STOPPED_GAME_GENIE) {
				// Return to stopped mode.
				pd_message(stopped_str);
				events = STOPPED;
			}
			else
				restart_events(megad);
			goto next_event;
		case STOPPED:
			// In basic stopped mode, handle a few keysyms.
			if (ksym == dgen_game_genie[0]) {
				pd_message_cursor(strlen(game_genie_str),
						  game_genie_str);
				events = STOPPED_GAME_GENIE;
			}
			else if (ksym == dgen_prompt[0]) {
				pd_message_cursor(strlen(prompt_str),
						  prompt_str);
				events = STOPPED_PROMPT;
			}
			else if (ksym == dgen_quit[0]) {
				restart_events(megad);
				return 0;
			}
			else if (ksym == dgen_stop[0]) {
				pd_message("RUNNING.");
				restart_events(megad);
			}
#ifdef WITH_DEBUGGER
			else if (ksym == dgen_debug_enter[0])
				ctl_dgen_debug_enter(*(struct ctl *)0, megad);
#endif
		default:
			goto next_event;
		}

		for (struct ctl* ctl = control; (ctl->rc != NULL); ++ctl) {
			if (ksym != (*ctl->rc)[RCBK])
				continue;
			assert(ctl->press != NULL);
			ctl->pressed = true;
			ctl->coord = false;
			if (ctl->press(*ctl, megad) == 0)
				return 0;
		}
		if (manage_bindings(megad, true, RCBK, ksym) == 0)
			return 0;
		break;
	case SDL_KEYUP:
		ksym = event.key.keysym.sym;
		ksym_uni = kpress[(ksym & 0xff)];
		if ((ksym_uni < 0x20) ||
		    ((ksym >= SDLK_KP0) && (ksym <= SDLK_KP_EQUALS)))
			ksym_uni = 0;
		kpress[(ksym & 0xff)] = 0;
		if (ksym_uni)
			ksym = ksym_uni;

		manage_combos(megad, false, RCBK, ksym);
		manage_combos(megad, false, RCBK, (ksym | KEYSYM_MOD_ALT));
		manage_combos(megad, false, RCBK, (ksym | KEYSYM_MOD_SHIFT));
		manage_combos(megad, false, RCBK, (ksym | KEYSYM_MOD_CTRL));
		manage_combos(megad, false, RCBK, (ksym | KEYSYM_MOD_META));

		if (calibrating)
			break;
		if (events != STARTED)
			break;

		// The only time we care about key releases is for the
		// controls, but ignore key modifiers so they never get stuck.
		for (struct ctl* ctl = control; (ctl->rc != NULL); ++ctl) {
			if (ksym != ((*ctl->rc)[RCBK] & ~KEYSYM_MOD_MASK))
				continue;
			ctl->pressed = false;
			ctl->coord = false;
			if ((ctl->release != NULL) &&
			    (ctl->release(*ctl, megad) == 0))
				return 0;
		}
		if (manage_bindings(megad, false, RCBK, ksym) == 0)
			return 0;
		break;
	case SDL_MOUSEMOTION:
		if (!mouse_is_grabbed()) {
			// Only show mouse pointer for a few seconds.
			SDL_ShowCursor(1);
			hide_mouse_when = (pd_usecs() + MOUSE_SHOW_USECS);
			hide_mouse = true;
			break;
		}
	mouse_motion:
		which = event.motion.which;
		pi = 0;
		ri = 0;
		if (event.motion.xrel < 0) {
			plist[pi++] = MO_MOTION(which, 'l');
			rlist[ri++] = MO_MOTION(which, 'r');
		}
		else if (event.motion.xrel > 0) {
			plist[pi++] = MO_MOTION(which, 'r');
			rlist[ri++] = MO_MOTION(which, 'l');
		}
		else {
			rlist[ri++] = MO_MOTION(which, 'r');
			rlist[ri++] = MO_MOTION(which, 'l');
		}
		if (event.motion.yrel < 0) {
			plist[pi++] = MO_MOTION(which, 'u');
			rlist[ri++] = MO_MOTION(which, 'd');
		}
		else if (event.motion.yrel > 0) {
			plist[pi++] = MO_MOTION(which, 'd');
			rlist[ri++] = MO_MOTION(which, 'u');
		}
		else {
			rlist[ri++] = MO_MOTION(which, 'd');
			rlist[ri++] = MO_MOTION(which, 'u');
		}
		if (pi)
			mouse_motion_delay_release(which, true);
		else
			mouse_motion_delay_release(which, false);
		for (i = 0; (i != ri); ++i)
			manage_combos(megad, false, RCBM, rlist[i]);
		for (i = 0; (i != pi); ++i)
			manage_combos(megad, true, RCBM, plist[i]);
		if (calibrating) {
			for (i = 0; ((calibrating) && (i != pi)); ++i)
				manage_calibration(RCBM, plist[i]);
			break;
		}
		if (events != STARTED)
			break;
		for (struct ctl* ctl = control; (ctl->rc != NULL); ++ctl) {
			// Release buttons first.
			for (i = 0; (i != ri); ++i) {
				if ((ctl->pressed == false) ||
				    ((uint32_t)(*ctl->rc)[RCBM] != rlist[i]))
					continue;
				ctl->pressed = false;
				ctl->coord = true;
				ctl->x = event.motion.x;
				ctl->y = event.motion.y;
				if ((ctl->release != NULL) &&
				    (ctl->release(*ctl, megad) == 0))
					return 0;
			}
			for (i = 0; (i != pi); ++i) {
				if ((uint32_t)(*ctl->rc)[RCBM] == plist[i]) {
					assert(ctl->press != NULL);
					ctl->pressed = true;
					ctl->coord = true;
					ctl->x = event.motion.x;
					ctl->y = event.motion.y;
					if (ctl->press(*ctl, megad) == 0)
						return 0;
				}
			}
		}
		for (i = 0; (i != ri); ++i)
			if (!manage_bindings(megad, false, RCBM, rlist[i]))
				return 0;
		for (i = 0; (i != pi); ++i)
			if (!manage_bindings(megad, true, RCBM, plist[i]))
				return 0;
		break;
	case SDL_MOUSEBUTTONDOWN:
		assert(event.button.state == SDL_PRESSED);
#ifdef WITH_DEBUGGER
		if (!debug_trap)
#endif
			mouse_grab(true);
		pressed = true;
		goto mouse_button;
	case SDL_MOUSEBUTTONUP:
		assert(event.button.state == SDL_RELEASED);
		pressed = false;
	mouse_button:
		mouse = MO_BUTTON(event.button.which, event.button.button);
		manage_combos(megad, pressed, RCBM, mouse);
		if (calibrating) {
			if (pressed)
				manage_calibration(RCBM, mouse);
			break;
		}
		if (events != STARTED)
			break;
		for (struct ctl* ctl = control; (ctl->rc != NULL); ++ctl) {
			if ((*ctl->rc)[RCBM] != mouse)
				continue;
			ctl->pressed = pressed;
			ctl->coord = true;
			ctl->x = event.button.x;
			ctl->y = event.button.y;
			if (pressed == false) {
				if ((ctl->release != NULL) &&
				    (ctl->release(*ctl, megad) == 0))
					return 0;
			}
			else {
				assert(ctl->press != NULL);
				if (ctl->press(*ctl, megad) == 0)
					return 0;
			}
		}
		if (manage_bindings(megad, pressed, RCBM, mouse) == 0)
			return 0;
		break;
	case SDL_VIDEORESIZE:
		switch (screen_init(event.resize.w, event.resize.h)) {
		case 0:
			pd_message("Video resized to %ux%u.",
				   screen.surface->w,
				   screen.surface->h);
			break;
		case -1:
			pd_message("Failed to resize video to %ux%u.",
				   event.resize.w,
				   event.resize.h);
			break;
		default:
			fprintf(stderr,
				"sdl: fatal error while trying to change screen"
				" resolution.\n");
			return 0;
		}
		break;
	case SDL_QUIT:
		// We've been politely asked to exit, so let's leave
		return 0;
	default:
		break;
	}
	goto next_event;
}

static size_t pd_message_write(const char *msg, size_t len, unsigned int mark)
{
	uint8_t *buf = (screen.buf.u8 +
			(screen.pitch * (screen.height - screen.info_height)));
	size_t ret = 0;

	screen_lock();
	// Clear text area.
	memset(buf, 0x00, (screen.pitch * screen.info_height));
	// Write message.
	if (len != 0)
		ret = font_text(buf, screen.width, screen.info_height,
				screen.Bpp, screen.pitch, msg, len,
				mark, FONT_TYPE_AUTO);
	screen_unlock();
	return ret;
}

static size_t pd_message_display(const char *msg, size_t len,
				 unsigned int mark, bool update)
{
	size_t ret = pd_message_write(msg, len, mark);

	if (update)
		screen_update();
	if (len == 0)
		info.displayed = 0;
	else {
		info.displayed = 1;
		info.since = pd_usecs();
	}
	return ret;
}

/**
 * Process status bar message.
 */
static void pd_message_process(void)
{
	size_t len = info.length;
	size_t n;
	size_t r;

	if (len == 0) {
		pd_clear_message();
		return;
	}
	for (n = 0; (n < len); ++n)
		if (info.message[n] == '\n') {
			len = (n + 1);
			break;
		}
	r = pd_message_display(info.message, n, ~0u, false);
	if (r < n)
		len = r;
	memmove(info.message, &(info.message[len]), (info.length - len));
	info.length -= len;
}

/**
 * Postpone a message.
 */
static void pd_message_postpone(const char *msg)
{
	strncpy(&info.message[info.length], msg,
		(sizeof(info.message) - info.length));
	info.length = strlen(info.message);
	info.displayed = 1;
}

/**
 * Write a message to the status bar.
 */

void pd_message(const char *msg, ...)
{
	va_list vl;

	va_start(vl, msg);
	vsnprintf(info.message, sizeof(info.message), msg, vl);
	va_end(vl);
	info.length = strlen(info.message);
	pd_message_process();
}

void pd_clear_message()
{
	pd_message_display(NULL, 0, ~0u, false);
}

void pd_show_carthead(md& megad)
{
	struct {
		const char *p;
		const char *s;
		size_t len;
	} data[] = {
#define CE(i, s) { i, s, sizeof(s) }
		CE("System", megad.cart_head.system_name),
		CE("Copyright", megad.cart_head.copyright),
		CE("Domestic name", megad.cart_head.domestic_name),
		CE("Overseas name", megad.cart_head.overseas_name),
		CE("Product number", megad.cart_head.product_no),
		CE("Memo", megad.cart_head.memo),
		CE("Countries", megad.cart_head.countries)
	};
	size_t i;

	pd_message_postpone("\n");
	for (i = 0; (i < (sizeof(data) / sizeof(data[0]))); ++i) {
		char buf[256];
		size_t j, k;

		k = (size_t)snprintf(buf, sizeof(buf), "%s: ", data[i].p);
		if (k >= (sizeof(buf) - 1))
			continue;
		// Filter out extra spaces.
		for (j = 0; (j < data[i].len); ++j)
			if (isgraph(data[i].s[j]))
				break;
		if (j == data[i].len)
			continue;
		while ((j < data[i].len) && (k < (sizeof(buf) - 2))) {
			if (isgraph(data[i].s[j])) {
				buf[(k++)] = data[i].s[j];
				++j;
				continue;
			}
			buf[(k++)] = ' ';
			while ((j < data[i].len) && (!isgraph(data[i].s[j])))
				++j;
		}
		if (buf[(k - 1)] == ' ')
			--k;
		buf[k] = '\n';
		buf[(k + 1)] = '\0';
		pd_message_postpone(buf);
	}
}

/* Clean up this awful mess :) */
void pd_quit()
{
	size_t i;

#ifdef WITH_THREADS
	screen_update_thread_stop();
#endif
	if (mdscr.data) {
		free((void*)mdscr.data);
		mdscr.data = NULL;
	}
	SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	pd_sound_deinit();
	if (mdpal)
		mdpal = NULL;
#ifdef WITH_OPENGL
	release_texture(screen.texture);
#endif
	free(filters_stack_data_buf[0].u8);
	free(filters_stack_data_buf[1].u8);
	assert(filters_stack_size <= elemof(filters_stack));
	assert(filters_stack_data[0].data == NULL);
	filters_stack_default = false;
	for (i = 0; (i != filters_stack_size); ++i) {
		free(filters_stack_data[i + 1].data);
		filters_stack_data[i + 1].data = NULL;
	}
	filters_stack_size = 0;
	SDL_Quit();
}
