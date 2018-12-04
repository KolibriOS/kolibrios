dword StdCall(dword count, name, args)
{
	dword tmp = 0;
	if(!strcmp(name, "print"))
	{
		consoleInit();
		con_printf stdcall (DSDWORD[args]);
	}
	else if(!strcmp(name, "input"))
	{
		
	}
	else if(!strcmp(name, "str"))
	{
		tmp = malloc(15);
		itoa_(tmp,DSDWORD[args]);
		return tmp;
	}
	else if(!strcmp(name, "exit"))
	{
		ExitProcess();
	}
	return 0;
}