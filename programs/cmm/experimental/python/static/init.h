:void ExitProcess()
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

:int AddFile(dword begin,data_size, buffer, file_path)
{
	write_file_70.func = 3;
	write_file_70.param1 = begin;
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
stdcall dword realloc(dword mptr, size)
{
        $push    ebx
        $push    ecx
        $push    edx
 
        $mov     eax, 68
        $mov     ebx, 20
        $mov     ecx, size
        $mov     edx, mptr
        $int     0x40
 
        $pop     edx
        $pop     ecx
        $pop     ebx
        return   EAX;
}

:void memcpy(dword x,y,size)
{
	dword ost = 0;
	dword del = 0;
	del = size/4;
	ost = size%4;
	WHILE(del)
	{
		DSDWORD[x] = DSDWORD[y];
		del--;
		x+=4;
		y+=4;
	}
	IF(ost>1)
	{
		DSWORD[x] = DSWORD[y];
		x+=2;
		y+=2;
		ost-=2;
	}
	IF(ost==1) DSBYTE[x] = DSBYTE[y];
}

:dword sleep(dword x)
{
	EAX = 5;
	EBX = x;
	$int 0x40
}
:dword memInit()
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

:dword strcpy(dword s1,s2)
{
	X = s1;
	WHILE(DSBYTE[s2])
	{
		DSBYTE[s1] = DSBYTE[s2];
		s1++;
		s2++;
	}
	RETURN s1-X;
}
:dword strlcpy(dword s1,s2,l)
{
	X = l;
	WHILE(l)
	{
		DSBYTE[s1] = DSBYTE[s2];
		s1++;
		s2++;
		l--;
	}
	RETURN X;
}

:dword _str(dword v)
{
	SWITCH(DSBYTE[v+4])
	{
		CASE PY_STR:
			RETURN DSDWORD[v];
		CASE PY_BOOL:
			IF(DSDWORD[v]) RETURN "True";
			RETURN "False";
	}
	
}

:dword stack = 0;
:dword popStack()
{
	stack-=4;
	RETURN DSDWORD[stack];
}
:void pushStack(dword x)
{
	dword tmp = 0;
	IF(!DSDWORD[stack]) tmp = malloc(30);
	ELSE tmp = DSDWORD[stack];
	IF(x) memcpy(tmp,x,30);
	DSDWORD[stack] = tmp;
	stack+=4;
}

:dword stackFast = 0;
:dword beginStackFast = 0;
:dword popFast()
{
	stackFast-=4;
	RETURN DSDWORD[stackFast];
}
:void pushFast(dword x)
{
	dword tmp = 0;
	IF(!DSDWORD[stackFast]) tmp = malloc(30);
	ELSE tmp = DSDWORD[stackFast];
	IF(x) memcpy(tmp,x,30);
	DSDWORD[stackFast] = tmp;
	stackFast+=4;
}

:dword stackLoop = 0;
:dword popLoop()
{
	stackLoop-=4;
	RETURN DSDWORD[stackLoop];
}
:void pushLoop(dword x)
{
	DSDWORD[stackLoop] = x;
	stackLoop+=4;
}

:void checkType(dword x,y)
{
	IF(DSBYTE[x+4]!=DSBYTE[y+4]) 
	{
		_stdout("Error type!");
		ExitProcess();
	}
}

:void sprintf(dword str, ...)
{
	//S = DSBYTE[format];
	X = DSDWORD[ESP]-ESP-8;
	X += ESP;
	A = DSDWORD[X];
	X-=4;
	B = DSDWORD[X];
	S = DSBYTE[B];
	while(S)
	{
		if(S=='%')
		{
			B++;
			S = DSBYTE[B];
			IF(S=='s')
			{
				X-=4;
				strcpy(A,DSDWORD[X]);
				A+=EAX;
				B++;
				S = DSBYTE[B];
				continue;
			}
			IF(S=='c')
			{
				X-=4;
				EAX = DSDWORD[X];
				test(EAX,1);
				A++;
				B++;
				S = DSBYTE[B];
				continue;
			}
			ELSE IF(S=='d')
			{
				
			}
			ELSE
			{
				DSBYTE[A] = S;
				A++;
			}
		}
		else 
		{
			DSBYTE[A] = S;
			A++;
		}
		B++;
		S = DSBYTE[B];
	}
}

:byte BUFTST[15] = {0};
:void test1(dword x,y)
{

	IF(y)
	{
		itoa(x,#BUFTST);
		_stdout(#BUFTST);
	}
	ELSE _stdout(x);
	EAX = -1;
	$int 0x40;
}

:void test2(dword x,y)
{
	_stdout("[");
	IF(y)
	{
		itoa(x,#BUFTST);
		_stdout(#BUFTST);
	}
	ELSE _stdout(x);
	_stdout("]");
}



:void ______INIT______()
{
	dword o = 0;
	dword o2 = 0;
	memInit();

	stack = malloc(1000);
	beginStack = stack;
	stackFast = malloc(1000);
	beginStackFast = stackFast;
	stackLoop = malloc(1000);
	TEMP = malloc(500);
	
	//o = malloc(16*4);
	//memcpy(o,"test123456789",9);

	//test(123);
	//sprintf(TEMP,"asd%sa=%c123","ok",123);
	//test(TEMP,0);
	
	//importLibrary("console");
	main();
	ExitProcess();
}

:void ______STOP______()
{
	ExitProcess();
}