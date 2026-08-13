// XD Pascal - a 32-bit compiler for KolibriOS
// Copyright (c) 2009-2010, 2019-2020, Vasiliy Tereshkov

unit System;


interface


const  
  // Windows API constants
  
  STD_INPUT_HANDLE      = -10;
  STD_OUTPUT_HANDLE     = -11;
  
  INVALID_HANDLE_VALUE  = -1;


  
  // Other constants
  
  Pi                    = 3.141592653589793;
  MaxStrLength          = 255;
  MaxSetElements        = 256;
  MaxSetIndex           = MaxSetElements div 32 - 1;
  


type
  LongInt = Integer;  
  Double = Real;
  Extended = Real;
  Text = file;  
  PChar = ^Char;  
  
  TFileRec = record
    Name: string;
    Handle: LongInt;
  end;

  PFileRec = ^TFileRec;  
  
  TStream = record
    Data: PChar;
    Index: Integer;
  end;

  PStream = ^TStream;

  TSetStorage = array [0..MaxSetIndex] of Integer; 



var
  StdInputFile, StdOutputFile: file;  
  DecimalSeparator: Char = '.';   

  

// KolibriOS definitions

type
  PPAnsiChar = ^PAnsiChar;
  PAnsiChar = ^AnsiChar;
  AnsiChar = Char;

  PString = ^string;
  
  TFileDate = Packed Record
    Day:   Byte;
    Month: Byte;
    Year:  Word;
  End;

  TFileTime = Packed Record
    Seconds: Byte;
    Minutes: Byte;
    Hours:   Byte;
    Zero:    Byte;
  End;

  TFileAttributes = Packed Record
    Attributes:   LongInt;
    Flags:        LongInt;
    CreationTime: TFileTime;
    CreationDate: TFileDate;
    AccessTime:   TFileTime;
    AccessDate:   TFileDate;
    ModifyTime:   TFileTime;
    ModifyDate:   TFileDate;
    SizeLo:       LongInt;
    SizeHi:       LongInt;
  End;
    
const
  MAX_PARAMS = 128;

type
  TArgValues = array[0..MAX_PARAMS - 1] of PAnsiChar;

var
  ArgCount: LongInt = 1; // at least program path
  ArgValues: TArgValues;
  
type  
  TGetProcAddressProc    = function(hLib: Pointer; ProcName: PAnsiChar): Pointer stdcall;
  TLoadLibraryProc       = function(Path: PAnsiChar): Pointer stdcall;
  TTerminateThreadProc   = procedure stdcall;
  TRDTSCProc             = function: LongInt stdcall;
  THeapAllocateProc      = function(Bytes: LongInt): Pointer stdcall;
  THeapFreeProc          = function(MemPtr: Pointer): LongInt stdcall;
  //THeapCreateProc        = function: LongInt stdcall;
  TProcessArgsProc       = procedure(var ArgCount: LongInt; var ArgValues: TArgValues) stdcall;
  TCreateFileProc        = function(Path: PAnsiChar): LongInt stdcall;
  TGetFileAttributesProc = function(Path: PAnsiChar; Var Buffer: TFileAttributes): LongInt stdcall;
  TWriteFileProc         = function(Path: PAnsiChar; Var Buffer; Count, LoPos, HiPos: LongInt; Var BytesWritten: LongInt): LongInt stdcall;
  TReadFileProc          = function(Path: PAnsiChar; Var Buffer; Count, LoPos, HiPos: LongInt; Var BytesRead: LongInt): LongInt stdcall;

// KolibriOS has no sys. function for FilePointers like Seek  
// also no any Handles
// therefore we emulate them
  PInternalFileHandle = ^TInternalFileHandle;
  TInternalFileHandle = record
    FilePointer: LongInt;
  end;  
  
  
var
  LoadLibrary:       TLoadLibraryProc;
  GetProcAddress:    TGetProcAddressProc;
  TerminateThread:   TTerminateThreadProc;
  RDTSC:             TRDTSCProc;
  HeapAllocate:      THeapAllocateProc;
  HeapFree:          THeapFreeProc; 
  //HeapCreate:        THeapCreateProc;
  ProcessArgs:       TProcessArgsProc;
  CreateFile:        TCreateFileProc;        
  GetFileAttributes: TGetFileAttributesProc;
  WriteFile:         TWriteFileProc;
  ReadFile:          TReadFileProc;  

  
  hConsole:     Pointer;
  ConsoleInit:  procedure(WndWidth, WndHeight, ScrWidth, ScrHeight: LongInt; Title: PAnsiChar) stdcall;
  ConsoleExit:  procedure(CloseWindow: Boolean) stdcall;
  WriteConsole: procedure(Str: PAnsiChar; Length: LongInt) stdcall;  
  ReadConsole:  function(Str: PAnsiChar; Length: LongInt): PAnsiChar stdcall;
  


// Other functions

procedure InitSystem;
//function Timer: LongInt; // not implemented
procedure GetMem(var P: Pointer; Size: Integer);
procedure FreeMem(var P: Pointer);
procedure Randomize;
function Random: Real;
function Length(const s: string): Integer;
procedure SetLength(var s: string; NewLength: Integer);
procedure AssignStr(var Dest: string; const Source: string);
procedure AppendStr(var Dest: string; const Source: string);
procedure ConcatStr(const s1, s2: string; var s: string);
function CompareStr(const s1, s2: string): Integer;
procedure Move(var Source; var Dest; Count: Integer);
function Copy(const S: string; Index, Count: Integer): string;
procedure FillChar(var Data; Count: Integer; Value: Char);
//function ParseCmdLine(Index: Integer; var Str: string): Integer; // for KolibriOS used ProcessArgs instead
function ParamCount: Integer;
function ParamStr(Index: Integer): string;
procedure IStr(Number: Integer; var s: string);
procedure Str(Number: Real; var s: string; MinWidth: Integer = 0; DecPlaces: Integer = 0);
procedure Val(const s: string; var Number: Real; var Code: Integer);
procedure IVal(const s: string; var Number: Integer; var Code: Integer);
procedure Assign(var F: file; const Name: string);
procedure Rewrite(var F: file; BlockSize: Integer = 1);
procedure Reset(var F: file; BlockSize: Integer = 1);
procedure Close(var F: file);
procedure BlockWrite(var F: file; var Buf; Len: Integer);
procedure BlockRead(var F: file; var Buf; Len: Integer; var LenRead: Integer);
procedure Seek(var F: file; Pos: Integer);
function FileSize(var F: file): Integer;
function FilePos(var F: file): Integer;
function EOF(var F: file): Boolean;
function IOResult: Integer;
procedure WriteRec(var F: file; P: PStream; var Buf; Len: Integer);
procedure WriteStringF(var F: file; P: PStream; const S: string; MinWidth, DecPlaces: Integer);
procedure WriteIntF(var F: file; P: PStream; Number: Integer; MinWidth, DecPlaces: Integer);
procedure WritePointerF(var F: file; P: PStream; Number: Integer; MinWidth, DecPlaces: Integer);
procedure WriteRealF(var F: file; P: PStream; Number: Real; MinWidth, DecPlaces: Integer);
procedure WriteBooleanF(var F: file; P: PStream; Flag: Boolean; MinWidth, DecPlaces: Integer);
procedure WriteNewLine(var F: file; P: PStream);
procedure ReadRec(var F: file; P: PStream; var Buf; Len: Integer);
procedure ReadCh(var F: file; P: PStream; var ch: Char);
procedure ReadInt(var F: file; P: PStream; var Number: Integer);
procedure ReadSmallInt(var F: file; P: PStream; var Number: SmallInt);
procedure ReadShortInt(var F: file; P: PStream; var Number: ShortInt);
procedure ReadWord(var F: file; P: PStream; var Number: Word);
procedure ReadByte(var F: file; P: PStream; var Number: Byte);
procedure ReadBoolean(var F: file; P: PStream; var Value: Boolean);
procedure ReadReal(var F: file; P: PStream; var Number: Real);
procedure ReadSingle(var F: file; P: PStream; var Number: Single);
procedure ReadString(var F: file; P: PStream; var s: string);
procedure ReadNewLine(var F: file; P: PStream);
function UpCase(ch: Char): Char;
procedure InitSet(var SetStorage: TSetStorage);
procedure AddToSet(var SetStorage: TSetStorage; FromElement, ToElement: Integer);
function InSet(Element: Integer; var SetStorage: TSetStorage): Boolean;
procedure SetUnion(const SetStorage1, SetStorage2: TSetStorage; var SetStorage: TSetStorage);
procedure SetDifference(const SetStorage1, SetStorage2: TSetStorage; var SetStorage: TSetStorage);
procedure SetIntersection(const SetStorage1, SetStorage2: TSetStorage; var SetStorage: TSetStorage);
function CompareSets(const SetStorage1, SetStorage2: TSetStorage): Integer;
function TestSubset(const SetStorage1, SetStorage2: TSetStorage): Integer;
function TestSuperset(const SetStorage1, SetStorage2: TSetStorage): Integer;

procedure ExitProcess;

function GetTickCount: LongInt; // not implemented, just for testing, used in raytracer example

implementation



var
  RandSeed: Integer;  
  IOError: Integer = 0;
  StdInputHandle, StdOutputHandle: LongInt;
  StdInputBuffer: string = '';
  StdInputBufferPos: Integer = 1;
  LastReadChar: Char = ' ';
  
  
procedure PtrStr(Number: Integer; var s: string); forward;  




// KolibriOS API functions

const
  // Procedure TerminateThread
  _TerminateThread: array[0..4] of Byte = (
    $83, $C8, $FF, $CD, $40
  );

  // Function LoadLibrary(Path: PAnsiChar): Pointer; StdCall
  _LoadLibrary: array[0..20] of Byte = (
    $53, $B8, $44, $00, $00, $00, $BB, $13, $00, $00, $00, $8B, $4C, $24, $08,
    $CD, $40, $5B, $C2, $04, $00
  );

  // Function  GetProcAddress(hLib: Pointer; ProcName: PAnsiChar): Pointer; StdCall;
  _GetProcAddress: array[0..55] of Byte = (
  $56, $57, $53, $8B, $54, $24, $10, $31, $C0, $85, $D2, $74, $25, $8B, $7C,
  $24, $14, $B9, $FF, $FF, $FF, $FF, $F2, $AE, $89, $CB, $F7, $D3, $8B, $32,
  $85, $F6, $74, $10, $89, $D9, $8B, $7C, $24, $14, $83, $C2, $08, $F3, $A6,
  $75, $ED, $8B, $42, $FC, $5B, $5F, $5E, $C2, $08, $00
);

// Function  HeapAllocate(Bytes: LongInt): Pointer; StdCall;
_HeapAllocate: array[0..20] of Byte = (
  $53, $B8, $44, $00, $00, $00, $BB, $0C, $00, $00, $00, $8B, $4C, $24, $08,
  $CD, $40, $5B, $C2, $04, $00
);

// Function  HeapFree(MemPtr: Pointer): LongInt; StdCall;
_HeapFree: array[0..20] of Byte = (
  $53, $B8, $44, $00, $00, $00, $BB, $0D, $00, $00, $00, $8B, $4C, $24, $08,
  $CD, $40, $5B, $C2, $04, $00
);

//// Function  HeapCreate: LongInt; StdCall;
//_HeapCreate: array[0..14] of Byte = (
//  $53, $B8, $44, $00, $00, $00, $BB, $0B, $00, $00, $00, $CD, $40, $5B, $C3
//);

// Function  RDTSC(): LongInt;
_RDTSC: array[0..2] of Byte = (
  $0F, $31, $C3
);

// procedure ProcessArgs(var ArgCount: LongInt; var ArgValues: TArgValues); stdcall;
_ProcessArgs: array[0..83] of Byte = (
  $53, $56, $57, $BE, $1C, $00, $00, $00, $8B, $36, $8B, $7C, $24, $14, $B8,
  $20, $00, $00, $00, $8B, $00, $89, $07, $83, $C7, $04, $31, $DB, $31, $C9,
  $8A, $0E, $E3, $26, $46, $80, $F9, $20, $74, $F6, $B0, $22, $43, $38, $C8,
  $89, $F2, $74, $03, $4A, $B0, $20, $89, $17, $8A, $0E, $E3, $0E, $46, $38,
  $C8, $75, $F7, $83, $C7, $04, $C6, $46, $FF, $00, $EB, $D6, $8B, $44, $24,
  $10, $01, $18, $5F, $5E, $5B, $C2, $04, $00
);

// Function  CreateFile(Path: PAnsiChar): LongInt; StdCall;
_CreateFile: array[0..35] of Byte = (
  $53, $FF, $74, $24, $08, $4C, $C6, $04, $24, $00, $6A, $00, $6A, $00, $6A,
  $00, $6A, $00, $6A, $02, $89, $E3, $B8, $46, $00, $00, $00, $CD, $40, $83,
  $C4, $19, $5B, $C2, $04, $00
);

// Function  GetFileAttributes(Path: PAnsiChar; Var Buffer: TFileAttributes): LongInt; StdCall;
_GetFileAttributes: array[0..37] of Byte = (
  $53, $FF, $74, $24, $08, $4C, $C6, $04, $24, $00, $FF, $74, $24, $11, $6A,
  $00, $6A, $00, $6A, $00, $6A, $05, $89, $E3, $B8, $46, $00, $00, $00, $CD,
  $40, $83, $C4, $19, $5B, $C2, $08, $00
);

// Function  WriteFile(Path: PAnsiChar; Const Buffer; Count, LoPos, HiPos: LongInt; Var BytesWritten: LongInt): LongInt; StdCall;
_WriteFile: array[0..49] of Byte = (
  $53, $FF, $74, $24, $08, $4C, $C6, $04, $24, $00, $FF, $74, $24, $11, $FF,
  $74, $24, $19, $FF, $74, $24, $25, $FF, $74, $24, $25, $6A, $03, $89, $E3,
  $B8, $46, $00, $00, $00, $CD, $40, $83, $C4, $19, $8B, $4C, $24, $1C, $89,
  $19, $5B, $C2, $18, $00
);
  
// Function  ReadFile(Path: PAnsiChar; Var Buffer; Count, LoPos, HiPos: LongInt; Var BytesRead: LongInt): LongInt; StdCall;
_ReadFile: array[0..49] of Byte = (
  $53, $FF, $74, $24, $08, $4C, $C6, $04, $24, $00, $FF, $74, $24, $11, $FF,
  $74, $24, $19, $FF, $74, $24, $25, $FF, $74, $24, $25, $6A, $00, $89, $E3,
  $B8, $46, $00, $00, $00, $CD, $40, $83, $C4, $19, $8B, $4C, $24, $1C, $89,
  $19, $5B, $C2, $18, $00
);


var
  StdInputFileHandle:  TInternalFileHandle = (FilePointer: 0);
  StdOutputFileHandle: TInternalFileHandle = (FilePointer: 0);
  
function GetStdHandle(nStdHandle: Integer): LongInt;
begin
// actually KolibriOS has no any stdinput\stdoutput
// but we need it somehow
// just emulate them
  case nStdHandle of
    STD_INPUT_HANDLE:  Result := LongInt(@StdInputFileHandle);
    STD_OUTPUT_HANDLE: Result := LongInt(@StdOutputFileHandle);
  end;
end;




// Initialization


procedure InitSystem;
var
  FileRecPtr: PFileRec;
begin
  LoadLibrary       := TLoadLibraryProc(@_LoadLibrary);
  GetProcAddress    := TGetProcAddressProc(@_GetProcAddress);
  TerminateThread   := TTerminateThreadProc(@_TerminateThread);
  ProcessArgs       := TProcessArgsProc(@_ProcessArgs);  
  RDTSC             := TRDTSCProc(@_RDTSC);  
  HeapAllocate      := THeapAllocateProc(@_HeapAllocate);
  HeapFree          := THeapFreeProc(@_HeapFree);    
  //HeapCreate        := THeapCreateProc(@_HeapCreate);
  CreateFile        := TCreateFileProc(@_CreateFile);
  GetFileAttributes := TGetFileAttributesProc(@_GetFileAttributes);
  WriteFile         := TWriteFileProc(@_WriteFile);
  ReadFile          := TReadFileProc(@_ReadFile);
  
  ProcessArgs(ArgCount, ArgValues);

  StdInputHandle := GetStdHandle(STD_INPUT_HANDLE);
  FileRecPtr := PFileRec(@StdInputFile);
  FileRecPtr^.Handle := StdInputHandle;
  
  StdOutputHandle := GetStdHandle(STD_OUTPUT_HANDLE);
  FileRecPtr := PFileRec(@StdOutputFile);
  FileRecPtr^.Handle := StdOutputHandle;

{$IFDEF CONSOLE}
  hConsole      := LoadLibrary('/sys/lib/console.obj');
  ConsoleInit   := GetProcAddress(hConsole, 'con_init');
  ConsoleExit   := GetProcAddress(hConsole, 'con_exit');
  WriteConsole  := GetProcAddress(hConsole, 'con_write_string');
  ReadConsole   := GetProcAddress(hConsole, 'con_gets');

  //ConsoleInit($ffffffff, $ffffffff, $ffffffff, $ffffffff, PPAnsiChar(32)^);
{$ELSE}
   //HeapCreate();
{$ENDIF}
end;


procedure EnsureConsoleLoaded;
begin
  if ConsoleInit <> nil then
  begin
    ConsoleInit($ffffffff, $ffffffff, $ffffffff, $ffffffff, PPAnsiChar(32)^);
    ConsoleInit := nil;
  end;
end;


procedure ExitProcess;
begin
{$IFDEF CONSOLE}
  if ConsoleInit = nil then
    ConsoleExit(False);
{$ENDIF}  
  TerminateThread();
end;




function GetTickCount: LongInt;
begin
//  Result := ;
end;




// Timer

{
function Timer: LongInt;
begin
// in KolibriOS we have GetTickCount and GetTickCount64
// the first one returns hundredths of second
// the second returns nanoseconds
// but not the milliseconds
// which of one should be used here i dont know
Result := GetTickCount;
end;
}



// Heap routines


procedure GetMem(var P: Pointer; Size: Integer);
begin
P := HeapAllocate(Size);
end;




procedure FreeMem(var P: Pointer);
begin
HeapFree(P);
end;




// Random number generator routines


procedure Randomize;
begin
RandSeed := RDTSC();
end;




function Random: Real;
begin
RandSeed := 1975433173 * RandSeed;
Result := 0.5 * (RandSeed / $7FFFFFFF + 1.0);
end;




// String manipulation routines


function Length(const s: string): Integer;
begin
Result := 0;
while s[Result + 1] <> #0 do Inc(Result);
end;




procedure SetLength(var s: string; NewLength: Integer);
begin
if NewLength >= 0 then s[NewLength + 1] := #0;
end;




procedure AssignStr(var Dest: string; const Source: string);
begin
Move(Source, Dest, Length(Source) + 1);
end;




procedure AppendStr(var Dest: string; const Source: string);
var
  DestLen, i: Integer;
begin
DestLen := Length(Dest);
i := 0;
repeat 
  Inc(i);
  Dest[DestLen + i] := Source[i];
until Source[i] = #0;
end;




procedure ConcatStr(const s1, s2: string; var s: string);
begin
s := s1;
AppendStr(s, s2);
end;




function CompareStr(const s1, s2: string): Integer;
var
  i: Integer;
begin
Result := 0;
i := 0;
repeat 
  Inc(i);
  Result := Integer(s1[i]) - Integer(s2[i]);
until (s1[i] = #0) or (s2[i] = #0) or (Result <> 0);
end;




procedure Move(var Source; var Dest; Count: Integer);
var
  S, D: ^string;
  i: Integer;
begin
S := @Source;
D := @Dest;

if S = D then Exit;

if (S > D) or (LongInt(D) > LongInt(S) + Count) then
  for i := 1 to Count do  
    D^[i] := S^[i]
else
  for i := Count downto 1 do
    D^[i] := S^[i]; 
end;




function Copy(const S: string; Index, Count: Integer): string;
begin
Move(S[Index], Result, Count);
Result[Count + 1] := #0;  
end;




procedure FillChar(var Data; Count: Integer; Value: Char);
var
  D: ^string;
  i: Integer;
begin
D := @Data;
for i := 1 to Count do
  D^[i] := Value;
end;




function ParamCount: LongInt;
begin
  if ArgCount > 1 then
    Result := ArgCount - 1
  else
    Result := 0;
end;




function ParamStr(Index: LongInt): string;
begin
  if Index < ArgCount then
    Result := PString(ArgValues[Index])^
  else
    Result := '';
end;




// File and console I/O routines



procedure Assign(var F: file; const Name: string);
var
  FileRecPtr: PFileRec;
begin
FileRecPtr := PFileRec(@F);
FileRecPtr^.Name := Name;
end;




procedure Rewrite(var F: file; BlockSize: Integer = 1);
var
  FileRecPtr: PFileRec;
begin
FileRecPtr := PFileRec(@F);

if CreateFile(FileRecPtr^.Name) <> 0 then
  FileRecPtr^.Handle := INVALID_HANDLE_VALUE
else
  // the KolibriOS currently has no file Handles
  // but we need return something unique
  // also need storage for FilePointer
  FileRecPtr^.Handle := LongInt(HeapAllocate(SizeOf(TInternalFileHandle)));

if FileRecPtr^.Handle = INVALID_HANDLE_VALUE then IOError := -2;
end;




procedure Reset(var F: file; BlockSize: Integer = 1);
var
  FileRecPtr: PFileRec;
  FileAttributes: TFileAttributes;  
begin
FileRecPtr := PFileRec(@F);

if GetFileAttributes(FileRecPtr^.Name, FileAttributes) <> 0 then
  FileRecPtr^.Handle := INVALID_HANDLE_VALUE
else
  // the KolibriOS currently has no file Handles
  // but we need return something unique
  // also need storage for FilePointer  
  FileRecPtr^.Handle := LongInt(HeapAllocate(SizeOf(TInternalFileHandle)));

if FileRecPtr^.Handle = INVALID_HANDLE_VALUE then IOError := -2;
end;




procedure Close(var F: file);
var
  FileRecPtr: PFileRec;
begin
  FileRecPtr := PFileRec(@F);
  // free memory which was allocated in Rewrite\Reset procedures
  HeapFree(PInternalFileHandle(FileRecPtr^.Handle));
end;



  
procedure BlockWrite(var F: file; var Buf; Len: Integer);
var
  FileRecPtr: PFileRec;
  LenWritten: Integer;
begin
  FileRecPtr := PFileRec(@F);
  
  if FileRecPtr^.Handle <> StdOutputHandle then
  begin
    if WriteFile(FileRecPtr^.Name, Buf, Len, PInternalFileHandle(FileRecPtr^.Handle)^.FilePointer, 0, LenWritten) <> 0 then
      IOError := -2;
    PInternalFileHandle(FileRecPtr^.Handle)^.FilePointer := PInternalFileHandle(FileRecPtr^.Handle)^.FilePointer + LenWritten;
  end
  else
  begin
    EnsureConsoleLoaded;
    WriteConsole(@Buf, Len);
    PInternalFileHandle(FileRecPtr^.Handle)^.FilePointer := PInternalFileHandle(FileRecPtr^.Handle)^.FilePointer + Len;
  end;
end;




procedure BlockRead(var F: file; var Buf; Len: Integer; var LenRead: Integer);
var
  FileRecPtr: PFileRec;
begin
  FileRecPtr := PFileRec(@F);
  
  if FileRecPtr^.Handle <> StdInputHandle then
  begin
    if ReadFile(FileRecPtr^.Name, Buf, Len, PInternalFileHandle(FileRecPtr^.Handle)^.FilePointer, 0, LenRead) <> 0 then
      IOError := -2;      
    PInternalFileHandle(FileRecPtr^.Handle)^.FilePointer := PInternalFileHandle(FileRecPtr^.Handle)^.FilePointer + LenRead;
   end
   else
   begin
     EnsureConsoleLoaded;
     ReadConsole(@Buf, Len);
     LenRead := 0; while PString(@Buf)^[LenRead] <> #0 do Inc(LenRead);
     PInternalFileHandle(FileRecPtr^.Handle)^.FilePointer := PInternalFileHandle(FileRecPtr^.Handle)^.FilePointer + LenRead;
   end;
end;




procedure Seek(var F: file; Pos: Integer);
var
  FileRecPtr: PFileRec;
begin
FileRecPtr := PFileRec(@F);
PInternalFileHandle(FileRecPtr^.Handle)^.FilePointer := Pos;
end;




function FileSize(var F: file): Integer;
var
  FileRecPtr: PFileRec;
  FileAttributes: TFileAttributes;
begin
FileRecPtr := PFileRec(@F);
GetFileAttributes(FileRecPtr^.Name, FileAttributes);
Result := FileAttributes.SizeLo;
end;




function FilePos(var F: file): Integer;
var
  FileRecPtr: PFileRec;
begin
FileRecPtr := PFileRec(@F);
Result := PInternalFileHandle(FileRecPtr^.Handle)^.FilePointer;
end;




function EOF(var F: file): Boolean;
var
  FileRecPtr: PFileRec;
begin
FileRecPtr := PFileRec(@F);
if (FileRecPtr^.Handle = StdInputHandle) or (FileRecPtr^.Handle = StdOutputHandle) then
  Result := FALSE
else
  Result := FilePos(F) >= FileSize(F);
end;




function IOResult: Integer;
begin
Result := IOError;
IOError := 0;
end;




procedure WriteRec(var F: file; P: PStream; var Buf; Len: Integer);
begin
BlockWrite(F, Buf, Len);
end;




procedure WriteCh(var F: file; P: PStream; ch: Char);
var
  Dest: PChar;
begin 
if P = nil then                                     // Console or file output
  BlockWrite(F, ch, 1)
else                                                // String stream output 
  begin                      
  Dest := PChar(Integer(P^.Data) + P^.Index);
  Dest^ := ch;
  Inc(P^.Index);
  end  
end;




procedure WriteString(var F: file; P: PStream; const S: string);
var
  Dest: PChar;
begin
if P = nil then                                     // Console or file output
  BlockWrite(F, S, Length(S))
else                                                // String stream output
  begin                      
  Dest := PChar(Integer(P^.Data) + P^.Index);
  Move(S, Dest^, Length(S));
  P^.Index := P^.Index + Length(S);
  end 
end;




procedure WriteStringF(var F: file; P: PStream; const S: string; MinWidth, DecPlaces: Integer);
var
  Spaces: string;
  i, NumSpaces: Integer;
begin
NumSpaces := MinWidth - Length(S);
if NumSpaces < 0 then NumSpaces := 0;

for i := 1 to NumSpaces do
  Spaces[i] := ' ';
Spaces[NumSpaces + 1] := #0;  
  
WriteString(F, P, Spaces + S);
end;




function WriteInt(var F: file; P: PStream; Number: Integer): Integer;
var
  Digit, Weight: Integer;
  Skip: Boolean;

begin
// Returns the string length

if Number = 0 then
  begin
  WriteCh(F, P,  '0');
  Result := 1;
  end
else
  begin
  Result := 0;
  if Number < 0 then
    begin
    WriteCh(F, P,  '-');
    Inc(Result);
    Number := -Number;
    end;

  Weight := 1000000000;
  Skip := TRUE;

  while Weight >= 1 do
    begin
    if Number >= Weight then Skip := FALSE;

    if not Skip then
      begin
      Digit := Number div Weight;
      WriteCh(F, P,  Char(ShortInt('0') + Digit));
      Inc(Result);
      Number := Number - Weight * Digit;
      end;

    Weight := Weight div 10;
    end; // while
  end; // else

end;




procedure WriteIntF(var F: file; P: PStream; Number: Integer; MinWidth, DecPlaces: Integer);
var
  S: string;
begin
IStr(Number, S);
WriteStringF(F, P, S, MinWidth, DecPlaces);
end;
  



procedure WritePointer(var F: file; P: PStream; Number: Integer);
var
  i, Digit: ShortInt;
begin
for i := 7 downto 0 do
  begin
  Digit := (Number shr (i shl 2)) and $0F;
  if Digit <= 9 then Digit := ShortInt('0') + Digit else Digit := ShortInt('A') + Digit - 10;
  WriteCh(F, P,  Char(Digit));
  end; 
end;




procedure WritePointerF(var F: file; P: PStream; Number: Integer; MinWidth, DecPlaces: Integer);
var
  S: string;
begin
PtrStr(Number, S);
WriteStringF(F, P, S, MinWidth, DecPlaces);
end;




function WriteReal(var F: file; P: PStream; Number: Real; MinWidth, DecPlaces: Integer): Integer;
const
  MaxDecPlaces = 16;
  ExponPlaces = 3;
  
var
  Integ, Digit, IntegExpon: Integer;
  Expon, Frac: Real;
  WriteExpon: Boolean;

begin
// Returns the string length
Result := 0;

Expon := ln(abs(Number)) / ln(10);
WriteExpon := (DecPlaces = 0) or (Expon > 9);

// Write sign
if Number < 0 then
  begin
  WriteCh(F, P,  '-');
  Inc(Result);
  Number := -Number;
  end
else if WriteExpon then
  begin
  WriteCh(F, P,  ' ');
  Inc(Result);
  end;  
  
// Normalize number
if not WriteExpon then
  begin
  IntegExpon := 0;
  if DecPlaces > MaxDecPlaces then DecPlaces := MaxDecPlaces;
  end
else  
  begin
  DecPlaces := MaxDecPlaces;
  
  if Number = 0 then 
    IntegExpon := 0 
  else 
    begin
    IntegExpon := Trunc(Expon);
    Number := Number / exp(IntegExpon * ln(10));
    
    if Number >= 10 then
      begin
      Number := Number / 10;
      Inc(IntegExpon);
      end
    else if Number < 1 then
      begin
      Number := Number * 10;
      Dec(IntegExpon);    
      end;
    end;  
  end;

// Write integer part
Integ := Trunc(Number);
Frac  := Number - Integ;

Result := Result + WriteInt(F, P, Integ);

// Write decimal separator  
WriteCh(F, P, DecimalSeparator);
Inc(Result);

// Truncate fractional part if needed
if (MinWidth > 0) and WriteExpon and (Result + DecPlaces + 2 + ExponPlaces > MinWidth) then  // + 2 for "e+" or "e-"
  begin
  DecPlaces := MinWidth - Result - 2 - ExponPlaces;
  if DecPlaces < 1 then DecPlaces := 1;
  end;
  
// Write fractional part
while DecPlaces > 0 do
  begin
  Frac := Frac * 10;
  Digit := Trunc(Frac);
  if Digit > 9 then Digit := 9;
  
  WriteCh(F, P,  Char(ShortInt('0') + Digit));
  Inc(Result);
  
  Frac := Frac - Digit;  
  Dec(DecPlaces);
  end; // while

// Write exponent
if WriteExpon then 
  begin
  WriteCh(F, P, 'e');

  if IntegExpon >= 0 then
    WriteCh(F, P, '+')
  else
    begin
    WriteCh(F, P, '-');  
    IntegExpon := -IntegExpon;
    end;
    
  // Write leading zeros
  if IntegExpon < 100 then WriteCh(F, P, '0');
  if IntegExpon <  10 then WriteCh(F, P, '0');
  
  WriteInt(F, P, IntegExpon);     
  Result := Result + 2 + ExponPlaces; 
  end;
 
end;




procedure WriteRealF(var F: file; P: PStream; Number: Real; MinWidth, DecPlaces: Integer);
var
  S: string;
begin
Str(Number, S, MinWidth, DecPlaces);
WriteStringF(F, P, S, MinWidth, DecPlaces);
end;




procedure WriteBoolean(var F: file; P: PStream; Flag: Boolean);
begin
if Flag then WriteString(F, P, 'TRUE') else WriteString(F, P, 'FALSE');
end;




procedure WriteBooleanF(var F: file; P: PStream; Flag: Boolean; MinWidth, DecPlaces: Integer);
begin
if Flag then WriteStringF(F, P, 'TRUE', MinWidth, DecPlaces) else WriteStringF(F, P, 'FALSE', MinWidth, DecPlaces);
end;




procedure WriteNewLine(var F: file; P: PStream);
begin
WriteCh(F, P, #13);  WriteCh(F, P, #10);
end;




procedure ReadRec(var F: file; P: PStream; var Buf; Len: Integer);
var
  LenRead: Integer;
begin
BlockRead(F, Buf, Len, LenRead);
end;




procedure ReadCh(var F: file; P: PStream; var ch: Char);
var
  Len: Integer;
  Dest: PChar;
  FileRecPtr: PFileRec;
  
begin
FileRecPtr := PFileRec(@F);
   
if P <> nil then                                       // String stream input
  begin                      
  Dest := PChar(Integer(P^.Data) + P^.Index);
  ch := Dest^;
  Inc(P^.Index);
  end
else if FileRecPtr^.Handle = StdInputHandle then       // Console input
  begin
  if StdInputBufferPos > Length(StdInputBuffer) then
    begin
    BlockRead(F, StdInputBuffer, SizeOf(StdInputBuffer) - 1, Len);
    StdInputBuffer[Len] := #0;   // Replace LF with end-of-string
    StdInputBufferPos := 1;
    end;
  
  ch := StdInputBuffer[StdInputBufferPos];
  Inc(StdInputBufferPos);
  end 
else                                                   // File input
  begin
  BlockRead(F, ch, 1, Len);
  if ch = #10 then BlockRead(F, ch, 1, Len);
  if Len <> 1 then ch := #0;
  end;

LastReadChar := ch;                                    // Required by ReadNewLine
end;




procedure ReadInt(var F: file; P: PStream; var Number: Integer);
var
  Ch: Char;
  Negative: Boolean;

begin
Number := 0;

// Skip spaces
repeat ReadCh(F, P, Ch) until (Ch = #0) or (Ch > ' ');

// Read sign  
Negative := FALSE; 
if Ch = '+' then
  ReadCh(F, P, Ch)
else if Ch = '-' then   
  begin
  Negative := TRUE;
  ReadCh(F, P, Ch);
  end;

// Read number
while (Ch >= '0') and (Ch <= '9') do
  begin
  Number := Number * 10 + ShortInt(Ch) - ShortInt('0');
  ReadCh(F, P, Ch);
  end; 

if Negative then Number := -Number;
end;




procedure ReadSmallInt(var F: file; P: PStream; var Number: SmallInt);
var
  IntNumber: Integer;
begin
ReadInt(F, P, IntNumber);
Number := IntNumber;
end;
  



procedure ReadShortInt(var F: file; P: PStream; var Number: ShortInt);
var
  IntNumber: Integer;
begin
ReadInt(F, P, IntNumber);
Number := IntNumber;
end;




procedure ReadWord(var F: file; P: PStream; var Number: Word);
var
  IntNumber: Integer;
begin
ReadInt(F, P, IntNumber);
Number := IntNumber;
end;




procedure ReadByte(var F: file; P: PStream; var Number: Byte);
var
  IntNumber: Integer;
begin
ReadInt(F, P, IntNumber);
Number := IntNumber;
end;




procedure ReadBoolean(var F: file; P: PStream; var Value: Boolean);
var
  IntNumber: Integer;
begin
ReadInt(F, P, IntNumber);
Value := IntNumber <> 0;
end;




procedure ReadReal(var F: file; P: PStream; var Number: Real);
var
  Ch: Char;
  Negative, ExponNegative: Boolean;
  Weight: Real;
  Expon: Integer;
 
begin
Number := 0;
Expon := 0;

// Skip spaces
repeat ReadCh(F, P, Ch) until (Ch = #0) or (Ch > ' ');

// Read sign
Negative := FALSE;
if Ch = '+' then
  ReadCh(F, P, Ch)
else if Ch = '-' then   
  begin
  Negative := TRUE;
  ReadCh(F, P, Ch);
  end;

// Read integer part
while (Ch >= '0') and (Ch <= '9') do
  begin
  Number := Number * 10 + ShortInt(Ch) - ShortInt('0');
  ReadCh(F, P, Ch);
  end;

if Ch = DecimalSeparator then        // Fractional part found
  begin
  ReadCh(F, P, Ch);

  // Read fractional part
  Weight := 0.1;
  while (Ch >= '0') and (Ch <= '9') do
    begin
    Number := Number + Weight * (ShortInt(Ch) - ShortInt('0'));
    Weight := Weight / 10;
    ReadCh(F, P, Ch);
    end;
  end;

if (Ch = 'E') or (Ch = 'e') then     // Exponent found
  begin
  // Read exponent sign
  ExponNegative := FALSE;
  ReadCh(F, P, Ch);
  if Ch = '+' then
    ReadCh(F, P, Ch)
  else if Ch = '-' then   
    begin
    ExponNegative := TRUE;
    ReadCh(F, P, Ch);
    end;

  // Read exponent
  while (Ch >= '0') and (Ch <= '9') do
    begin
    Expon := Expon * 10 + ShortInt(Ch) - ShortInt('0');
    ReadCh(F, P, Ch);
    end;

  if ExponNegative then Expon := -Expon;
  end;
     
if Expon <> 0 then Number := Number * exp(Expon * ln(10));
if Negative then Number := -Number;
end;




procedure ReadSingle(var F: file; P: PStream; var Number: Single);
var
  RealNumber: Real;
begin
ReadReal(F, P, RealNumber);
Number := RealNumber;
end;




procedure ReadString(var F: file; P: PStream; var s: string);
var
  i: Integer;
  Ch: Char;
begin
i := 1;
ReadCh(F, P, Ch);

// while Ch <> #13 do // !!! in KolibriOS Console LF only, not CR
while Ch <> #10 do
  begin
  s[i] := Ch;
  Inc(i);
  ReadCh(F, P, Ch);
  end;

s[i] := #0;
end;




procedure ReadNewLine(var F: file; P: PStream);
var
  Ch: Char;
begin
Ch := LastReadChar;
// while not EOF(F) and (Ch <> #13) do ReadCh(F, P, Ch); // !!! in KolibriOS Console LF only, not CR
while not EOF(F) and (Ch <> #10) do ReadCh(F, P, Ch);
LastReadChar := #0;
end;




// Conversion routines


procedure Val(const s: string; var Number: Real; var Code: Integer);
var
  Stream: TStream;
begin
Stream.Data := PChar(@s);
Stream.Index := 0;

ReadReal(StdInputFile, @Stream, Number);

if Stream.Index - 1 <> Length(s) then Code := Stream.Index else Code := 0;
end;




procedure Str(Number: Real; var s: string; MinWidth: Integer = 0; DecPlaces: Integer = 0);
var
  Stream: TStream;
begin
Stream.Data := PChar(@s);
Stream.Index := 0;

WriteReal(StdOutputFile, @Stream, Number, MinWidth, DecPlaces);
s[Stream.Index + 1] := #0;
end;




procedure IVal(const s: string; var Number: Integer; var Code: Integer);
var
  Stream: TStream;
begin
Stream.Data := PChar(@s);
Stream.Index := 0;

ReadInt(StdInputFile, @Stream, Number);

if Stream.Index - 1 <> Length(s) then Code := Stream.Index else Code := 0;
end;




procedure IStr(Number: Integer; var s: string);
var
  Stream: TStream;
begin
Stream.Data := PChar(@s);
Stream.Index := 0;

WriteInt(StdOutputFile, @Stream, Number);
s[Stream.Index + 1] := #0;
end;




procedure PtrStr(Number: Integer; var s: string);
var
  Stream: TStream;
begin
Stream.Data := PChar(@s);
Stream.Index := 0;

WritePointer(StdOutputFile, @Stream, Number);
s[Stream.Index + 1] := #0;
end;




function UpCase(ch: Char): Char;
begin
if (ch >= 'a') and (ch <= 'z') then
  Result := Chr(Ord(ch) - Ord('a') + Ord('A'))
else
  Result := ch;
end; 




// Set manipulation routines


procedure InitSet(var SetStorage: TSetStorage);
begin
FillChar(SetStorage, SizeOf(SetStorage), #0);
end;




procedure AddToSet(var SetStorage: TSetStorage; FromElement, ToElement: Integer);
var
  Element: Integer;
  ElementPtr: ^Integer;
begin
ElementPtr := @SetStorage[FromElement shr 5];
ElementPtr^ := ElementPtr^ or (1 shl (FromElement and 31));

if ToElement > FromElement then
  for Element := FromElement + 1 to ToElement do
    begin
    ElementPtr := @SetStorage[Element shr 5];
    ElementPtr^ := ElementPtr^ or (1 shl (Element and 31));
    end;
end;




function InSet(Element: Integer; var SetStorage: TSetStorage): Boolean;
begin
Result := SetStorage[Element shr 5] and (1 shl (Element and 31)) <> 0;  
end;




procedure SetUnion(const SetStorage1, SetStorage2: TSetStorage; var SetStorage: TSetStorage);
var
  i: Integer;
begin
for i := 0 to MaxSetIndex do
  SetStorage[i] := SetStorage1[i] or SetStorage2[i];
end;




procedure SetDifference(const SetStorage1, SetStorage2: TSetStorage; var SetStorage: TSetStorage);
var
  i: Integer;
begin
for i := 0 to MaxSetIndex do
  SetStorage[i] := SetStorage1[i] and not SetStorage2[i];
end; 




procedure SetIntersection(const SetStorage1, SetStorage2: TSetStorage; var SetStorage: TSetStorage);
var
  i: Integer;
begin
for i := 0 to MaxSetIndex do
  SetStorage[i] := SetStorage1[i] and SetStorage2[i];
end; 




function CompareSets(const SetStorage1, SetStorage2: TSetStorage): Integer;
var
  i: Integer;
begin
Result := 0;
for i := 0 to MaxSetIndex do
  if SetStorage1[i] <> SetStorage2[i] then
    begin
    Result := 1;
    Exit;
    end;
end; 




function TestSubset(const SetStorage1, SetStorage2: TSetStorage): Integer;
var
  IntersectionStorage: TSetStorage;
begin
SetIntersection(SetStorage1, SetStorage2, IntersectionStorage);
if CompareSets(SetStorage1, IntersectionStorage) = 0 then Result := -1 else Result := 1;
end;




function TestSuperset(const SetStorage1, SetStorage2: TSetStorage): Integer;
var
  IntersectionStorage: TSetStorage;
begin
SetIntersection(SetStorage1, SetStorage2, IntersectionStorage);
if CompareSets(SetStorage2, IntersectionStorage) = 0 then Result := 1 else Result := -1;
end; 
 
 
end.
