; to generate  random  colors use "fillscr rnd"  ;
; otherwise use "filscr r,g,b, r,g,b, r,g,b,..." ;
use32
	org 0
	db 'MENUET01'
version dd 1
	dd program.start
	dd program.end
	dd program.memory
	dd program.stack
	dd program.params
	dd 0
; ---------------------------- ;

include '../../macros.inc'
include '../../KOSfuncs.inc'

; ---------------------------- ;
POINTS_SIZE = 108
points_count   dd 0
rnd            dd 0
; ---------------------------- ;
align 4
program.start:
        mov    edi, points
        mov    esi, program.params
        cmp    [esi], dword "rnd"
        jne    .not_rnd
        mov    [points_count], dword POINTS_SIZE / 3
        mov    ecx, POINTS_SIZE / 2
.next_rnd:
        rdtsc
        xor    eax, edx
        xor    [rnd], eax
        imul   eax, [rnd], 134775813
        add    eax, 2531011
        mov    [rnd], eax
        shr    eax, 16
        mov    [edi], ax
        inc    edi
        inc    edi
        loop   .next_rnd
        jmp    .exit
.not_rnd:
        xor    ebp, ebp
.next:
.skip_spaces:
        cmp    [esi], byte " "
        jne    .spaces_skipped
        inc    esi
        jmp    .skip_spaces
.spaces_skipped:
;         cmp [esi], byte 0
;         je .exit
        mov    eax, esi
.find_end_or_comma:
        cmp    [esi], byte ","
        je     .end_or_comma_found
        cmp    [esi], byte 0
        je     .end_or_comma_found
        inc    esi
        jmp    .find_end_or_comma
.end_or_comma_found:
; in eax start   of number
; in esi pointer to comma or end after number
        mov    ecx, esi
        sub    ecx, eax
        xor    ebx, ebx
        cmp    cl, 1
        jne    .cmp2
        xor    cl, cl
        jmp    .1
.cmp2:
        cmp    cl, 2
        jne    .cmp3
        xor    cl, cl
        jmp    .2
.cmp3:
        cmp    cl, 3
        jne    .exit
        xor    cl, cl
.3:
        movzx  edx, byte[eax + ecx]
        sub    dl, 48
        shl    dl, 2
        lea    edx, [edx * 4 + edx]
        lea    edx, [edx * 4 + edx]
        add    bl, dl
        inc    ecx
.2:
        movzx  edx, byte[eax + ecx]
        sub    dl, 48
        shl    dl,  1
        lea    edx, [edx * 4 + edx]
        add    bl, dl
        inc    ecx
.1:
        movzx  edx, byte[eax + ecx]
        sub    dl, 48
        add    bl, dl
        test   ebp, ebp
        jnz    .cmp_next
        mov    [edi + 2], bl
        jmp    .putted
.cmp_next:
        cmp    ebp, 2
        jne    .cmp_next1
        mov    [edi - 2], bl
        jmp    .putted
.cmp_next1:
        mov    [edi], bl
.putted:
        inc    ebp
        cmp    ebp, 3
        jne    .not_3
        xor    ebp, ebp
        inc    dword[points_count]
.not_3:
        inc    edi
        inc    esi
        jmp    .next
.exit:
; width = height = sqrt(points_count)
        mov    eax, [points_count]
        or     edx, -1
@@:
        add    edx, 2
        sub    eax, edx
        jnle   @b
        sbb    edx, -1
        shr    edx, 1
; set width, height
        mov    ecx, edx
        mcall SF_BACKGROUND_SET,SSF_SIZE_BG
; set "stretch"
        mcall ,SSF_MODE_BG
; put pixels
        xor    edx, edx
        mov    esi, [points_count] ; size of data = count * 3
        lea    esi, [esi * 2 + esi]
        mcall ,SSF_IMAGE_BG, points       ; BBGGRRBBGGRR...
; refresh screen
        mcall ,SSF_REDRAW_BG
; thread terminate
        mcall SF_TERMINATE_PROCESS

align 4
program.end:
	points rb POINTS_SIZE
	program.params rb 256
	rb 256
align 16
program.stack:
program.memory: