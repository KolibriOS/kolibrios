#ifndef __MENUET_NETWORK_HPP__
#define __MENUET_NETWORK_HPP__

#include<menuet/net.h>

class IP_Address
{
public:
 unsigned long this_ip_inet_fmt;
 IP_Address(unsigned long);
 IP_Address(__u8,__u8,__u8,__u8);
 ~IP_Address();
 unsigned long operator = (IP_Address&);
 IP_Address& operator = (unsigned long);
};

class UDP_Socket
{
public:
 UDP_Socket(__u32 local_port,__u32 remote_port,__u32 remote_ip,bool close_on_delete);
 virtual ~UDP_Socket();
 virtual int Open();
 virtual int Close();
 virtual int Read(__u8 * data);
 virtual int Write(int count,void * data);
 virtual int Poll();
protected:
 unsigned long p[3];
 bool f;
 int sock;
};

#endif
