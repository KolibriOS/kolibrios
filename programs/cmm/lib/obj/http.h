#ifndef INCLUDE_LIBHTTP_H
#define INCLUDE_LIBHTTP_H

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

#ifndef INCLUDE_DLL_H
#include "../lib/dll.h"
#endif

dword libHTTP = #alibHTTP;
char alibHTTP[] = "/sys/lib/http.obj";

dword http_lib_init          = #aHTTPinit;
dword http_get               = #aHTTPget;
dword http_head              = #aHTTPhead;
dword http_post              = #aHTTPpost;
dword http_find_header_field = #aFHF;
dword http_send              = #aHTTPsend;
dword http_receive           = #aHTTPreceive;
dword http_disconnect        = #aHTTPdisconnect;
dword http_free              = #aHTTPfree;
dword uri_escape             = #aURIescape;
dword uri_unescape           = #aURIunescape;
$DD 2 dup 0

char aHTTPinit[]             = "lib_init";
char aHTTPget[]              = "get";
char aHTTPhead[]             = "head";
char aHTTPpost[]             = "post";
char aFHF[]                  = "find_header_field";
char aHTTPsend[]             = "send";
char aHTTPreceive[]          = "receive";
char aHTTPdisconnect[]       = "disconnect";
char aHTTPfree[]             = "free";
char aURIescape[]            = "escape";
char aURIunescape[]          = "unescape";

// status flags
#define FLAG_HTTP11             1 << 0
#define FLAG_GOT_HEADER         1 << 1
#define FLAG_GOT_ALL_DATA       1 << 2
#define FLAG_CONTENT_LENGTH     1 << 3
#define FLAG_CHUNKED            1 << 4
#define FLAG_CONNECTED          1 << 5

// user flags
#define FLAG_KEEPALIVE          1 << 8
#define FLAG_MULTIBUFF          1 << 9

// error flags
#define FLAG_INVALID_HEADER     1 << 16
#define FLAG_NO_RAM             1 << 17
#define FLAG_SOCKET_ERROR       1 << 18
#define FLAG_TIMEOUT_ERROR      1 << 19
#define FLAG_TRANSFER_FAILED    1 << 20

struct  http_msg{
        dword   socket;
        dword   flags;
        dword   write_ptr;
        dword   buffer_length;
        dword   chunk_ptr;
        dword   timestamp;
        dword   status;
        dword   header_length;
        dword   content_ptr;
        dword   content_length;
        dword   content_received;
        char    http_header;
};


#endif