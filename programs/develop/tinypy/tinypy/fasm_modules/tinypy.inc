; Python object types
TP_NONE         equ 0
TP_NUMBER       equ 1
TP_STRING       equ 2
TP_DICT         equ 3
TP_LIST         equ 4
TP_FNC          equ 5
TP_DATA         equ 6

; Python "number" object. Can be treated as integer or float.
struct tp_number_
  type rd 1
  val  rd 1
ends

; GC information for "string" object
struct _tp_string
  gci rd 1
  s   rb 1
ends

; Python "string" object
struct tp_string_
  type rd 1
  info rd 1 ;pointer to _tp_string
  val  rd 1
  len  rd 1
ends

; GC information for "list" object
struct _tp_list
  gci   rd 1
  items rd 1
  len   rd 1
  alloc rd 1
ends

; Python "list" object
struct tp_list_
  type  rd 1
  val   rd 1 ;pointer to _tp_list
ends


; GC information for "dict" object
struct _tp_dict
  gci   rd 1
  items rd 1
  len   rd 1
  alloc rd 1
  cur   rd 1
  mask  rd 1
  used  rd 1
ends

; Python "dict" object
struct tp_dict_
  type  rd 1
  val   rd 1 ;pointer to _tp_dict
ends

; GC information for "function" object
; todo

; Python "function" object
struct tp_fnc_
  type  rd 1
  info  rd 1
  ftype rd 1
  val   rd 1
ends

; Python "data" object
struct tp_data_
  type  rd 1
  info  rd 1
  val   rd 1
  magic rd 1
ends

struct tp_gc_info
  type  rd 1
  data  rd 1
ends

; Generic Python object
struct tp_obj
union
  type  rd 1
  string tp_string_
  number tp_number_
  gc_info tp_gc_info
  dict tp_dict_
  list tp_list_
  fnc  tp_fnc_
  dat tp_data_
ends
ends

; Bytecode element
struct sregs
  .i rb 1
  .a rb 1
  .b rb 1
  .c rb 1
ends

struct tp_code
union
  i      rb 1
  regs   sregs
  string rb 4
  number rp 1
ends
ends

; TinyPy VM frame
struct tp_frame_
  .codes    rd      1
  .cur      rd      1
  .jump     rd      1
  .regs     rd      1
  .ret_dest rd      1
  .fname    rb      sizeof.tp_obj
  .name     rb      sizeof.tp_obj
  .line     rb      sizeof.tp_obj
  .globals  rb      sizeof.tp_obj
  .lineno   rd      1
  .cregs    rd      1
ends

; TinyPy VM
TP_FRAMES equ 256
struct tp_vm
  builtins rb  sizeof.tp_obj
  modules  rb  sizeof.tp_obj
  frames   rb  TP_FRAMES*sizeof.tp_frame_
  _params  rb  sizeof.tp_obj
  params   rb  sizeof.tp_obj
  _regs    rb  sizeof.tp_obj
  regs     rq  1
  root     rb  sizeof.tp_obj
  jmp_buf  rd     1
  jump     rd     1
  ex       rb  sizeof.tp_obj
  chars    rb  512
  cur      rd     1
  white    rb  sizeof._tp_list
  grey     rb  sizeof._tp_list
  black    rb  sizeof._tp_list
  strings  rb  sizeof._tp_dict
  steps    rd     1
ends

macro push_tp_obj obj
{
    local .push_more
    mov  ebx, obj + 12
    mov  ecx, 4
.push_more:
    push dword[ebx]
    sub  ebx, 4
    loop .push_more
}

macro push_tp_obj_at_reg reg
{
    local .push_more
    add reg, 12
    mov ecx, 4
.push_more:
    push dword[reg]
    sub  reg, 4
    loop .push_more
}
