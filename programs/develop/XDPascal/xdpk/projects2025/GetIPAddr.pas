program GetIPAddr;

uses
  KolibriOS, Network;
  
var
  AddrInfo: PAddrInfo;
  InputStr: string;

begin
  Network_initialization;
  
  while True do
  begin  
    Write('Input host:'); ReadLn(InputStr);
    if getaddrinfo(InputStr, nil, nil, AddrInfo) = 0{success} then
      WriteLn(PString(inet_ntoa(AddrInfo^.ai_addr^.sin_addr))^)
    else
      WriteLn('Error!');
    freeaddrinfo(AddrInfo);
    Write('Continue?[Y/N]:'); ReadLn(InputStr);
    if UpCase(InputStr[1]) = 'N' then Exit;
  end;
end.