use32
org 0x0
  db 'MENUET01'
  dd 0x1
  dd MenuetEntryPoint
  dd I_END
  dd U_END+STACKSIZE+HEAPSIZE
  dd U_END+STACKSIZE
  dd 0x0,0x0

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

macro nextdef x,[link]
{
  common
    usedef x
    end if
    if x eq
    else if used x
    x:
    usedef link
}
