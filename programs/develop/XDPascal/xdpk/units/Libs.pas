unit Libs;

interface

uses
  KolibriOS;

procedure InitLibrary(LibInit: Pointer);

implementation

procedure LibInitialize(MemoryAllocate, MemoryFree, MemoryReallocate, DLLLoad, LibInit: Pointer);
type
  TLibInitializeProc = procedure(MemoryAllocate, MemoryFree, MemoryReallocate,
    DLLLoad, LibInit: Pointer) stdcall;
const
  _LibInitialize: array[0..24] of Byte = (
    $60, $8B, $44, $24, $24, $8B, $5C, $24, $28, $8B, $4C, $24, $2C, $8B, $54,
    $24, $30, $FF, $54, $24, $34, $61, $C2, $14, $00
  );
var
  LibInitializeProc: TLibInitializeProc;
begin
  LibInitializeProc := TLibInitializeProc(@_LibInitialize);
  LibInitializeProc(MemoryAllocate,
                    MemoryFree,
                    MemoryReallocate,
                    DLLLoad,
                    LibInit);
end;

function MemoryAllocate(Bytes: Integer): Pointer stdcall;
type
  TMemoryAllocateProc = function(Bytes: Integer): Pointer stdcall;
const
  _MemoryAllocate: array[0..22] of Byte = (
    $51, $53, $B8, $44, $00, $00, $00, $BB, $0C, $00, $00, $00, $8B, $4C, $24,
    $0C, $CD, $40, $5B, $59, $C2, $04, $00
  );
var
  MemoryAllocateProc: TMemoryAllocateProc;
begin
  MemoryAllocateProc := TMemoryAllocateProc(@_MemoryAllocate);
  Result := MemoryAllocateProc(Bytes);
end;

function MemoryFree(MemPtr: Pointer): Integer stdcall;
type
  TMemoryFreeProc = function(MemPtr: Pointer): Integer stdcall;
const
  _MemoryFree: array[0..22] of Byte = (
    $51, $53, $B8, $44, $00, $00, $00, $BB, $0D, $00, $00, $00, $8B, $4C, $24,
    $0C, $CD, $40, $5B, $59, $C2, $04, $00
  );
var
  MemoryFreeProc: TMemoryFreeProc;
begin
  MemoryFreeProc := TMemoryFreeProc(@_MemoryFree);
  Result := MemoryFreeProc(MemPtr);
end;

function MemoryReallocate(MemPtr: Pointer; Bytes: Integer): Pointer stdcall;
type
  TMemoryReallocateProc = function(MemPtr: Pointer; Bytes: Integer): Pointer stdcall;
const
  _MemoryReallocate: array[0..28] of Byte = (
    $53, $51, $52, $B8, $44, $00, $00, $00, $BB, $14, $00, $00, $00, $8B, $4C,
    $24, $14, $8B, $54, $24, $10, $CD, $40, $5A, $59, $5B, $C2, $08, $00
  );
var
  MemoryReallocateProc: TMemoryReallocateProc;
begin
  MemoryReallocateProc := TMemoryReallocateProc(@_MemoryReallocate);
  Result := MemoryReallocateProc(MemPtr, Bytes);
end;

const
  LIB_PATH = '/sys/lib/';

type
  PNameAddr = ^TNameAddr;
  TNameAddr = packed record
    Name: PAnsiChar;
    Addr: Pointer;
  end;

  PAddrName = ^TAddrName;
  TAddrName = packed record
    Addr: Pointer;
    Name: PAnsiChar;
  end;

function StrEqual(Str1, Str2: PAnsiChar): Boolean;
begin
  while (Str1^ = Str2^) and (Str1^ <> #0) do
  begin
    Inc(Str1);
    Inc(Str2);
  end;
  Result := Str1^ = Str2^;
end;

procedure StrCopy(StrFrom, StrTo: PAnsiChar);
begin
  repeat
    StrTo^ := StrFrom^;
    Inc(StrFrom);
    Inc(StrTo);
  until StrFrom[-1] = #0;
end;

function DLLLoad(ImportTable: PAddrName): Integer stdcall;
var
  ExportTable: PNameAddr;
  ProcAddr: Pointer;
  Name: PPAnsiChar;
  LibPath: array [0..32] of AnsiChar;
begin
  Result := 1;
  StrCopy(LIB_PATH, @LibPath[0]);
  while ImportTable^.Addr <> nil do
  begin
    StrCopy(ImportTable^.Name, @LibPath[Length(LIB_PATH)]);
    ExportTable := LoadLibrary(LibPath);
    if ExportTable = nil then
      Exit;
    Name := PPAnsiChar(ImportTable^.Addr);
    while Name^ <> nil do
    begin
      ProcAddr := GetProcAddress(ExportTable, Name^);
      if ProcAddr <> nil then
        Name^ := PAnsiChar(ProcAddr)
      else
        Exit;
      Inc(Name);
    end;
    if StrEqual(ExportTable^.Name, 'lib_init') then
      InitLibrary(ExportTable^.Addr);
    Inc(ImportTable);
  end;
  Result := 0;
end;

procedure InitLibrary(LibInit: Pointer);
begin
  LibInitialize(@MemoryAllocate,
                @MemoryFree,
                @MemoryReallocate,
                @DLLLoad,
                LibInit);
end;

end.