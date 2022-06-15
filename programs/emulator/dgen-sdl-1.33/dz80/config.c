/*
	dZ80 configuration support

	dZ80 will attempt to load and use a file called dz80.ini in the current folder
	Any settings in this file will override dZ80's defaults. Similarly, for the Windows version
	of dZ80, any settings in the config file will override the settings set in the GUI.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "dissz80p.h"
#include "lualib.h"
#include "lauxlib.h"

typedef struct CFGITEM_tag
	{
	char	*varName;
	BYTE	type;
	int		(*fnHandler)(void *pv, DISZ80 *d);
	} CFGITEM;


/* Local fn's */
static int SetArea(lua_State *ls, BYTE type);
static int ProcessConfig(lua_State *ls, DISZ80 *d);
static int SetDisFlag(DISZ80 *d, void *pv, DWORD flag);

/* Configuration file Lua extensions */
static int d_SetCodeRegion(lua_State *ls);
static int d_SetDataRegion(lua_State *ls);
static int d_Exist(lua_State *ls);

/* Config handlers */
static int cfg_cpu(void *pv, DISZ80 *d);
static int cfg_inputfile(void *pv, DISZ80 *d);
static int cfg_outputfile(void *pv, DISZ80 *d);
static int cfg_fileheader(void *pv, DISZ80 *d);
static int cfg_filebase(void *pv, DISZ80 *d);
static int cfg_start(void *pv, DISZ80 *d);
static int cfg_end(void *pv, DISZ80 *d);
static int cfg_quiet(void *pv, DISZ80 *d);
static int cfg_labelledoutput(void *pv, DISZ80 *d);
static int cfg_labelreference(void *pv, DISZ80 *d);
static int cfg_addresscomumn(void *pv, DISZ80 *d);
static int cfg_opcodecolumn(void *pv, DISZ80 *d);
static int cfg_relativejumpcomment(void *pv, DISZ80 *d);
static int cfg_uppercase(void *pv, DISZ80 *d);
static int cfg_autoblanklines(void *pv, DISZ80 *d);
static int cfg_radix(void *pv, DISZ80 *d);
static int cfg_db(void *pv, DISZ80 *d);
static int cfg_comment(void *pv, DISZ80 *d);
static int cfg_numprefix(void *pv, DISZ80 *d);
static int cfg_numsuffix(void *pv, DISZ80 *d);
static int cfg_script(void *pv, DISZ80 *d);
static int cfg_referencefile(void *pv, DISZ80 *d); 
static int cfg_inportreference(void *pv, DISZ80 *d);
static int cfg_outportreference(void *pv, DISZ80 *d);
static int cfg_addressreference(void *pv, DISZ80 *d);
static int cfg_indirectaddressreference(void *pv, DISZ80 *d);
static int cfg_limitreferences(void *pv, DISZ80 *d);
static int cfg_opmapfile(void *pv, DISZ80 *d);

#define		CFG_BOOL			1
#define		CFG_STRING			2
#define		CFG_NUM16			3
#define		CFG_NUM32			4

CFGITEM cfgTable[] = 
{
	{"cpu",						CFG_STRING,	cfg_cpu},
	{"inputfile",				CFG_STRING,	cfg_inputfile},
	{"outputfile",				CFG_STRING,	cfg_outputfile},
	{"fileheadersize",			CFG_NUM32,	cfg_fileheader},
	{"filebaseaddr",			CFG_NUM32,	cfg_filebase},
	{"disstart",				CFG_NUM16,	cfg_start},
	{"disend",					CFG_NUM16,	cfg_end},
	{"quiet",					CFG_BOOL,	cfg_quiet},
	{"labelledoutput",			CFG_BOOL,	cfg_labelledoutput},
	{"labelreference",			CFG_BOOL,	cfg_labelreference},
	{"addresscolumn",			CFG_BOOL,	cfg_addresscomumn},
	{"opcodecolumn",			CFG_BOOL,	cfg_opcodecolumn},
	{"relativejumpcomment",		CFG_BOOL,	cfg_relativejumpcomment},
	{"uppercase",				CFG_BOOL,	cfg_uppercase},
	{"autoblanklines",			CFG_BOOL,	cfg_autoblanklines},
	{"radix",					CFG_NUM16,	cfg_radix},
	{"db",						CFG_STRING,	cfg_db},
	{"comment",					CFG_STRING,	cfg_comment},
	{"numprefix",				CFG_STRING, cfg_numprefix},
	{"numsuffix",				CFG_STRING, cfg_numsuffix},
	{"script",					CFG_STRING, cfg_script},
	{"referencefile",			CFG_STRING, cfg_referencefile}, 
	{"inportreference",			CFG_BOOL,	cfg_inportreference},
	{"outportreference",		CFG_BOOL,	cfg_outportreference},
	{"addressreference",		CFG_BOOL,	cfg_addressreference},
	{"indirectaddressreference",CFG_BOOL,	cfg_indirectaddressreference},
	{"limitreferences",			CFG_BOOL,	cfg_limitreferences},
	{"opmapfile",				CFG_STRING,	cfg_opmapfile},

	{NULL,						0,			NULL}
};


int dZ80_LoadConfiguration(DISZ80 *d, char *pConfigFile)
{
	int			err;
	lua_State	*ls;
	FILE		*f;
	char		buf[_MAX_PATH];
	
	if (pConfigFile == NULL)
		pConfigFile = CONFIGFILENAME;

/* Clear any flags or parameter modified flags */
	d->flagsModified = d->parametersModified = 0;

	
/* If there's a configuration file here, set up Lua and let it process the file */
	f = fopen(pConfigFile, "rt");
	if (f != NULL)
		{
		fclose(f);

		sprintf(buf, "Loading configuration file \"%s\"", pConfigFile);
		dZ80_ShowMsg(d, buf);

		ls = lua_open(0);
		if (ls == NULL)
			return DERR_OUTOFMEM;

		lua_baselibopen(ls);
		lua_strlibopen(ls);

		lua_pushuserdata(ls, d);
		lua_setglobal(ls, "DISZ80STRUC");

		lua_register(ls, LUA_ERRORMESSAGE, LuaErrorHandler);

/* Register the config functions */
		lua_register(ls, "d_SetCodeRegion",		d_SetCodeRegion);
		lua_register(ls, "d_SetDataRegion",		d_SetDataRegion);
		lua_register(ls, "d_Exist",				d_Exist);

/* Register the general support functions */
		lua_register(ls, "hex",					d_FromHex);

		err = lua_dofile(ls, pConfigFile);

		if (!err)
			err = ProcessConfig(ls, d);

		lua_close(ls);

		if (err)
			return DERR_SCRIPTERROR;
		}

	return DERR_NONE;
}


static int ProcessConfig(lua_State *ls, DISZ80 *d)
{
	CFGITEM	*c;
	int		err, fnErr;
	double	n;
	char	buf[256];
	union  {
		char	*s;
		BYTE	b;
		WORD	w;
		DWORD	dw;
		} args;
	
	err = DERR_NONE;
	c = cfgTable;
		
	while (c->varName != NULL && err == DERR_NONE)
		{
		lua_getglobal(ls, c->varName);
		if (!lua_isnil(ls, -1))
			{
			switch(c->type)
				{
				case CFG_BOOL:
					if (lua_isnumber(ls, -1))
						{
						n = lua_tonumber(ls, -1);
						args.b = (n != 0);
						}
					else
						err = DERR_WRONGARGUMENTTYPE;
					break;

				case CFG_STRING:
					if (lua_isstring(ls, -1))
						args.s = (char *)lua_tostring(ls, -1);
					else
						err = DERR_WRONGARGUMENTTYPE;
					break;

				case CFG_NUM16:
					if (lua_isnumber(ls, -1))
						{
						n = lua_tonumber(ls, -1);
						if (n >= 0 && n <= 65535)
							args.w = (WORD)(n);
						else
							err = DERR_WRONGARGUMENTTYPE;
						}
					else
						err = DERR_WRONGARGUMENTTYPE;
					break;

				case CFG_NUM32:
					if (lua_isnumber(ls, -1))
						{
						n = lua_tonumber(ls, -1);
						args.dw = (DWORD)(n);
						}
					else
						err = DERR_WRONGARGUMENTTYPE;
					break;
				}

			if (err == DERR_NONE)
				{
				fnErr = c->fnHandler(&args, d);
				if (fnErr)
					err = DERR_SCRIPTERROR;
				}

			if (err != DERR_NONE)
				{
				sprintf(buf, "Error retrieving \"%s\" value from configuration file.\n", c->varName);
				dZ80_Error(d, buf);
				}
			}	/* if (!lua_isnil(ls, -1)) */

		lua_pop(ls, 1);
		c++;
		}

	return err;
}

static int SetDisFlag(DISZ80 *d, void *pv, DWORD flag)
{
	assert(d != NULL);

	d->flagsModified |= flag;

	if (*(BYTE *)pv)
		d->flags |= flag;
	else
		d->flags &= ~flag;

	return 0;
}

static int d_Exist(lua_State *ls)
{
	char	*pFileName;
	double	n;
	FILE	*f;
	
	n = 0;

	if (lua_isstring(ls, 1))
		{
		pFileName = (char *)lua_tostring(ls, 1);
		f = fopen(pFileName, "rb");
		if (f != NULL)
			{
			fclose(f);
			n = 1;
			}
		}
		
	lua_pushnumber(ls, n);
	return 1;
}



static int d_SetCodeRegion(lua_State *ls)
{
	return SetArea(ls, 1);
}


static int d_SetDataRegion(lua_State *ls)
{
	return SetArea(ls, 0);
}

/*
	SetArea

	Set (or reset) the appropriate opMap bits
*/

static int SetArea(lua_State *ls, BYTE type)
{
	DISZ80		*d;
	DWORD		start, end;
	BYTE		mask;
	
	d = GetD(ls);
	
	if (d->opMap != NULL)
		{
		d->parametersModified |= DPM_OPMAP;

		start = luaL_check_long(ls, 1);
		end = start + luaL_check_long(ls, 2) - 1;
		
		if (type)
			type = 0xff;
		
		while(start <= end && start < Z80MEMSIZE)
			{
			mask = 1 << (start & 7);
			d->opMap[start/8] &= ~mask;
			d->opMap[start/8] |= (mask & type);
			start++;
			}
		}
		
	return 0;

}

/*
	Configuration handlers
*/

static int cfg_cpu(void *pv, DISZ80 *d)
{
	char	i;
	char	buf[16];

	dZ80_SafeStringCopy(buf, *(char **)pv, sizeof(buf));
	dZ80_StringToUpper(buf);

	for(i=0; i < DCPU_TOTAL; i++)
		{
		if (!strcmp(buf, dZ80CpuTypeNames[i]))
			break;
		}

	if (i < DCPU_TOTAL)
		{
		d->cpuType = i;
		}
	else
		{
		dZ80_Error(d, "Unknown CPU type");
		return 1;
		}
		
	d->parametersModified |= DPM_CPUTYPE;
	return 0;
}


static int cfg_inputfile(void *pv, DISZ80 *d)
{
	dZ80_SafeStringCopy(d->srcFileName, *(char **)pv, sizeof(d->srcFileName));
	return 0;
}

static int cfg_outputfile(void *pv, DISZ80 *d)
{
	dZ80_SafeStringCopy(d->outFileName, *(char **)pv, sizeof(d->outFileName));
	return 0;
}

static int cfg_fileheader(void *pv, DISZ80 *d)
{
	d->fileHeaderSize = *(DWORD *)pv;
	d->parametersModified |= DPM_HDRSIZE;
	return 0;
}

static int cfg_filebase(void *pv, DISZ80 *d)
{
	d->fileStartAddr = *(WORD *)pv;
	d->parametersModified |= DPM_FILESTARTADDR;
	return 0;
}

static int cfg_start(void *pv, DISZ80 *d)
{
	d->start = *(WORD *)pv;
	d->parametersModified |= DPM_STARTADDR;
	return 0;
}

static int cfg_end(void *pv, DISZ80 *d)
{
	d->end = *(WORD *)pv;
	d->parametersModified |= DPM_ENDADDR;
	return 0;
}


static int cfg_quiet(void *pv, DISZ80 *d)
{
	return SetDisFlag(d, pv, DISFLAG_QUIET);
}


static int cfg_labelledoutput(void *pv, DISZ80 *d)
{
	return SetDisFlag(d, pv, DISFLAG_LABELLED);	
}

static int cfg_labelreference(void *pv, DISZ80 *d)
{
	return SetDisFlag(d, pv, DISFLAG_USELABELADDRS);
}

static int cfg_addresscomumn(void *pv, DISZ80 *d)
{
	return SetDisFlag(d, pv, DISFLAG_ADDRDUMP);
}

static int cfg_opcodecolumn(void *pv, DISZ80 *d)
{
	return SetDisFlag(d, pv, DISFLAG_OPCODEDUMP);
}

static int cfg_relativejumpcomment(void *pv, DISZ80 *d)
{
	return SetDisFlag(d, pv, DISFLAG_RELCOMMENT);
}

static int cfg_uppercase(void *pv, DISZ80 *d)
{
	return SetDisFlag(d, pv, DISFLAG_UPPER);
}

static int cfg_autoblanklines(void *pv, DISZ80 *d)
{
	return SetDisFlag(d, pv, DISFLAG_LINECOMMANDS);
}

static int cfg_radix(void *pv, DISZ80 *d)
{
	BYTE	r;

	r = *(BYTE *)pv;
	switch(r)
		{
		case 8:
			r = DRADIX_OCTAL;
			break;

		case 10:
			r = DRADIX_DECIMAL;
			break;

		case 16:
			r = DRADIX_HEX;
			break;

		default:
			return 1;
		}

	d->layoutRadix = r;
	d->parametersModified |= DPM_RADIX;
	return 0;
}

static int cfg_db(void *pv, DISZ80 *d)
{
	dZ80_SafeStringCopy(d->layoutDefineByte, *(char **)pv, sizeof(d->layoutDefineByte));
	return 0;
}

static int cfg_comment(void *pv, DISZ80 *d)
{
	dZ80_SafeStringCopy(d->layoutComment, *(char **)pv, sizeof(d->layoutComment));
	return 0;
}

static int cfg_numprefix(void *pv, DISZ80 *d)
{
	dZ80_SafeStringCopy(d->layoutNumberPrefix, *(char **)pv, sizeof(d->layoutNumberPrefix));
	d->parametersModified |= DPM_NUMPREFIX;
	return	0;
}

static int cfg_numsuffix(void *pv, DISZ80 *d)
{
	dZ80_SafeStringCopy(d->layoutNumberSuffix, *(char **)pv, sizeof(d->layoutNumberSuffix));
	d->parametersModified |= DPM_NUMSUFFIX;
	return 0;
}

static int cfg_script(void *pv, DISZ80 *d)
{
	dZ80_SafeStringCopy(d->scriptFileName, *(char **)pv, sizeof(d->scriptFileName));
	return 0;	
}

static int cfg_referencefile(void *pv, DISZ80 *d) 
{
	dZ80_SafeStringCopy(d->refFileName, *(char **)pv, sizeof(d->refFileName));
	return 0;	
}

static int cfg_inportreference(void *pv, DISZ80 *d)
{
	return SetDisFlag(d, pv, DISFLAG_REFINPORT);
}

static int cfg_outportreference(void *pv, DISZ80 *d)
{
	return SetDisFlag(d, pv, DISFLAG_REFOUTPORT);
}

static int cfg_addressreference(void *pv, DISZ80 *d)
{
	return SetDisFlag(d, pv, DISFLAG_REFADDR);
}

static int cfg_indirectaddressreference(void *pv, DISZ80 *d)
{
	return SetDisFlag(d, pv, DISFLAG_REFINDIRECT);
}

static int cfg_limitreferences(void *pv, DISZ80 *d)
{
	return SetDisFlag(d, pv, DISFLAG_REFLIMITRANGE);
}

static int cfg_opmapfile(void *pv, DISZ80 *d)
{
	dZ80_SafeStringCopy(d->opMapFileName, *(char **)pv, sizeof(d->opMapFileName));
	return 0;
}

