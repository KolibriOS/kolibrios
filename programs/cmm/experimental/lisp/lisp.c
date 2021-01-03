/*
 * Author Pavel Iakovlev by PaulCodeman
*/

#define MEMSIZE 4096*10

#include "../lib/io.h"
#include "../lib/obj/console.h"

#define TString 1
#define TSymbol 2
#define TNumber 3
#define TList   4
#define Proc    5
#define Lambda  6
#define TObject 7

#define sizeStruct 4*4

byte initConsole = 0;
dword maxLoop = 100;
dword buffer = 0;
dword bufferSize = 1000;
dword variable = 0;
dword key = 0;
dword treeItem = 0;
dword NIL = 0;

void consoleInit()
{
	IF(!initConsole)
	{
		load_dll(libConsole, #con_init, 0);
		con_init stdcall (-1, -1, -1, -1, "Lisp interpreter");
		initConsole = 0xFF;
	}
}

dword error_message(dword text)
{
	con_printf stdcall (text);
	ExitProcess();
}

dword crc32Table = 0;
dword makeCRCTable(void)
{
	dword i = 0;
	dword c = 0;
	dword ii = 0;
	dword crcTable = 0;
	IF (crc32Table) RETURN 0;
	crc32Table = malloc(4*256);
	crcTable = crc32Table;
	WHILE (i < 256)
	{
		c = i;
		ii = 0;
		WHILE (ii < 8)
		{
			IF (c&1)
			{
				c >>= 1;
				c ^= 0xEDB88320;
			}
			ELSE c >>= 1;
			ii++;
		}
		DSDWORD[crcTable] = c;
		crcTable += 4;
		i++;
	}
}
dword crc32(dword bytes)
{
	dword crc = 0;
	byte b = 0;
	IF (!crc32Table) makeCRCTable();
	crc = 0xFFFFFFFF;
	WHILE (DSBYTE[bytes])
	{
		b = DSBYTE[bytes];
		bytes++;
		EDX = crc^b;
		EDX &= 0xFF;
		EDX <<= 2;
		EDX += crc32Table;
		crc >>= 8;
		crc ^= DSDWORD[EDX];
	}
	RETURN crc ^ 0xFFFFFFFF;
}

dword indexArray(dword address, key)
{
	dword offset = key&11b;
	dword offsetAddress = offset*4+address;
	IF (key==offset) RETURN 4*4+offsetAddress;
	IF (!DSDWORD[offsetAddress]) DSDWORD[offsetAddress] = malloc(4*4*2);
	RETURN indexArray(DSDWORD[offsetAddress], key>>2);
}

void set_procedure(dword name, address)
{
	dword data = 0;
	data = malloc(sizeStruct);
	DSDWORD[data] = Proc;
	DSDWORD[data+4] = address;
	indexArray(variable, crc32(name));
	DSDWORD[EAX] = data;
}

void set_variable(dword name, data)
{
	indexArray(variable, crc32(name));
	DSDWORD[EAX] = data;
}

dword string(dword lisp)
{
	dword buffer = 0;
	if (DSDWORD[lisp] == TList)
	{
		
		return "";
	}
	switch (DSDWORD[lisp])
	{
		case TString:
		case TSymbol:
			return DSDWORD[lisp+4];
		case TNumber:
			
			return itoa(DSDWORD[lisp+4]);
		case Lambda:
			return "[LAMBDA]";
		case Proc:
			return "[PROC]";
		case TObject:
			return "[OBJECT]";
		case TList:
			return "[LIST]";
	}
}

dword number(dword lisp)
{
	if (DSDWORD[lisp] == TNumber)
	{
		return DSDWORD[lisp+4];
	}
	if (DSDWORD[lisp] == TString)
	{
		//return atoi(DSDWORD[lisp+4]);
	}
	return 0;
}

dword lexer(dword code)
{
	byte s = 0;
	dword alloc = 0;
	dword buffer = 0;
	dword position = 0;
	dword key = 0;
	alloc = malloc(32);
	//con_printf stdcall(code);
	while(DSBYTE[code])
	{
		s = DSBYTE[code];
		code++;
		if (s == ' ') || (s == '\n') || (s == '\t') || (s == '\r') continue;
		if (s == '(') || (s == ')')
		{
			buffer = malloc(2);
			DSBYTE[buffer] = s;
			indexArray(alloc, key);
			DSDWORD[EAX] = buffer;
			key++;
			continue;
		}
		buffer = malloc(25);
		position = buffer;
		DSBYTE[position] = s;
		if (s == '"')
		{
			while(DSBYTE[code])
			{
				s = DSBYTE[code];
				position++;
				DSBYTE[position] = s;
				code++;
				if (s == '"') && (DSBYTE[code-2] != '\\') break;
			}
		}
		else
		{
			while(DSBYTE[code])
			{
				s = DSBYTE[code];
				if (s == ' ') || (s == '\n') || (s == '\t') || (s == '\r') || (s == '(') || (s == ')') break;
				position++;
				DSBYTE[position] = s;
				code++;
			}
		}
		indexArray(alloc, key);
		DSDWORD[EAX] = buffer;
		key++;
	}
	indexArray(alloc, key);
	DSDWORD[EAX] = 0;
	/*key = 0;
	do {
	buffer = indexArray(alloc, key);
	if (!DSDWORD[buffer]) break;
	con_printf stdcall ("\r\n");
	con_printf stdcall (DSDWORD[buffer]);key++;
	} while(1);*/
	return alloc;
}


dword tree(dword alloc)
{
	dword token = 0;
	dword list = 0;
	dword buffer = 0;
	dword temp = 0;
	dword listBuffer = 0;
	dword i = 0;
	token = indexArray(alloc, treeItem);
	treeItem++;
	buffer = DSDWORD[token];

	if (DSBYTE[buffer] == '(')
	{
		list = malloc(32);
		while(1) {
			token = indexArray(alloc, treeItem);
			token = DSDWORD[token];
			if (!token) || (DSBYTE[token] == ')') break;
			buffer = indexArray(list, i);
			DSDWORD[buffer] = tree(alloc);
			i++;
		}
		treeItem++;
		indexArray(list, i);
		DSDWORD[EAX] = 0;
		malloc(sizeStruct);
		DSDWORD[EAX] = TList;
		DSDWORD[EAX+4] = list;
		return EAX;
	}
	return atom(DSDWORD[token]);
}

dword hexdec2(dword buffer, length)
{
	dword r = 0;
	length += buffer;
	while (length != buffer)
	{
		length--;
		r <<= 4;
		if (DSBYTE[length] >= 'A') && (DSBYTE[length] <= 'F') r |= DSBYTE[length]-'A'+10;
		else if (DSBYTE[length] >= 'a') && (DSBYTE[length] <= 'f') r |= DSBYTE[length]-'a'+10;
		else if (DSBYTE[length] >= '0') && (DSBYTE[length] <= '9') r |= '9'-DSBYTE[length];
	}
	return r;
}

dword atom(dword token)
{
	dword buffer = 0;
	dword pos = 0;
	dword data = 0;
	if (DSBYTE[token] == '-') && (DSBYTE[token+1] >= '0') && (DSBYTE[token+1] <= '9')
	{
		malloc(sizeStruct);
		DSDWORD[EAX] = TNumber;
		DSDWORD[EAX+4] = atoi(token);
		return EAX;
	}
	if (DSBYTE[token] >= '0') && (DSBYTE[token] <= '9')
	{
		while (DSBYTE[token]) && (DSBYTE[token] >= '0') && (DSBYTE[token] <= '9')
		{
			data *= 10;
			data += DSBYTE[token]-'0';
			token++;
		}

		malloc(sizeStruct);
		DSDWORD[EAX] = TNumber;
		DSDWORD[EAX+4] = data;
		return EAX;
	}
	if (DSBYTE[token] == '"')
	{	
		pos = token;
		buffer = token;
		pos++;
		while (DSBYTE[pos]) && (DSBYTE[pos] != '"')
		{
			if (DSBYTE[pos] == '\\')
			{
				pos++;
				switch (DSBYTE[pos])
				{
					case 'n': DSBYTE[buffer] = 13; break; 
					case 'r': DSBYTE[buffer] = 10; break; 
					case 't': DSBYTE[buffer] = 9;  break; 
					case 'x':
						pos++;
						DSBYTE[buffer] = hexdec2(pos, 2);
						pos++;
					break; 
					default:
						DSBYTE[buffer] = DSBYTE[pos];
				}
			}
			else DSBYTE[buffer] = DSBYTE[pos];
			buffer++;
			pos++;
		}
		DSBYTE[buffer] = 0;
		malloc(sizeStruct);
		DSDWORD[EAX] = TString;
		DSDWORD[EAX+4] = token;
		DSDWORD[EAX+8] = token-buffer;
		return EAX;
	}
	pos = token;
	while (DSBYTE[pos])
	{
		if (DSBYTE[pos] >= 'a') && (DSBYTE[pos] <= 'z') DSBYTE[pos] = DSBYTE[pos]-'a'+'A';
		pos++;
	}
	malloc(sizeStruct);
	DSDWORD[EAX] = TSymbol;
	DSDWORD[EAX+4] = token;
	return EAX;
}

dword lisp(dword tree)
{
	dword buffer = 0;
	dword list = 0;
	dword args = 0;
	dword key = 0;
	dword item = 0;

	switch (DSDWORD[tree])
	{
		case TSymbol:
			buffer = indexArray(variable, crc32(DSDWORD[tree+4]));
			IF (!DSDWORD[buffer]) return tree;
			return DSDWORD[buffer];
		case TNumber:
		case TString:
			return tree;	
		case TList:
			list = DSDWORD[tree+4];
			buffer = indexArray(list, 0);
			if (!buffer) {
				malloc(sizeStruct);
				DSDWORD[buffer] = TSymbol;
				DSDWORD[buffer+4] = NIL;
				return buffer;
			}
			
			buffer = DSDWORD[buffer];
			if (DSDWORD[buffer] == TSymbol) || (DSDWORD[buffer] == TList)
			{
				buffer = DSDWORD[buffer+4];
				if (DSBYTE[buffer] == '\'') return tree;

				args = malloc(32);
				key = 0;
				while (1)
				{
					buffer = indexArray(list, key);
					buffer = DSDWORD[buffer];
					if (!buffer) break;
					item = indexArray(args, key);
					DSDWORD[item] = lisp(buffer);
					key++;
				}
				item = indexArray(args, 0);
				item = DSDWORD[item];
				if (DSDWORD[item] == Proc)
				{
					EAX = DSDWORD[item+4];
					EAX(args);
					if (!EAX)
					{
						malloc(sizeStruct);
						DSDWORD[EAX] = TSymbol;
						DSDWORD[EAX+4] = NIL;
						return EAX;
					}
					return EAX;
				}
				malloc(sizeStruct);
				DSDWORD[EAX] = TSymbol;
				DSDWORD[EAX+4] = NIL;
				return EAX;
			}

			malloc(sizeStruct);
				DSDWORD[EAX] = TSymbol;
				DSDWORD[EAX+4] = NIL;
				return EAX;
	}
}

#include "stdcall.h"

void main()
{
	dword xxx = 0;
	dword item = 0;
	dword data = 0;
	
	buffer = malloc(bufferSize);

	variable = malloc(32);
	NIL = "NIL";

	initFunctionLisp();
	
	if(DSBYTE[I_Param])
	{
		IF(io.read(I_Param))
		{
			lisp(tree(lexer(EAX)));
		}
	}
	else
	{
		consoleInit();
		con_printf stdcall ("Lisp v2.0\r\n");
		while(maxLoop)
		{
			treeItem = 0;
			con_printf stdcall ("\r\n$ ");
			con_gets stdcall(buffer+1, bufferSize);
			DSBYTE[buffer] = '(';
			xxx= lisp(tree(lexer(buffer)));
			con_printf stdcall (string(xxx));
			maxLoop--;
		}
	}

	IF(initConsole) con_exit stdcall (1);
	ExitProcess();
}

