:dword std_print(dword count, args)
{
	consoleInit();
	WHILE(count)
	{
		con_printf stdcall (DSDWORD[args]);
		args+=4;
		count--;
	}
}

:dword std_str(dword count, args)
{
	dword tmp = 0;
	tmp = malloc(15);
	itoa_(tmp,DSDWORD[args]);
	RETURN tmp;
}

:dword std_add(dword count, args)
{
	dword ret = 0;
	WHILE(count)
	{
		ret += DSDWORD[args];
		args+=4;
		count--;
	}
	RETURN ret;
}

:dword std_sub(dword count, args)
{
	dword ret = 0;
	IF(count)
	{ 
		ret = DSDWORD[args];
		count--;
		args+=4;
	}
	WHILE(count)
	{
		ret -= DSDWORD[args];
		args+=4;
		count--;
	}
	RETURN ret;
}

void Init()
{
	functions.init(100);
	
	/* Console functions */
	functions.set("print", #std_print);
	
	/* String functions */
	functions.set("str", #std_str);
	
	/* System functions */
	functions.set("exit", #ExitProcess);
	
	/* Math functions */
	functions.set("+", #std_add);
	functions.set("-", #std_sub);
	
	variables.init(100);
}

dword StdCall(dword count, name, args)
{
	dword tmp = 0;
	functions.get(name);
	IF(EAX)RETURN EAX(count, args);
	RETURN 0;
}