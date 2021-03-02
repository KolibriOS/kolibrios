/* Copyright (C) 2019-2021 Logaev Maxim (turbocat2001), GPLv2 */

#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <stddef.h>
#include <ksys.h>
#include <errno.h>

// Socket Types
#define SOCK_STREAM 1
#define SOCK_DGRAM 2
#define SOCK_RAW 3
 
// IP protocols
#define IPPROTO_IP 0
#define IPPROTO_ICMP 1
#define IPPROTO_TCP 6
#define IPPROTO_UDP 17
#define IPPROTO_RAW 255
 
// IP options
#define IP_TTL 2
 
// Address families
#define AF_UNSPEC 0
#define AF_LOCAL 1
#define AF_INET  2     // Default INET=IPv4
#define AF_INET4 2     // IPv4
#define AF_INET6 10    // IPv6

#define PF_UNSPEC AF_UNSPEC
#define PF_LOCAL  AF_LOCAL
#define PF_INET4  AF_INET4
#define PF_INET6  AF_INET6
 
// internal definition
#define AI_SUPPORTED 0x40F

// for system function 76
#define API_ETH (0<<16)
#define API_IPv4 (1<<16)
#define API_ICMP (2<<16)
#define API_UDP (3<<16)
#define API_TCP (4<<16)
#define API_ARP (5<<16)
#define API_PPPOE (6<<16)

// Socket flags for user calls
#define MSG_NOFLAG 0
#define MSG_PEEK 0x02
#define MSG_DONTWAIT 0x40
 
// Socket levels
#define SOL_SOCKET 0xffff

//Socket options
#define SO_BINDTODEVICE (1<<9)
#define SO_NONBLOCK (1<<31)

#define PORT(X) (X<<8)

#pragma pack(push,1)
struct sockaddr{
    unsigned short sin_family;
    unsigned short sin_port; 
    unsigned int sin_addr;
    unsigned long long sin_zero;
}; 

typedef struct{
  unsigned int level;
  unsigned int optionname;
  unsigned int optlenght;
  unsigned char options;
}optstruct;
#pragma pack(pop)

static inline 
void _conv_socket_err(){
    switch(errno){
        case 1:  errno = ENOBUFS; break;
        case 2:  errno = EINPROGRESS; break;
        case 4:  errno = EOPNOTSUPP; break; 
        case 6:  errno = EWOULDBLOCK; break;
        case 9:  errno = ENOTCONN; break;
        case 10: errno = EALREADY; break;
        case 11: errno = EINVAL; break;
        case 12: errno = EMSGSIZE; break;
        case 18: errno = ENOMEM; break;
        case 20: errno = EADDRINUSE; break;
        case 61: errno = ECONNREFUSED; break;
        case 52: errno = ECONNRESET; break;
        case 56: errno = EISCONN; break;
        case 60: errno = ETIMEDOUT; break;
        case 54: errno = ECONNABORTED; break;
        default: errno = 0; break;
    }
}

static inline 
int socket(int domain, int type, int protocol)
{
    int socket;
    asm_inline(
        "int $0x40"
        :"=b"(errno), "=a"(socket)
        :"a"(75), "b"(0), "c"(domain), "d"(type), "S"(protocol)
    );
    _conv_socket_err();
    return socket;
}

static inline 
int close(int socket)
{
    int status;
    asm_inline(
        "int $0x40"
        :"=b"(errno), "=a"(status)
        :"a"(75), "b"(1), "c"(socket)
    );
    _conv_socket_err();
    return status;
}

static inline 
int bind(int socket, const struct sockaddr *addres, int addres_len)
{
    int status;
    asm_inline(
        "int $0x40"
        :"=b"(errno), "=a"(status)
        :"a"(75), "b"(2), "c"(socket), "d"(addres), "S"(addres_len)
    );
    _conv_socket_err();
    return status;
}

static inline 
int listen(int socket, int backlog)
{
    int status;
    asm_inline(
        "int $0x40"
        :"=b"(errno), "=a"(status)
        :"a"(75), "b"(3), "c"(socket), "d"(backlog)
    );
    _conv_socket_err();
    return status;
}

static inline 
int connect(int socket, const struct sockaddr* address, int socket_len)
{
    int status;
    asm_inline(
        "int $0x40"
        :"=b"(errno), "=a"(status)
        :"a"(75), "b"(4), "c"(socket), "d"(address), "S"(socket_len)
    );
    _conv_socket_err();
    return status;
}

static inline int 
accept(int socket, const struct sockaddr *address, int address_len)
{
    int new_socket;
    asm_inline(
        "int $0x40"
        :"=b"(errno), "=a"(new_socket)
        :"a"(75), "b"(5), "c"(socket), "d"(address), "S"(address_len)
    );
    _conv_socket_err();
    return new_socket;
}

static inline 
int send(int socket, const void *message, size_t msg_len, int flag)
{
    int status;
    asm_inline(
        "int $0x40"
        :"=b"(errno), "=a"(status)
        :"a"(75), "b"(6), "c"(socket), "d"(message), "S"(msg_len), "D"(flag)
    );
    _conv_socket_err();
    return status;
}

static inline 
int recv(int socket, void *buffer, size_t buff_len, int flag)
{
    int status;
    asm_inline(
        "int $0x40"
        :"=b"(errno), "=a"(status)
        :"a"(75), "b"(7), "c"(socket), "d"(buffer), "S"(buff_len), "D"(flag)
    );
    _conv_socket_err();
    return status;
}

static inline 
int setsockopt(int socket,const optstruct* opt)
{
    int status;
    asm_inline(
        "int $0x40"
        :"=b"(errno), "=a"(status)
        :"a"(75), "b"(8), "c"(socket),"d"(opt)
    );
    _conv_socket_err();
    return status;
}

static inline 
int getsockopt(int socket, optstruct* opt)
{
    int status; 
    asm_inline(
        "int $0x40"
        :"=b"(errno), "=a"(status)
        :"a"(75), "b"(9), "c"(socket),"d"(opt)
    );
    _conv_socket_err();
    return status;
}

static inline 
int socketpair(int *socket1, int *socket2)
{
   asm_inline(
        "int $0x40"
        :"=b"(*socket2), "=a"(*socket1)
        :"a"(75), "b"(10)
    ); 
    errno=*socket2;
    _conv_socket_err();
    return *socket1;
}

#endif //_SOCKET_H_
