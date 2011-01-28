define @Menuet@StrLen$qpxc
  push edi
  cld
  mov  edi,[esp+8]
  mov  ecx,-1
  xor  al,al
  repnz scas byte [edi]
  not  ecx
  lea  eax,[ecx-1]
  pop  edi
  ret
enddef

define @Menuet@StrCopy$qpcpxc
  push esi
  push edi
  cld
  mov  edi,[esp+16]
  mov  ecx,-1
  mov  esi,edi
  xor  al,al
  repnz scas byte [edi]
  not  ecx
  mov  edi,[esp+12]
  mov  edx,ecx
  mov  eax,edi
  shr  ecx,2
  rep movs dword [edi],[esi]
  mov  ecx,edx
  and  ecx,3
  rep movs byte [edi],[esi]
  pop  edi
  pop  esi
  ret
enddef

define @Menuet@MemCopy$qpvpxvui
  push esi
  push edi
  cld
  mov  edi,[esp+12]
  mov  eax,edi
  mov  ecx,[esp+20]
  mov  esi,[esp+16]
  mov  edx,ecx
  shr  ecx,2
  rep movs dword [edi],[esi]
  mov  ecx,edx
  and  ecx,3
  rep movs byte [edi],[esi]
  pop  edi
  pop  esi
  ret
enddef

define @Menuet@MemSet$qpvcui
  push edi
  cld
  mov  edi,[esp+8]
  mov  al,[esp+12]
  mov  ah,al
  mov  dx,ax
  shl  eax,16
  mov  ax,dx
  mov  ecx,[esp+16]
  mov  edx,ecx
  shr  ecx,2
  rep stos dword [edi]
  mov  ecx,edx
  and  ecx,3
  rep stos byte [edi]
  pop  edi
  mov  eax,[esp+4]
  ret
enddef

define __ftol
  sub  esp,12
  wait
  fstcw word [esp+8]
  wait
  mov  al,[esp+9]
  or   byte [esp+9],0x0c
  fldcw word [esp+8]
  fistp qword [esp]
  mov  [esp+9],al
  fldcw word [esp+8]
  mov  eax,[esp]
  mov  edx,[esp+4]
  add  esp,12
  ret
enddef

define @Menuet@Floor$qd
  fld  qword [esp+4]
  mov  ax,[esp+10]
  shl  ax,1
  cmp  ax,0x8680
  ja   Menuet_floor_end
  mov  ch,4
  sub  esp,2
  wait
  fstcw word [esp]
  mov  ax,0xf3ff
  wait
  mov  dx,[esp]
  and  ax,dx
  or   ah,ch
  mov  [esp],ax
  fldcw word [esp]
  frndint
  mov  [esp],dx
  fldcw word [esp]
  add  esp,2
Menuet_floor_end:
  ret
enddef

