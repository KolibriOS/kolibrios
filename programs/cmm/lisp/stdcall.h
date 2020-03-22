
/* Lisp functions */

:dword std_sleep(dword count, args)
{
	dword ret = 0;
	dword arg = 0;
	dword val = 0;
	WHILE(count)
	{
		arg = DSDWORD[args];
		REPEAT1:
		IF (DSDWORD[arg+4] == TSym) 
		{
			arg = std_get(1, args);
			goto REPEAT1;
		}
		IF (DSDWORD[arg+4] == TInt)
		{
			EAX = 5;
			EBX = DSDWORD[arg];
			$int 0x40
		}
		args+=4;
		count--;
	}
	RETURN ret;
}

:dword std_set(dword count, args)
{
	dword name = 0;
	dword value = 0;
	WHILE(count > 0)
	{
		name = DSDWORD[args];
		IF (DSDWORD[name+4] == TSym) name = DSDWORD[name];
		ELSE 
		{
			con_printf stdcall ("Error variable!");
			ExitProcess();
		}
		args += 4;
		value = DSDWORD[args];
		args += 4;
		variables.set(name, value);
		count -= 2;
	}
}

:dword std_get(dword count, args)
{
	dword name = 0;
	IF(!count) RETURN 0;
	name = DSDWORD[args];
	IF (DSDWORD[name+4] != TSym)
	{
		con_printf stdcall ("Error variable!");
		ExitProcess();
	}
	RETURN variables.get(DSDWORD[name]);
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

:dword std_exit(dword count, args)
{
	IF(initConsole) con_exit stdcall (1);
	ExitProcess();
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

:dword std_type(dword count, args)
{
	dword ret = 0;
	dword arg = 0;
	dword val = 0;
	ret = malloc(TLen);
	DSDWORD[ret] = "nil";
	DSDWORD[ret+4] = TStr;
	WHILE(count)
	{
		arg = DSDWORD[args];
		REPEAT1:
		IF (DSDWORD[arg+4] == TSym) 
		{
			arg = std_get(1, args);
			goto REPEAT1;
		}
		switch (DSDWORD[arg+4])
		{
			case TStr:
				DSDWORD[ret] = "string";
			break;
			case TInt:
				DSDWORD[ret] = "integer";
			break;
		}
		args+=4;
		count--;
	}
	RETURN ret;
}

/* Console functions */
:dword std_print(dword count, args)
{
	dword ret = 0;
	dword arg = 0;
	dword val = 0;
	consoleInit();
	IF (!count) con_printf stdcall ("nil");
	WHILE(count)
	{
		arg = DSDWORD[args];
		REPEAT1:
		IF (DSDWORD[arg+4] == TInt) val = itoa(DSDWORD[arg]);
		ELSE IF (DSDWORD[arg+4] == TStr) val = DSDWORD[arg];
		ELSE IF (DSDWORD[arg+4] == TSym) 
		{
			arg = std_get(1, args);
			goto REPEAT1;
		}
		IF(!arg) con_printf stdcall ("nil");
		ELSE con_printf stdcall (val);
		args+=4;
		count--;
	}
	RETURN ret;
}

:dword std_len(dword count, args)
{
	dword ret = 0;
	dword arg = 0;
	dword val = 0;
	ret = malloc(TLen);
	DSDWORD[ret] = 0;
	DSDWORD[ret+4] = TInt;
	WHILE(count)
	{
		arg = DSDWORD[args];
		REPEAT1:
		IF (DSDWORD[arg+4] == TStr) val = DSDWORD[arg];
		ELSE IF (DSDWORD[arg+4] == TSym) 
		{
			arg = std_get(1, args);
			goto REPEAT1;
		}
		ELSE return ret;
		DSDWORD[ret] += DSDWORD[arg+8];
		args+=4;
		count--;
	}
	RETURN ret;
}

:dword std_cmp(dword count, args)
{
	dword ret = 0;
	dword arg = 0;
	dword val = 0;
	dword tmp = 0;
	dword x = 0;
	dword y = 0;
	byte start = 0;
	ret = malloc(TLen);
	DSDWORD[ret] = 0;
	DSDWORD[ret+4] = TInt;
	IF (!count) return ret;
	while(count)
	{
		arg = DSDWORD[args];
		REPEAT2:
		IF (DSDWORD[arg+4] == TSym) 
		{
			arg = std_get(1, args);
			goto REPEAT2;
		}
		IF (!start)
		{
			start = 1;
			tmp = arg;
			args+=4;
			count--;
			continue;
		}
		IF (DSDWORD[tmp+4] != DSDWORD[arg+4]) return ret;
		IF (DSDWORD[tmp+4] == TInt)
		{
			IF (DSDWORD[tmp] != DSDWORD[arg]) return ret;
		}
		ELSE IF (DSDWORD[tmp+4] == TStr)
		{
			/*IF (!DSDWORD[tmp+8]) DSDWORD[tmp+8] = crc32(DSDWORD[tmp]);
			IF (!DSDWORD[arg+8]) DSDWORD[arg+8] = crc32(DSDWORD[arg]);
			IF (DSDWORD[tmp+8] != DSDWORD[arg+8]) return ret;*/
			IF (strcmp(DSDWORD[tmp], DSDWORD[arg])) return ret;
		}
		args+=4;
		count--;
	}
	DSDWORD[ret] = 1;
	return ret;
}

:dword std_input(dword count, args)
{
	dword buf = 0;
	consoleInit();
	buf = malloc(100);
	IF (count) std_print(count, args);
	con_gets stdcall(buf, 100);
	EDX = malloc(TLen);
	DSDWORD[EDX] = buf;
	DSDWORD[EDX+4] = TStr;
	RETURN EDX;
}

void Init()
{
	functions.init(100);
	
	/* Console functions */
	functions.set("print", #std_print);
	functions.set("input", #std_input);
	
	/* String functions */
	functions.set("str", #std_str);
	functions.set("len", #std_len);
	
	/* System functions */
	functions.set("exit", #std_exit);
	
	/* Math functions */
	functions.set("+", #std_add);
	functions.set("-", #std_sub);
	functions.set("==", #std_cmp);
	
	/* Lisp functions */
	functions.set("set", #std_set);
	functions.set("get", #std_get);
	functions.set("type", #std_type);
	functions.set("sleep", #std_sleep);

	
	variables.init(100);
}

dword StdCall(dword count, name, args)
{
	functions.get(name);
	IF(EAX) RETURN EAX(count, args);
	RETURN 0;
}

