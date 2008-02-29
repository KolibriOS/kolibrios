unit Windows;

{$mode objfpc}


interface


type
  WinBool = LongBool;
  Bool    = WinBool;
  Handle  = System.THandle;
  THandle = Handle;

  OVERLAPPED = record
    Internal : DWORD;
    InternalHigh : DWORD;
    Offset : DWORD;
    OffsetHigh : DWORD;
    hEvent : HANDLE;
  end;
  LPOVERLAPPED = ^OVERLAPPED;
  _OVERLAPPED = OVERLAPPED;
  TOVERLAPPED = OVERLAPPED;
  POVERLAPPED = ^OVERLAPPED;



function GetStdHandle(nStdHandle: DWord): Handle;
{function SetStdHandle(nStdHandle:DWORD; hHandle:HANDLE): WinBool;}

function WriteFile(hFile: THandle; const Buffer; nNumberOfBytesToWrite: DWord; var lpNumberOfBytesWritten: DWord; lpOverlapped: POverlapped): Bool;

function Std_Input_Handle: DWord;
function Std_Output_Handle: DWord;
function Std_Error_Handle: DWord;

function GetTickCount: DWord;
function QueryPerformanceCounter(var lpPerformanceCount: Int64): WinBool;
function QueryPerformanceFrequency(var lpFrequency: Int64): WinBool;

function AllocConsole: WinBool;
{function FreeConsole: WinBool;}


implementation


function GetStdHandle(nStdHandle: DWord): Handle;
begin
  Result := 0;
end;

function Std_Input_Handle: DWord;
begin
  Result := 0;
end;

function Std_Output_Handle: DWord;
begin
  Result := 1;
end;

function Std_Error_Handle: DWord;
begin
  Result := 2;
end;


function WriteFile(hFile: THandle; const Buffer; nNumberOfBytesToWrite: DWord; var lpNumberOfBytesWritten: DWord; lpOverlapped: POverlapped): Bool;
begin
  Result := True;
end;


function GetTickCount: DWord;
begin
  Result := kos_timecounter() * 10;
end;

function QueryPerformanceCounter(var lpPerformanceCount: Int64): WinBool;
begin
  lpPerformanceCount := kos_timecounter();
  Result := True
end;

function QueryPerformanceFrequency(var lpFrequency: Int64): WinBool;
begin
  lpFrequency := 100;
  Result := True
end;


function AllocConsole: WinBool;
begin
  Result := True;
end;


end.
