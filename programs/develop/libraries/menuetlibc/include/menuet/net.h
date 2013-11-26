#ifndef __MENUET_NET_H
#define __MENUET_NET_H

#ifdef __cplusplus
extern "C" {
#endif

#include<menuet/os.h>

  
// System functions
#define _SOCKETF 75

// Socket subfunctions
#define _OPENSF    0
#define _CLOSESF   1 
#define _BINDSF    2
#define _LISTENSF  3
#define _CONNECTSF 4
#define _ACCEPTSF  5
#define _SENDSF    6
#define _RECEIVESF 7
  
// Socket types
#define SOCK_STREAM 1
#define SOCK_DGRAM  2
#define SOCK_RAW    3
  
// Socket options
#define SO_NONBLOCK 1 << 31
  
// IP protocols
#define IPPROTO_IP   0
#define IPPROTO_ICMP 1
#define IPPROTO_TCP  6
#define IPPROTO_UDP  17
  
// Address families
#define AF_UNSPEC 0
#define AF_LOCAL  1
#define AF_INET   2  // IPv4
#define AF_INET6  28 // IPv6 (not supported yet)

#define PF_UNSPEC AF_UNSPEC
#define PF_LOCAL  AF_LOCAL
#define PF_INET  AF_INET
#define PF_INET6  AF_INET6
  
// Flags for addrinfo
#define AI_PASSIVE     1
#define AI_CANONNAME   2
#define AI_NUMERICHOST 4
#define AI_NUMERICSERV 8
#define AI_ADDRCONFIG  0x400  
  
// Internal definition
#define AI_SUPPORTED 0x40F
  
// For system function 76
#define API_ETH   0 << 16
#define API_IPv4  1 << 16
#define API_ICMP  2 << 16
#define API_UDP   3 << 16
#define API_TCP   4 << 16
#define API_ARP   5 << 16
#define API_PPPOE 6 << 16
  
// Socket flags for user calls
#define MSG_PEEK     0x02
#define MSG_DONTWAIT 0x40
  
  
struct in_addr {
    unsigned int s_addr;
};

struct sockaddr {
    unsigned short    sa_family;    // address family, AF_xxx
    char              sa_data[14];  // 14 bytes of protocol address
}; 

struct sockaddr_in { // IPv4 only, we need sockaddr_in6 for IPv6
    short int          sin_family;  // Address family, AF_INET
    unsigned short int sin_port;    // Port number
    struct in_addr     sin_addr;    // Internet address
    unsigned char      sin_zero[8]; // Same size as struct sockaddr
};
  
struct addrinfo {
    int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
    int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
    int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
    int              ai_protocol;  // use 0 for "any"
    int              ai_addrlen;   // size of ai_addr in bytes
    struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
    char            *ai_canonname; // full canonical hostname
    struct addrinfo *ai_next;      // linked list, next node
};
  
#define EAI_ADDRFAMILY 1
#define EAI_AGAIN      2
#define EAI_BADFLAGS   3
#define EAI_FAIL       4
#define EAI_FAMILY     5
#define EAI_MEMORY     6
#define EAI_NONAME     8
#define EAI_SERVICE    9
#define EAI_SOCKTYPE   10
#define EAI_BADHINTS   12
#define EAI_PROTOCOL   13
#define EAI_OVERFLOW   14  

// Socket error codes
// Error Codes
#define ENOBUFS       1
#define EINPROGRESS   2
#define EOPNOTSUPP    4
#define EWOULDBLOCK   6
#define ENOTCONN      9
#define EALREADY      10
#define EINVAL        11
#define EMSGSIZE      12
#define ENOMEM        18
#define EADDRINUSE    20
#define ECONNREFUSED  61
#define ECONNRESET    52
#define EISCONN       56
#define ETIMEDOUT     60
#define ECONNABORTED  53


int socket(int domain, int type, int protocol); 
int closesocket(int s);
int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
int listen(int s, int backlog);
int connect(int sockfd, const struct sockaddr *serv_addr, int addrlen);
int accept(int s, struct sockaddr *addr, int *addrlen);
int send(int s, const void *buf, int len, int flags);
int recv(int sockfd, void *buf, int len, int flags);

// extern from src/libc/menuetos/netowrk.c
#define __stdcall __attribute__((stdcall))
extern void NETWORK_INIT();
extern void (* __stdcall freeaddrinfo)(struct addrinfo* ai);
extern int (* __stdcall getaddrinfo)( const char* hostname, const char* servname, const struct addrinfo* hints, struct addrinfo **res);
extern char * (* __stdcall inet_ntoa)(struct in_addr in);
extern unsigned long (* __stdcall inet_addr)( const char* hostname);


// Old stuff
//---------------------------------------------  

#define __NET_stack_rd_cfg_word	0
#define __NET_stack_get_ip	1
#define __NET_stack_wr_cfg_word	2
#define __NET_stack_put_ip	3

#define __NET_sock_open_UDP	0
#define __NET_sock_open_TCP	5
#define __NET_sock_close_UDP	1
#define __NET_sock_close_TCP	8
#define __NET_sock_poll		2
#define __NET_sock_read		3
#define __NET_sock_write_UDP	4
#define __NET_sock_get_status	6
#define __NET_sock_write_TCP	7
#define __NET_sock_check_port   9

#define __NET_socket		53
#define __NET_stack		52

int __menuet__get_stack_config_word(void);
__u32 __menuet__get_my_IP(void);
void __menuet__set_stack_config_word(int cfg);
void __menuet__set_my_IP(__u32 my_IP);

__u32 __menuet__open_UDP_socket(__u32 local_port,__u32 remote_port,__u32 remote_ip);
__u32 __menuet__open_TCP_socket(__u32 local_port,__u32 remote_port,__u32 remote_ip,int mode);
int __menuet__close_UDP_socket(int socket);
int __menuet__close_TCP_socket(int socket);
int __menuet__poll_socket(int sock);
int __menuet__read_socket(int sock,__u8 * return_data);
int __menuet__get_TCP_socket_status(int sock);
int __menuet__write_UDP_socket(int sock,int count,void * buffer);
int __menuet__write_TCP_socket(int sock,int count,void * buffer);
int __menuet__check_net_port_availability(int port);

/* Values returned by __menuet__check_net_port_availability */
#define PORT_AVAILABLE		(1)
#define PORT_UNAVAILABLE	(0)

#define NET_OP_OK		((int)(0))
#define NET_OP_ERR		((int)(-1))

/* These are socket modes */
#define SOCKET_PASSIVE		0
#define SOCKET_ACTIVE		1

/* These belong to socket status */
#define TCB_LISTEN		1
#define TCB_SYN_SENT		2
#define TCB_SYN_RECEIVED	3
#define TCB_ESTABLISHED		4
#define TCB_FIN_WAIT_1		5
#define TCB_FIN_WAIT_2		6
#define TCB_CLOSE_WAIT		7
#define TCB_CLOSING		8
#define TCB_LAST_ACK		9
#define TCB_TIME_AWAIT		10
#define TCB_CLOSED		11

#ifdef __cplusplus
}
#endif

#endif
