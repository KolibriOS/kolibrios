;
;   LAUNCHER - АВТОЗАПУСК ПРОГРАММ
;     Код программы совсем не оптимизирован, но очень прост для понимания.
;     Этот лаунчер грузит информацию о программах для запуска из файла
;     AUTORUN.DAT. Формат очень прост и в комментариях не нуждается.
;
;   Компилируйте с помощью FASM 1.52 и выше
;
include "MACROS.INC"

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
;   int  0x40

   mcall 18,15

   mov  eax, 58               ; load AUTORUN.DAT
   mov  ebx, autorun_dat_info
   int  0x40

   call get_number
   mov  [number_of_files], eax
;dps "NUMBER OF FILES: "
;dpd eax
;dps <13,10>
   call next_line

 start_program:
;dps <"STARTING A PROGRAM",13,10>
   call clear_strings
   mov  edi, program
   call get_string
   mov  edi, parameters
   call get_string
   call get_number
   call run_program
   call next_line
   dec  [number_of_files]
   jnz  start_program

 exit:
   or   eax, -1
   int  0x40


 run_program:     ; time to delay in eax
   push eax
   mcall 58, start_info
   pop  ebx

   mov  eax, 5
   int  0x40
 ret


 clear_strings:   ; clears buffers
   pushad

   mov  ecx, 60
   mov  edi, program
   xor  al, al ;mov  al, ' '
   rep  stosb

   mov  ecx, 60
   mov  edi, parameters
   xor  al, al
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
   xor  ebx, ebx
  .start:
   lodsb
   cmp  al, '0'
   jb   .finish
   cmp  al, '9'
   ja   .finish
   sub  al, '0'
   imul ebx, 10
   add  ebx, eax
   inc  [position]
   jmp  .start
  .finish:
   mov  eax, ebx
   pop  esi ebx
 ret


 skip_spaces:
   pushad
   xor  eax, eax
   mov  esi, [position]
   add  esi, file_data
  .start:
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
   popad
 ret


 next_line:
   pushad
   mov  esi, [position]
   add  esi, file_data
  .start:
   lodsb
   cmp  al, 13
   je   .finish
   inc  [position]
   jmp  .start
  .finish:
   add  [position], 2
   inc  esi
   lodsb
   cmp  al, '#'
   je   .skipline
   cmp  al, 13
   jne  .donotskip
  .skipline:
   call next_line
  .donotskip:
   popad
 ret



; DATA:
 position          dd 0            ; position in file

 autorun_dat_info:                 ; AUTORUN.DAT
   .mode           dd 0            ; read file
   .start_block    dd 0            ; block to read
   .blocks         dd 0x10         ; 16*512 bytes max
   .address        dd file_data
   .workarea       dd work_area
   db "/RD/1/AUTORUN.DAT",0

 start_info:
   .mode           dd 16
                   dd 0
   .params         dd parameters
                   dd 0
   .workarea       dd work_area
   .path: ;      

I_END:

 program           rb 61 ; 60 + [0] char
 parameters        rb 61

 number_of_files   dd ?

 work_area         rb 0x4000
 file_data         rb 16*512
