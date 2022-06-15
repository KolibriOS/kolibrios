/*
	Z80 Disassembler Module v2.0

		Copyright 1996-2002 Mark Incley.
*/

#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "dissz80p.h"


int LookOpcode(DISZ80 *d, int offset)
{
	assert(d != NULL);
	if (d->flags & DISFLAG_CALLBACK)
		return d->Z80MemCB((void *)d->Z80MemBase,
				   ((d->PC + offset) & 0xffff));
	return d->Z80MemBase[(d->PC + offset) & 0xffff];
}


int GetNextOpCode(DISZ80 *d)
{	
	char		buf[8];
	
	assert(d != NULL);

	if (d->flags & DISFLAG_CALLBACK)
		d->op = d->Z80MemCB((void *)d->Z80MemBase, d->PC++);
	else
		d->op = d->Z80MemBase[d->PC++];

	if (d->PC == 0)
		d->haveWrapped = TRUE;
	
	if (d->currentPass == DPASS_WRITE && d->labelledOutput == FALSE)
		{
		sprintf(buf, "%02x", (BYTE)d->op);
		strcat(d->hexDisBuf, buf);
		}
	
	d->bytesProcessed++;
	return d->op;
}

char *dZ80_GetVersionString(void)
{
	return VersionString;
}

/*
	void dZ80_SetDefaultOptions(DISZ80 *d)

	Set the default radix (hex), comment marker and DB directive string.
*/

void dZ80_SetDefaultOptions(DISZ80 *d)
{
	assert(d != NULL);

/* Standard comment and DB */
	strcpy(d->layoutComment, "; ");
	strcpy(d->layoutDefineByte, "db");

/* Hex by default */
	dZ80_SetRadix(d, DRADIX_DEFAULT);
	return;
}


void dZ80_InheritRadix(DISZ80 *dst, DISZ80 *src)
{
	assert(dst != NULL && src != NULL);
	assert(dst != src);

	dst->layoutRadix = src->layoutRadix;
	strcpy(dst->layoutComment, src->layoutComment);
	strcpy(dst->layoutDefineByte, src->layoutDefineByte);
	strcpy(dst->layoutNumberPrefix, src->layoutNumberPrefix);
	strcpy(dst->layoutNumberSuffix, src->layoutNumberSuffix);
	return;
}

void dZ80_SetRadix(DISZ80 *d, int radix)
{
	assert(d != NULL);
	assert(radix >= 0 && radix < DRADIX_TOTAL);

	d->layoutNumberPrefix[0] = d->layoutNumberSuffix[0] = 0;
	d->layoutRadix = radix;

	switch(radix)
		{
		case DRADIX_OCTAL:
			strcpy(d->layoutNumberPrefix, "0");
			break;

		case DRADIX_DECIMAL:
			break;

		case DRADIX_HEX:
			strcpy(d->layoutNumberPrefix, "{");
			strcpy(d->layoutNumberSuffix, "h");
			break;
		}

	return;
}



void PrepareDisInstruction(DISZ80 *d)
{
	assert(d != NULL);
	
	d->hexDisBuf[0] = d->disBuf[0] = d->commentBuf[0] = 0;
	d->Z80Flags = d->numRecursions = 0; 		/* Clear and IX/IY prefixes, etc. */
	d->lineCmd = 0; 							/* Clear out line commands */
	d->disBufIndex = 0; 						/* Point to the start of the line again */
	d->haveTabbed = FALSE;
	d->lastPC = d->PC;
	return;
}
		
int dZ80_Disassemble(DISZ80 *d)
{
	int 	i, err, skipped;
	char	buf[256], num1[16], num2[16];

	assert(d != NULL);

	if (d == NULL)
		return DERR_INVALIDPARAMS;

	if ((d->flags & DISFLAG_CALLBACK) && (d->memCB == NULL))
		return DERR_INVALIDPARAMS;
	else if (d->mem0Start == NULL)
		return DERR_INVALIDPARAMS;

	d->createdRefOK = FALSE;
	d->numInstructions = 0;
	d->currentPass = DPASS_INIT;
	d->Z80MemBase = d->mem0Start;
	d->Z80MemCB = d->memCB;
	d->totalPasses = 1;
	d->labelledOutput = FALSE;
	d->outStream =	NULL;
	d->fnMap = NULL;
		
	if ( (!(d->flags & DISFLAG_SINGLE)) && (d->outFileName[0] == 0) )
		d->dissingToConsole = TRUE;
	else
		d->dissingToConsole = FALSE;

/* Has labelled output been requested? */
	if (d->flags & DISFLAG_LABELLED)
		{
		d->totalPasses = 2;
		d->labelledOutput = TRUE;
/* Switch off the opcode and address dump */
		d->flags &= ~(DISFLAG_ADDRDUMP | DISFLAG_OPCODEDUMP);
		}
	
	for(i=0; i < DISREF_TOTAL; i++)
		{
		d->pRefHead[i] = NULL;
		d->numRefs[i] = 0;
		}
	
	d->disStart = d->start;
	d->disEnd = d->end;

	if (d->disEnd < d->disStart)
		{
		d->lastPC = d->disStart;
		d->disStart = d->disEnd;
		d->disEnd = d->lastPC;
		}

	d->bytesToProcess = ((DWORD)d->disEnd - (DWORD)d->disStart + 1) * d->totalPasses;
	d->bytesProcessed = 0;

/*
	2.0: Perform the single instruction disassembly now
*/
	if (d->flags & DISFLAG_SINGLE)
		{
		d->flags &= ~DISFLAG_ANYREF;
		d->flags |= DISFLAG_QUIET;
		d->currentPass = DPASS_WRITE;
		StartPass(d);
		DisassembleInstruction(d);
		WriteDisLine(d, d->lastPC);
		return DERR_NONE;
		}

/*
	We never get here if DISFLAG_SINGLE has been specified 
*/

	if (!(d->flags & DISFLAG_QUIET))
		{
		sprintf(buf, "       Input file: %s", d->srcFileName);
		dZ80_ShowMsg(d, buf);

		sprintf(buf, "      Output file: %s", (d->outFileName[0]) ? d->outFileName : "(none)");
		dZ80_ShowMsg(d, buf);

		if (d->scriptFileName[0])
			{
			sprintf(buf, "      Script file: %s", d->scriptFileName);
			dZ80_ShowMsg(d, buf);
			}

		if (d->opMapFileName[0])
			{
			sprintf(buf, "  Opcode Map file: %s", d->opMapFileName);
			dZ80_ShowMsg(d, buf);
			}
	
		Make16BitNum(d, num1, d->disStart);
		Make16BitNum(d, num2, d->disEnd);
		sprintf(buf, "Disassembly range: %s - %s", num1, num2);
		dZ80_ShowMsg(d, buf);
		
		dZ80_ShowMsg(d, "");
		}

/* 2.0: Opcode map file is allocated regardless of whether an opcode map file is loaded */
	if ((err = PrepareOpMap(d)) != DERR_NONE)
		{
		DisZ80CleanUp(d);
		return err;
		}

/* 2.0: If we're using a script to trap opcodes, set it up now */
	if ((err = InitOpcodeTraps(d)) != DERR_NONE)
		{
		DisZ80CleanUp(d);
		return err;
		}
		
/* Assembler output (2 passes) ? */
	if (d->labelledOutput == TRUE)
		{
		if ((d->fnMap = AllocateMap(d, "Couldn't allocate memory for the function map.", Z80MEMSIZE))	== NULL)
			{
			DisZ80CleanUp(d);
			return DERR_OUTOFMEM;
			}

		for (i=0; i < 8; i++)
			FlagFn(d, i << 3);					/* Flag the RST addresses */
		}

/* If a destination file has been given, disassemble to file. */
	if ((err = CreateOutputASMFile(d)) != DERR_NONE)
		{
		DisZ80CleanUp(d);
		return err;
		}

/* Draw the progress indicator */
		DoProgress(d, TRUE);

/* Ok, at last, here's the disassembly loop! */
	for (d->currentPass = d->totalPasses; d->currentPass > 0; d->currentPass--)
		{
		StartPass(d);

		if (!(d->flags & DISFLAG_QUIET))
			{
			sprintf(buf, "Pass %d of %d", d->totalPasses - d->currentPass + 1, d->totalPasses);
			dZ80_ShowMsg(d, buf);
			}

		while (WithinDisRange(d))
			{
			DoProgress(d, FALSE);

/* Check if we should skip this */
			skipped = FALSE;
			d->firstByte = (BYTE)LookOpcode(d, 0);

			if (! ISCODEBYTE(d, d->PC))
				{
				skipped = TRUE;
				AddToDisTabDB(d);
				Make8BitNum(d, buf, GetNextOpCode(d));
				AddToDis(d, buf);
				}
	
/* Execute the pre-trap script and if we added an instruction */
			if (!skipped)
				{
				if (ExecPreTrap(d))
					continue;
/* And do the deeds */
				DisassembleInstruction(d);
				}
							
/* Write out the disline (where the disassembly is built up) */
			err = WriteDisLine(d, d->lastPC);
			if (err)
				{
				DisZ80CleanUp(d);
				return err;
				}
				
/* Execute any post trap */
			if (!skipped)
				ExecPostTrap(d);
			}					/* Next instruction */

		DoProgress(d, TRUE);
		}						/* Next pass */

	if (!(d->flags & DISFLAG_QUIET))
		{
		sprintf(buf, "\nDisassembled %u instructions.", d->numInstructions);
		dZ80_ShowMsg(d, buf);
		}

	WriteReferenceFile(d);

	DisZ80CleanUp(d);

	return DERR_NONE;			/* Disassembled OK */
}


int WriteDisLine(DISZ80 *d, unsigned int Addr)
{
	int 	i;
	char	disLine[512], buf[128];

	if (d->currentPass == DPASS_WRITE)
		{
		disLine[0] = 0;

/* 2.0 - Upper case the components separately */
		if (d->flags & DISFLAG_UPPER)
			{
			dZ80_StringToUpper(d->hexDisBuf);
			dZ80_StringToUpper(d->disBuf);
			}
			
/* Add the instruction's address ? */
		if (d->flags & DISFLAG_ADDRDUMP)
			{
			MakeLJustified16BitNum(d, buf, Addr);
			strcat(buf, " ");
			strcat(disLine, buf);
			}

/* Add the hex dump? */
		if (d->flags & DISFLAG_OPCODEDUMP)
			{
			sprintf(buf, "%-8s  ", d->hexDisBuf);
			strcat(disLine, buf);
			}

/* Add the disassembly */
		if (d->labelledOutput && IsFnUsed(d, Addr))
			sprintf(disLine, "l%04x:  ", Addr);
		else
			if ((d->flags & (DISFLAG_OPCODEDUMP | DISFLAG_ADDRDUMP)) == 0)
				strcat(disLine, "        ");

		strcat(disLine, d->disBuf);


/* Add any comment */
		if (d->commentBuf[0])
			{
			i = strlen(disLine);
			if (i < COMMENTCOLUMN)
				{
				memset(buf, ' ', COMMENTCOLUMN-i);
				buf[COMMENTCOLUMN - i] = 0;
				strcat(disLine, buf);
				}
			strcat(disLine, d->commentBuf);
			}

/* Finally, add the new-line */
		strcat(disLine,"\n");

/* Really finally, process any line commands */
		if (d->flags & DISFLAG_LINECOMMANDS)
			{
			if (d->lineCmd & LC_BLANKLINE)
				strcat(disLine, "\n");
			}

		if (d->dissingToConsole)
			{
			printf("%s", disLine);
			}
		else
			{
			if (d->outStream != NULL)
				{
				if (!fwrite(disLine, strlen(disLine), 1, d->outStream))
					{
					DisZ80CleanUp(d);
					return DERR_COULDNTWRITETOFILE;
					}
				}
			}

		}	/* d->currentPass == DPASS_WRITE */

/* Increment the # instructions disassembled counter */
	d->numInstructions++;

/* Note that if we're performing a single instruction disassembly, we do not want to 
	clear the buffers */
	if (!(d->flags & DISFLAG_SINGLE))
		PrepareDisInstruction(d);

	return DERR_NONE;
}



/* Release allocated memory, close open files, etc.. */

void DisZ80CleanUp(DISZ80 *d)
{
	int 		i;
	DISREF		*dr, *drLast;
	DISREFADDR	*ra, *raLast;

	assert(d != NULL);

	if (d->outStream != NULL)
		{
		fclose(d->outStream);
		d->outStream =	NULL;
		}

	for(i=0; i < DISREF_TOTAL; i++)
		{
		dr = d->pRefHead[i];
		while(dr != NULL)
			{
			ra = dr->pRefAddrHead;
			while (ra != NULL)
				{
				raLast = ra;
				ra = ra->pNext;
				free(raLast);
				}
			drLast = dr;
			dr = dr->pNext;
			free(drLast);
			}
		d->pRefHead[i] = NULL;
		}
	
	if (d->opMap != NULL)
		{
		free(d->opMap);
		d->opMap = NULL;
		}

	if (d->fnMap != NULL)
		{
		free(d->fnMap);
		d->fnMap = NULL;
		}

	if (d->pTrapMap != NULL)
		{
		free(d->pTrapMap);
		d->pTrapMap = NULL;
		}

	ShutdownScripting(d);
	return;
}


/* Time to disassemble an instruction */

void DisassembleInstruction(DISZ80 *d)
{
	int 	o;
	
	if (++d->numRecursions > 3)
		{
		AddToDisUnknown(d, NULL);
		return;
		}

/* Get initial opcode */
	o = GetNextOpCode(d);

	switch (o)
		{
		case 0xcb:		/* Disassemble the Rotates, SETs and RES's */
			d->Z80Flags |= Z80CB;
			DisCB(d);
			return;

		case 0xed:
			if (d->cpuType == DCPU_Z80GB)
				{
				AddToDisUndocNop(d);
				}
			else
				{
				d->Z80Flags |= Z80ED;
				DisED(d);
				}
			return;

		case 0xdd:
			if (d->cpuType == DCPU_Z80GB)
				{
				AddToDisUndocNop(d);
				}
			else
				{
				d->Z80Flags |= Z80IX;		/* Signal IX prefix */
				d->Z80Flags &= ~Z80IY;		/* Clear IY prefix */
				DisassembleInstruction(d);
				}
			return;

		case 0xfd:
			if (d->cpuType == DCPU_Z80GB)
				{
				AddToDisUndocNop(d);
				}
			else
				{
				d->Z80Flags |= Z80IY;		/* Signal IY prefix */
				d->Z80Flags &= ~Z80IX;		/* Clear IX prefix */
				DisassembleInstruction(d);
				}
			return;
		}

	d->realop = d->op;

	if (o < 0x40)
		{
		Dis00to3F(d);
		return;
		}

	if (o >= 0x40 && o <= 0x7f)
		{
		Dis40to7F(d);			/* All the LD's */
		return;
		}

	if (o >= 0x80 && o <= 0xbf)
		{
		Dis80toBF(d);
		return;
		}

	if (o >= 0xc0)
		{
		DisC0toFF(d);
		return;
		}

	AddToDisUnknown(d, NULL);
	return;
}


void DisCB(DISZ80 *d)
{
	char			num[3];
	int 			o, oi;

/*
	If there's an IX IY prefix, then the displacement comes *BEFORE* the
	final opcode. LD (IX+dd),nn is a similar case.
*/

	if (d->Z80Flags & Z80IXIY)
		GetIXIYDisplacement(d);

	o = GetNextOpCode(d);

/* Test for undocumented DDCB stuff */

	if (d->Z80Flags & Z80IXIY)
		if ((o & 7) != REG_HL)
			{
			DisDDCB(d);
			return;
			}

	if (o < 0x40)
		{
/* Do the rotates */
		oi = o >> 3;
		if ((oi == 6) && (d->cpuType == DCPU_Z80GB))
			AddToDisTab(d, "swap");
		else
			AddToDisTab(d, (char *)CBRotType[oi & 7]);

		AddToDisReg8(d, o, FALSE);
		}
	else
		{
		switch (o >> 6)
			{
			case 1:
				AddToDisTab(d, "bit");
				break;

			case 2:
				AddToDisTab(d, "res");
				break;

			case 3:
				AddToDisTab(d, "set");
				break;
			}

		num[0] = (char)('0'+ ((o >> 3) & 7));
		num[1] = ',';
		num[2] = 0;
		AddToDis(d, num);

/* Finally, add the register component. */
		AddToDisReg8(d, o, FALSE);
		}
}


/* Decode the DDCB instruction */

void DisDDCB(DISZ80 *d)
{
	char	num[4];
	int 	BitResSet;

	if (d->op < 0x40)
		{
/* Do the undocumented rotates */
		AddToDisTabLD(d, "");
		AddToDisReg8(d, d->op, REG_HL); 	/* REG_HL stops L->IXl, etc.. */
		AddToDis(d, ",");
		AddToDis(d, CBRotType[(d->op >> 3) & 7]);
		AddToDis(d, " ");
		AddToDisReg8(d, REG_HL, FALSE);
		return;
		}

	BitResSet = d->op >> 6;

	if (BitResSet >= 2)
		{
		AddToDisTabLD(d, "");
		AddToDisReg8(d, d->op, REG_HL); 	/* REG_HL stops L->IXl, etc.. */
		AddToDis(d, ",");
		}

	switch (BitResSet)
		{
		case 1:
			AddToDisTab(d, "bit");
			break;
		case 2:
			AddToDisTab(d, "res");
			break;
		case 3:
			AddToDisTab(d, "set");
			break;
		}

	num[0] = ' ';
	num[1] = (char)('0'+ ((d->op >> 3) & 7));
	num[2] = ',';
	num[3] = 0;
	
	AddToDis(d, num + (BitResSet < 2) );

/* Finally, add the register component. */
	AddToDisReg8(d, REG_HL, FALSE);

	if (BitResSet == 1)
		AddToDisUndoc(d);

	return;
}


void DisED(DISZ80 *d)
{
	int 	o;

	o = GetNextOpCode(d);

	if (o <= 0x3f)
	   if (DisED00to3F(d))
		  return;

	if (o >= 0x40 && o <= 0x7f)
	   if (DisED40to7F(d))
		  return;

	if (o >= 0x80 && o <= 0xbf)
		if (DisED80toBF(d))
		   return;

/* Unknown EDxx opcode */
	AddToDisUnknown(d, "Undocumented 8 T-State NOP");
	return;
}

/* ED00 - ED3F are currently only Z180 instructions */

int DisED00to3F(DISZ80 *d)
{
	int 	r, bb;

	if (d->cpuType < DCPU_Z180)
		return FALSE;

	bb = d->op & 7;
	r = (d->op >> 3) & 7;

	switch (bb)
		{
		case 0: 	/* ED 0x00 - 0x38 */
			AddToDisTab(d, "in0");
			AddToDis(d, Reg8Idx[r]);
			AddToDis(d, ",(");
			AddToDis8BitAbs(d, FALSE);
			AddToDis(d, ")");
			AddToDisCommentZ180(d);
			return TRUE;

		case 1: 	/* ED 0x01 - 0x39 */
			AddToDisTab(d, "out0");
			AddToDis(d, "(");
			AddToDis8BitAbs(d, FALSE);
			AddToDis(d, "),");
			AddToDis(d, Reg8Idx[r]);
			AddToDisCommentZ180(d);
			return TRUE;

		case 4: 	/* ED 0x04 - 0x3c */
			AddToDisTab(d, "tst");
			AddToDis(d, Reg8Idx[r]);
			AddToDisCommentZ180(d);
			return TRUE;
		}

	return FALSE;
}


int DisED40to7F(DISZ80 *d)
{
	char	Address[16];
	int 	EDop;

/* First, let's get those nasty special case opcodes. */

	EDop = d->op;

	switch (EDop)
		{
		case 0x76:		/* This is SLP which clashes with the undocumented Z80's IM 1 */
			if (d->cpuType >= DCPU_Z180)
				{
				AddToDisTab(d, "slp");
				AddToDisCommentZ180(d);
				return TRUE;
				}
			break;

		case 0x4c:		/* The Z180's MLT instructions */
		case 0x5c:
		case 0x6c:
		case 0x7c:
			if (d->cpuType < DCPU_Z180)
				return FALSE;

			AddToDisTab(d, "mlt");
			AddToDis(d, Reg16Idx[(EDop >> 4) & 3]);
			AddToDisCommentZ180(d);
			return TRUE;

		case 0x64:		/* Z180's TST nn */
			if (d->cpuType < DCPU_Z180)
				return FALSE;

			AddToDisTab(d, "tst");
			AddToDis8BitAbs(d, FALSE);
			AddToDisCommentZ180(d);
			return TRUE;

		case 0x74:
			if (d->cpuType < DCPU_Z180)
				return FALSE;

			AddToDisTab(d, "tstio");
			AddToDis(d, "(");
			AddToDis8BitAbs(d, FALSE);
			AddToDis(d, ")");
			AddToDisCommentZ180(d);
			return TRUE;

/* Back to the regular Z80's stuff */

		case 0x45:
			AddToDisTab(d, "retn");
			MARKBLANKLINE;
			return TRUE;

		case 0x47:
			AddToDisTabLD(d, "i,a");
			return TRUE;

		case 0x4d:
			AddToDisTab(d, "reti");
			MARKBLANKLINE;
			return TRUE;

		case 0x4f:
			AddToDisTabLD(d, "r,a");
			return TRUE;

		case 0x57:
			AddToDisTabLD(d, "a,i");
			return TRUE;

		case 0x5f:
			AddToDisTabLD(d, "a,r");
			return TRUE;

		case 0x67:
			AddToDisTab(d, "rrd");
			return TRUE;

		case 0x6f:
			AddToDisTab(d, "rld");
			return TRUE;
		}

	switch (EDop & 7)
		{
		case 0:
			AddToDisTab(d, "in");
			AddToDis(d, Reg8AFIdx[(EDop >> 3) & 7] );
			AddToDis(d, ",(c)");
			return TRUE;

		case 1:
			AddToDisTab(d, "out");
			AddToDis(d, "(c),");
			AddToDis(d, Reg8AFIdx[(EDop >> 3) & 7] );
			return TRUE;

		case 2:
			AddToDisTab(d, (EDop & 0x8) ? "adc" : "sbc");
			AddToDisHLIXIY(d);
			AddToDis(d, ",");
			AddToDisReg16(d, EDop >> 4);
			return TRUE;

		case 3:
			Make16BitNum(d, Address, Get16BitParam(d));
			AddRefEntry(d, d->lastRefAddr, d->lastPC, DISREF_ADDR);
			AddToDisTabLD(d, "");
			if (EDop & 8)
				{
				AddToDisReg16(d, EDop >> 4);
				AddToDis(d, ",");
				AddToDis(d, "(");
				AddToDis(d, Address);
				AddToDis(d, ")");
				}
			else
				{
				AddToDis(d, "(");
				AddToDis(d, Address);
				AddToDis(d, ")");
				AddToDis(d, ",");
				AddToDisReg16(d, EDop >> 4);
				}
			return TRUE;

		case 4:
			AddToDisTab(d, "neg");	/* It's a NEG */
			if (EDop != 0x44)
				AddToDisUndoc(d);	/* But undocumented? */
			return TRUE;

		case 5:
			AddToDisTab(d, "ret");
			AddToDisUndoc(d);
			MARKBLANKLINE;
			return TRUE;

		case 6:
			AddToDisTab(d, "im");		/* Interrupt mode... */
			AddToDis(d, IMModes[(EDop & 0x18) >> 3] );
			if ((EDop == 0x4e) || (EDop >= 0x60))
				AddToDisUndoc(d);
			return TRUE;
		}

	return FALSE;
}


int DisED80toBF(DISZ80 *d)
{
	int 	op;

	op = d->op;

	if (d->cpuType >= DCPU_Z180)
		{
		switch (op)
			{
			case 0x83:		/* otim */
			case 0x8b:		/* otdm */
			case 0x93:		/* otimr */
			case 0x9b:		/* otdmr */
				AddToDisTab(d, Z180RepeatOps[(op >> 3) & 3]);
				AddToDisCommentZ180(d);
				return TRUE;
			}
		}

	if (op >= 0xA0)
		{
		if ((op & 7) > 3)
			return FALSE;		/* Don't know this! */

		AddToDisTab(d, RepeatOps[(op & 3)+((op & 0x18) >> 1)] );
		return TRUE;
		}

	return FALSE;
}


void Dis00to3F(DISZ80 *d)
{
	int 	op;

	op = d->op;

	if (d->cpuType == DCPU_Z80GB)
		{
		switch(op)
			{
			case 0x08:			/* ld (nn),sp */
				AddToDisTabLD(d, "(");
				AddToDis16BitAbs(d, FALSE);
				AddToDis(d, "),sp");
				AddRefEntry(d, d->lastRefAddr, d->lastPC, DISREF_INDIRECT);
				return;

			case 0x10:
				AddToDis(d, "stop");
				return;

			case 0x22:
				AddToDisTabLD(d, "(hli),a");
				return;

			case 0x2a:
				AddToDisTabLD(d, "a,(hli)");
				return;

			case 0x32:
				AddToDisTabLD(d, "(hld),a");
				return;

			case 0x3a:
				AddToDisTabLD(d, "a,(hld)");
				return;
			}
		}

	switch (op & 0x0f)
		{
		case 0:
		case 8:
			switch ((op >> 3) & 7)
				{
				case 0: 	/* 0x00 */
					AddToDisTab(d, "nop");
					return;

				case 1: 	/* 0x08 */
					AddToDisTab(d, "ex");
					AddToDis(d, "af,af'");
					return;

				case 2: 	/* 0x10 */ 
					AddToDisTab(d, "djnz");
					FlagFn(d, AddToDisRel8(d, FALSE));
					return;

				case 3: 	/* 0x18 */
					AddToDisTab(d, "jr");
					FlagFn(d, AddToDisRel8(d, FALSE));
					return;

				default:
					AddToDisTab(d, "jr");
					AddToDis(d, Conditions[(op >> 3) & 3] );
					FlagFn(d, AddToDisRel8(d, TRUE));
					return;
				}
		case 1:
			AddToDisTabLD(d, "");
			AddToDisReg16(d, op >> 4);
			AddToDis16BitAbs(d, TRUE);
			AddRefEntry(d, d->lastRefAddr, d->lastPC, DISREF_ADDR);
			return;

		case 2:
			switch ((op >> 4) & 3)
				{
				case 0: 	/* ld (bc),a */
				case 1: 	/* ld (de),a */
					AddToDisTabLD(d, "(");
					AddToDis(d, Reg16Idx[(op >> 4) & 1] );
					AddToDis(d, "),a");
					return;

				case 2: 	/* 0x22 = ld (nn),hl */
				case 3: 	/* 0x32 = ld (nn),a */
					AddToDisTabLD(d, "(");
					AddToDis16BitAbs(d, FALSE);
					AddToDis(d, "),");
						
					if (d->realop & 0x10)
						{
						AddToDis(d, "a");
						}
					else
						{
						AddToDisHLIXIY(d);
						}

					AddRefEntry(d, d->lastRefAddr, d->lastPC, DISREF_INDIRECT);
					return;
				}

		case 3:
			AddToDisTab(d, "inc");
			AddToDisReg16(d, op>>4);
			return;

		case 4:
			AddToDisTab(d, "inc");
			AddToDisReg8(d, op>>3, op>>3);
			return;

		case 5:
			AddToDisTab(d, "dec");
			AddToDisReg8(d, op>>3, op>>3);
			return;

		case 6:
		case 0x0e:
			AddToDisTabLD(d, "");
			AddToDisReg8(d, op>>3, op>>3);
			AddToDis8BitAbs(d, TRUE);
			return;

		case 7:
		case 0x0f:
			AddToDisTab(d, AccRotType[(op>>3) & 7] );
			return;

		case 9:
			AddToDisTab(d, "add");
			AddToDisHLIXIY(d);
			AddToDis(d, ",");
			AddToDisReg16(d, (op>>4) & 3);
			return;

		case 0x0a:
			switch ((op >> 4) & 3)
				{
				case 0:
				case 1:
					AddToDisTabLD(d, "a,(");
					AddToDis(d, Reg16Idx[(op >> 4) & 1] );
					AddToDis(d, ")");
					return;

				case 2:
				case 3:
					if (op & 0x10)
						AddToDisTabLD(d, "a");
					else
						{
						AddToDisTabLD(d, "");
						AddToDisHLIXIY(d);
						}
					AddToDis(d, ",(");
					AddToDis16BitAbs(d, FALSE);
					AddToDis(d, ")");
					AddRefEntry(d, d->lastRefAddr, d->lastPC, DISREF_INDIRECT);
					return;
				}

		case 0xb:
			AddToDisTab(d, "dec");
			AddToDisReg16(d, (op>>4) & 3);
			return;


		case 0xc:
			AddToDisTab(d, "inc");
			AddToDisReg8(d, op>>3, op>>3);
			return;

		case 0xd:
			AddToDisTab(d, "dec");
			AddToDisReg8(d, op>>3, op>>3);
			return;
		}

	AddToDisUnknown(d, NULL);
	return;
}

void Dis40to7F(DISZ80 *d)
{
	if (d->op == 0x76)
		{
		AddToDisTab(d, "halt");
		return;
		}

	AddToDisTabLD(d, "");
	AddToDisReg8(d, d->realop >> 3, d->realop);
	AddToDis(d, ",");
	AddToDisReg8(d, d->realop, d->realop >> 3);
	return;
}


void Dis80toBF(DISZ80 *d)
{
	int GenOp;

	GenOp = (d->op >> 3) & 7;
	AddToDisTab(d, BasicOps[GenOp]);

	if (GenOp < 2 || GenOp == 3)
		AddToDis(d, "a,");

	AddToDisReg8(d, d->op, d->op);
	return;
}


void DisC0toFF(DISZ80 *d)
{
	int		GenOp, op;
	char		port[32], buf[128], num[16], num2[16];
	signed char	offset;
	WORD		addr;

	op = d->op;

	if (d->cpuType == DCPU_Z80GB)
		{
		switch(op)
			{
			case 0xd3:
			case 0xdb:
			case 0xe3:
			case 0xe4:
			case 0xeb:
			case 0xec:
			case 0xf2:
			case 0xf4:
			case 0xfc:
				AddToDisUndocNop(d);
				return;

			case 0xd9:
				AddToDis(d, "reti");
				return;

			case 0xe0:			/* ld ($ff00+n),a */
			case 0xf0:			/* ld a,($ff00+n) */
				addr = 0xff00 + GetNextOpCode(d);
				Make16BitNum(d, num, 0xff00);
				Make8BitNum(d, num2, addr & 0xff);
				
				AddToDisTabLD(d,"");
				if (op == 0xe0)
					sprintf(buf, "(%s+%s),a", num, num2);
				else
					sprintf(buf, "a,(%s+%s)", num, num2);
				AddToDis(d, buf);
				AddRefEntry(d, addr, d->lastPC, DISREF_INDIRECT);
				return;

			case 0xe2:			/* ld ($ff00+c),a */
				AddToDisTabLD(d, "");
				Make16BitNum(d, num, 0xff00);
				sprintf(buf, "(%s+c),a", num);
				AddToDis(d, buf);
				return;

			case 0xe8:				/* add sp,nn */
				AddToDisTab(d, "add");
				AddToDis(d, "sp");
				AddToDis16BitAbs(d, TRUE);
				return;

			case 0xea:
				addr = Get16BitParam(d);
				Make16BitNum(d, num, addr);
				sprintf(buf, "(%s),a", num);
				AddToDisTabLD(d, buf);
				AddRefEntry(d, addr, d->lastPC, DISREF_INDIRECT);
				return;

			case 0xf8:
				offset = GetNextOpCode(d);
				AddToDisTab(d, "ldhl");
				AddToDis(d, "sp,");
				if (offset >= 0)
					{
					Make8BitNum(d, num, offset);
					sprintf(buf, "%s", num);
					}
				else
					{
					Make8BitNum(d, num, -offset);
					sprintf(buf, "-%s", num);
					}
				AddToDis(d, buf);
				return;

			case 0xfa:
				addr = Get16BitParam(d);
				Make16BitNum(d, num, addr);
				sprintf(buf, "a,(%s)", num);
				AddToDisTabLD(d, buf);
				AddRefEntry(d, addr, d->lastPC, DISREF_INDIRECT);
				return;
			}
		}

	GenOp = (d->op >> 3) & 7;

	switch (op & 0xf)		/* Break it down into eight basics */
		{
		case 0:
		case 8:
			AddToDisTab(d, "ret");
			AddToDis(d, Conditions[GenOp]);
			MARKBLANKLINE;
			return;

		case 1: 		/* POP rr */
		case 5: 		/* PUSH rr */
			AddToDisTab(d, (op & 4) ? "push" : "pop");
			AddToDisReg16NoAnd(d, ((op >> 4) & 3) + (op >= 0xf0));
			return;

		case 2:
		case 0x0a:
			AddToDisTab(d, "jp");
			AddToDis(d, Conditions[GenOp]);
			FlagFn(d, AddToDis16BitAbs(d, TRUE));
			return;

		case 4:
		case 0x0c:
			AddToDisTab(d, "call");
			AddToDis(d, Conditions[GenOp]);
			FlagFn(d, AddToDis16BitAbs(d, TRUE));
			return;

		case 3:
			switch ((op >> 4) & 3)
				{
				case 0: 	/* 0xc3 */
					AddToDisTab(d, "jp");
					FlagFn(d, AddToDis16BitAbs(d, FALSE));
					return;

				case 1: 	/* 0xd3 */
					AddToDisTab(d, "out");
					Make8BitNum(d, port, GetNextOpCode(d));
					sprintf(buf, "(%s),a", port);
					AddToDis(d, buf);
					AddRefEntry(d, d->op, d->lastPC, DISREF_OUTPORT);
					return;

				case 2: 	/* 0xe3 */
					AddToDisTab(d, "ex");
					AddToDis(d, "(sp),");
					AddToDisHLIXIY(d);
					return;

				case 3: 	/* 0xf3 */
					AddToDisTab(d, "di");
					return;
				}

		case 6:
		case 0x0e:
			AddToDisTab(d, BasicOps[GenOp] );
			if (GenOp < 2 || GenOp == 3)
				AddToDis(d, "a,");
			AddToDis8BitAbs(d, FALSE);
			return;

		case 7:
		case 0x0f:
			AddToDisTab(d, "rst");
			Add8BitNum(d, op & (7 << 3));
			return;

		case 9:
			switch ((op >> 4) & 3)
				{
				case 0: 	/* 0xc9 */
					AddToDisTab(d, "ret");
					MARKBLANKLINE;
					return;

				case 1: 	/* 0xd9 */
					AddToDisTab(d, "exx");
					return;

				case 2: 	/* 0xe9 */
					AddToDisTab(d, "jp");
					AddToDis(d, "(");
					AddToDisHLIXIY(d);
					AddToDis(d, ")");
					return;

				case 3: 	/* 0xf9 */
					AddToDisTabLD(d, "sp,");
					AddToDisHLIXIY(d);
					return;
				}


		case 0x0b:
			switch ((op >> 4) & 3)
				{
				case 1: 	/* 0xdb */
					AddToDisTab(d, "in");
					Make8BitNum(d, port, GetNextOpCode(d));
					sprintf(buf,"a,(%s)", port);
					AddToDis(d, buf);
					AddRefEntry(d, d->op, d->lastPC, DISREF_INPORT);
					return;

				case 2: 	/* 0xeb */
					AddToDisTab(d, "ex");
					AddToDis(d, "de,hl");
					return;

				case 3: 	/* 0xfb */
					AddToDisTab(d, "ei");
					return;
				}



		case 0x0d:			/* N.B. this can only get here with #cd */
			AddToDisTab(d, "call");
			FlagFn(d, AddToDis16BitAbs(d, FALSE));
			return;
		}

	AddToDisUnknown(d, NULL);
}


void AddToDis(DISZ80 *d, char *str)
{
	char	c, *p;
	unsigned int i;

	if (d->currentPass == DPASS_WRITE)
		{
		p = d->disBuf;
		i = d->disBufIndex;
		assert(i < sizeof(d->disBuf));

		while ((c = *str++))
			p[i++] = c;

		p[i] = 0;
					
		assert(i < sizeof(d->disBuf));
		d->disBufIndex = i;
		}

	return;
}

void AddToDisTab(DISZ80 *d, char *str)
{
	int 	l;
	char	buf[64];

	if (d->currentPass == DPASS_WRITE)
		{
		AddToDis(d, str);

		if (d->haveTabbed == FALSE)
			{
			l = strlen(d->disBuf);
			memset(buf, ' ', TABSIZE - l);
			buf[TABSIZE - l] = 0;
			AddToDis(d, buf);

			d->haveTabbed = TRUE;
			}
		}
		
	return;
}

void AddToDisTabDB(DISZ80 *d)
{
	AddToDisTab(d, (d->layoutDefineByte == NULL) ? "db" : d->layoutDefineByte);
	return;
}


void AddToDisTabLD(DISZ80 *d, char *str)
{
	AddToDisTab(d, "ld");
	AddToDis(d, str);
	return;
}


void AddToDisCommentZ180(DISZ80 *d)
{
	AddToDisComment(d, "Z180 instruction");
	return;
}

void AddToDisComment(DISZ80 *d, char *str)
{
	if (d->currentPass == DPASS_WRITE)
		{
		if (!(d->Z80Flags & Z80COMMENT))
			{
			strcpy(d->commentBuf, d->layoutComment);
			d->Z80Flags |= Z80COMMENT;
			}

		assert((strlen(d->commentBuf) + strlen(str)) < sizeof(d->commentBuf));

		strcat(d->commentBuf, str);
		}
	
	return;
}

void AddToDisHLIXIY(DISZ80 *d)
{
	if (d->Z80Flags & Z80IXIY)
		{
		AddToDis(d,  (d->Z80Flags & Z80IX) ? "ix" : "iy");
		}
	else
		{
		AddToDis(d, "hl");
		}

	return;
}


/*	AddToDisReg8(opcode)

	Adds b,c,d,e,h,l,(hl) or a to disassembly, taking into consideration
	the IX and IY prefixes. "op2" is used to determine whether a "IXl"
	reference is valid or not.
*/

void AddToDisReg8(DISZ80 *d, int op, int op2)
{
	char	num[16];
	char	buf[64];

	op &= 7;

	if (d->Z80Flags & Z80IXIY)
		{
		op2 &= 7;

		if (!(op & (Z80CB | Z80ED)) && (op2 != REG_HL) )
			{
			if (op == REG_L)
				{
				AddToDis(d, (d->Z80Flags & Z80IX) ? "ixl" : "iyl");
				return;
				}
		
			if (op == REG_H)
				{
				AddToDis(d, (d->Z80Flags & Z80IX) ? "ixh" : "iyh");
				return;
				}
			}

		if (op == REG_HL)
			{
			GetIXIYDisplacement(d);

			if (d->IXIYDisp >= 0)
				{
				Make8BitNum(d, num, d->IXIYDisp);
				sprintf(buf, "(%s+%s)", ((d->Z80Flags & Z80IX) ? "ix" : "iy"), num);
				}
			else
				{
				Make8BitNum(d, num, -d->IXIYDisp);
				sprintf(buf, "(%s-%s)", ((d->Z80Flags & Z80IX) ? "ix" : "iy"), num);
				}

			AddToDis(d, buf);
			return;
			}
		}

	AddToDis(d, Reg8Idx[op]);
	return;
}


/*	AddToDisReg16(opcode)

	Adds bc,de,hl or sp to disassembly, taking into consideration
	the IX and IY prefixes
*/

void AddToDisReg16(DISZ80 *d, int op)
{
	op &= 3;
	if ( (op == 2) && (d->Z80Flags & Z80IXIY) )
		AddToDisHLIXIY(d);
	else
		AddToDis(d, Reg16Idx[op]);
	return;
}

void AddToDisReg16NoAnd(DISZ80 *d, int op)
{
	if ( (op == 2) && (d->Z80Flags & Z80IXIY) )
		AddToDisHLIXIY(d);
	else
		AddToDis(d, Reg16Idx[op]);
	return;
}


WORD AddToDisRel8(DISZ80 *d, int CommaFlag)
{
	char		buf[64], num[16];
	signed char	o;

	if (CommaFlag)
		AddToDis(d, ",");

	o = (char)GetNextOpCode(d);

	d->lastRefAddr = (WORD)(d->PC + o);
	Add16BitAddress(d, d->lastRefAddr);

	if (d->flags & DISFLAG_RELCOMMENT)
		{
		Make8BitNum(d, num, (o >= 0) ? o : -o);
		sprintf(buf, "(%s%s)", (o >= 0) ? "+" : "-", num);
		AddToDisComment(d, buf);
		}

	return d->lastRefAddr;
}

void AddToDis8BitAbs(DISZ80 *d, int CommaFlag)
{
	if (CommaFlag)
		AddToDis(d, ",");

	GetNextOpCode(d);
	Add8BitNum(d, d->op);
	return;
}

WORD AddToDis16BitAbs(DISZ80 *d, int CommaFlag)
{
	if (CommaFlag)
		AddToDis(d, ",");

	Get16BitParam(d);
	Add16BitAddress(d, d->lastRefAddr);
	return d->lastRefAddr;
}

void AddToDisUndoc(DISZ80 *d)
{
	AddToDisComment(d, "Undocumented");
	return;
}

void AddToDisUndocNop(DISZ80 *d)
{
	AddToDis(d, "nop");
	AddToDisComment(d, "Undocumented");
	return;
}



/*
	void AddToDisUnknown(char *Comment) 

	Handles the dumping of an unknown opcode sequence
*/

void AddToDisUnknown(DISZ80 *d, char *Comment)
{
	int 	i, numOpCodes;
	char	buf[64];

	if (d->currentPass != DPASS_WRITE)
		return;

	AddToDisTabDB(d);
	
	numOpCodes = abs(d->PC - d->lastPC);
	d->PC = d->lastPC;

/* We're going to rewind back to the start of the bad opcode, so clear the hex stream dump */
	d->hexDisBuf[0] = 0;			

	for (i = 0; i < numOpCodes; i++)
		{
		Make8BitNum(d, buf, GetNextOpCode(d));
		if (i < (numOpCodes-1))
			strcat(buf, ", ");
		AddToDis(d, buf);
		}

	if (Comment == NULL)
		AddToDisComment(d, "Unknown opcode");
	else
		AddToDisComment(d, Comment);

	return;
}


char GetIXIYDisplacement(DISZ80 *d)
{
	if (!(d->Z80Flags & Z80GOTIXIYDISP))		/* Already got IXIY displacement? */
		{
		d->IXIYDisp = (char)GetNextOpCode(d);
		d->Z80Flags |= Z80GOTIXIYDISP;
		}

	return d->IXIYDisp;
}


WORD Get16BitParam(DISZ80 *d)
{
	d->lastRefAddr = (WORD)GetNextOpCode(d);
	d->lastRefAddr |= (GetNextOpCode(d) << 8);
	return d->lastRefAddr;
}


void FlagFn(DISZ80 *d, unsigned int Addr)
{
	if (d->labelledOutput == FALSE || d->currentPass != DPASS_ANALYSE)
		return;

	assert(d->fnMap != NULL);
	assert(Addr < Z80MEMSIZE);
	d->fnMap[Addr] = TRUE;
	return;
}


int IsFnUsed(DISZ80 *d, unsigned int Addr)
{
	assert(Addr < Z80MEMSIZE);

	if (d->fnMap != NULL)
		return (d->fnMap[Addr]);
	
	return 0;
}



BYTE* AllocateMap(DISZ80 *d, char *errorStr, unsigned int bytesWanted)
{
	BYTE	*pMap;

	pMap = (BYTE *)malloc(bytesWanted);

	if (pMap == NULL)
		dZ80_Error(d, errorStr);
	else
		memset(pMap, 0, bytesWanted);
		
	return pMap;
}


/* Create the output .asm file and header */
int CreateOutputASMFile(DISZ80 *d)
{
	char	MsgBuf[_MAX_PATH + 80];
	char	buf[256];
	time_t	secs_now;
	struct	tm *time_now;

/* Set up the time structures */
	time(&secs_now);
	time_now = localtime(&secs_now);

	if (d->outFileName[0])
		{
		d->outStream = fopen(d->outFileName, "wt");

		if (d->outStream == NULL)
			{
			sprintf(buf, "Couldn't create the output file %s\n", d->outFileName);
			dZ80_Error(d, buf);
			DisZ80CleanUp(d);
			return DERR_COULDNTCREATEFILE;
			}

		sprintf(MsgBuf, "%sDisassembly of the file \"%s\"\n%s\n", d->layoutComment, d->srcFileName, d->layoutComment);
		fwrite(MsgBuf, strlen(MsgBuf), 1, d->outStream);

		sprintf(MsgBuf, "%sCPU Type: %s\n%s\n", d->layoutComment, dZ80CpuTypeNames[d->cpuType], d->layoutComment);
		fwrite(MsgBuf, strlen(MsgBuf), 1, d->outStream);

		if (d->opMapFileName[0])
			{
			sprintf(MsgBuf, "%sUsing the opcode map file \"%s\"\n%s", d->layoutComment, d->opMapFileName, d->layoutComment);
			fwrite(MsgBuf, strlen(MsgBuf), 1, d->outStream);
			}

		sprintf(MsgBuf, "%sCreated with dZ80 %s\n%s", d->layoutComment, VersionString, d->layoutComment);
		fwrite(MsgBuf, strlen(MsgBuf), 1, d->outStream);

		if (d->scriptFileName[0])
			{
			sprintf(MsgBuf, "   using script \"%s\"\n%s", d->scriptFileName, d->layoutComment);
			fwrite(MsgBuf, strlen(MsgBuf), 1, d->outStream);
			}

		sprintf(MsgBuf, "\n%s", d->layoutComment);
		fwrite(MsgBuf, strlen(MsgBuf), 1, d->outStream);

		strftime(MsgBuf, sizeof(MsgBuf), "on %A, %d of %B %Y at %I:%M %p", time_now);
		fwrite(MsgBuf, strlen(MsgBuf), 1, d->outStream);

		sprintf(MsgBuf, "\n%s\n", d->layoutComment);
		fwrite(MsgBuf, strlen(MsgBuf), 1, d->outStream);

		if (d->labelledOutput)
			{
			PrepareDisInstruction(d);
			AddToDisTab(d, "org");
			Make16BitNum(d, MsgBuf, d->disStart);
			strcat(MsgBuf,"\n");
			AddToDis(d, MsgBuf);
			WriteDisLine(d, d->lastPC);
			}
		}

	return DERR_NONE;
}


/*
	PrepareOpMap()

	This was changed for dZ80 2.0 so that the opcode map is always allocated and initialised with
	all bits set to "code". This was to allow for the script to manipulate the opMap regardless of
	whether one had been loaded.
*/

int dZ80_AllocateOpMap(DISZ80 *d)
	{     
	if (d->opMap == NULL)
		{
		if ((d->opMap = AllocateMap(d, "Couldn't allocate memory for the opcode map.", Z80MEMSIZE/8)) == NULL)
			{
			DisZ80CleanUp(d);
			return DERR_OUTOFMEM;
			}

/* Set all the bits to "code" */
	memset(d->opMap, 0xff, Z80MEMSIZE / 8);
		}
	
	return DERR_NONE;
}


int PrepareOpMap(DISZ80 *d)
{
	char	buf[256];
	int		err;
	FILE	*opStream;

	err = dZ80_AllocateOpMap(d);
	if (err)
		return err;
		
	if (d->opMapFileName[0])
		{
		opStream = fopen(d->opMapFileName, "rb");

		if (opStream == NULL)
			{
			sprintf(buf, "Couldn't open the opcode map file \"%s\"", d->opMapFileName);
			dZ80_Error(d, buf);
			DisZ80CleanUp(d);
			return DERR_COULDNTOPENFILE;
			}

		if (fread(d->opMap, 1, Z80MEMSIZE / 8, opStream) < (Z80MEMSIZE / 8) )
			dZ80_ShowMsg(d, "Warning: Couldn't read the entire opcode map file");

		fclose(opStream);
		}
	
	return DERR_NONE;
}



void WriteReferenceFile(DISZ80 *d)
{
	unsigned int	t, n;
	char			buf[80 + _MAX_PATH];
	char			*TypeMsg;
	char			num1[32], num2[32];
	DISREF			*p;
	DISREFADDR		*ra;
	DISZ80			qd;
	FILE			*refStream;

	if (!(d->flags & DISFLAG_ANYREF))
		return;
	
	if (d->refFileName[0] == 0)
		{
		dZ80_Error(d, "Missing reference filename.");
		return;
		}

	if (!(d->flags & DISFLAG_QUIET))
		{
		sprintf(buf, "Writing reference file: %s", d->refFileName);
		dZ80_ShowMsg(d, buf);
		}

	refStream = fopen(d->refFileName, "wt");
	if (refStream == NULL)
		{
		sprintf(buf, "Couldn't create the reference file \"%s\".", d->refFileName);
		dZ80_Error(d, buf);
		return;
		}

	sprintf(buf,"dZ80 %s Reference file from the disassembly of \"%s\".\n\n", VersionString, d->srcFileName);
	fwrite(buf, 1, strlen(buf), refStream);

	for(t = 0; t < DISREF_TOTAL; t++)
		{
		switch(t)
			{
			case DISREF_INPORT:
				TypeMsg = "Input Port";
				break;

			case DISREF_OUTPORT:
				TypeMsg = "Output Port";
				break;

			case DISREF_ADDR:
				TypeMsg = "Direct address";
				break;
			
			case DISREF_INDIRECT:
				TypeMsg = "Indirect address";
				break;

			default:
				assert(FALSE);
				TypeMsg = "wah?";
				break;
			}

		n = d->numRefs[t];
		if (n)
			{
			sprintf(buf, "%s Reference (%d entries)\n", TypeMsg, n);
			fwrite(buf, 1, strlen(buf), refStream);
			UnderlineText(refStream, buf);

			p = d->pRefHead[t];
			while(p != NULL)
				{
				Make16BitNum(d, num1, p->Addr);
				sprintf(buf, "%s %s. %u references:\n", TypeMsg, num1, p->Hits);
				fwrite(buf, 1, strlen(buf), refStream);
				UnderlineText(refStream, buf);
				
				ra = p->pRefAddrHead;
				while (ra != NULL)
					{
					memset(&qd, 0, sizeof(qd));
					qd.mem0Start = d->mem0Start;
					qd.memCB = d->memCB;
					qd.start = qd.end = ra->RefAddress;
					qd.flags = d->flags | DISFLAG_SINGLE;
					qd.cpuType = d->cpuType;
					dZ80_InheritRadix(&qd, d);
		
					dZ80_Disassemble(&qd);
					
					Make16BitNum(d, num2, ra->RefAddress);
					sprintf(buf, "%8s: %s\n", num2, qd.disBuf);
					fwrite(buf, 1, strlen(buf), refStream);

					ra = ra->pNext;
					}

				sprintf(buf, "\n");
				fwrite(buf, 1, strlen(buf), refStream);
				p = p->pNext;
				}

			sprintf(buf, "\n");
			fwrite(buf, 1, strlen(buf), refStream);
			}
		}

	sprintf(buf, "End of reference file for \"%s\"\n\n", d->srcFileName);
	fwrite(buf, 1, strlen(buf), refStream);

	fclose(refStream);
	d->createdRefOK = TRUE;
	return;
}


void UnderlineText(FILE *stream, char *text)
{
	int 	l;
	char	buf[256];

	l = strlen(text) - 1;
	memset(buf, '-', l);
	buf[l] = 0;
	strcat(buf, "\n\n");
	fwrite(buf, 1, strlen(buf), stream);
	return;
}


void AddRefEntry(DISZ80 *d, int Addr, int PC, int refType)
{
	DISREF		*p, *pIns, *pPrev;

/* Don't add reference entries if we're scanning for functions */
	if ((d->currentPass != DPASS_WRITE) || (!(d->flags & DISFLAG_ANYREF)))
		return;

	switch (refType)
		{
		case DISREF_INPORT:
			if (!(d->flags & DISFLAG_REFINPORT))
				return;
			break;

		case DISREF_OUTPORT:
			if (!(d->flags & DISFLAG_REFOUTPORT))
				return;
			break;

		case DISREF_ADDR:
		case DISREF_INDIRECT:
			if (d->flags & DISFLAG_REFLIMITRANGE)
				{
				if (Addr >= d->disStart && Addr <= d->disEnd)
					return;
				}

			if ((refType == DISREF_ADDR) && !(d->flags & DISFLAG_REFADDR))
				return;

			if ((refType == DISREF_INDIRECT) && !(d->flags & DISFLAG_REFINDIRECT))
				return;

			break;

		default:
			assert(FALSE);
			return;
		}

/* 
	Locate the insertion point of this new entry, or if
	  there's already an existing one (same address), use that
*/
	pIns = d->pRefHead[refType];
	pPrev = NULL;
	
	while (pIns != NULL)
		{
		if (pIns->Addr == Addr)
			{
			AddReferenceAddr(d, pIns, PC);
			return;
			}

		if (pIns->Addr > Addr)
			break;
		
		pPrev = pIns;
		pIns = pIns->pNext;
		}


/* We have a brand new entry (either at the head or tail of the list, or needs to be crow-barred in :) */
	p = malloc(sizeof(DISREF));
	if (p == NULL)
		return;

	memset(p, 0, sizeof(DISREF));

/* First entry for this reference type? */
	
	if (pPrev != NULL)
		{
		p->pPrev = pPrev;
		pPrev->pNext = p;
		}
	else
		{
		d->pRefHead[refType] = p;
		}

	if (pIns != NULL)
		{
		p->pNext = pIns;
		pIns->pPrev = p;
		}


#if 0
	p->pPrev = pPrev;					/* Link to previous entry (if any) */
	p->pNext = pIns;					/* Link the next to the..erm..next */

	if (pPrev != NULL)
		pPrev->pNext = p;				/* Link the following entry to this one (barge in)	*/

	if (d->pRefHead[refType] == NULL)
		d->pRefHead[refType] = p;		/* Must be the first in the list */
		
	if (pIns != NULL)
		pIns->pPrev = p;				/* Link the following entry to me */
#endif

	p->RefType = refType;
	p->Addr = (WORD)Addr;
	p->Hits = 0;
	
	AddReferenceAddr(d, p, PC);
	return;
}


void AddReferenceAddr(DISZ80 *d, DISREF *p, int PC)
{
	DISREFADDR	*n, *t;


	n = malloc(sizeof(DISREFADDR));
	if (n == NULL)
		return;

	memset(n, 0, sizeof(DISREFADDR));

	t = p->pRefAddrTail;

	if (t == NULL)
		{
		p->pRefAddrHead = p->pRefAddrTail = n;
		}
	else
		{
		assert(t->RefAddress < PC);
		t->pNext = n;
		p->pRefAddrTail = n;
		}

	n->RefAddress = (WORD)PC;
	p->Hits++;

	d->numRefs[p->RefType]++;
	return;
}


void DoProgress(DISZ80 *d, int forceUpdate)
{
	if (!(d->flags & DISFLAG_QUIET))
		{
		if (d->fnProgressCallback != NULL)
			{
			d->progressCounter--;
			if ((forceUpdate) || d->progressCounter <= 0)
				{
				d->fnProgressCallback(d);
				d->progressCounter = PROGRESSUPDATEFREQ;
				}
			}
		}
	
	return;
}


void Add16BitAddress(DISZ80 *d, WORD Addr)
{
	char	buf[32];

	if ((d->labelledOutput == TRUE) && (d->currentPass == DPASS_WRITE) && (d->flags & DISFLAG_USELABELADDRS))
		{
		if (IsFnUsed(d, Addr))
			{
			sprintf(buf, "l%04x", Addr);
			AddToDis(d, buf);
			return;
			}
		}

	Add16BitNum(d, Addr);
	return;
}


void Add8BitNum(DISZ80 *d, int Num)
{
	char	buf[32];

	Make8BitNum(d, buf, Num);
	AddToDis(d, buf);
	return;
}


void Make8BitNum(DISZ80 *d, char *Dst, int Num)
{
	char	num[16];

	switch(d->layoutRadix)
		{
		case DRADIX_OCTAL:
			sprintf(num, "%o", Num);
			break;

		case DRADIX_DECIMAL:
			sprintf(num, "%d", Num);
			break;

		case DRADIX_HEX:
			sprintf(num, "%02x", Num);
			break;

		default:
			num[0] = '\0';
			break;
		}

/*	Added in 2.0 - "{" is a special case that adds a "0" only if the first digit
	is non-numeric
*/

	if (d->layoutNumberPrefix[0] == '{')
		sprintf(Dst, "%s%s%s", (num[0] <'0' || num[0] > '9') ? "0" : "", num, d->layoutNumberSuffix);
	else
		sprintf(Dst, "%s%s%s", d->layoutNumberPrefix, num, d->layoutNumberSuffix);

	return;
}

void Add16BitNum(DISZ80 *d, int Num)
{
	char	buf[32];

	Make16BitNum(d, buf, Num);
	AddToDis(d, buf);
	return;
}

void Make16BitNum(DISZ80 *d, char *Dst, int Num)
{
	char	num[8];

	switch(d->layoutRadix)
		{
		case DRADIX_OCTAL:
			sprintf(num, "%o", Num);
			break;

		case DRADIX_DECIMAL:
			sprintf(num, "%d", Num);
			break;

		case DRADIX_HEX:
			sprintf(num, "%04x", Num);
			break;

		default:
			num[0] = '\0';
			break;
		}

/*
	Added in 2.0 - "{" is a special case that adds a "0" only if the first digit
	is non-numeric
*/

	if (d->layoutNumberPrefix[0] == '{')
		sprintf(Dst, "%s%s%s", (num[0] <'0' || num[0] > '9') ? "0" : "", num, d->layoutNumberSuffix);
	else
		sprintf(Dst, "%s%s%s", d->layoutNumberPrefix, num, d->layoutNumberSuffix);
	
	return;
}

void MakeLJustified16BitNum(DISZ80 *d, char *dst, int num)
{
	switch(d->layoutRadix)
		{
		case DRADIX_OCTAL:
			sprintf(dst, "%6o", num);
			break;

		case DRADIX_DECIMAL:
			sprintf(dst, "%5d", num);
			break;

		case DRADIX_HEX:
			sprintf(dst, "%04x", num);
			break;
		}

	return;
}


void dZ80_StringToLower(char *s)
{
	while (*s) {
		*s = (char)tolower(*s);
		s++;
	}

	return;
}

void dZ80_StringToUpper(char *s)
{
	while (*s) {
		*s = (char)toupper(*s);
		s++;
	}

	return;
}

void dZ80_Error(DISZ80 *d, char *msg)
{
	if (d->fnErrorMessage != NULL)
		d->fnErrorMessage(msg);

	return;
}

void dZ80_ShowMsg(DISZ80 *d, char *msg)
{
	if(d->fnOutputMessage != NULL)
		d->fnOutputMessage(msg);

	return;
}

void dZ80_SafeStringCopy(char *dst, char *src, int dstSize)
{
	int 	l;

	l = strlen(src);
	if (l < dstSize)
		{
		strcpy(dst, src);
		}
	else
		{
		strncpy(dst, src, dstSize - 1);
		dst[dstSize - 1] = 0;
		}
	
	return;
}
	

const char *dZ80_GetErrorText(int errNum)
{
	if (errNum >= 0 && errNum < DERR_TOTAL)
		return dZ80ErrorMsgs[errNum];

	return "dZ80_GetErrorText: bad error #";
}


void StartPass(DISZ80 *d)
{
	d->numInstructions = 0; 				/* Number of instructions disassembled */
	d->haveWrapped = FALSE; 				/* PC hasn't wrapped around */
	d->PC = d->disStart;

/* Clear the disassembly buffer */
	PrepareDisInstruction(d);
	return;
}

int WithinDisRange(DISZ80 *d)
{
	return ((d->PC <= d->disEnd) && (!d->haveWrapped));
}

