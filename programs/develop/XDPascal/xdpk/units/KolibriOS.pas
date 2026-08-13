unit KolibriOS;

interface

type
  TSize = packed record
    Height: Word;
    Width:  Word;
  end;

  TPoint = packed record
    Y: SmallInt;
    X: SmallInt;
  end;

  TRect = packed record
    Left:   Integer;
    Top:    Integer;
    Right:  Integer;
    Bottom: Integer;
  end;

  TBox = packed record
    Left:   Integer;
    Top:    Integer;
    Width:  Integer;
    Height: Integer;
  end;

  TSystemDate = packed record
    Year:  Byte;
    Month: Byte;
    Day:   Byte;
    Zero:  Byte;
  end;

  TSystemTime = packed record
    Hours:   Byte;
    Minutes: Byte;
    Seconds: Byte;
    Zero:    Byte;
  end;

  TThreadInfo = packed record
    CpuUsage:     Integer;
    WinStackPos:  Word;
    Reserved0:    Word;
    Reserved1:    Word;
    Name:         array[0..11] of AnsiChar;
    MemAddress:   Integer;
    MemUsage:     Integer;
    Identifier:   Integer;
    Window:       TBox;
    ThreadState:  Word;
    Reserved2:    Word;
    Client:       TBox;
    WindowState:  Byte;
    EventMask:    Integer;
    KeyboardMode: Byte;
    Reserved3:    array[0..947] of Byte;
  end;

  TKeyboardInputMode = (kmChar, kmScan);

  TKeyboardInputFlag = (kfCode, kfEmpty, kfHotKey);

  TKeyboardInput = packed record
    Flag: TKeyboardInputFlag;
    Code: AnsiChar;
    case TKeyboardInputMode of
      kmChar:
        (ScanCode: AnsiChar);
      kmScan:
        (case TKeyboardInputFlag of
          kfCode:
            ();
          kfHotKey:
            (Control: Word);
        );
  end;

  TButtonInput = packed record
    MouseButton: Byte;
    ID:          Word;
    HiID:        Byte;
  end;

  TKeyboardLayout = array[0..127] of AnsiChar;

  TStandardColors = packed record
    Frames:         Integer;
    Grab:           Integer;
    Work3DDark:     Integer;
    Work3DLight:    Integer;
    GrabText:       Integer;
    Work:           Integer;
    WorkButton:     Integer;
    WorkButtonText: Integer;
    WorkText:       Integer;
    WorkGraph:      Integer;
  end;

const
  WINDOW_BORDER_SIZE = 5;  
  
  // Window styles 
  WS_SKINNED_FIXED     =  $4000000;
  WS_SKINNED_SIZABLE   =  $3000000;
  WS_FIXED             =  $0000000;
  WS_SIZABLE           =  $2000000;
  WS_NO_DRAW           =  $1000000;  
  WS_TRANSPARENT_FILL  = $40000000;
  WS_GRADIENT_FILL     = $80000000;
  WS_CLIENT_COORDS     = $20000000;
  WS_CAPTION           = $10000000;

  // Caption styles 
  CAPTION_MOVABLE      = $00000000;
  CAPTION_NONMOVABLE   = $01000000;

  // Window Z-ordering modes
  ZORDER_DESKTOP       = -2;
  ZORDER_BOTTOM        = -1;
  ZORDER_NORMAL        = 0; 
  ZORDER_TOP           = 1; 
    
  // Events 
  REDRAW_EVENT         = 1;
  KEY_EVENT            = 2;
  BUTTON_EVENT         = 3;
  BACKGROUND_EVENT     = 5;
  MOUSE_EVENT          = 6;
  IPC_EVENT            = 7;
  NETWORK_EVENT        = 8;
  DEBUG_EVENT          = 9;

  // Event Mask constants 
  EM_REDRAW            = $001;
  EM_KEY               = $002;
  EM_BUTTON            = $004;
  EM_BACKGROUND        = $010;
  EM_MOUSE             = $020;
  EM_IPC               = $040;
  EM_NETWORK           = $080;
  EM_DEBUG             = $100;

  // Size multipliers for DrawText 
  DT_x1                =  $0000000;
  DT_x2                =  $1000000;
  DT_x3                =  $2000000;
  DT_x4                =  $3000000;
  DT_x5                =  $4000000;
  DT_x6                =  $5000000;
  DT_x7                =  $6000000;
  DT_x8                =  $7000000;

  // Charset specifiers for DrawText 
  DT_CP866_6x9         = $00000000;
  DT_CP866_8x16        = $10000000;
  DT_UTF16LE_8x16      = $20000000;
  DT_UTF8_8x16         = $30000000;

  // Fill styles for DrawText 
  DT_TRANSPARENT_FILL  = $00000000;
  DT_FILL_OPAQUE       = $40000000;

  // Draw zero terminated string for DrawText 
  DT_ZSTRING           = $80000000;

  // Button styles 
  BS_TRANSPARENT_FILL  = $40000000;
  BS_NO_FRAME          = $20000000;

  // OpenSharedMemory open\access flags
  SHM_OPEN             = $00;
  SHM_OPEN_ALWAYS      = $04;
  SHM_CREATE           = $08;
  SHM_READ             = $00;
  SHM_WRITE            = $01;

  // KeyboardLayout flags 
  KBL_NORMAL           = 1;
  KBL_SHIFT            = 2;
  KBL_ALT              = 3;

  // SystemShutdown parameters 
  SHUTDOWN_TURNOFF     = 2;
  SHUTDOWN_REBOOT      = 3;
  SHUTDOWN_RESTART     = 4;

  // Blit flags 
  BLIT_CLIENT_RELATIVE = $20000000;

{-1}      procedure ExitThread;
{0}       procedure DrawWindow(Left, Top, Width, Height: Integer; Caption: PAnsiChar; BackColor, Style, CapStyle: Integer);
{2}       function GetKey: TKeyboardInput;
{4}       procedure DrawText(X, Y: Integer; Text: PAnsiChar; ForeColor, BackColor, Flags, Count: Integer);
{5}       procedure Sleep(Time: Integer);
{8}       procedure DrawButton(Left, Top, Width, Height, BackColor, Style, ID: Integer);
{9}       function GetThreadInfo(Slot: Integer; var Buffer: TThreadInfo): Integer;
{10}      function WaitEvent: Integer;
{11}      function CheckEvent: Integer;
{12.1}    procedure BeginDraw;
{12.2}    procedure EndDraw;
{17}      function GetButton: TButtonInput;
{23}      function WaitEventByTime(Time: Integer): Integer;
{37.1}    function GetWindowMousePos: TPoint;
{37.2}    function GetMouseButtons: Integer;
{40}      function SetEventMask(Mask: Integer): Integer;
{61.1}    function GetScreenSize: TSize;
{68.19}   function LoadLibrary(FileName: PAnsiChar): Pointer;
          function GetProcAddress(hLib: Pointer; ProcName: PAnsiChar): Pointer;

implementation

function WaitEvent: Integer;
type
  TWaitEventProc = function: Integer stdcall;
const
  _WaitEvent: array[0..7] of Byte = (
    $B8, $0A, $00, $00, $00, $CD, $40, $C3
  );
var
  WaitEventProc: TWaitEventProc;
begin
  WaitEventProc := TWaitEventProc(@_WaitEvent);
  Result := WaitEventProc();
end;

function CheckEvent: Integer;
type
  TCheckEventProc = function: Integer stdcall;
const
  _CheckEvent: array[0..7] of Byte = (
    $B8, $0B, $00, $00, $00, $CD, $40, $C3
  );
var
  CheckEventProc: TCheckEventProc;
begin
  CheckEventProc := TCheckEventProc(@_CheckEvent);
  Result := CheckEventProc();
end;

function WaitEventByTime(Time: Integer): Integer;
type
  TWaitEventByTimeProc = function(Time: Integer): Integer stdcall;
const
  _WaitEventByTime: array[0..15] of Byte = (
    $53, $B8, $17, $00, $00, $00, $8B, $5C, $24, $08, $CD, $40, $5B, $C2, $04,
    $00
  );
var
  WaitEventByTimeProc: TWaitEventByTimeProc;  
begin
  WaitEventByTimeProc := TWaitEventByTimeProc(@_WaitEventByTime);
  Result := WaitEventByTimeProc(Time);
end;

procedure DrawWindow(Left, Top, Width, Height: Integer; Caption: PAnsiChar; BackColor, Style, CapStyle: Integer);
type
  TDrawWindowProc = procedure(Left, Top, Width, Height: Integer; Caption: PAnsiChar; BackColor, Style, CapStyle: Integer) stdcall;
const
  _DrawWindow: array[0..50] of Byte = (
    $53, $57, $56, $31, $C0, $8B, $5C, $24, $10, $8B, $4C, $24, $14, $C1, $E3,
    $10, $C1, $E1, $10, $0B, $5C, $24, $18, $0B, $4C, $24, $1C, $8B, $54, $24,
    $28, $0B, $54, $24, $24, $8B, $7C, $24, $20, $8B, $74, $24, $2C, $CD, $40,
    $5E, $5F, $5B, $C2, $20, $00
  );
var
  DrawWindowProc: TDrawWindowProc;
begin
  DrawWindowProc := TDrawWindowProc(@_DrawWindow);
  DrawWindowProc(Left, Top, Width, Height, Caption, BackColor, Style, CapStyle);
end;

procedure ExitThread;
type
  TExitThreadProc = procedure stdcall;
const
  _ExitThread: array[0..4] of Byte = (
    $83, $C8, $FF, $CD, $40
  );
var
  ExitThreadProc: TExitThreadProc;
begin
  ExitThreadProc := TExitThreadProc(@_ExitThread);
  ExitThreadProc();
end;

procedure BeginDraw;
type
  TBeginDrawProc = procedure stdcall;
const
  _BeginDraw: array[0..14] of Byte = (
    $53, $B8, $0C, $00, $00, $00, $BB, $01, $00, $00, $00, $CD, $40, $5B, $C3
  );
var
  BeginDrawProc: TBeginDrawProc;
begin
  BeginDrawProc := TBeginDrawProc(@_BeginDraw);
  BeginDrawProc();
end;

procedure EndDraw;
type
  TEndDrawProc = procedure stdcall;
const
  _EndDraw: array[0..14] of Byte = (
    $53, $B8, $0C, $00, $00, $00, $BB, $02, $00, $00, $00, $CD, $40, $5B, $C3
  );
var
  EndDrawProc: TEndDrawProc;
begin
  EndDrawProc := TEndDrawProc(@_EndDraw);
  EndDrawProc();
end;

function GetButton: TButtonInput;
type
  TGetButtonProc = function: TButtonInput stdcall;
const
  _GetButton: array[0..7] of Byte = (
    $B8, $11, $00, $00, $00, $CD, $40, $C3
  );
var
  GetButtonProc: TGetButtonProc;
begin
  GetButtonProc := TGetButtonProc(@_GetButton);
  Result := GetButtonProc();
end;

function LoadLibrary(FileName: PAnsiChar): Pointer;
type
  TLoadLibraryProc = function(FileName: PAnsiChar): Pointer stdcall;
const
  _LoadLibrary: array[0..20] of Byte = (
    $53, $B8, $44, $00, $00, $00, $BB, $13, $00, $00, $00, $8B, $4C, $24, $08,
    $CD, $40, $5B, $C2, $04, $00
  );
var
  LoadLibraryProc: TLoadLibraryProc;
begin
  LoadLibraryProc := TLoadLibraryProc(@_LoadLibrary);
  Result := LoadLibraryProc(FileName);
end;

function GetProcAddress(hLib: Pointer; ProcName: PAnsiChar): Pointer;
type
  TGetProcAddressProc = function(hLib: Pointer; ProcName: PAnsiChar): Pointer stdcall;
const
  _GetProcAddress: array[0..55] of Byte = (
  $56, $57, $53, $8B, $54, $24, $10, $31, $C0, $85, $D2, $74, $25, $8B, $7C,
  $24, $14, $B9, $FF, $FF, $FF, $FF, $F2, $AE, $89, $CB, $F7, $D3, $8B, $32,
  $85, $F6, $74, $10, $89, $D9, $8B, $7C, $24, $14, $83, $C2, $08, $F3, $A6,
  $75, $ED, $8B, $42, $FC, $5B, $5F, $5E, $C2, $08, $00
);
var
  GetProcAddressProc: TGetProcAddressProc;
begin
  GetProcAddressProc := TGetProcAddressProc(@_GetProcAddress);
  Result := GetProcAddressProc(hLib, ProcName);
end;

function GetKey: TKeyboardInput;
type
  TGetKeyProc = function: TKeyboardInput stdcall;
const
  _GetKey: array[0..7] of Byte = (
    $B8, $02, $00, $00, $00, $CD, $40, $C3
  );
var
  GetKeyProc: TGetKeyProc;
begin
  GetKeyProc := TGetKeyProc(@_GetKey);
  Result := GetKeyProc();
end;

function SetEventMask(Mask: Integer): Integer;
type
  TSetEventMaskProc = function(Mask: Integer): Integer stdcall;
const
  _SetEventMask: array[0..15] of Byte = (
    $53, $B8, $28, $00, $00, $00, $8B, $5C, $24, $08, $CD, $40, $5B, $C2, $04,
    $00
  );
var
  SetEventMaskProc: TSetEventMaskProc;  
begin
  SetEventMaskProc := TSetEventMaskProc(@_SetEventMask);
  Result := SetEventMaskProc(Mask);
end;

function GetScreenSize: TSize;
type
  TGetScreenSizeProc = function: TSize stdcall;
const
  _GetScreenSize: array[0..16] of Byte = (
    $53, $B8, $3D, $00, $00, $00, $BB, $01, $00, $00, $00, $CD, $40, $5B, $C2,
    $00, $00
  );
var
  GetScreenSizeProc: TGetScreenSizeProc;
begin
  GetScreenSizeProc := TGetScreenSizeProc(@_GetScreenSize);
  Result := GetScreenSizeProc();
end;

function GetThreadInfo(Slot: Integer; var Buffer: TThreadInfo): Integer;
type
  TGetThreadInfoProc = function(Slot: Integer; var Buffer: TThreadInfo): Integer stdcall;
const
  _GetThreadInfo: array[0..19] of Byte = (
    $53, $B8, $09, $00, $00, $00, $8B, $5C, $24, $0C, $8B, $4C, $24, $08, $CD,
    $40, $5B, $C2, $08, $00
  );
var
  GetThreadInfoProc: TGetThreadInfoProc;
begin
  GetThreadInfoProc := TGetThreadInfoProc(@_GetThreadInfo);
  Result := GetThreadInfoProc(Slot, Buffer);
end;

procedure Sleep(Time: Integer);
type
  TSleepProc = procedure(Time: Integer) stdcall;
const
  _Sleep: array[0..15] of Byte = (
  $53, $B8, $05, $00, $00, $00, $8B, $5C, $24, $08, $CD, $40, $5B, $C2, $04,
  $00
  );
var
  SleepProc: TSleepProc;  
begin
  SleepProc := TSleepProc(@_Sleep);
  SleepProc(Time);
end;

function GetMouseButtons: Integer;
type
  TGetMouseButtonsProc = function: Integer stdcall;
const
  _GetMouseButtons: array[0..14] of Byte = (
    $53, $B8, $25, $00, $00, $00, $BB, $02, $00, $00, $00, $CD, $40, $5B, $C3
  );
var
  GetMouseButtonsProc: TGetMouseButtonsProc;
begin
  GetMouseButtonsProc := TGetMouseButtonsProc(@_GetMouseButtons);
  Result := GetMouseButtonsProc();
end;

function GetWindowMousePos: TPoint;
type
  TGetWindowMousePosProc = function: TPoint stdcall;
const
  _GetWindowMousePos: array[0..14] of Byte = (
    $53, $B8, $25, $00, $00, $00, $BB, $01, $00, $00, $00, $CD, $40, $5B, $C3
  );
var
  GetWindowMousePosProc: TGetWindowMousePosProc;
begin
  GetWindowMousePosProc := TGetWindowMousePosProc(@_GetWindowMousePos);
  Result := GetWindowMousePosProc();
end;

procedure DrawButton(Left, Top, Width, Height, BackColor, Style, ID: Integer);
type
  TDrawButtonProc = procedure(Left, Top, Width, Height, BackColor, Style, ID: Integer) stdcall;
const
  _DrawButton: array[0..47] of Byte = (
    $53, $56, $B8, $08, $00, $00, $00, $8B, $5C, $24, $0C, $8B, $4C, $24, $10,
    $C1, $E3, $10, $C1, $E1, $10, $0B, $5C, $24, $14, $0B, $4C, $24, $18, $8B,
    $54, $24, $24, $0B, $54, $24, $20, $8B, $74, $24, $1C, $CD, $40, $5E, $5B,
    $C2, $1C, $00
  );
var
  DrawButtonProc: TDrawButtonProc;
begin
  DrawButtonProc := TDrawButtonProc(@_DrawButton);
  DrawButtonProc(Left, Top, Width, Height, BackColor, Style, ID);
end;

procedure DrawText(X, Y: Integer; Text: PAnsiChar; ForeColor, BackColor, Flags, Count: Integer);
type
  TDrawTextProc = procedure(X, Y: Integer; Text: PAnsiChar; ForeColor, BackColor, Flags, Count: Integer) stdcall;
const
  _DrawText: array[0..46] of Byte = (
    $53, $57, $56, $B8, $04, $00, $00, $00, $8B, $5C, $24, $10, $C1, $E3, $10,
    $0B, $5C, $24, $14, $8B, $4C, $24, $24, $0B, $4C, $24, $1C, $8B, $54, $24,
    $18, $8B, $7C, $24, $20, $8B, $74, $24, $28, $CD, $40, $5E, $5F, $5B, $C2,
    $1C, $00
  );
var
  DrawTextProc: TDrawTextProc;
begin
  DrawTextProc := TDrawTextProc(@_DrawText);
  DrawTextProc(X, Y, Text, ForeColor, BackColor, Flags, Count);
end;

end.
