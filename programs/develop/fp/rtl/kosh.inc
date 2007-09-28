{}

type
  TKosPoint = packed record
    X: Longint;
    Y: Longint;
  end;

  TKosRect = packed record
  case Integer of
    0: (Left, Top, Width, Height: Longint);
    1: (TopLeft, HeightWidth: TKosPoint);
  end;

{ User interface }
procedure kos_definewindow(x, y, w, h: Word; style: DWord = $23FFFFFF; header: DWord = $008899FF; clframe: DWord = $008899FF);
procedure kos_movewindow(x, y, w, h: DWord);
function kos_getkey(): DWord;
function kos_getevent(wait: Boolean = True): DWord;
function kos_waitevent(timeout: DWord): DWord;
function kos_getbutton(): DWord;
function kos_getmousepos(): TKosPoint;
function kos_getmousewinpos(): TKosPoint;
function kos_getmousebuttons(): DWord;
procedure kos_maskevents(mask: DWord);
procedure kos_setcaption(caption: PChar);

{ Graphics }
function kos_screensize(): TKosPoint;
procedure kos_begindraw();
procedure kos_enddraw();
procedure kos_putpixel(x, y: Word; color: DWord = $000000);
procedure kos_drawtext(x, y: Word; text: String; flags: DWord = $000000; bgcolor: DWord = $00FFFFFF);
procedure kos_drawrect(x, y, w, h: Word; color: DWord = $000000);
procedure kos_drawline(x1, y1, x2, y2: Word; color: DWord = $000000);
procedure kos_drawimage(x, y, w, h, depth: DWord; image: Pointer; palette: Pointer = nil; xoffset: DWord = 0);
procedure kos_drawimage24(x, y, w, h: DWord; image: Pointer);

{ Work with system }

{ Work with system - System services }
function kos_killthread(tid: TThreadID): Boolean;
procedure kos_setactivewindow(slot: TThreadSlot);
function kos_getthreadslot(tid: TThreadID): TThreadSlot;

{ Work with system - Set system parameters }
procedure kos_enablepci();

{ Work with system - Internal system services }
procedure kos_switchthread();
function kos_initheap(): DWord;
function kos_alloc(size: DWord): Pointer;
function kos_free(ptr: Pointer): Boolean;
function kos_loaddriver(name: PChar): THandle;

{ Processes and threads }
type
  PKosThreadInfo = ^TKosThreadInfo;
  TKosThreadInfo = packed record
    Speed: DWord;
    WindowID: Word;
    ThreadSlot: Word;
    Reserved1: Word;
    AppName: array[0..10] of Char;
    Reserved2: Byte;
    ProcessBase: Pointer;
    MemoryUsage: DWord;
    ThreadID: TThreadID;
    WindowRect: TKosRect;
    Unknown0: array[1..1066] of Byte;
  end;

  {Буфер IPC}
  PKosIPC = ^TKosIPC;
  TKosIPC = packed record
    Lock: LongBool;
    Size: DWord;
    {сообщение #1...}
    {сообщение #2...}
    {...}
  end;

  {Сообщение IPC}
  PKosMessage = ^TKosMessage;
  TKosMessage = packed record
    SenderID: TThreadID;
    Size: DWord;
    {тело сообщения...}
  end;

function kos_threadinfo(info: PKosThreadInfo; slot: TThreadSlot = -1): DWord;
function kos_newthread(entry, stack: Pointer): TThreadID;
procedure kos_initipc(ipc: PKosIPC; size: DWord);
function kos_sendmsg(tid: TThreadID; msg: Pointer; size: DWord): DWord;
function kos_resizemem(size: DWord): Boolean;

{ File system }
{ File system - Work with the current folder }

procedure kos_setdir(path: PChar);
function kos_getdir(path: PChar; size: DWord): DWord;

{ File system - Work with file system with long names support }

const
  kfReadOnly   = $01;
  kfHidden     = $02;
  kfSystem     = $04;
  kfLabel      = $08;
  kfFolder     = $10;
  kfNotArchive = $20;

type
  PKosFile = ^TKosFile;
  TKosFile = packed record
    SubFunc: DWord;
    Position, PositionReserved: DWord;
    Size: DWord;
    Data: Pointer;
    Name: array[0..0] of Char; {...ASCIIZ}
  end;

  PKosBDFE = ^TKosBDFE;
  TKosBDFE = packed record
    Attributes: DWord;
    NameType: Byte; {bit0 - 0:ascii, 1:unicode}
    Reserved: array[0..2] of Byte;
    CTime: DWord;  {ss,mm,hh,00}
    CDate: DWord;  {dd,mm,yyyy}
    ATime: DWord;
    ADate: DWord;
    MTime: DWord;
    MDate: DWord;
    Size: QWord;
    Name: array[0..519] of Char;
  end;

function kos_readfile(kosfile: PKosFile; var readed: Longint): DWord;
function kos_rewritefile(kosfile: PKosFile; var writed: Longint): DWord;
function kos_writefile(kosfile: PKosFile; var writed: Longint): DWord;
function kos_fileinfo(kosfile: PKosFile): DWord;

{ Sound }

function kos_speaker(notes: Pointer): Boolean;

{ Work with hardware }
function kos_readport(index: DWord): DWord;
procedure kos_writeport(index, value: DWord);
function kos_reserveport(port: DWord): Boolean;

{ Work with hardware - Low-level access to PCI}
function kos_lastpcibus(): Byte;
function kos_readpcib(bus, dev, func, reg: Byte): Byte;
function kos_readpciw(bus, dev, func, reg: Byte): Word;
function kos_readpcid(bus, dev, func, reg: Byte): DWord;

{ Other }
procedure kos_delay(ms: DWord); {1/100 s}

{ my }
type
  TKosSign = array[0..7] of Byte;
  PKosHeader = ^TKosHeader;
  TKosHeader = packed record
    sign   : TKOSSign;
    version: DWord;
    start  : DWord;
    size   : DWord;
    memory : DWord;
    stack  : DWord;
    args   : PChar;
    path   : PChar;
  end;


type
  PKonsole = ^TKonsole;
  TKonsole = object
  private
    FCaption: String;
    FLines: array of String;
    FCursor: TKosPoint;
    FMaxLines: Word;
    FThreadID: TThreadID;
    FThreadSlot: TThreadSlot;
    FIPCBuffer: PKosIPC;
    FIPCBufferSize: DWord;
    FTerminate: Boolean;
    FOpened: Boolean;
    FKeyPressed: Word;
    function ReceiveMessage(var Message: ShortString): Boolean;
    procedure ProcessMessage(Message: ShortString);
    procedure ProcessKeyboard(Key: Word);
    function GetRect(): TKosRect;
    function GetKeyPressed(): Word;
    procedure Paint(BottomRow: Boolean = False);
  public
    constructor Init(ACaption: String = '');
    destructor Done();
    procedure Write(Message: ShortString);
    property KeyPressed: Word read GetKeyPressed;
    property Opened: Boolean read FOpened;
    property ThreadID: TThreadID read FThreadID;       {JustForFun, must be hidden, do not use}
    property ThreadSlot: TThreadSlot read FThreadSlot; {JustForFun, must be hidden, do not use}
  end;

  IStreamIO = interface
    function Read(Size: DWord = 0): AnsiString;
    procedure Write(Str: AnsiString; Error: Boolean = False);
  end;
