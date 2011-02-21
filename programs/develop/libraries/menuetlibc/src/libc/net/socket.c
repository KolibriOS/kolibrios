#include<menuet/net.h>

__u32 __menuet__open_UDP_socket(__u32 local_port,__u32 remote_port,__u32 remote_ip)
{
 __u32 __ret;
 __asm__("int $0x40":"=a"(__ret):"a"(__NET_socket),"b"(__NET_sock_open_UDP),
  "c"(local_port),"d"(remote_port),"S"(remote_ip));
 return __ret;
}

__u32 __menuet__open_TCP_socket(__u32 local_port,__u32 remote_port,__u32 remote_ip,int mode)
{
 __u32 __ret;
 __asm__("int $0x40":"=a"(__ret):"a"(__NET_socket),"b"(__NET_sock_open_TCP),
  "c"(local_port),"d"(remote_port),"S"(remote_ip),"D"(mode));
 return __ret;
}

int __menuet__close_UDP_socket(int socket)
{
 int __ret;
 __asm__("int $0x40":"=a"(__ret):"a"(__NET_socket),"b"(__NET_sock_close_UDP),
  "c"(socket));
 return __ret;
}

int __menuet__close_TCP_socket(int socket)
{
 int __ret;
 __asm__("int $0x40":"=a"(__ret):"a"(__NET_socket),"b"(__NET_sock_close_TCP),
  "c"(socket));
 return __ret;
}

int __menuet__poll_socket(int sock)
{ 
 int __ret;
 __asm__("int $0x40":"=a"(__ret):"a"(__NET_socket),"b"(__NET_sock_poll),
  "c"(sock));
 return __ret;
}

int __menuet__read_socket(int sock,__u8 * return_data)
{
 int data_remaining;
 __u8 data_byte;
 __asm__("int $0x40":"=a"(data_remaining),"=b"(data_byte):"a"(__NET_socket),
  "b"(__NET_sock_poll),"c"(sock));
 if(return_data) *return_data=data_byte;
 return data_remaining;
}

int __menuet__get_TCP_socket_status(int sock)
{
 int __ret;
 __asm__("int $0x40":"=a"(__ret):"a"(__NET_socket),"b"(__NET_sock_get_status),
  "c"(sock));
 return __ret;
}

int __menuet__write_UDP_socket(int sock,int count,void * buffer)
{
 int __ret;
 __asm__("int $0x40":"=a"(__ret):"a"(__NET_socket),"b"(__NET_sock_write_UDP),
  "c"(sock),"d"(count),"S"((__u32)buffer));
 return __ret;
}

int __menuet__write_TCP_socket(int sock,int count,void * buffer)
{
 int __ret;
 __asm__("int $0x40":"=a"(__ret):"a"(__NET_socket),"b"(__NET_sock_write_TCP),
  "c"(sock),"d"(count),"S"((__u32)buffer));
 return __ret;
}

int __menuet__check_net_port_availability(int port)
{
 int __ret;
 __asm__("int $0x40":"=a"(__ret):"a"(__NET_socket),"b"(__NET_sock_check_port),
  "c"(port));
 return __ret;
}
