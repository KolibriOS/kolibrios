/*
	example.c 

        How to use dZ80's disassembler in your own programs.
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#include "types.h"
#include "dissz80.h"

/* Use the example Z80 file supplied with the command line version of dZ80 */
const char *pZ80Filename = "mayhem.sna";

int main(void)
{
	FILE	*f;
	BYTE	*pMem;			/* Pointer to our Z80's memory */
	DISZ80	*d;			/* Pointer to the Disassembly structure */
	int		line, err;		/* line count */
	WORD	dAddr;			/* Disassembly address */

/* Allocate the dZ80 structure */
	d = malloc(sizeof(DISZ80));
	if (d == NULL)
		{
		printf("Cannot allocate %d bytes\n", sizeof(DISZ80));
		exit(1);
		}

	memset(d, 0, sizeof(DISZ80));

/* Allocate the memory space for our virtual Z80 */
	pMem = malloc(Z80MEMSIZE);
	if (pMem == NULL)
		{
		free(d);
		printf("Cannot allocate %d bytes\n", Z80MEMSIZE);
		exit(1);
		}

	memset(pMem, 0, Z80MEMSIZE);

	f = fopen(pZ80Filename, "rb");
	if (f == NULL)
		{
		printf("Stone the crows - couldn't open %s\n", pZ80Filename);
		exit(1);
		}
	else
		{
		fseek(f, 27, SEEK_SET);			/* Skip the .sna's header - go straight to the memory dump */
		fread(pMem + 16384, 49152, 1, f);
		fclose(f);
		}

/* Starting disassembly address */
	dAddr = 0x8000;

/* Set up dZ80's structure - it's not too fussy */
	memset(d, 0, sizeof(DISZ80));

/* Set the default radix and strings (comments and "db") */ 
	dZ80_SetDefaultOptions(d);		

/* Set the CPU type */
    d->cpuType = DCPU_Z80;

/* Set the start of the Z80's memory space */
	d->mem0Start = pMem;

/* Indicate we're disassembling a single instruction */
	d->flags |= DISFLAG_SINGLE;

/* And we're off! Let's disassemble 20 instructions from dAddr */ 
	for(line=0; line < 20; line++)
		{
/* Set the disassembly address */
		d->start = d->end = dAddr;
		
		err = dZ80_Disassemble(d);
		if (err != DERR_NONE)
			{
			printf("**** dZ80 error:  %s\n", dZ80_GetErrorText(err));
			break;
			}
		
/* Display the disassembled line, using the hex dump and disassembly buffers in the DISZ80 structure */
		printf("%04x: %10s  %s\n", dAddr, d->hexDisBuf, d->disBuf);
		
/* Point to the next instruction (bytesProcessed holds the number of bytes for the last instruction disassembled) */
		dAddr += (WORD)d->bytesProcessed;
		}

	free(d);
	free(pMem);

	getc(stdin);

	exit(0);
}
