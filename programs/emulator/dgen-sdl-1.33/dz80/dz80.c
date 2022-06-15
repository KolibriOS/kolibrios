/*
		dZ80 - Z80 Disassembler v2.0

		Written by Mark Incley, 1st November, 1996 (v1.0)
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#include "types.h"
#include "dissz80.h"
#include "dz80.h"

char	reqLayoutNumberPrefix[D_CUSTOMSTRING_MAXLEN], reqLayoutNumberSuffix[D_CUSTOMSTRING_MAXLEN];
int 	disRadix, showVersion;

static	DWORD bytesLoaded;
static void ProgressUpdate(DISZ80 *d);

/* Not a great big table at the moment :) */

DISFILE FileExtList[]=
{
	{"sna", 27, 16384}, 	/* .SNA - Spectrum Snapshots */
	{NULL, 0, 0}
};


int main(int argc, char* argv[])
{
	char		buf[256];
	int			err, startArgc;
	DISZ80		*d;

	sprintf(buf, "dZ80 %s, Copyright 1996-2002 by Mark Incley.\n", dZ80_GetVersionString());
	PrintToConsole(buf);

	d = malloc(sizeof(DISZ80));
	if (d == NULL)
		{
		PrintToErrOut("Couldn't allocate DISZ80 structure");
		exit(1);
		}

/* Set up our DISZ80 structure */
	memset(d, 0, sizeof(DISZ80));

	d->fnOutputMessage	= PrintToConsole;
	d->fnErrorMessage	= PrintToErrOut;
	d->fnProgressCallback = ProgressUpdate;

	d->cpuType		= DCPU_Z80;
	d->flags		= DISFLAG_OPCODEDUMP | DISFLAG_ADDRDUMP | DISFLAG_USELABELADDRS;
	dZ80_SetDefaultOptions(d);
	
	showVersion		= FALSE;
	d->start		= 0;
	d->end	 		= 65535;
	disRadix		= DRADIX_DEFAULT;

/* Allocate the opcode map so that the config.file may modify it */
	dZ80_AllocateOpMap(d);

/* Have we passed the special case switch? */
	startArgc = 1;
	if (argc > 1)
		{
		if (!strcmp(argv[1], OMITCONFIGSWITCH))
			startArgc++;
		}

/* Load the config file dZ80.ini */
	if (startArgc == 1)
		{
		err = dZ80_LoadConfiguration(d, NULL);
		if (err != DERR_NONE && err != DERR_SCRIPTING_NA)
			{
			sprintf(buf, "Error loading %s: %s", CONFIGFILENAME, dZ80_GetErrorText(err));
			PrintToConsole(buf);
			exit(1);
			}
		}

/* Parse the command line, overriding any config file settings */
	if (ParseCmdLine(d, startArgc, argc, argv))
		exit(1);

	if (showVersion)
		{
		ShowVersionInfo();
		exit(0);
		}

	if (d->srcFileName[0] == 0)
		{
		ShowUsage();
		exit(0);
		}

/*
	Note if we've specified a radix, we set it and then separately copy the prefix and suffixes 
	across, as they will have been clobbered by dZ80_SetRadix 
*/	
	if (d->layoutRadix != disRadix)
		dZ80_SetRadix(d, disRadix);

	if (d->parametersModified & DPM_NUMPREFIX)
		strcpy(d->layoutNumberPrefix, reqLayoutNumberPrefix);

	if (d->parametersModified & DPM_NUMSUFFIX)
		strcpy(d->layoutNumberSuffix, reqLayoutNumberSuffix);

	ScanFilenameForPresets(d);
	ParseFilenames(d);

	err = dZ80_LoadZ80File(d, &bytesLoaded);
	if (err)
		exit(err);

	if (bytesLoaded == 0)
		{
		dZ80_Error(d, "Cannot load a zero byte file.\n");
		exit(1);
		}

/* Make sure we've got sensible start and end addresses */
	if (d->start < d->fileStartAddr)
		{
		if (!(d->parametersModified & DPM_STARTADDR))
			d->start = d->fileStartAddr;
		}

	if (d->end > (d->fileStartAddr + bytesLoaded))
		{
		if (!(d->parametersModified & DPM_ENDADDR))
			d->end = d->fileStartAddr + (WORD)(bytesLoaded - 1);
		}
	
/* ...and do the biz */
	dZ80_Disassemble(d);

	free(d->mem0Start);
	free(d);

	exit(0);
}


void ParseFilenames(DISZ80 *d)
{
	char	*extPtr;

/* If specified a reference filename, don't interfere */
	if (d->refFileName[0])
		return;

/* Are we going to need a reference filename ? */
	if (d->flags & DISFLAG_ANYREF)
		{
		strcpy(d->refFileName, d->srcFileName);

/* Hardly, foolproof, but this will replace any extension with .REF */
		extPtr = (char *)strrchr(d->refFileName, '.');

/* If there is a period, make sure it isn't a terminating one */
		if (extPtr != NULL)
			{
			if (*extPtr+1)
				{
				strcpy(extPtr+1, "ref");
				return;
				}
			}
		else
			{
			strcat(d->refFileName, ".ref");
			}
		}

	return;
}


void ScanFilenameForPresets(DISZ80 *d)
{
	char	*extPtr;
	char	fileExt[16], buf[128];
	int 	i;

	extPtr = strrchr(d->srcFileName, '.');
	if (extPtr == NULL)
		return; 						/* No extension, so no presets. */

	strcpy(fileExt, (extPtr)+1);
	dZ80_StringToLower(fileExt);		/* Make lower case */

	for (i=0; ; i++)
		{
		if (FileExtList[i].Extension == 0)
			return;

		if (!strcmp(fileExt, FileExtList[i].Extension) )
			{
			if (!(d->flags & DISFLAG_QUIET))
				{
				sprintf(buf, "Using presets for .%s file extension.\n", fileExt);
				PrintToConsole(buf);
				}

			d->fileHeaderSize = FileExtList[i].FileHeaderSize;
			d->fileStartAddr = FileExtList[i].BaseOffset;
			return;
			}
		}
}


void PrintToErrOut(char *Str)
{
	fwrite(Str, 1, strlen(Str), stderr);
	fwrite("\n", 1, 1, stderr);
	return;
}

void PrintToConsole(char *Str)
{
	printf("%s\n", Str);
	return;
}


/*
	This function is frequently called by the disassembler module to allow its progress
	to be displayed. It's used by the Windows version of dZ80 to implement a progress bar.
*/

static void ProgressUpdate(DISZ80 *d)
{
	return;
}


void ShowVersionInfo(void)
{
	char	buf[256];

	sprintf(buf, "dZ80 core %s, %s", dZ80_GetVersionString(), LUA_VERSION);
	PrintToConsole(buf);
	return;
}


void ShowUsage(void)
{
	char	buf[256];

	sprintf(buf, 
	   "Disassembles a Z80 binary file.                        E-mail: %s\n", DZ80_EMAIL);
	PrintToConsole(buf);
	PrintToConsole("DZ80 [!] [switches] infile [outfile]\n"
	   "\n"
	   "  !         Prevents dZ80 from automatically loading the configuration file.\n"
	   "  -h=nn     Skips past the first nn bytes of the input file.\n"
	   "  -m=nn     Specifies whereabouts in Z80 memory the file starts.\n"
	   "  -s=nn     Specifies the address to start disassembling from.\n"
	   "  -e=nn     Specifies the ending disassembly address.\n"
	   "  -r=file   Specify the name for the reference file. Used with -xa -xn -xo -xi.\n"
	   "  -xi       Create reference of input ports.\n"
	   "  -xo       Create reference of output ports.\n"
	   "  -xa       Create reference of addresses.\n"
	   "  -xn       Create reference of indirect addresses.\n"
	   "  -k=script Use the specified script file.\n"
	   "  -l	    Create a labelled (assembleable) output file.\n"
	   "  -z=cpu    Set the CPU type to Z80GB, Z80 (default) or Z180.\n"
	   );

	sprintf(buf,
	   "Note that this is only a partial list of switches. You can use the %s\n"
	   "configuration file to control dz80. Please read the dz80.txt file for details.", CONFIGFILENAME);
	PrintToConsole(buf);

	return;
}

