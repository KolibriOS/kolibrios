{utf8}
unit System;

{$i _defines.inc}
{$define HAS_CMDLINE}

interface

{$i systemh.inc}
{$i kos_def.inc}
{$i kosh.inc}

const
  LineEnding = #13#10;
  LFNSupport = True;
  DirectorySeparator = '/';
  DriveSeparator = '/';
  PathSeparator = ';';
  MaxExitCode = 65535;
  MaxPathLen = 512;

  UnusedHandle   : THandle = -1;
  StdInputHandle : THandle = 0;
  StdOutputHandle: THandle = 0;
  StdErrorHandle : THandle = 0;
  FileNameCaseSensitive: Boolean = True;
  CtrlZMarksEOF: Boolean = True;
  sLineBreak = LineEnding;
  DefaultTextLineBreakStyle: TTextLineBreakStyle = tlbsCRLF;

var
  Argc: Longint = 0;
  Argv: PPChar = nil;

  Konsole: TKonsole;


implementation

var
  SysInstance: Longint; public name '_FPC_SysInstance';

{$i system.inc}


procedure SetupCmdLine;
var
  Ptrs: array of PChar;
  Args: PChar;
  InQuotes: Boolean;
  I, L: Longint;
begin
  Argc := 1;
  Args := PKosHeader(0)^.args;
  if Assigned(Args) then
  begin
    while Args^ <> #0 do
    begin
      {Пропустить лидирующие пробелы}
      while Args^ in [#1..#32] do Inc(Args);
      if Args^ = #0 then Break;

      {Запомнить указатель на параметр}
      SetLength(Ptrs, Argc);
      Ptrs[Argc - 1] := Args;
      Inc(Argc);

      {Пропустить текущий параметр}
      InQuotes := False;
      while (Args^ <> #0) and (not (Args^ in [#1..#32]) or InQuotes) do
      begin
        if Args^ = '"' then InQuotes := not InQuotes;
        Inc(Args);
      end;

      {Установить окончание параметра}
      if Args^ in [#1..#32] then
      begin
        Args^ := #0;
        Inc(Args);
      end;
    end;
  end;
  Argv := GetMem(Argc * SizeOf(PChar));  {XXX: память не освобождается}
  Argv[0] :=  PKosHeader(0)^.path;
  for I := 1 to Argc - 1 do
  begin
    Argv[I] := Ptrs[I - 1];
    {Исключить кавычки из строки}
    Args := Argv[I];
    L := 0;
    while Args^ <> #0 do begin Inc(Args); Inc(L); end;
    Args := Argv[I];
    while Args^ <> #0 do
    begin
      if Args^ = '"' then
      begin
        Move(PChar(Args + 1)^, Args^, L);
        Dec(L);
      end;
      Inc(Args);
      Dec(L);
    end;
  end;
end;

function ParamCount: Longint;
begin
  Result := Argc - 1;
end;

function ParamStr(L: Longint): String;
begin
  if (L >= 0) and (L < Argc) then
    Result := StrPas(Argv[L]) else
    Result := '';
end;

procedure Randomize;
begin
  randseed := kos_timecounter();
end;

const
  ProcessID: SizeUInt = 0;

function GetProcessID: SizeUInt;
begin
  GetProcessID := ProcessID;
end;

function CheckInitialStkLen(stklen: SizeUInt): SizeUInt;
begin
  {TODO}
  Result := stklen;
end;

{$i kos_stdio.inc}

procedure SysInitStdIO;
begin
  if IsConsole then
  begin
    AssignStdin(Input);
    AssignStdout(Output);
    AssignStdout(ErrOutput);
    AssignStdout(StdOut);
    AssignStdout(StdErr);
  end;
end;

procedure System_Exit; [public, alias: 'SystemExit'];
var
  event, count: DWord;
begin
  if IsConsole then
  begin
    if ExitCode <> 0 then
    begin
      {XXX: обязательное условие на однопоточный Konsole}
      Write(StdErr, '[Error #', ExitCode,', press any key]');
      {ожидать нажатия клавиши}
      Konsole.KeyPressed;
      while Konsole.KeyPressed = 0 do kos_delay(2);
      {TODO: исправить косяк при перерисовке Konsole}
      {это невозможно, так как куча освобождается еще до вызова этой процедуры}
      {можно написать свой диспетчер памяти, но это сложно}
      {а если в Konsole использовать выделение памяти напрямую через KosAPI?!}
    end;
    Close(StdErr);
    Close(StdOut);
    Close(ErrOutput);
    Close(Input);
    Close(Output);
    Konsole.Done();
  end;
  asm
    movl $-1, %eax
    int $0x40
  end;
end;

{$i kos.inc}

begin
  SysResetFPU;
  StackLength := CheckInitialStkLen(InitialStkLen);
  StackBottom := Pointer(StackTop - StackLength);
  kos_initheap();
  InitHeap;
  SysInitExceptions;
  FPC_CpuCodeInit();
  InOutRes := 0;
  InitSystemThreads;
  if IsConsole then
    Konsole.Init();
  SysInitStdIO;
  SetupCmdLine;
  InitVariantManager;
  {InitWideStringManager;}
  DispCallByIDProc := @DoDispCallByIDError;
end.
