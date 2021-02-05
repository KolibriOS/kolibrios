#include "tinypy.h"
#include <sys/socket.h>

#define  GET_STR_ARG() TP_TYPE(TP_STRING).string.val
#define  NET_ERROR(FUNC, MSG)  if(FUNC==-1){ tp_raise_f(tp_None, "%s Error code: %d", MSG , err_code);}

static tp_obj inet_pton(TP, const char *buf, int len, unsigned *addr)
{
    char *p = (char *)buf;
    int i = 0;
    unsigned val = 0;
    *addr = 0;
    while (*p && p < buf + len && i < 4){
        if (*p == '.' || !*p){
            if (val > 255)
                tp_raise(tp_None, tp_string("ValueError: number > 255 in IP address"));
            *addr += (val << ((i++) << 3));
            val = 0;
        }else{
            if (*p < '0' || *p > '9')
                tp_raise(tp_None, tp_string("ValueError: bad char in IP address, digit expected"));
            val = val * 10 + *p - '0';
        }
        p++;
    }
    if (!*p){
        if (i == 3){
            *addr += (val << ((i++) << 3));
        }else{
            tp_raise_f(tp_None, "ValueError: bad IP address!", tp_None);
        }
    }
 }

static tp_obj _close(TP){
    tp_obj socket_info=TP_TYPE(TP_DICT);
    int id = (int)tp_get(tp, socket_info, tp_string("id")).number.val;
    NET_ERROR(close(id), "Unable to close socket!")
    else{
        tp_set(tp, socket_info, tp_string("closed"), tp_True);
    }
    return tp_None;
}

static tp_obj _bind(TP)
{
    tp_obj self = TP_TYPE(TP_DICT);
    const char* local_addr = TP_TYPE(TP_STRING).string.val;
    unsigned local_port = PORT((unsigned)TP_NUM());
    
    unsigned addr_n;
    inet_pton(tp, local_addr, strlen(local_addr), &addr_n);
    unsigned family  = (unsigned)tp_get(tp, self, tp_string("domain")).number.val;
    int id = (int)tp_get(tp, self, tp_string("id")).number.val;
    struct sockaddr addr={addr_n, family, local_port,0};    
    NET_ERROR(bind(id, &addr, sizeof(addr)), "Bind error!");
    return tp_None;
}

static tp_obj _listen(TP){
    int backlog = (int)TP_NUM();
    tp_obj socket_obj = TP_TYPE(TP_DICT);
    int id = (int)tp_get(tp,socket_obj, tp_string("id")).number.val;
    NET_ERROR(listen(id, backlog), "Listen error!");
    return tp_None;
}

static tp_obj _send(TP){
    const char * msg = GET_STR_ARG();
    tp_obj socket_obj = TP_TYPE(TP_DICT);
    int id = (int)tp_get(tp,socket_obj, tp_string("id")).number.val;
    NET_ERROR(send(id, (const void*)msg, strlen(msg), MSG_NOFLAG),"Sending failed!");
    return tp_None;
}

static tp_obj _connect(TP)
{
    tp_obj self = TP_TYPE(TP_DICT);
    const char* local_addr = TP_TYPE(TP_STRING).string.val;
    unsigned local_port = PORT((unsigned)TP_NUM());
    
    unsigned addr_n;
    inet_pton(tp, local_addr, strlen(local_addr), &addr_n);
    unsigned family  = (unsigned)tp_get(tp, self, tp_string("domain")).number.val;
    int id = (int)tp_get(tp, self, tp_string("id")).number.val;
    struct sockaddr addr={addr_n, family, local_port,0};    
    NET_ERROR(connect(id, &addr, sizeof(addr)), "Connection failed!");
    return tp_None;
}

static tp_obj _socket(TP){
    int domain = (int)TP_NUM();
    int type = (int)TP_NUM();
    int protocol=(int)TP_NUM();
    int id = socket(domain, type, protocol);
    tp_obj socket_info=tp_dict(tp);
    NET_ERROR(id, "Unable to open socket!")
    else{
        tp_set(tp, socket_info, tp_string("id"), tp_number(id));
        tp_set(tp, socket_info, tp_string("domain"), tp_number(domain));
        tp_set(tp, socket_info, tp_string("type"), tp_number(type));
        tp_set(tp, socket_info, tp_string("protocol"), tp_number(protocol));
    }
    tp_set(tp, socket_info, tp_string("connect"), tp_method(tp, socket_info, _connect));
    tp_set(tp, socket_info, tp_string("bind"), tp_method(tp, socket_info, _bind));
    tp_set(tp, socket_info, tp_string("close"), tp_method(tp, socket_info, _close));
    tp_set(tp, socket_info, tp_string("send"), tp_method(tp, socket_info, _send));
    return socket_info;
}


