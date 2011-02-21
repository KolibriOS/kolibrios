#include<menuet/network.hpp>

UDP_Socket::UDP_Socket(__u32 local_port,__u32 remote_port,__u32 remote_ip,bool close_on_delete)
{
 this->p[0]=local_port;
 this->p[1]=remote_port;
 this->p[2]=remote_ip;
 this->f=close_on_delete;
}

UDP_Socket::~UDP_Socket()
{
 if(this->f) this->Close();
}

int UDP_Socket::Open()
{
 return (sock=__menuet__open_UDP_socket(p[0],p[1],p[2]));
}

int UDP_Socket::Close()
{
 return __menuet__close_UDP_socket(sock);
}

int UDP_Socket::Read(__u8 * data)
{
 return __menuet__read_socket(sock,data);
}

int UDP_Socket::Write(int count,void * data)
{
 return __menuet__write_UDP_socket(sock,count,data);
}

int UDP_Socket::Poll()
{
 return __menuet__poll_socket(sock);
}
