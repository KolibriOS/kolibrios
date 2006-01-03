oX = 6
oY = 22+3
tW = 6
tH = 9

VK_TAB    =   9
VK_RETURN =  13
VK_ESCAPE =  27
VK_F10    =  49
VK_F5     =  54
VK_LEFT   = 176
VK_DOWN   = 177
VK_UP     = 178
VK_RIGHT  = 179
VK_HOME   = 180
VK_END    = 181
VK_PGDOWN = 183
VK_PGUP   = 184

VK_CTRLF3 = 212
VK_CTRLF4 = 213

FA_READONLY = 00000001b
FA_HIDDEN   = 00000010b
FA_SYSTEM   = 00000100b
FA_LABEL    = 00001000b
FA_FOLDER   = 00010000b
FA_ARCHIVED = 00100000b

struc mfar_colors {
 .background dd $00000080
 .default    dd $0000FFFF
 .system     dd $00008080
 .selection  dd $00009090
 .folder     dd $00FFFFFF
 .executable dd $0000FF00
 .bitmap     dd $00FF00FF
 .source     dd $00FF0000
 .path       dd $00C0C0C0
 .pathbg     dd $00000000
 .keys       dd $00000000
 .keysbg     dd $00008080
}
struct mfar_colors

macro __mov reg,a {
 if ~a eq
  mov reg,a
 end if
}

macro mcall a,b,c,d,e,f {
 __mov eax,a
 __mov ebx,b
 __mov ecx,c
 __mov edx,d
 __mov esi,e
 __mov edi,f
 int   $40
}

macro mcall1 a,b1,b2,c,d,e,f {
 __mov eax,a
 if (~b1 eq)|(~b2 eq)
  mov ebx,(b1)*65536+(b2)
 end if
 __mov ecx,c
 __mov edx,d
 __mov esi,e
 __mov edi,f
 int   $40
}

macro mcall2 a,b1,b2,c1,c2,d,e,f {
 __mov eax,a
 if (~b1 eq)|(~b2 eq)
  mov ebx,(b1)*65536+(b2)
 end if
 if (~c1 eq)|(~c2 eq)
  mov ecx,(c1)*65536+(c2)
 end if
 __mov edx,d
 __mov esi,e
 __mov edi,f
 int   $40
}

macro sz name,[data] {
  common
   if used name
    label name
   end if
  forward
   if used name
    db data
   end if
  common
   if used name
    .size = $-name
   end if
}

macro lsz name,[lng,data] {
  common
   if used name
    label name
   end if
  forward
   if (used name)&(lang eq lng)
    db data
   end if
  common
   if used name
    .size = $-name
   end if
}

macro mmov reg,a1,a2 {
 mov reg,(a1)*65536+a2
}

macro madd reg,a1,a2 {
 add reg,(a1)*65536+a2
}

macro msub reg,a1,a2 {
 sub reg,(a1)*65536+a2
}

macro jmpe reg,def,[val,lab] {
 forward
  cmp reg,val
  je  lab
 common
  if ~def eq
   jmp def
  end if
}

macro func name {
 if used name
  label name
}

macro endf {
 end if
}