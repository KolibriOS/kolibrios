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
#include "animlib.h"

#include "file.h"
#include "keyboard.h"
#include "network.h"
#include "nortsong.h"
#include "palette.h"
#include "sizebuf.h"
#include "video.h"

#include <assert.h>

/*** Structs ***/
/* The actual header has a lot of fields that are basically useless to us since
 * we both set our own framerate and the format itself only allows for
 * 320x200x8.  Should a (nonexistent) ani be played that doesn't have the same
 * assumed values we are going to use, TOO BAD.  It'll just be treated as
 * corrupt in playback.
 */
#define PALETTE_OFFSET    0x100 // 128 + sizeof(header)
#define PAGEHEADER_OFFSET 0x500 // PALETTE_OFFSET + sizeof(palette)
#define ANIM_OFFSET   0x0B00    // PAGEHEADER_OFFSET + sizeof(largepageheader) * 256
#define ANI_PAGE_SIZE 0x10000   // 65536.
typedef struct anim_FileHeader_s
{
	unsigned int nlps;            /* Number of 'pages', max 256. */
	unsigned int nRecords;        /* Number of 'records', max 65535 */
} anim_FileHeader_t;
typedef struct anim_LargePageHeader_s
{
	unsigned int baseRecord;      /* The first record's number */
	unsigned int nRecords;        /* Number of records.  Supposedly there are bit flags but I saw no such code */
	unsigned int nBytes;	      /* Number of bytes used, excluding headers */
} anim_LargePageHeader_t;


/*** Globals ***/
Uint8 CurrentPageBuffer[65536];
anim_LargePageHeader_t PageHeader[256];
unsigned int CurrentPageRecordSizes[256];

anim_LargePageHeader_t CurrentPageHeader;
anim_FileHeader_t FileHeader;

unsigned int Curlpnum;

FILE * InFile;


/*** Function decs ***/
int JE_playRunSkipDump( Uint8 *, unsigned int );
void JE_closeAnim( void );
int JE_loadAnim( const char * );
int JE_renderFrame( unsigned int );
int JE_findPage ( unsigned int );
int JE_drawFrame( unsigned int );
int JE_loadPage( unsigned int );

/*** Implementation ***/


/* Loads the given page into memory.
 *
 * Returns  0 on success or nonzero on failure (bad data)
 */
int JE_loadPage( unsigned int pagenumber )
{
	unsigned int i, pageSize;


	if (Curlpnum == pagenumber) { return(0); } /* Already loaded */
	Curlpnum = pagenumber;

	/* We need to seek to the page and load it into our buffer.
	 * Pages have a fixed size of 0x10000; any left over space is padded
	 * unless it's the end of the file.
	 *
	 * Pages repeat their headers for some reason.  They then have two bytes of
	 * padding folowed by a word for every record.  THEN the data starts.
	 */
	fseek(InFile, ANIM_OFFSET + (pagenumber * ANI_PAGE_SIZE), SEEK_SET);
	efread(&CurrentPageHeader.baseRecord, 2, 1, InFile);
	efread(&CurrentPageHeader.nRecords,   2, 1, InFile);
	efread(&CurrentPageHeader.nBytes,     2, 1, InFile);

	fseek(InFile, 2, SEEK_CUR);
	for (i = 0; i < CurrentPageHeader.nRecords; i++)
	{
		efread(&CurrentPageRecordSizes[i], 2, 1, InFile);
	}

	/* What remains is the 'compressed' data */
	efread(CurrentPageBuffer, 1, CurrentPageHeader.nBytes, InFile);

	/* Okay, we've succeeded in all our IO checks.  Now, make sure the
	 * headers aren't lying or damaged or something.
	 */
	pageSize = 0;
	for (i = 0; i < CurrentPageHeader.nRecords; i++)
	{
		pageSize += CurrentPageRecordSizes[i];
	}

	if(pageSize != CurrentPageHeader.nBytes) { return(-1); }

	/* So far, so good */
	return(0);
}

int JE_drawFrame( unsigned int framenumber )
{
	int ret;


	ret = JE_loadPage(framenumber);
	if (ret) { return(ret); }

	ret = JE_renderFrame (framenumber);
	if (ret) { return(ret); }

	return(0);
}

int JE_findPage( unsigned int framenumber )
{
	unsigned int i;


	for (i = 0; i < FileHeader.nlps; i++)
	{
		if (PageHeader[i].baseRecord <= framenumber
		 && PageHeader[i].baseRecord + PageHeader[i].nRecords > framenumber)
		{
			return(i);
		}
	}

	return(-1); /* Did not find */
}

int JE_renderFrame( unsigned int framenumber )
{
	unsigned int i, offset, destframe;


	destframe = framenumber - CurrentPageHeader.baseRecord;

	offset = 0;
	for (i = 0; i < destframe; i++)
	{
		offset += CurrentPageRecordSizes[i];
	}

	return (JE_playRunSkipDump(CurrentPageBuffer + offset + 4, CurrentPageRecordSizes[destframe] - 4));
}

void JE_playAnim( const char *animfile, JE_byte startingframe, JE_byte speed )
{
	unsigned int i;
	int pageNum;

	if (JE_loadAnim(animfile) != 0)
	{
		return; /* Failed to open or process file */
	}

	/* Blank screen */
	JE_clr256(VGAScreen);
	JE_showVGA();


	/* re FileHeader.nRecords-1: It's -1 in the pascal too.
	 * The final frame is a delta of the first, and we don't need that.
	 * We could also, if we ever ended up needing to loop anis, check
	 * the bools in the header to see if we should render the last
	 * frame.  But that's never going to be encessary :)
	 */
    for (i = startingframe; i < FileHeader.nRecords-1; i++)
    {
    	/* Handle boring crap */
    	setjasondelay(speed);

		/* Load required frame.  The loading function is smart enough to not re-load an already loaded frame */
		pageNum = JE_findPage(i);
		if(pageNum == -1) { break; }
		if (JE_loadPage(pageNum) != 0) { break; }

		/* render frame. */
    	if (JE_renderFrame(i) != 0) { break; }
    	JE_showVGA();


		/* Return early if user presses a key */
		service_SDL_events(true);
		if (newkey)
		{
			break;
		}

		/* Wait until we need the next frame */
		NETWORK_KEEP_ALIVE();
		wait_delay();
    }

	JE_closeAnim();
}

/* loadAnim opens the file and loads data from it into the header structs.
 * It should take care to clean up after itself should an error occur.
 */
int JE_loadAnim( const char *filename )
{
	unsigned int i, fileSize;
	char temp[4];


	Curlpnum = -1;
	InFile = dir_fopen(data_dir(), filename, "rb");
	if(InFile == NULL)
	{
		return(-1);
	}

	fileSize = ftell_eof(InFile);
	if(fileSize < ANIM_OFFSET)
	{
		/* We don't know the exact size our file should be yet,
		 * but we do know it should be way more than this */
		fclose(InFile);
		return(-1);
	}

	/* Read in the header.  The header is 256 bytes long or so,
	 * but that includes a lot of padding as well as several
	 * vars we really don't care about.  We shall check the ID and extract
	 * the handful of vars we care about.  Every value in the header that
	 * is constant will be ignored.
	 */

	efread(&temp, 1, 4, InFile); /* The ID, should equal "LPF " */
	fseek(InFile, 2, SEEK_CUR); /* skip over this word */
	efread(&FileHeader.nlps, 2, 1, InFile); /* Number of pages */
	efread(&FileHeader.nRecords, 4, 1, InFile); /* Number of records */

	if (memcmp(temp, "LPF ", 4) != 0
	 || FileHeader.nlps == 0  || FileHeader.nRecords == 0
	 || FileHeader.nlps > 256 || FileHeader.nRecords > 65535)
	{
		fclose(InFile);
		return(-1);
	}

	/* Read in headers */
	fseek(InFile, PAGEHEADER_OFFSET, SEEK_SET);
	for (i = 0; i < FileHeader.nlps; i++)
	{
		efread(&PageHeader[i].baseRecord, 2, 1, InFile);
		efread(&PageHeader[i].nRecords,   2, 1, InFile);
		efread(&PageHeader[i].nBytes,     2, 1, InFile);
	}


	/* Now we have enough information to calculate the 'expected' file size.
	 * Our calculation SHOULD be equal to fileSize, but we won't begrudge
	 * padding */
	if (fileSize < (FileHeader.nlps-1) * ANI_PAGE_SIZE + ANIM_OFFSET
	  + PageHeader[FileHeader.nlps-1].nBytes
	  + PageHeader[FileHeader.nlps-1].nRecords * 2 + 8)
	{
		fclose(InFile);
		return(-1);
	}


	/* Now read in the palette. */
	fseek(InFile, PALETTE_OFFSET, SEEK_SET);
	for (i = 0; i < 256; i++)
	{
		efread(&colors[i].b,      1, 1, InFile);
		efread(&colors[i].g,      1, 1, InFile);
		efread(&colors[i].r,      1, 1, InFile);
		efread(&colors[i].unused, 1, 1, InFile);
	}
	set_palette(colors, 0, 255);

	/* Whew!  That was hard.  Let's go grab some beers! */
	return(0);
}

void JE_closeAnim( void )
{
	fclose(InFile);
}

/* RunSkipDump decompresses the video.  There are three operations, run, skip,
 * and dump.  They can be used in either byte or word variations, making six
 * possible actions, and there's a seventh 'stop' action, which looks
 * like 0x80 0x00 0x00.
 *
 * Run is a memset.
 * Dump is a memcpy.
 * Skip leaves the old data intact and simply increments the pointers.
 *
 * returns 0 on success or 1 if decompressing failed.  Failure to decompress
 * indicates a broken or malicious file; playback should terminate.
 */
int JE_playRunSkipDump( Uint8 *incomingBuffer, unsigned int IncomingBufferLength )
{
	sizebuf_t Buffer_IN, Buffer_OUT;
	sizebuf_t * pBuffer_IN = &Buffer_IN, * pBuffer_OUT = &Buffer_OUT;

	#define ANI_SHORT_RLE  0x00
	#define ANI_SHORT_SKIP 0x80
	#define ANI_LONG_OP    0x80
	#define ANI_LONG_COPY_OR_RLE  0x8000
	#define ANI_LONG_RLE   0x4000
	#define ANI_STOP       0x0000

	SZ_Init(pBuffer_IN,  incomingBuffer,    IncomingBufferLength);
	SZ_Init(pBuffer_OUT, VGAScreen->pixels, VGAScreen->h * VGAScreen->pitch);


	/* 320x200 is the only supported format.
	 * Assert is here as a hint should our screen size ever changes.
	 * As for how to decompress to the wrong screen size... */
	assert(VGAScreen->h * VGAScreen->pitch == 320 * 200);


	while (1)
	{
		/* Get one byte.  This byte may have flags that tell us more */
		unsigned int opcode = MSG_ReadByte(pBuffer_IN);

		/* Before we continue, check the error states/
		 * We should *probably* check these after every read and write, but
		 * I've rigged it so that the buffers will never go out of bounds.
		 * So we can afford to be lazy; if the buffer overflows below it will
		 * silently fail its writes and we'll catch the failure on our next
		 * run through the loop.  A failure means we should be
		 * leaving ANYWAY.  The contents of our buffers doesn't matter.
		 */
		if (SZ_Error(pBuffer_IN) || SZ_Error(pBuffer_OUT))
		{
			return(-1);
		}

		/* Divide into 'short' and 'long' */
		if (opcode == ANI_LONG_OP) /* long ops */
		{
			opcode = MSG_ReadWord(pBuffer_IN);

			if (opcode == ANI_STOP) /* We are done decompressing.  Leave */
			{
				break;
			}
			else if (!(opcode & ANI_LONG_COPY_OR_RLE)) /* If it's not those two, it's a skip */
			{
				unsigned int count = opcode;
				SZ_Seek(pBuffer_OUT, count, SEEK_CUR);
			}
			else /* Now things get a bit more interesting... */
			{
				opcode &= ~ANI_LONG_COPY_OR_RLE; /* Clear that flag */

				if (opcode & ANI_LONG_RLE) /* RLE */
				{
					unsigned int count = opcode & ~ANI_LONG_RLE; /* Clear flag */

					/* Extract another byte */
					unsigned int value = MSG_ReadByte(pBuffer_IN);

					/* The actual run */
					SZ_Memset(pBuffer_OUT, value, count);
				}
				else
				{ /* Long copy */
					unsigned int count = opcode;

					/* Copy */
					SZ_Memcpy2(pBuffer_OUT, pBuffer_IN, count);
				}
			}
		} /* End of long ops */
		else /* short ops */
		{
			if (opcode & ANI_SHORT_SKIP) /* Short skip, move pointer only */
			{
				unsigned int count = opcode & ~ANI_SHORT_SKIP; /* clear flag to get count */
				SZ_Seek(pBuffer_OUT, count, SEEK_CUR);
			}
			else if (opcode == ANI_SHORT_RLE) /* Short RLE, memset the destination */
			{
				/* Extract a few more bytes */
				unsigned int count = MSG_ReadByte(pBuffer_IN);
				unsigned int value = MSG_ReadByte(pBuffer_IN);

				/* Run */
				SZ_Memset(pBuffer_OUT, value, count);
			}
			else /* Short copy, memcpy from src to dest. */
			{
				unsigned int count = opcode;

				/* Dump */
				SZ_Memcpy2(pBuffer_OUT, pBuffer_IN, count);
			}
		} /* End of short ops */
	}

	/* And that's that */
	return(0);
}

