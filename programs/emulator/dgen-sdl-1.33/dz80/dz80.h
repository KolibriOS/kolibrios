/* Header file for dZ80 */

#ifndef __MIDZ80__
#define __MIDZ80__

#define	OMITCONFIGSWITCH	"!"

typedef struct
{
	char *Extension;
	WORD FileHeaderSize;
	WORD BaseOffset;
} DISFILE;

extern	char	reqLayoutNumberPrefix[D_CUSTOMSTRING_MAXLEN], reqLayoutNumberSuffix[D_CUSTOMSTRING_MAXLEN];
extern	int 	disRadix, showVersion;

void	ShowVersionInfo(void);
void	ShowUsage(void);
void	ScanFilenameForPresets(DISZ80 *d);
void	ParseFilenames(DISZ80 *d);
void	PrintToErrOut(char *Str);
void	PrintToConsole(char *Str);
int		ParseCmdLine(DISZ80 *d, int startArgc, int argc, char* argv[]);

#endif	/* __MIDZ80__ */
