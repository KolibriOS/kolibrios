{}

unit sysinitpas;

interface

implementation

var
  SysInstance: Longint; external name '_FPC_SysInstance';

procedure PascalMain; stdcall; external name 'PASCALMAIN';
procedure SystemExit; external name 'SystemExit';

procedure EntryConsole; [public, alias:'_mainCRTStartup'];
var
  ESP_: Pointer;
begin
  asm movl %esp, ESP_; end;
  StackTop := ESP_ + 8;
  IsConsole := True;
  PascalMain;
  SystemExit;
end;

procedure EntryWindow; [public, alias:'_WinMainCRTStartup'];
var
  ESP_: Pointer;
begin
  asm movl %esp, ESP_; end;
  StackTop := ESP_ + 8;
  IsConsole := False;
  PascalMain;
  SystemExit;
end;

procedure _FPC_DLLMainCRTStartup(_hinstance, _dllreason, _dllparam: Longint); stdcall; public name '_DLLMainCRTStartup';
begin
  {TODO}
  IsConsole := True;
  SysInstance := _hinstance;
end;


procedure _FPC_DLLWinMainCRTStartup(_hinstance, _dllreason, _dllparam: Longint); stdcall; public name '_DLLWinMainCRTStartup';
begin
  {TODO}
  IsConsole := False;
  SysInstance := _hinstance;
end;

end.
