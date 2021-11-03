lang equ ru

;
;   Assembler
;     SMALL
;       CODE
;         Arrays
;           Processing
;             Libary
;
;   Ver 0.04
;

; This macro iterates through an array of elements,
; processing each element using the specified process.
; modificate eax,{ebx},{ecx},edx,esi,[edi],[ebp]

macro array_processing object_array,process
{

   movt edi,object_array
   movt ebp,process
   call array_processing_proc

if ~ defined array_processing_used
array_processing_used equ 1
   jmp end_ap
array_processing_proc:
   push ecx
   push ebx
   mov ecx,[edi]   ; dword contain quantity of elements
   mov ebx,[edi+4] ; dword contain size of each element
   add edi,8
ap_read_next:
   ;pop ebx
   ;pop ecx
   pushad
   call ebp ;eax ;process
   popad
   ;push ecx
   ;push ebx
   add edi,ebx
   dec ecx
   jnz ap_read_next
   pop ebx
   pop ecx
   ret
end_ap:
end if
}

; This macro iterates through an array of elements,
; processing each element using the specified process
; if object finded ina array macro returned
; with CF=0 and address of element in edi.
; if object not finded macro return CF=1
; process must be return CF=0 if value euqal finded value
; and CF=1 if value not equal
; modificate eax,{ebx},{ecx},edx,esi,[edi],[ebp]

macro array_find object_array,process
{

   movt edi,object_array
   movt ebp,process
   call array_find_proc

if ~ defined array_find_used
array_find_used equ 1
   jmp end_af
array_find_proc:
   push ecx
   push ebx
   mov ecx,[edi]   ; dword contain quantity of elements
   mov ebx,[edi+4] ; dword contain size of each element
   add edi,8
af_read_next:
   pushad
   call ebp ;eax ;process
   popad
   jnc af_finded
   add edi,ebx
   dec ecx
   jnz af_read_next
; not finded
   stc          ; if not finded CF = 1
af_finded:         ; if finded CF = 0
   pop ebx
   pop ecx
   ret
end_af:
end if
}

;
; process may get offests of elements from registers.

macro compmas object_array_1,object_array_2,process
{
local loo,loo2
   ;lea,[oa+8]
   mov esi,object_array_2     ; current position
   add esi,8
   mov ecx,[object_array_2]   ; dword contain quantity of elements
   mov ebx,[object_array_2+4] ; dword contain size of each element
   mov eax,0
loo2:
   push eax
   mov edi,object_array_1     ; current position
   add edi,8
   mov ebp,[object_array_1]   ; dword contain quantity of elements
   mov edx,[object_array_1+4] ; dword contain size of each element
   mov eax,0 ;count
loo:
   pushad
   call process
   popad
   add  edi,edx
   inc  eax
   cmp  eax,ebp
   jne  loo
   add  esi,ebx
   pop  eax
   inc  eax
   cmp  eax,ecx
   jne  loo2
}



macro array_processing_nocall object_array,process
{
local read_next
   lea edi,[object_array+8]
   mov ecx,[object_array]    ; dword contain quantity of elements
read_next:
   pushad
   call process
   popad
   add edi,[object_array+4] ; edi = edi + size of one element
   dec ecx
   jnz read_next
}

macro array_find_nocall object_array,process
{
local read_next,finded
   lea edi,[object_array+8] ; edi = current position
   mov ecx,[object_array]   ; dword contain quantity of elements
read_next:
   pushad
   call process
   popad
   jnc finded
   add edi,[object_array+4] ; dword contain size of each element
   dec ecx
   jnz read_next
; not finded
   stc          ; if not finded CF = 1
finded:      ; if finded CF = 0
}