{utf8}

function DecodeErrNo(ErrNo: DWord): Word;
{0 = успешно
 1 = не определена база и/или раздел жёсткого диска (подфункциями 7, 8 функции 21)
 2 = функция не поддерживается для данной файловой системы
 3 = неизвестная файловая система
 4 = зарезервировано, никогда не возвращается в текущей реализации
 5 = файл не найден
 6 = файл закончился
 7 = указатель вне памяти приложения
 8 = диск заполнен
 9 = таблица FAT разрушена
 10 = доступ запрещён
 11 = ошибка устройства}
begin
  case ErrNo of
    0: Result := 0;
    1: Result := 152;
    2: Result := 153;
    3: Result := 151;
    4: Result := 1;
    5: Result := 2;
    6: Result := 0;
    8: Result := 101;
  else
    Result := 153; { Unknown command (неизвестная команда) }
  end;
end;

function do_isdevice(handle:thandle): Boolean;
begin
  InOutRes := 211;
  Result := False;
end;

procedure do_close(handle: THandle);
begin
  FreeMem(PKosFile(handle));
  InOutRes := 0;
end;

procedure do_erase(p : pchar);
begin
  InOutRes := 211;
end;

procedure do_rename(p1,p2 : pchar);
begin
  InOutRes := 211;
end;

function do_write(handle: THandle; addr: Pointer; len: Longint): Longint;
begin
  PKosFile(handle)^.Size := len;
  PKosFile(handle)^.Data := addr;
  InOutRes := DecodeErrNo(kos_writefile(PKosFile(handle), Result));
  Inc(PKosFile(handle)^.Position, Result);
end;

function do_read(handle: THandle; addr: Pointer; len: Longint): Longint;
begin
  PKosFile(handle)^.Size := len;
  PKosFile(handle)^.Data := addr;
  InOutRes := DecodeErrNo(kos_readfile(PKosFile(handle), Result));
  Inc(PKosFile(handle)^.Position, Result);
end;

function do_filepos(handle: THandle): Int64;
begin
  Result := PKosFile(handle)^.Position;
end;

procedure do_seek(handle: THandle; pos: Int64);
begin
  PKosFile(handle)^.Position := pos;
end;

function do_seekend(handle: THandle): Int64;
begin
  InOutRes := 211;
  Result := 0;
end;

function do_filesize(handle: THandle): Int64;
var
  BDFE: TKosBDFE;
begin
  PKosFile(handle)^.Data := @BDFE;
  InOutRes := DecodeErrNo(kos_fileinfo(PKosFile(handle)));
  Result := BDFE.Size;
end;

procedure do_truncate(handle: THandle; pos: Int64);
begin
  InOutRes := 211;
end;


{ FIXME: Поправить RTL, факт отсутствия файла не фиксируется при его открытии. }
procedure do_open(var f; p: PChar; flags: Longint);
var
  KosFile: PKosFile;
  FilePath: PChar;
  FilePathLen: Longint;
  RecSize: Longint;
  CurrDir: array[0..2048] of Char;
  CurrDirLen: Longint;
  Dummy: Longint;
begin
  case flags and 3 of
    0: FileRec(f).Mode := fmInput;
    1: FileRec(f).Mode := fmOutput;
    2: FileRec(f).Mode := fmInOut;
  end;

  {Формирование имени абсолютного пути}
  FilePathLen := Length(p);
  if p^ <> DirectorySeparator then
  begin
    {XXX: размер буфера CurrDir может оказаться недостаточным}
    CurrDirLen := kos_getdir(@CurrDir, SizeOf(CurrDir) - FilePathLen - 1) - 1;
    FilePath := @CurrDir;

    if FilePath[CurrDirLen - 1] <> DirectorySeparator then
    begin
      FilePath[CurrDirLen] := DirectorySeparator;
      Inc(CurrDirLen);
    end;
    Move(p^, FilePath[CurrDirLen], FilePathLen + 1);
    Inc(FilePathLen, CurrDirLen);
  end else
    FilePath := p;

  {Создание структуры TKosFile}
  RecSize := SizeOf(TKosFile) + FilePathLen;
  KosFile := GetMem(RecSize);
  FillChar(KosFile^, RecSize, 0);
  Move(FilePath^, KosFile^.Name, FilePathLen);
  FileRec(f).Handle := DWord(KosFile);

  if flags and $1000 <> 0 then
  begin
    { создать файл }
    InOutRes := DecodeErrNo(kos_rewritefile(KosFile, RecSize));
  end else
  begin
    { попытаться прочитать файл }
    KosFile^.Size := 1;
    KosFile^.Data := @Dummy;
    InOutRes := DecodeErrNo(kos_readfile(KosFile, Dummy));
  end;
  
  if InOutRes <> 0 then
  begin
    FreeMem(KosFile);
    FileRec(f).Handle := DWord(-1);
  end;
end;
