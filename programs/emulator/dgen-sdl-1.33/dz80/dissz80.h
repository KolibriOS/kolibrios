/* dZ80 Disassembler Header */

#ifndef _MIDISSZ80_
#define _MIDISSZ80_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _DZ80_EXCLUDE_SCRIPT
#include "lua.h"
#else
#define	LUA_VERSION		"(scripting not included)"
#endif

#define	DZ80_WWW		"http://www.inkland.org"
#define	DZ80_EMAIL		"dz80@inkland.org"

#define Z80MEMSIZE				65536

#define	D_CUSTOMSTRING_MAXLEN	6

#define DISFLAG_OPCODEDUMP		0x01
#define DISFLAG_ADDRDUMP		0x02
#define DISFLAG_UPPER			0x04
#define DISFLAG_SINGLE	 		0x08	/* Disassemble a single instruction */
#define DISFLAG_VERBOSE 		0x10
#define DISFLAG_QUIET			0x20
#define DISFLAG_LABELLED		0x40
#define DISFLAG_REFINPORT		0x80
#define DISFLAG_REFOUTPORT		0x100
#define DISFLAG_REFADDR 		0x200
#define DISFLAG_REFINDIRECT 	0x400
#define DISFLAG_REFLIMITRANGE	0x800
#define DISFLAG_USELABELADDRS	0x1000
#define DISFLAG_RELCOMMENT		0x2000	/* Relative jump comments */
#define DISFLAG_LINECOMMANDS	0x4000	/* Process line commands (auto blank lines) */
#define DISFLAG_CALLBACK	0x8000  /* Access memory through a callback */

/* Reference listing stuff */
#define DISFLAG_ANYREF			(DISFLAG_REFINPORT | DISFLAG_REFOUTPORT | DISFLAG_REFADDR | DISFLAG_REFINDIRECT)

/* parametersModified flags (used by the GUI when importing a config file) */
#define	DPM_STARTADDR			0x0001
#define	DPM_ENDADDR				0x0002
#define	DPM_HDRSIZE				0x0004
#define	DPM_FILESTARTADDR		0x0008
#define	DPM_CPUTYPE				0x0010
#define	DPM_RADIX				0x0020
#define	DPM_NUMPREFIX			0x0040
#define	DPM_NUMSUFFIX			0x0080
#define	DPM_OPMAP				0x0100

/* Configuration file name */
#define	CONFIGFILENAME			"dz80.ini"

enum dz80Cpus
	{
	DCPU_Z80GB,
	DCPU_Z80,
	DCPU_Z180,

	DCPU_TOTAL
	};


enum dz80Passes
	{
	DPASS_INIT,
	DPASS_WRITE,
	DPASS_ANALYSE,

	DPASS_TOTAL
	};



enum dz80Errors
	{
	DERR_NONE,
	DERR_OUTOFMEM,
	DERR_COULDNTCREATEFILE,
	DERR_COULDNTWRITETOFILE,
	DERR_BADOPCODELENGTH,
	DERR_INVALIDPARAMS,
	DERR_SCRIPTERROR,
	DERR_WRONGARGUMENTTYPE,
	DERR_COULDNTOPENFILE,
	DERR_SCRIPTING_NA,
	
	DERR_TOTAL
	};


enum DRADIX
{
	DRADIX_HEX,
	DRADIX_DECIMAL,
	DRADIX_OCTAL,		

	DRADIX_TOTAL,

	DRADIX_DEFAULT = DRADIX_HEX
};


enum DISREF_TYPES
{
	DISREF_INPORT,
	DISREF_OUTPORT,
	DISREF_ADDR,
	DISREF_INDIRECT,

	DISREF_TOTAL
};


typedef struct DISREFADDR
{
	WORD				RefAddress;
	struct DISREFADDR	*pNext;
} DISREFADDR;

typedef struct DISREF
{
	WORD		RefType;				/* Reference type */
	WORD		Addr;					/* Memory or port address */
	WORD		Hits;					/* Number of times referenced */
	DISREFADDR	*pRefAddrHead;			/* Pointer to the list of referenced addresses */
	DISREFADDR	*pRefAddrTail;			/* Pointer to the last referenced address */

	struct DISREF *pPrev;
	struct DISREF *pNext;
} DISREF;


typedef struct DISZ80
{
	BYTE		*mem0Start; 				/* Pointer to Z80's zero address */
	BYTE		(*memCB)(void *ctx, WORD addr);		/* Indirect access through callback (DISFLAG_CALLBACK) */
	WORD		start;						/* Starting disassembler address */
	WORD		end;						/* Ending disassembler address */
	DWORD		flags; 						/* See DISFLAG_ defines */
	DWORD		flagsModified;				/* Which bits have been modified via config files */
	DWORD		parametersModified;			/* Which other parameters have been modified via config file */
	int 		numInstructions;			/* How many instructions were disassembled */
	int 		createdRefOK;				/* Set to TRUE if created .ref file OK */
	char		srcFileName[_MAX_PATH];		/* Used only to display the source filename in the report header */
	char		outFileName[_MAX_PATH];		/* Destination assembler file */
	char		opMapFileName[_MAX_PATH];	/* Opcode map file name */
	char		refFileName[_MAX_PATH];		/* Reference file name */
	char		scriptFileName[_MAX_PATH];	/* Lua script file name */

/* CPU type */
	BYTE		cpuType;					/* See the DCPU_ enums */
	
/* New flexible number formats */
	BYTE		layoutRadix;							/* Do ya want hex with that? */
	char		layoutComment[D_CUSTOMSTRING_MAXLEN];	/* The string used as a comment marker */
	char		layoutNumberPrefix[D_CUSTOMSTRING_MAXLEN];	
	char		layoutNumberSuffix[D_CUSTOMSTRING_MAXLEN]; 
	char		layoutDefineByte[D_CUSTOMSTRING_MAXLEN];/* Default of "db" */

/* Pointer to the function callbacks */
	void		(*fnProgressCallback)(struct DISZ80 *d);
	void		(*fnErrorMessage)(char *msg);
	void		(*fnOutputMessage)(char *msg);

/* dZ80 private workspace stuff - hands off ;) */
	char		hexDisBuf[32];				/* Holds the hex. code bytes (easy one!) */
	char		disBuf[128];				/* Holds the disassembly */
	char		commentBuf[128];
	WORD		lastDisPC, lastRefAddr, disStart, disEnd;
	int 		op, realop, lineCmd;
	BYTE		*Z80MemBase;
	BYTE		(*Z80MemCB)(void *ctx, WORD addr);
	signed char	IXIYDisp;				/* The IXIY displacement (-128 to 127) */
	int 		Z80Flags;
	int 		currentPass, totalPasses;
	int 		numRecursions;
	BYTE		*fnMap, *opMap;
	DISREF		*pRefHead[DISREF_TOTAL];
	DWORD		numRefs[DISREF_TOTAL];
	DWORD		bytesToProcess, bytesProcessed;
	FILE		*outStream, *refStream;
	int 		labelledOutput, dissingToConsole, haveTabbed;
	int 		progressCounter;
	WORD		PC, lastPC;
	BYTE		*pTrapMap;				/* Pointer to the trap map */
	BYTE		firstByte;
	int 		haveWrapped;
	int			disBufIndex;			/* Index into the disassembly buffer */
	DWORD		fileHeaderSize;			/* The size of the file's header (to skip) */
	WORD		fileStartAddr;			/* The Z80 address of the start of the file */

#ifndef _DZ80_EXCLUDE_SCRIPT
	lua_State	*ls;					/* Pointer to the Lua state in which the scripts are run */
#endif
	
} DISZ80;


int		dZ80_LoadConfiguration(DISZ80 *d, char *pConfigFile);
void	dZ80_SetDefaultOptions(DISZ80 *d);
int 	dZ80_Disassemble(DISZ80 *d);
void	dZ80_SetRadix(DISZ80 *d, int radix);
int 	dZ80_SingleDisassembly(DISZ80 *d);
void	dZ80_InheritRadix(DISZ80 *dst, DISZ80 *src);
char	*dZ80_GetVersionString(void);
void	dZ80_StringToLower(char *s);
void	dZ80_StringToUpper(char *s);
void	dZ80_SafeStringCopy(char *dst, char *src, int dstSize);
const char *dZ80_GetErrorText(int errNum);
int 	dZ80_LoadZ80File(DISZ80 *d, DWORD *BytesLoaded);
void	dZ80_Error(DISZ80 *d, char *msg);
void	dZ80_ShowMsg(DISZ80 *d, char *msg);
int		dZ80_AllocateOpMap(DISZ80 *d);

extern char *dZ80CpuTypeNames[DCPU_TOTAL];

#ifdef __cplusplus
}
#endif

#endif		/* _MIDISSZ80_ */
