#include "kosnet/socket.h"

int err_code = 0;

int socket(int domain, int type, int protocol)
{
    int socket;
    asm volatile(
    "int $0x40"
    :"=b"(err_code), "=a"(socket)
    :"a"(75), "b"(0), "c"(domain), "d"(type), "S"(protocol)
    );
    return socket;
}
 
int closesocket(int socket)
{
    int status;
    asm volatile(
    "int $0x40"
    :"=b"(err_code), "=a"(status)
    :"a"(75), "b"(1), "c"(socket)
    );
    return status;
}

int bind(int socket, const sockaddr *addres, int addres_len)
{
    int status;
    asm volatile(
    "int $0x40"
    :"=b"(err_code), "=a"(status)
    :"a"(75), "b"(2), "c"(socket), "d"(addres), "S"(addres_len)
    );
    return status;
}
 
int listen(int socket, int backlog)
{
    int status;
    asm volatile(
    "int $0x40"
    :"=b"(err_code), "=a"(status)
    :"a"(75), "b"(3), "c"(socket), "d"(backlog)
    );
    return status;
}
 
int connect(int socket,const sockaddr* address, int socket_len)
{
    int status;
    asm volatile(
    "int $0x40"
    :"=b"(err_code), "=a"(status)
    :"a"(75), "b"(4), "c"(socket), "d"(address), "S"(socket_len)
    );
    return status;
}
 
int accept(int socket, const sockaddr *address, int address_len)
{
    int new_socket;
    asm volatile(
    "int $0x40"
    :"=b"(err_code), "=a"(new_socket)
    :"a"(75), "b"(5), "c"(socket), "d"(address), "S"(address_len)
    );
    return new_socket;
}
 
int send(int socket, const void *message, size_t msg_len, int flag)
{
    int status;
    asm volatile(
    "int $0x40"
    :"=b"(err_code), "=a"(status)
    :"a"(75), "b"(6), "c"(socket), "d"(message), "S"(msg_len), "D"(flag)
    );
    return status;
}
 
int recv(int socket, void *buffer, size_t buff_len, int flag)
{
    int status;
    asm volatile(
    "int $0x40"
    :"=b"(err_code), "=a"(status)
    :"a"(75), "b"(7), "c"(socket), "d"(buffer), "S"(buff_len), "D"(flag)
    );
    return status;
}
 
int setsockopt(int socket,const optstruct* opt)
{
    int status;
    asm volatile(
        "int $0x40"
        :"=b"(err_code), "=a"(status)
        :"a"(75), "b"(8), "c"(socket),"d"(opt)
    );
    return status;
}
 
int getsockopt(int socket, optstruct* opt)
{
    int status;
    asm volatile(
        "int $0x40"
        :"=b"(err_code), "=a"(status)
        :"a"(75), "b"(9), "c"(socket),"d"(opt)
    );
    return status;
}
 
int socketpair(int *socket1, int *socket2)
{
    asm volatile(
        "int $0x40"
        :"=b"(*socket2), "=a"(*socket1)
        :"a"(75), "b"(10)
    );
    err_code=*socket2;
    return *socket1;
}