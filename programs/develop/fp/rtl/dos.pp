unit Dos;


interface


type
  SearchRec = record
    {FindHandle  : THandle;
    WinFindData : TWinFindData;
    ExcludeAttr : longint;}
    Time : longint;
    Size : longint;
    Attr : longint;
    Name : string;
  end;


{$i dosh.inc}


implementation


procedure Intr(intno: byte; var regs: registers); begin end;
procedure MSDos(var regs: registers); begin end;


function  DosVersion: Word; begin end;
procedure GetDate(var year, month, mday, wday: word); begin end;
procedure GetTime(var hour, minute, second, sec100: word); begin end;
procedure SetDate(year,month,day: word); begin end;
procedure SetTime(hour,minute,second,sec100: word); begin end;
procedure UnpackTime(p: longint; var t: datetime); begin end;
procedure PackTime(var t: datetime; var p: longint); begin end;


procedure Exec(const path: pathstr; const comline: comstr); begin end;
function  DosExitCode: word; begin end;


function  DiskFree(drive: byte) : int64; begin end;
function  DiskSize(drive: byte) : int64; begin end;
procedure FindFirst(const path: pathstr; attr: word; var f: searchRec); begin end;
procedure FindNext(var f: searchRec); begin end;
procedure FindClose(Var f: SearchRec); begin end;


procedure GetFAttr(var f; var attr: word); begin end;
procedure GetFTime(var f; var time: longint); begin end;
function  FSearch(path: pathstr; dirlist: string): pathstr; begin end;
function  FExpand(const path: pathstr): pathstr; begin end;
procedure FSplit(path: pathstr; var dir: dirstr; var name: namestr; var ext: extstr); begin end;
function  GetShortName(var p : String) : boolean; begin end;
function  GetLongName(var p : String) : boolean; begin end;


function  EnvCount: longint; begin end;
function  EnvStr (Index: longint): string; begin end;
function  GetEnv(envvar: string): string; begin end;


procedure SetFAttr(var f; attr: word); begin end;
procedure SetFTime(var f; time: longint); begin end;
procedure GetCBreak(var breakvalue: boolean); begin end;
procedure SetCBreak(breakvalue: boolean); begin end;
procedure GetVerify(var verify: boolean); begin end;
procedure SetVerify(verify: boolean); begin end;


procedure SwapVectors; begin end;
procedure GetIntVec(intno: byte; var vector: pointer); begin end;
procedure SetIntVec(intno: byte; vector: pointer); begin end;
procedure Keep(exitcode: word); begin end;


function  GetMsCount: int64; begin end;


end.
