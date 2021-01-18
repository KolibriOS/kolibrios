/* Copyright (C) 2019-2021 Logaev Maxim (turbocat2001), GPLv3 */

#include <net/socket.h>

int socket(int domain, int type, int protocol)
{
    asm volatile(
    "int $0x40"
    :"=b"(errno)
    :"a"(75), "b"(0), "c"(domain), "d"(type), "S"(protocol)
    );
}

int close(int socket)
{
    asm volatile(
    "int $0x40"
    :"=b"(errno)
    :"a"(75), "b"(1), "c"(socket)
    );
}
int bind(int socket, const struct sockaddr *addres, int addres_len)
{
    asm volatile(
    "int $0x40"
    :"=b"(errno)
    :"a"(75), "b"(2), "c"(socket), "d"(addres), "S"(addres_len)
    );
}

int listen(int socket, int backlog)
{
    asm volatile(
    "int $0x40"
    :"=b"(errno)
    :"a"(75), "b"(3), "c"(socket), "d"(backlog)
    );
}

int connect(int socket,const struct sockaddr* address, int socket_len)
{
    asm volatile(
    "int $0x40"
    :"=b"(errno)
    :"a"(75), "b"(4), "c"(socket), "d"(address), "S"(socket_len)
    );
}

int accept(int socket, const struct sockaddr *address, int address_len)
{
    asm volatile(
    "int $0x40"
    :"=b"(errno)
    :"a"(75), "b"(5), "c"(socket), "d"(address), "S"(address_len)
    );   
}

int send(int socket, const void *message, size_t msg_len, int flag)
{
    asm volatile(
    "int $0x40"
    :"=b"(errno)
    :"a"(75), "b"(6), "c"(socket), "d"(message), "S"(msg_len), "D"(flag)
    );   
}

int recv(int socket, void *buffer, size_t buff_len, int flag)
{
    asm volatile(
    "int $0x40"
    :"=b"(errno)
    :"a"(75), "b"(7), "c"(socket), "d"(buffer), "S"(buff_len), "D"(flag)
    );  
}

int setsockopt(int socket,const optstruct* opt)
{
    asm volatile(
        "int $0x40"
        :"=b"(errno)
        :"a"(75), "b"(8), "c"(socket),"d"(opt)
    );
}

int getsockopt(int socket, optstruct* opt)
{
    asm volatile(
        "int $0x40"
        :"=b"(errno)
        :"a"(75), "b"(9), "c"(socket),"d"(opt)
    );
}

int socketpair(int *sock1, int *sock2)
{
    asm volatile(
        "int $0x40"
        :"=b"(*sock2), "=a"(*sock1) 
        :"a"(75), "b"(10)
    );
    errno = *sock2;
    return *sock1;
}
