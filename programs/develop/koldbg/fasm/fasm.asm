        use32

open:   cmp     edx,input_magic
        jne     fail
        mov     ebx,[edx]
        ret

close:  cmp     ebx,[input_magic]
        jne     fail
        xor     ebx,ebx         ;CF=0
        ret

lseek:  cmp     ebx,[input_magic]
        jne     fail

        cmp     al,2
        je      .end
        cmp     al,1
        je      .pos
        cmp     al,0
        jne     fail

      .str:
        mov     eax,edx
        cmp     eax,[input_size]
        ja      fail
        jmp     .ok

      .end:
        mov     eax,[input_size]
        neg     edx
        add     eax,edx
        jc      fail
      .ok:
        mov     [input_start],eax
        clc
        ret

      .pos:
        mov     eax,[input_start]
        add     eax,edx
        cmp     eax,[input_size]
        ja      fail
        jmp     .ok

read:   cmp     ebx,[input_magic]
        jne     fail
        push    esi edi
        mov     esi,[input_start]
        add     esi,[input_code]
        mov     eax,[input_size]
        mov     edi,edx
        cmp     eax,ecx
        jnc     skip
        mov     ecx,eax
skip:   mov     eax,ecx
        add     [input_start],eax
        shr     ecx,2
        cld
        rep     movsd
        mov     cl,al
        and     cl,3
        rep     movsb
        pop     edi esi
        clc
        ret

write:  ;not implemented
create: ;not implemented

fail:   stc
        ret

make_timestamp:
        xor     eax,eax         ;not implemented
get_environment_variable:       ;not implemented
display_block:                  ;not implemented
        ret

assembler_error: fatal_error:
        pop     [output_errs]
        stc
        jmp     stack_restore

Assemble:
        xor     eax,eax
        mov     [output_errs],eax
        mov     [output_size],eax
        mov     [input_start],eax

        mov     [input_file],input_magic
        mov     [input_size],ecx
        mov     [input_code],esi
        mov     [input_stack],esp

        mov     [memory_start],StdMemStr
        mov     [memory_end],StdMemEnd
        mov     [additional_memory],AddMemStr
        mov     [additional_memory_end],AddMemEnd

        lea     eax,[esp-4*1024]
        mov     [stack_limit],eax

        call    preprocessor
        call    parser
        call    assembler

        mov     esi,[code_start]
        mov     ecx,edi
        sub     ecx,esi

        mov     eax,256
        cmp     ecx,eax
        jna     max_256
        mov     ecx,eax
      max_256:

        mov     [output_size],ecx
        mov     edi,output_data
        rep     movsb

     stack_restore:

        mov     esp,[input_stack]
        mov     esi,[output_errs]
        ret

include 'fasm\version.inc'
include 'fasm\errors.inc'
include 'fasm\symbdump.inc'
include 'fasm\preproce.inc'
include 'fasm\parser.inc'
include 'fasm\exprpars.inc'
include 'fasm\assemble.inc'
include 'fasm\exprcalc.inc'
include 'fasm\formats.inc'
include 'fasm\x86_64.inc'
include 'fasm\avx.inc'

include 'fasm\tables.inc'
include 'fasm\messages.inc'

iglobal
input_magic     dd 55AA55AAh
endg
uglobal
input_code      dd ?
input_size      dd ?
input_start     dd ?
input_stack     dd ?
output_size     dd ?
output_errs     dd ?
output_data     rb 256
include 'fasm\variable.inc'
endg
