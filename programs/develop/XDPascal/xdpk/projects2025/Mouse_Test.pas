program Mouse_Test;

uses
  KolibriOS;

const
  MY_BUTTON = 1000;  
  
var  
  WndLeft, WndTop, WndWidth, WndHeight: Integer;
  
  TxtClickResult: string = 'Yet not clicked.';

procedure On_Redraw;
begin 
  BeginDraw;
  DrawWindow(300, 200, 350, 250, 'Mouse test.', $00D5E3F1,
    WS_SKINNED_SIZABLE + WS_CLIENT_COORDS + WS_CAPTION, CAPTION_MOVABLE);
  DrawButton(10, 20, 50, 30, $0075D3F1, 0, MY_BUTTON);  
  DrawText(20, 55, 'Click on window', $00AF4F3F, 0,
      DT_CP866_8X16 + DT_TRANSPARENT_FILL + DT_ZSTRING, 0);
  DrawText(20, 70, 'or click on button', $003F4FAF, 0,
      DT_CP866_8X16 + DT_TRANSPARENT_FILL + DT_ZSTRING, 0);
  DrawText(15, 90, TxtClickResult, $001F7F1F, 0,
      DT_CP866_8X16 + DT_TRANSPARENT_FILL, Length(TxtClickResult));              
  EndDraw;        
end;
  
begin
  with GetScreenSize do
  begin
    WndWidth  := Width div 2;
    WndHeight := Height div 2;
    WndLeft   := (Width - WndWidth) div 2;
    WndTop    := (Height - WndHeight) div 2;
  end;

  SetEventMask(EM_REDRAW + EM_BUTTON + EM_KEY + EM_MOUSE);
  
  while True do
    case WaitEvent of
      REDRAW_EVENT:
        On_Redraw;
      KEY_EVENT:
        GetKey;      
      MOUSE_EVENT:
      begin
        case GetMouseButtons of
          1: if GetWindowMousePos.X > 100 then 
               TxtClickResult := 'Left mouse button down(X > 100).'
             else
               TxtClickResult := 'Left mouse button down(X <= 100).';
          2: TxtClickResult := 'Right mouse button down.';
          4: TxtClickResult := 'Middle mouse button down.';
        end;
        On_Redraw;                
      end;        
      BUTTON_EVENT:
        with GetButton do
          case ID of
            1: Exit;          
            MY_BUTTON:
              begin
                case MouseButton of
                  0: TxtClickResult := 'Button clicked by left mouse button.';
                  2: TxtClickResult := 'Button clicked by right mouse button.';
                  4: TxtClickResult := 'Button clicked by middle mouse button.';
                end;
                On_Redraw;
              end;
          end;
        
    end;
end.