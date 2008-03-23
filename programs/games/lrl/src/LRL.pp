program LodeRunnerLive;

{$apptype gui}


uses
  LRLRoutines,
  LRLSprites,
  LRLLevels,
  LRLMainMenu,
  LRLHighScores,
  LRLEditor,
  LRLIntroduction;

const
  Version: PChar = 'Lode Runner LIVE. Version 1.5';


procedure LRLInitialize;
begin
  ImagesInitialize;
  KeyboardInitialize;
  MouseInitialize;
  ScreenMode(1);
  ScreenTitle := Version;
end;


procedure LRLDeinitialize;
begin
  ImagesDeinitialize;
end;


procedure LRLGameStart;
var
  cl: Integer;
begin
  Palette256Set(LRLPalette^);

  ShowLives := True;
  ShowScore := True;
  ShowLevel := True;
  LRLLives := 5;
  LRLScore := 0;

  cl := 1;
  repeat
    LRLPlayLevel(cl);
    KeyboardFlush;

    if GameResult = 10 then
    begin
      Inc(LRLLives);
      LRLScore := LRLScore + 10000 * Longint(cl);
      Inc(cl);
    end else
      Dec(LRLLives);
  until (LRLLives = 0) or (GameResult = 100);

  if (GameResult <> 100) and LRLBestScore(LRLScore) then
  begin
    LRLInsertScore(LRLEnterName, LRLScore);
    LRLShowHighScores;
  end;
end;

procedure LRLShell;
var
  MenuSelection: word;
begin
  MenuSelection := 1;
  repeat
    LRLSelectItem(MenuSelection);
    if MenuSelection = 1 then LRLGameStart;
    if MenuSelection = 2 then LRLEditLevels;
    if MenuSelection = 3 then LRLShowHighScores;
  until MenuSelection = 4;
end;


begin
  LRLInitialize;
  LRLIntro;
  LRLShell;
  LRLDeinitialize;
end.
