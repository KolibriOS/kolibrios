unit CRT;

interface

uses
  KolibriOS;

type
  PKey = ^TKey;
  TKey = packed record
    CharCode, ScanCode: AnsiChar;
  end;

const
  Black         = 0;
  Blue          = 1;
  Green         = 2;
  Cyan          = 3;
  Red           = 4;
  Magenta       = 5;
  Brown         = 6;
  LightGray     = 7;
  DarkGray      = 8;
  LightBlue     = 9;
  LightGreen    = 10;
  LightCyan     = 11;
  LightRed      = 12;
  LightMagenta  = 13;
  Yellow        = 14;
  White         = 15;

procedure GotoXY(X, Y: Integer);
function WhereX: Integer;
function WhereY: Integer;

function NormVideo: Integer;
function TextBackground(Color: Byte): Integer;
function TextColor(Color: Byte): Integer;

procedure ClrEOL;
procedure ClrScr;

function KeyPressed: Boolean;
function ReadKey: AnsiChar;
function ReadKeyEx: TKey;
function ReadKeyWord: Word;

procedure SetTitle(Title: PAnsiChar);

procedure Delay(Milliseconds: Integer);

procedure CRT_initialization;

implementation

var
  con_cls: procedure stdcall;
  con_get_cursor_pos: procedure(var X, Y: Integer) stdcall;
  con_get_flags: function: Integer stdcall;
  con_getch: function: Integer stdcall;
  con_getch2: function: Word stdcall;
  con_kbhit: function: Boolean stdcall;
  con_set_flags: function(Flags: Integer): Integer stdcall;
  con_set_cursor_pos: procedure(X, Y: Integer) stdcall;
  con_set_title: procedure(Title: PAnsiChar) stdcall;

  ClrEOLWidth: Integer = 80;

procedure SetTitle(Title: PAnsiChar);
begin
  con_set_title(Title);
end;

function NormVideo: Integer;
begin
  Result := con_set_flags(con_get_flags() and $300 or $07);
end;

function TextBackground(Color: Byte): Integer;
begin
  Result := con_set_flags(con_get_flags() and $30F or Color and $0F shl 4);
end;

function TextColor(Color: Byte): Integer;
begin
  Result := con_set_flags(con_get_flags() and $3F0 or Color and $0F);
end;

procedure GotoXY(X, Y: Integer);
begin
  con_set_cursor_pos(X - 1, Y - 1);
end;

function WhereX: Integer;
var
  Y: Integer;
begin
  con_get_cursor_pos(Result, Y);
  Inc(Result);
end;

function WhereY: Integer;
var
  X: Integer;
begin
  con_get_cursor_pos(X, Result);
  Inc(Result);
end;

procedure ClrEOL;
var
  X, Y, Count: Integer;
  Buf: array [0..255] of AnsiChar;
begin
  con_get_cursor_pos(X, Y);
  Count := ClrEOLWidth - X - 1;
  if Count > 0 then
  begin
    FillChar(Buf[0], Count, ' ');
    Buf[Count] := #0;
    Write(Buf);
    con_set_cursor_pos(X, Y);
  end;
end;

procedure ClrScr;
begin
  con_cls();
end;

function KeyPressed: Boolean;
begin
  Result := con_kbhit();
end;

function ReadKey: AnsiChar;
begin
  Result := Chr(con_getch());
end;

function ReadKeyEx: TKey;
begin
  Result := PKey(con_getch2())^;
end;

function ReadKeyWord: Word;
begin
  Result := con_getch2();
end;

procedure Delay(Milliseconds: Integer);
begin
  Sleep((Milliseconds + 10 div 2) div 10);
end;

procedure CRT_initialization;
begin
  con_cls := GetProcAddress(hConsole, 'con_cls');
  con_getch := GetProcAddress(hConsole, 'con_getch');
  con_getch2 := GetProcAddress(hConsole, 'con_getch2');
  con_get_cursor_pos := GetProcAddress(hConsole, 'con_get_cursor_pos');
  con_get_flags := GetProcAddress(hConsole, 'con_get_flags');
  con_kbhit := GetProcAddress(hConsole, 'con_kbhit');
  con_set_cursor_pos := GetProcAddress(hConsole, 'con_set_cursor_pos');
  con_set_flags := GetProcAddress(hConsole, 'con_set_flags');
  con_set_title := GetProcAddress(hConsole, 'con_set_title');
end;

end.