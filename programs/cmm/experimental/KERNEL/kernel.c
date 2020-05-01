#pragma option OST
#pragma option ON
#pragma option cri-
#pragma option -CPA
#initallvar 0
#jumptomain FALSE
 
#startaddress 0
 
#code32 TRUE

char   os_name[8]   = {'M','E','N','U','E','T','0','1'};
dword  os_version   = 0x00000001;
dword  start_addr   = #____INIT____;
dword  final_addr   = #____STOP____+32;
dword  alloc_mem    = 4*1024*1024;
dword  x86esp_reg   = 4*1024*1024;
dword  I_Param      = #param;
dword  I_Path       = #program_path;
char param[4096];
char program_path[4096];


void ExitProcess()
{
	EAX = -1;
	$int 0x40
}

segmentDisableFunction:
dword eaxFunctionDestroy(){RETURN 0;}
segmentDisableFunctionEnd:

void ____INIT____()
{

//    Disable door kernel
	EAX = 81;
	EBX = 81; // Self function
	ECX = #eaxFunctionDestroy; // Function execute
	EDX = #segmentDisableFunction; // Begin segment
	ESI = #segmentDisableFunctionEnd; // End segment
	$int 0x40
    
	ExitProcess();
}

void ____STOP____()
{
	ExitProcess();
}
