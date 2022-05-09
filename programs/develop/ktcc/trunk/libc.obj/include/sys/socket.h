/* Copyright (C) 2019-2021 Logaev Maxim (turbocat2001), GPLv2 */

#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <errno.h>
#include <stddef.h>
#include <sys/ksys.h>

// Socket Types
#define SOCK_STREAM 1
#define SOCK_DGRAM  2
#define SOCK_RAW    3

// IP protocols
#define IPPROTO_IP   0
#define IPPROTO_ICMP 1
#define IPPROTO_TCP  6
#define IPPROTO_UDP  17
#define IPPROTO_RAW  255

// IP options
#define IP_TTL 2

// Address families
#define AF_UNSPEC 0
#define AF_LOCAL  1
#define AF_INET   2  // Default INET=IPv4
#define AF_INET4  2  // IPv4
#define AF_INET6  10 // IPv6

#define PF_UNSPEC AF_UNSPEC
#define PF_LOCAL  AF_LOCAL
#define PF_INET4  AF_INET4
#define PF_INET6  AF_INET6

// internal definition
#define AI_SUPPORTED 0x40F

// for system function 76
#define API_ETH   (0 << 16)
#define API_IPv4  (1 << 16)
#define API_ICMP  (2 << 16)
#define API_UDP   (3 << 16)
#define API_TCP   (4 << 16)
#define API_ARP   (5 << 16)
#define API_PPPOE (6 << 16)

// Socket flags for user calls
#define MSG_NOFLAG   0
#define MSG_PEEK     0x02
#define MSG_DONTWAIT 0x40

// Socket levels
#define SOL_SOCKET 0xffff

// Socket options
#define SO_BINDTODEVICE (1 << 9)
#define SO_NONBLOCK     (1 << 31)

#define PORT(X) (X << 8)

#pragma pack(push, 1)
struct sockaddr {
    unsigned short sin_family;
    unsigned short sin_port;
    unsigned int sin_addr;
    unsigned long long sin_zero;
};

typedef struct {
    unsigned int level;
    unsigned int optionname;
    unsigned int optlenght;
    unsigned char options;
} optstruct;
#pragma pack(pop)

DLLAPI int socket(int domain, int type, int protocol);
DLLAPI int close(int socket);
DLLAPI int bind(int socket, const struct sockaddr* addres, int addres_len);
DLLAPI int listen(int socket, int backlog);
DLLAPI int connect(int socket, const struct sockaddr* address, int socket_len);
DLLAPI int accept(int socket, const struct sockaddr* address, int address_len);
DLLAPI int send(int socket, const void* message, size_t msg_len, int flag);
DLLAPI int recv(int socket, void* buffer, size_t buff_len, int flag);
DLLAPI int setsockopt(int socket, const optstruct* opt);
DLLAPI int getsockopt(int socket, optstruct* opt);
DLLAPI int socketpair(int* socket1, int* socket2);

#endif //_SOCKET_H_