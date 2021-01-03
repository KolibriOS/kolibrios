/*
	STDCALL function
	Author: PaulCodeman
*/

void initFunctionLisp()
{
	set_procedure("TEST",  #lisp_test);
	set_procedure("SLEEP", #lisp_sleep);
	set_procedure("PRINT", #lisp_print);
	set_procedure("INPUT", #lisp_input);	
	set_procedure("STDCALL", #lisp_stdcall);
	set_procedure("SETQ",   #lisp_setq);
	set_procedure("DEFVAR", #lisp_setq);
	set_procedure("+",     #lisp_add);
	set_procedure("-",     #lisp_sub);
	set_procedure("=",     #lisp_cmp);
}

dword lisp_test(dword args)
{
	malloc(sizeStruct);
	DSDWORD[EAX] = TString;
	DSDWORD[EAX+4] = "ZZZ";
	return EAX;
}

dword lisp_setq(dword args)
{
	dword i = 0;
	dword name = 0;
	dword data = 0;
	while(1)
	{
		i++;
		data = indexArray(args, i);
		data = DSDWORD[data];
		IF (!data) break;
		
		if (i&1)
		{
			name = DSDWORD[data+4];
		}
		else
		{
			set_variable(name, data);
		}
	}
	return 0;
}

dword lisp_print(dword args)
{
	dword i = 0;
	consoleInit();
	while(1)
	{
		i++;
		indexArray(args, i);
		IF (!DSDWORD[EAX]) break;
		con_printf stdcall (string(DSDWORD[EAX]));
	}
	con_printf stdcall ("\r\n");
	return 0;
}

dword lisp_stdcall(dword args)
{
	dword i = 0;
	dword buffer = 0;
	while(1)
	{
		i++;
		indexArray(args, i);
		buffer = DSDWORD[EAX];
		IF (!buffer) break;
		$push DSDWORD[buffer+4];
	}
	IF (i == 2) $pop eax
	IF (i == 3) $pop ebx
	IF (i == 4) $pop ecx
	$int 0x40
	return EAX;
}

dword lisp_input(dword args)
{
	dword buffer = 0;
	consoleInit();
	buffer = malloc(100);
	con_gets stdcall(buffer, 100);
	malloc(sizeStruct);
	DSDWORD[EAX] = TString;
	DSDWORD[EAX+4] = buffer;
	return EAX;
}

dword lisp_inc(dword args)
{
	dword i = 0;
	dword sum = 0;
	dword buffer = 0;
	while(1)
	{
		i++;
		buffer = indexArray(args, i);
		IF (!DSDWORD[buffer]) break;
		buffer = DSDWORD[buffer];
	}
	return 0;
}

dword lisp_add(dword args)
{
	dword i = 0;
	dword sum = 0;
	dword buffer = 0;
	while(1)
	{
		i++;
		buffer = indexArray(args, i);
		IF (!DSDWORD[buffer]) break;
		buffer = DSDWORD[buffer];
		
		sum += number(buffer);
	}
	malloc(sizeStruct);
	DSDWORD[EAX] = TNumber;
	DSDWORD[EAX+4] = sum;
	return EAX;
}

dword lisp_sub(dword args)
{
	dword i = 0;
	dword sum = 0;
	while(1)
	{
		i++;
		indexArray(args, i);
		IF (!DSDWORD[EAX]) break;
		sum -= number(DSDWORD[EAX]);
	}
	malloc(sizeStruct);
	DSDWORD[EAX] = TNumber;
	DSDWORD[EAX+4] = sum;
	return EAX;
}

dword lisp_cmp(dword args)
{
	dword i = 0;
	dword first = 0;
	dword buffer = 0;
	
	while(1)
	{
		i++;
		buffer = indexArray(args, i);
		buffer = DSDWORD[buffer];
		IF (!buffer) break;
		if (i == 1)
		{
			first = buffer;
		}
		else 
		{
			if (DSDWORD[first+4] != DSDWORD[buffer+4])
			{
				malloc(sizeStruct);
				DSDWORD[EAX] = TSymbol;
				DSDWORD[EAX+4] = NIL;
				return EAX;
			}
		}
	}
	if (i == 1) error_message("*** - EVAL: too few arguments given to =: (=)");
	malloc(sizeStruct);
	DSDWORD[EAX] = TSymbol;
	DSDWORD[EAX+4] = "T";
	return EAX;
}

dword lisp_sleep(dword args)
{
	dword time = 0;
	indexArray(args, 1);
	time = number(DSDWORD[EAX]);
	EAX = 5;
	EBX = time;
	$int 0x40
	return 0;
}
