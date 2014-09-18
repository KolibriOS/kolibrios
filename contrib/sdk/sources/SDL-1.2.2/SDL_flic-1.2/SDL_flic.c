/*
SDL_flic - renders FLIC animations
Copyright (C) 2003 Andre de Leiradella

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

For information about SDL_flic contact leiradella@bigfoot.com

Version 1.0: first public release.
Version 1.1: fixed bug to set *error to FLI_OK when returning successfully from FLI_Open
             added function FLI_Reset to reset the animation to the first frame
Version 1.2: added function FLI_Skip to skip the current frame without rendering
             FLI_Animation->surface is now correctly locked and unlocked
             the rwops stream is now part of the FLI_Animation structure and is closed inside FLI_Close
             renamed FLI_Reset to FLI_Rewind
             added function FLI_Version that returns the library version
*/
#include <SDL_flic.h>
#include <setjmp.h>
#include <stdlib.h>
#include <mem.h>

/* Library version. */
#define FLI_MAJOR 1
#define FLI_MINOR 2

/* Chunk types. */
#define FLI_COLOR256 4
#define FLI_SS2      7
#define FLI_COLOR    11
#define FLI_LC       12
#define FLI_BLACK    13
#define FLI_BRUN     15
#define FLI_COPY     16
#define FLI_PSTAMP   18

typedef struct {
        Uint32 size, type, numchunks;
} FLI_Frame;

typedef struct {
        Uint32 size, type, index;
} FLI_Chunk;

static INLINE void readbuffer(FLI_Animation *flic, void *buffer, int size) {
        if (SDL_RWread(flic->rwops, buffer, 1, size) != size)
                longjmp(flic->error, FLI_READERROR);
}

static INLINE Uint8 readu8(FLI_Animation *flic) {
        Uint8 b;

        readbuffer(flic, &b, 1);
        return b;
}

static INLINE Uint16 readu16(FLI_Animation *flic) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        Uint16 hi, lo;

        readbuffer(flic, &lo, 1);
        readbuffer(flic, &hi, 1);
        return hi << 8 | lo;
#else
        Uint16 w;

        readbuffer(flic, &w, 2);
        return w;
#endif
}

static INLINE Uint32 readu32(FLI_Animation *flic) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        Uint32 hi;

        hi = readu16(flic);
        return hi << 16 | readu16(flic);
#else
        Uint32 u;

        readbuffer(flic, &u, 4);
        return u;
#endif
}

static void readheader(FLI_Animation *flic) {
        /* Skip size, we don't need it. */
        SDL_RWseek(flic->rwops, 4, SEEK_CUR);
        /* Read and check magic. */
        flic->format = readu16(flic);
        if (flic->format != FLI_FLI && flic->format != FLI_FLC)
                longjmp(flic->error, FLI_CORRUPTEDFILE);
        /* Read number of frames, maximum is 4000 for FLI and FLC files. */
        flic->numframes = readu16(flic);
        if (flic->numframes > 4000)
                longjmp(flic->error, FLI_CORRUPTEDFILE);
        /* Read width and height, must be 320x200 for FLI files. */
        flic->width = readu16(flic);
        flic->height = readu16(flic);
        if (flic->format == FLI_FLI && (flic->width != 320 || flic->height != 200))
                longjmp(flic->error, FLI_CORRUPTEDFILE);
        /* Read color depth, must be 8 for FLI and FLC files. */
        flic->depth = readu16(flic);
        if (flic->depth != 8)
                longjmp(flic->error, FLI_CORRUPTEDFILE);
        /* Skip the flags, it doesn't look like it follows the specs. */
        readu16(flic);
        /* Read the delay between frames. */
        flic->delay = (flic->format == FLI_FLI) ? readu16(flic) : readu32(flic);
        /* Skip rest of the header. */
        SDL_RWseek(flic->rwops, (flic->format == FLI_FLI) ? 110 : 108, SEEK_CUR);
}

static INLINE void readframe(FLI_Animation *flic, FLI_Frame *frame) {
        /* Read the size of the frame, must be less than or equal to 64k in FLI files. */
        frame->size = readu32(flic);
        if (flic->format == FLI_FLI && frame->size > 65536)
                longjmp(flic->error, FLI_CORRUPTEDFILE);
        /* Read the type of the frame, must be 0xF1FA in FLI files or 0xF1FA or 0xF100 in FLC files. */
        frame->type = readu16(flic);
        if (frame->type != 0xF1FA && (flic->format == FLI_FLC && frame->type != 0xF100))
                longjmp(flic->error, FLI_CORRUPTEDFILE);
        /* Read the number of chunks in this frame. */
        frame->numchunks = readu16(flic);
        /* Skip rest of the data. */
        SDL_RWseek(flic->rwops, 8, SEEK_CUR);
}

static INLINE void readchunk(FLI_Animation *flic, FLI_Chunk *chunk) {
        /* Read the chunk size. */
        chunk->size = readu32(flic);
        /* Read the chunk type. */
        chunk->type = readu16(flic);
}

static void handlecolor(FLI_Animation *flic, FLI_Chunk *chunk) {
        int       numpackets, index, count;
        SDL_Color color;

        (void)chunk;
        /* Number of packets. */
        numpackets = readu16(flic);
        /* Color index that will be changed. */
        index = 0;
        while (numpackets-- > 0) {
                /* Skip some colors. */
                index += readu8(flic);
                /* And change some others. */
                count = readu8(flic);
                if (count == 0)
                        count = 256;
                while (count-- > 0) {
                        /* r, g and b are in the range [0..63]. */
                        color.r = ((Uint32)readu8(flic)) * 255 / 63;
                        color.g = ((Uint32)readu8(flic)) * 255 / 63;
                        color.b = ((Uint32)readu8(flic)) * 255 / 63;
                        SDL_SetColors(flic->surface, &color, index++, 1);
                }
        }
}

static void handlelc(FLI_Animation *flic, FLI_Chunk *chunk) {
        int    numlines, numpackets, size;
        Uint8  *line, *p;

        (void)chunk;
        /* Skip lines at the top of the image. */
        line = (Uint8 *)flic->surface->pixels + readu16(flic) * flic->surface->pitch;
        /* numlines lines will change. */
        numlines = readu16(flic);
        while (numlines-- > 0) {
                p = line;
                line += flic->surface->pitch;
                /* Each line has numpackets changes. */
                numpackets = readu8(flic);
                while (numpackets-- > 0) {
                        /* Skip pixels at the beginning of the line. */
                        p += readu8(flic);
                        /* size pixels will change. */
                        size = (Sint8)readu8(flic);
                        if (size >= 0) {
                                /* Pixels follow. */
                                readbuffer(flic, (void *)p, size);
                        } else {
                                size = -size;
                                /* One pixel to be repeated follow. */
                                memset((void *)p, readu8(flic), size);
                        }
                        p += size;
                }
        }
}

static void handleblack(FLI_Animation *flic, FLI_Chunk *chunk) {
        (void)chunk;
        /* Fill the surface with color 0. */
        if (SDL_FillRect(flic->surface, NULL, 0) != 0)
                longjmp(flic->error, FLI_SDLERROR);
}

static void handlebrun(FLI_Animation *flic, FLI_Chunk *chunk) {
        int    numlines, size;
        Uint8  *p, *next;

        (void)chunk;
        /* Begin at the top of the image. */
        p = (Uint8 *)flic->surface->pixels;
        /* All lines will change. */
        numlines = flic->height;
        while (numlines-- > 0) {
                /* The number of packages is ignored, packets run until the next line is reached. */
                readu8(flic);
                next = p + flic->surface->pitch;
                while (p < next) {
                        /* size pixels will change. */
                        size = (Sint8)readu8(flic);
                        if (size < 0) {
                                size = -size;
                                /* Pixels follow. */
                                readbuffer(flic, (void *)p, size);
                        } else {
                                /* One pixel to be repeated follow. */
                                memset((void *)p, readu8(flic), size);
                        }
                        p += size;
                }
        }
}

static void handlecopy(FLI_Animation *flic, FLI_Chunk *chunk) {
        (void)chunk;
        /* Read the entire image from the stream. */
        readbuffer(flic, (void *)flic->surface->pixels, flic->width * flic->height);
}

static void handlecolor256(FLI_Animation *flic, FLI_Chunk *chunk) {
        int       numpackets, index, count;
        SDL_Color color;

        (void)chunk;
        if (flic->format == FLI_FLI)
                longjmp(flic->error, FLI_CORRUPTEDFILE);
        /* Number of packets. */
        numpackets = readu16(flic);
        /* Color index that will be changed. */
        index = 0;
        while (numpackets-- > 0) {
                /* Skip some colors. */
                index += readu8(flic);
                /* And change some others. */
                count = readu8(flic);
                if (count == 0)
                        count = 256;
                while (count-- > 0) {
                        /* r, g and b are in the range [0..255]. */
                        color.r = readu8(flic);
                        color.g = readu8(flic);
                        color.b = readu8(flic);
                        SDL_SetColors(flic->surface, &color, index++, 1);
                }
        }
}

static void handless2(FLI_Animation *flic, FLI_Chunk *chunk) {
        int   numlines, y, code, size;
        Uint8 *p, c;

        (void)chunk;
        if (flic->format == FLI_FLI)
                longjmp(flic->error, FLI_CORRUPTEDFILE);
        /* numlines lines will change. */
        numlines = readu16(flic);
        y = 0;
        while (numlines > 0) {
                /* Read the code. */
                code = readu16(flic);
                switch ((code >> 14) & 0x03) {
                        case 0x00:
                                p = (Uint8 *)flic->surface->pixels + flic->surface->pitch * y;
                                while (code-- > 0) {
                                        /* Skip some pixels. */
                                        p += readu8(flic);
                                        size = ((Sint8)readu8(flic)) * 2;
                                        if (size >= 0) {
                                                /* Pixels follow. */
                                                readbuffer(flic, (void *)p, size);
                                        } else {
                                                size = -size;
                                                readu8(flic);
                                                /* One pixel to be repeated follow. */
                                                memset((void *)p, readu8(flic), size);
                                        }
                                        p += size;
                                }
                                y++;
                                numlines--;
                                break;
                        case 0x01:
                                longjmp(flic->error, FLI_CORRUPTEDFILE);
                        case 0x02:
                                /* Last pixel of the line. */
                                p = (Uint8 *)flic->surface->pixels + flic->surface->pitch * (y + 1);
                                p[-1] = code & 0xFF;
                                break;
                        case 0x03:
                                /* Skip some lines. */
                                y += (code ^ 0xFFFF) + 1;
                                break;
                }
        }
}

int FLI_Version(void) {
        return FLI_MAJOR << 16 | FLI_MINOR;
}

FLI_Animation *FLI_Open(SDL_RWops *rwops, int *error) {
        FLI_Animation *flic;
        FLI_Frame     frame;
        int           err;

        /* Alloc animation. */
        flic = (FLI_Animation *)malloc(sizeof(FLI_Animation));
        if (flic == NULL) {
                if (error != NULL) *error = FLI_OUTOFMEMORY;
                return NULL;
        }
        flic->rwops = rwops;
        flic->surface = NULL;
        /* Error handling. */
        err = setjmp(flic->error);
        if (err != 0) {
                if (error != NULL) *error = err;
                FLI_Close(flic);
                return NULL;
        }
        /* Read the header. */
        readheader(flic);
        /* Create a buffer to hold the rendered frame. */
        flic->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, flic->width, flic->height, 8, 0, 0, 0, 0);
        if (flic->surface == NULL)
                longjmp(flic->error, FLI_SDLERROR);
        /* Read the first frame. */
        flic->offframe1 = SDL_RWtell(rwops);
        readframe(flic, &frame);
        /* If it's a prefix frame, skip it. */
        if (frame.type == 0xF100) {
                SDL_RWseek(rwops, frame.size - 16, SEEK_CUR);
                flic->offframe1 = SDL_RWtell(rwops);
                flic->numframes--;
        }
        flic->offnextframe = flic->offframe1;
        flic->nextframe = 1;
        if (error != NULL) *error = FLI_OK;
        return flic;
}

void FLI_Close(FLI_Animation *flic) {
        if (flic != NULL) {
                if (flic->rwops != NULL)
                        SDL_RWclose(flic->rwops);
                if (flic->surface != NULL)
                        SDL_FreeSurface(flic->surface);
                free(flic);
        }
}

int FLI_NextFrame(FLI_Animation *flic) {
        FLI_Frame frame;
        FLI_Chunk chunk;
        int       error, locked;
        Uint32    i;

        /* Flag to tell if the surface is locked. */
        locked = 0;
        /* Error handling. */
        error = setjmp(flic->error);
        if (error != 0) {
                if (locked)
                        SDL_UnlockSurface(flic->surface);
                return error;
        }
        /* Seek to the current frame. */
        SDL_RWseek(flic->rwops, flic->offnextframe, SEEK_SET);
        /* Read the current frame. */
        readframe(flic, &frame);
        /* Read and process each of the chunks of this frame. */
        SDL_LockSurface(flic->surface);
        locked = 1;
        (void)locked;
        for (i = frame.numchunks; i != 0; i--) {
                readchunk(flic, &chunk);
                switch (chunk.type) {
                        case FLI_COLOR:
                                handlecolor(flic, &chunk);
                                break;
                        case FLI_LC:
                                handlelc(flic, &chunk);
                                break;
                        case FLI_BLACK:
                                handleblack(flic, &chunk);
                                break;
                        case FLI_BRUN:
                                handlebrun(flic, &chunk);
                                break;
                        case FLI_COPY:
                                handlecopy(flic, &chunk);
                                break;
                        case FLI_COLOR256:
                                handlecolor256(flic, &chunk);
                                break;
                        case FLI_SS2:
                                handless2(flic, &chunk);
                                break;
                        case FLI_PSTAMP:
                                /* Ignore this chunk. */
                                break;
                        default:
                                longjmp(flic->error, FLI_CORRUPTEDFILE);
                }
        }
        SDL_UnlockSurface(flic->surface);
        /* Setup the number and position of next frame. If it wraps, go to the first one. */
        if (++flic->nextframe > flic->numframes) {
                flic->offnextframe = flic->offframe1;
                flic->nextframe = 1;
        } else
                flic->offnextframe += frame.size;
        return FLI_OK;
}

int FLI_Rewind(FLI_Animation *flic) {
        flic->offnextframe = flic->offframe1;
        flic->nextframe = 1;
        return FLI_OK;
}

int FLI_Skip(FLI_Animation *flic) {
        FLI_Frame frame;
        int       error;

        /* Error handling. */
        error = setjmp(flic->error);
        if (error != 0)
                return error;
        /* Seek to the current frame. */
        SDL_RWseek(flic->rwops, flic->offnextframe, SEEK_SET);
        /* Read the current frame. */
        readframe(flic, &frame);
        /* Skip to the next frame without rendering. */
        if (++flic->nextframe > flic->numframes) {
                flic->offnextframe = flic->offframe1;
                flic->nextframe = 1;
        } else
                flic->offnextframe += frame.size;
        return FLI_OK;
}
