use32
org 0
  db 'MENUET01'
  dd 1
  dd @Kolibri@Main$qv
  dd I_END
  dd U_END+STACKSIZE+HEAPSIZE
  dd U_END+STACKSIZE
  dd @Kolibri@CommandLine,0

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

macro usedef [link]
{
  common
    if ~link eq
    virtual at 0
  forward
    dd   link
  common
    end virtual
    end if
}

macro define x,[link]
{
  common
    if x eq
    else if used x
    x:
    usedef link
}

macro enddef [link]
{
  common
    usedef link
    end if
}

macro newdef x,[link]
{
  common
    end if
    if x eq
    else if used x
    x:
    usedef link
}

macro nextdef x
{
x:
}

