format PE DLL native
entry START

__DEBUG__               = 1
__DEBUG_LEVEL__         = 2

section '.flat' readable writable executable

include '../../proc32.inc'
include '../../struct.inc'
include '../../macros.inc'
include '../../fdo.inc'

proc START c, reason:dword

        DEBUGF  2,"Loading 'asm_drv' driver\n"
        cmp     [reason], DRV_ENTRY
        jne     .fail
        
        invoke  RegService, my_service, service_proc        
        ret

  .fail:
        xor     eax, eax
        ret

endp

proc service_proc stdcall, ioctl:dword

        mov     ebx, [ioctl]
        mov     eax, [ebx + IOCTL.io_code]
        cmp     eax, 0 ; FUNCTION ADD(a+b);
        jne     .fail
        
        mov     eax, [ebx + IOCTL.input]
        mov     ecx, eax
        add     ecx, 4
        
        mov     eax, [eax]
        add     eax, [ecx]
        
        mov     ecx, [ebx + IOCTL.output]
        mov     [ecx], eax
        
        xor     eax, eax
        ret

  .fail:
        or      eax, -1
        ret
endp

data fixups
end data

include '../../peimport.inc'

my_service   db 'asm_drv',0    ; max 16 chars include zero

include_debug_strings
