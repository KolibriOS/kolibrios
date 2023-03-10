{}

procedure OpenStdout(var f: TextRec); forward;
procedure WriteStdout(var f: TextRec); forward;
procedure FlushStdout(var f: TextRec); forward;
procedure CloseStdout(var f: TextRec); forward;

procedure OpenStdin(var f: TextRec); forward;
procedure ReadStdin(var f: TextRec); forward;
procedure CloseStdin(var f: TextRec); forward;



procedure AssignStdout(var f: Text);
begin
  Assign(f, '');
  TextRec(f).OpenFunc := @OpenStdout;
  Rewrite(f);
end;

procedure OpenStdout(var f: TextRec);
begin
  TextRec(f).InOutFunc := @WriteStdout;
  TextRec(f).FlushFunc := @FlushStdout;
  TextRec(f).CloseFunc := @CloseStdout;
end;

procedure WriteStdout(var f: TextRec);
var
  msg: String;
begin
  msg := StrPas(PChar(f.bufptr));
  SetLength(msg, f.bufpos);
  f.bufpos := 0;
  Konsole.Write(msg);
end;

procedure FlushStdout(var f: TextRec);
begin
  WriteStdout(f);
  Konsole.Flush;
end;

procedure CloseStdout(var f: TextRec);
begin
end;



procedure AssignStdin(var f: Text);
begin
  Assign(f, '');
  TextRec(f).OpenFunc := @OpenStdin;
  Reset(f);
end;

procedure OpenStdin(var f: TextRec);
begin
  TextRec(f).InOutFunc := @ReadStdin;
  TextRec(f).FlushFunc := nil;
  TextRec(f).CloseFunc := @CloseStdin;
end;

procedure ReadStdin(var f: TextRec);
var
  max, curpos: Longint;
  c: Longint;
begin
  max := f.bufsize - Length(LineEnding);
  curpos := 0;
  repeat
    c := 13{l4_getc()};
    case c of
      13:
      begin
        {f.bufptr^[curpos] := LineEnding;}
        Inc(curpos);
        f.bufpos := 0;
        f.bufend := curpos;
        {l4_putc(Longint(LineEnding));}
        break;
      end;
      32..126: if curpos < max then
      begin
        f.bufptr^[curpos] := Char(c);
        Inc(curpos);
        {l4_putc(c);}
      end;
    end;
  until False;
end;

procedure CloseStdin(var f: TextRec);
begin
end;


{ TKonsole }

procedure KonsoleThreadMain(Console: PKonsole);
{????稩 横? ???᮫?}
var
  ThreadInfo: TKosThreadInfo;
  Message: ShortString;
  Event: DWord;
begin
  kos_maskevents(ME_PAINT or ME_KEYBOARD or ME_IPC);
  kos_threadinfo(@ThreadInfo);
  Console^.FThreadSlot := kos_getthreadslot(ThreadInfo.ThreadID);

  kos_initipc(Console^.FIPCBuffer, Console^.FIPCBufferSize);

  {?ࠧ? ?⮡ࠧ??? ? ??⨢?஢??? ????}
  Console^.Paint();
  {$ifndef EMULATOR}
  kos_setactivewindow(Console^.FThreadSlot);
  {$endif}

  {??⮢ ? ??ࠡ?⪥ ᮡ?⨩}
  Console^.FOpened := True;
  while not Console^.FTerminate do
  begin
    Event := kos_getevent();
    Console^.FOnAir := True;
    if not Console^.FTerminate then
    case Event of
      SE_PAINT: Console^.Paint();
      SE_KEYBOARD: Console^.ProcessKeyboard(kos_getkey());
      SE_IPC: while Console^.ReceiveMessage(Message) do Console^.ProcessMessage(Message);
    end;
    Console^.FOnAir := False;
  end;
  Console^.FOpened := False;
end;

constructor TKonsole.Init(ACaption: String);
const
  IPC_SIZE = 4096;
var
  ThreadInfo: TKosThreadInfo;
begin
  if ACaption <> '' then
    FCaption := ACaption else
  begin
    kos_threadinfo(@ThreadInfo);
    FCaption := StrPas(ThreadInfo.AppName);
  end;
  SetLength(FLines, 1);
  FLines[0] := '';
  FCursor.X := 1;
  FCursor.Y := 0;
  FMaxLines := 150;
  FTerminate := False;
  FOpened := False;
  FOnAir  := False;
  FIPCBufferSize := SizeOf(TKosIPC) + IPC_SIZE;
  FIPCBuffer := GetMem(FIPCBufferSize);
  FIPCBuffer^.Lock := False;
  FIPCBuffer^.Size := 0;
  FThreadSlot := -1;
  FThreadID := BeginThread(TThreadFunc(@KonsoleThreadMain), @Self);
  if FThreadID <> 0 then
    {XXX: ????? ?????????}
    while not FOpened do ThreadSwitch;
end;

destructor TKonsole.Done();
begin
  FTerminate := True;
  if FOpened then begin Self.Write(#0); kos_delay(01); end;
  if FOpened then begin Self.Write(#0); kos_delay(10); end;
  if FOpened then begin Self.Write(#0); kos_delay(20); end;
  if FOpened then
  begin
    FOpened := False;
    FOnAir  := False;
    KillThread(FThreadID);
  end;
  {FreeMem(FIPCBuffer);
  SetLength(FLines, 0);}
end;

function TKonsole.ReceiveMessage(var Message: ShortString): Boolean;
{??????? ??ࢮ? ᮮ?饭?? ?? ??????}
var
  PMsg: PKosMessage;
  Size: Longword;
begin
  FIPCBuffer^.Lock := True;

  if FIPCBuffer^.Size > 0 then
  begin
    PMsg := Pointer(Longword(FIPCBuffer) + SizeOf(TKosIPC));
    {TODO: ?஢?ઠ PMsg^.SenderID}
    {Size := PMsg^.Size;
    Dec(FIPCBuffer^.Size, Size + SizeOf(TKosMessage));
    if Size > 255 then Size := 255;
    SetLength(Message, Size);
    Move(Pointer(Longword(PMsg) + SizeOf(TKosMessage))^, Message[1], Size);
    if FIPCBuffer^.Size > 0 then
      Move(Pointer(Longword(PMsg) + SizeOf(TKosMessage) + PMsg^.Size)^, PMsg^, FIPCBuffer^.Size);}

    {XXX}
    Size := FIPCBuffer^.Size;
    Dec(FIPCBuffer^.Size, Size);
    if Size > 255 then Size := 255;
    SetLength(Message, Size);
    Move(PMsg^, Message[1], Size);

    Result := True;
  end else
  begin
    Message := '';
    Result := False;
  end;

  {FIXME: ?᫨ FIPCBuffer^.Size = 0, ?? FIPCBuffer^.Lock ??? ࠢ?? > 0}
  FIPCBuffer^.Lock := False;
end;

procedure TKonsole.ProcessMessage(Message: ShortString);
{ ?뢥??? ᮮ?饭?? ?? ???᮫? }
var
  OnlyBottomLine: Boolean = True;

  procedure PutChar(C: Char);
  var
    LinesCount: Longint;
    PLine: PShortString;
    I: Longint;
  begin
    { ??ॢ?? ????⪨ ?? ??????? ????? }
    if C = #8 then
    begin
      if FCursor.X > 1 then
        Dec(FCursor.X);
    end else

    { ??ॢ?? ????⪨ ?? ᫥?????? ??ப? }
    if C = #10 then
    begin
      OnlyBottomLine := False;
      Inc(FCursor.Y);
      LinesCount := Length(FLines);
      while FCursor.Y >= FMaxLines do Dec(FCursor.Y, FMaxLines);
      if FCursor.Y < LinesCount then FLines[FCursor.Y] := '';
      while FCursor.Y >= LinesCount do
      begin
        SetLength(FLines, LinesCount + 1);
        FLines[LinesCount] := '';
        Inc(LinesCount);
      end;
    end else

    { ??ॢ?? ????⪨ ? ??砫? ??ப? }
    if C = #13 then
      FCursor.X := 1 else

    { ????饭?? ᨬ???? ? ??ப? }
    begin
      if FCursor.X > 200 then
      begin
        PutChar(#13);
        PutChar(#10);
      end;

      { FIXME: ?᫨ ? PascalMain ⮫쪮 ???? Write/Ln, ?? ????ᮭ.
        ??. FPC_DO_EXIT, InternalExit }
      PLine := @FLines[FCursor.Y];
      I := Length(PLine^);
      if FCursor.X > I then
      begin
        SetLength(PLine^, FCursor.X);
        Inc(I);
        while I < FCursor.X do
        begin
          PLine^[I] := ' ';
          Inc(I);
        end;
      end;
      FLines[FCursor.Y][FCursor.X] := C;

      Inc(FCursor.X);
    end;
  end;

var
  I: Longint;
begin
  for I := 1 to Length(Message) do
    PutChar(Message[I]);
  Paint(OnlyBottomLine);
end;

procedure TKonsole.ProcessKeyboard(Key: Word);
begin
  FKeyPressed := Key;
end;

function TKonsole.GetRect(): TKosRect;
var
  ThreadInfo: TKosThreadInfo;
begin
  kos_threadinfo(@ThreadInfo, FThreadSlot);
  Result := ThreadInfo.WindowRect;
end;

function TKonsole.GetKeyPressed(): Word;
begin
  Result := FKeyPressed;
  FKeyPressed := 0;
end;

procedure TKonsole.Paint(BottomRow: Boolean);
var
  Buffer: array[Byte] of Char;
  Rect: TKosRect;
  J: Longint;
  Width, Height, Row: Longint;
  CaptionHeight, BorderWidth, FontWidth, FontHeight: Longint;
begin
  CaptionHeight := 16;
  BorderWidth := 5;
  FontWidth := 6;
  FontHeight := 9;

  kos_begindraw();

  if not BottomRow then
  begin
    {????ᮢ?? ????}
    kos_definewindow(60, 60, 400, 400, $63000000);
    {?뢮? ?????????}
    Move(FCaption[1], Buffer, Length(FCaption));
    Buffer[Length(FCaption)] := #0;
    kos_setcaption(Buffer);
  end;

  {?????⮢?? ? ?뢮?? ??ப}
  Rect := GetRect();
  Dec(Rect.Width, BorderWidth * 2);
  Dec(Rect.Height, CaptionHeight + BorderWidth * 2);
  Width  := Rect.Width div FontWidth;
  Height := Rect.Height - FontHeight;
  Row := FCursor.Y;

  while Height > 0 do
  begin
    {?뢮? ????? ??ப?}
    J := Length(FLines[Row]);
    if J > Width then J := Width;
    kos_drawtext(0, Height, Copy(FLines[Row], 1, J), $00DD00, $FF000000);
    {??????? ??⠢襣??? ?????࠭?⢠ ? ??ப?}
    J := J * FontWidth;
    kos_drawrect(J, Height, Rect.Width - J + 1, FontHeight, $000000);
    {?????⮢?? ? ?뢮?? ᫥???饩 ??ப?}
    Dec(Height, FontHeight);
    Dec(Row);
    if BottomRow or ((Row < 0) and (Length(FLines) < FMaxLines)) then Break;
    while Row < 0 do Inc(Row, FMaxLines);
  end;
  if FCursor.X <= Width then
    {????ᮢ?? ???????}
    kos_drawrect((FCursor.X - 1) * FontWidth, Rect.Height - 2, FontWidth, 2, $FFFFFF);
  if not BottomRow then
    {??????? ??⠢襩?? ????? ????}
    kos_drawrect(0, 0, Rect.Width + 1, Height + FontHeight, $000000);

  kos_enddraw();
end;

procedure TKonsole.Write(Message: ShortString);
var
  I: Integer;
begin
  {XXX: ???????? ???????? ??? ????ன ᮮ?饭?? ?? ?㤥? ???ࠢ????}
  if FOpened then
  begin
    I := 100;
    while (kos_sendmsg(FThreadID, @Message[1], Length(Message)) = 2) and (I > 0) do
    begin
      Dec(I);
      ThreadSwitch;
    end;
  end;
end;

procedure TKonsole.Flush();
begin
  while FOnAir do ThreadSwitch;
end;
