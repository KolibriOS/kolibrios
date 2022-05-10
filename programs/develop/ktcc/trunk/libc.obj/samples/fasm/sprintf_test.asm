; 2022, Edited by Coldy
; Added changes for auto load & linking

format binary as "kex"

use32
        org     0x0
        db      'MENUET01'      
        dd      0x02          ; 2 - for enable autoload feature            
        dd      START           
        dd      IM_END          
        dd      MEM  
        dd      MEM
        dd      0
        dd      0
        
        ; { Begin of KX header (this need for recognition by dll.obj)
        
        db  'KX',0            ; Signature/Magic 
        db  0                 ; Revision
        db  1000000b          ; Flags0  (user app w/import)
        db  0                 ; Flags1 
        dw  0                 ; Reserved
   
        dd  @IMPORT           ; Pointer to import table
        
        ; } End of KX header

include '../../../../../../macros.inc'
include '../../../../../../proc32.inc'
include '../../../../../../KOSfuncs.inc'

; no needed for autoload 
;include '../../../../../dll.inc'
;include '../../../../../debug-fdo.inc'

;__DEBUG__               = 1
;__DEBUG_LEVEL__         = 2

START:
        ; Disabled, dll.obj doing this
        ;stdcall dll.Load, @IMPORT
        ;test    eax, eax
        ;jnz     exit

        cinvoke libc_strlen, test_str1
        ;DEBUGF 2, "%d", eax
        mcall   SF_SYS_MISC, SSF_MEM_ALLOC, eax
        mov     [test_str2], eax
        
        cinvoke libc_sprintf, [test_str2], format_str, str_var, [dec_var], dword [float_var], dword[float_var+4], [hex_var]
        cinvoke libc_puts, test_str1
        cinvoke libc_puts, [test_str2]
        
        cinvoke libc_strcmp, test_str1, [test_str2]
        
        test    eax, eax
        jz      print_succ
        jmp     print_fail
        
print_succ:
        cinvoke libc_puts, success_msg
        jmp exit
        
print_fail:
        cinvoke libc_puts, failed_msg
        
exit:
        mcall   SF_SYS_MISC, SSF_MEM_FREE, [test_str2]
        //mcall   SF_TERMINATE_PROCESS - disabled, dll.obj also doing this
        ret
        
; data

format_str  db "%s %d %f 0x%x", 0
test_str1   db "Test 463 -26.432100 0x9d81", 0
test_str2   dd 0

str_var     db "Test",0
dec_var     dd 463
float_var   dq -26.4321
hex_var     dd 40321

success_msg db "Test successful!", 0
failed_msg  db "Test failed!", 0
        
align 4

@IMPORT:
library libc,   'libc.obj'
import  libc,   \
        libc_sprintf, 'sprintf', \
        libc_strcmp, 'strcmp', \
        libc_strlen, 'strlen', \
        libc_puts, 'puts'

IM_END:
align   4
rb      1024    ; stack
MEM: 
