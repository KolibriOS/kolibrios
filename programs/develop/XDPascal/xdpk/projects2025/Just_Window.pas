program Just_Window;

uses
  KolibriOS;

begin
  while True do
    case WaitEvent of
      REDRAW_EVENT:
        begin
          BeginDraw;
          DrawWindow(300, 200, 350, 250, 'Hello from XDPascal!', $00D5E3F1,
            WS_SKINNED_SIZABLE + WS_CLIENT_COORDS + WS_CAPTION, CAPTION_MOVABLE);
          EndDraw;
        end;
      KEY_EVENT:
        GetKey;
      BUTTON_EVENT:
        if GetButton.ID = 1 then
          Exit;
    end;
end.