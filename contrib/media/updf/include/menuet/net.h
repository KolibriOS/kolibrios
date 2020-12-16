#ifndef __MENUET_NET_H
#define __MENUET_NET_H

#ifdef __cplusplus
extern "C" {
#endif

#include<menuet/os.h>

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
