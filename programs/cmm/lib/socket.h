#define SOCKET_PASSIVE 0
#define SOCKET_ACTIVE  1
inline fastcall dword OpenSocket( ECX, EDX, ESI, EDI)
{
	$mov eax,53
	$mov ebx,5
	$int 0x40
} 

inline fastcall dword CloseSocket( ECX )
{
	$mov eax, 53
	$mov ebx, 8
	$int 0x40
}

inline fastcall int StatusSocket( ECX)
{
	$mov eax,53
	$mov ebx,6
	$int 0x40
} 

inline fastcall dword ReadSocket( ECX)
{
	$mov eax,53
	$mov ebx,3
	$int 0x40
	return BL;
}

inline fastcall dword ReadNetworkData( ECX, EDX, ESI)
{
	$mov eax, 53
	$mov ebx, 11
	$int 0x40
}

inline fastcall dword PollSocket( ECX)
{
	$mov eax,53
	$mov ebx,2
	$int 0x40 
} 


inline fastcall dword WriteSocket( ECX, EDX, ESI)
{
	$mov eax,53
	$mov ebx,7
	$int 0x40
} 

inline fastcall int IsPortFree( ECX)
{
	$mov eax,53
	$mov ebx,9
	$int 0x40
}

unsigned int GetFreePort(int port_i)
{
	for (; port_i<65535; port_i++;)
		if (IsPortFree(port_i)==1) return port_i;
	return 0;
}
