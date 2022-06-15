#ifndef __PD_H__
#define __PD_H__

#include <stdio.h>
#include <stdlib.h>
#include "md.h"

// DGen/SDL v1.17+
// Platform-dependent interface
// Your platform implementation needs to define all these functions and
// variables!

// Return the number of microseconds elapsed since an unspecified time.
unsigned long pd_usecs(void);
// This is the struct bmap setup by your implementation.
// It should be 336x240 (or 336x256 in PAL mode), in 8, 12, 15, 16, 24 or 32
// bits-per-pixel.
extern struct bmap mdscr;
// Also, you should allocate a 256-char palette array, if need be. Otherwise
// this can be NULL if you don't have a paletted display.
extern unsigned char *mdpal;
// Initialize graphics, in NTSC (320x224) or PAL (320x240) mode.
// Since many interfaces require that DGen be setuid-root, this should also
// discard root priviledges, if at all necessary.
// It should return 1 on successful setup, or 0 if something wrong happened.
int pd_graphics_init(int want_sound, int want_pal, int hz);
int pd_graphics_reinit(int want_sound, int want_pal, int hz);
// This updats the palette, if necessary.
void pd_graphics_palette_update();
// This updates the screen, with the mdscr bitmap.
void pd_graphics_update(bool update);

// This is the struct sndinfo, also setup by your implementation.
// Note that the buffers pointed to in this struct should ALWAYS be 16-bit
// signed format, regardless of the actual audio format.
extern struct sndinfo sndi;
// Initialize sound, with the given frequency and number of samples.
// It should keep samples' worth of sound buffered.
// The parameters should all be modified to reflect the actual characteristics.
// This is always called after pd_graphics_init, so you can count on graphics
// stuff being initialized. :)
// It should return 1 on successful setup, or 0 if something wrong happened.
int pd_sound_init(long &freq, unsigned int &samples);
void pd_sound_deinit();
// This should return samples read/write indices in the buffer.
unsigned int pd_sound_rp();
unsigned int pd_sound_wp();
// And this function is called to commit the sound buffers to be played.
void pd_sound_write();

// Register platform-specific rc variables
void pd_rc();
// This should be a list of all the command-line options specific to this
// platform, in the form given to getopt(3), i.e "a:b::c".
extern const char *pd_options;
// And, this is called to handle platform-specific stuff.
void pd_option(char c, const char *optarg);

// This is called after displaying the base options for help, so that you may
// document your platform-specific command-line options etc.
void pd_help();

// This is called before each frame to handle events, and update the MegaDrive
// accordingly. It returns 1 to continue playing the game, or 0 to quit.
int pd_handle_events(md &megad);

// Tells whether DGen stopped intentionally so emulation can resume without
// skipping frames.
int pd_stopped();

// If true, stop emulation (display last frame repeatedly).
extern bool pd_freeze;

// These are called to display and clear game messages.
void pd_message(const char *fmt, ...);
void pd_clear_message();
// This should display cartridge header info. You can do this any way you like,
// I don't care. :)
void pd_show_carthead(md &megad);

// This should clean up the mess you made. ;)
void pd_quit();

#endif // __PD_H__
