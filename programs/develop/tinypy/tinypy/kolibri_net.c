#include <sys/socket.h>
#include <menuet/net.h>

#include "tp.h"

extern tp_obj tp_dict(TP);
extern tp_obj tp_method(TP,tp_obj self,tp_obj v(TP));
extern tp_obj tp_fnc(TP,tp_obj v(TP));
extern tp_obj tp_get(TP,tp_obj self, tp_obj k);
tp_obj tp_has(TP,tp_obj self, tp_obj k);
#define _cdecl __attribute__((cdecl))
extern int (* _cdecl con_printf)(const char* format,...);

#define PRECISION 0.000001

#define GET_SOCKET_DESCRIPTOR(_obj, _sock) do{                  \
    if (fabs(tp_has(tp, _obj, tp_string("socket")).number.val) < PRECISION)\
        tp_raise(tp_None, "Socket not open", tp_None);          \
    _sock = (__u32)(tp_get(tp, _obj, tp_string("socket")).number.val + PRECISION);\
} while(0)

/* Socket close method.
 *
 * Example: 
 * s.close() # s must be a socket object created by socket.
 *
 * Raises exception if socket was not opened. Otherwise returns True.
 */
static tp_obj kolibri_close_socket(TP)
{
    tp_obj self = TP_TYPE(TP_DICT);
    __u32  socktype;
    __u32  s;

    GET_SOCKET_DESCRIPTOR(self, s);

    socktype = (__u32)tp_get(tp, self, tp_string("type")).number.val;
    GET_SOCKET_DESCRIPTOR(self, s);
    if (socktype == SOCK_STREAM)
        __menuet__close_TCP_socket(s);
    else if (socktype == SOCK_DGRAM)
        __menuet__close_UDP_socket(s);
    return tp_True;
}

/* Socket send method.
 *
 * Example:
 * data="<html><head><title>Preved!!!</title></head><body>Example.</body></html>"
 * s.send(data)
 * or:
 * s.send(data, 20) # Send just 20 bytes
 */
static tp_obj kolibri_send(TP)
{
    tp_obj self = TP_TYPE(TP_DICT);
    tp_obj data_obj = TP_TYPE(TP_STRING);
    __u32  datalen = TP_DEFAULT(tp_False).number.val;
    __u32  socktype = (__u32)tp_get(tp, self, tp_string("type")).number.val;
    __u32  s;
    int result;

    GET_SOCKET_DESCRIPTOR(self, s);

    if (datalen < 0 || datalen > data_obj.string.len)
        datalen = data_obj.string.len;
    if (socktype == SOCK_STREAM)
        result = __menuet__write_TCP_socket(s, datalen, (void *)data_obj.string.val);
    else if (socktype == SOCK_DGRAM)
        result = __menuet__write_UDP_socket(s, datalen, (void *)data_obj.string.val);
    return tp_number(!(result != 0));
}

/* Socket recv method.
 *
 * data="<html><head><title>Preved!!!</title></head><body>Example.</body></html>"
 * s.recv(data)
 * or:
 * s.recv(data, 20) # Send just 20 bytes
 */
static tp_obj kolibri_recv(TP)
{
    tp_obj self = TP_TYPE(TP_DICT);
    __u32  datalen = TP_DEFAULT(tp_False).number.val;
    __u32  s;
    __u8 c;
    __u8 *buf, *p;
    __u32 buf_size;
    __u32 bytes_read = 0;
    int i;

    GET_SOCKET_DESCRIPTOR(self, s);

    if (datalen)
        buf_size = datalen;
    else
        buf_size = 2048;
    if (!(buf = malloc(datalen)))
        tp_raise(tp_None, "Cannot allocate buffer for received data", tp_None);
    p = buf;
    while (__menuet__read_socket(s, &c) && bytes_read < buf_size)
    {
        *p++ = c;
        bytes_read++;
        if (bytes_read >= buf_size && !datalen)
        {
            buf_size += 1024;
            buf = realloc(buf, buf_size);
        }
    }
    return tp_string_n(buf, bytes_read);
}

static void inet_pton(TP, const char *buf, int len, __u32 *addr)
{
    char *p = (char *)buf;
    int i = 0;
    __u32 val = 0;
    *addr = 0;
    while (*p && p < buf + len && i < 4)
    {
        if (*p == '.' || !*p)
        {
            if (val > 255)
                tp_raise(tp_None, "ValueError: number > 255 in IP address", tp_None);
            *addr += (val << ((i++) << 3));
            val = 0;
        }
        else
        {
            if (*p < '0' || *p > '9')
                tp_raise(tp_None, "ValueError: bad char in IP address, digit expected", tp_None);
            val = val * 10 + *p - '0';
        }
        p++;
    }
    if (!*p)
    {
        if (i == 3)
            *addr += (val << ((i++) << 3));
        else
            tp_raise(tp_None, "ValueError: bad IP address", tp_None);
    }

}

/* Converter from string presentation to binary address. */
static tp_obj kolibri_inet_pton(TP)
{
    tp_obj obj;
    __u32 addr;
    obj = TP_TYPE(TP_STRING);
    inet_pton(tp, (char *)obj.string.val, (int)obj.string.len, &addr);
    return tp_number(addr);
}

/* Socket bind method.
 *
 * In KolibriOS it just sets local address and port.
 *
 * Example:
 * s.bind('10.10.1.2', 6000) #Connects to 10.10.1.2:6000
 */
tp_obj kolibri_bind(TP)
{
    tp_obj self = TP_TYPE(TP_DICT);
    tp_obj local_addr_obj = TP_OBJ();
    __u32  local_port = (__u32)TP_TYPE(TP_NUMBER).number.val;
    __u32  local_addr;

    if (local_addr_obj.type == TP_NUMBER)
        local_addr = local_addr_obj.number.val;
    else if (local_addr_obj.type == TP_STRING)
        inet_pton(tp, (const char *)local_addr_obj.string.val, local_addr_obj.string.len, &local_addr);

    tp_set(tp, self, tp_string("local_addr"), tp_number(local_addr));
    tp_set(tp, self, tp_string("local_port"), tp_number(local_port));
    return tp_None;
}

/* Socket connect method.
 *
 * Example:
 * s.connect('10.10.1.1', 7000) #Connects to 10.10.1.1:7000
 */
tp_obj kolibri_connect(TP)
{
    tp_obj self = TP_TYPE(TP_DICT);
    tp_obj  remote_addr_obj = TP_OBJ();
    __u32  remote_addr;
    __u32  remote_port = (__u32)TP_TYPE(TP_NUMBER).number.val;
    __u32  local_port  = tp_get(tp, self, tp_string("local_port")).number.val;
    __u32  socktype = (__u32)tp_get(tp, self, tp_string("type")).number.val;
    int  s = -1; /* Socket descriptor */


    if (remote_addr_obj.type == TP_NUMBER)
        remote_addr = remote_addr_obj.number.val;
    else if (remote_addr_obj.type == TP_STRING)
        inet_pton(tp, (const char *)remote_addr_obj.string.val, remote_addr_obj.string.len, &remote_addr);

    if (socktype == SOCK_STREAM)
        s = __menuet__open_TCP_socket(local_port, remote_port, remote_addr, 1);
    else if (socktype == SOCK_DGRAM)
        s = __menuet__open_UDP_socket(local_port, remote_port, remote_addr);
    if (s >= 0)
    {
        tp_set(tp, self, tp_string("socket"), tp_number(s));
        return tp_True;
    }
    else
        return tp_False;
}

/* Socket listen method.
 * 
 * Example:
 * s.listen('10.10.1.1', 5000)
 */
tp_obj kolibri_listen(TP)
{
    tp_obj self = TP_TYPE(TP_DICT);
    tp_obj remote_addr_obj = TP_OBJ();
    __u32  remote_addr;
    __u32  remote_port = (__u32)TP_TYPE(TP_NUMBER).number.val;
    __u32  local_port  = tp_get(tp, self, tp_string("local_port")).number.val;
    __u32  socktype = (__u32)tp_get(tp, self, tp_string("type")).number.val;
    int  s = -1; /* Socket descriptor */

    if (socktype != SOCK_STREAM)
        tp_raise(tp_None, "IOError: attempt to listen on non-TCP socket", tp_None);

    if (remote_addr_obj.type == TP_NUMBER)
        remote_addr = remote_addr_obj.number.val;
    else if (remote_addr_obj.type == TP_STRING)
        inet_pton(tp, (const char *)remote_addr_obj.string.val, remote_addr_obj.string.len, &remote_addr);

    if ((s = __menuet__open_TCP_socket(local_port, remote_port, remote_addr, 0)) >= 0)
    {
        tp_set(tp, self, tp_string("socket"), tp_number(s));
        return tp_True;
    }
    else
        return tp_False;
}


/* Exported function.
 *
 * Example:
 *
 * s = socket(socket.AF_INET, socket.SOCK_DGRAM)
 *
 * Returns socket object.
 */
tp_obj kolibri_socket(TP)
{
    tp_obj s;
    tp_obj sockfamily = TP_TYPE(TP_NUMBER);
    tp_obj socktype = TP_TYPE(TP_NUMBER);

    if (fabs(sockfamily.number.val - AF_INET) > PRECISION ||
        (fabs(socktype.number.val - SOCK_STREAM) > PRECISION &&
         fabs(socktype.number.val - SOCK_DGRAM) > PRECISION))
        return tp_None;
    s = tp_dict(tp);
    tp_set(tp, s, tp_string("family"), sockfamily);
    tp_set(tp, s, tp_string("type"), socktype);
    tp_set(tp, s, tp_string("bind"), tp_method(tp, s, kolibri_bind));
    tp_set(tp, s, tp_string("connect"), tp_method(tp, s, kolibri_connect));
    tp_set(tp, s, tp_string("send"), tp_method(tp, s, kolibri_send));
    tp_set(tp, s, tp_string("recv"), tp_method(tp, s, kolibri_recv));
    tp_set(tp, s, tp_string("close"), tp_method(tp, s, kolibri_close_socket));
    if (fabs(socktype.number.val - SOCK_STREAM) < PRECISION)
        tp_set(tp, s, tp_string("listen"), tp_method(tp, s, kolibri_listen));
    return s;
}

tp_obj kolibri_socket_module(TP)
{
    tp_obj socket_mod = tp_dict(tp);

    tp_set(tp, socket_mod, tp_string("AF_INET"), tp_number(AF_INET));
    tp_set(tp, socket_mod, tp_string("SOCK_STREAM"), tp_number(SOCK_STREAM));
    tp_set(tp, socket_mod, tp_string("SOCK_DGRAM"), tp_number(SOCK_DGRAM));
    tp_set(tp, socket_mod, tp_string("inet_pton"), tp_fnc(tp, kolibri_inet_pton));
    tp_set(tp, socket_mod, tp_string("socket"), tp_fnc(tp, kolibri_socket));
    return socket_mod;
}
