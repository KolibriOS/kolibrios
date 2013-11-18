#include <menuet/net.h>


unsigned long inet_addr(char *cp)
{
  // Adapted from here http://stackoverflow.com/a/1684635
  unsigned long __ret;
  unsigned int c1,c2,c3,c4;
  sscanf(cp, "%d.%d.%d.%d", &c1,&c2,&c3,&c4);
  __ret = (unsigned long)c4+c3*256+c2*256*256+c1*256*256*256;
  return htonl(__ret);
}

int socket(int domain, int type, int protocol)
{
  int __ret;
  __asm__("int $0x40":"=a"(__ret):"a"(_SOCKETF),"b"(_OPENSF),"c"(domain),"d"(type),"S"(protocol));
  return __ret;
}

int closesocket(int s)
{
  int __ret;
  __asm__("int $0x40":"=a"(__ret):"a"(_SOCKETF),"b"(_CLOSESF),"c"(s));
  return __ret;
}


int connect(int sockfd, const struct sockaddr *serv_addr, int addrlen)
{
  int __ret;
  __asm__("int $0x40":"=a"(__ret):"a"(_SOCKETF),"b"(_CONNECTSF),"c"(sockfd),"d"(serv_addr),"S"(addrlen));
  return __ret;
}

int send(int s, const void *buf, int len, int flags)
{
  int __ret;
  __asm__("int $0x40":"=a"(__ret):"a"(_SOCKETF),"b"(_SENDSF),"c"(s),"d"(buf),"S"(len),"D"(flags));
  return __ret;
}

int recv(int sockfd, void *buf, int len, int flags)
{
  int __ret;
  __asm__("int $0x40":"=a"(__ret):"a"(_SOCKETF),"b"(_RECEIVESF),"c"(sockfd),"d"(buf),"S"(len),"D"(flags));
  return __ret;
}



// --------------------------------------------------------------

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
