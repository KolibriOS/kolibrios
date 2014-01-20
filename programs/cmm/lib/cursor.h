// cursor file should be 32x32 in default MS Windows .cur format

struct CustomCursor
{
    dword CursorPointer;
    dword Load();
    dword Set();
    dword Restore();
    void Delete();
};

dword CustomCursor::Load(dword CursorFilePath)
{
    if (CursorPointer) return;
    EAX = 37;
    EBX = 4;
    ECX = CursorFilePath;
    EDX = 1;
    $int 0x40
    CursorPointer = EAX; // 0 - err, other - handle
}

dword CustomCursor::Set()
{
  EAX = 37;
  EBX = 5;
  ECX = CursorPointer;
  $int 0x40
}

dword CustomCursor::Restore()
{
  EAX = 37;
  EBX = 5;
  ECX = 0;
  $int 0x40
}

void CustomCursor::Delete()
{
    EAX = 37;
    EBX = 6;
    ECX = CursorPointer;
    $int 0x40
}
