unit LRLEditor;

{$mode objfpc}
{$asmmode intel}


interface


uses
  LRLRoutines, LRLSprites, LRLLevels;


procedure LRLEditLevels;


implementation


const
  RefreshDelay = 5;  { 1 = 1/100 sec }

var
  CurrentLevel: Word;
  CurrentTool: Word;
  TotalLevels: Word;
  MouseX, MouseY: Integer;

  TimeToRefresh  : Boolean;
  LastRefreshTime: DWord;


procedure LRLRedrawLevel;
var
  i, j: Integer;
begin
  ImageClear(LRLScreen^);

  for i := 1 to 16 do for j := 1 to 30 do
  with LRLLevel.Field[j, i] do
    ImagePut(LRLScreen^, LRLEnvironment[Image].Image[Count].Data^, j * 10, i * 10, 0, 0, 319, 199);

  for i := 1 to 10 do
  with LRLLevel.Player[i] do
  if Controller <> 0 then
    ImagePutTransparent(LRLScreen^, LRLFigure[Colour, SpriteData].Image[Sprite].Data^, Position.x * 10 + Position.xoffs, Position.y * 10 + Position.yoffs, 0, 0, 319, 199);

  ImageFill(LRLFontBuffer^, 12, 12, 252);
  ImagePut(LRLScreen^, LRLFontBuffer^, 10, 184, 0, 0, 319, 199);

  for i := 1 to 13 do
  if i > 9 then
    ImagePut(LRLScreen^, LRLFigure[i - 9, 1].Image[1].Data^, i * 15 - 4, 185, 0, 0, 319, 199) else
    ImagePut(LRLScreen^, LRLEnvironment[i].Image[1].Data^, i * 15 - 4, 185, 0, 0, 319, 199);

  for i := 1 to 6 do
  if LRLEditorButton[i].Lit then
    ImagePut(LRLScreen^, LRLEditorButton[i].LightIcon^, LRLEditorButton[i].x1, LRLEditorButton[i].y1, 0, 0, 319, 199) else
    ImagePut(LRLScreen^, LRLEditorButton[i].DarkIcon^, LRLEditorButton[i].x1, LRLEditorButton[i].y1, 0, 0, 319, 199);

  LRLDrawOrnamental(0, 0, 31, 17, 1);
end;


procedure LRLMoveMouse;
var
  s, s2: String[20];
begin
  MSMouseGetXY(MouseX, MouseY);

  if not MSMouseInArea(200, 180, 325, 205) then
    if CurrentTool < 10 then
      ImagePut(LRLScreen^, LRLEnvironment[CurrentTool].Image[1].Data^, MouseX - 5, MouseY - 5, 0, 0, 319, 199) else
      ImagePut(LRLScreen^, LRLFigure[CurrentTool - 9, 1].Image[1].Data^, MouseX - 5, MouseY - 5, 0, 0, 319, 199);

  if not MSMouseInArea(-2, -2, 55, 20) then
  begin
    ImageFill(LRLFontBuffer^, 50, 15, 0);
    ImagePut(LRLScreen^, LRLFontBuffer^, 0, 0, 0, 0, 319, 199);
    Str(CurrentLevel, s);
    Str(TotalLevels, s2);
    ImageStringGet(s + '/' + s2, LRLFont^, LRLFontBuffer^, 251);
    ImagePut(LRLScreen^, LRLFontBuffer^, 25 - ImageSizex(LRLFontBuffer^) div 2, 0, 0, 0, 319, 199);
  end;

  ImagePutTransparent(LRLScreen^, LRLMousePointer^, MouseX, MouseY, 0, 0, 319, 199);
end;


procedure Repress;
var
  x, y: Integer;
begin
  MSMouseButtonWasPressed(0, x, y);
  MSMouseButtonWasReleased(0, x, y);
  MSMouseButtonWasPressed(1, x, y);
  MSMouseButtonWasReleased(1, x, y);
end;


procedure LRLEditLevels;
var
  Keypress: Word;
  DrawNow: Boolean;
  i, j: Integer;
  x, y: Integer;
  Cmd:  Word;
begin
  if not MSMouseDriverExist then Exit;
  Repress;
  Palette256Set(LRLPalette^);

  ShowLives := False;
  ShowScore := False;
  Keypress  := 0;
  CurrentLevel := 1;
  CurrentTool  := 2;
  TotalLevels  := LRLLevelCount;

  DrawNow := False;
  TimeToRefresh   := True;
  LastRefreshTime := 0;

  MSMouseSetXY(160, 100);
  LRLLoadLevel(CurrentLevel);

  WaitForEvent(1);

  repeat
    if not TimeToRefresh then
      TimeToRefresh := kos_timecounter() - LastRefreshTime >= RefreshDelay;

    if TimeToRefresh then
    begin
      LRLRedrawLevel;
      LRLMoveMouse;
      ScreenApply(LRLScreen^);
      TimeToRefresh   := False;
      LastRefreshTime := kos_timecounter();
    end;

    if Keypressed then
      Keypress := Readkey;

    if MSMouseButtonWasReleased(1, x, y) then
    begin
      LRLScore := 0;

      FadeClear;
      ImageClear(LRLScreen^);
      ScreenApply(LRLScreen^);
      Palette256Set(LRLPalette^);
      LRLPlayLevel(CurrentLevel);

      FadeClear;
      ImageClear(LRLScreen^);
      ScreenApply(LRLScreen^);
      Palette256Set(LRLPalette^);
      LRLLoadLevel(CurrentLevel);

      Repress;
    end;

    if MSMouseButtonWasPressed(0, x, y) then
      DrawNow := True;

    if MSMouseButtonWasReleased(0, x, y) then
    begin
      DrawNow := False;

      Cmd := 0;

      for i := 1 to 6 do
        LRLEditorButton[i].Lit := False;

      for i := 1 to 6 do
      if MSMouseInArea(LRLEditorButton[i].x1, LRLEditorButton[i].y1, LRLEditorButton[i].x2, LRLEditorButton[i].y2) then
      begin
        Cmd := LRLEditorButton[i].Command;
        Break;
      end;

      if Cmd = 1 then
      begin
        LRLSaveLevel(CurrentLevel);

        Repress;
      end;

      if Cmd = 2 then
      begin
        LRLInsertLevel(CurrentLevel);
        Inc(CurrentLevel);
        TotalLevels := LRLLevelCount;
        LRLLoadLevel(CurrentLevel);

        Repress;
      end;

      if (Cmd = 3) and (CurrentLevel < TotalLevels) then
      begin
        Inc(CurrentLevel);
        LRLLoadLevel(CurrentLevel);

        Repress;
      end;

      if Cmd = 4 then
      begin
        for i := 1 to 16 do for j := 1 to 30 do
          LRLLevel.Field[j, i].Image := 1;
        for i := 1 to 10 do
          LRLLevel.Player[i].Controller := 0;

        Repress;
      end;

      if (Cmd = 5) and (TotalLevels > 1) then
      begin
        LRLDeleteLevel(CurrentLevel);
        TotalLevels := LRLLevelCount;
        if CurrentLevel > TotalLevels then
          CurrentLevel := TotalLevels;
        LRLLoadLevel(CurrentLevel);

        Repress;
      end;

      if (Cmd = 6) and (CurrentLevel > 1) then
      begin
        Dec(CurrentLevel);
        LRLLoadLevel(CurrentLevel);

        Repress;
      end;

      MSMouseGetXY(MouseX, MouseY);

      if MouseY > 180 then
        for i := 1 to 13 do
        if (MouseY > 184) and (MouseY < 195) and (MouseX > i * 15 - 5) and (MouseX < i * 15 + 6) then
        begin
          CurrentTool := i;
          Break;
        end;
    end;

    if DrawNow then
    begin
      for i := 1 to 6 do
        LRLEditorButton[i].Lit := False;

      for i := 1 to 6 do
      if MSMouseInArea(LRLEditorButton[i].x1, LRLEditorButton[i].y1, LRLEditorButton[i].x2, LRLEditorButton[i].y2) then
      begin
        LRLEditorButton[i].Lit := True;
        Break;
      end;

      MSMouseGetXY(MouseX, MouseY);

      x := MouseX div 10;
      y := MouseY div 10;

      if (x > 0) and (x < 31) and (y > 0) and (y < 17) then
      begin
        for i := 1 to 10 do
        if (LRLLevel.Player[i].Controller <> 0) and (LRLLevel.Player[i].Position.x = x) and (LRLLevel.Player[i].Position.y = y) and
           (CurrentTool <> 2) and (CurrentTool <> 3) and (CurrentTool <> 4) and (CurrentTool <> 7) then
          begin
            LRLLevel.Player[i].Controller := 0;
            Break;
          end;

        if CurrentTool < 10 then
          LRLLevel.Field[x, y].Image := CurrentTool else
        if (LRLLevel.Field[x, y].Image = 2) or (LRLLevel.Field[x, y].Image = 3) or
           (LRLLevel.Field[x, y].Image = 4) or (LRLLevel.Field[x, y].Image = 1) then
        begin
          if CurrentTool = 10 then
          begin
            LRLLevel.Player[1].Controller := 1;
            LRLLevel.Player[1].Position.x := x;
            LRLLevel.Player[1].Position.y := y;
            LRLLevel.Player[1].Colour := 1;
          end else
          begin
            j := 2;
            for i := 2 to 10 do
            if LRLLevel.Player[i].Controller = 0 then
            begin
              j := i;
              Break;
            end;
            LRLLevel.Player[j].Controller := 2;
            LRLLevel.Player[j].Position.x := x;
            LRLLevel.Player[j].Position.y := y;
            LRLLevel.Player[j].Colour := CurrentTool - 9;
          end;
        end;
      end;
    end;

    WaitForEvent(5);

  until (Keypress = KEY_ESC);
end;


end.
