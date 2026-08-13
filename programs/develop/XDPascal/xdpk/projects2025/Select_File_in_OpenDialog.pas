program Select_File_in_OpenDialog;
 
uses
  KolibriOS, OpenDlg;
 
var
  WndLeft, WndTop, WndWidth, WndHeight: Integer;
  OpenDialog: TOpenDialog;  
  TxtSelectResult: string = 'Yet not selected.';

procedure On_Redraw;  
begin
  BeginDraw;
  DrawWindow(WndLeft, WndTop, WndWidth, WndHeight, 'Select File in OpenDialog', $00FFFFFF,
    WS_SKINNED_FIXED + WS_CLIENT_COORDS + WS_CAPTION, CAPTION_MOVABLE);
  DrawText(2, 75, 'press key ENTER to select file', $00AF4F3F, 0,
      DT_CP866_8X16 + DT_TRANSPARENT_FILL + DT_ZSTRING, 0);
  DrawText(2, 90, 'press key SPACE to select dir', $003F4FAF, 0,
      DT_CP866_8X16 + DT_TRANSPARENT_FILL + DT_ZSTRING, 0);    
  DrawText(15, 110, TxtSelectResult, $001F7F1F, 0,
      DT_CP866_8X16 + DT_TRANSPARENT_FILL, Length(TxtSelectResult));       
  EndDraw;
end;

procedure BrowseFile;
begin
  with OpenDialog do
  begin
    Mode := ODM_OPEN;
    Start();
    if Status = ODS_OK then
    begin
      TxtSelectResult := 'File = "' + PString(OpenFilePath)^ + '"';
      On_Redraw;
    end;
  end;
end;

procedure BrowseDir;
begin
  with OpenDialog do
  begin
    Mode := ODM_DIR;
    Start();
    if Status = ODS_OK then
    begin
      TxtSelectResult := 'Dir = "' + PString(OpenFilePath)^ + '"';
      On_Redraw;
    end;
  end;
end;

begin
  OpenDlg_initialization;
  
  with OpenDialog do
  begin
    DirDefaultPath := '/sys';
    DrawWindow := @On_Redraw;
    XSize := 400;
    YSize := 480;
  end;

  with GetScreenSize do
  begin
    WndWidth := Width div 4;
    WndHeight := Height div 4;
    WndLeft := (Width - WndWidth) div 2;
    WndTop := (Height - WndHeight) div 2;
  end;
 
  OpenDialog.Init();
  OpenDialog.SetFilter('txt,png,dat,ini,inf,sys,asm,htm');
 
  while True do
    case WaitEvent of
      REDRAW_EVENT:
        On_Redraw;
      KEY_EVENT:
        case GetKey.Code of
          #13{ENTER}: BrowseFile;
          #32{SPACE}: BrowseDir;
        else 
          // nothing to do        
        end;
      BUTTON_EVENT:
        if GetButton.ID = 1 then
          Exit;
    end;
end.
