{}

type
  THandle = DWord;
  TThreadID = Longint;
  TThreadSlot = Longint;
  UINT = Cardinal;
  BOOL = Longbool;
  ULONG_PTR = DWord;
  SIZE_T = ULONG_PTR;

  PRTLCriticalSection = ^TRTLCriticalSection;
  TRTLCriticalSection = packed record
    OwningThread: TThreadID;
  end;
