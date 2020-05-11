

:void copyVariable(dword x)
{
	load_const(DSDWORD[x],DSBYTE[x+4],DSDWORD[x+5]);
}


:dword LenStdout = 0;
:void _stdout(dword txt)
{
	dword addr = txt;
	WHILE(DSBYTE[addr]) addr++;
	IF(!LenStdout)
	{
		LenStdout = addr-txt;
		CreateFile(LenStdout,txt,"/sys/stdout");
	}
	ELSE 
	{
		AddFile(LenStdout,addr-txt,txt,"/sys/stdout");
		LenStdout+=addr-txt;
	}
}
:dword std__stdStr(dword count)
{
	return _str(popFast());
}
:dword stdcall std__stdout(dword count)
{
	std__str(count);
	popStack();
	_stdout(DSDWORD[EAX]);
}
:dword stdcall std__len(dword c)
{
	popFast();
	load_const(DSDWORD[EAX+5],PY_INT,0);
	RETURN 0;
}

:dword stdcall std__INT40(dword c)
{
	X = popFast();
	Y = DSDWORD[X+5];
	X = DSDWORD[X];
	X = DSDWORD[X];
	X = DSDWORD[X];
	test(X,1);
	
	RETURN 0;
}

:void hexAddress(dword x,y)
{
	dword i = 0;
	i = 8;
	WHILE(i)
	{
		DSBYTE[x] = DSBYTE[y&0xF+#arraySymbolHEX];
		y>>=4;
		x--;
		i--;
	}
}
#define rSize DSBYTE[#RDATA+5]
#define rData DSBYTE[#RDATA]
#define rType DSBYTE[#RDATA+4]

:dword stdcall std__str(dword c)
{
	dword n = 0;
	dword m = 0;
	dword l = 0;
	dword v = 0;
	dword f = "<function a at 0x00000000>";
	dword t = 0;
	dword o = 0;
	
	v = popFast();
	IF(DSBYTE[v+4] == PY_STR)
	{
		copyVariable(v);
		return 0;
	}

	switch(DSBYTE[v+4])
	{
		case PY_BOOL:
			if(DSDWORD[v])
			{
				load_const("True",PY_STR,4);
				return 0;
			}
			load_const("False",PY_STR,5);
			return 0;
		break;
		case PY_INT:
			m = malloc(MEMBUF);
			l = itoa(DSDWORD[v],m);
			load_const(m,PY_STR,l);
			return 0;
		break;
		case PY_NONE:
			load_const("None",PY_STR,4);
			return 0;
		break;
		case PY_FNC:
			hexAddress(f+24,v);
			load_const(f,PY_STR,26);
			return 0;
		break;
	}
}