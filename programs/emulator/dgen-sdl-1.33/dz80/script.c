/*
	dZ80 scripting support for opcode traps
*/

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "dissz80p.h"
#include "lualib.h"
#include "lauxlib.h"

const static char *hexChars = "0123456789ABCDEF";

/* Lua dZ80 extensions */ 
static int d_RegTrap(lua_State *ls);
static int d_AddComment(lua_State *ls);
static int d_AddToDis(lua_State *ls);
static int d_AddToDisTab(lua_State *ls);
static int d_GetByte(lua_State *ls);
static int d_LookByte(lua_State *ls);
static int d_DB(lua_State *ls);
static int d_FlushLine(lua_State *ls);
static int d_IsCodeByte(lua_State *ls);
static int d_GetPC(lua_State *ls);
static int d_LookByteAddr(lua_State *ls);
static int d_GetPass(lua_State *ls);
static int d_Message(lua_State *ls);


int InitOpcodeTraps(DISZ80 *d)
{
	int		err;
	char	buf[_MAX_PATH + 64];

	if (d->scriptFileName[0])
		{
		if (d->pTrapMap == NULL)
			{
			if ((d->pTrapMap = AllocateMap(d, "Couldn't allocate memory for the opcode trap map.", 256)) == NULL)
				{
				DisZ80CleanUp(d);
				return DERR_OUTOFMEM;
				}
			}
		
		err = InitScripting(d);
		if (err != DERR_NONE)
			return err;

		err = lua_dofile(d->ls, d->scriptFileName);
		if (err)
			{
			switch(err)
				{
				case 2:
					sprintf(buf, "Couldn't open the script file \"%s\"", d->scriptFileName);
					dZ80_Error(d, buf);
					return DERR_COULDNTOPENFILE;

				default:
					return DERR_SCRIPTERROR;
				}
			}
		}

	return DERR_NONE;
}


int LuaErrorHandler(lua_State *ls)
{
	char		buf[256];
	DISZ80		*d;
	const char	*s;
	
	d = GetD(ls);
	s = lua_tostring(ls, 1);
	if (s == NULL)
		s = "(no message)";
	sprintf(buf, "Script error: %s\n", s);

	dZ80_Error(d, buf);
	return 0;
}

int ShutdownScripting(DISZ80 *d)
{
	if (d->ls != NULL)
		{
		lua_close(d->ls);
		d->ls = NULL;
		}

	return DERR_NONE;
}

int InitScripting(DISZ80 *d)
{
	char	buf[256];
	int		err;

	if (d->ls == NULL)
		{
		d->ls = lua_open(0);

		if (d->ls == NULL)
			return DERR_OUTOFMEM;
		
		lua_baselibopen(d->ls);
		lua_strlibopen(d->ls);

		lua_pushuserdata(d->ls, d);
		lua_setglobal(d->ls, "DISZ80STRUC");

		lua_register(d->ls, LUA_ERRORMESSAGE, LuaErrorHandler);

/* Register the dZ80 support functions */
		lua_register(d->ls, "d_RegTrap",		d_RegTrap);
		lua_register(d->ls, "d_AddComment", 	d_AddComment);
		lua_register(d->ls, "d_AddToDis",		d_AddToDis);
		lua_register(d->ls, "d_AddToDisTab",	d_AddToDisTab);
		lua_register(d->ls, "d_GetByte",		d_GetByte);
		lua_register(d->ls, "d_LookByte",		d_LookByte);
		lua_register(d->ls, "d_DB", 			d_DB);
		lua_register(d->ls, "d_FlushLine",		d_FlushLine);
		lua_register(d->ls, "d_IsCodeByte",		d_IsCodeByte);
		lua_register(d->ls, "d_GetPC",			d_GetPC);
		lua_register(d->ls, "d_LookByteAddr",	d_LookByteAddr);
		lua_register(d->ls, "d_GetPass",		d_GetPass);
		lua_register(d->ls, "d_Message",		d_Message);

/* Register the general support functions */
		lua_register(d->ls, "hex",				d_FromHex);

/* Set up the trap registration functions */
		sprintf(buf,
			"__dz80_pre={}\n"
			"__dz80_post={}\n"
			"function d_PreTrap(op, fn) __dz80_pre[op]=fn d_RegTrap(op, %d) end\n"
			"function d_PostTrap(op, fn) __dz80_post[op]=fn d_RegTrap(op, %d) end\n",
			D_SCRIPT_PRE, D_SCRIPT_POST);

		err = lua_dostring(d->ls, buf);
		if (err)
			return DERR_SCRIPTERROR;
		}

	return DERR_NONE;
}


/*
	ExecPreTrap

	Execute the pre-decode trap
*/

int ExecPreTrap(DISZ80 *d)
{
	int 		ni;
	lua_State	*ls;

	if (d->pTrapMap != NULL)
		{
		if (d->pTrapMap[d->firstByte] & D_SCRIPT_PRE)
			{
			ni = d->numInstructions;

			ls = d->ls;
			assert(ls != NULL);

			lua_getglobal(ls, "__dz80_pre");
			lua_pushnumber(ls, d->firstByte);
			lua_gettable(ls, -2);
			lua_call(ls, 0, 0);			/* call the opcode trap function */
			lua_pop(ls, 1);				/* remove the table name */

			return (ni != d->numInstructions);
			}
		}

	return 0;
}



/*
	ExecPostTrap

	Execute the post-decode trap
*/

void ExecPostTrap(DISZ80 *d)
{
	lua_State	*ls;

	if (d->pTrapMap != NULL)
		{
		if (d->pTrapMap[d->firstByte] & D_SCRIPT_POST)
			{
			ls = d->ls;
			assert(ls != NULL);

			lua_getglobal(ls, "__dz80_post");
			lua_pushnumber(ls, d->firstByte);
			lua_gettable(ls, -2);
			lua_call(ls, 0, 0);			/* call the opcode trap function */
			lua_pop(ls, 1);				/* remove the table name */
			}
		}

	return;
}


/*
	Lua's dZ80 Interface
*/

DISZ80 *GetD(lua_State *ls)
	{
	DISZ80 *d;

	lua_getglobal(ls, "DISZ80STRUC");

	d = (DISZ80 *)lua_touserdata(ls, -1);
	assert(d != NULL);
	
	lua_pop(ls, 1);
	return d;
	}


/*
	d_RegTrap(opcode, type)
*/

static int d_RegTrap(lua_State *ls)
	{
	DISZ80	*d;
	int 	op, type;

	d = GetD(ls);
	
	op = luaL_check_long(ls, 1);
	assert(op >= 0 && op < 256);

	type = luaL_check_long(ls, 2);

	d->pTrapMap[op] |= (unsigned char)type;
	return 0;
	}


static int d_AddComment(lua_State *ls)
	{
	DISZ80		*d;
	const char	*str;
	size_t		len;

	d = GetD(ls);
	str = luaL_check_lstr(ls, 1, &len);

	AddToDisComment(d, (char *)str);
	return 0;
	}


static int d_AddToDis(lua_State *ls)
	{
	DISZ80		*d;
	const char	*str;
	size_t		len;

	d = GetD(ls);
	str = luaL_check_lstr(ls, 1, &len);

	AddToDis(d, (char *)str);
	return 0;
	}


static int d_AddToDisTab(lua_State *ls)
	{
	DISZ80		*d;
	const char	*str;
	size_t		len;

	d = GetD(ls);
	str = luaL_check_lstr(ls, 1, &len);

	AddToDisTab(d, (char *)str);
	return 0;
	}


static int d_GetByte(lua_State *ls)
	{
	DISZ80		*d;
	BYTE		b;
	
	d = GetD(ls);

	if (!WithinDisRange(d))
		lua_error(ls, "d_GetByte requesting a byte past the user end address");

	b = GetNextOpCode(d);
	lua_pushnumber(ls, (double)b);
	return 1;
	}


static int d_LookByte(lua_State *ls)
	{
	DISZ80		*d;
	BYTE		b;
	int 		offset;

	d = GetD(ls);
	offset = luaL_check_long(ls, 1);
		
	b = LookOpcode(d, offset);
	lua_pushnumber(ls, (double)b);
	return 1;
	}


static int d_DB(lua_State *ls)
	{
	DISZ80		*d;
	int 		b, n, i;

	d = GetD(ls);

	n = 0;
	while (lua_isnumber(ls, n+1))
		n++;

	if (n)
		{
		AddToDisTabDB(d);
			
		for(i=1; i <= n; i++)
			{
			b = luaL_check_long(ls, i) & 0xff;
			if (i > 1)
				AddToDis(d, ",");
			Add8BitNum(d, b);
			}
		}

	return 0;
	}


static int d_FlushLine(lua_State *ls)
{
	DISZ80		*d;
	int 		err;

	d = GetD(ls);
	err = WriteDisLine(d, d->lastPC);
	lua_pushnumber(ls, (double)err);
	return 1; 
}


static int d_IsCodeByte(lua_State *ls)
	{
	DISZ80		*d;
	int 		addr, isCodeByte;

	isCodeByte = 1;

	d = GetD(ls);
	addr = luaL_check_long(ls, 1);
	
	if (d->opMap != NULL)
		{
		if (!ISCODEBYTE(d, addr))
			isCodeByte = 0;
		}
	
	lua_pushnumber(ls, (double)isCodeByte);
	return 1;
	}


static int d_GetPC(lua_State *ls)
	{
	DISZ80		*d;

	d = GetD(ls);
	lua_pushnumber(ls, (double)d->PC);
	return 1;
	}


static int d_LookByteAddr(lua_State *ls)
	{
	DISZ80		*d;
	int 		addr;

	d = GetD(ls);
	addr = luaL_check_long(ls, 1);
	lua_pushnumber(ls, (double)d->Z80MemBase[addr]);
	return 1;
	}


static int d_GetPass(lua_State *ls)
	{
	DISZ80		*d;
	
	d = GetD(ls);
	lua_pushnumber(ls, (double)d->currentPass);
	return 1;
	}


static int d_Message(lua_State *ls)
	{
	DISZ80		*d;
	const char	*ps;

	ps = luaL_check_string(ls, 1);
	d = GetD(ls);
	dZ80_ShowMsg(d, (char *)ps);
	return 0;
	}


int d_FromHex(lua_State *ls)
	{
	DWORD		h;
	const char	*ps, *p;
	char		c;

	h = 0;
	c = 0;

	ps = luaL_check_string(ls, 1);
	
	while (c = *ps++)
		{
		p = strchr(hexChars, toupper(c));
		if (p == NULL)
			break;

		h = (p - hexChars) + 16 * h;
		}

/* If c is not zero, we've exited the conversion loop prematurely so there must have been an error */
	if (c)
		luaL_argerror(ls, 1, "bad hex number");
		
	lua_pushnumber(ls, (double)h);
	return 1;
	}



