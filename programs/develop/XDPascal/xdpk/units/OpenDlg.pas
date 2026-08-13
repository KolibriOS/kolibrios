unit OpenDlg;

interface

uses
  KolibriOS, ProcLib;

const
// OpenDialog.Mode constants
  ODM_OPEN = 0;
  ODM_SAVE = 1;
  ODM_DIR  = 2;

// OpenDialog.Status constants
  ODS_CANCEL = 0;
  ODS_OK     = 1;
  ODS_ALTER  = 2;

type
// actually structure is:
//   first four bytes - size of this structure
//   and next are zero separated string values
  TOpenDialogFilter = packed record
    Size: Integer;
    Text: array [0..255] of AnsiChar;
  end;

  POpenDialog = ^TOpenDialog;
  TOpenDialog = packed record
    Mode:           Integer;
    ProcInfo:       Pointer;
    ComAreaName:    PAnsiChar;
    ComArea:        Pointer;
    OpenDirPath:    PAnsiChar;
    DirDefaultPath: PAnsiChar;
    StartPath:      PAnsiChar;
    DrawWindow:     procedure;
    Status:         Integer;
    OpenFilePath:   PAnsiChar;
    FileNameArea:   PAnsiChar;
    FilterArea:     ^TOpenDialogFilter;
    XSize:          Word; // at least 350
    XStart:         SmallInt;
    YSize:          Word; // at least 250
    YStart:         SmallInt;
  {private}
    ProcInfoBuffer: array [0..Pred(SizeOf(TThreadInfo))] of Byte;
    OpenDialogFilter: TOpenDialogFilter;
    OpenFilePathBuffer: array [0..4095] of AnsiChar;
    OpenDirPathBuffer: array [0..3071] of AnsiChar;
    FileNameAreaBuffer: array [0..1023] of AnsiChar;
  end;

procedure Init for OpenDialog: TOpenDialog;
procedure Start for OpenDialog: TOpenDialog;
procedure SetFilter for OpenDialog: TOpenDialog(Filter: PAnsiChar);

procedure OpenDlg_initialization;

implementation

var
  OpenDialog_init: procedure(var OpenDialog: TOpenDialog) stdcall;
  OpenDialog_start: procedure(var OpenDialog: TOpenDialog) stdcall;

procedure Init for OpenDialog: TOpenDialog;
begin
  with OpenDialog do
  begin
    ProcInfo := @ProcInfoBuffer;
    ComAreaName := 'FFFFFFFF_open_dialog';
    StartPath := '/sys/File managers/opendial';
    OpenDirPath := @OpenDirPathBuffer[0];
    OpenFilePath := @OpenFilePathBuffer[0];
    FileNameArea := @FileNameAreaBuffer[0];
    FilterArea := @OpenDialogFilter;
  end;
  OpenDialog_init(OpenDialog);
end;

procedure Start for OpenDialog: TOpenDialog;
begin
  OpenDialog_start(OpenDialog);
end;

// copy comma separated string values from Filter
// to OpenDialogFilter and replace commas with zeroes
// 'txt,png,bmp' -> 'txt'#0'png'#0'bmp'#0
procedure SetFilter for OpenDialog: TOpenDialog(Filter: PAnsiChar);
var
  i: Integer;
begin
  i := 0;
  with OpenDialog.OpenDialogFilter do
  begin
    repeat
      if Filter[i] = ',' then
        Text[i] := #0
      else
        Text[i] := Filter[i];
      Inc(i);
    until (Filter[Pred(i)] = #0) or (i = Pred(SizeOf(Text)));
    Text[i] := #0;
    Size := i + SizeOf(Size);
  end;
end;

procedure OpenDlg_initialization;
begin
  if hProcLib = nil then ProcLib_initialization;
  OpenDialog_init  := GetProcAddress(hProcLib, 'OpenDialog_init');
  OpenDialog_start := GetProcAddress(hProcLib, 'OpenDialog_start');
end;

end.