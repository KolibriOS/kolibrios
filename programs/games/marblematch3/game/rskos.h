#ifndef RS_KOS_H
#define RS_KOS_H

// KolibriOS Stuff
// by Roman Shuvalov

unsigned int rskos_get_time();

#define RSKOS_BGRA      32
#define RSKOS_BGR       24

void rskos_draw_area(int x, int y, int w, int h, int k_scale, unsigned char *data, unsigned char *scaled_buffer, int image_format);

void rskos_resize_window(int w, int h);
void rskos_get_screen_size(unsigned int *pw, unsigned int *ph);

void rskos_exit();

// files

int rskos_file_save(char *filename, unsigned char *data, int length);
int rskos_file_load(char *filename, unsigned char *data, int length);

// sound

#ifndef SNDBUF
    #ifndef RS_KOS
        #include "rs/rsaudio.h"
        typedef rs_sound_t* SNDBUF;
    #else 
        typedef unsigned int SNDBUF;
    #endif
#endif

//void rskos_snd_init();
void rskos_snd_create_buffer(SNDBUF *phbuf, signed short *buffer, unsigned int length_samples);
void rskos_snd_update_buffer(SNDBUF *phbuf, signed short *buffer, unsigned int length_samples);
void rskos_snd_play(SNDBUF *phbuf, unsigned int mode);
void rskos_snd_stop(SNDBUF *phbuf);

#endif
