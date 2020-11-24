use32
org 0
  db 'MENUET01'
  dd 1
  dd @Kolibri@Main$qv
  dd I_END
  dd U_END+STACKSIZE ;+HEAPSIZE
  dd U_END+STACKSIZE
if defined @Kolibri@CommandLine
  dd @Kolibri@CommandLine
else
  dd 0
end if
if defined @Kolibri@CurrentDirectoryPath
  dd @Kolibri@CurrentDirectoryPath
else
  dd 0
end if

include "..\..\KOSfuncs.inc"
include "..\..\proc32.inc"
include "..\..\macros.inc"
include "..\..\dll.inc"
include "..\..\load_lib.mac"

ptr equ
offset equ
short equ
tbyte equ tword

PTR equ
OFFSET equ
SHORT equ
TBYTE equ TWORD

macro movsb a,b
{
  if a eq & b eq
    movsb
  else
    movsx a,b
  end if
}
  
macro movsw a,b
{
  if a eq & b eq
    movsw
  else
    movsx a,b
  end if
}

macro segment name {}

macro endseg  name {}
