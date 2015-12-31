; Socket types
SOCK_STREAM     = 1
SOCK_DGRAM      = 2
SOCK_RAW        = 3

; IP protocols
IPPROTO_IP      = 0
IPPROTO_ICMP    = 1
IPPROTO_TCP     = 6
IPPROTO_UDP     = 17
IPPROTO_RAW     = 255

; IP options
IP_TTL          = 2

; Address families
AF_UNSPEC       = 0
AF_LOCAL        = 1
AF_INET4        = 2     ; IPv4
AF_INET6        = 10    ; IPv6

PF_UNSPEC       = AF_UNSPEC
PF_LOCAL        = AF_LOCAL
PF_INET4        = AF_INET4
PF_INET6        = AF_INET6

; Flags for addrinfo
AI_PASSIVE      = 1
AI_CANONNAME    = 2
AI_NUMERICHOST  = 4
AI_NUMERICSERV  = 8
AI_ADDRCONFIG   = 0x400

; internal definition
AI_SUPPORTED    = 0x40F

; for system function 76
API_ETH         = 0 shl 16
API_IPv4        = 1 shl 16
API_ICMP        = 2 shl 16
API_UDP         = 3 shl 16
API_TCP         = 4 shl 16
API_ARP         = 5 shl 16
API_PPPOE       = 6 shl 16

; Socket flags for user calls
MSG_PEEK        = 0x02
MSG_DONTWAIT    = 0x40

; Socket levels
SOL_SOCKET      = 0xffff

; Socket options
SO_BINDTODEVICE = 1 shl 9
SO_NONBLOCK     = 1 shl 31

struct sockaddr_in
        sin_family      dw      ?       ; sa_family_t
        sin_port        dw      ?       ; in_port_t
        sin_addr        dd      ?       ; struct in_addr
        sin_zero        rb      8       ; zero
ends

struct addrinfo
        ai_flags        dd      ?       ; bitmask of AI_*
        ai_family       dd      ?       ; PF_*
        ai_socktype     dd      ?       ; SOCK_*
        ai_protocol     dd      ?       ; 0 or IPPROTO_*
        ai_addrlen      dd      ?       ; length of ai_addr
        ai_canonname    dd      ?       ; char*
        ai_addr         dd      ?       ; struct sockaddr*
        ai_next         dd      ?       ; struct addrinfo*
ends

EAI_ADDRFAMILY  = 1
EAI_AGAIN       = 2
EAI_BADFLAGS    = 3
EAI_FAIL        = 4
EAI_FAMILY      = 5
EAI_MEMORY      = 6
EAI_NONAME      = 8
EAI_SERVICE     = 9
EAI_SOCKTYPE    = 10
EAI_BADHINTS    = 12
EAI_PROTOCOL    = 13
EAI_OVERFLOW    = 14

socket          fix 75, 0
close           fix 75, 1
bind            fix 75, 2
listen          fix 75, 3
connect         fix 75, 4
accept          fix 75, 5
send            fix 75, 6
recv            fix 75, 7
setsockopt      fix 75, 8
getsockopt      fix 75, 9
socketpair      fix 75, 10


struct  ARP_entry
        IP      dd ?
        MAC     dp ?
        status  dw ?
        TTL     dw ?
ends


; Socket error codes
; Error Codes
ENOBUFS                 = 1
EINPROGRESS             = 2
EOPNOTSUPP              = 4
EWOULDBLOCK             = 6
ENOTCONN                = 9
EALREADY                = 10
EINVAL                  = 11
EMSGSIZE                = 12
ENOMEM                  = 18
EADDRINUSE              = 20
ECONNREFUSED            = 61
ECONNRESET              = 52
EISCONN                 = 56
ETIMEDOUT               = 60
ECONNABORTED            = 53