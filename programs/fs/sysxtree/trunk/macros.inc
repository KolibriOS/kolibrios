; language for programs
lang fix en ; ru en fr ge fi

@^ fix macro comment {
^@ fix }


macro m2m dest,src {
 push src
 pop  dest
}

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


; strings
macro sz name,[data] {         ; from MFAR [mike.dld]
 common
  if used name
   name db data
   .size = $-name
  end if
}

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
   virtual at 0
    db data
    s = $
   end virtual
   d#elsz s
   if m < s
    m = s
   end if
   db data
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

;macro __mov reg,a {         ; mike.dld
; if ~a eq
;   mov reg,a
; end if
;}

macro __mov reg,a,b {        ; mike.dld
 if (~a eq)&(~b eq)
   mpack reg,a,b
 else if (~a eq)&(b eq)
   mov reg,a
 end if
}

macro mcall a,b,c,d,e,f {   ; mike.dld
 __mov eax,a
 __mov ebx,b
 __mov ecx,c
 __mov edx,d
 __mov esi,e
 __mov edi,f
 int   0x40
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

; optimize the code for size
__regs fix <eax,ebx,ecx,edx,esi,edi,ebp,esp>

macro add arg1,arg2
 {
   if (arg2 eqtype 0)
      if (arg2) = 1
     inc arg1
      else
     add arg1,arg2
      end if
   else
      add arg1,arg2
   end if
 }

macro sub arg1,arg2
 {
   if (arg2 eqtype 0)
      if (arg2) = 1
     dec arg1
      else
     sub arg1,arg2
      end if
   else
      sub arg1,arg2
   end if
 }

macro mov arg1,arg2
 {
   if (arg1 in __regs) & (arg2 eqtype 0)
      if (arg2) = 0
     xor arg1,arg1
      else if (arg2) = 1
     xor arg1,arg1
     inc arg1
      else if (arg2) = -1
     or  arg1,-1
      else if (arg2) > -128 & (arg2) < 128
     push arg2
     pop  arg1
      else
     mov  arg1,arg2
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


; Macroinstructions for defining data structures

macro struct name
 { fields@struct equ name
   match child parent, name \{ fields@struct equ child,fields@\#parent \}
   sub@struct equ
   struc db [val] \{ \common fields@struct equ fields@struct,.,db,<val> \}
   struc dw [val] \{ \common fields@struct equ fields@struct,.,dw,<val> \}
   struc du [val] \{ \common fields@struct equ fields@struct,.,du,<val> \}
   struc dd [val] \{ \common fields@struct equ fields@struct,.,dd,<val> \}
   struc dp [val] \{ \common fields@struct equ fields@struct,.,dp,<val> \}
   struc dq [val] \{ \common fields@struct equ fields@struct,.,dq,<val> \}
   struc dt [val] \{ \common fields@struct equ fields@struct,.,dt,<val> \}
   struc rb count \{ fields@struct equ fields@struct,.,db,count dup (?) \}
   struc rw count \{ fields@struct equ fields@struct,.,dw,count dup (?) \}
   struc rd count \{ fields@struct equ fields@struct,.,dd,count dup (?) \}
   struc rp count \{ fields@struct equ fields@struct,.,dp,count dup (?) \}
   struc rq count \{ fields@struct equ fields@struct,.,dq,count dup (?) \}
   struc rt count \{ fields@struct equ fields@struct,.,dt,count dup (?) \}
   macro db [val] \{ \common \local anonymous
             fields@struct equ fields@struct,anonymous,db,<val> \}
   macro dw [val] \{ \common \local anonymous
             fields@struct equ fields@struct,anonymous,dw,<val> \}
   macro du [val] \{ \common \local anonymous
             fields@struct equ fields@struct,anonymous,du,<val> \}
   macro dd [val] \{ \common \local anonymous
             fields@struct equ fields@struct,anonymous,dd,<val> \}
   macro dp [val] \{ \common \local anonymous
             fields@struct equ fields@struct,anonymous,dp,<val> \}
   macro dq [val] \{ \common \local anonymous
             fields@struct equ fields@struct,anonymous,dq,<val> \}
   macro dt [val] \{ \common \local anonymous
             fields@struct equ fields@struct,anonymous,dt,<val> \}
   macro rb count \{ \local anonymous
             fields@struct equ fields@struct,anonymous,db,count dup (?) \}
   macro rw count \{ \local anonymous
             fields@struct equ fields@struct,anonymous,dw,count dup (?) \}
   macro rd count \{ \local anonymous
             fields@struct equ fields@struct,anonymous,dd,count dup (?) \}
   macro rp count \{ \local anonymous
             fields@struct equ fields@struct,anonymous,dp,count dup (?) \}
   macro rq count \{ \local anonymous
             fields@struct equ fields@struct,anonymous,dq,count dup (?) \}
   macro rt count \{ \local anonymous
             fields@struct equ fields@struct,anonymous,dt,count dup (?) \}
   macro union \{ fields@struct equ fields@struct,,union,<
          sub@struct equ union \}
   macro struct \{ fields@struct equ fields@struct,,substruct,<
          sub@struct equ substruct \}
   virtual at 0 }

macro ends
 { match , sub@struct \{ restruc db,dw,du,dd,dp,dq,dt
             restruc rb,rw,rd,rp,rq,rt
             purge db,dw,du,dd,dp,dq,dt
             purge rb,rw,rd,rp,rq,rt
             purge union,struct
             match name=,fields,fields@struct \\{ fields@struct equ
                                  make@struct name,fields
                                  fields@\\#name equ fields \\}
             end virtual \}
   match any, sub@struct \{ fields@struct equ fields@struct> \}
   restore sub@struct }

macro make@struct name,[field,type,def]
 { common
    if $
     display 'Error: definition of ',`name,' contains illegal instructions.',0Dh,0Ah
     err
    end if
    local define
    define equ name
   forward
    local sub
    match , field \{ make@substruct type,name,sub def
             define equ define,.,sub, \}
    match any, field \{ define equ define,.#field,type,<def> \}
   common
    match fields, define \{ define@struct fields \} }

macro define@struct name,[field,type,def]
 { common
    local list
    list equ
   forward
    if ~ field eq .
     name#field type def
     sizeof.#name#field = $ - name#field
    else
     rb sizeof.#type
    end if
    local value
    match any, list \{ list equ list, \}
    list equ list <value>
   common
    sizeof.#name = $
    restruc name
    match values, list \{
    struc name value \\{
    match any, fields@struct \\\{ fields@struct equ fields@struct,.,name,<values> \\\}
    match , fields@struct \\\{ label .
   forward
     match , value \\\\{ field type def \\\\}
     match any, value \\\\{ field type value
               if ~ field eq .
                rb sizeof.#name#field - ($-field)
               end if \\\\}
   common \\\} \\} \} }

macro enable@substruct
 { macro make@substruct substruct,parent,name,[field,type,def]
    \{ \common
    \local define
    define equ parent,name
       \forward
    \local sub
    match , field \\{ match any, type \\\{ enable@substruct
                           make@substruct type,name,sub def
                           purge make@substruct
                           define equ define,.,sub, \\\} \\}
    match any, field \\{ define equ define,.\#field,type,<def> \\}
       \common
    match fields, define \\{ define@\#substruct fields \\} \} }

enable@substruct

macro define@union parent,name,[field,type,def]
 { common
    virtual at 0
   forward
    if ~ field eq .
     virtual at 0
      parent#field type def
      sizeof.#parent#field = $ - parent#field
     end virtual
     if sizeof.#parent#field > $
      rb sizeof.#parent#field - $
     end if
    else if sizeof.#type > $
     rb sizeof.#type - $
    end if
   common
    sizeof.#name = $
    end virtual
    struc name [value] \{ \common
    label .\#name
    last@union equ
   forward
    match any, last@union \\{ virtual at .\#name
                   field type def
                  end virtual \\}
    match , last@union \\{ match , value \\\{ field type def \\\}
               match any, value \\\{ field type value \\\} \\}
    last@union equ field
   common rb sizeof.#name - ($ - .\#name) \} }

macro define@substruct parent,name,[field,type,def]
 { common
    virtual at 0
   forward
    if ~ field eq .
     parent#field type def
     sizeof.#parent#field = $ - parent#field
    else
     rb sizeof.#type
    end if
    local value
   common
    sizeof.#name = $
    end virtual
    struc name value \{
    label .\#name
   forward
     match , value \\{ field type def \\}
     match any, value \\{ field type value
              if ~ field eq .
               rb sizeof.#parent#field - ($-field)
              end if \\}
   common \} }

; structures used in MeOS
struct process_information
  cpu_usage          dd ?    ; +0
  window_stack_position   dw ?    ; +4
  window_stack_value      dw ?    ; +6
  not_used1          dw ?    ; +8
  process_name          rb 12 ; +10
  memory_start          dd ?    ; +22
  used_memory          dd ?    ; +26
  PID              dd ?    ; +30
  x_start          dd ?    ; +34
  y_start          dd ?    ; +38
  x_size          dd ?    ; +42
  y_size          dd ?    ; +46
  slot_state          dw ?    ; +50
  rb (1024-52)
ends

struct system_colors
  frame        dd ?
  grab           dd ?
  grab_button       dd ?
  grab_button_text dd ?
  grab_text       dd ?
  work           dd ?
  work_button       dd ?
  work_button_text dd ?
  work_text       dd ?
  work_graph       dd ?
ends


; constants

; events
EV_IDLE        = 0
EV_TIMER       = 0
EV_REDRAW      = 1
EV_KEY           = 2
EV_BUTTON      = 3
EV_EXIT        = 4
EV_BACKGROUND  = 5
EV_MOUSE       = 6
EV_IPC           = 7
EV_STACK       = 8

; event mask bits for function 40
EVM_REDRAW     =    1b
EVM_KEY        =       10b
EVM_BUTTON     =      100b
EVM_EXIT       =     1000b
EVM_BACKGROUND =    10000b
EVM_MOUSE      =   100000b
EVM_IPC        =  1000000b
EVM_STACK      = 10000000b
