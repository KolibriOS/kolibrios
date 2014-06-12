#include<menuet/network.hpp>

IP_Address::IP_Address(unsigned long addr)
{
 this_ip_inet_fmt=addr;
}

IP_Address::IP_Address(__u8 p1,__u8 p2,__u8 p3,__u8 p4)
{
 this_ip_inet_fmt=p4;
 this_ip_inet_fmt<<=8;
 this_ip_inet_fmt|=p3;
 this_ip_inet_fmt<<=8;
 this_ip_inet_fmt|=p2;
 this_ip_inet_fmt<<=8;
 this_ip_inet_fmt|=p1;
}

IP_Address::~IP_Address()
{
}

unsigned long IP_Address::operator = (IP_Address& a)
{
 return a.this_ip_inet_fmt;
}

IP_Address& IP_Address::operator = (unsigned long a)
{
 this->this_ip_inet_fmt=a;
 return *this;
}
