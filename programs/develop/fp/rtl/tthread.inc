{ TODO }
{ Thread management routines }

type
  PRaiseFrame = ^TRaiseFrame;
  TRaiseFrame = record
    NextRaise: PRaiseFrame;
    ExceptAddr: Pointer;
    ExceptObject: TObject;
    ExceptionRecord: pointer; {PExceptionRecord}
  end;

var
  ThreadCount: Integer;


procedure AddThread;
begin
  InterlockedIncrement(ThreadCount);
end;

procedure RemoveThread;
begin
  InterlockedDecrement(ThreadCount);
end;

constructor TThread.Create(CreateSuspended: Boolean; const StackSize: SizeUInt = DefaultStackSize);
begin
  inherited Create;
  AddThread;
  FSuspended := CreateSuspended;
  {TODO}
  FFatalException := nil;
end;

destructor TThread.Destroy;
begin
  if not FFinished and not Suspended then
  begin
    Terminate;
    WaitFor;
  end;
  FFatalException.Free;
  FFatalException := nil;
  inherited Destroy;
  RemoveThread;
end;

procedure TThread.CallOnTerminate;
begin
  FOnTerminate(Self);
end;

procedure TThread.DoTerminate;
begin
  if Assigned(FOnTerminate) then
    Synchronize(@CallOnTerminate);
end;

function TThread.GetPriority: TThreadPriority;
begin
  {TODO}
end;

procedure TThread.SetPriority(Value: TThreadPriority);
begin
  {TODO}
end;

procedure TThread.SetSuspended(Value: Boolean);
begin
  if Value <> FSuspended then
    if Value then Suspend else Resume;
end;

procedure TThread.Suspend;
begin
  FSuspended := True;
  SuspendThread(FHandle);
end;

procedure TThread.Resume;
begin
  if ResumeThread(FHandle) = 1 then FSuspended := False;
end;

procedure TThread.Terminate;
begin
  FTerminated := True;
end;

function TThread.WaitFor: Integer;
begin
  {TODO}
end;
