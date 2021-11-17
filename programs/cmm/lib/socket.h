#ifndef INCLUDE_SOCKET_H
#define INCLUDE_SOCKET_H
#print "[include <socket.h>]\n"

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

#define SOCK_STREAM 1
#define	SOCK_DGRAM 2

#define AF_INET4 2

#define MSG_PEEK 0x02
#define MSG_DONTWAIT 0x40

struct 	SockAddr
{
  word    sin_family;
  char    data[14];
};

// ecx = domain
// edx = type
// esi = protocol
inline fastcall dword socket_open(ECX, EDX, ESI)
{
	$mov 	eax, 75
	$mov 	bl, 0
	$int 	0x40
}

// ecx = socket number
inline fastcall dword socket_close(ECX)
{
	$mov 	eax, 75
	$mov 	bl, 1
	$int 	0x40
}

// ecx = socket number
// edx = pointer to sockaddr structure
// esi = length of sockaddr structure
inline fastcall dword socket_bind(ECX, EDX, ESI)
{
	$mov 	eax, 75
	$mov 	bl, 2
	$int 	0x40
}

// ecx = socket number
// edx = backlog
inline fastcall dword socket_listen(ECX, EDX)
{
	$mov 	eax, 75
	$mov 	bl, 3
	$int 	0x40
}

// ecx = socket number
// edx = pointer to sockaddr structure
// esi = length of sockaddr structure
inline fastcall dword socket_connect(ECX, EDX, ESI)
{
	$mov 	eax, 75
	$mov 	bl, 4
	$int 	0x40
}

// ecx = socket number
// edx = pointer to sockaddr structure
// esi = length of sockaddr structure
inline fastcall dword socket_accept(ECX, EDX, ESI)
{
	$mov 	eax, 75
	$mov 	bl, 5
	$int 	0x40
}

// ecx = socket number
// edx = pointer to buffer
// esi = length of buffer
// edi = flags
inline fastcall dword socket_send(ECX, EDX, ESI, EDI)
{
	$mov 	eax, 75
	$mov 	bl, 6
	$int 	0x40
}

// ecx = socket number
// edx = pointer to buffer
// esi = length of buffer
// edi = flags
inline fastcall dword socket_receive(ECX, EDX, ESI, EDI)
{
	$mov 	eax, 75
	$mov 	bl, 7
	$int 	0x40
}

#endif