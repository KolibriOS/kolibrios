#pragma option OST
#pragma option ON
#pragma option cri-
#pragma option -CPA
#initallvar 0
#jumptomain FALSE

#startaddress 0x0000

#code32 TRUE

char   os_name[8]   = {'M','E','N','U','E','T','0','1'};
dword  os_version   = 0x00000001;
dword  start_addr   = #main;
dword  final_addr   = #______STOP______+32;
dword  alloc_mem    = 20000;
dword  x86esp_reg   = 20000;
dword  I_Param      = #param;
dword  I_Path       = #program_path;
char param[4096] ={0};
char program_path[4096] = {0};


:dword mallocSystem(dword size)
{
	$push ebx
	$push ecx
	
	$mov     eax, 68
	$mov     ebx, 12
	$mov     ecx, size
	$int     0x40
	
	$pop ecx
	$pop ebx
	return  EAX;
}

:dword offsetAllocData = 0;
:dword malloc(dword size)
{
	dword array = 0;
	dword orderSize = 1;
	dword order = 0;
	dword stackAlloc = 0;
	dword stackKey = 0;
	dword result = 0;
	size+=4;
	IF(!offsetAllocData) offsetAllocData = mallocSystem(4*32);
	
	WHILE(orderSize<size)
	{
		orderSize<<=1;
		order++;
	}
	order<<=2;
	order += offsetAllocData;
	
	IF(!DSDWORD[order]) 
	{
		stackAlloc = mallocSystem(4*50);
		DSDWORD[order] = stackAlloc;
		DSDWORD[stackAlloc] = stackAlloc;
	}
	ELSE stackAlloc = DSDWORD[order];
	
	stackKey = DSDWORD[stackAlloc];
	IF(stackKey == stackAlloc)
	{
		result = mallocSystem(orderSize);
		DSDWORD[result] = orderSize;
		return result+4;
	}
	ELSE
	{
		result = DSDWORD[stackKey];
		DSDWORD[stackAlloc] = stackKey-4;
		return result+4;
	}
}

/*:dword freeSystem(dword mptr)
{
	$push    eax
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
	$pop     eax
	return 0;
}*/

:dword free(dword ptr)
{
	dword array = 0;
	dword orderSize = 1;
	dword order = 0;
	dword stackAlloc = 0;
	dword stackKey = 0;
	dword result = 0;
	dword size = 0;
	size = DSDWORD[ptr-4];
	IF(!offsetAllocData) return 0;
	
	WHILE(orderSize!=size)
	{
		orderSize<<=1;
		order++;
	}
	order<<=2;
	order += offsetAllocData;
	stackAlloc = DSDWORD[order];
	DSDWORD[stackAlloc] += 4;
	stackKey = DSDWORD[stackAlloc];
	order = ptr;
	orderSize -= 4;
	WHILE(orderSize)
	{
		DSBYTE[order] = 0;
		order++;
		orderSize--;
	}
	DSDWORD[stackKey] = ptr-4;
}

:dword realloc(dword ptr,size)
{
	dword newMem = 0;
	dword oldSize = 0;
	dword ptr1 = 0;
	dword ptr2 = 0;
	newMem = malloc(size);
	oldSize = DSDWORD[ptr-4] - 4;
	ptr1 = ptr;
	ptr2 = newMem;
	WHILE(oldSize)
	{
		DSBYTE[ptr2] = DSBYTE[ptr1];
		ptr1++;
		ptr2++;
		oldSize--;
	}
	free(ptr);
	RETURN newMem;
}

void main()
{
	dword o1 = 0;
	dword o2 = 0;
	dword o3 = 0;
	while(1)
	{
		o1 = malloc(1000);
		o2 = malloc(10000);
		o3 = malloc(1000);
		o1 = realloc(o3,2000);
		free(o2);
		free(o1);
		free(o3);
	}
	
	EAX = -1;
	$int 0x40;
}


______STOP______: