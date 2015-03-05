#define SOCK_STREAM 1
#define	SOCK_DGRAM 2

#define AF_INET4 2 

#define MSG_PEEK 0x02
#define MSG_DONTWAIT 0x40

dword errorcode;

struct 	sockaddr_in{
        word    sin_family; 
        word    sin_port; 
        dword   sin_addr;
        char    padding[8];
};  

inline fastcall dword Socket(ECX, EDX, ESI)
{
	$push	ebx
	$mov 	eax, 75
	$mov 	ebx, 0
	$int 	0x40
	errorcode = EBX;
	$pop	ebx
} 

inline fastcall dword Close(ECX)
{
	$push	ebx
	$mov 	eax, 75
	$mov 	ebx, 1
	$int 	0x40
	errorcode = EBX;
	$pop	ebx	
}

inline fastcall dword Bind(ECX, EDX, ESI)
{
	$push	ebx
	$mov 	eax, 75
	$mov 	ebx, 2
	$int 	0x40
	errorcode = EBX;
	$pop	ebx
}

inline fastcall dword Listen(ECX, EDX)
{
	$push	ebx
	$mov 	eax, 75
	$mov 	ebx, 3
	$int 	0x40
	errorcode = EBX;
	$pop	ebx
}

inline fastcall dword Connect(ECX, EDX, ESI)
{
	$push	ebx
	$mov 	eax, 75
	$mov 	ebx, 4
	$int 	0x40
	errorcode = EBX;
	$pop	ebx
}

inline fastcall dword Accept(ECX, EDX, ESI)
{
	$push	ebx
	$mov 	eax, 75
	$mov 	ebx, 5
	$int 	0x40
	errorcode = EBX;
	$pop	ebx
}

inline fastcall dword Send(ECX, EDX, ESI, EDI)
{
	$push	ebx
	$mov 	eax, 75
	$mov 	ebx, 6
	$int 	0x40
	errorcode = EBX;
	$pop	ebx
}

inline fastcall dword Receive(ECX, EDX, ESI, EDI)
{
	$push	ebx
	$mov 	eax, 75
	$mov 	ebx, 7
	$int 	0x40
	errorcode = EBX;
	$pop	ebx
}
