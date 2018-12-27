
/* Lisp functions */
:dword std_exit()
{
	con_exit stdcall (1);
	ExitProcess();
}

:dword std_set(dword count, args)
{
	dword name = 0;
	dword value = 0;
	WHILE(count > 0)
	{
		name = DSDWORD[args];
		args += 4;
		value = DSDWORD[args];
		args += 4;
		variables.set(name, value);
		count -= 2;
	}
}

:dword std_get(dword count, args)
{
	IF(!count) RETURN 0;
	RETURN variables.get(DSDWORD[args]);
}

:dword std_str(dword count, args)
{
	dword tmp = 0;
	IF(!count) RETURN "";
	tmp = malloc(15);
	itoa_(tmp,DSDWORD[args]);
	RETURN tmp;
}

/* Math functions */
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
		args += 4;
		count--;
	}
	RETURN ret;
}

/* Console functions */
:dword std_print(dword count, args)
{
	dword ret = 0;
	consoleInit();
	WHILE(count)
	{
		IF(!DSDWORD[args]) con_printf stdcall ("nil");
		ELSE con_printf stdcall (DSDWORD[args]);
		args+=4;
		count--;
	}
	RETURN ret;
}

:dword std_input(dword count, args)
{
	dword buf = 0;
	consoleInit();
	buf = malloc(100);
	WHILE(count)
	{
		con_printf stdcall (DSDWORD[args]);
		args+=4;
		count--;
	}
	con_gets stdcall(buf, 100);
	RETURN EAX;
}

void Init()
{
	functions.init(100);
	
	/* Console functions */
	functions.set("print", #std_print);
	functions.set("input", #std_input);
	
	/* String functions */
	functions.set("str", #std_str);
	
	/* System functions */
	functions.set("exit", #std_exit);
	
	/* Math functions */
	functions.set("+", #std_add);
	functions.set("-", #std_sub);
	
	/* Lisp functions */
	functions.set("set", #std_set);
	functions.set("get", #std_get);
	
	variables.init(100);
}

dword StdCall(dword count, name, args)
{
	functions.get(name);
	IF(EAX) RETURN EAX(count, args);
	RETURN 0;
}

