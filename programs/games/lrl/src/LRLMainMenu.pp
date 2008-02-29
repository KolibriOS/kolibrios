unit LRLMainMenu;


interface


uses
  LRLRoutines, LRLSprites;


procedure LRLSelectItem(var Item: Word);


implementation


var
  MainScreen: Pointer;
  Selection:  array[1..4] of Pointer;
  SelectionDark: array[1..4] of Pointer;
  SelectionSize: array[1..4] of Word;
  SelectionDarkSize: array[1..4] of Word;


procedure LoadData;
var
  j: Word;
  i: Integer;
begin
  GetMem(MainScreen, 64004);
  Seek(ImageFile, LRLImagesFilePosition + 7940);
  BlockRead(ImageFile, MainScreen^, 64004, j);
  for i := 1 to 4 do
  begin
    BlockRead(ImageFile, SelectionSize[i], 2, j);
    GetMem(Selection[i], SelectionSize[i]);
    BlockRead(ImageFile, Selection[i]^, SelectionSize[i], j);
    BlockRead(ImageFile, SelectionDarkSize[i], 2, j);
    GetMem(SelectionDark[i], SelectionDarkSize[i]);
    BlockRead(ImageFile, SelectionDark[i]^, SelectionDarkSize[i], j);
  end;
end;


procedure DisposeData;
var
  i: Integer;
begin
  FreeMem(MainScreen, 64004);
  for i := 1 to 4 do
  begin
    FreeMem(Selection[i], SelectionSize[i]);
    FreeMem(SelectionDark[i], SelectionDarkSize[i]);
  end;
end;


procedure LRLSelectItem(var Item: Word);
var
  Keypress: Word;
  RedrawAll: Boolean;
  NeedToFade: Boolean;
  p: Pointer;
  i: Integer;
begin
  GetMem(p, 768);
  DataFill(p^, 768, 0, 0);
  Palette256Set(p^);
  FreeMem(p, 768);

  LoadData;
  NeedToFade := True;
  ImagePut(LRLScreen^, MainScreen^, 0, 0, 0, 0, 319, 199);
  RedrawAll := True;
  KeyboardFlush;

  repeat
    if RedrawAll then
    begin
      for i := 1 to 4 do
      if i = Item then
        ImagePutTransparent(LRLScreen^, Selection[i]^, 63, 66 + (i - 1) * 30, 0, 0, 319, 199) else
        ImagePutTransparent(LRLScreen^, SelectionDark[i]^, 63, 66 + (i - 1) * 30, 0, 0, 319, 199);

      ScreenApply(LRLScreen^);

      if NeedToFade then
      begin
        FadeTo(LRLMenuPalette);
        NeedToFade := False;
      end;

      RedrawAll := False;
    end;

    Keypress := ReadKey;

    if (Keypress = KEY_DOWN) and (Item < 4) then
    begin
      Inc(Item);
      RedrawAll := True;
    end else
    if (Keypress = KEY_UP) and (Item > 1) then
    begin
      Dec(Item);
      RedrawAll := True;
    end;
  until Keypress = KEY_ENTER;

  FadeClear;
  ImageClear(LRLScreen^);
  ScreenApply(LRLScreen^);
  DisposeData;
end;


end.
