
;~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-
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

;~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-
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
struc mls [sstring] {
	forward
	local ssize
	virtual at 0
		db sstring	; mod
		ssize = $
	end virtual
	db ssize
	db sstring
	common

	db -1			; mod
}



; strings
macro sz name,[data] {       ; from MFAR [mike.dld]
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

macro lsz name,[lng,data] {  ; from MFAR [mike.dld]
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

macro __mov reg,a,b {         ; mike.dld
 if (~a eq)&(~b eq)
   mpack reg,a,b
 else if (~a eq)&(b eq)
   mov reg,a
 end if
}

__CPU_type	equ	p5
SYSENTER_VAR	equ	0

macro mcall a,b,c,d,e,f {   ; mike.dld, updated by Ghost for Fast System Calls
 local	..ret_point
 __mov eax,a
 __mov ebx,b
 __mov ecx,c
 __mov edx,d
 __mov esi,e
 __mov edi,f

 if __CPU_type eq p5
	int	0x40
 else
  if __CPU_type eq p6
	push	ebp
	mov	ebp, esp
	push	..ret_point	; it may be 2 or 5 byte
	sysenter
 ..ret_point:
	pop	edx
	pop	ecx

  else
   if __CPU_type eq k6
	push	ecx
	syscall
	pop	ecx
   else
	display 'ERROR : unknown CPU type'
	int	0x40
   end if
  end if
 end if
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
   if (arg1 in __regs) & ((arg2 eqtype 0) | (arg2 eqtype '0'))
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


macro struct name
 {
  virtual at 0
   name name
   sizeof.#name = $ - name
  end virtual
 }

; structures used in MeOS
struc process_information
 {
  .cpu_usage               dd ?  ; +0
  .window_stack_position   dw ?  ; +4
  .window_stack_value      dw ?  ; +6
  .not_used1               dw ?  ; +8
  .process_name            rb 12 ; +10
  .memory_start            dd ?  ; +22
  .used_memory             dd ?  ; +26
  .PID                     dd ?  ; +30
  .x_start                 dd ?  ; +34
  .y_start                 dd ?  ; +38
  .x_size                  dd ?  ; +42
  .y_size                  dd ?  ; +46
  .slot_state              dw ?  ; +50
                           dw ?  ; +52 - reserved
  .client_left             dd ?  ; +54
  .client_top              dd ?  ; +58
  .client_width            dd ?  ; +62
  .client_height           dd ?  ; +66
  .wnd_state               db ?  ; +70
  rb (1024-71)
 }
struct process_information

struc system_colors
 {
  .frame            dd ?
  .grab             dd ?
  .grab_button      dd ?
  .grab_button_text dd ?
  .grab_text        dd ?
  .work             dd ?
  .work_button      dd ?
  .work_button_text dd ?
  .work_text        dd ?
  .work_graph       dd ?
 }
struct system_colors


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
EVM_REDRAW     =        1b
EVM_KEY        =       10b
EVM_BUTTON     =      100b
EVM_EXIT       =     1000b
EVM_BACKGROUND =    10000b
EVM_MOUSE      =   100000b
EVM_IPC        =  1000000b
EVM_STACK      = 10000000b