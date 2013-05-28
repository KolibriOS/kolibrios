; ICMP types & codes

ICMP_ECHOREPLY                  equ 0               ; echo reply message

ICMP_UNREACH                    equ 3
ICMP_UNREACH_NET                equ  0               ; bad net
ICMP_UNREACH_HOST               equ  1               ; bad host
ICMP_UNREACH_PROTOCOL           equ  2               ; bad protocol
ICMP_UNREACH_PORT               equ  3               ; bad port
ICMP_UNREACH_NEEDFRAG           equ  4               ; IP_DF caused drop
ICMP_UNREACH_SRCFAIL            equ  5               ; src route failed
ICMP_UNREACH_NET_UNKNOWN        equ  6               ; unknown net
ICMP_UNREACH_HOST_UNKNOWN       equ  7               ; unknown host
ICMP_UNREACH_ISOLATED           equ  8               ; src host isolated
ICMP_UNREACH_NET_PROHIB         equ  9               ; prohibited access
ICMP_UNREACH_HOST_PROHIB        equ 10              ; ditto
ICMP_UNREACH_TOSNET             equ 11              ; bad tos for net
ICMP_UNREACH_TOSHOST            equ 12              ; bad tos for host
ICMP_UNREACH_FILTER_PROHIB      equ 13              ; admin prohib
ICMP_UNREACH_HOST_PRECEDENCE    equ 14             ; host prec vio.
ICMP_UNREACH_PRECEDENCE_CUTOFF  equ 15           ; prec cutoff

ICMP_SOURCEQUENCH               equ 4               ; Packet lost, slow down

ICMP_REDIRECT                   equ 5               ; shorter route, codes:
ICMP_REDIRECT_NET               equ  0               ; for network
ICMP_REDIRECT_HOST              equ  1               ; for host
ICMP_REDIRECT_TOSNET            equ  2               ; for tos and net
ICMP_REDIRECT_TOSHOST           equ  3               ; for tos and host

ICMP_ALTHOSTADDR                equ 6               ; alternate host address
ICMP_ECHO                       equ  8               ; echo service
ICMP_ROUTERADVERT               equ  9               ; router advertisement
ICMP_ROUTERADVERT_NORMAL        equ  0                  ; normal advertisement
ICMP_ROUTERADVERT_NOROUTE_COMMON equ 16         ; selective routing

ICMP_ROUTERSOLICIT              equ 10              ; router solicitation
ICMP_TIMXCEED                   equ 11              ; time exceeded, code:
ICMP_TIMXCEED_INTRANS           equ 0               ; ttl==0 in transit
ICMP_TIMXCEED_REASS             equ 1               ; ttl==0 in reass

ICMP_PARAMPROB                  equ 12               ; ip header bad
ICMP_PARAMPROB_ERRATPTR         equ 0            ; error at param ptr
ICMP_PARAMPROB_OPTABSENT        equ 1            ; req. opt. absent
ICMP_PARAMPROB_LENGTH           equ 2            ; bad length

ICMP_TSTAMP                     equ 13              ; timestamp request
ICMP_TSTAMPREPLY                equ 14              ; timestamp reply
ICMP_IREQ                       equ 15              ; information request
ICMP_IREQREPLY                  equ 16              ; information reply
ICMP_MASKREQ                    equ 17              ; address mask request
ICMP_MASKREPLY                  equ 18              ; address mask reply
ICMP_TRACEROUTE                 equ 30              ; traceroute
ICMP_DATACONVERR                equ 31              ; data conversion error
ICMP_MOBILE_REDIRECT            equ 32              ; mobile host redirect
ICMP_IPV6_WHEREAREYOU           equ 33              ; IPv6 where-are-you
ICMP_IPV6_IAMHERE               equ 34              ; IPv6 i-am-here
ICMP_MOBILE_REGREQUEST          equ 35              ; mobile registration req
ICMP_MOBILE_REGREPLY            equ 36              ; mobile registreation reply
ICMP_SKIP                       equ 39              ; SKIP

ICMP_PHOTURIS                   equ 40              ; Photuris
ICMP_PHOTURIS_UNKNOWN_INDEX     equ 1                ; unknown sec index
ICMP_PHOTURIS_AUTH_FAILED       equ 2                ; auth failed
ICMP_PHOTURIS_DECRYPT_FAILED    equ 3                ; decrypt failed



virtual at 0
        ICMP_Packet:
        .Type           db   ?
        .Code           db   ?
        .Checksum       dw   ?
        .Identifier     dw   ?
        .SequenceNumber dw   ?
        .Data:
end virtual