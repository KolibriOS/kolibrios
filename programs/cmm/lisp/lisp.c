/*
 * Author Pavel Iakovlev
*/

#define MEMSIZE 4096*10

#include "../lib/io.h"
#include "../lib/obj/console.h"
#include "../lib/array.h"

byte initConsole = 0;
Dictionary functions = {0};
Dictionary variables = {0};

#include "stdcall.h"

#define bufferSize 10000;
#define memoryBrainfuck 30000*4
#define memoryByteBF 1
#define stackBrainFuck 4*1024

dword buffer = 0;
word bufferSymbol = 0;
dword memory = 0;

dword stack = 0;
dword code = 0;
byte tempBuffer[100] = {0};

void consoleInit()
{
	IF(!initConsole) 
	{
		load_dll(libConsole, #con_init, 0);
		con_init stdcall (-1, -1, -1, -1, "Lisp interpreter");
		initConsole = 0xFF;
	}
}

dword evalLisp()
{
	byte s = 0;
	byte args = 0;
	dword pos = 0;
	dword name = 0;
	dword tmp = 0;
	dword dataArgs = 0;
	dword posArgs = 0;
	dword ret = 0;
	dword p = 0;
	dataArgs = malloc(16*4);
	posArgs = dataArgs;
	name = malloc(100);
	pos = name;
	loop()
	{
		s = DSBYTE[code];
		
		while(s == ' ')
		{
			code++;
			s = DSBYTE[code];
		}
		
		if (!s) || (s==')') 
		{
			code++;
			break;
		}
		
		if (!args) 
		{
			if (s != ')') // name function
			{
				while (s != ' ') && (s != ')')
				{
					DSBYTE[pos] = s;
					pos++;
					code++;
					s = DSBYTE[code];
				}
				code--;
			}
		}
		else
		{
			if(s == '(') 
			{
				code++;
				tmp = evalLisp();
			}
			else if (s >= '0') && (s <= '9')
			{
				tmp = 0;
				while (s >= '0') && (s <= '9')
				{
					tmp *= 10;
					tmp += s-'0';
					code++;
					s = DSBYTE[code];
				}
				code--;
			}
			else if (s == '"')
			{
				tmp = malloc(100);
				p = tmp;
				code++;
				s = DSBYTE[code];
				while (s != '"') && (s)
				{
					DSBYTE[p] = s;
					p++;
					
					code++;
					s = DSBYTE[code];
				}
				DSBYTE[p] = 0;
			}
			else if(s >= 'A') && (s <= 'z')
			{
				tmp = malloc(100);
				p = tmp;
				while (s >= 'A') && (s <= 'z') 
				{
					DSBYTE[p] = s;
					p++;
					
					code++;
					s = DSBYTE[code];
				}
				DSBYTE[p] = 0;
			}
			
			DSDWORD[posArgs] = tmp;
			posArgs += 4;
		}
		code++;
		args++;
	}
	args--;
	ret = StdCall(args, name, dataArgs);
	free(name);
	free(dataArgs);
	return ret;
}

void main()
{
	dword brainFuckCode = 0;
	word maxLoop = 1000;
	dword txt = "  (print 1)(print 2)";
	
	buffer = malloc(bufferSize);
	memory = malloc(memoryBrainfuck);
	stack = malloc(stackBrainFuck);
	
	Init();
	
	IF(DSBYTE[I_Param])
	{
		IF(io.read(I_Param))
		{
			code = EAX;
			loop()
			{
				WHILE(DSBYTE[code] == ' ') code++;
				IF(DSBYTE[code]!='(') BREAK;
				ELSE code++;
				evalLisp();
				code--;
				IF(DSBYTE[code]!=')') BREAK;
				ELSE code++;
			}
		}
	}
	else 
	{
		consoleInit();
		con_printf stdcall ("Lisp interpreter v1.2");
		while(maxLoop)
		{
			con_printf stdcall ("\r\n\r\nEnter code: ");
			con_gets stdcall(buffer, bufferSize);
			code = EAX;
			//code = txt;
			con_printf stdcall ("Output: ");
			nextLispLine:
			while(DSBYTE[code] == ' ') code++;
			if(DSBYTE[code] == '(') code++;
			else goto endNext;
			evalLisp();
			code--;
			if(DSBYTE[code]==')') code++;
			else goto endNext;
			IF(!DSBYTE[code]) goto endNext;
			goto nextLispLine;
			endNext:
			maxLoop--;
		}
	}
	
	IF(initConsole) con_exit stdcall (0);
	ExitProcess();
}

