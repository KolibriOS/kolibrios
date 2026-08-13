unit Network;

interface

uses
  KolibriOS, Libs;

const
// Socket types
  SOCK_STREAM    = 1;
  SOCK_DGRAM     = 2;
  SOCK_RAW       = 3;

// IP protocols
  IPPROTO_IP     = 0;
  IPPROTO_ICMP   = 1;
  IPPROTO_TCP    = 6;
  IPPROTO_UDP    = 17;
  IPPROTO_RAW    = 255;

// IP options
  IP_TTL         = 2;

// Address families
  AF_UNSPEC      = 0;
  AF_LOCAL       = 1;
  AF_INET4       = 2;  // IPv4
  AF_INET6       = 10; // IPv6

  PF_UNSPEC      = AF_UNSPEC;
  PF_LOCAL       = AF_LOCAL;
  PF_INET4       = AF_INET4;
  PF_INET6       = AF_INET6;

// Flags for addrinfo
  AI_PASSIVE     = 1;
  AI_CANONNAME   = 2;
  AI_NUMERICHOST = 4;
  AI_NUMERICSERV = 8;
  AI_ADDRCONFIG  = $0400;

type
  PSockAddr = ^TSockAddr;
  TSockAddr = packed record
    sin_family: Word;                // sa_family_t
    sin_port:   Word;                // in_port_t
    sin_addr:   Integer;             // struct in_addr
    sin_zero:   array[0..7] of Byte; // zero
  end;

  PAddrInfo = ^TAddrInfo;
  TAddrInfo = packed record
    ai_flags:     Integer;  // bitmask of AI_*
    ai_family:    Integer;  // PF_*
    ai_socktype:  Integer;  // SOCK_*
    ai_protocol:  Integer;  // 0 or IPPROTO_*
    ai_addrlen:   Integer;  // length of ai_addr
    ai_canonname: PAnsiChar; // char*
    ai_addr:      PSockAddr; // struct sockaddr*
    ai_next:      PAddrInfo; // struct addrinfo*
  end;

  PReqData = ^TReqData;
  TReqData = packed record
    socketnum: Integer;
    // external code should not look on rest of this structure,
    // it is internal for getaddrinfo_start/process/abort
    reqid:     Word; // DNS request ID
    socktype:  Byte; // SOCK_* or 0 for any
    reserved1: Byte;
    service:   Integer;
    flags:     Integer;
    reserved:  array[0..15] of Byte;
  end;

var
  inet_ntoa: function(Address: Integer): PAnsiChar stdcall;
  inet_addr: function(cp: PAnsiChar): Integer stdcall;
  getaddrinfo: function(HostName, ServName: PAnsiChar; Hints: PAddrInfo; var Res: PAddrInfo): Integer stdcall;
  freeaddrinfo: procedure(AddrInfo: PAddrInfo) stdcall;
  getaddrinfo_start: function(HostName, ServName: PAnsiChar; Hints: PAddrInfo; var Res: PAddrInfo; var ReqData: TReqData): Integer stdcall;
  getaddrinfo_process: function(var ReqData: TReqData; var Res: PAddrInfo): Integer stdcall;
  getaddrinfo_abort: procedure(var ReqData: TReqData) stdcall;

procedure Network_initialization;  
  
implementation

var
  hNetwork: Pointer;

procedure Network_initialization;
begin
  hNetwork := LoadLibrary('/sys/lib/network.obj');
  inet_ntoa := GetProcAddress(hNetwork, 'inet_ntoa');
  inet_addr := GetProcAddress(hNetwork, 'inet_addr');
  getaddrinfo := GetProcAddress(hNetwork, 'getaddrinfo');
  freeaddrinfo := GetProcAddress(hNetwork, 'freeaddrinfo');
  getaddrinfo_start := GetProcAddress(hNetwork, 'getaddrinfo_start');
  getaddrinfo_process := GetProcAddress(hNetwork, 'getaddrinfo_process');
  getaddrinfo_abort := GetProcAddress(hNetwork, 'getaddrinfo_abort');
  InitLibrary(GetProcAddress(hNetwork, 'lib_init'));
end;

end.