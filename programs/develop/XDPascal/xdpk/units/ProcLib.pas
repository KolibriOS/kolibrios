unit ProcLib;

interface

uses
  KolibriOS;
  
var
  hProcLib: Pointer;
  
procedure ProcLib_initialization;  

implementation

procedure ProcLib_initialization;
begin
  hProcLib := LoadLibrary('/sys/lib/proc_lib.obj');
end;

end.