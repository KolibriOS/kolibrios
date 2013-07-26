//Network library

dword network_lib = #a_network_lib;
char a_network_lib[21]="/sys/lib/network.obj\0";

dword network_lib_init    = #aLib_init;
dword inet_addr           = #aInet_addr;  //"192.168.0.1" -> dword IP
dword inet_ntoa           = #aInet_ntoa;
dword getaddrinfo         = #aGetaddrinfo;
dword getaddrinfo_start   = #aGetaddrinfo_start;
dword getaddrinfo_process = #aGetaddrinfo_process;
dword getaddrinfo_abort   = #aGetaddrinfo_abort;
dword freeaddrinfo        = #aFreeaddrinfo;
$DD 2 dup 0

char aLib_init[9]              = "lib_init\0";
char aInet_addr[10]            = "inet_addr\0";
char aInet_ntoa[10]            = "inet_ntoa\0";
char aGetaddrinfo[12]          = "getaddrinfo\0";
char aGetaddrinfo_start[18]    = "getaddrinfo_start\0";
char aGetaddrinfo_process[20]  = "getaddrinfo_process\0";
char aGetaddrinfo_abort[18]    = "getaddrinfo_abort\0";
char aFreeaddrinfo[13]         = "freeaddrinfo\0";

/*
addr соответствует IP 10.101.102.103
itoa((addr&0xFF000000)>>24) равно 103
itoa((addr&0xFF0000)>>16) —это 102
itoa((addr&0xFF00)>>8) — это 101
itoa(addr&0xFF) — это 10
*/

dword GetIPfromAdress(dword addr)
{
	dword lpointer, IPa;
	getaddrinfo stdcall (addr, 0, 0, #lpointer);
	if (EAX!=0) IPa = 0; else IPa = DSDWORD[DSDWORD[lpointer+24]+4];
	freeaddrinfo stdcall (lpointer);
	return IPa;
}

/*
//Convert the string from standard IPv4 dotted notation to integer IP addr.
inet_addr stdcall ("192.168.0.1");
server_IP = EAX;


char* __stdcall inet_ntoa(struct in_addr in);
Convert the Internet host address to standard IPv4 dotted notation.

getaddrinfo(__in const char* hostname, __in const char* servname,
            __in const struct addrinfo* hints, __out struct addrinfo **res);    
struct addrinfo {
    int     ai_flags;
    int     ai_family;
    int     ai_socktype;
    int     ai_protocol;
    size_t  ai_addrlen;
    struct sockaddr *ai_addr;
    char   *ai_canonname;
    struct addrinfo *ai_next;
}; 
*/
