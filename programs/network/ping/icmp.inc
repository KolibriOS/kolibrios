; ICMP types & codes

ICMP_ECHOREPLY                  =   0               ; echo reply message

ICMP_UNREACH                    =   3
ICMP_UNREACH_NET                =    0               ; bad net
ICMP_UNREACH_HOST               =    1               ; bad host
ICMP_UNREACH_PROTOCOL           =    2               ; bad protocol
ICMP_UNREACH_PORT               =    3               ; bad port
ICMP_UNREACH_NEEDFRAG           =    4               ; IP_DF caused drop
ICMP_UNREACH_SRCFAIL            =    5               ; src route failed
ICMP_UNREACH_NET_UNKNOWN        =    6               ; unknown net
ICMP_UNREACH_HOST_UNKNOWN       =    7               ; unknown host
ICMP_UNREACH_ISOLATED           =    8               ; src host isolated
ICMP_UNREACH_NET_PROHIB         =    9               ; prohibited access
ICMP_UNREACH_HOST_PROHIB        =   10              ; ditto
ICMP_UNREACH_TOSNET             =   11              ; bad tos for net
ICMP_UNREACH_TOSHOST            =   12              ; bad tos for host
ICMP_UNREACH_FILTER_PROHIB      =   13              ; admin prohib
ICMP_UNREACH_HOST_PRECEDENCE    =   14             ; host prec vio.
ICMP_UNREACH_PRECEDENCE_CUTOFF  =   15           ; prec cutoff

ICMP_SOURCEQUENCH               =   4               ; Packet lost, slow down

ICMP_REDIRECT                   =   5               ; shorter route, codes:
ICMP_REDIRECT_NET               =    0               ; for network
ICMP_REDIRECT_HOST              =    1               ; for host
ICMP_REDIRECT_TOSNET            =    2               ; for tos and net
ICMP_REDIRECT_TOSHOST           =    3               ; for tos and host

ICMP_ALTHOSTADDR                =   6               ; alternate host address
ICMP_ECHO                       =    8               ; echo service
ICMP_ROUTERADVERT               =    9               ; router advertisement
ICMP_ROUTERADVERT_NORMAL        =    0                  ; normal advertisement
ICMP_ROUTERADVERT_NOROUTE_COMMON =   16         ; selective routing

ICMP_ROUTERSOLICIT              =   10              ; router solicitation
ICMP_TIMXCEED                   =   11              ; time exceeded, code:
ICMP_TIMXCEED_INTRANS           =   0               ; ttl==0 in transit
ICMP_TIMXCEED_REASS             =   1               ; ttl==0 in reass

ICMP_PARAMPROB                  =   12               ; ip header bad
ICMP_PARAMPROB_ERRATPTR         =   0            ; error at param ptr
ICMP_PARAMPROB_OPTABSENT        =   1            ; req. opt. absent
ICMP_PARAMPROB_LENGTH           =   2            ; bad length

ICMP_TSTAMP                     =   13              ; timestamp r=  est
ICMP_TSTAMPREPLY                =   14              ; timestamp reply
ICMP_IREQ                       =   15              ; information r=  est
ICMP_IREQREPLY                  =   16              ; information reply
ICMP_MASKREQ                    =   17              ; address mask r=  est
ICMP_MASKREPLY                  =   18              ; address mask reply
ICMP_TRACEROUTE                 =   30              ; traceroute
ICMP_DATACONVERR                =   31              ; data conversion error
ICMP_MOBILE_REDIRECT            =   32              ; mobile host redirect
ICMP_IPV6_WHEREAREYOU           =   33              ; IPv6 where-are-you
ICMP_IPV6_IAMHERE               =   34              ; IPv6 i-am-here
ICMP_MOBILE_REGREQUEST          =   35              ; mobile registration req
ICMP_MOBILE_REGREPLY            =   36              ; mobile registreation reply
ICMP_SKIP                       =   39              ; SKIP

ICMP_PHOTURIS                   =   40              ; Photuris
ICMP_PHOTURIS_UNKNOWN_INDEX     =   1                ; unknown sec index
ICMP_PHOTURIS_AUTH_FAILED       =   2                ; auth failed
ICMP_PHOTURIS_DECRYPT_FAILED    =   3                ; decrypt failed



struct  ICMP_header
        Type            db ?
        Code            db ?
        Checksum        dw ?
        Identifier      dw ?
        SequenceNumber  dw ?
ends