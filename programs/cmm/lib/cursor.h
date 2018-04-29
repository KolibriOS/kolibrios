// cursor file should be 32x32 in default MS Windows .cur format
#ifndef INCLUDE_CURSOR_H
#define INCLUDE_CURSOR_H
#print "[include <cursor.h>]\n"

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

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
  if (!CursorPointer) return;
  EAX = 37;
  EBX = 5;
  ECX = 0;
  $int 0x40
  CursorPointer = 0;
}

void CustomCursor::Delete()
{
    EAX = 37;
    EBX = 6;
    ECX = CursorPointer;
    $int 0x40
}

#endif