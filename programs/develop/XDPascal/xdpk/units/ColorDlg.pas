unit ColorDlg;

interface

uses
  KolibriOS, ProcLib;

const
// ColorDialog.Mode constants
  CDM_PALETTE_TONE = 0;

// ColorDialog.Status constants
  CDS_CANCEL = 0;
  CDS_OK     = 1;
  CDS_ALTER  = 2;

// ColorDialog.ColorType constants
  CDCT_RGB = 0;

type
  PColorDialog = ^TColorDialog;
  TColorDialog = packed record
    Mode:        Integer;
    ProcInfo:    Pointer;
    ComAreaName: PAnsiChar;
    ComArea:     Pointer;
    StartPath:   PAnsiChar;
    DrawWindow:  procedure;
    Status:      Integer;
    XSize:       Word;
    XStart:      SmallInt;
    YSize:       Word;
    YStart:      SmallInt;
    ColorType:   Integer;
    Color:       Integer;
  {private}
    ProcInfoBuffer: array [0..Pred(SizeOf(TThreadInfo))] of Byte;
  end;

procedure Init for ColorDialog: TColorDialog;
procedure Start for ColorDialog: TColorDialog;

procedure ColorDlg_initialization;

implementation

var
  ColorDialog_init: procedure(var ColorDialog: TColorDialog) stdcall;
  ColorDialog_start: procedure(var ColorDialog: TColorDialog) stdcall;

procedure Init for ColorDialog: TColorDialog;
begin
  with ColorDialog do
  begin
    ProcInfo := @ProcInfoBuffer;
    ComAreaName := 'FFFFFFFF_color_dialog';
    StartPath := '/sys/colrdial';
  end;
  ColorDialog_init(ColorDialog);
end;

procedure Start for ColorDialog: TColorDialog;
begin
  ColorDialog_start(ColorDialog);
end;

procedure ColorDlg_initialization;
begin
  if hProcLib = nil then ProcLib_initialization;
  ColorDialog_init  := GetProcAddress(hProcLib, 'ColorDialog_init');
  ColorDialog_start := GetProcAddress(hProcLib, 'ColorDialog_start');
end;

end.