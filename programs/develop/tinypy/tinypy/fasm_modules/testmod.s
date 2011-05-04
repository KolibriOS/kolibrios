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

public testmod_init
public getsize

;Module name
modname        db "testmod"
modnamelen = $-modname
debug_print_name db "debug_print"
debug_print_namelen = $-debug_print_name

testmod_dict    rb sizeof.tp_obj
debug_print_obj rb sizeof.tp_obj
tmp_tp_obj      rb sizeof.tp_obj

getsize:
        mov eax, tp_vm.params
        ret

;void debug_print(tp_vm *tp)
debug_print:
     push ebp
     mov  ebp, esp
     ; Store registers
     push eax
     push ebx
     push ecx
     push edx
     ;Reserve space for tp_obj variable in stack
     sub  esp, sizeof.tp_obj
     ; Obtain tp_string parameter
     push_tp_obj tp_None
     mov  eax, dword[ebp+8]
     add  eax, tp_vm.params
     push_tp_obj_at_reg eax
     push dword[ebp+8]
     push tmp_tp_obj;esp+(sizeof.tp_obj*2+4)
     call tp_get
     ;Restore stack
     add  esp, 56;sizeof.tp_obj*3+4;8?
     ;Leave if parameter is not a string. tp_raise() should be called here.
     ;cmp  dword[esp], TP_STRING
     ;jne  .exit
     ;mov  ecx, dword[esp+12]
     ;mov  edx, dword[esp+8]
;.cont:
     ; Print message.
;     mov  eax, 63
;     mov  ebx, 1
;     push ecx
;     mov  cl, byte[edx]
;     inc  edx
;     int  40h
;     pop  ecx
;     loop .cont
;.exit:
     pop  edx
     pop  ecx
     pop  ebx
     pop  eax
     mov  esp, ebp
     pop  ebp
     ret

;void testmod_init(tp_vm *tp);
testmod_init:
     push ebp
     mov ebp, esp
     ;Save registers
     push eax
     push ebx
     push ecx
     ; Create module dictionary and store its address in testmod_str
     mov eax, dword [ebp + 8]
     push eax
     push testmod_dict
     call tp_dict
     add  esp, 4        ;Clear stack
     ; Push tp_obj structure pointed by testmod_dict
     push_tp_obj testmod_dict
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
     push_tp_obj testmod_dict
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
