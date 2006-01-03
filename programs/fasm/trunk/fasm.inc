center       fix false
SYSTEMCOLORS fix sc
PROCESSINFO  fix pinfo

macro get_sys_colors wnd_skin,font_1 {
 mcall 48,3,SYSTEMCOLORS,sizeof.system_colors
 if wnd_skin <> 0
  or [SYSTEMCOLORS+system_colors.work],0x03000000
 end if
 if font_1 <> 0
  or [SYSTEMCOLORS+system_colors.grab_text],0x10000000
 end if
}

macro draw_caption _edx,_esi {
 mov  edx,_edx
 mov  esi,_esi
 call __draw_caption
}

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

macro __mov reg,a,b {
 if (~a eq)&(~b eq)
   mpack reg,a,b
 else if (~a eq)&(b eq)
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
 int   0x40
}

macro sz name,[data] {
 if used name
  common
   label name
  forward
   db data
  common
   .size = $-name
 end if
}

macro lsz name,[lng,data] {
 if used name
  common
   label name
  forward
   if lang eq lng
    db data
   end if
  common
   .size = $-name
 end if
}

macro mmov reg,a1,a2 {
 mov reg,(a1) shl 16 + (a2)
}

macro madd reg,a1,a2 {
 add reg,(a1) shl 16 + (a2)
}

macro msub reg,a1,a2 {
 sub reg,(a1) shl 16 + (a2)
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
  name:
}

macro endf {
 end if
}

@^ fix macro comment {
^@ fix }

; structure definition helper

;include '%fasminc%struct.inc'
include 'struct.inc'

; structures used in MeOS
struct process_information
  cpu_usage               dd ?  ; +0
  window_stack_position   dw ?  ; +4
  window_stack_value      dw ?  ; +6
  not_used1               dw ?  ; +8
  process_name            db 12 dup(?) ; +10
  memory_start            dd ?  ; +22
  used_memory             dd ?  ; +26
  PID                     dd ?  ; +30
  x_start                 dd ?  ; +34
  y_start                 dd ?  ; +38
  x_size                  dd ?  ; +42
  y_size                  dd ?  ; +46
  slot_state              dw ?  ; +50
  not_used2               db 1024-52 dup(?)
ends

struct system_colors
  frame            dd ?
  grab             dd ?
  grab_button      dd ?
  grab_button_text dd ?
  grab_text        dd ?
  work             dd ?
  work_button      dd ?
  work_button_text dd ?
  work_text        dd ?
  work_graph       dd ?
ends
