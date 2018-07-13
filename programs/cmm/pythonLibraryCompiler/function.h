
:void _stdout(dword txt)
{
	dword addr = txt;
	while(DSBYTE[addr]) addr++;
	CreateFile(addr-txt,txt,"/sys/stdout");
}

:dword stdcall std_stdout(dword count)
{
	_stdout(_str(popFast()));
}

:dword stdcall std_str(dword c,v)
{
	dword n = 0;
	dword m = 0;
	dword l = 0;
	
	if(DSBYTE[v+4] == PY_STR)
	{
		return v;
	}
	
	n = malloc(30);
	m = malloc(MEMBUF);
	
	DSDWORD[n] = m;
	DSBYTE[n+4] = PY_STR;
	DSDWORD[n+9] = MEMBUF;
	DSDWORD[n+13] = HASH;
	HASH++;

	switch(DSBYTE[v+4])
	{
		case PY_BOOL:

			if(DSDWORD[v])
			{
				strlcpy(m,"True",4);
				DSDWORD[n+5] = 4;
			}
			else
			{
				strlcpy(m,"False",5);
				DSDWORD[n+5] = 5;
			}

		break;
		case PY_INT:
			l = itoa(DSDWORD[v],m);
			DSDWORD[n+5] = l;
		break;
		case PY_NONE:
			strlcpy(m,"None",4);
			DSDWORD[n+5] = 4;
		break;
	}
	return n;
}