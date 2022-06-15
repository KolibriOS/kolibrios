/*
	dZ80_LoadZ80File.

	Allocates the Z80's address space and loads in the source file.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dissz80p.h"

int dZ80_LoadZ80File(DISZ80 *d, DWORD *pBytesLoaded)
{
	FILE	*f;
	char	buf[_MAX_PATH + 64];

	*pBytesLoaded = 0;			/* We've not loaded anything yet */

	if (d->mem0Start == NULL)
		d->mem0Start = (BYTE *)malloc(Z80MEMSIZE);

	if (d->mem0Start == NULL)
		{
		dZ80_Error(d, "dZ80 couldn't allocate memory for the Z80 program.");
		return DERR_OUTOFMEM;
		}

	memset(d->mem0Start, 0, Z80MEMSIZE);

	f = fopen(d->srcFileName, "rb");
	if (f == NULL)
		{
		sprintf(buf, "Couldn't open the source file \"%s\"", d->srcFileName);
		dZ80_Error(d, buf);
		return(DERR_COULDNTOPENFILE);
		}

/* Get the file size */
	if (fseek(f, 0, SEEK_END))
		{
		sprintf(buf, "Couldn't determine the size of the file \"%s\"", d->srcFileName);
		dZ80_Error(d, buf);
		}
	else
		{
		if (ftell(f) > 65536L )
			{
			sprintf(buf, "Warning: The file \"%s\" is over 65,536 bytes in length.", d->srcFileName);
			dZ80_Error(d, buf);
			}
		}

/* Put the file's position back in the correct place */
	fseek(f, d->fileHeaderSize, SEEK_SET);

/* Read the whole file in one gulp (should be only a small file) */
	*pBytesLoaded = fread(d->mem0Start + d->fileStartAddr, 1, Z80MEMSIZE - d->fileStartAddr, f);

	fclose(f);	
	
	return DERR_NONE;	/* All's well */
}

