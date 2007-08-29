unit sysutils;

{$i _defines.inc}

interface

{$mode objfpc}
{ force ansistrings }
{$h+}

{$DEFINE HAS_SLEEP}
{-$DEFINE HAS_OSERROR}
{-$DEFINE HAS_OSCONFIG}
{-$DEFINE HAS_CREATEGUID}


{ Include platform independent interface part }
{$i sysutilh.inc}

implementation


uses
  SysConst;


{-$define HASCREATEGUID}
{-$define HASEXPANDUNCFILENAME}
{-$DEFINE FPC_NOGENERICANSIROUTINES}
{-$DEFINE FPC_FEXPAND_UNC} (* UNC paths are supported *)
{-$DEFINE FPC_FEXPAND_DRIVES} (* Full paths begin with drive specification *)

{ Include platform independent implementation part }
{$i sysutils.inc}


{****************************************************************************
                              File Functions
****************************************************************************}

const
  FILEHANDLEPREFIX = $4000;
type
  PFileRecord = ^TFileRecord;
  TFileRecord = record
    Filled: Boolean;
    F: File;
  end;
var
  FileHandles: array of TFileRecord;

function FileRecordByHandle(Handle: THandle): PFileRecord;
begin
  Dec(Handle, FILEHANDLEPREFIX);
  Result := @FileHandles[Handle];
end;

function CreateFileRecord(): THandle;
var
  I, C: Longword;
begin
  Result := -1;
  C := Length(FileHandles);
  for I := 0 to C - 1 do
  if not FileHandles[I].Filled then
  begin
    Result := I;
    Break;
  end;
  if Result < 0 then
  begin
    SetLength(FileHandles, C + 1);
    Result := C;
  end;
  FileHandles[Result].Filled := True;
  FillChar(FileHandles[Result].F, SizeOf(FileRec), 0);
  Inc(Result, FILEHANDLEPREFIX);
end;

procedure ReleaseFileRecord(Handle: THandle);
begin
  FileRecordByHandle(Handle)^.Filled := False;
end;

function FileOpen(const FileName: String; Mode: Integer): THandle;
var
  F: File;
begin
  Filemode := Mode;
  Assign(F, FileName);
  Reset(F, 1);
  if InOutRes = 0 then
  begin
    Result := CreateFileRecord();
    FileRecordByHandle(Result)^.F := F;
  end else
    Result := feInvalidHandle;
end;

function FileCreate(const FileName: String): THandle;
var
  F: File;
begin
  Assign(F, FileName);
  Rewrite(F, 1);
  if InOutRes = 0 then
  begin
    Result := CreateFileRecord();
    FileRecordByHandle(Result)^.F := F;
  end else
    Result := feInvalidHandle;
end;

function FileCreate(const FileName: String; Mode: Integer): THandle;
var
  F: File;
begin
  Filemode := Mode;
  Assign(F, FileName);
  Rewrite(F, 1);
  if InOutRes = 0 then
  begin
    Result := CreateFileRecord();
    FileRecordByHandle(Result)^.F := F;
  end else
    Result := feInvalidHandle;
end;

function FileRead(Handle: THandle; var Buffer; Count: Longint): Longint;
begin
  BlockRead(FileRecordByHandle(Handle)^.F, Buffer, Count, Result);
end;

function FileWrite(Handle: THandle; const Buffer; Count: Longint): Longint;
begin
  BlockWrite(FileRecordByHandle(Handle)^.F, Buffer, Count, Result);
end;

function FileSeek(Handle: THandle; FOffset, Origin: Longint): Longint;
begin
  Result := FileSeek(Handle, Int64(FOffset), Origin);
end;

function FileSeek(Handle: THandle; FOffset: Int64; Origin: Longint): Int64;
var
  Position: Int64;
begin
  case Origin of
    fsFromBeginning: Position := FOffset;
    fsFromCurrent: Position := FilePos(FileRecordByHandle(Handle)^.F) + FOffset;
    fsFromEnd: Position := FileSize(FileRecordByHandle(Handle)^.F) + FOffset;
  end;
  {TODO: проверка соответствия [0..filesize]}
  Seek(FileRecordByHandle(Handle)^.F, Position);
  Result := Position;
end;

procedure FileClose(Handle: THandle);
begin
  Close(FileRecordByHandle(Handle)^.F);
  ReleaseFileRecord(Handle);
end;

function FileTruncate(Handle: THandle; Size: Longint): Boolean;
begin
  Result := False;
end;

function FileAge(const FileName: String): Longint;
begin
  Result := 0;
end;

function FileExists(const FileName: String): Boolean;
var
  F: File;
begin
  Assign(F, FileName);
  try
    Reset(F);
    FileSize(F);
    Result := True;
  except
    Result := False;
  end;
  Close(F);
end;

function DirectoryExists(const Directory: String): Boolean;
begin
  Result := False;
end;

function FindMatch(var f: TSearchRec): Longint;
begin
  Result := feInvalidHandle;
end;

function FindFirst(const Path: String; Attr: Longint; out Rslt: TSearchRec): Longint;
begin
  Result := feInvalidHandle;
end;

function FindNext(var Rslt: TSearchRec): Longint;
begin
  Result := feInvalidHandle;
end;

procedure FindClose(var F: TSearchrec);
begin
end;

function FileGetDate(Handle: THandle): Longint;
begin
  Result := feInvalidHandle;
end;

function FileSetDate(Handle: THandle; Age: Longint): Longint;
begin
  Result := feInvalidHandle;
end;

function FileGetAttr(const FileName: String): Longint;
begin
  Result := feInvalidHandle;
end;

function FileSetAttr(const Filename: String; Attr: longint): Longint;
begin
  Result := feInvalidHandle;
end;

function DeleteFile(const FileName: String): Boolean;
begin
  Result := False;
end;

function RenameFile(const OldName, NewName: String): Boolean;
begin
  Result := False;
end;


{****************************************************************************
                              Disk Functions
****************************************************************************}

function DiskFree(drive: Byte): Int64;
begin
  Result := 0;
end;

function DiskSize(drive: Byte): Int64;
begin
  Result := 0;
end;

function GetCurrentDir: String;
begin
  GetDir(0, Result);
end;

function SetCurrentDir(const NewDir: String): Boolean;
var
  Path: String;
begin
  ChDir(NewDir);
  GetDir(0, Path);
  Result := Path = NewDir;
end;

function CreateDir(const NewDir: String): Boolean;
begin
  Result := False;
end;

function RemoveDir(const Dir: String): Boolean;
begin
  Result := False;
end;


{****************************************************************************
                              Time Functions
****************************************************************************}

procedure GetLocalTime(var SystemTime: TSystemTime);
begin
end;


{****************************************************************************
                              Misc Functions
****************************************************************************}

procedure Beep;
begin
end;


{****************************************************************************
                              Locale Functions
****************************************************************************}

procedure GetFormatSettings;
var
  HF: String;
begin
  ShortMonthNames[1] := SShortMonthNameJan;
  ShortMonthNames[2] := SShortMonthNameFeb;
  ShortMonthNames[3] := SShortMonthNameMar;
  ShortMonthNames[4] := SShortMonthNameApr;
  ShortMonthNames[5] := SShortMonthNameMay;
  ShortMonthNames[6] := SShortMonthNameJun;
  ShortMonthNames[7] := SShortMonthNameJul;
  ShortMonthNames[8] := SShortMonthNameAug;
  ShortMonthNames[9] := SShortMonthNameSep;
  ShortMonthNames[10] := SShortMonthNameOct;
  ShortMonthNames[11] := SShortMonthNameNov;
  ShortMonthNames[12] := SShortMonthNameDec;

  LongMonthNames[1] := SLongMonthNameJan;
  LongMonthNames[2] := SLongMonthNameFeb;
  LongMonthNames[3] := SLongMonthNameMar;
  LongMonthNames[4] := SLongMonthNameApr;
  LongMonthNames[5] := SLongMonthNameMay;
  LongMonthNames[6] := SLongMonthNameJun;
  LongMonthNames[7] := SLongMonthNameJul;
  LongMonthNames[8] := SLongMonthNameAug;
  LongMonthNames[9] := SLongMonthNameSep;
  LongMonthNames[10] := SLongMonthNameOct;
  LongMonthNames[11] := SLongMonthNameNov;
  LongMonthNames[12] := SLongMonthNameDec;

  ShortDayNames[1] := SShortDayNameMon;
  ShortDayNames[2] := SShortDayNameTue;
  ShortDayNames[3] := SShortDayNameWed;
  ShortDayNames[4] := SShortDayNameThu;
  ShortDayNames[5] := SShortDayNameFri;
  ShortDayNames[6] := SShortDayNameSat;
  ShortDayNames[7] := SShortDayNameSun;

  LongDayNames[1] := SLongDayNameMon;
  LongDayNames[2] := SLongDayNameTue;
  LongDayNames[3] := SLongDayNameWed;
  LongDayNames[4] := SLongDayNameThu;
  LongDayNames[5] := SLongDayNameFri;
  LongDayNames[6] := SLongDayNameSat;
  LongDayNames[7] := SShortDayNameSun;

  DateSeparator := '/';
  ShortDateFormat := 'd/mm/yy';
  LongDateFormat := 'd mmmm yyyy';
  { Time stuff }
  TimeSeparator := ':';
  TimeAMString := 'AM';
  TimePMString := 'PM';
  HF := 'hh';
  // No support for 12 hour stuff at the moment...
  ShortTimeFormat := HF + ':nn';
  LongTimeFormat := HF + ':nn:ss';
  { Currency stuff }
  CurrencyString := '';
  CurrencyFormat := 0;
  NegCurrFormat := 0;
  { Number stuff }
  ThousandSeparator := ',';
  DecimalSeparator := '.';
  CurrencyDecimals := 2;
end;

Procedure InitInternational;
begin
  InitInternationalGeneric;
  GetFormatSettings;
end;


{****************************************************************************
                           Target Dependent
****************************************************************************}

function SysErrorMessage(ErrorCode: Integer): String;
const
  MaxMsgSize = 255;
var
  MsgBuffer: PChar;
begin
  GetMem(MsgBuffer, MaxMsgSize);
  FillChar(MsgBuffer^, MaxMsgSize, #0);
  {TODO}
  Result := StrPas(MsgBuffer);
  FreeMem(MsgBuffer, MaxMsgSize);
end;

{****************************************************************************
                              Initialization code
****************************************************************************}

Function GetEnvironmentVariable(Const EnvVar: String): String;
begin
  Result := '';
end;

Function GetEnvironmentVariableCount: Integer;
begin
  Result := 0;
end;

Function GetEnvironmentString(Index : Integer) : String;
begin
  Result := '';
end;

function ExecuteProcess(Const Path: AnsiString; Const ComLine: AnsiString): Integer;
begin
  Result := 0;
end;

function ExecuteProcess(Const Path: AnsiString; Const ComLine: Array of AnsiString): Integer;
var
  CommandLine: AnsiString;
  i: Integer;
begin
  Commandline:='';
  For i:=0 to high(ComLine) Do
   Commandline:=CommandLine+' '+Comline[i];
  ExecuteProcess:=ExecuteProcess(Path,CommandLine);
end;

procedure Sleep(Milliseconds: Cardinal);
begin
  kos_delay(Milliseconds div 10);
end;

function GetLastOSError: Integer;
begin
  Result := -1;
end;



initialization
  InitExceptions;
  InitInternational;
finalization
  DoneExceptions;
end.
