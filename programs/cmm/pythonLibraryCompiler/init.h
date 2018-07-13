void ExitProcess()
{
	EAX = -1;
	$int 0x40
}

:f70 write_file_70;
:int CreateFile(dword data_size, buffer, file_path)
{
        write_file_70.func = 2;
        write_file_70.param1 = 0;
        write_file_70.param2 = 0;
        write_file_70.param3 = data_size;
        write_file_70.param4 = buffer;
        write_file_70.rezerv = 0;
        write_file_70.name = file_path;
        $mov eax,70
        $mov ebx,#write_file_70.func
        $int 0x40
}

dword malloc(dword size)
{
        $push    ebx
        $push    ecx
 
        $mov     eax, 68
        $mov     ebx, 12
        $mov     ecx, size
        $int     0x40
       
        $pop     ecx
        $pop     ebx
        return  EAX;
}
dword free(dword mptr)
{
        $push    ebx
        $push    ecx
       
        $mov     eax, 68
        $mov     ebx, 13
        $mov     ecx, mptr
        $test    ecx, ecx
        $jz      end0
        $int     0x40
   @end0:
        $pop     ecx
        $pop     ebx
        return 0;
}
:dword sleep(dword x)
{
	EAX = 5;
	EBX = x;
	$int 0x40
}
:dword mem_init()
{
        $push    ebx
        $mov     eax, 68
        $mov     ebx, 11
        $int     0x40
       
        $pop     ebx
        return  EAX;
}

:dword itoa(signed long number,dword p)
{
        dword ret=0;
        byte cmd=0;
        long mask=0;
		long tmp=0;
		IF(!number)
		{
			DSBYTE[p] = '0';
			p++;
			DSBYTE[p] = 0;
			return 1;
		}
        mask = 1000000000;
        cmd = 1;
        if(!number){
                ESBYTE[p] = '0';
                ESBYTE[p+1] = 0;
                return p;
        }
        ret = p;
        if(number<0)
        {
                $neg number
                ESBYTE[p] = '-';
                $inc p
        }
        while(mask)
        {
                tmp = number / mask;
                tmp = tmp%10;
               
                if(cmd){
                        if(tmp){
                                ESBYTE[p] = tmp + '0';
                                $inc p
                                cmd = 0;
                        }
                }
                else {
                        ESBYTE[p] = tmp + '0';
                        $inc p
                }
                mask /= 10;
        }
        ESBYTE[p] = 0;
        return p - ret;
}

:void strlcpy(dword s1,s2,l)
{
	WHILE(l)
	{
		DSBYTE[s1] = DSBYTE[s2];
		s1++;
		s2++;
		l--;
	}
}

:dword _str(dword v)
{
	switch(DSBYTE[v+4])
	{
		case PY_STR:
			return DSDWORD[v];
		break;
		case PY_BOOL:
			if(DSDWORD[v])return "True";
			return "False";
		break;
		
	}
}

:dword stack = 0;

:dword popStack()
{
	stack-=4;
	return DSDWORD[stack];
}
:void pushStack(dword x)
{
	DSDWORD[stack] = x;
	stack+=4;
}
:dword stackFast = 0;
:dword popFast()
{
	stackFast-=4;
	return DSDWORD[stackFast];
}
:void pushFast(dword x)
{
	DSDWORD[stackFast] = x;
	stackFast+=4;
}

:dword stackWhile = 0;

:dword popWhile()
{
	stackWhile-=4;
	return DSDWORD[stackWhile];
}
:void pushWhile(dword x)
{
	DSDWORD[stackWhile] = x;
	stackWhile+=4;
}

:void checkType(dword x,y)
{
	if(DSBYTE[x+4]!=DSBYTE[y+4]) 
	{
		stdout("Error type!");
		ExitProcess();
	}
}


:byte BUFTST[15] = {0};
:void test(dword x,c)
{
	if(c)
	{
		itoa(x,#BUFTST);
		_stdout(#BUFTST);
	}
	else _stdout(x);
	EAX = -1;
	$int 0x40;
}

void ______INIT______()
{
	mem_init();
	
	HASH = 0;
	
	stack = malloc(1000);
	stackFast = malloc(1000);
	stackWhile = malloc(1000);
	
	main();
	ExitProcess();
}

void ______STOP______()
{
	ExitProcess();
}