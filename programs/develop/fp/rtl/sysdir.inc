{TODO}

procedure mkdir(const s: String); [IOCHECK];
begin
  InOutRes := 211;
end;

procedure rmdir(const s: String); [IOCHECK];
begin
  InOutRes := 211;
end;

procedure chdir(const s: String); [IOCHECK];
var
  Path: array[Byte] of Char;
begin
  Path := s;
  kos_setdir(Path);
  InOutRes := 0;
end;

procedure getdir(DriveNr: Byte; var Dir: ShortString);
{ DriveNr ?? ?ᯮ????????, ?? ?ᥣ?? ?????? ???? ࠢ?? 0 }
var
  Path: array[Byte] of Char;
  Size: Longword;
begin
  if DriveNr <> 0 then
    InOutRes := 15 { Invalid drive number (???ࠢ?????? ????? ???ன?⢠) }  else
  begin
    Size := kos_getdir(@Path, SizeOf(Path));
    Dir := StrPas(Path);
    InOutRes := 0;
  end;
end;
