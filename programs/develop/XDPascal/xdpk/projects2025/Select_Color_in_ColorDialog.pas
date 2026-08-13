program Select_Color_in_ColorDialog;
 
uses
  KolibriOS, ColorDlg;
 
var
  WndLeft, WndTop, WndWidth, WndHeight: Integer;
  TextColor: Integer = $00707070;
  ColorDialog: TColorDialog;
  
procedure On_Redraw;
begin
  BeginDraw;
  DrawWindow(WndLeft, WndTop, WndWidth, WndHeight, 'Test ColorDialog, press key to select text color', $00FAFBFC,
    WS_SKINNED_SIZABLE + WS_CLIENT_COORDS + WS_CAPTION, CAPTION_MOVABLE);
  DrawText(2, 75, 'press key to select text color', TextColor, 0,
    DT_CP866_8X16 + DT_TRANSPARENT_FILL + DT_ZSTRING + DT_X2, 0);
  EndDraw;
end;
  
begin
  ColorDlg_initialization;

  with ColorDialog do
  begin
    Mode       :=  CDM_PALETTE_TONE;
    DrawWindow :=  @On_Redraw;
    XSize      :=  510;
    YSize      :=  310;
    ColorType  :=  CDCT_RGB;
  end;
  
  with GetScreenSize do
  begin
    WndWidth := 600;
    WndHeight := Height div 3;
    WndLeft := (Width - WndWidth) div 2;
    WndTop := (Height - WndHeight) div 2;
  end;

  ColorDialog.Init();
  
  while True do
    case WaitEvent of
      REDRAW_EVENT:
        On_Redraw;
      KEY_EVENT:
        begin
          GetKey;
          ColorDialog.Start();
          with ColorDialog do
            if Status = CDS_OK then
            begin
              TextColor := Color;
              On_Redraw;
            end;
        end;
      BUTTON_EVENT:
        Exit;
    end;
end.