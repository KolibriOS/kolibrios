macro writestr [arg]
{
  common

  local straddr
  local strend
  pushad
  push straddr
  push strend
  jmp  @Kolibri@DebugPutString$qpxc
  straddr db arg,0
  strend:
  pop  eax
  popad
}

macro writeint arg
{
  push dword arg
  xchg eax,[esp]
  pushad
  push eax
  call @DebugPutNumber$qi
  pop  eax
  popad
  pop  eax
}

macro write [arg]
{
  forward

  if arg eq
  else if arg eq endline
	writestr 10
  else if arg eqtype ''
	writestr arg
  else
    writeint arg
  end if
}

macro writeln [arg]
{
  common

  write arg,endline
}

