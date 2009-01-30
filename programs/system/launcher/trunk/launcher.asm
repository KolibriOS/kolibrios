;
;   LAUNCHER - АВТОЗАПУСК ПРОГРАММ
;     Код программы совсем не оптимизирован, но очень прост для понимания.
;     Этот лаунчер грузит информацию о программах для запуска из файла
;     AUTORUN.DAT. Формат очень прост и в комментариях не нуждается.
;
;   Компилируйте с помощью FASM 1.52 и выше
;
include "..\..\..\MACROS.INC"

  use32
  org    0x0
  db     'MENUET01'              ; 8 byte id
  dd     0x01                    ; header version
  dd     START                   ; start of code
  dd     I_END                   ; size of image
  dd     0x8000                 ; memory for app
  dd     0x8000                 ; esp
  dd     0x0 , 0x0               ; I_Param , I_Icon

;include "DEBUG.INC"

START:                           ; start of execution

;   mov  eax, 5
;   mov  ebx, 10
;   mcall

;   mcall 18,15

   mov  eax, 70               ; load AUTORUN.DAT
   mov  ebx, autorun_dat_info
   mcall
   add  ebx, file_data
   mov  [fileend], ebx

; this cycle does not contain an obvious exit condition,
; but auxiliary procedures (like "get_string") will exit
; at EOF
 start_program:
   call skip_spaces
   cmp  al,'#'
   jz   skip_this_string
   call clear_strings
   mov  edi, program
   call get_string
   mov  edi, parameters
   call get_string
   call get_number
;dps <"STARTING A PROGRAM",13,10>
   call run_program
 skip_this_string:
   call next_line
   jmp  start_program

 exit:
   or   eax, -1
   mcall


 run_program:     ; time to delay in eax
   push eax
   mcall 70, start_info
   pop  ebx

; if delay is negative, wait for termination
;   of the spawned process
   test ebx, ebx
   js   must_wait_for_termination
; otherwise, simply wait
   mov  eax, 5
   mcall
   ret
 must_wait_for_termination:
   mov  esi, eax  ; save slot for the future
; get process slot
   mov  ecx, eax
   mcall 18, 21
; if an error has occured, exit
   test eax, eax
   jz   child_exited
   mov  ecx, eax
; wait
 wait_for_termination:
   mcall 5, 1
   mov  ebx, processinfo
   mcall 9
   cmp  word [ebx+50], 9 ; the slot was freed?
   jz   child_exited
   cmp  dword [ebx+30], esi ; the slot is still occupied by our child?
   jz   wait_for_termination
 child_exited:
   ret

 clear_strings:   ; clears buffers
   pushad

   mov  ecx, 60
   mov  edi, program
   xor  al, al ;mov  al, ' '
   rep  stosb

   mov  ecx, 60
   mov  edi, parameters
   rep  stosb

   popad
 ret


 get_string: ; pointer to destination buffer in edi
   pushad
   call skip_spaces
   mov  esi, [position]
;dpd esi
;dps <13,10>
   add  esi, file_data
  .start:
   cmp  esi, [fileend]
   jae  exit
   lodsb
   cmp  al, ' '
   je   .finish
   stosb
   inc  [position]
   jmp  .start
  .finish:
   popad
 ret


 get_number:
   push ebx esi
   call skip_spaces
   mov  esi, [position]
   add  esi, file_data
   xor  eax, eax
   cmp  byte [esi], '-'
   jnz  @f
   inc  eax
   inc  esi
   inc  [position]
@@:
   push eax
   xor  eax, eax
   xor  ebx, ebx
  .start:
   cmp  esi, [fileend]
   jae  .finish
   lodsb
   sub  al, '0'
   cmp  al, 9
   ja   .finish
   lea  ebx,[ebx*4+ebx]
   lea  ebx,[ebx*2+eax]
   inc  [position]
   jmp  .start
  .finish:
   pop  eax
   dec  eax
   jnz  @f
   neg  ebx
@@:
   mov  eax, ebx
   pop  esi ebx
 ret


 skip_spaces:
   push esi
   xor  eax, eax
   mov  esi, [position]
   add  esi, file_data
  .start:
   cmp  esi, [fileend]
   jae  .finish
   lodsb
   cmp  al, ' '
   jne  .finish
   inc  [position]
   jmp  .start
  .finish:
;dps "NOW AL = "
;mov [tmp],al
;mov edx, tmp
;call debug_outstr
;dps <13,10>
   pop  esi
 ret


 next_line:
   mov  esi, [position]
   add  esi, file_data
  .start:
   cmp  esi, [fileend]
   jae  exit
   lodsb
   cmp  al, 13
   je   .finish
   cmp  al, 10
   je   .finish
   inc  [position]
   jmp  .start
  .finish:
   inc  [position]
   cmp  esi, [fileend]
   jae  exit
   lodsb
   cmp  al, 13
   je   .finish
   cmp  al, 10
   je   .finish
   ret



; DATA:
 position          dd 0            ; position in file

 autorun_dat_info:                 ; AUTORUN.DAT
   .mode           dd 0            ; read file
   .start_block    dd 0            ; block to read
                   dd 0
   .blocks         dd 16*512       ; 16*512 bytes max
   .address        dd file_data
   db "AUTORUN.DAT",0

 start_info:
   .mode           dd 7
                   dd 0
   .params         dd parameters
                   dd 0
                   dd 0
   .path: ;      

I_END:

 program           rb 61 ; 60 + [0] char
 parameters        rb 61

 processinfo       rb 1024
 fileend           dd ?

 file_data         rb 16*512
