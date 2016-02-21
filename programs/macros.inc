@^ fix macro comment {
^@ fix }

; -------------------------
macro library [lname,fname]
{
  forward
    dd __#lname#_library_table__,__#lname#_library_name__
  common
    dd 0
  forward
    align 4
    __#lname#_library_name__ db fname,0
}

macro import lname,[name,sname]
{
  common
    align 4
    __#lname#_library_table__:
  forward
    if used name
      name dd __#name#_import_name__
    end if
  common
    dd 0
  forward
    if used name
      align 4
      __#name#_import_name__ db sname,0
    end if
}

macro export [name,sname]
{
  forward
    dd __#name#_export_name__,name
  common
    dd 0
  forward
    align 4
    __#name#_export_name__ db sname,0
}
; -------------------------

macro m2m dest,src {
 push src
 pop  dest
}


macro iglobal {
  IGlobals equ IGlobals,
  macro __IGlobalBlock { }
  
macro uglobal {
  UGlobals equ UGlobals,
  macro __UGlobalBlock { }

endg fix }      ; Use endg for ending iglobal and uglobal blocks.


macro IncludeIGlobals{
  macro IGlobals dummy,[n] \{ __IGlobalBlock
     purge __IGlobalBlock  \}
  match I, IGlobals \{ I \} }

macro IncludeUGlobals{
  macro UGlobals dummy,[n] \{
    \common
      \local begin, size
      begin = $
      virtual at $
    \forward
      __UGlobalBlock
      purge __UGlobalBlock
    \common
      size = $ - begin
    end virtual
    rb size
  \}
  match U, UGlobals \{ U \} }

uglobal
endg
  
iglobal
endg


; new application structure
macro meos_app_start
 {
  use32
  org 0x0

  db 'MENUET01'
  dd 0x01
  dd __start
  dd __end
  dd __memory
  dd __stack

  if used __params & ~defined __params
    dd __params
  else
    dd 0x0
  end if

  dd 0x0
 }
MEOS_APP_START fix meos_app_start

macro code
 {
  __start:
 }
CODE fix code

macro data
 {
  __data:
  IncludeIGlobals
 }
DATA fix data

macro udata
 {
  if used __params & ~defined __params
    __params:
      db 0
    __end:
      rb 255
  else
    __end:
  end if
  __udata:
  IncludeUGlobals
 }
UDATA fix udata

macro meos_app_end
 {
  align 32
  rb 2048
  __stack:
  __memory:
 }
MEOS_APP_END fix meos_app_end


; macro for defining multiline text data
struc mstr [sstring]
 {
  forward
    local ssize
    virtual at 0
      db sstring
      ssize = $
    end virtual
    dd ssize
    db sstring
  common
    dd -1
 }

; macro for defining multiline text data
struc mls [sstring]
 {
  forward
    local ssize
    virtual at 0
      db sstring  ; mod
      ssize = $
    end virtual
    db ssize
    db sstring
  common
    db -1         ; mod
 }



; strings
macro sz name,[data] {       ; from MFAR [mike.dld]
 common
  if used name
   name db data
   .size = $-name
  end if
}

macro szZ name,[data] {      ; same as sz, but for zero terminated string [dunkaist]
 common
  if used name
   name db data,0
   .size = $-name-1
  end if
}
sz0 fix szZ

macro lsz name,[lng,data] {  ; from MFAR [mike.dld]
 common
  if used name
   label name
 forward
  if lang eq lng
   db data
  end if
 common
   .size = $-name
  end if
}

macro szc name,elsz,[data] {         ; from MFAR [mike.dld]
 common
  local s,m
  m = 0
  if used name
   label name
 forward
   virtual at 0
    db data
    s = $
   end virtual
   d#elsz s
   if m < s
    m = s
   end if
   db data
 common
   .size = $-name
   .maxl = m
  end if
}

macro lszc name,elsz,[lng,data] {  ; from MFAR [mike.dld]
 common
  local s,m,c
  m = 0
  c = 0
  if used name
   label name
 forward
  if lang eq lng
   virtual at 0
    db data
    s = $
   end virtual
   d#elsz s
   if m < s
    m = s
   end if
   db data
   c = c+1
  end if
 common
   .size  = $-name
   .maxl  = m
   .count = c
  end if
}


; easy system call macro
macro mpack dest, hsrc, lsrc
{
  if (hsrc eqtype 0) & (lsrc eqtype 0)
    mov dest, (hsrc) shl 16 + lsrc
  else
    if (hsrc eqtype 0) & (~lsrc eqtype 0)
      mov dest, (hsrc) shl 16
      add dest, lsrc
    else
      mov dest, hsrc
      shl dest, 16
      add dest, lsrc
    end if
  end if
}

macro __mov reg,a,b {       ; mike.dld
 if (~a eq)&(~b eq)
   mpack reg,a,b
 else if (~a eq)&(b eq)
   mov reg,a
 end if
}


include 'config.inc'
;__CPU_type equ p5
SYSENTER_VAR    equ 0

macro mcall a,b,c,d,e,f {   ; mike.dld, updated by Ghost for Fast System Calls
 local  ..ret_point
 __mov eax,a
 __mov ebx,b
 __mov ecx,c
 __mov edx,d
 __mov esi,e
 __mov edi,f

 if __CPU_type eq p5
        int     0x40
 else
  if __CPU_type eq p6
        push    ebp
        mov     ebp, esp
        push    ..ret_point     ; it may be 2 or 5 byte
        sysenter
 ..ret_point:
        pop     edx
        pop     ecx

  else
   if __CPU_type eq k6
        push    ecx
        syscall
        pop     ecx
   else
        display 'ERROR : unknown CPU type (set to p5)', 10, 13
        __CPU_type equ p5
        int     0x40
   end if
  end if
 end if
}


; -------------------------
macro header a,[b] {
 common
  use32
  org 0
  db 'MENUET',a
 forward
  if b eq
   dd 0
  else
   dd b
  end if }
macro section name { align 16
 label name }
macro func name {
 if ~used name
  display 'FUNC NOT USED: ',`name,13,10
 else
  align 4
  name:
  ;diff16 `name,0,name
;pushad
;pushfd
;dps `name
;newline
;mcall 5,1
;popfd
;popad
}
macro endf { end if }

macro diff16 title,l1,l2
 {
  local s,d
  s = l2-l1
  display title,': 0x'
  repeat 8
   d = '0' + s shr ((8-%) shl 2) and $0F
   if d > '9'
    d = d + 'A'-'9'-1
   end if
   display d
  end repeat
  display 13,10
 }

macro diff10 title,l1,l2
 {
  local s,d,z,m
  s = l2-l1
  z = 0
  m = 1000000000
  display title,': '
  repeat 10
   d = '0' + s / m
   s = s - (s/m)*m
   m = m / 10
   if d <> '0'
    z = 1
   end if
   if z <> 0
    display d
   end if
  end repeat
  display 13,10
 }


macro movi arg1,arg2
{
if (arg1 in <eax,ebx,ecx,edx,esi,edi,ebp,esp>) & ((arg2 eqtype 0) | (arg2 eqtype '0'))
  if (arg2) = 0
    xor arg1,arg1
  else if (arg2) = 1
    xor arg1,arg1
    inc arg1
  else if (arg2) = -1
    or arg1,-1
  else if (arg2) >= -128 & (arg2) <= 127
    push arg2
    pop arg1
  else
    mov arg1,arg2
  end if
else
  mov arg1,arg2
end if
}


macro RGB [a] {
 common
  match (r=,g=,b),a \{
   \dd ((r) shl 16) or ((g) shl 8) or (b)
  \}
}


struc POINT _t,_dx,_dy {
 .x _t _dx
 .y _t _dy
}

; structure definition helper
include 'struct.inc'

struct RECT
  left   dd ?
  top    dd ?
  right  dd ?
  bottom dd ?
ends

struct BOX
  left   dd ?
  top    dd ?
  width  dd ?
  height dd ?
ends

; structures used in KolibriOS
struct process_information
  cpu_usage               dd ?  ; +0
  window_stack_position   dw ?  ; +4
  window_stack_value      dw ?  ; +6
                          dw ?  ; +8
  process_name            rb 12 ; +10
  memory_start            dd ?  ; +22
  used_memory             dd ?  ; +26
  PID                     dd ?  ; +30
  box                     BOX   ; +34
  slot_state              dw ?  ; +50
                          dw ?  ; +52
  client_box              BOX   ; +54
  wnd_state               db ?  ; +70
  rb (1024-71)
ends

struct system_colors
  frame            dd ? ;nonset1
  grab             dd ? ;nonset2
  work_3d_dark     dd ?
  work_3d_light    dd ?
  grab_text        dd ? ;window_title
  work             dd ?
  work_button      dd ?
  work_button_text dd ?
  work_text        dd ?
  work_graph       dd ?
ends

struct FILEDATE
  Second db ?
  Minute db ?
  Hour   db ?
         db ?
  Day    db ?
  Month  db ?
  Year   dw ?
ends

struct FILEINFO
  Attributes dd ?
  IsUnicode  db ?
             db 3 dup(?)
  DateCreate FILEDATE
  DateAccess FILEDATE
  DateModify FILEDATE
  Size       dq ?
ends

cmove   fix     cmovz

macro cmovz reg1, reg2 {

        local   ..jumpaddr

if __CPU_type eq p5             ; CMOVcc isnt supported on the P5
        jnz     ..jumpaddr
        mov     reg1, reg2
  ..jumpaddr:
else
        cmovz   reg1, reg2
end if

}

cmovne  fix     cmovnz

macro cmovnz reg1, reg2 {

        local   ..jumpaddr

if __CPU_type eq p5             ; CMOVcc isnt supported on the P5
        jz      ..jumpaddr
        mov     reg1, reg2
  ..jumpaddr:
else
        cmovnz  reg1, reg2
end if

}

macro cmovg reg1, reg2 {

        local   ..jumpaddr

if __CPU_type eq p5             ; CMOVcc isnt supported on the P5
        jle     ..jumpaddr
        mov     reg1, reg2
  ..jumpaddr:
else
        cmovg   reg1, reg2
end if
}

macro cmovl reg1, reg2 {

        local   ..jumpaddr

if __CPU_type eq p5             ; CMOVcc isnt supported on the P5
        jge     ..jumpaddr
        mov     reg1, reg2
  ..jumpaddr:
else
        cmovl   reg1, reg2
end if
}

; constants

; events
EV_IDLE        = 0
EV_TIMER       = 0
EV_REDRAW      = 1
EV_KEY         = 2
EV_BUTTON      = 3
EV_EXIT        = 4
EV_BACKGROUND  = 5
EV_MOUSE       = 6
EV_IPC         = 7
EV_STACK       = 8

; event mask bits for function 40
EVM_REDRAW     =          1b
EVM_KEY        =         10b
EVM_BUTTON     =        100b
EVM_EXIT       =       1000b
EVM_BACKGROUND =      10000b
EVM_MOUSE      =     100000b
EVM_IPC        =    1000000b
EVM_STACK      =   10000000b
EVM_DEBUG      =  100000000b
EVM_STACK2     = 1000000000b

EVM_MOUSE_FILTER = 0x80000000
EVM_CURSOR_FILTER = 0x40000000
