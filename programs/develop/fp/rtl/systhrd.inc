{}

{XXX: Thread vars & TLS}

const
  ThreadVarBlockSize: DWord = 0;
  TLSGrowFor = 4096;

type
  PTLSIndex = ^TTLSIndex;
  TTLSIndex = record
    CS: TRTLCriticalSection;
    Slots: array[0..TLSGrowFor - 1] of record
      TID: DWord;
      Value: Pointer;
    end;
  end;

var
  TLSKey: PTLSIndex;


function TLSAlloc(): PTLSIndex;
var
  I: DWord;
begin
  {New(Result);}
  Result := kos_alloc(SizeOf(TTLSIndex));
  InitCriticalSection(Result^.CS);
  {SetLength(Result^.Slots, TLSGrowFor);}
  for I := 0 to TLSGrowFor - 1 do
    Result^.Slots[I].TID := 0;
end;


function TLSFree(TLSIndex: PTLSIndex): Boolean;
begin
  DoneCriticalSection(TLSIndex^.CS);
  {SetLength(TLSIndex^.Slots, 0);
  Dispose(TLSIndex);}
  kos_free(TLSIndex);
  Result := True;
end;


procedure TLSSetValue(TLSIndex: PTLSIndex; Value: Pointer);
var
  TID, I, Count, Slot: DWord;
begin
  TID := GetCurrentThreadID();
  EnterCriticalSection(TLSIndex^.CS);

  Count := Length(TLSIndex^.Slots);
  Slot := Count;

  for I := 0 to Count - 1 do
  if TLSIndex^.Slots[I].TID = TID then
  begin
    Slot := I;
    Break;
  end else
  if TLSIndex^.Slots[I].TID = 0 then
    Slot := I;

  if Slot >= Count then
  begin
    Halt(123);
    {SetLength(TLSIndex^.Slots, Count + TLSGrowFor);
    FillChar(TLSIndex^.Slots[Count], TLSGrowFor * SizeOf(TLSIndex^.Slots[0]), #0);
    Slot := Count;}
  end;

  TLSIndex^.Slots[Slot].TID := TID;
  TLSIndex^.Slots[Slot].Value := Value;

  LeaveCriticalSection(TLSIndex^.CS);
end;


function TLSGetValue(TLSIndex: PTLSIndex): Pointer;
var
  TID, I, Count: DWord;
begin
  Result := nil;
  TID := GetCurrentThreadID();

  EnterCriticalSection(TLSIndex^.CS);

  Count := Length(TLSIndex^.Slots);

  for I := 0 to Count - 1 do
  if TLSIndex^.Slots[I].TID = TID then
  begin
    Result := TLSIndex^.Slots[I].Value;
    break;
  end;

  LeaveCriticalSection(TLSIndex^.CS);
end;


procedure SysInitThreadVar(var Offset: DWord; Size: DWord);
begin
  Offset := ThreadVarBlockSize;
  Inc(ThreadVarBlockSize, Size);
end;

procedure SysAllocateThreadVars;
var
  DataIndex: Pointer;
begin
  {DataIndex := GetMem(ThreadVarBlockSize);}
  DataIndex := kos_alloc(ThreadVarBlockSize);
  FillChar(DataIndex^, ThreadVarBlockSize, #0);
  TLSSetValue(TLSKey, DataIndex);
end;

function SysRelocateThreadVar(Offset: DWord): Pointer;
var
  DataIndex: Pointer;
begin
  DataIndex := TLSGetValue(TLSKey);
  if DataIndex = nil then
  begin
    SysAllocateThreadVars;
    DataIndex := TLSGetValue(TLSKey);
  end;
  Result := DataIndex + Offset;
end;

procedure SysReleaseThreadVars;
begin
  {FreeMem(TLSGetValue(TLSKey));}
  kos_free(TLSGetValue(TLSKey));
end;



{XXX: Thread}
type
  PThreadInfo = ^TThreadInfo;
  TThreadInfo = record
    Func: TThreadFunc;
    Arg: Pointer;
    StackSize: PtrUInt;
    Stack: Pointer;
  end;

procedure DoneThread;
begin
  SysReleaseThreadVars;
end;

procedure ThreadMain(ThreadInfo: PThreadInfo);
var
  Result: PtrInt;
begin
  SysAllocateThreadVars;
  with ThreadInfo^ do
  begin
    InitThread(StackSize);
    try
       Result := Func(Arg);
    except
      {TODO: Обработать ошибки}
      WriteLn(StdErr, 'Exception in thread');
    end;
    FreeMem(Stack);
  end;
  asm
    movl $-1, %eax
    int $0x40
  end;
end;

function SysBeginThread(sa: Pointer; StackSize: PtrUInt; ThreadFunction: TThreadFunc; Arg: Pointer; CreationFlags: DWord; var ThreadID: TThreadID): TThreadID;
{Stack, esp, ThreadInfo}

  procedure EntryThreadMain; assembler;
  asm
    movl %esp, %eax
    jmp ThreadMain
  end;

var
  Stack: Pointer;
  ThreadInfo: PThreadInfo;
begin
  if not IsMultiThread then
  begin
    TLSKey := TLSAlloc();
    InitThreadVars(@SysRelocateThreadVar); {XXX: must be @SysRelocateThreadvar}
    IsMultiThread := True;
  end;

  StackSize := (StackSize + 3) div 4;
  Stack := GetMem(StackSize + SizeOf(TThreadInfo));
  ThreadInfo := PThreadInfo(PByte(Stack) + StackSize);
  ThreadInfo^.Func := ThreadFunction;
  ThreadInfo^.Arg := Arg;
  ThreadInfo^.StackSize := StackSize;
  ThreadInfo^.Stack := Stack;
  ThreadID := kos_newthread(@EntryThreadMain, ThreadInfo);
  Result := ThreadID;
end;


procedure SysEndThread(ExitCode: DWord);
begin
  WriteLn('..SysEndThread');
  {TODO: SysEndThread}
  SysReleaseThreadVars;
end;


function SysSuspendThread(ThreadHandle: TThreadID): DWord;
begin
  {TODO: SysSuspendThread}
  Result := -1;
end;


function SysResumeThread(ThreadHandle: TThreadID): DWord;
begin
  {TODO: SysResumeThread}
  Result := -1;
end;


function SysKillThread(ThreadHandle: TThreadID): DWord;
begin
  if kos_killthread(ThreadHandle) then
    Result := 0 else
    Result := -1;
end;


procedure SysThreadSwitch;
begin
  {$ifdef EMULATOR}
  kos_delay(0);{$else}
  kos_switchthread();{$endif}
end;


function SysGetCurrentThreadID: TThreadID;
var
  ThreadInfo: TKosThreadInfo;
begin
  kos_threadinfo(@ThreadInfo);
  Result := ThreadInfo.ThreadID;
end;


{XXX: CriticalSection}
procedure SysInitCriticalSection(var CS);
begin
  PRTLCriticalSection(CS)^.OwningThread := -1;
end;

procedure SysDoneCriticalSection(var CS);
begin
  PRTLCriticalSection(CS)^.OwningThread := -1;
end;

procedure SysEnterCriticalSection(var CS);
var
  ThisThread: TThreadID;
begin
  ThisThread := GetCurrentThreadId();
  if PRTLCriticalSection(CS)^.OwningThread <> ThisThread then
    while PRTLCriticalSection(CS)^.OwningThread <> -1 do;
  PRTLCriticalSection(CS)^.OwningThread := ThisThread;
end;

procedure SysLeaveCriticalSection(var CS);
begin
  if PRTLCriticalSection(CS)^.OwningThread = GetCurrentThreadId() then
    PRTLCriticalSection(CS)^.OwningThread := -1;
end;


{TODO: RTLEvent}
function SysRTLEventCreate: PRTLEvent;
begin
  Result := nil;
end;

procedure SysRTLEventDestroy(State: PRTLEvent);
begin
end;



{$ifndef HAS_MT_MEMORYMANAGER}
var
  HeapMutex: TRTLCriticalSection;

procedure KosHeapMutexInit;
begin
  InitCriticalSection(HeapMutex);
end;

procedure KosHeapMutexDone;
begin
  DoneCriticalSection(HeapMutex);
end;

procedure KosHeapMutexLock;
begin
  EnterCriticalSection(HeapMutex);
end;

procedure KosHeapMutexUnlock;
begin
  LeaveCriticalSection(HeapMutex);
end;

const
  KosMemoryMutexManager: TMemoryMutexManager = (
    MutexInit: @KosHeapMutexInit;
    MutexDone: @KosHeapMutexDone;
    MutexLock: @KosHeapMutexLock;
    MutexUnlock: @KosHeapMutexUnlock);

procedure InitHeapMutexes;
begin
  SetMemoryMutexManager(KosMemoryMutexManager);
end;
{$endif HAS_MT_MEMORYMANAGER}


var
  KosThreadManager: TThreadManager;

procedure InitSystemThreads;
begin
  ThreadID := TThreadID(1);
  with KosThreadManager do
  begin
    InitManager            := nil;
    DoneManager            := nil;

    BeginThread            := @SysBeginThread;
    EndThread              := @SysEndThread;
    SuspendThread          := @SysSuspendThread;
    ResumeThread           := @SysResumeThread;
    KillThread             := @SysKillThread;
    ThreadSwitch           := @SysThreadSwitch;
    WaitForThreadTerminate := nil; //@NoWaitForThreadTerminate;
    ThreadSetPriority      := nil; //@NoThreadSetPriority;
    ThreadGetPriority      := nil; //@NoThreadGetPriority;

    GetCurrentThreadID     := @SysGetCurrentThreadID;
    InitCriticalSection    := @SysInitCriticalSection;
    DoneCriticalSection    := @SysDoneCriticalSection;
    EnterCriticalSection   := @SysEnterCriticalSection;
    LeaveCriticalSection   := @SysLeaveCriticalSection;
    InitThreadVar          := @SysInitThreadVar;
    RelocateThreadVar      := @SysRelocateThreadVar;
    AllocateThreadVars     := @SysAllocateThreadVars;
    ReleaseThreadVars      := @SysReleaseThreadVars;

    BasicEventCreate       := @NoBasicEventCreate;
    BasicEventDestroy      := @NoBasicEventDestroy;
    BasicEventResetEvent   := @NoBasicEventResetEvent;
    BasicEventSetEvent     := @NoBasicEventSetEvent;
    BasicEventWaitFor      := @NoBasicEventWaitFor;
    RTLEventCreate         := @SysRTLEventCreate;
    RTLEventDestroy        := @SysRTLEventDestroy;
    RTLEventSetEvent       := @NoRTLEventSetEvent;
    RTLEventWaitFor        := @NoRTLEventWaitFor;
    RTLEventSync           := @NoRTLEventSync;
    RTLEventWaitForTimeout := @NoRTLEventWaitForTimeout;

    SemaphoreInit          := @NoSemaphoreInit;
    SemaphoreDestroy       := @NoSemaphoreDestroy;
    SemaphoreWait          := @NoSemaphoreWait;
    SemaphorePost          := @NoSemaphorePost;
  end;
  SetThreadManager(KosThreadManager);
{$ifndef HAS_MT_MEMORYMANAGER}
  InitHeapMutexes;
{$endif HAS_MT_MEMORYMANAGER}
  ThreadID := GetCurrentThreadID;
end;
