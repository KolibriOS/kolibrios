#ifndef __NETWORK_H
#define __NETWORK_H

#include <sys/socket.h>

/// @brief EAI errors codes
enum EAI
{
    EAI_ADDRFAMILY = 1,
    EAI_AGAIN = 2,
    EAI_BADFLAGS = 3,
    EAI_FAIL = 4,
    EAI_FAMILY = 5,
    EAI_MEMORY = 6,
    EAI_NONAME = 8,
    EAI_SERVICE = 9,
    EAI_SOCKTYPE = 10,
    EAI_BADHINTS = 12,
    EAI_PROTOCOL = 13,
    EAI_OVERFLOW = 14
};

/// @brief Flags for addrinfo
enum AddresInfoFlags
{
    AI_PASSIVE = 1,
    AI_CANONNAME = 2,
    AI_NUMERICHOST = 4,
    AI_NUMERICSERV = 8,
    AI_ADDRCONFIG = 0x400
};

#pragma pack(push, 1)
struct ARP_entry {
    unsigned int IP;
    unsigned char MAC[6];
    unsigned short status;
    unsigned short TTL;
};
#pragma pack(pop)

#pragma pack(push, 1)  
struct addrinfo {
    /// @brief bitmask of AI_*
    int     ai_flags;
    /// @brief PF_*
    int     ai_family;
    /// @brief SOCK_*
    int     ai_socktype;
    /// @brief 0 or IPPROTO_*
    int     ai_protocol;
    /// @brief length of ai_addr
    int     ai_addrlen;
    char   *ai_canonname;
    struct sockaddr *ai_addr;
    struct addrinfo *ai_next;
};  
#pragma pack(pop)
/// @brief Inilizate networklib
/// @return -1 if unsuccessful
extern int networklib_init();

/// @brief Convert the string from standard IPv4 dotted notation to integer IP addr
/// @param hostname host name
/// @return IP addres on success 
/// @return -1 on error
extern int (*inet_addr)(const char* hostname) __attribute__ ((stdcall));

/// @brief Convert the Internet host address to standard IPv4 dotted notation.
/// @param ip_addr host address
/// @return pointer to resulting string
extern char* (*inet_ntoa)(int ip_addr) __attribute__ ((stdcall));

/// @brief Get a list of IP addresses and port numbers for given host and service
/// @param hostname host name
/// @param servname service name (decimal number for now)
/// @param hints hints for socket type
/// @param res pointer to result
/// @return 0 on success
/// @return 
extern int (*getaddrinfo)(char* hostname, char *servname, struct addrinfo* hints, struct addrinfo** res) __attribute__ ((stdcall));

/// @brief Free one or more addrinfo structures returned by getaddrinfo.
/// @param ai head of list of structures
extern void (*freeaddrinfo)(struct addrinfo* ai) __attribute__ ((stdcall));

#endif
