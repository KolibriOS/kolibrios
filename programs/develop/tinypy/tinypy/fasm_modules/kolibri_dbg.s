; TinyPy module
; Name: kolibri_dbg
;
; Exports: 
;           debug_print(msg) - prints a message to debug board.

format ELF
use32
include 'proc32.inc'
include 'struct.inc'
include 'tinypy.inc'

extrn tp_dict
extrn tp_set
extrn tp_get
extrn tp_None
extrn tp_fnc

public kolibri_dbg_init

; Module name
modname        db "kolibri_dbg"
modnamelen = $-modname

; Exported function name
debug_print_name db "debug_print"
debug_print_namelen = $-debug_print_name
; Export dictionary for module
kolibri_dbg_dict    rb sizeof.tp_obj
; tp_obj of exported function
debug_print_obj rb sizeof.tp_obj

; Function debug_print(tp_vm *tp)
; return nothing
debug_print:
     push ebp
     mov  ebp, esp
     ; Store registers
     push eax
     push ebx
     push ecx
     push edx
     sub  esp, sizeof.tp_obj; Reserve tp_obj tmp
     mov  edx, esp
     push edx           ;Store &tmp
     ; Obtain tp_string parameter
     ; tp_get(&tmp, tp, *(tp_obj *)(tp_vm->params), tp_None)
     push_tp_none
     mov  eax, dword[ebp+12] ;4 for return address, 4 for result pointer; 4 for tp_vm
     add  eax, tp_vm.params
     push_tp_obj_at_reg eax
     push dword[ebp+12]
     push edx   ;ebx contains address of reserved tp_obj variable
     call tp_get

     ;Restore stack
     add  esp, sizeof.tp_obj*2+4;2 tp_obj's and 2 pointers in parameters minus 1 pointer to result (cleared inside tp_get)

     ;Leave if parameter is not a string. tp_raise() should be called here.
     pop  edx; edx = &tmp
     cmp  dword[edx], TP_STRING ; Check that function returned a TP_STRING
     jne  .exit
     mov  ecx, dword[edx+tp_string_.len]    ; String length
     mov  edx, dword[edx+tp_string_.val]     ;
     mov  eax, 63
     mov  ebx, 1
.cont:
      ; Print message.
     push ecx   ; Store ecx to use it in inner loop
     mov  cl, byte[edx]
     inc  edx
     int  40h
     pop  ecx   ; Get ecx back
     loop .cont
.exit:
     add  esp, sizeof.tp_obj ; Release tp_obj reserved in stack.
     ; Returning tp_None
     mov  eax, dword[ebp+8]
     mov  dword[eax], 0
     ; Restore registers
     pop  edx
     pop  ecx
     pop  ebx
     pop  eax
     mov  esp, ebp
     pop  ebp
     ret

;void kolibri_dbg_init(tp_vm *tp);
kolibri_dbg_init:
     push ebp
     mov ebp, esp
     ;Save registers
     push eax
     push ebx
     push ecx
     ; Create module dictionary and store its address in kolibri_dbg_str
     mov eax, dword [ebp + 8]
     push eax
     push kolibri_dbg_dict
     call tp_dict
     add  esp, 4        ;Clear stack
     ; Push tp_obj structure pointed by kolibri_dbg_dict
     push_tp_obj kolibri_dbg_dict
     ; Push modname as a tp_obj object
     push modnamelen; len
     push modname   ; val
     push 0         ;_tp_string info
     push TP_STRING ; type
     ; Push tp_obj structure pointed by tp->modules
     mov eax, dword [ebp + 8]
     add eax, tp_vm.modules + 16
     mov ecx, 4
.push_tpobj1:
     sub  eax, 4
     push dword[eax]
loop .push_tpobj1
     push eax
     call tp_set
     add  esp, sizeof.tp_obj*3+4
     ; Register "debug_print" function
     ;Reserve memory for function tp_obj object
     sub  esp, sizeof.tp_obj
     mov  eax, esp
     push debug_print
     push dword[ebp+8]
     push eax
     call tp_fnc
     add  esp, 8; tp_obj is already in stack, adding other arguments
     ;Pushing function name tp_obj
     ;mov eax, esp
     ;push_tp_obj_at_reg eax
     push debug_print_namelen
     push debug_print_name
     push 0
     push TP_STRING
     ;Pushing module dictionary tp_obj
     push_tp_obj kolibri_dbg_dict
     ;Pushing tp_vm
     push dword[ebp+8]
     call tp_set
     add  esp, sizeof.tp_obj*3+4
     ; Leaving function
     pop ecx
     pop ebx
     pop eax
     mov esp, ebp
     pop ebp
     ret
