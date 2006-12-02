/*
	mpg123: main code of the program (not of the decoder...)

	copyright 1995-2006 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.de
	initially written by Michael Hipp

	mpg123 defines 
	used source: musicout.h from mpegaudio package
*/

#ifdef __cplusplus
extern "C" {
#endif

#include        <stdio.h>
#include        <math.h>
#ifndef _AUDIO_H_
#define _AUDIO_H_

typedef unsigned char byte;
#define off_t long

//#define I486_OPT 1

#define SKIP_JUNK 1
# define M_PI       3.14159265358979323846
# define M_SQRT2	1.41421356237309504880
# define REAL_IS_FLOAT
# define NEW_DCT9

#ifdef REAL_IS_FLOAT
#  define real float
#  define REAL_SCANF "%f"
#  define REAL_PRINTF "%f"
#else
#  define real double
#  define REAL_SCANF "%lf"
#  define REAL_PRINTF "%f"
#endif

#ifndef DOUBLE_TO_REAL
# define DOUBLE_TO_REAL(x)     (x)
#endif
#ifndef REAL_TO_SHORT
# define REAL_TO_SHORT(x)      (x)
#endif
#ifndef REAL_PLUS_32767
# define REAL_PLUS_32767       32767.0
#endif
#ifndef REAL_MINUS_32768
# define REAL_MINUS_32768      -32768.0
#endif
#ifndef REAL_MUL
# define REAL_MUL(x, y)                ((x) * (y))
#endif

#define INLINE
/* AUDIOBUFSIZE = n*64 with n=1,2,3 ...  */
#define		AUDIOBUFSIZE		16384

#define         FALSE                   0
#define         TRUE                    1

#define         MAX_NAME_SIZE           81
#define         SBLIMIT                 32
#define         SCALE_BLOCK             12
#define         SSLIMIT                 18

#define         MPG_MD_STEREO           0
#define         MPG_MD_JOINT_STEREO     1
#define         MPG_MD_DUAL_CHANNEL     2
#define         MPG_MD_MONO             3

/* I suspect that 32767 would be a better idea here, but Michael put this in... */
#define MAXOUTBURST 32768

/* Pre Shift fo 16 to 8 bit converter table */
#define AUSHIFT (3)

struct bitstream_info
{  int bitindex;
    unsigned char *wordpointer;
};
extern struct bitstream_info bsi;

struct reader
{  char *hFile;
    unsigned char *buffer;
    unsigned char *stream;
    int strpos;
    int strremain;
    int filelen;
    int filepos;

    int  (*head_read)(struct reader *,unsigned long *newhead);
    int  (*read_frame_body)(struct reader *,unsigned char *,int size);
 };

struct al_table 
{ short bits;
    short d;
};

struct frame {
    struct al_table *alloc;
    int (*synth)(real *,int,unsigned char *,int *);
    int (*synth_mono)(real *,unsigned char *,int *);
    int stereo; /* I _think_ 1 for mono and 2 for stereo */
    int jsbound;
    int single;
    int II_sblimit;
    int down_sample_sblimit;
    int lsf; /* 0: MPEG 1.0; 1: MPEG 2.0/2.5 -- both used as bool and array index! */
    int mpeg25;
    int down_sample;
    int header_change;
    int lay;
    int (*do_layer)(struct frame *fr,byte *pcm_out, int *pcm_size);
    int error_protection;
    int bitrate_index;
    int sampling_frequency;
    int padding;
    int extension;
    int mode;
    int mode_ext;
    int copyright;
    int original;
    int emphasis;
    int framesize; /* computed framesize */
    int vbr; /* 1 if variable bitrate was detected */
		unsigned long num; /* the nth frame in some stream... */
};

#if 0
struct parameter {
  int aggressive; /* renice to max. priority */
  int shuffle;	/* shuffle/random play */
  int remote;	/* remote operation */
  int remote_err;	/* remote operation to stderr */
  int outmode;	/* where to out the decoded sampels */
  int quiet;	/* shut up! */
  int xterm_title;	/* Change xterm title to song names? */
  long usebuffer;	/* second level buffer size */
  int tryresync;  /* resync stream after error */
  int verbose;    /* verbose level */
  int force_mono;
  int force_stereo;
  int force_8bit;
  long force_rate;
  int down_sample;
  int checkrange;
  long doublespeed;
  long halfspeed;
  int force_reopen;
  long realtime;
  char filename[256];
  long listentry; /* possibility to choose playback of one entry in playlist (0: off, > 0 : select, < 0; just show list*/
  int rva; /* (which) rva to do: <0: nothing, 0: radio/mix/track 1: album/audiophile */
  char* listname; /* name of playlist */
  int long_id3;
};
#endif

#if 0
struct reader {
  int  (*init)(struct reader *);
  void (*close)(struct reader *);
  int  (*head_read)(struct reader *,unsigned long *newhead);
  int  (*head_shift)(struct reader *,unsigned long *head);
  long  (*skip_bytes)(struct reader *,off_t len);
  int  (*read_frame_body)(struct reader *,unsigned char *,int size);
  int  (*back_bytes)(struct reader *,off_t bytes);
  int  (*back_frame)(struct reader *,struct frame *,long num);
  off_t (*tell)(struct reader *);
  void (*rewind)(struct reader *);
  off_t filelen;
  off_t filepos;
  int  filept;
  int  flags;
  unsigned char id3buf[128];
};
#endif

#define READER_FD_OPENED 0x1
#define READER_ID3TAG    0x2
#define READER_SEEKABLE  0x4

//extern void audio_flush(int, struct audio_info_struct *);

//extern void print_header(struct frame *);
//extern void print_header_compact(struct frame *);
//extern void print_id3_tag(unsigned char *buf);

//extern int split_dir_file(const char *path, char **dname, char **fname);

extern unsigned int   get1bit(void);
extern unsigned int   getbits(int);
extern unsigned int   getbits_fast(int);
//extern void           backbits(int);
//extern int            getbitoffset(void);
//extern int            getbyte(void);

//extern void set_pointer(long);

//extern unsigned char *pcm_sample;
//extern int pcm_point;
//extern int audiobufsize;

//extern int OutputDescriptor;

#ifdef VARMODESUPPORT
extern int varmode;
extern int playlimit;
#endif

struct gr_info_s {
      int scfsi;
      unsigned part2_3_length;
      unsigned big_values;
      unsigned scalefac_compress;
      unsigned block_type;
      unsigned mixed_block_flag;
      unsigned table_select[3];
      unsigned subblock_gain[3];
      unsigned maxband[3];
      unsigned maxbandl;
      unsigned maxb;
      unsigned region1start;
      unsigned region2start;
      unsigned preflag;
      unsigned scalefac_scale;
      unsigned count1table_select;
      real *full_gain[3];
      real *pow2gain;
};

struct III_sideinfo
{
  unsigned main_data_begin;
  unsigned private_bits;
  struct {
    struct gr_info_s gr[2];
  } ch[2];
};

extern int open_stream(char *,int fd);
extern void read_frame_init (struct frame* fr);
int read_frame(struct reader *rd, struct frame *fr);


/* why extern? */
void prepare_audioinfo(struct frame *fr, struct audio_info_struct *nai);
int play_frame(int init,struct frame *fr);
int do_layer1(struct frame *fr,byte *pcm_sample, int *pcm_point);
int do_layer2(struct frame *fr,byte *pcm_sample, int *pcm_point);
int do_layer3(struct frame *fr,byte *pcm_sample, int *pcm_point);
extern void do_equalizer(real *bandPtr,int channel);

#ifdef PENTIUM_OPT
extern int synth_1to1_pent (real *,int,unsigned char *);
#endif
extern int synth_1to1 (real *,int,unsigned char *,int *);
extern int synth_1to1_8bit (real *,int,unsigned char *,int *);
extern int synth_1to1_mono (real *,unsigned char *,int *);
extern int synth_1to1_mono2stereo (real *,unsigned char *,int *);
extern int synth_1to1_8bit_mono (real *,unsigned char *,int *);
extern int synth_1to1_8bit_mono2stereo (real *,unsigned char *,int *);

extern int synth_2to1 (real *,int,unsigned char *,int *);
extern int synth_2to1_8bit (real *,int,unsigned char *,int *);
extern int synth_2to1_mono (real *,unsigned char *,int *);
extern int synth_2to1_mono2stereo (real *,unsigned char *,int *);
extern int synth_2to1_8bit_mono (real *,unsigned char *,int *);
extern int synth_2to1_8bit_mono2stereo (real *,unsigned char *,int *);

extern int synth_4to1 (real *,int,unsigned char *,int *);
extern int synth_4to1_8bit (real *,int,unsigned char *,int *);
extern int synth_4to1_mono (real *,unsigned char *,int *);
extern int synth_4to1_mono2stereo (real *,unsigned char *,int *);
extern int synth_4to1_8bit_mono (real *,unsigned char *,int *);
extern int synth_4to1_8bit_mono2stereo (real *,unsigned char *,int *);

extern int synth_ntom (real *,int,unsigned char *,int *);
extern int synth_ntom_8bit (real *,int,unsigned char *,int *);
extern int synth_ntom_mono (real *,unsigned char *,int *);
extern int synth_ntom_mono2stereo (real *,unsigned char *,int *);
extern int synth_ntom_8bit_mono (real *,unsigned char *,int *);
extern int synth_ntom_8bit_mono2stereo (real *,unsigned char *,int *);

extern void rewindNbits(int bits);
extern int  hsstell(void);
extern void set_pointer(long);
extern void huffman_decoder(int ,int *);
extern void huffman_count1(int,int *);
extern void print_stat(struct frame *fr,unsigned long no,long buffsize,struct audio_info_struct *ai);
extern int get_songlen(struct frame *fr,int no);

extern void init_layer3(int);
extern void init_layer2(void);
extern void make_decode_tables(long scale);
extern int make_conv16to8_table(int);
extern void dct64(real *,real *,real *);

#ifdef USE_MMX
extern void dct64_MMX(short *a,short *b,real *c);
extern int synth_1to1_MMX(real *, int, short *, short *, int *);
#endif

extern int synth_ntom_set_step(long,long);


extern unsigned char *conv16to8;
extern long freqs[9];
extern real muls[27][64];
extern real decwin[512+32];
#ifndef USE_MMX
extern real *pnts[5];
#endif

extern real equalizer[2][32];
extern real equalizer_sum[2][32];
extern int equalizer_cnt;

extern struct audio_name audio_val2name[];

//extern struct parameter param;

/* 486 optimizations */
#define FIR_BUFFER_SIZE  128
extern void dct64_486(int *a,int *b,real *c);
extern int synth_1to1_486(real *bandPtr,int channel,unsigned char *out,int nb_blocks);

/* 3DNow! optimizations */
#ifdef USE_3DNOW
extern int getcpuflags(void);
extern void dct36(real *,real *,real *,real *,real *);
extern void dct36_3dnow(real *,real *,real *,real *,real *);
extern int synth_1to1_3dnow(real *,int,unsigned char *,int *);
#endif

/* avoid the SIGINT in terminal control */
void next_track(void);
extern long outscale;

#endif

void set_pointer(long backstep);
int __stdcall create_reader(struct reader *rd,byte *buffer, int buffsize);
int __stdcall init_reader(struct reader *rd, char *file);
int __stdcall decode_header(struct frame *fr,unsigned long newhead);
int __stdcall set_reader(struct reader *rd, unsigned int filepos);
double pow_test(double, double);
void * __cdecl mem_cpy(void * dst,const void * src,size_t count);

#ifdef __cplusplus
}
#endif
