#include "tinypy.h"
#include "net.c"

#define EXPORT(MOD_NAME, F_NAME, F_POINT) tp_set(tp, MOD_NAME , tp_string(F_NAME), tp_fnc(tp, F_POINT))

extern tp_obj tp_dict(TP);
extern tp_obj tp_fnc(TP,tp_obj v(TP));

void socket_init(TP)
{
    tp_obj socket_mod = tp_dict(tp);
    EXPORT(socket_mod, "socket"  , _socket);
    
    tp_set(tp, socket_mod, tp_string("AF_INET"), tp_number(AF_INET));
    tp_set(tp, socket_mod, tp_string("AF_INET6"), tp_number(AF_INET6));
    tp_set(tp, socket_mod, tp_string("AF_LOCAL"), tp_number(AF_LOCAL));
    tp_set(tp, socket_mod, tp_string("AF_UNSPEC"), tp_number(AF_UNSPEC));
    
    tp_set(tp, socket_mod, tp_string("SOCK_STREAM"), tp_number(SOCK_STREAM));
    tp_set(tp, socket_mod, tp_string("SOCK_DGRAM"), tp_number(SOCK_DGRAM));
    tp_set(tp, socket_mod, tp_string("SOCK_RAW"), tp_number(SOCK_RAW));
    
    tp_set(tp, socket_mod, tp_string("IPPROTO_IP"), tp_number(IPPROTO_IP));
    tp_set(tp, socket_mod, tp_string("IPPROTO_TCP"), tp_number(IPPROTO_TCP));
    tp_set(tp, socket_mod, tp_string("IPPROTO_UDP"), tp_number(IPPROTO_UDP));
    tp_set(tp, socket_mod, tp_string("IPPROTO_RAW"), tp_number(IPPROTO_RAW));
    tp_set(tp, socket_mod, tp_string("IPPROTO_ICMP"), tp_number(IPPROTO_ICMP));
    
    
    tp_set(tp, socket_mod, tp_string("__doc__"), tp_string("Working with network sockets"));
    tp_set(tp, socket_mod, tp_string("__name__"), tp_string("Sockets"));
    tp_set(tp, socket_mod, tp_string("__file__"), tp_string(__FILE__));
    tp_set(tp, tp->modules, tp_string("socket"), socket_mod);
}
