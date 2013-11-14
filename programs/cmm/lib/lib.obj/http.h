//HTTP library

dword libHTTP = #alibHTTP;
char alibHTTP[23] = "/sys/lib/http.obj\0";

dword http_lib_init       = #aLib_init;
dword http_get            = #aHTTPget;
dword http_process        = #aHTTPprocess;
$DD 2 dup 0

char aLib_init[9]              = "lib_init\0";
char aHTTPget[4]               = "get\0";
char aHTTPprocess[8]           = "process\0";

#define FLAG_HTTP11             1 << 0
#define FLAG_GOT_HEADER         1 << 1
#define FLAG_GOT_DATA           1 << 2
#define FLAG_CONTENT_LENGTH     1 << 3
#define FLAG_CHUNKED            1 << 4

// error flags go into the upper word
#define FLAG_INVALID_HEADER     1 << 16
#define FLAG_NO_RAM             1 << 17
#define FLAG_SOCKET_ERROR       1 << 18

struct  http_msg{
        dword   socket;
        dword   flags;
        dword   write_ptr;
        dword   buffer_length;
        dword   chunk_ptr;

        dword   status;
        dword   header_length;
        dword   content_length;
        dword   content_received;
        char    data;
};