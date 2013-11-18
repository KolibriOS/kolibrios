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
#define SO_NONBLOCK ((long)(1))
  
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
#define PF_INET4  AF_INET4
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
#define API_ETH   ((int)(0))
#define API_IPv4  ((int)(1))
#define API_ICMP  ((int)(2))
#define API_UDP   ((int)(3))
#define API_TCP   ((int)(4))
#define API_ARP   ((int)(5))
#define API_PPPOE ((int)(6))
  
// Socket flags for user calls
#define MSG_PEEK     0x02
#define MSG_DONTWAIT 0x40
  
  
struct in_addr {
  unsigned long s_addr;
};

struct sockaddr {
    unsigned short    sa_family;    // address family, AF_xxx
    char              sa_data[14];  // 14 bytes of protocol address
}; 

struct sockaddr_in {
        short sin_family;  // sa_family_t
        unsigned short sin_port;  // in_port_t
        struct in_addr sin_addr;
        char sin_zero[8];
};
  
struct addrinfo {
        int ai_flags;  // bitmask of AI_*
        int longai_family;  // PF_*
        int ai_socktype;  //SOCK_*
        int ai_protocol;  // 0 or IPPROTO_*
        int ai_addrlen;  // length of ai_addr
        char *ai_canonname;
        struct sockaddr *ai_addr;  // struct sockaddr*
        struct addrinfo *ai_next;  // struct addrinfo*
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


unsigned long inet_addr(char *cp);
int socket(int domain, int type, int protocol); 
int close_socket(int s);
int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
int listen(int s, int backlog);
int connect(int sockfd, const struct sockaddr *serv_addr, int addrlen);
int accept(int s, struct sockaddr *addr, int *addrlen);
int send(int s, const void *buf, int len, int flags);
int recv(int sockfd, void *buf, int len, int flags);
// Review int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen);
// Review int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen);
// Add socketpair()


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
