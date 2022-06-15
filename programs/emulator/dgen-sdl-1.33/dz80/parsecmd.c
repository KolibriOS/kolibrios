/*
	dZ80 Parse Command Line
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "types.h"
#include "dissz80.h"
#include "dz80.h"

static int GetDWord(char *Str, DWORD *pDW);
static int GetWord(char *Str, WORD *pWord);
static int CopyStringParam(char *dst, char *src, int maxlen);

int ParseCmdLine(DISZ80 *d, int startArgc, int argc, char* argv[])
{
	int 	i, numFileNames;
	WORD	r;
	char	j, cpu[16];

	d->flagsModified = 0;
	d->parametersModified = 0;
	numFileNames = 0;

	for (i = startArgc; i < argc; i++)
		{
		if (argv[i][0] == '/' || argv[i][0] == '-')
			{
			switch (tolower(argv[i][1]))
				{
				case 'l':
					d->flags |= DISFLAG_LABELLED;
					continue;

				case 'h':
					if (GetDWord(&argv[i][2], &d->fileHeaderSize))
						return TRUE;
					continue;

				
				case 'm':
					if (GetWord(&argv[i][2], &d->fileStartAddr))
						return TRUE;
					continue;

					
				case 's':
					if (GetWord(&argv[i][2], &d->start))
						return TRUE;
					d->parametersModified |= DPM_STARTADDR;
					continue;
				

				case 'e':
					if (GetWord(&argv[i][2], &d->end))
						return TRUE;
					d->parametersModified |= DPM_ENDADDR;
					continue;
				

				case 'o':
					if (CopyStringParam(d->opMapFileName, &argv[i][2], sizeof(d->opMapFileName)) )
						return TRUE;
					continue;
				

				case 'r':
					if (CopyStringParam(d->refFileName, &argv[i][2], sizeof(d->refFileName)) )
						return TRUE;
					continue;
	

				case 'a':
					d->flags &= ~DISFLAG_ADDRDUMP;
					continue;

				
				case 'n':
					d->flags &= ~DISFLAG_OPCODEDUMP;
					continue;

				
				case 'u':
					d->flags |= DISFLAG_UPPER;
					continue;

				
				case 'q':
					d->flags |= DISFLAG_QUIET;
					continue;

				
				case 'z':
					CopyStringParam(cpu, &argv[i][2], sizeof(cpu));


					for(j=0; j < DCPU_TOTAL; j++)
						{
						if (!(stricmp(cpu, dZ80CpuTypeNames[j])))
							{
							d->cpuType = j;
							break;
							}
						}

					if (j >= DCPU_TOTAL)
						{
						printf("Unknown CPU type \"%s\". Must be one of the following:\n\n", cpu);
						for(j=0; j < DCPU_TOTAL; j++)
							printf("%s\n", dZ80CpuTypeNames[j]);
						return TRUE;
						}

					continue;

				
				case 'b':
					d->flags &= ~DISFLAG_USELABELADDRS;
					continue;

				
				case 'w':
					d->flags &= ~DISFLAG_LINECOMMANDS;
					continue;

				case 'x':
					switch (tolower(argv[i][2]))
						{
						
						case 'o':
							d->flags |= DISFLAG_REFOUTPORT;
							break;

						
						case 'i':
							d->flags |= DISFLAG_REFINPORT;
							break;

						
						case 'a':
							d->flags |= DISFLAG_REFADDR;
							break;

						
						case 'n':
							d->flags |= DISFLAG_REFINDIRECT;
							break;

						
						case 'r':
							d->flags |= DISFLAG_REFLIMITRANGE;
							break;

						
						default:
							printf("Unknown reference type \"%c\". Valid types are o, i, a, n and r.\n",  argv[i][2]);
							return TRUE;
						}
					continue;

				
				case '?':
					ShowUsage();
					return TRUE;

/* Added for 1.50 */
				case 'd':		
					if (CopyStringParam(d->layoutDefineByte, &argv[i][2], sizeof(d->layoutDefineByte)))
						return TRUE;
					continue;

				
				case 'f':
					if (CopyStringParam(d->layoutComment, &argv[i][2], sizeof(d->layoutComment)))
						return TRUE;
					continue;

				
				case 'p':
					if (CopyStringParam(reqLayoutNumberPrefix, &argv[i][2], sizeof(reqLayoutNumberPrefix)))
						return TRUE;
					d->parametersModified |= DPM_NUMPREFIX;
					continue;

				
				case 'y':
					if (CopyStringParam(reqLayoutNumberSuffix, &argv[i][2], sizeof(reqLayoutNumberSuffix)))
						return TRUE;
					d->parametersModified |= DPM_NUMSUFFIX;
					continue;

				
				case 'i':
					if (GetWord(&argv[i][2], &r))
						return TRUE;
				
					switch(r)
						{
						case 8:
							disRadix = DRADIX_OCTAL;
							break;

						case 10:
							disRadix = DRADIX_DECIMAL;
							break;

						case 16:
							disRadix = DRADIX_HEX;
							break;

						default:
							printf("Radix must be 8, 10 or 16\n"
								   "  Use -? for help.\n");
							return TRUE;
						}
					continue;

/* 2.0 */
				case 'j':
					d->flags |= DISFLAG_RELCOMMENT;
					continue;

				case 'k':
					if (CopyStringParam(d->scriptFileName, &argv[i][2],  sizeof(d->scriptFileName)) )
						return TRUE;
					continue;

				case 'v':
					showVersion = TRUE;
					continue;

				default:
					printf("Don't know what to do with \"%s\".\n\n", argv[i]);
					printf("  Use -? for help.\n");
					return TRUE;
				}
			}


		if (numFileNames >= 1)
			{
			if (numFileNames >= 2)
				{
				printf("Already have two file names.\n");
				return(1);
				}
			else
				{
				strcpy(d->outFileName, argv[i]);
				}
			}
		else
			{
			strcpy(d->srcFileName, argv[i]);
			}

		numFileNames++;

		}	/* for (i = 1; i < argc; i++) */

	return FALSE;
}


static int GetDWord(char *Str, DWORD *pDW)
{
	char *pEnd;

	if (Str[0] == '=')
		Str++;

	*pDW = (DWORD)strtol(Str, &pEnd, 0);

	if (pEnd == Str)
		{
		printf("Invalid number \"%s\".\n"
			   "  Use -? for help.\n", Str);
		return TRUE;
		}

	return FALSE;
}

static int GetWord(char *Str, WORD *pWord)
{
	char *pEnd;

	if (Str[0] == '=')
		Str++;

	*pWord = (WORD)strtol(Str, &pEnd, 0);

	if (pEnd == Str)
		{
		printf("Invalid number \"%s\".\n"
			   "  Use -? for help.\n", Str);
		return TRUE;
		}

	return FALSE;
}


static int CopyStringParam(char *dst, char *src, int maxlen)
{
	if (src[0] == '=')
		src++;

	if (strlen(src) == 0)
		{
		printf("Missing string\n"
			"  Use -? for help.\n");
		return TRUE;
		}

	dZ80_SafeStringCopy(dst, src, maxlen);
	return FALSE;
}



