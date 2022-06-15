// DGen/SDL v1.16+
// New raster effects engine
// I'd like to thank the Mac folks for giving me a good template to work from.
// This is just a cheap rehash of their code, except friendlier to other bit
// depths. :) I also put in a few little optimizations, like blank checking
// and especially the sprites.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "system.h"
#include "md.h"
#include "pd.h"
#include "rc-vars.h"

// This is marked each time the palette is updated. Handy for the 8bpp
// implementation, so we don't waste time changing the palette unnecessarily.
int pal_dirty;

// Macros, to route draw_tile and draw_tile_solid to the right handler
#define draw_tile(which, line, where) \
	switch(Bpp)\
	  {\
	  case 1:\
	    draw_tile1((which),(line),(where)); break;\
	  case 2:\
	    draw_tile2((which),(line),(where)); break;\
	  case 3:\
	    draw_tile3((which),(line),(where)); break;\
	  case 4:\
	    draw_tile4((which),(line),(where)); break;\
	  }

#define draw_tile_solid(which, line, where) \
	switch(Bpp)\
	  {\
	  case 1:\
	    draw_tile1_solid((which),(line),(where)); break;\
	  case 2:\
	    draw_tile2_solid((which),(line),(where)); break;\
	  case 3:\
	    draw_tile3_solid((which),(line),(where)); break;\
	  case 4:\
	    draw_tile4_solid((which),(line),(where)); break;\
	  }

// Silly utility function, get a big-endian word
#ifdef WORDS_BIGENDIAN
static inline int get_word(unsigned char *where)
  { return (int)(*(unsigned short*)where); }
#else
static inline int get_word(unsigned char *where)
  { return (where[0] << 8) | where[1]; }
#endif

// Tile pixel masks
#ifdef WORDS_BIGENDIAN
#  define PIXEL0 (0xf0000000)
#  define PIXEL1 (0x0f000000)
#  define PIXEL2 (0x00f00000)
#  define PIXEL3 (0x000f0000)
#  define PIXEL4 (0x0000f000)
#  define PIXEL5 (0x00000f00)
#  define PIXEL6 (0x000000f0)
#  define PIXEL7 (0x0000000f)
#  define SHIFT0 (28)
#  define SHIFT1 (24)
#  define SHIFT2 (20)
#  define SHIFT3 (16)
#  define SHIFT4 (12)
#  define SHIFT5 ( 8)
#  define SHIFT6 ( 4)
#  define SHIFT7 ( 0)
#else // WORDS_BIGENDIAN
#  define PIXEL0 (0x000000f0)
#  define PIXEL1 (0x0000000f)
#  define PIXEL2 (0x0000f000)
#  define PIXEL3 (0x00000f00)
#  define PIXEL4 (0x00f00000)
#  define PIXEL5 (0x000f0000)
#  define PIXEL6 (0xf0000000)
#  define PIXEL7 (0x0f000000)
#  define SHIFT0 ( 4)
#  define SHIFT1 ( 0)
#  define SHIFT2 (12)
#  define SHIFT3 ( 8)
#  define SHIFT4 (20)
#  define SHIFT5 (16)
#  define SHIFT6 (28)
#  define SHIFT7 (24)
#endif // WORDS_BIGENDIAN

#ifdef WITH_X86_TILES
extern "C" {

void asm_tiles_init(unsigned char *vram,
		    unsigned char *reg,
		    unsigned *highpal);

void drawtile1(int which, int line, unsigned char *where);
void drawtile1_solid(int which, int line, unsigned char *where);
void drawtile2(int which, int line, unsigned char *where);
void drawtile2_solid(int which, int line, unsigned char *where);
void drawtile3(int which, int line, unsigned char *where);
void drawtile3_solid(int which, int line, unsigned char *where);
void drawtile4(int which, int line, unsigned char *where);
void drawtile4_solid(int which, int line, unsigned char *where);
}

// Pass off these calls to assembler counterparts
inline void md_vdp::draw_tile1_solid(int which, int line, unsigned char *where)
  { drawtile1_solid(which, line, where); }

inline void md_vdp::draw_tile1(int which, int line, unsigned char *where)
  { drawtile1(which, line, where); }

inline void md_vdp::draw_tile2_solid(int which, int line, unsigned char *where)
  { drawtile2_solid(which, line, where); }

inline void md_vdp::draw_tile2(int which, int line, unsigned char *where)
  { drawtile2(which, line, where); }

inline void md_vdp::draw_tile3_solid(int which, int line, unsigned char *where)
  { drawtile3_solid(which, line, where); }

inline void md_vdp::draw_tile3(int which, int line, unsigned char *where)
  { drawtile3(which, line, where); }

inline void md_vdp::draw_tile4_solid(int which, int line, unsigned char *where)
  { drawtile4_solid(which, line, where); }

inline void md_vdp::draw_tile4(int which, int line, unsigned char *where)
  { drawtile4(which, line, where); }

#else // WITH_X86_TILES

static bool has_zero_nibbles(uint32_t u32)
{
	return ((u32 - 0x11111111) & ~u32 & 0x88888888);
}

// Blit tile solidly, for 1 byte-per-pixel
inline void md_vdp::draw_tile1_solid(int which, int line, unsigned char *where)
{
  unsigned tile, pal;

  pal = (which >> 9 & 0x30); // Determine which 16-color palette

  if(which & 0x1000) // y flipped
    line ^= 7; // take from the bottom, instead of the top

  if(reg[12] & 2) // interlace
    tile = *(unsigned*)(vram + ((which&0x7ff) << 6) + (line << 3));
  else
    tile = *(unsigned*)(vram + ((which&0x7ff) << 5) + (line << 2));

  // Blit the tile!
  if(which & 0x800) // x flipped
    {
      *(where  ) = ((tile & PIXEL7)>>SHIFT7) | pal;
      *(where+1) = ((tile & PIXEL6)>>SHIFT6) | pal;
      *(where+2) = ((tile & PIXEL5)>>SHIFT5) | pal;
      *(where+3) = ((tile & PIXEL4)>>SHIFT4) | pal;
      *(where+4) = ((tile & PIXEL3)>>SHIFT3) | pal;
      *(where+5) = ((tile & PIXEL2)>>SHIFT2) | pal;
      *(where+6) = ((tile & PIXEL1)>>SHIFT1) | pal;
      *(where+7) = ((tile & PIXEL0)>>SHIFT0) | pal;
    } else {
      *(where  ) = ((tile & PIXEL0)>>SHIFT0) | pal;
      *(where+1) = ((tile & PIXEL1)>>SHIFT1) | pal;
      *(where+2) = ((tile & PIXEL2)>>SHIFT2) | pal;
      *(where+3) = ((tile & PIXEL3)>>SHIFT3) | pal;
      *(where+4) = ((tile & PIXEL4)>>SHIFT4) | pal;
      *(where+5) = ((tile & PIXEL5)>>SHIFT5) | pal;
      *(where+6) = ((tile & PIXEL6)>>SHIFT6) | pal;
      *(where+7) = ((tile & PIXEL7)>>SHIFT7) | pal;
    }
}

// Blit tile, leaving color zero transparent, for 1 byte per pixel
inline void md_vdp::draw_tile1(int which, int line, unsigned char *where)
{
  unsigned tile, pal;

  pal = (which >> 9 & 0x30); // Determine which 16-color palette

  if(which & 0x1000) // y flipped
    line ^= 7; // take from the bottom, instead of the top

  if(reg[12] & 2) // interlace
    tile = *(unsigned*)(vram + ((which&0x7ff) << 6) + (line << 3));
  else
    tile = *(unsigned*)(vram + ((which&0x7ff) << 5) + (line << 2));
  // If the tile is all 0's, why waste the time?
  if(!tile) return;

  // If the tile doesn't have any transparent pixels, draw it solidly.
  if (!has_zero_nibbles(tile)) {
    if (which & 0x800) {
      // x flipped
      *(where  ) = ((tile & PIXEL7)>>SHIFT7) | pal;
      *(where+1) = ((tile & PIXEL6)>>SHIFT6) | pal;
      *(where+2) = ((tile & PIXEL5)>>SHIFT5) | pal;
      *(where+3) = ((tile & PIXEL4)>>SHIFT4) | pal;
      *(where+4) = ((tile & PIXEL3)>>SHIFT3) | pal;
      *(where+5) = ((tile & PIXEL2)>>SHIFT2) | pal;
      *(where+6) = ((tile & PIXEL1)>>SHIFT1) | pal;
      *(where+7) = ((tile & PIXEL0)>>SHIFT0) | pal;
    }
    else {
      *(where  ) = ((tile & PIXEL0)>>SHIFT0) | pal;
      *(where+1) = ((tile & PIXEL1)>>SHIFT1) | pal;
      *(where+2) = ((tile & PIXEL2)>>SHIFT2) | pal;
      *(where+3) = ((tile & PIXEL3)>>SHIFT3) | pal;
      *(where+4) = ((tile & PIXEL4)>>SHIFT4) | pal;
      *(where+5) = ((tile & PIXEL5)>>SHIFT5) | pal;
      *(where+6) = ((tile & PIXEL6)>>SHIFT6) | pal;
      *(where+7) = ((tile & PIXEL7)>>SHIFT7) | pal;
    }
    return;
  }

  // Blit the tile!
  if(which & 0x800) // x flipped
    {
      if(tile & PIXEL7) *(where  ) = ((tile & PIXEL7)>>SHIFT7) | pal;
      if(tile & PIXEL6) *(where+1) = ((tile & PIXEL6)>>SHIFT6) | pal;
      if(tile & PIXEL5) *(where+2) = ((tile & PIXEL5)>>SHIFT5) | pal;
      if(tile & PIXEL4) *(where+3) = ((tile & PIXEL4)>>SHIFT4) | pal;
      if(tile & PIXEL3) *(where+4) = ((tile & PIXEL3)>>SHIFT3) | pal;
      if(tile & PIXEL2) *(where+5) = ((tile & PIXEL2)>>SHIFT2) | pal;
      if(tile & PIXEL1) *(where+6) = ((tile & PIXEL1)>>SHIFT1) | pal;
      if(tile & PIXEL0) *(where+7) = ((tile & PIXEL0)>>SHIFT0) | pal;
    } else {
      if(tile & PIXEL0) *(where  ) = ((tile & PIXEL0)>>SHIFT0) | pal;
      if(tile & PIXEL1) *(where+1) = ((tile & PIXEL1)>>SHIFT1) | pal;
      if(tile & PIXEL2) *(where+2) = ((tile & PIXEL2)>>SHIFT2) | pal;
      if(tile & PIXEL3) *(where+3) = ((tile & PIXEL3)>>SHIFT3) | pal;
      if(tile & PIXEL4) *(where+4) = ((tile & PIXEL4)>>SHIFT4) | pal;
      if(tile & PIXEL5) *(where+5) = ((tile & PIXEL5)>>SHIFT5) | pal;
      if(tile & PIXEL6) *(where+6) = ((tile & PIXEL6)>>SHIFT6) | pal;
      if(tile & PIXEL7) *(where+7) = ((tile & PIXEL7)>>SHIFT7) | pal;
    }
}

// Blit tile solidly, for 2 byte-per-pixel
inline void md_vdp::draw_tile2_solid(int which, int line, unsigned char *where)
{
  unsigned tile, temp, *pal;
  unsigned short *wwhere = (unsigned short*)where;

  pal = highpal + (which >> 9 & 0x30); // Determine which 16-color palette
  temp = *pal; *pal = highpal[reg[7]&0x3f]; // Get background color

  if(which & 0x1000) // y flipped
    line ^= 7; // take from the bottom, instead of the top

  if(reg[12] & 2) // interlace
    tile = *(unsigned*)(vram + ((which&0x7ff) << 6) + (line << 3));
  else
    tile = *(unsigned*)(vram + ((which&0x7ff) << 5) + (line << 2));

  // Blit the tile!
  if(which & 0x800) // x flipped
    {
      *(wwhere  ) = pal[((tile & PIXEL7)>>SHIFT7)];
      *(wwhere+1) = pal[((tile & PIXEL6)>>SHIFT6)];
      *(wwhere+2) = pal[((tile & PIXEL5)>>SHIFT5)];
      *(wwhere+3) = pal[((tile & PIXEL4)>>SHIFT4)];
      *(wwhere+4) = pal[((tile & PIXEL3)>>SHIFT3)];
      *(wwhere+5) = pal[((tile & PIXEL2)>>SHIFT2)];
      *(wwhere+6) = pal[((tile & PIXEL1)>>SHIFT1)];
      *(wwhere+7) = pal[((tile & PIXEL0)>>SHIFT0)];
    } else {
      *(wwhere  ) = pal[((tile & PIXEL0)>>SHIFT0)];
      *(wwhere+1) = pal[((tile & PIXEL1)>>SHIFT1)];
      *(wwhere+2) = pal[((tile & PIXEL2)>>SHIFT2)];
      *(wwhere+3) = pal[((tile & PIXEL3)>>SHIFT3)];
      *(wwhere+4) = pal[((tile & PIXEL4)>>SHIFT4)];
      *(wwhere+5) = pal[((tile & PIXEL5)>>SHIFT5)];
      *(wwhere+6) = pal[((tile & PIXEL6)>>SHIFT6)];
      *(wwhere+7) = pal[((tile & PIXEL7)>>SHIFT7)];
    }
  // Restore the original color
  *pal = temp;
}

// Blit tile, leaving color zero transparent, for 2 byte per pixel
inline void md_vdp::draw_tile2(int which, int line, unsigned char *where)
{
  unsigned tile, *pal;
  unsigned short *wwhere = (unsigned short*)where;

  pal = highpal + (which >> 9 & 0x30); // Determine which 16-color palette

  if(which & 0x1000) // y flipped
    line ^= 7; // take from the bottom, instead of the top

  if(reg[12] & 2) // interlace
    tile = *(unsigned*)(vram + ((which&0x7ff) << 6) + (line << 3));
  else
    tile = *(unsigned*)(vram + ((which&0x7ff) << 5) + (line << 2));
  // If the tile is all 0's, why waste the time?
  if(!tile) return;

  // If the tile doesn't have any transparent pixels, draw it solidly.
  if (!has_zero_nibbles(tile)) {
    if (which & 0x800) {
      // x flipped
      *(wwhere  ) = pal[((tile & PIXEL7)>>SHIFT7)];
      *(wwhere+1) = pal[((tile & PIXEL6)>>SHIFT6)];
      *(wwhere+2) = pal[((tile & PIXEL5)>>SHIFT5)];
      *(wwhere+3) = pal[((tile & PIXEL4)>>SHIFT4)];
      *(wwhere+4) = pal[((tile & PIXEL3)>>SHIFT3)];
      *(wwhere+5) = pal[((tile & PIXEL2)>>SHIFT2)];
      *(wwhere+6) = pal[((tile & PIXEL1)>>SHIFT1)];
      *(wwhere+7) = pal[((tile & PIXEL0)>>SHIFT0)];
    }
    else {
      *(wwhere  ) = pal[((tile & PIXEL0)>>SHIFT0)];
      *(wwhere+1) = pal[((tile & PIXEL1)>>SHIFT1)];
      *(wwhere+2) = pal[((tile & PIXEL2)>>SHIFT2)];
      *(wwhere+3) = pal[((tile & PIXEL3)>>SHIFT3)];
      *(wwhere+4) = pal[((tile & PIXEL4)>>SHIFT4)];
      *(wwhere+5) = pal[((tile & PIXEL5)>>SHIFT5)];
      *(wwhere+6) = pal[((tile & PIXEL6)>>SHIFT6)];
      *(wwhere+7) = pal[((tile & PIXEL7)>>SHIFT7)];
    }
    return;
  }

  // Blit the tile!
  if(which & 0x800) // x flipped
    {
      if(tile & PIXEL7) *(wwhere  ) = pal[((tile & PIXEL7)>>SHIFT7)];
      if(tile & PIXEL6) *(wwhere+1) = pal[((tile & PIXEL6)>>SHIFT6)];
      if(tile & PIXEL5) *(wwhere+2) = pal[((tile & PIXEL5)>>SHIFT5)];
      if(tile & PIXEL4) *(wwhere+3) = pal[((tile & PIXEL4)>>SHIFT4)];
      if(tile & PIXEL3) *(wwhere+4) = pal[((tile & PIXEL3)>>SHIFT3)];
      if(tile & PIXEL2) *(wwhere+5) = pal[((tile & PIXEL2)>>SHIFT2)];
      if(tile & PIXEL1) *(wwhere+6) = pal[((tile & PIXEL1)>>SHIFT1)];
      if(tile & PIXEL0) *(wwhere+7) = pal[((tile & PIXEL0)>>SHIFT0)];
    } else {
      if(tile & PIXEL0) *(wwhere  ) = pal[((tile & PIXEL0)>>SHIFT0)];
      if(tile & PIXEL1) *(wwhere+1) = pal[((tile & PIXEL1)>>SHIFT1)];
      if(tile & PIXEL2) *(wwhere+2) = pal[((tile & PIXEL2)>>SHIFT2)];
      if(tile & PIXEL3) *(wwhere+3) = pal[((tile & PIXEL3)>>SHIFT3)];
      if(tile & PIXEL4) *(wwhere+4) = pal[((tile & PIXEL4)>>SHIFT4)];
      if(tile & PIXEL5) *(wwhere+5) = pal[((tile & PIXEL5)>>SHIFT5)];
      if(tile & PIXEL6) *(wwhere+6) = pal[((tile & PIXEL6)>>SHIFT6)];
      if(tile & PIXEL7) *(wwhere+7) = pal[((tile & PIXEL7)>>SHIFT7)];
    }
}

inline void md_vdp::draw_tile3_solid(int which, int line, unsigned char *where)
{
  unsigned tile, temp, *pal;
  uint24_t *wwhere = (uint24_t *)where;

  pal = highpal + (which >> 9 & 0x30); // Determine which 16-color palette
  temp = *pal; *pal = highpal[reg[7]&0x3f]; // Get background color

  if(which & 0x1000) // y flipped
    line ^= 7; // take from the bottom, instead of the top

  if(reg[12] & 2) // interlace
    tile = *(unsigned*)(vram + ((which&0x7ff) << 6) + (line << 3));
  else
    tile = *(unsigned*)(vram + ((which&0x7ff) << 5) + (line << 2));

  // Blit the tile!
  if(which & 0x800) // x flipped
    {
      u24cpy(&wwhere[0], (uint24_t *)&pal[((tile & PIXEL7) >> SHIFT7)]);
      u24cpy(&wwhere[1], (uint24_t *)&pal[((tile & PIXEL6) >> SHIFT6)]);
      u24cpy(&wwhere[2], (uint24_t *)&pal[((tile & PIXEL5) >> SHIFT5)]);
      u24cpy(&wwhere[3], (uint24_t *)&pal[((tile & PIXEL4) >> SHIFT4)]);
      u24cpy(&wwhere[4], (uint24_t *)&pal[((tile & PIXEL3) >> SHIFT3)]);
      u24cpy(&wwhere[5], (uint24_t *)&pal[((tile & PIXEL2) >> SHIFT2)]);
      u24cpy(&wwhere[6], (uint24_t *)&pal[((tile & PIXEL1) >> SHIFT1)]);
      u24cpy(&wwhere[7], (uint24_t *)&pal[((tile & PIXEL0) >> SHIFT0)]);
    } else {
      u24cpy(&wwhere[0], (uint24_t *)&pal[((tile & PIXEL0) >> SHIFT0)]);
      u24cpy(&wwhere[1], (uint24_t *)&pal[((tile & PIXEL1) >> SHIFT1)]);
      u24cpy(&wwhere[2], (uint24_t *)&pal[((tile & PIXEL2) >> SHIFT2)]);
      u24cpy(&wwhere[3], (uint24_t *)&pal[((tile & PIXEL3) >> SHIFT3)]);
      u24cpy(&wwhere[4], (uint24_t *)&pal[((tile & PIXEL4) >> SHIFT4)]);
      u24cpy(&wwhere[5], (uint24_t *)&pal[((tile & PIXEL5) >> SHIFT5)]);
      u24cpy(&wwhere[6], (uint24_t *)&pal[((tile & PIXEL6) >> SHIFT6)]);
      u24cpy(&wwhere[7], (uint24_t *)&pal[((tile & PIXEL7) >> SHIFT7)]);
    }
  // Restore the original color
  *pal = temp;
}

inline void md_vdp::draw_tile3(int which, int line, unsigned char *where)
{
  unsigned tile, *pal;
  uint24_t *wwhere = (uint24_t *)where;

  pal = highpal + (which >> 9 & 0x30); // Determine which 16-color palette

  if(which & 0x1000) // y flipped
    line ^= 7; // take from the bottom, instead of the top

  if(reg[12] & 2) // interlace
    tile = *(unsigned*)(vram + ((which&0x7ff) << 6) + (line << 3));
  else
    tile = *(unsigned*)(vram + ((which&0x7ff) << 5) + (line << 2));
  // If it's empty, why waste the time?
  if(!tile) return;

  // If the tile doesn't have any transparent pixels, draw it solidly.
  if (!has_zero_nibbles(tile)) {
    if (which & 0x800) {
      // x flipped
      u24cpy(&wwhere[0], (uint24_t *)&pal[((tile & PIXEL7) >> SHIFT7)]);
      u24cpy(&wwhere[1], (uint24_t *)&pal[((tile & PIXEL6) >> SHIFT6)]);
      u24cpy(&wwhere[2], (uint24_t *)&pal[((tile & PIXEL5) >> SHIFT5)]);
      u24cpy(&wwhere[3], (uint24_t *)&pal[((tile & PIXEL4) >> SHIFT4)]);
      u24cpy(&wwhere[4], (uint24_t *)&pal[((tile & PIXEL3) >> SHIFT3)]);
      u24cpy(&wwhere[5], (uint24_t *)&pal[((tile & PIXEL2) >> SHIFT2)]);
      u24cpy(&wwhere[6], (uint24_t *)&pal[((tile & PIXEL1) >> SHIFT1)]);
      u24cpy(&wwhere[7], (uint24_t *)&pal[((tile & PIXEL0) >> SHIFT0)]);
    }
    else {
      u24cpy(&wwhere[0], (uint24_t *)&pal[((tile & PIXEL0) >> SHIFT0)]);
      u24cpy(&wwhere[1], (uint24_t *)&pal[((tile & PIXEL1) >> SHIFT1)]);
      u24cpy(&wwhere[2], (uint24_t *)&pal[((tile & PIXEL2) >> SHIFT2)]);
      u24cpy(&wwhere[3], (uint24_t *)&pal[((tile & PIXEL3) >> SHIFT3)]);
      u24cpy(&wwhere[4], (uint24_t *)&pal[((tile & PIXEL4) >> SHIFT4)]);
      u24cpy(&wwhere[5], (uint24_t *)&pal[((tile & PIXEL5) >> SHIFT5)]);
      u24cpy(&wwhere[6], (uint24_t *)&pal[((tile & PIXEL6) >> SHIFT6)]);
      u24cpy(&wwhere[7], (uint24_t *)&pal[((tile & PIXEL7) >> SHIFT7)]);
    }
    return;
  }

  // Blit the tile!
  if(which & 0x800) // x flipped
    {
      if (tile & PIXEL7)
		u24cpy(&wwhere[0],
		       (uint24_t *)&pal[((tile & PIXEL7) >> SHIFT7)]);
      if(tile & PIXEL6)
		u24cpy(&wwhere[1],
		       (uint24_t *)&pal[((tile & PIXEL6) >> SHIFT6)]);
      if(tile & PIXEL5)
		u24cpy(&wwhere[2],
		       (uint24_t *)&pal[((tile & PIXEL5) >> SHIFT5)]);
      if(tile & PIXEL4)
		u24cpy(&wwhere[3],
		       (uint24_t *)&pal[((tile & PIXEL4) >> SHIFT4)]);
      if(tile & PIXEL3)
		u24cpy(&wwhere[4],
		       (uint24_t *)&pal[((tile & PIXEL3) >> SHIFT3)]);
      if(tile & PIXEL2)
		u24cpy(&wwhere[5],
		       (uint24_t *)&pal[((tile & PIXEL2) >> SHIFT2)]);
      if(tile & PIXEL1)
		u24cpy(&wwhere[6],
		       (uint24_t *)&pal[((tile & PIXEL1) >> SHIFT1)]);
      if(tile & PIXEL0)
		u24cpy(&wwhere[7],
		       (uint24_t *)&pal[((tile & PIXEL0) >> SHIFT0)]);
    } else {
      if(tile & PIXEL0)
		u24cpy(&wwhere[0],
		       (uint24_t *)&pal[((tile & PIXEL0) >> SHIFT0)]);
      if(tile & PIXEL1)
		u24cpy(&wwhere[1],
		       (uint24_t *)&pal[((tile & PIXEL1) >> SHIFT1)]);
      if(tile & PIXEL2)
		u24cpy(&wwhere[2],
		       (uint24_t *)&pal[((tile & PIXEL2) >> SHIFT2)]);
      if(tile & PIXEL3)
		u24cpy(&wwhere[3],
		       (uint24_t *)&pal[((tile & PIXEL3) >> SHIFT3)]);
      if(tile & PIXEL4)
		u24cpy(&wwhere[4],
		       (uint24_t *)&pal[((tile & PIXEL4) >> SHIFT4)]);
      if(tile & PIXEL5)
		u24cpy(&wwhere[5],
		       (uint24_t *)&pal[((tile & PIXEL5) >> SHIFT5)]);
      if(tile & PIXEL6)
		u24cpy(&wwhere[6],
		       (uint24_t *)&pal[((tile & PIXEL6) >> SHIFT6)]);
      if(tile & PIXEL7)
		u24cpy(&wwhere[7],
		       (uint24_t *)&pal[((tile & PIXEL7) >> SHIFT7)]);
    }
}

// Blit tile solidly, for 4 byte-per-pixel
inline void md_vdp::draw_tile4_solid(int which, int line, unsigned char *where)
{
  unsigned tile, temp, *pal;
  unsigned *wwhere = (unsigned*)where;

  pal = highpal + (which >> 9 & 0x30); // Determine which 16-color palette
  temp = *pal; *pal = highpal[reg[7]&0x3f]; // Get background color

  if(which & 0x1000) // y flipped
    line ^= 7; // take from the bottom, instead of the top

  if(reg[12] & 2) // interlace
    tile = *(unsigned*)(vram + ((which&0x7ff) << 6) + (line << 3));
  else
    tile = *(unsigned*)(vram + ((which&0x7ff) << 5) + (line << 2));

  // Blit the tile!
  if(which & 0x800) // x flipped
    {
      *(wwhere  ) = pal[((tile & PIXEL7)>>SHIFT7)];
      *(wwhere+1) = pal[((tile & PIXEL6)>>SHIFT6)];
      *(wwhere+2) = pal[((tile & PIXEL5)>>SHIFT5)];
      *(wwhere+3) = pal[((tile & PIXEL4)>>SHIFT4)];
      *(wwhere+4) = pal[((tile & PIXEL3)>>SHIFT3)];
      *(wwhere+5) = pal[((tile & PIXEL2)>>SHIFT2)];
      *(wwhere+6) = pal[((tile & PIXEL1)>>SHIFT1)];
      *(wwhere+7) = pal[((tile & PIXEL0)>>SHIFT0)];
    } else {
      *(wwhere  ) = pal[((tile & PIXEL0)>>SHIFT0)];
      *(wwhere+1) = pal[((tile & PIXEL1)>>SHIFT1)];
      *(wwhere+2) = pal[((tile & PIXEL2)>>SHIFT2)];
      *(wwhere+3) = pal[((tile & PIXEL3)>>SHIFT3)];
      *(wwhere+4) = pal[((tile & PIXEL4)>>SHIFT4)];
      *(wwhere+5) = pal[((tile & PIXEL5)>>SHIFT5)];
      *(wwhere+6) = pal[((tile & PIXEL6)>>SHIFT6)];
      *(wwhere+7) = pal[((tile & PIXEL7)>>SHIFT7)];
    }
  // Restore the original color
  *pal = temp;
}

// Blit tile, leaving color zero transparent, for 4 byte per pixel
inline void md_vdp::draw_tile4(int which, int line, unsigned char *where)
{
  unsigned tile, *pal;
  unsigned *wwhere = (unsigned*)where;

  pal = highpal + (which >> 9 & 0x30); // Determine which 16-color palette

  if(which & 0x1000) // y flipped
    line ^= 7; // take from the bottom, instead of the top

  if(reg[12] & 2) // interlace
    tile = *(unsigned*)(vram + ((which&0x7ff) << 6) + (line << 3));
  else
    tile = *(unsigned*)(vram + ((which&0x7ff) << 5) + (line << 2));
  // If the tile is all 0's, why waste the time?
  if(!tile) return;

  // If the tile doesn't have any transparent pixels, draw it solidly.
  if (!has_zero_nibbles(tile)) {
    if (which & 0x800) {
      // x flipped
      *(wwhere  ) = pal[((tile & PIXEL7)>>SHIFT7)];
      *(wwhere+1) = pal[((tile & PIXEL6)>>SHIFT6)];
      *(wwhere+2) = pal[((tile & PIXEL5)>>SHIFT5)];
      *(wwhere+3) = pal[((tile & PIXEL4)>>SHIFT4)];
      *(wwhere+4) = pal[((tile & PIXEL3)>>SHIFT3)];
      *(wwhere+5) = pal[((tile & PIXEL2)>>SHIFT2)];
      *(wwhere+6) = pal[((tile & PIXEL1)>>SHIFT1)];
      *(wwhere+7) = pal[((tile & PIXEL0)>>SHIFT0)];
    }
    else {
      *(wwhere  ) = pal[((tile & PIXEL0)>>SHIFT0)];
      *(wwhere+1) = pal[((tile & PIXEL1)>>SHIFT1)];
      *(wwhere+2) = pal[((tile & PIXEL2)>>SHIFT2)];
      *(wwhere+3) = pal[((tile & PIXEL3)>>SHIFT3)];
      *(wwhere+4) = pal[((tile & PIXEL4)>>SHIFT4)];
      *(wwhere+5) = pal[((tile & PIXEL5)>>SHIFT5)];
      *(wwhere+6) = pal[((tile & PIXEL6)>>SHIFT6)];
      *(wwhere+7) = pal[((tile & PIXEL7)>>SHIFT7)];
    }
    return;
  }

  // Blit the tile!
  if(which & 0x800) // x flipped
    {
      if(tile & PIXEL7) *(wwhere  ) = pal[((tile & PIXEL7)>>SHIFT7)];
      if(tile & PIXEL6) *(wwhere+1) = pal[((tile & PIXEL6)>>SHIFT6)];
      if(tile & PIXEL5) *(wwhere+2) = pal[((tile & PIXEL5)>>SHIFT5)];
      if(tile & PIXEL4) *(wwhere+3) = pal[((tile & PIXEL4)>>SHIFT4)];
      if(tile & PIXEL3) *(wwhere+4) = pal[((tile & PIXEL3)>>SHIFT3)];
      if(tile & PIXEL2) *(wwhere+5) = pal[((tile & PIXEL2)>>SHIFT2)];
      if(tile & PIXEL1) *(wwhere+6) = pal[((tile & PIXEL1)>>SHIFT1)];
      if(tile & PIXEL0) *(wwhere+7) = pal[((tile & PIXEL0)>>SHIFT0)];
    } else {
      if(tile & PIXEL0) *(wwhere  ) = pal[((tile & PIXEL0)>>SHIFT0)];
      if(tile & PIXEL1) *(wwhere+1) = pal[((tile & PIXEL1)>>SHIFT1)];
      if(tile & PIXEL2) *(wwhere+2) = pal[((tile & PIXEL2)>>SHIFT2)];
      if(tile & PIXEL3) *(wwhere+3) = pal[((tile & PIXEL3)>>SHIFT3)];
      if(tile & PIXEL4) *(wwhere+4) = pal[((tile & PIXEL4)>>SHIFT4)];
      if(tile & PIXEL5) *(wwhere+5) = pal[((tile & PIXEL5)>>SHIFT5)];
      if(tile & PIXEL6) *(wwhere+6) = pal[((tile & PIXEL6)>>SHIFT6)];
      if(tile & PIXEL7) *(wwhere+7) = pal[((tile & PIXEL7)>>SHIFT7)];
    }
}
#endif // WITH_X86_TILES

// Draw the window (front or back)
void md_vdp::draw_window(int line, int front)
{
  int size;
  int x, y, w, start;
  int pl, add;
  int total_window;
  unsigned char *where;
  int which;
  // Set everything up
  y = line >> 3;
  total_window = (y < (reg[18]&0x1f)) ^ (reg[18] >> 7);

  // Wide or narrow
  size = (reg[12] & 1)? 64 : 32;

  pl = (reg[3] << 10) + ((y&0x3f)*size*2);

  // Wide(320) or narrow(256)?
  if(reg[12] & 1)
    {
      w = 40;
      start = -8;
    } else {
      w = 32;
      start = 24;
    }
  add = -2;
  where = dest + (start * (int)Bpp);
	for (x = -1; (x < w); ++x) {
		if (!total_window) {
			if (reg[17] & 0x80) {
				if (x < ((reg[17] & 0x1f) << 1))
					goto skip;
			}
			else {
				if (x >= ((reg[17] & 0x1f) << 1))
					goto skip;
			}
		}
		which = get_word(((unsigned char *)vram) +
				 (pl + (add & ((size - 1) << 1))));
		if ((which >> 15) == front)
			draw_tile(which, (line & 7), where);
	skip:
		add += 2;
		where += Bpp_times8;
	}
}

inline void md_vdp::get_sprite_info(struct sprite_info& info, int index)
{
	uint_fast16_t prop;

	info.sprite = (sprite_base + (index << 3));

	// Get the sprite's location
	info.y = get_word(info.sprite);
	info.x = (get_word(info.sprite + 6) & 0x1ff);

	// Interlace?
	// XXX
	// "& 1" below is a workaround for a GCC (g++) <= 4.2.1 bug seen
	// in OpenBSD.
	// info.inter is a bit-field member of size 1 that normally cannot
	// store anything other than 0 or 1, but which does in practice,
	// causing info.tile to point to a bad address and crashing.
	info.inter = ((reg[12] >> 1) & 1);

	// Properties
	prop = get_word(info.sprite + 4);
	info.prio = (prop >> 15);
	info.xflip = (prop >> 11);
	info.yflip = (prop >> 12);
	info.tile = (uint32_t *)(vram + ((prop & 0x07ff) << (5 + info.inter)));

	if (info.inter)
		info.y = ((info.y & 0x3fe) >> 1);
	else
		info.y &= 0x1ff;

	info.x -= 0x80;
	info.y -= 0x80;

	// Narrow mode?
	if (!(reg[12] & 1))
		info.x += 32;

	info.tw = (((info.sprite[2] >> 2) & 0x03) + 1);
	info.th = ((info.sprite[2] & 0x03) + 1);
	info.w = (info.tw << 3);
	info.h = (info.th << 3);
}

void md_vdp::sprite_masking_overflow(int line)
{
	int masking_sprite_index;
	bool masking_effective;
	int frame_limit;
	int line_limit;
	int dots;
	int i;

	/*
	 * Search for the highest priority sprite with x = 0. Call this sprite
	 * s0.  Any sprite with a lower priority than s0 (therefore higher
	 * index in the array) is not drawn on the scanlines that s0 occupies
	 * on the y-axis. This is called sprite masking and is used by games
	 * like Streets of Rage and Lotus Turbo Challenge.
	 *
	 * Thanks for Charles MacDonald for explaining this to me (vext01).
	 *
	 * This loop also limits the number of sprites per line, which is 20
	 * in H40 and 16 in H32 _or_ 320 pixels wide in H40 and 256 pixels
	 * wide in H32, with a possibility for the last sprite to be only
	 * partially drawn.
	 */
	masking_sprite_index = -1;
	// If sprites on the previous line overflowed, sprite masking becomes
	// effective by default for the current line (it normally isn't).
	masking_effective = (sprite_overflow_line == (line - 1));
	// Set sprites and dots limits for the current line.
	if (reg[12] & 1) {
		frame_limit = 80;
		line_limit = 20;
		dots = 320;
	}
	else {
		frame_limit = 64;
		line_limit = 16;
		dots = 256;
	}
	for (i = 0; i < sprite_count; i++) {
		int x, y, w, h;
		int idx;
		uint8_t *sprite;

		// First, make sure the frame limit hasn't been reached.
		idx = sprite_order[i];
		if (idx >= frame_limit) {
			if (masking_sprite_index == -1)
				masking_sprite_index = (i - 1);
			break;
		}
		// Get current sprite coordinates and dimensions.
		sprite = (sprite_base + (idx << 3));
		x = get_word(sprite + 6) & 0x1ff;
		y = get_word(sprite);
		if (reg[12] & 2)
			y = ((y & 0x3fe) >> 1);
		else
			y &= 0x1ff;
		h = (((sprite[2] & 0x03) << 3) + 8);
		w = (((sprite[2] << 1) & 0x18) + 8);
		// If this sprite isn't found on the current line, skip it.
		if (!(((line + 0x80) >= y) && ((line + 0x80) < (y + h))))
			continue;
		// Substract sprite from the dots limit and decrease the
		// sprites limit.
		dots -= w;
		--line_limit;
		// If this sprite is not a masking sprite (x != 0), sprite
		// masking becomes effective. The next sprite with (x == 0)
		// will be a masking sprite.
		if (x != 0)
			masking_effective = true;
		// If a dot overflow occured, update sprite_overflow_line with
		// the current line. This update must be done only once for a
		// given line.
		if (dots <= 0) {
			sprite_overflow_line = line;
			// If no masking sprite index has been set so far, do
			// it now.  Otherwise reset dots, because this sprite
			// must not be truncated.
			if (masking_sprite_index == -1)
				masking_sprite_index = i;
			else
				dots = 0;
			// Don't process any more sprites, exit from the loop.
			break;
		}
		// Check whether sprites limit has been reached.
		if (line_limit == 0) {
			// If no masking sprite index has been set so far, do
			// it now.
			if (masking_sprite_index == -1)
				masking_sprite_index = i;
			// Trigger sprite overflow bit (d6).
			belongs.coo5 |= 0x40;
			// Don't process any more sprites, exit from the loop.
			break;
		}
		// If sprite masking is effective and the current sprite is a
		// masking sprite (x == 0), if we haven't already found one
		// before, use this one, then continue to process the sprites
		// list as we still need to know whether a dot overflow
		// occured.
		if ((masking_effective) &&
		    (x == 0) &&
		    (masking_sprite_index == -1))
			masking_sprite_index = i;
	}
	// If no masking sprite index was found, display them all.
	if (masking_sprite_index == -1)
		masking_sprite_index = (sprite_count - 1);
	masking_sprite_index_cache = masking_sprite_index;
	dots_cache = dots;
}

inline void md_vdp::sprite_mask_add(uint8_t* dest, int pitch,
				    struct sprite_info& info, int value)
{
	uint32_t *tile = info.tile;
	int len = (info.tw * info.h);
	int lines = (8 << info.inter);
	int line = 0;
	int wrap = 0;
	int unit = 1;

	dest += info.x;
	dest += (pitch * info.y);
	if (info.yflip) {
		dest += (pitch * (info.h - 1));
		pitch = -pitch;
	}
	if (info.xflip) {
		dest += (info.w - 1);
		unit = -unit;
	}
	while (len) {
		uint_fast32_t dots = be2h32(*tile);
		unsigned int tmp;

		for (tmp = 0; (tmp != 8); ++tmp) {
			assert(dest >= (uint8_t *)sprite_mask);
			assert(dest < ((uint8_t *)sprite_mask +
				       sizeof(sprite_mask)));
			/*
			 * If a non-transparent sprite dot has already been
			 * drawn here, trigger the collision bit (d5).
			 * FIXME: doing this here is hackish and doesn't take
			 * sprites with the high priority bit into account.
			 */
			if (*dest != 0xff)
				belongs.coo5 |= 0x20;
#ifdef WORDS_BIGENDIAN
			if (dots & 0x0000000f)
				*dest = value;
			dots >>= 4;
#else
			if (dots & 0xf0000000)
				*dest = value;
			dots <<= 4;
#endif
			dest += unit;
		}
		++tile;
		++line;
		if (line == lines) {
			/* Next tile. */
			line = 0;
			++wrap;
			if (wrap == info.th) {
				/* Next tiles column. */
				dest -= (pitch * (info.h - 1));
				wrap = 0;
			}
			else {
				/* Next tiles row. */
				dest += (pitch - (8 * unit));
			}
		}
		else {
			/* Next line of dots. */
			dest -= (8 * unit);
			dest += pitch;
		}
		--len;
	}
}

void md_vdp::sprite_mask_generate()
{
	int i;

	memset(sprite_mask, 0xff, sizeof(sprite_mask));
	for (i = (sprite_count - 1); (i >= 0); --i) {
		sprite_info info;

		get_sprite_info(info, sprite_order[i]);
		// We only care about sprites with the low priority bit unset.
		if (info.prio)
			continue;
		// Don't bother with hidden sprites.
		if ((info.x >= 320) || ((info.x + info.w) < 0))
			continue;
		if ((info.y >= 256) || ((info.y + info.h) < 0))
			continue;
		info.x += 0x80;
		info.y += 0x80;
		// Draw overlap mask for this sprite in a 512x512 virtual area.
		sprite_mask_add((uint8_t *)sprite_mask, sizeof(sprite_mask[0]),
				info, i);
	}
}

void md_vdp::draw_sprites(int line, bool front)
{
  unsigned int which;
  int tx, ty, x, y, xend, ysize, yoff, i, masking_sprite_index;
  int dots;
  unsigned char *where;
#ifdef WITH_DEBUG_VDP
  static int ant[2];
  static unsigned long ant_last[2];
  unsigned long ant_cur;

  if ((dgen_vdp_sprites_boxing) && (line == 0)) {
    ant_cur = pd_usecs();
    if ((ant_cur - ant_last[front]) > 100000) {
      ant_last[front] = ant_cur;
      ant[front] ^= 1;
    }
  }
#endif
  masking_sprite_index = masking_sprite_index_cache;
  dots = dots_cache;
  // If dots_cache is less than zero, draw the first sprite partially.
  if (dots > 0)
    dots = 0;
  // Sprites have to be in reverse order :P
  for (i = masking_sprite_index; i >= 0; --i)
    {
      sprite_info info;

      get_sprite_info(info, sprite_order[i]);
      // Only do it if it's on the right priority.
      if (info.prio == front)
	{
	  which = get_word(info.sprite + 4);
	  // Get the sprite's location
	  y = info.y;
	  x = info.x;
	  yoff = (line - y);
	  xend = ((info.w - 8) + x);
	  // Partial draw if negative.
	  xend += dots;
	  ysize = ((info.h - 8) >> 3);
	  // Render if this sprite's on this line
	  if(xend > -8 && x < 320 && yoff >= 0 && yoff <= (ysize<<3)+7)
	    {
	      ty = yoff & 7;
	      // y flipped?
	      if(which & 0x1000)
		which += ysize - (yoff >> 3);
	      else
		which += (yoff >> 3);
	      ++ysize;
	      // Unconditionally draw this sprite. It's supposed to always
	      // appear on top of other sprites.
	      if (!front) {
		// x flipped?
		if (which & 0x800) {
		  where = dest + (xend * (int)Bpp);
		  for(tx = xend; tx >= x; tx -= 8)
		    {
		      if(tx > -8 && tx < 320)
			draw_tile(which, ty, where);
		      which += ysize;
		      where -= Bpp_times8;
		    }
	        }
		else {
		  where = dest + (x * (int)Bpp);
		  for(tx = x; tx <= xend; tx += 8)
		    {
		      if(tx > -8 && tx < 320)
			draw_tile(which, ty, where);
		      which += ysize;
		      where += Bpp_times8;
		    }
		}
	      }
	      // Draw sprite with the high priority bit set only where it's
	      // not covered by a higher priority sprite (lower index in the
	      // list) but with this bit unset. Those have already been drawn
	      // during the previous pass.
	      else {
		union {
		  uint32_t t4[8];
		  uint24_t t3[8];
		  uint16_t t2[8];
		  uint8_t t1[8];
		} tile;

		// x flipped?
		if (which & 0x800) {
		  where = dest + (xend * (int)Bpp);
		  for (tx = xend; (tx >= x); tx -= 8) {
		    if ((tx > -8) && (tx < 320)) {
		      int xx;
		      int xo;

		      memcpy(tile.t1, where, Bpp_times8);
		      draw_tile(which, ty, tile.t1);
		      for (xx = tx, xo = 0; (xo != 8); ++xo, ++xx)
			if (sprite_mask[(line + 0x80)][(xx + 0x80)] >= i)
			  memcpy(&dest[(xx * (int)Bpp)],
				 &tile.t1[(xo * (int)Bpp)],
				 (int)Bpp);
		    }
		    which += ysize;
		    where -= Bpp_times8;
		  }
	        }
		else {
		  where = dest + (x * (int)Bpp);
		  for (tx = x; (tx <= xend); tx += 8) {
		    if ((tx > -8) && (tx < 320)) {
		      int xx;
		      int xo;

		      memcpy(tile.t1, where, Bpp_times8);
		      draw_tile(which, ty, tile.t1);
		      for (xx = tx, xo = 0; (xo != 8); ++xo, ++xx)
			if (sprite_mask[(line + 0x80)][(xx + 0x80)] >= i)
			  memcpy(&dest[(xx * (int)Bpp)],
				 &tile.t1[(xo * (int)Bpp)],
				 (int)Bpp);
		    }
		    which += ysize;
		    where += Bpp_times8;
		  }
		}
	      }
#ifdef WITH_DEBUG_VDP
	      if (dgen_vdp_sprites_boxing) {
		uint32_t color[2] = {
		  (uint32_t)dgen_vdp_sprites_boxing_bg,
		  (uint32_t)dgen_vdp_sprites_boxing_fg
		};
		int ph;
		int fx;

		if ((ph = 0, (y == line)) ||
		    (ph = 1, ((y + info.h - 1) == line)))
		  for (fx = (ant[front] ^ ph); (fx < info.w); fx += 2)
		    draw_pixel(this->bmap, (info.x + fx),
			       line, color[info.prio]);
		else
		  draw_pixel(this->bmap,
			     (((line & 1) == ant[front]) ?
			      (info.x + info.w - 1) : info.x),
			     line, color[info.prio]);
	      }
#endif
	    }
	}
      dots = 0;
    }
}

// The body for the next few functions is in an extraneous header file.
// Phil, I hope I left enough in this file for GLOBAL to hack it right. ;)
// Thanks to John Stiles for this trick :)

void md_vdp::draw_plane_back0(int line)
{
#define FRONT 0
#define PLANE 0
#include "ras-drawplane.h"
#undef PLANE
#undef FRONT
}

void md_vdp::draw_plane_back1(int line)
{
#define FRONT 0
#define PLANE 1
#include "ras-drawplane.h"
#undef PLANE
#undef FRONT
}

void md_vdp::draw_plane_front0(int line)
{
#define FRONT 1
#define PLANE 0
#include "ras-drawplane.h"
#undef PLANE
#undef FRONT
}

void md_vdp::draw_plane_front1(int line)
{
#define FRONT 1
#define PLANE 1
#include "ras-drawplane.h"
#undef PLANE
#undef FRONT
}

// Allow frame components to be hidden when WITH_DEBUG_VDP is defined.
#ifdef WITH_DEBUG_VDP
#define vdp_hide_if(a, b) ((a) ? (void)0 : (void)(b))
#else
#define vdp_hide_if(a, b) (void)(b)
#endif

// The main interface function, to generate a scanline
void md_vdp::draw_scanline(struct bmap *bits, int line)
{
  unsigned *ptr, i;
  // Set the destination in the bmap
  bmap = bits;
  dest = bits->data + (bits->pitch * (line + 8) + 16);
  // If bytes per pixel hasn't yet been set, do it
  if ((Bpp == 0) || (Bpp != BITS_TO_BYTES(bits->bpp)))
    {
           if(bits->bpp <= 8)  Bpp = 1;
      else if(bits->bpp <= 16) Bpp = 2;
      else if(bits->bpp <= 24) Bpp = 3;
      else		       Bpp = 4;
      Bpp_times8 = Bpp << 3; // used for tile blitting
#ifdef WITH_X86_TILES
      asm_tiles_init(vram, reg, highpal); // pass these values to the asm tiles
#endif
    }

  // If the palette's been changed, update it
  if(dirt[0x34] & 2)
    {
      ptr = highpal;
      // What color depth are we?
      switch(bits->bpp)
        {
	case 24:
#ifdef WORDS_BIGENDIAN
		for (i = 0; (i < 128); i += 2)
			*ptr++ = (((cram[(i + 1)] & 0x0e) << 28) |
				  ((cram[(i + 1)] & 0xe0) << 16) |
				  ((cram[i] & 0x0e) << 12));
		break;
#else
		for (i = 0; (i < 128); i += 2)
			*ptr++ = (((cram[(i + 1)] & 0x0e) << 4) |
				  ((cram[(i + 1)] & 0xe0) << 16) |
				  ((cram[i] & 0x0e) << 12));
		break;
#endif
	case 32:
	  for(i = 0; i < 128; i += 2)
	    *ptr++ = ((cram[i+1]&0x0e) << 20) |
		     ((cram[i+1]&0xe0) << 8 ) |
		     ((cram[i]  &0x0e) << 4 );
	  break;
	case 16:
	  for(i = 0; i < 128; i += 2)
	    *ptr++ = ((cram[i+1]&0x0e) << 12) |
		     ((cram[i+1]&0xe0) << 3 ) |
		     ((cram[i]  &0x0e) << 1 );
	  break;
	case 15:
	  for(i = 0; i < 128; i += 2)
	    *ptr++ = ((cram[i+1]&0x0e) << 11) |
		     ((cram[i+1]&0xe0) << 2 ) |
		     ((cram[i]  &0x0e) << 1 );
	  break;
	case 8:
	default:
	  // Let the hardware palette sort it out :P
	  for(i = 0; i < 64; ++i) *ptr++ = i;
	}
      // Clean up the dirt
      dirt[0x34] &= ~2;
      pal_dirty = 1;
    }
  // Render the screen if it's turned on
  if(reg[1] & 0x40)
    {
      // Recalculate the sprite order, if it's dirty
      if((dirt[0x30] & 0x20) || (dirt[0x34] & 1))
	{
	  unsigned next = 0;
	  // Max number of sprites per frame: 80 in H40, 64 in H32.
	  int max = ((reg[12] & 1) ? 80 : 64);
	  // Find the sprite base in VRAM
	  sprite_base = vram + (reg[5]<<9);
	  // Order the sprites
	  sprite_count = sprite_order[0] = 0;
	  do {
	    next = sprite_base[(next << 3) + 3];
	    sprite_order[++sprite_count] = next;
	  } while (next && sprite_count < max);
	  // Clean up the dirt
	  dirt[0x30] &= ~0x20; dirt[0x34] &= ~1;
	  // Generate overlap mask for sprites with high priority bit
	  sprite_mask_generate();
	}
      // Calculate sprite masking and overflow.
      sprite_masking_overflow(line);
      // Draw, from the bottom up
      // Low priority
      vdp_hide_if(dgen_vdp_hide_plane_b, draw_plane_back1(line));
      vdp_hide_if(dgen_vdp_hide_plane_a, draw_plane_back0(line));
      vdp_hide_if(dgen_vdp_hide_plane_w, draw_window(line, 0));
      vdp_hide_if(dgen_vdp_hide_sprites, draw_sprites(line, 0));
      // High priority
      vdp_hide_if(dgen_vdp_hide_plane_b, draw_plane_front1(line));
      vdp_hide_if(dgen_vdp_hide_plane_a, draw_plane_front0(line));
      vdp_hide_if(dgen_vdp_hide_plane_w, draw_window(line, 1));
      vdp_hide_if(dgen_vdp_hide_sprites, draw_sprites(line, 1));
    } else {
      // The display is off, paint it black
      // Do it a dword at a time
      unsigned *destl = (unsigned*)dest;
      for(i = 0; i < (80 * Bpp); ++i) destl[i] = 0;
    }

  // If we're in narrow (256) mode, cut off the messy edges
  if(!(reg[12] & 1))
    {
      unsigned *destl = (unsigned*)dest;
      for(i = 0; i < Bpp_times8; ++i)
        destl[i] = destl[i + (72 * Bpp)] = 0;
    }
}

void md_vdp::draw_pixel(struct bmap *bits, int x, int y, uint32_t rgb)
{
	uint8_t *out;

	if ((x < 0) || (x >= bits->w) || (y < 0) || (y >= bits->h))
		return;
	out = ((bits->data + (bits->pitch * (y + 8) + 16)) +
	       (x * BITS_TO_BYTES(bits->bpp)));
	switch (bits->bpp) {
		uint16_t tmp;

	case 32:
		memcpy(out, &rgb, sizeof(rgb));
		break;
	case 24:
		u24cpy((uint24_t *)out,
		       (const uint24_t *)((uint8_t *)&rgb + 1));
		break;
	case 16:
		tmp = (((rgb >> 5) & 0xf800) |
		       ((rgb >> 3) & 0x07e0) |
		       (rgb & 0x1f));
		memcpy(out, &tmp, sizeof(tmp));
		break;
	case 15:
		tmp = (((rgb >> 6) & 0x7c00) |
		       ((rgb >> 3) & 0x03e0) |
		       (rgb & 0x1f));
		memcpy(out, &tmp, sizeof(tmp));
		break;
	}
}
