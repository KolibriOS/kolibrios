; language for programs
lang fix en ; ru en fr ge fi

@^ fix macro comment {
^@ fix }

; strings
macro sz name,[data] {	     ; from MFAR [mike.dld]
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

macro szc name,elsz,[data] {	     ; from MFAR [mike.dld]
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
  local s,m
  m = 0
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
  end if
 common
   .size = $-name
   .maxl = m
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

macro __mov reg,a,b {	    ; mike.dld
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
macro section name,algn
 {
  local boundary
  boundary = 16
  if ~algn eq
   boundary = algn
  end if
  align boundary
  label name
 }
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

macro jif _op1,_cond,_op2,_label,_op
 {
  if _op eq
   cmp _op1,_op2
  else
   _op _op1,_op2
  end if
  j#_cond _label
 }

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


struc POINT _t,_dx,_dy {
 .x _t _dx
 .y _t _dy
}

; structure definition helper
macro struct name, [arg]
 {
  common
   name@struct fix name
   struc name arg {
 }

macro struct_helper name
 {
   virtual at 0
   name name
   sizeof.#name = $ - name
   name equ sizeof.#name
   end virtual
 }

ends fix } struct_helper name@struct

macro union [def]
 {
   common size@union = 0
	  origin@union = $
   forward virtual
	    def
	    if $-origin@union > size@union
	     size@union = $-origin@union
	    end if
	   end virtual
   common rb size@union
 }

; structures used in MeOS
struc process_information
{
  .cpu_usage		   dd ?  ; +0
  .window_stack_position   dw ?  ; +4
  .window_stack_value	   dw ?  ; +6
  .not_used1		   dw ?  ; +8
  .process_name 	   rb 12 ; +10
  .memory_start 	   dd ?  ; +22
  .used_memory		   dd ?  ; +26
  .PID			   dd ?  ; +30
  .x_start		   dd ?  ; +34
  .y_start		   dd ?  ; +38
  .x_size		   dd ?  ; +42
  .y_size		   dd ?  ; +46
  .slot_state		   dw ?  ; +50
  rb (1024-52)
}

struc system_colors
{
  .frame	    dd ?
  .grab 	    dd ?
  .grab_button	    dd ?
  .grab_button_text dd ?
  .grab_text	    dd ?
  .work 	    dd ?
  .work_button	    dd ?
  .work_button_text dd ?
  .work_text	    dd ?
  .work_graph	    dd ?
}


; constants

; events
EV_IDLE        = 0
EV_TIMER       = 0
EV_REDRAW      = 1
EV_KEY	       = 2
EV_BUTTON      = 3
EV_EXIT        = 4
EV_BACKGROUND  = 5
EV_MOUSE       = 6
EV_IPC	       = 7
EV_STACK       = 8

; event mask bits for function 40
EVM_REDRAW     =	1b
EVM_KEY        =       10b
EVM_BUTTON     =      100b
EVM_EXIT       =     1000b
EVM_BACKGROUND =    10000b
EVM_MOUSE      =   100000b
EVM_IPC        =  1000000b
EVM_STACK      = 10000000b