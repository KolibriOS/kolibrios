#include <sys/socket.h>

static void _conv_socket_err(){
    switch(errno){
        case 1:  errno = ENOBUFS; break;
        case 2:  errno = EINPROGRESS; break;
        case 4:  errno = EOPNOTSUPP; break; 
        case 6:  errno = EWOULDBLOCK; break;
        case 9:  errno = ENOTCONN; break;
        case 10: errno = EALREADY; break;
        case 11: errno = EINVAL; break;
        case 12: errno = EMSGSIZE; break;
        case 18: errno = ENOMEM; break;
        case 20: errno = EADDRINUSE; break;
        case 61: errno = ECONNREFUSED; break;
        case 52: errno = ECONNRESET; break;
        case 56: errno = EISCONN; break;
        case 60: errno = ETIMEDOUT; break;
        case 54: errno = ECONNABORTED; break;
        default: errno = 0; break;
    }
}

int socket(int domain, int type, int protocol)
{
    int socket;
    asm_inline(
        "int $0x40"
        :"=b"(errno), "=a"(socket)
        :"a"(75), "b"(0), "c"(domain), "d"(type), "S"(protocol)
    );
    _conv_socket_err();
    return socket;
}
 
int close(int socket)
{
    int status;
    asm_inline(
        "int $0x40"
        :"=b"(errno), "=a"(status)
        :"a"(75), "b"(1), "c"(socket)
    );
    _conv_socket_err();
    return status;
}
 
int bind(int socket, const struct sockaddr *addres, int addres_len)
{
    int status;
    asm_inline(
        "int $0x40"
        :"=b"(errno), "=a"(status)
        :"a"(75), "b"(2), "c"(socket), "d"(addres), "S"(addres_len)
    );
    _conv_socket_err();
    return status;
}
 
int listen(int socket, int backlog)
{
    int status;
    asm_inline(
        "int $0x40"
        :"=b"(errno), "=a"(status)
        :"a"(75), "b"(3), "c"(socket), "d"(backlog)
    );
    _conv_socket_err();
    return status;
}

int connect(int socket, const struct sockaddr* address, int socket_len)
{
    int status;
    asm_inline(
        "int $0x40"
        :"=b"(errno), "=a"(status)
        :"a"(75), "b"(4), "c"(socket), "d"(address), "S"(socket_len)
    );
    _conv_socket_err();
    return status;
}

int accept(int socket, const struct sockaddr *address, int address_len)
{
    int new_socket;
    asm_inline(
        "int $0x40"
        :"=b"(errno), "=a"(new_socket)
        :"a"(75), "b"(5), "c"(socket), "d"(address), "S"(address_len)
    );
    _conv_socket_err();
    return new_socket;
}

int send(int socket, const void *message, size_t msg_len, int flag)
{
    int status;
    asm_inline(
        "int $0x40"
        :"=b"(errno), "=a"(status)
        :"a"(75), "b"(6), "c"(socket), "d"(message), "S"(msg_len), "D"(flag)
    );
    _conv_socket_err();
    return status;
}
 
int recv(int socket, void *buffer, size_t buff_len, int flag)
{
    int status;
    asm_inline(
        "int $0x40"
        :"=b"(errno), "=a"(status)
        :"a"(75), "b"(7), "c"(socket), "d"(buffer), "S"(buff_len), "D"(flag)
    );
    _conv_socket_err();
    return status;
}

int setsockopt(int socket,const optstruct* opt)
{
    int status;
    asm_inline(
        "int $0x40"
        :"=b"(errno), "=a"(status)
        :"a"(75), "b"(8), "c"(socket),"d"(opt)
    );
    _conv_socket_err();
    return status;
}
 
int getsockopt(int socket, optstruct* opt)
{
    int status; 
    asm_inline(
        "int $0x40"
        :"=b"(errno), "=a"(status)
        :"a"(75), "b"(9), "c"(socket),"d"(opt)
    );
    _conv_socket_err();
    return status;
}

int socketpair(int *socket1, int *socket2)
{
   asm_inline(
        "int $0x40"
        :"=b"(*socket2), "=a"(*socket1)
        :"a"(75), "b"(10)
    ); 
    errno=*socket2;
    _conv_socket_err();
    return *socket1;
}
 
